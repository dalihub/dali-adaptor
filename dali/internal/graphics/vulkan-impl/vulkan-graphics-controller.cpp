/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>

// INTERNAL INCLUDES
#include <dali/integration-api/pixel-data-integ.h>

#include <dali/internal/graphics/vulkan/vulkan-device.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-fence-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-memory.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-pipeline.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-program.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-target.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-sampler.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-shader.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-texture.h>
#include <dali/internal/window-system/common/window-render-surface.h>

#include <queue>
#include <unordered_map>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{
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

/**
 * @brief Custom deleter for all Graphics objects created
 * with use of the Controller.
 *
 * When Graphics object dies the unique pointer (Graphics::UniquePtr)
 * doesn't destroy it directly but passes the ownership back
 * to the Controller. The GraphicsDeleter is responsible for passing
 * the graphics object to the discard queue (by calling Resource::DiscardResource()).
 */
template<typename T>
struct GraphicsDeleter
{
  GraphicsDeleter() = default;

  void operator()(T* object)
  {
    // Discard resource (add it to discard queue)
    object->DiscardResource();
  }
};

/**
 * @brief Helper function allocating graphics object
 *
 * @param[in] info Create info structure
 * @param[in] controller Controller object
 * @param[out] out Unique pointer to the return object
 */
template<class VKType, class GfxCreateInfo, class T>
auto NewGraphicsObject(const GfxCreateInfo& info, VulkanGraphicsController& controller, T&& oldObject)
{
  // Use allocator
  using Type = typename T::element_type;
  using UPtr = Dali::Graphics::UniquePtr<Type>;
  if(info.allocationCallbacks)
  {
    auto* memory = info.allocationCallbacks->allocCallback(
      sizeof(VKType),
      0,
      info.allocationCallbacks->userData);
    return UPtr(new(memory) VKType(info, controller), GraphicsDeleter<VKType>());
  }
  else // Use standard allocator
  {
    // We are given all object for recycling
    if(oldObject)
    {
      auto reusedObject = oldObject.release();
      // If succeeded, attach the object to the unique_ptr and return it back
      if(static_cast<VKType*>(reusedObject)->TryRecycle(info, controller))
      {
        return UPtr(reusedObject, GraphicsDeleter<VKType>());
      }
      else
      {
        // can't reuse so kill object by giving it back to original
        // unique pointer.
        oldObject.reset(reusedObject);
      }
    }

    // Create brand-new object
    UPtr gfxObject(new VKType(info, controller), GraphicsDeleter<VKType>());
    static_cast<VKType*>(gfxObject.get())->InitializeResource(); // @todo Consider using create queues?
    return gfxObject;
  }
}

template<class T0, class T1>
T0* CastObject(T1* apiObject)
{
  return static_cast<T0*>(apiObject);
}

namespace DepthStencilFlagBits
{
static constexpr uint32_t DEPTH_BUFFER_BIT   = 1; // depth buffer enabled
static constexpr uint32_t STENCIL_BUFFER_BIT = 2; // stencil buffer enabled
} // namespace DepthStencilFlagBits

// State of the depth-stencil buffer
using DepthStencilFlags = uint32_t;

struct VulkanGraphicsController::Impl
{
  explicit Impl(VulkanGraphicsController& controller)
  : mGraphicsController(controller)
  {
  }

  bool Initialize(Vulkan::Device& device)
  {
    mGraphicsDevice = &device;

    // @todo Create pipeline cache & descriptor set allocator here

    mThreadPool.Initialize();
    return true;
  }

  void AcquireNextFramebuffer()
  {
    // @todo for all swapchains acquire new framebuffer
    auto surface   = mGraphicsDevice->GetSurface(0u);
    auto swapchain = mGraphicsDevice->GetSwapchainForSurfaceId(0u);

    if(mGraphicsDevice->IsSurfaceResized())
    {
      swapchain->Invalidate();
    }

    swapchain->AcquireNextFramebuffer(true);

    if(!swapchain->IsValid())
    {
      // make sure device doesn't do any work before replacing swapchain
      mGraphicsDevice->DeviceWaitIdle();

      // replace swapchain
      swapchain = mGraphicsDevice->ReplaceSwapchainForSurface(surface, std::move(swapchain));

      // get new valid framebuffer
      swapchain->AcquireNextFramebuffer(true);
    }
  }

