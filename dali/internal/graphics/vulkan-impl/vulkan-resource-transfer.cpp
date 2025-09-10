/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dali/internal/graphics/vulkan-impl/vulkan-resource-transfer.h>

#include <dali/integration-api/pixel-data-integ.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-fence-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-resource-transfer-request.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-texture.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-utils.h>

namespace Dali::Graphics::Vulkan
{
/**
 * If a texture appears more than once we need to process it preserving the order
 * of updates. It's necessary to make sure that all updates will run on
 * the same thread.
 */
struct TextureTask
{
  TextureTask(const Dali::Graphics::TextureUpdateInfo* i, const Dali::Task& t)
  : pInfo(i),
    copyTask(t)
  {
  }
  const Dali::Graphics::TextureUpdateInfo* pInfo;
  Dali::Task                               copyTask;
};

static bool TestCopyRectIntersection(const ResourceTransferRequest* srcRequest, const ResourceTransferRequest* currentRequest)
{
  auto srcOffset = srcRequest->bufferToImageInfo.copyInfo.imageOffset;
  auto srcExtent = srcRequest->bufferToImageInfo.copyInfo.imageExtent;

  auto curOffset = currentRequest->bufferToImageInfo.copyInfo.imageOffset;
  auto curExtent = currentRequest->bufferToImageInfo.copyInfo.imageExtent;

  auto offsetX0 = std::min(srcOffset.x, curOffset.x);
  auto offsetY0 = std::min(srcOffset.y, curOffset.y);
  auto offsetX1 = std::max(srcOffset.x + int32_t(srcExtent.width), curOffset.x + int32_t(curExtent.width));
  auto offsetY1 = std::max(srcOffset.y + int32_t(srcExtent.height), curOffset.y + int32_t(curExtent.height));

  return ((offsetX1 - offsetX0) < (int32_t(srcExtent.width) + int32_t(curExtent.width)) &&
          (offsetY1 - offsetY0) < (int32_t(srcExtent.height) + int32_t(curExtent.height)));
}

ResourceTransfer::ResourceTransfer(VulkanGraphicsController& graphicsController)
: mGraphicsController(graphicsController)
{
}

void ResourceTransfer::Initialize()
{
  mThreadPool.Initialize();
}

void ResourceTransfer::UpdateTextures(
  const std::vector<Dali::Graphics::TextureUpdateInfo>&       updateInfoList,
  const std::vector<Dali::Graphics::TextureUpdateSourceInfo>& sourceList)
{
  UpdateWithFutures(updateInfoList, sourceList);
  // UpdateSerially(updateInfoList, sourceList);
}

void ResourceTransfer::UpdateSerially(
  const std::vector<Dali::Graphics::TextureUpdateInfo>&       updateInfoList,
  const std::vector<Dali::Graphics::TextureUpdateSourceInfo>& sourceList)
{
  std::vector<uint8_t*>        memoryDiscardQ;
  std::vector<Dali::PixelData> pixelDataDiscardQ;

  // Sort into tasks per texture.
  std::map<Dali::Graphics::Texture*, std::vector<TextureTask>> updateMap;
  for(auto& info : updateInfoList)
  {
    updateMap[info.dstTexture].emplace_back(&info, nullptr);
  }

  // make a copy of update info lists by storing additional information
  for(auto& aTextureInfo : updateMap)
  {
    auto gfxTexture = aTextureInfo.first;
    auto texture    = static_cast<Vulkan::Texture*>(gfxTexture);

    for(auto& textureTask : aTextureInfo.second)
    {
      auto&       info   = *textureTask.pInfo;
      const auto& source = sourceList[info.srcReference];
      if(source.sourceType == TextureUpdateSourceInfo::Type::MEMORY ||
         source.sourceType == TextureUpdateSourceInfo::Type::PIXEL_DATA)
      {
        uint8_t* sourcePtr    = nullptr;
        Format   sourceFormat = Format::UNDEFINED;
        if(source.sourceType == TextureUpdateSourceInfo::Type::MEMORY)
        {
          sourcePtr = reinterpret_cast<uint8_t*>(source.memorySource.memory);
          memoryDiscardQ.push_back(sourcePtr);
        }
        else
        {
          auto pixelBufferData = Integration::GetPixelDataBuffer(source.pixelDataSource.pixelData);
          sourcePtr            = pixelBufferData.buffer + info.srcOffset;
          sourceFormat         = ConvertPixelFormat(source.pixelDataSource.pixelData.GetPixelFormat());

          if(Dali::Integration::IsPixelDataReleaseAfterUpload(source.pixelDataSource.pixelData) &&
             info.srcOffset == 0u)
          {
            pixelDataDiscardQ.push_back(source.pixelDataSource.pixelData);
          }
        }

        // Create a staging buffer for this image task.
        const auto stagingBufferSize = texture->GetMemoryRequirements().size;
        InitializeTextureStagingBuffer(stagingBufferSize, false);
        DALI_ASSERT_ALWAYS(mTextureStagingBufferMappedPtr != nullptr);

        texture->InitializeImageView();
        const auto& properties = texture->GetProperties();

        if(properties.emulated)
        {
          texture->TryConvertPixelData(sourcePtr, info.srcSize, info.srcExtent2D.width, info.srcExtent2D.height, mTextureStagingBufferMappedPtr);
        }
        else if(sourceFormat != Format::UNDEFINED && sourceFormat != texture->GetCreateInfo().format)
        {
          texture->TryConvertPixelData(sourcePtr, sourceFormat, info.srcSize, info.srcExtent2D.width, info.srcExtent2D.height, mTextureStagingBufferMappedPtr);
        }
        else
        {
          std::copy(sourcePtr, sourcePtr + info.srcSize, reinterpret_cast<uint8_t*>(mTextureStagingBufferMappedPtr));
        }
        UnmapTextureStagingBuffer();

        switch(source.sourceType)
        {
          // directly copy buffer
          case TextureUpdateSourceInfo::Type::BUFFER:
          {
            CopyBufferAndTransition(*this,
                                    *texture,
                                    *source.bufferSource.buffer,
                                    info.srcOffset,
                                    info.srcExtent2D,
                                    info.dstOffset2D,
                                    info.layer, // layer
                                    info.level, // mipmap
                                    {});        // update mode, deprecated
            break;
          }
          // for memory, use staging buffer
          case TextureUpdateSourceInfo::Type::PIXEL_DATA:
          case TextureUpdateSourceInfo::Type::MEMORY:
          {
            CopyBufferAndTransition(*this,
                                    *texture,
                                    *mTextureStagingBuffer,
                                    0,
                                    info.srcExtent2D,
                                    info.dstOffset2D,
                                    info.layer, // layer
                                    info.level, // mipmap
                                    {});        // update mode, deprecated
            break;
          }

          case Dali::Graphics::TextureUpdateSourceInfo::Type::TEXTURE:
            // Unsupported
            break;
        }
      }
    }
  }

  // Free source data
  for(uint8_t* ptr : memoryDiscardQ)
  {
    free(reinterpret_cast<void*>(ptr));
  }
  for(PixelData pixelData : pixelDataDiscardQ)
  {
    Dali::Integration::ReleasePixelDataBuffer(pixelData);
  }
}

void ResourceTransfer::UpdateWithFutures(
  const std::vector<Dali::Graphics::TextureUpdateInfo>&       updateInfoList,
  const std::vector<Dali::Graphics::TextureUpdateSourceInfo>& sourceList)
{
  using MemoryUpdateAndOffset = std::pair<const Dali::Graphics::TextureUpdateInfo*, uint32_t>;
  std::vector<MemoryUpdateAndOffset> relevantUpdates{};

  std::vector<Task> copyTasks{};

  relevantUpdates.reserve(updateInfoList.size());
  copyTasks.reserve(updateInfoList.size());

  uint32_t totalStagingBufferSize{0u};

  void* stagingBufferMappedPtr = nullptr;

  std::vector<uint8_t*>        memoryDiscardQ;
  std::vector<Dali::PixelData> pixelDataDiscardQ;

  std::map<Dali::Graphics::Texture*, std::vector<TextureTask>> updateMap;
  for(auto& info : updateInfoList)
  {
    updateMap[info.dstTexture].emplace_back(&info, nullptr);
  }

  // make a copy of update info lists by storing additional information
  for(auto& aTextureInfo : updateMap)
  {
    auto gfxTexture = aTextureInfo.first;
    auto texture    = static_cast<Vulkan::Texture*>(gfxTexture);

    for(auto& textureTask : aTextureInfo.second)
    {
      auto&       info   = *textureTask.pInfo;
      const auto& source = sourceList[info.srcReference];
      if(source.sourceType == Dali::Graphics::TextureUpdateSourceInfo::Type::MEMORY ||
         source.sourceType == Dali::Graphics::TextureUpdateSourceInfo::Type::PIXEL_DATA)
      {
        uint8_t* sourcePtr    = nullptr;
        Format   sourceFormat = Format::UNDEFINED;
        if(source.sourceType == Graphics::TextureUpdateSourceInfo::Type::MEMORY)
        {
          sourcePtr = reinterpret_cast<uint8_t*>(source.memorySource.memory);
          memoryDiscardQ.push_back(sourcePtr);
        }
        else
        {
          auto pixelBufferData = Dali::Integration::GetPixelDataBuffer(source.pixelDataSource.pixelData);
          sourcePtr            = pixelBufferData.buffer + info.srcOffset;
          sourceFormat         = ConvertPixelFormat(source.pixelDataSource.pixelData.GetPixelFormat());

          if(Dali::Integration::IsPixelDataReleaseAfterUpload(source.pixelDataSource.pixelData) &&
             info.srcOffset == 0u)
          {
            pixelDataDiscardQ.push_back(source.pixelDataSource.pixelData);
          }
        }

        auto sourceInfoPtr = &source;
        auto pInfo         = textureTask.pInfo;

        // If the destination texture supports direct write access, then we can
        // schedule direct copy task and skip the GPU upload. The update
        // should be fully complete.
        auto destTexture = static_cast<Vulkan::Texture*>(info.dstTexture);

        if(destTexture->GetProperties().directWriteAccessEnabled)
        {
          ResourceTransfer* resourceTransfer = this;
          auto              taskLambda       = [resourceTransfer, pInfo, sourcePtr, sourceInfoPtr, texture](auto workerIndex) {
            const auto& properties = texture->GetProperties();

            if(properties.emulated)
            {
              std::vector<char> data;
              auto              memoryRequirements = texture->GetMemoryRequirements();
              data.resize(memoryRequirements.size);
              texture->TryConvertPixelData(sourcePtr, pInfo->srcSize, pInfo->srcExtent2D.width, pInfo->srcExtent2D.height, &data[0]);

              // substitute temporary source
              Graphics::TextureUpdateSourceInfo newSource{};
              newSource.sourceType          = Graphics::TextureUpdateSourceInfo::Type::MEMORY;
              newSource.memorySource.memory = data.data();
              CopyMemoryDirect(*resourceTransfer, *texture, *pInfo, newSource, false);
            }
            else
            {
              CopyMemoryDirect(*resourceTransfer, *texture, *pInfo, *sourceInfoPtr, false);
            }
          };
          textureTask.copyTask = taskLambda;
        }
        else
        {
          const auto size          = destTexture->GetMemoryRequirements().size;
          auto       currentOffset = totalStagingBufferSize;

          totalStagingBufferSize += uint32_t(size);
          auto ppStagingMemory = &stagingBufferMappedPtr; // this pointer will be set later!

          // The staging buffer is not allocated yet. The task knows pointer to the pointer which will point
          // at staging buffer right before executing tasks. The function will either perform direct copy
          // or will do suitable conversion if source format isn't supported and emulation is available.
          auto taskLambda = [ppStagingMemory, currentOffset, pInfo, sourcePtr, sourceFormat, texture](auto workerThread) {
            char* pStagingMemory = reinterpret_cast<char*>(*ppStagingMemory);

            // Try to initialise texture resources explicitly if they are not yet initialised
            texture->InitializeImageView();

            // If texture is 'emulated' convert pixel data otherwise do direct copy
            const auto& properties = texture->GetProperties();

            if(properties.emulated)
            {
              texture->TryConvertPixelData(sourcePtr, pInfo->srcSize, pInfo->srcExtent2D.width, pInfo->srcExtent2D.height, &pStagingMemory[currentOffset]);
            }
            else if(sourceFormat != Format::UNDEFINED && sourceFormat != texture->GetCreateInfo().format)
            {
              // Allow auto-conversion from pixel data format if we support it
              texture->TryConvertPixelData(sourcePtr, sourceFormat, pInfo->srcSize, pInfo->srcExtent2D.width, pInfo->srcExtent2D.height, &pStagingMemory[currentOffset]);
            }
            else
            {
              std::copy(sourcePtr, sourcePtr + pInfo->srcSize, &pStagingMemory[currentOffset]);
            }
          };

          // Add task
          textureTask.copyTask = taskLambda;
          relevantUpdates.emplace_back(&info, currentOffset);
        }
      }
      else
      {
        // for other source types offset within staging buffer doesn't matter
        relevantUpdates.emplace_back(&info, 1u);
      }
    }
  }

  // Prepare one task per each texture to make sure sequential order of updates
  // for the same texture.
  // @todo: this step probably can be avoid in case of using optimal tiling!
  for(auto& item : updateMap)
  {
    auto pUpdates = &item.second;
    auto task     = [pUpdates](auto workerIndex) {
      for(auto& update : *pUpdates)
      {
        update.copyTask(workerIndex);
      }
    };
    copyTasks.emplace_back(task);
  }

  // Allocate staging buffer for all updates using CPU memory
  // as source. The staging buffer exists only for a time of 1 frame.
  auto& threadPool = mThreadPool;

  // Make sure the Initialise() function is not busy with creating first staging buffer
  if(mTextureStagingBufferFuture)
  {
    mTextureStagingBufferFuture->Wait();
    mTextureStagingBufferFuture.reset();
  }

  // Check whether we need staging buffer and if we can reuse existing staging buffer for that frame.
  if(totalStagingBufferSize)
  {
    if(!mTextureStagingBuffer ||
       mTextureStagingBuffer->GetImpl()->GetSize() < totalStagingBufferSize)
    {
      // Initialise new staging buffer. Since caller function is parallelized, initialisation
      // stays on the caller thread.
      InitializeTextureStagingBuffer(totalStagingBufferSize, false);
    }
    MapTextureStagingBuffer();
    stagingBufferMappedPtr = mTextureStagingBufferMappedPtr;
  }

  DALI_ASSERT_DEBUG(mTextureStagingBuffer && "Staging buffer is un-initialized");

  // Submit tasks
  auto futures = threadPool.SubmitTasks(copyTasks, 100u);
  futures->Wait();

  UnmapTextureStagingBuffer();

  for(auto& pair : relevantUpdates)
  {
    auto&       info        = *pair.first;
    const auto& source      = sourceList[info.srcReference];
    auto        destTexture = static_cast<Vulkan::Texture*>(info.dstTexture);

    switch(source.sourceType)
    {
      // directly copy buffer
      case Dali::Graphics::TextureUpdateSourceInfo::Type::BUFFER:
      {
        CopyBuffer(*this,
                   *destTexture,
                   *source.bufferSource.buffer,
                   info.srcOffset,
                   info.srcExtent2D,
                   info.dstOffset2D,
                   info.layer, // layer
                   info.level, // mipmap
                   {});        // update mode, deprecated
        break;
      }
      // for memory, use staging buffer
      case Dali::Graphics::TextureUpdateSourceInfo::Type::PIXEL_DATA:
      case Dali::Graphics::TextureUpdateSourceInfo::Type::MEMORY:
      {
        auto memoryBufferOffset = pair.second;
        CopyBuffer(*this,
                   *destTexture,
                   *mTextureStagingBuffer,
                   memoryBufferOffset,
                   info.srcExtent2D,
                   info.dstOffset2D,
                   info.layer, // layer
                   info.level, // mipmap
                   {});        // update mode, deprecated
        break;
      }

      case Dali::Graphics::TextureUpdateSourceInfo::Type::TEXTURE:
        // Unsupported
        break;
    }
  }

  // Free source data
  for(uint8_t* ptr : memoryDiscardQ)
  {
    free(reinterpret_cast<void*>(ptr));
  }
  for(PixelData pixelData : pixelDataDiscardQ)
  {
    Dali::Integration::ReleasePixelDataBuffer(pixelData);
  }

  // Process transfers
  CreateTransferFutures();
}

/**
 * Mapping the staging buffer may take some time, so can delegate to a worker thread
 * if necessary.
 */
Dali::SharedFuture ResourceTransfer::InitializeTextureStagingBuffer(uint32_t size, bool useWorkerThread)
{
  // Check if we can reuse existing staging buffer for that frame
  if(!mTextureStagingBuffer ||
     mTextureStagingBuffer->GetImpl()->GetSize() < size)
  {
    auto workerFunc = [&, size](auto workerIndex) {
      Graphics::BufferCreateInfo createInfo{};
      createInfo.SetSize(size)
        .SetUsage(0u | Dali::Graphics::BufferUsage::TRANSFER_SRC);
      mTextureStagingBuffer.reset(static_cast<Vulkan::Buffer*>(mGraphicsController.CreateBuffer(createInfo, nullptr).release()));
      MapTextureStagingBuffer();
    };

    if(useWorkerThread)
    {
      return mThreadPool.SubmitTask(0u, workerFunc);
    }
    else
    {
      workerFunc(0);
    }
  }
  else
  {
    MapTextureStagingBuffer();
  }

  return {};
}

void ResourceTransfer::MapTextureStagingBuffer()
{
  // Write into memory in parallel
  if(!mTextureStagingBufferMappedMemory)
  {
    auto          size = mTextureStagingBuffer->GetImpl()->GetSize();
    MapBufferInfo mapInfo{mTextureStagingBuffer.get(), 0 | Graphics::MemoryUsageFlagBits::WRITE, 0, size};
    mTextureStagingBufferMappedMemory = mGraphicsController.MapBufferRange(mapInfo);
    mTextureStagingBufferMappedPtr    = mTextureStagingBufferMappedMemory->LockRegion(0, size);
  }
}

void ResourceTransfer::UnmapTextureStagingBuffer()
{
  // Unmap memory
  mTextureStagingBufferMappedPtr = nullptr;
  mTextureStagingBufferMappedMemory.reset();
}

void ResourceTransfer::CopyBufferAndTransition(
  ResourceTransfer&                  resourceTransfer,
  Texture&                           destTexture,
  const Dali::Graphics::Buffer&      gfxBuffer,
  uint32_t                           bufferOffset,
  Dali::Graphics::Extent2D           extent2D,
  Dali::Graphics::Offset2D           textureOffset2D,
  uint32_t                           layer,
  uint32_t                           level,
  Dali::Graphics::TextureUpdateFlags flags)
{
  if(!destTexture.GetImageView())
  {
    destTexture.InitializeImageView();
  }

  auto& graphicsController = resourceTransfer.GetController();
  auto& device             = graphicsController.GetGraphicsDevice();
  auto& image              = *destTexture.GetImage();

  //@todo Ensure this isn't double buffered.
  Graphics::CommandBufferCreateInfo createInfo{};
  createInfo.SetLevel(Graphics::CommandBufferLevel::PRIMARY);
  auto gfxCommandBuffer = graphicsController.CreateCommandBuffer(createInfo, nullptr);
  auto commandBuffer    = static_cast<Vulkan::CommandBuffer*>(gfxCommandBuffer.get());

  // Fence between submissions
  auto                             fence = FenceImpl::New(device, {});
  Graphics::CommandBufferBeginInfo beginInfo{0 | CommandBufferUsageFlagBits::ONE_TIME_SUBMIT};
  commandBuffer->Begin(beginInfo);

  std::vector<vk::ImageMemoryBarrier> preLayoutBarriers;
  if(layer == 0)
  {
    preLayoutBarriers.emplace_back(image.CreateMemoryBarrier(vk::ImageLayout::eTransferDstOptimal));
  }
  else
  {
    auto subResourceRange = vk::ImageSubresourceRange{}
                              .setBaseMipLevel(0)
                              .setLevelCount(1)
                              .setBaseArrayLayer(layer)
                              .setLayerCount(1)
                              .setAspectMask(image.GetAspectFlags());

    preLayoutBarriers.emplace_back(image.CreateMemoryBarrier(image.GetImageLayout(),
                                                             vk::ImageLayout::eTransferDstOptimal,
                                                             subResourceRange));
  }
  commandBuffer->GetImpl()->PipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                            vk::PipelineStageFlagBits::eTransfer,
                                            {},
                                            {},
                                            {},
                                            preLayoutBarriers);

  auto copyInfo = vk::BufferImageCopy{}
                    .setImageSubresource(vk::ImageSubresourceLayers{}
                                           .setBaseArrayLayer(layer)
                                           .setLayerCount(1)
                                           .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                           .setMipLevel(level))
                    .setImageOffset({textureOffset2D.x, textureOffset2D.y, 0})
                    .setImageExtent({extent2D.width, extent2D.height, 1})
                    .setBufferRowLength(0u)
                    .setBufferOffset(0)
                    .setBufferImageHeight(extent2D.height);

  commandBuffer->GetImpl()->CopyBufferToImage(resourceTransfer.mTextureStagingBuffer->GetImpl(),
                                              &image,
                                              vk::ImageLayout::eTransferDstOptimal,
                                              {copyInfo});

  std::vector<vk::ImageMemoryBarrier> postLayoutBarriers;
  if(layer == 0)
  {
    postLayoutBarriers.emplace_back(image.CreateMemoryBarrier(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal));
  }
  else
  {
    auto subResourceRange = vk::ImageSubresourceRange{}
                              .setBaseMipLevel(0)
                              .setLevelCount(1)
                              .setBaseArrayLayer(layer)
                              .setLayerCount(1)
                              .setAspectMask(image.GetAspectFlags());

    postLayoutBarriers.emplace_back(image.CreateMemoryBarrier(vk::ImageLayout::eTransferDstOptimal,
                                                              vk::ImageLayout::eShaderReadOnlyOptimal,
                                                              subResourceRange));
  }
  commandBuffer->GetImpl()->PipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, postLayoutBarriers);

  commandBuffer->End();
  device.GetTransferQueue(0u).Submit({Vulkan::SubmissionData{{}, {}, {commandBuffer->GetImpl()}, {}}}, fence.get());
  fence->Wait();
  fence->Reset();
}