  bool EnableDepthStencilBuffer(const RenderTarget& renderTarget, bool enableDepth, bool enableStencil)
  {
    auto surface = static_cast<const Vulkan::RenderTarget*>(&renderTarget)->GetSurface();
    if(!surface)
    {
      // Do nothing if this is not a surface.
      return false;
    }

    auto renderSurface = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface);
    auto surfaceId     = renderSurface->GetSurfaceId();

    mDepthStencilBufferRequestedState = (enableDepth ? DepthStencilFlagBits::DEPTH_BUFFER_BIT : 0u) |
                                        (enableStencil ? DepthStencilFlagBits::STENCIL_BUFFER_BIT : 0u);

    auto retval = mDepthStencilBufferRequestedState != mDepthStencilBufferCurrentState;

    // @todo move state vars to surface
    if(surface && mDepthStencilBufferCurrentState != mDepthStencilBufferRequestedState)
    {
      DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "UpdateDepthStencilBuffer(): New state: DEPTH: %d, STENCIL: %d\n", int(mDepthStencilBufferRequestedState & 1), int((mDepthStencilBufferRequestedState >> 1) & 1));

      // Formats
      const std::array<vk::Format, 4> DEPTH_STENCIL_FORMATS = {
        vk::Format::eUndefined,     // no depth nor stencil needed
        vk::Format::eD16Unorm,      // only depth buffer
        vk::Format::eS8Uint,        // only stencil buffer
        vk::Format::eD24UnormS8Uint // depth and stencil buffers
      };

      mGraphicsDevice->DeviceWaitIdle();

      mGraphicsDevice->GetSwapchainForSurfaceId(surfaceId)->SetDepthStencil(DEPTH_STENCIL_FORMATS[mDepthStencilBufferRequestedState]);

      // make sure GPU finished any pending work
      mGraphicsDevice->DeviceWaitIdle();

      mDepthStencilBufferCurrentState = mDepthStencilBufferRequestedState;
    }
    return retval;
  }

  /**
   * Mapping the staging buffer may take some time, so can delegate to a worker thread
   * if necessary.
   */
  Dali::SharedFuture InitializeTextureStagingBuffer(uint32_t size, bool useWorkerThread)
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
    return {};
  }

  void MapTextureStagingBuffer()
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

  void UnmapTextureStagingBuffer()
  {
    // Unmap memory
    mTextureStagingBufferMappedPtr = nullptr;
    mTextureStagingBufferMappedMemory.reset();
  }

  void ProcessResourceTransferRequests(bool immediateOnly = false)
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
        preLayoutBarriers.push_back(image.CreateMemoryBarrier(vk::ImageLayout::eTransferDstOptimal));
        postLayoutBarriers.push_back(image.CreateMemoryBarrier(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal));
        image.SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
      }

      // Build command buffer for each image until reaching next sync point
      Graphics::CommandBufferCreateInfo createInfo{};
      createInfo.SetLevel(Graphics::CommandBufferLevel::PRIMARY);
      auto gfxCommandBuffer = mGraphicsController.CreateCommandBuffer(createInfo, nullptr);
      auto commandBuffer    = static_cast<Vulkan::CommandBuffer*>(gfxCommandBuffer.get());

      // Fence between submissions
      auto fence = FenceImpl::New(*mGraphicsDevice, {});

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
        mGraphicsDevice->Submit(mGraphicsDevice->GetTransferQueue(0u), {Vulkan::SubmissionData{{}, {}, {commandBuffer->GetImpl()}, {}}}, fence.get());
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

  /**
   * @brief Processes a discard queue for objects created with NewObject
   *
   * @param[in,out] queue Reference to the queue
   */
  template<class ResourceType>
  void ProcessResourceDiscardQueue(std::queue<ResourceType*>& queue)
  {
    while(!queue.empty())
    {
      auto* object = const_cast<ResourceType*>(queue.front());

      // Destroy
      object->DestroyResource();

      auto* allocationCallbacks = object->GetAllocationCallbacks();
      if(allocationCallbacks)
      {
        object->InvokeDeleter();
        allocationCallbacks->freeCallback(object, allocationCallbacks->userData);
      }
      else
      {
        delete object;
      }
      queue.pop();
    }
  }

  /**
   * Processes a discard queue for direct instantiated objects
   */
  template<class ResourceType>
  void ProcessDiscardQueue(std::queue<ResourceType*>& queue)
  {
    while(!queue.empty())
    {
      auto* object = queue.front();

      // Destroy
      object->DestroyResource();
      delete object;

      queue.pop();
    }
  }

  void GarbageCollect()
  {
    ProcessResourceDiscardQueue<ResourceWithDeleter>(mResourceDiscardQueue);
    ProcessDiscardQueue<ResourceBase>(mDiscardQueue);
  }

  void Flush()
  {
    // Flush any outstanding queues.

    GarbageCollect();
  }

  VulkanGraphicsController& mGraphicsController;
  Vulkan::Device*           mGraphicsDevice{nullptr};

  // used for texture<->buffer<->memory transfers
  std::vector<ResourceTransferRequest>     mResourceTransferRequests;
  std::recursive_mutex                     mResourceTransferMutex{};
  std::queue<Vulkan::ResourceBase*>        mDiscardQueue;
  std::queue<Vulkan::ResourceWithDeleter*> mResourceDiscardQueue;

  std::unique_ptr<Vulkan::Buffer>       mTextureStagingBuffer{};
  Dali::SharedFuture                    mTextureStagingBufferFuture{};
  Graphics::UniquePtr<Graphics::Memory> mTextureStagingBufferMappedMemory{nullptr};
  void*                                 mTextureStagingBufferMappedPtr{nullptr};

  ThreadPool mThreadPool;

  DepthStencilFlags mDepthStencilBufferCurrentState{0u};
  DepthStencilFlags mDepthStencilBufferRequestedState{0u};

  std::unordered_map<uint32_t, Graphics::UniquePtr<Graphics::Texture>> mExternalTextureResources;        ///< Used for ResourceId.
  std::queue<const Vulkan::Texture*>                                   mTextureMipmapGenerationRequests; ///< Queue for texture mipmap generation requests

  std::size_t mCapacity{0u}; ///< Memory Usage (of command buffers)
};

VulkanGraphicsController::VulkanGraphicsController()
: mImpl(std::make_unique<Impl>(*this))
{
}

VulkanGraphicsController::~VulkanGraphicsController()
{
  mImpl->GarbageCollect();
}

void VulkanGraphicsController::Initialize(Dali::Graphics::VulkanGraphics& graphicsImplementation,
                                          Vulkan::Device&                 graphicsDevice)
{
  mImpl->Initialize(graphicsDevice);
}

Integration::GraphicsConfig& VulkanGraphicsController::GetGraphicsConfig()
{
  return *this;
}

void VulkanGraphicsController::FrameStart()
{
  mImpl->mCapacity = 0;
  mImpl->AcquireNextFramebuffer();
}

void VulkanGraphicsController::SetResourceBindingHints(const std::vector<SceneResourceBinding>& resourceBindings)
{
  // Check if there is some extra information about used resources
  // if so then apply optimizations

  // update programs with descriptor pools
  for(auto& binding : resourceBindings)
  {
    if(binding.type == ResourceType::PROGRAM)
    {
      auto programImpl = static_cast<Vulkan::Program*>(binding.programBinding->program)->GetImplementation();

      // Pool index is returned and we may do something with it later (storing it per cmdbuf?)
      [[maybe_unused]] auto poolIndex = programImpl->AddDescriptorPool(binding.programBinding->count, 3); // add new pool, limit pools to 3 per program
    }
  }
}

void VulkanGraphicsController::SubmitCommandBuffers(const SubmitInfo& submitInfo)
{
  // Figure out where to submit each command buffer.
  for(auto gfxCmdBuffer : submitInfo.cmdBuffer)
  {
    auto cmdBuffer = static_cast<const CommandBuffer*>(gfxCmdBuffer);
    auto swapchain = cmdBuffer->GetLastSwapchain();
    if(swapchain)
    {
      swapchain->Submit(cmdBuffer->GetImpl());
    }
  }

  // If flush bit set, flush all pending tasks
  if(submitInfo.flags & (0 | SubmitFlagBits::FLUSH))
  {
    Flush();
  }
}

void VulkanGraphicsController::PresentRenderTarget(Graphics::RenderTarget* renderTarget)
{
  auto surface   = static_cast<Vulkan::RenderTarget*>(renderTarget)->GetSurface();
  auto surfaceId = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface)->GetSurfaceId();
  auto swapchain = mImpl->mGraphicsDevice->GetSwapchainForSurfaceId(surfaceId);

  swapchain->Present();
}