void ResourceTransfer::CopyBuffer(
  ResourceTransfer&                  resourceTransfer,
  Texture&                           destTexture,
  const Dali::Graphics::Buffer&      gfxBuffer,
  uint32_t                           bufferOffset,
  Dali::Graphics::Extent2D           extent2D,
  Dali::Graphics::Offset2D           textureOffset2D,
  uint32_t                           layer,
  uint32_t                           level,
  Dali::Graphics::TextureUpdateFlags flags)
{
  if(!destTexture.GetImageView())
  {
    destTexture.InitializeImageView();
  }

  ResourceTransferRequest transferRequest(TransferRequestType::BUFFER_TO_IMAGE);

  transferRequest.bufferToImageInfo.copyInfo
    .setImageSubresource(vk::ImageSubresourceLayers{}
                           .setBaseArrayLayer(layer)
                           .setLayerCount(1)
                           .setAspectMask(vk::ImageAspectFlagBits::eColor)
                           .setMipLevel(level))
    .setImageOffset({textureOffset2D.x, textureOffset2D.y, 0})
    .setImageExtent({extent2D.width, extent2D.height, 1})
    .setBufferRowLength(0u)
    .setBufferOffset(bufferOffset)
    .setBufferImageHeight(extent2D.height);

  auto& buffer                                = const_cast<Vulkan::Buffer&>(static_cast<const Vulkan::Buffer&>(gfxBuffer));
  transferRequest.bufferToImageInfo.dstImage  = destTexture.GetImage();
  transferRequest.bufferToImageInfo.srcBuffer = buffer.GetImpl();
  transferRequest.deferredTransferMode        = false;

  // schedule transfer
  resourceTransfer.ScheduleResourceTransfer(std::move(transferRequest));
}

void ResourceTransfer::CopyMemoryDirect(
  ResourceTransfer&                              resourceTransfer,
  Texture&                                       destTexture,
  const Dali::Graphics::TextureUpdateInfo&       info,
  const Dali::Graphics::TextureUpdateSourceInfo& sourceInfo,
  bool                                           keepMapped)
{
  /**
   * Early return if the texture doesn't use linear tiling and
   * the memory isn't host writable.
   */
  if(destTexture.GetTiling() != Dali::Graphics::TextureTiling::LINEAR)
  {
    return;
  }

  // try to initialise resource
  destTexture.InitializeImageView();

  auto& device = resourceTransfer.GetDevice();
  auto  image  = destTexture.GetImage();
  auto  memory = image->GetMemory();
  auto  ptr    = memory->MapTyped<char>();

  /**
   * Get subresource layout to find out the rowPitch size
   */
  auto subresourceLayout = device.GetLogicalDevice().getImageSubresourceLayout(
    image->GetVkHandle(),
    vk::ImageSubresource{}
      .setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setMipLevel(info.level)
      .setArrayLayer(info.layer));

  auto formatInfo   = Vulkan::GetFormatInfo(image->GetFormat());
  int  sizeInBytes  = int(formatInfo.blockSizeInBits / 8);
  auto dstRowLength = subresourceLayout.rowPitch;
  auto dstPtr       = ptr + int(dstRowLength) * info.dstOffset2D.y + sizeInBytes * info.dstOffset2D.x;

  uint8_t* srcPtr = nullptr;
  if(sourceInfo.sourceType == Dali::Graphics::TextureUpdateSourceInfo::Type::MEMORY)
  {
    srcPtr = reinterpret_cast<uint8_t*>(sourceInfo.memorySource.memory);
  }
  else if(sourceInfo.sourceType == Dali::Graphics::TextureUpdateSourceInfo::Type::PIXEL_DATA)
  {
    auto pixelBufferData = Dali::Integration::GetPixelDataBuffer(sourceInfo.pixelDataSource.pixelData);
    srcPtr               = pixelBufferData.buffer + info.srcOffset;
  }

  if(DALI_LIKELY(srcPtr))
  {
    if(formatInfo.compressed)
    {
      std::copy(reinterpret_cast<const char*>(srcPtr), reinterpret_cast<const char*>(srcPtr) + info.srcSize, ptr);
    }
    else
    {
      auto srcRowLength = int(info.srcExtent2D.width) * sizeInBytes;

      /**
       * Copy content line by line
       */
      for(auto i = 0u; i < info.srcExtent2D.height; ++i)
      {
        std::copy(srcPtr, srcPtr + int(info.srcExtent2D.width) * sizeInBytes, dstPtr);
        dstPtr += dstRowLength;
        srcPtr += srcRowLength;
      }
    }
  }

  if(!keepMapped)
  {
    // Unmap
    memory->Unmap();

    // ...and flush
    memory->Flush();
  }

  ResourceTransferRequest transferRequest(TransferRequestType::LAYOUT_TRANSITION_ONLY);
  transferRequest.imageLayoutTransitionInfo.image     = image;
  transferRequest.imageLayoutTransitionInfo.srcLayout = image->GetImageLayout();
  transferRequest.imageLayoutTransitionInfo.dstLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
  transferRequest.deferredTransferMode                = false;

  // schedule transfer
  resourceTransfer.ScheduleResourceTransfer(std::move(transferRequest));
}