void VulkanGraphicsController::WaitIdle()
{
}

void VulkanGraphicsController::Pause()
{
}

void VulkanGraphicsController::Resume()
{
  // Draw on resume - update manager could query.
  // could use this to trigger debug output for a few frames (EGL dumps each swap-buffer)
}

void VulkanGraphicsController::Shutdown()
{
}

void VulkanGraphicsController::Destroy()
{
}

void VulkanGraphicsController::UpdateTextures(
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
        uint8_t* sourcePtr = nullptr;
        if(source.sourceType == Graphics::TextureUpdateSourceInfo::Type::MEMORY)
        {
          sourcePtr = reinterpret_cast<uint8_t*>(source.memorySource.memory);
          memoryDiscardQ.push_back(sourcePtr);
        }
        else
        {
          auto pixelBufferData = Dali::Integration::GetPixelDataBuffer(source.pixelDataSource.pixelData);

          sourcePtr = pixelBufferData.buffer + info.srcOffset;
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
          auto taskLambda = [pInfo, sourcePtr, sourceInfoPtr, texture](auto workerIndex) {
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
              texture->CopyMemoryDirect(*pInfo, newSource, false);
            }
            else
            {
              texture->CopyMemoryDirect(*pInfo, *sourceInfoPtr, false);
            }
          };
          textureTask.copyTask = taskLambda;
        }
        else
        {
          const auto size          = destTexture->GetMemoryRequirements().size;
          auto       currentOffset = totalStagingBufferSize;

          relevantUpdates.emplace_back(&info, currentOffset);
          totalStagingBufferSize += uint32_t(size);
          auto ppStagingMemory = &stagingBufferMappedPtr; // this pointer will be set later!

          // The staging buffer is not allocated yet. The task knows pointer to the pointer which will point
          // at staging buffer right before executing tasks. The function will either perform direct copy
          // or will do suitable conversion if source format isn't supported and emulation is available.
          auto taskLambda = [ppStagingMemory, currentOffset, pInfo, sourcePtr, texture](auto workerThread) {
            char* pStagingMemory = reinterpret_cast<char*>(*ppStagingMemory);

            // Try to initialise` texture resources explicitly if they are not yet initialised
            texture->InitializeImageView();

            // If texture is 'emulated' convert pixel data otherwise do direct copy
            const auto& properties = texture->GetProperties();

            if(properties.emulated)
            {
              texture->TryConvertPixelData(sourcePtr, pInfo->srcSize, pInfo->srcExtent2D.width, pInfo->srcExtent2D.height, &pStagingMemory[currentOffset]);
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
  auto& threadPool = mImpl->mThreadPool;

  // Make sure the Initialise() function is not busy with creating first staging buffer
  if(mImpl->mTextureStagingBufferFuture)
  {
    mImpl->mTextureStagingBufferFuture->Wait();
    mImpl->mTextureStagingBufferFuture.reset();
  }

  // Check whether we need staging buffer and if we can reuse existing staging buffer for that frame.
  if(totalStagingBufferSize)
  {
    if(!mImpl->mTextureStagingBuffer ||
       mImpl->mTextureStagingBuffer->GetImpl()->GetSize() < totalStagingBufferSize)
    {
      // Initialise new staging buffer. Since caller function is parallelized, initialisation
      // stays on the caller thread.
      mImpl->InitializeTextureStagingBuffer(totalStagingBufferSize, false);
    }
    mImpl->MapTextureStagingBuffer();
  }

  // Submit tasks
  auto futures = threadPool.SubmitTasks(copyTasks, 100u);
  futures->Wait();

  mImpl->UnmapTextureStagingBuffer();

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
        destTexture->CopyBuffer(*source.bufferSource.buffer,
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
      {
      }
      case Dali::Graphics::TextureUpdateSourceInfo::Type::MEMORY:
      {
        auto memoryBufferOffset = pair.second;
        destTexture->CopyBuffer(*mImpl->mTextureStagingBuffer,
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
}

void VulkanGraphicsController::ScheduleResourceTransfer(Vulkan::ResourceTransferRequest&& transferRequest)
{
  std::lock_guard<std::recursive_mutex> lock(mImpl->mResourceTransferMutex);
  mImpl->mResourceTransferRequests.emplace_back(std::move(transferRequest));

  // if we requested immediate upload then request will be processed instantly with skipping
  // all the deferred update requests
  if(!mImpl->mResourceTransferRequests.back().deferredTransferMode)
  {
    mImpl->ProcessResourceTransferRequests(true);
  }
}

void VulkanGraphicsController::GenerateTextureMipmaps(const Graphics::Texture& texture)
{
}

bool VulkanGraphicsController::EnableDepthStencilBuffer(
  const Graphics::RenderTarget& gfxRenderTarget,
  bool                          enableDepth,
  bool                          enableStencil)
{
  // if we enable depth/stencil dynamically we need to block and invalidate pipeline cache
  auto renderTarget = static_cast<const Vulkan::RenderTarget*>(&gfxRenderTarget);
  return mImpl->EnableDepthStencilBuffer(*renderTarget, enableDepth, enableStencil);
}

void VulkanGraphicsController::RunGarbageCollector(size_t numberOfDiscardedRenderers)
{
  mImpl->GarbageCollect();
}

void VulkanGraphicsController::DiscardUnusedResources()
{
}

bool VulkanGraphicsController::IsDiscardQueueEmpty()
{
  return true;
}

bool VulkanGraphicsController::IsDrawOnResumeRequired()
{
  return true;
}

UniquePtr<Graphics::RenderTarget> VulkanGraphicsController::CreateRenderTarget(const Graphics::RenderTargetCreateInfo& renderTargetCreateInfo, UniquePtr<Graphics::RenderTarget>&& oldRenderTarget)
{
  return NewGraphicsObject<Vulkan::RenderTarget>(renderTargetCreateInfo, *this, std::move(oldRenderTarget));
}

UniquePtr<Graphics::CommandBuffer> VulkanGraphicsController::CreateCommandBuffer(const Graphics::CommandBufferCreateInfo& commandBufferCreateInfo, UniquePtr<Graphics::CommandBuffer>&& oldCommandBuffer)
{
  return NewGraphicsObject<Vulkan::CommandBuffer>(commandBufferCreateInfo, *this, std::move(oldCommandBuffer));
}

UniquePtr<Graphics::RenderPass> VulkanGraphicsController::CreateRenderPass(const Graphics::RenderPassCreateInfo& renderPassCreateInfo, UniquePtr<Graphics::RenderPass>&& oldRenderPass)
{
  auto renderPass = UniquePtr<Graphics::RenderPass>(new Vulkan::RenderPass(renderPassCreateInfo, *this));

  // Don't create actual vulkan resource here. It will instead be done on demand. (e.g. framebuffer creation, CommandBuffer::BeginRenderPass())
  return renderPass;
}

UniquePtr<Graphics::Buffer> VulkanGraphicsController::CreateBuffer(const Graphics::BufferCreateInfo& bufferCreateInfo, UniquePtr<Graphics::Buffer>&& oldBuffer)
{
  return NewGraphicsObject<Vulkan::Buffer>(bufferCreateInfo, *this, std::move(oldBuffer));
}

UniquePtr<Graphics::Texture> VulkanGraphicsController::CreateTexture(const Graphics::TextureCreateInfo& textureCreateInfo, UniquePtr<Graphics::Texture>&& oldTexture)
{
  return NewGraphicsObject<Vulkan::Texture>(textureCreateInfo, *this, std::move(oldTexture));
}

UniquePtr<Graphics::Framebuffer> VulkanGraphicsController::CreateFramebuffer(const Graphics::FramebufferCreateInfo& framebufferCreateInfo, UniquePtr<Graphics::Framebuffer>&& oldFramebuffer)
{
  return UniquePtr<Graphics::Framebuffer>{};
}

UniquePtr<Graphics::Pipeline> VulkanGraphicsController::CreatePipeline(const Graphics::PipelineCreateInfo& pipelineCreateInfo, UniquePtr<Graphics::Pipeline>&& oldPipeline)
{
  return UniquePtr<Graphics::Pipeline>(new Vulkan::Pipeline(pipelineCreateInfo, *this, nullptr));
}

UniquePtr<Graphics::Program> VulkanGraphicsController::CreateProgram(const Graphics::ProgramCreateInfo& programCreateInfo, UniquePtr<Graphics::Program>&& oldProgram)
{
  return UniquePtr<Graphics::Program>(new Vulkan::Program(programCreateInfo, *this));
}

UniquePtr<Graphics::Shader> VulkanGraphicsController::CreateShader(const Graphics::ShaderCreateInfo& shaderCreateInfo, UniquePtr<Graphics::Shader>&& oldShader)
{
  return UniquePtr<Graphics::Shader>(new Vulkan::Shader(shaderCreateInfo, *this));
}

UniquePtr<Graphics::Sampler> VulkanGraphicsController::CreateSampler(const Graphics::SamplerCreateInfo& samplerCreateInfo, UniquePtr<Graphics::Sampler>&& oldSampler)
{
  return NewGraphicsObject<Vulkan::Sampler>(samplerCreateInfo, *this, std::move(oldSampler));
}

UniquePtr<Graphics::SyncObject> VulkanGraphicsController::CreateSyncObject(const Graphics::SyncObjectCreateInfo& syncObjectCreateInfo,
                                                                           UniquePtr<Graphics::SyncObject>&&     oldSyncObject)
{
  return UniquePtr<Graphics::SyncObject>{};
}

void VulkanGraphicsController::DiscardResource(Vulkan::ResourceBase* resource)
{
  mImpl->mDiscardQueue.push(resource);
}

void VulkanGraphicsController::DiscardResource(Vulkan::ResourceWithDeleter* resource)
{
  mImpl->mResourceDiscardQueue.push(resource);
}

UniquePtr<Graphics::Memory> VulkanGraphicsController::MapBufferRange(const MapBufferInfo& mapInfo)
{
  // @todo: Process create queues
  auto        buffer     = static_cast<Vulkan::Buffer*>(mapInfo.buffer);
  BufferImpl* bufferImpl = buffer->GetImpl();
  DALI_ASSERT_DEBUG(bufferImpl && "Mapping CPU allocated buffer is not used in Vulkan");
  if(bufferImpl)
  {
    auto memoryImpl = bufferImpl->GetMemory();
    auto memory     = UniquePtr<Memory>(new Memory(mapInfo, *this));
    memory->Initialize(memoryImpl);
    return memory;
  }
  return nullptr;
}

UniquePtr<Graphics::Memory> VulkanGraphicsController::MapTextureRange(const MapTextureInfo& mapInfo)
{
  // Not implemented (@todo Remove from Graphics API?
  return nullptr;
}

void VulkanGraphicsController::UnmapMemory(UniquePtr<Graphics::Memory> memory)
{
  // Do nothing. Let unique ptr die, and deal with it in the destructor.
}

MemoryRequirements VulkanGraphicsController::GetBufferMemoryRequirements(Graphics::Buffer& gfxBuffer) const
{
  auto        buffer     = static_cast<Vulkan::Buffer*>(&gfxBuffer);
  BufferImpl* bufferImpl = buffer->GetImpl();
  return bufferImpl->GetMemoryRequirements();
}

MemoryRequirements VulkanGraphicsController::GetTextureMemoryRequirements(Graphics::Texture& gfxTexture) const
{
  const Vulkan::Texture* texture = static_cast<const Vulkan::Texture*>(&gfxTexture);
  return texture->GetMemoryRequirements();
}

TextureProperties VulkanGraphicsController::GetTextureProperties(const Graphics::Texture& gfxTexture)
{
  auto* texture = const_cast<Vulkan::Texture*>(static_cast<const Vulkan::Texture*>(&gfxTexture));
  return texture->GetProperties();
}

const Graphics::Reflection& VulkanGraphicsController::GetProgramReflection(const Graphics::Program& program)
{
  return (static_cast<const Vulkan::Program*>(&program))->GetReflection();
}

bool VulkanGraphicsController::PipelineEquals(const Graphics::Pipeline& pipeline0, const Graphics::Pipeline& pipeline1) const
{
  return true;
}

bool VulkanGraphicsController::GetProgramParameter(Graphics::Program& program, uint32_t parameterId, void* outData)
{
  return false;
}

uint32_t VulkanGraphicsController::GetDeviceLimitation(Graphics::DeviceCapability capability)
{
  if(capability == DeviceCapability::MIN_UNIFORM_BUFFER_OFFSET_ALIGNMENT)
  {
    const auto& properties = mImpl->mGraphicsDevice->GetPhysicalDeviceProperties();
    return properties.limits.minUniformBufferOffsetAlignment;
  }
  return 0;
}

bool VulkanGraphicsController::IsBlendEquationSupported(DevelBlendEquation::Type blendEquation)
{
  switch(blendEquation)
  {
    case DevelBlendEquation::ADD:
    case DevelBlendEquation::SUBTRACT:
    case DevelBlendEquation::REVERSE_SUBTRACT:
    case DevelBlendEquation::MIN:
    case DevelBlendEquation::MAX:
    {
      return true;
    }
    case DevelBlendEquation::MULTIPLY:
    case DevelBlendEquation::SCREEN:
    case DevelBlendEquation::OVERLAY:
    case DevelBlendEquation::DARKEN:
    case DevelBlendEquation::LIGHTEN:
    case DevelBlendEquation::COLOR_DODGE:
    case DevelBlendEquation::COLOR_BURN:
    case DevelBlendEquation::HARD_LIGHT:
    case DevelBlendEquation::SOFT_LIGHT:
    case DevelBlendEquation::DIFFERENCE:
    case DevelBlendEquation::EXCLUSION:
    case DevelBlendEquation::HUE:
    case DevelBlendEquation::SATURATION:
    case DevelBlendEquation::COLOR:
    case DevelBlendEquation::LUMINOSITY:
    {
      return IsAdvancedBlendEquationSupported();
    }
    default:
    {
      return false;
    }
  }
  return false;
}

bool VulkanGraphicsController::IsAdvancedBlendEquationSupported()
{
  //@todo Implement this!
  return false;
}

uint32_t VulkanGraphicsController::GetShaderLanguageVersion()
{
  return 4;
}

std::string VulkanGraphicsController::GetShaderVersionPrefix()
{
  return "";
}

std::string VulkanGraphicsController::GetVertexShaderPrefix()
{
  return "";
}

std::string VulkanGraphicsController::GetFragmentShaderPrefix()
{
  return "";
}

Vulkan::Device& VulkanGraphicsController::GetGraphicsDevice()
{
  return *mImpl->mGraphicsDevice;
}

Graphics::Texture* VulkanGraphicsController::CreateTextureByResourceId(uint32_t resourceId, const Graphics::TextureCreateInfo& createInfo)
{
  Graphics::UniquePtr<Graphics::Texture> texture;

  // Check that this resource id hasn't been used previously
  auto iter = mImpl->mExternalTextureResources.find(resourceId);
  DALI_ASSERT_ALWAYS(iter == mImpl->mExternalTextureResources.end());

  texture = CreateTexture(createInfo, std::move(texture));

  auto gfxTexture = texture.get();
  mImpl->mExternalTextureResources.insert(std::make_pair(resourceId, std::move(texture)));

  return gfxTexture;
}

void VulkanGraphicsController::DiscardTextureFromResourceId(uint32_t resourceId)
{
  auto iter = mImpl->mExternalTextureResources.find(resourceId);
  if(iter != mImpl->mExternalTextureResources.end())
  {
    mImpl->mExternalTextureResources.erase(iter);
  }
}

Graphics::Texture* VulkanGraphicsController::GetTextureFromResourceId(uint32_t resourceId)
{
  Graphics::Texture* gfxTexture = nullptr;

  auto iter = mImpl->mExternalTextureResources.find(resourceId);
  if(iter != mImpl->mExternalTextureResources.end())
  {
    gfxTexture = iter->second.get();
  }

  return gfxTexture;
}

Graphics::UniquePtr<Graphics::Texture> VulkanGraphicsController::ReleaseTextureFromResourceId(uint32_t resourceId)
{
  Graphics::UniquePtr<Graphics::Texture> gfxTexture;

  auto iter = mImpl->mExternalTextureResources.find(resourceId);
  if(iter != mImpl->mExternalTextureResources.end())
  {
    gfxTexture = std::move(iter->second);
    mImpl->mExternalTextureResources.erase(iter);
  }

  return gfxTexture;
}

void VulkanGraphicsController::Flush()
{
  mImpl->Flush();
}

std::size_t VulkanGraphicsController::GetCapacity() const
{
  return mImpl->mCapacity;
}

bool VulkanGraphicsController::HasClipMatrix() const
{
  return true;
}

const Matrix& VulkanGraphicsController::GetClipMatrix() const
{
  constexpr float CLIP_MATRIX_DATA[] = {
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f};
  static const Matrix CLIP_MATRIX(CLIP_MATRIX_DATA);
  return CLIP_MATRIX;
}

} // namespace Dali::Graphics::Vulkan