void ResourceTransfer::ScheduleResourceTransfer(Vulkan::ResourceTransferRequest&& transferRequest)
{
  std::lock_guard<std::recursive_mutex> lock(mResourceTransferMutex);
  mResourceTransferRequests.emplace_back(std::move(transferRequest));

  // if we requested immediate upload then request will be processed instantly with skipping
  // all the deferred update requests
  if(!mResourceTransferRequests.back().deferredTransferMode)
  {
    ProcessResourceTransferRequests(true);
  }
}

void ResourceTransfer::ProcessResourceTransferRequests(bool immediateOnly)
{
  std::lock_guard<std::recursive_mutex> lock(mResourceTransferMutex);
  if(!mResourceTransferRequests.empty())
  {
    using ResourceTransferRequestList = std::vector<const ResourceTransferRequest*>;

    /**
     * Structure associating unique images and lists of transfer requests for which
     * the key image is a destination. It contains separate lists of requests per image.
     * Each list of requests groups non-intersecting copy operations into smaller batches.
     */
    struct ResourceTransferRequestPair
    {
      ResourceTransferRequestPair(Vulkan::Image& key)
      : image(key),
        requestList{{}}
      {
      }

      Vulkan::Image&                           image;
      std::vector<ResourceTransferRequestList> requestList;
    };

    // Map of all the requests where 'image' is a key.
    std::vector<ResourceTransferRequestPair> requestMap;

    auto highestBatchIndex = 1u;

    // Collect all unique destination images and all transfer requests associated with them
    for(const auto& req : mResourceTransferRequests)
    {
      Vulkan::Image* image{nullptr};
      if(req.requestType == TransferRequestType::BUFFER_TO_IMAGE)
      {
        image = req.bufferToImageInfo.dstImage;
      }
      else if(req.requestType == TransferRequestType::IMAGE_TO_IMAGE)
      {
        image = req.imageToImageInfo.dstImage;
      }
      else if(req.requestType == TransferRequestType::USE_TBM_SURFACE)
      {
        image = req.useTBMSurfaceInfo.srcImage;
      }
      else if(req.requestType == TransferRequestType::LAYOUT_TRANSITION_ONLY)
      {
        image = req.imageLayoutTransitionInfo.image;
      }
      assert(image);

      auto predicate = [&](auto& item) -> bool {
        return image->GetVkHandle() == item.image.GetVkHandle();
      };
      auto it = std::find_if(requestMap.begin(), requestMap.end(), predicate);

      if(it == requestMap.end())
      {
        // initialise new array
        requestMap.emplace_back(*image);
        it = requestMap.end() - 1;
      }

      auto& transfers = it->requestList;

      // Compare with current transfer list whether there are any intersections
      // with current image copy area. If intersection occurs, start new list
      auto& currentList = transfers.back();

      bool intersects(false);
      for(auto& item : currentList)
      {
        // if area intersects create new list
        if((intersects = TestCopyRectIntersection(item, &req)))
        {
          transfers.push_back({});
          highestBatchIndex = std::max(highestBatchIndex, uint32_t(transfers.size()));
          break;
        }
      }

      // push request to the most recently created list
      transfers.back().push_back(&req);
    }

    // For all unique images prepare layout transition barriers as all of them must be
    // in eTransferDstOptimal layout
    std::vector<vk::ImageMemoryBarrier> preLayoutBarriers;
    std::vector<vk::ImageMemoryBarrier> postLayoutBarriers;
    for(auto& item : requestMap)
    {
      auto& image = item.image;
      // add barrier
      uint32_t layer{0u};
      auto&    req = item.requestList.front().front();
      if(req->requestType == TransferRequestType::BUFFER_TO_IMAGE)
      {
        layer = req->bufferToImageInfo.copyInfo.imageSubresource.baseArrayLayer;
      }
      if(layer == 0u)
      {
        preLayoutBarriers.push_back(image.CreateMemoryBarrier(vk::ImageLayout::eTransferDstOptimal));
        postLayoutBarriers.push_back(image.CreateMemoryBarrier(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal));
      }
      else
      {
        auto subResourceRange = vk::ImageSubresourceRange{}
                                  .setBaseMipLevel(0)
                                  .setLevelCount(1)
                                  .setBaseArrayLayer(layer)
                                  .setLayerCount(1)
                                  .setAspectMask(image.GetAspectFlags());
        preLayoutBarriers.push_back(image.CreateMemoryBarrier(image.GetImageLayout(), vk::ImageLayout::eTransferDstOptimal, subResourceRange));
        postLayoutBarriers.push_back(image.CreateMemoryBarrier(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, subResourceRange));
      }
    }

    // Build command buffer for each image until reaching next sync point
    auto& graphicsController = GetController();

    Graphics::CommandBufferCreateInfo createInfo{};
    createInfo.SetLevel(Graphics::CommandBufferLevel::PRIMARY);

    //@todo Ensure this isn't double buffered.
    auto gfxCommandBuffer = graphicsController.CreateCommandBuffer(createInfo, nullptr);
    auto commandBuffer    = static_cast<Vulkan::CommandBuffer*>(gfxCommandBuffer.get());

    // Fence between submissions
    auto& device = graphicsController.GetGraphicsDevice();
    auto  fence  = FenceImpl::New(device, {});

    /**
     * The loop iterates through requests for each unique image. It parallelizes
     * transfers to images until end of data in the batch.
     * After submitting copy commands the loop waits for the fence to be signalled
     * and repeats recording for the next batch of transfer requests.
     */
    for(auto i = 0u; i < highestBatchIndex; ++i)
    {
      Graphics::CommandBufferBeginInfo beginInfo{0 | CommandBufferUsageFlagBits::ONE_TIME_SUBMIT};
      commandBuffer->Begin(beginInfo);

      // change image layouts only once
      if(i == 0)
      {
        commandBuffer->GetImpl()->PipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, preLayoutBarriers);
      }

      for(auto& item : requestMap)
      {
        auto& batchItem = item.requestList;
        if(batchItem.size() <= i)
        {
          continue;
        }

        auto& requestList = batchItem[i];

        // record all copy commands for this batch
        for(auto& req : requestList)
        {
          if(req->requestType == TransferRequestType::BUFFER_TO_IMAGE)
          {
            commandBuffer->GetImpl()->CopyBufferToImage(req->bufferToImageInfo.srcBuffer,
                                                        req->bufferToImageInfo.dstImage,
                                                        vk::ImageLayout::eTransferDstOptimal,
                                                        {req->bufferToImageInfo.copyInfo});
          }
          else if(req->requestType == TransferRequestType::IMAGE_TO_IMAGE)
          {
            commandBuffer->GetImpl()->CopyImage(req->imageToImageInfo.srcImage,
                                                vk::ImageLayout::eTransferSrcOptimal,
                                                req->imageToImageInfo.dstImage,
                                                vk::ImageLayout::eTransferDstOptimal,
                                                {req->imageToImageInfo.copyInfo});
          }
        }
      }

      // if this is the last batch restore original layouts
      if(i == highestBatchIndex - 1)
      {
        commandBuffer->GetImpl()->PipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, postLayoutBarriers);
      }
      commandBuffer->End();

      // submit to the queue
      device.GetTransferQueue(0u).Submit({Vulkan::SubmissionData{{}, {}, {commandBuffer->GetImpl()}, {}}}, fence.get());
      fence->Wait();
      fence->Reset();
    }

    // Destroy staging resources immediately
    for(auto& request : mResourceTransferRequests)
    {
      if(request.requestType == TransferRequestType::BUFFER_TO_IMAGE)
      {
        auto& buffer = request.bufferToImageInfo.srcBuffer;
        // Do not destroy
        if(buffer != mTextureStagingBuffer->GetImpl())
        {
          buffer->Destroy();
        }
      }
      else if(request.requestType == TransferRequestType::IMAGE_TO_IMAGE)
      {
        auto& image = request.imageToImageInfo.srcImage;
        if(image->GetVkHandle())
        {
          image->Destroy();
        }
      }
    }

    // Clear transfer queue
    mResourceTransferRequests.clear();
  }
}

void ResourceTransfer::CreateTransferFutures()
{
  mTransferFutures.emplace_back(mThreadPool.SubmitTask(0, Task([this](uint32_t workerIndex) {
    // execute all scheduled resource transfers
    ProcessResourceTransferRequests(); })));
}

void ResourceTransfer::WaitOnResourceTransferFutures()
{
  for(auto& future : mTransferFutures)
  {
    future->Wait();
    future.reset();
  }
  mTransferFutures.clear();
}

} // namespace Dali::Graphics::Vulkan
