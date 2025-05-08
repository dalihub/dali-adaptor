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
 *
 */

// CLASS HEADER
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-impl.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-pool-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-view-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-pipeline-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-sampler-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-sampler.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-swapchain-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-texture.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-vector.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gLogCmdBufferFilter;
#endif

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{
CommandBufferImpl::CommandBufferImpl(CommandPool&                         commandPool,
                                     uint32_t                             poolIndex,
                                     const vk::CommandBufferAllocateInfo& allocateInfo,
                                     vk::CommandBuffer                    vulkanHandle)
: mOwnerCommandPool(&commandPool),
  mGraphicsDevice(mOwnerCommandPool->GetGraphicsDevice()),
  mPoolAllocationIndex(poolIndex),
  mAllocateInfo(allocateInfo),
  mCommandBuffer(vulkanHandle)
{
}

CommandBufferImpl::~CommandBufferImpl()
{
  Destroy();
}

void CommandBufferImpl::Destroy()
{
  // Command buffer Pool cleanup will remove the vulkan command buffer
}

/** Begin recording */
void CommandBufferImpl::Begin(vk::CommandBufferUsageFlags       usageFlags,
                              vk::CommandBufferInheritanceInfo* inheritanceInfo)
{
  assert(!mRecording && "CommandBufferImpl already is in the recording state");
  mDeferredPipelineToBind = nullptr;
  mDepthStencilState      = vk::PipelineDepthStencilStateCreateInfo();

  auto info = vk::CommandBufferBeginInfo{};

  static vk::CommandBufferInheritanceInfo defaultInheritanceInfo{};
  static bool                             init{false};
  if(!init)
  {
    defaultInheritanceInfo.pNext                = nullptr;
    defaultInheritanceInfo.subpass              = 0;
    defaultInheritanceInfo.occlusionQueryEnable = false;
    defaultInheritanceInfo.queryFlags           = static_cast<vk::QueryControlFlags>(0);
    defaultInheritanceInfo.pipelineStatistics   = static_cast<vk::QueryPipelineStatisticFlags>(0);
    init                                        = true;
  }

  if(!inheritanceInfo)
  {
    inheritanceInfo = &defaultInheritanceInfo;
  }
  info.setPInheritanceInfo(inheritanceInfo);
  info.setFlags(usageFlags);

  VkAssert(mCommandBuffer.begin(info));

  mRecording = true;
}

/** Finish recording */
void CommandBufferImpl::End()
{
  assert(mRecording && "CommandBufferImpl is not in the recording state!");
  VkAssert(mCommandBuffer.end());
  mRecording = false;
}

/** Reset command buffer */
void CommandBufferImpl::Reset()
{
  assert(!mRecording && "Can't reset command buffer during recording!");
  assert(mCommandBuffer && "Invalid command buffer!");
  mCommandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
  mDeferredPipelineToBind = nullptr;
  mDepthStencilState      = vk::PipelineDepthStencilStateCreateInfo();
}

/** Free command buffer */
void CommandBufferImpl::Free()
{
  assert(mCommandBuffer && "Invalid command buffer!");
  mGraphicsDevice->GetLogicalDevice().freeCommandBuffers(mOwnerCommandPool->GetVkHandle(), mCommandBuffer);
}

void CommandBufferImpl::BindPipeline(const Graphics::Pipeline* pipeline)
{
  assert(mCommandBuffer && "Invalid command buffer!");
  assert(mRecording && "Can't bind pipeline when buffer isn't recording!");
  assert(pipeline && "Can't bind null pipeline!");

  // We don't bind anything yet, we will bind it when we resolve depth/stencil state
  mDeferredPipelineToBind = const_cast<Vulkan::Pipeline*>(static_cast<const Vulkan::Pipeline*>(pipeline));
}

void CommandBufferImpl::BindVertexBuffers(
  uint32_t                        firstBinding,
  const std::vector<BufferImpl*>& buffers,
  const std::vector<uint32_t>&    offsets)
{
  // update list of used resources and create an array of VkBuffers
  std::vector<vk::Buffer> vkBuffers;
  vkBuffers.reserve(buffers.size());
  for(auto&& buffer : buffers)
  {
    vkBuffers.emplace_back(buffer->GetVkHandle());
  }
  std::vector<vk::DeviceSize> vkOffsets;
  vkOffsets.reserve(offsets.size());
  for(auto&& offset : offsets)
  {
    vkOffsets.emplace_back(static_cast<vk::DeviceSize>(offset));
  }
  mCommandBuffer.bindVertexBuffers(firstBinding, vkBuffers.size(), vkBuffers.data(), vkOffsets.data());
}

void CommandBufferImpl::BindIndexBuffer(
  BufferImpl& buffer,
  uint32_t    offset,
  Format      format)
{
  if(format == Graphics::Format::R16_UINT)
  {
    mCommandBuffer.bindIndexBuffer(buffer.GetVkHandle(), offset, vk::IndexType::eUint16);
  }
  else if(format == Graphics::Format::R32_UINT)
  {
    mCommandBuffer.bindIndexBuffer(buffer.GetVkHandle(), offset, vk::IndexType::eUint32);
  }
}

void CommandBufferImpl::BindUniformBuffers(const std::vector<UniformBufferBinding>& bindings)
{
  // Needs descriptor set pools.
  bool standalone = true;
  for(const auto& uniformBinding : bindings)
  {
    if(standalone)
    {
      // First buffer is not used in Vulkan (it's a fake buffer in GLES)
      standalone = false;
      continue;
    }

    auto buffer = const_cast<Vulkan::Buffer*>(static_cast<const Vulkan::Buffer*>(uniformBinding.buffer));

    CommandBufferImpl::DeferredUniformBinding deferredUniformBinding{};
    deferredUniformBinding.buffer  = buffer->GetImpl()->GetVkHandle();
    deferredUniformBinding.offset  = uniformBinding.offset;
    deferredUniformBinding.range   = uniformBinding.dataSize;
    deferredUniformBinding.binding = uniformBinding.binding;

    mDeferredUniformBindings.push_back(deferredUniformBinding);
  }
}

void CommandBufferImpl::BindTextures(const std::vector<TextureBinding>& textureBindings)
{
  for(const auto& textureBinding : textureBindings)
  {
    auto texture     = static_cast<const Vulkan::Texture*>(textureBinding.texture);
    auto sampler     = const_cast<Vulkan::Sampler*>(static_cast<const Vulkan::Sampler*>(textureBinding.sampler));
    auto samplerImpl = sampler ? sampler->GetImpl() : texture->GetSampler();
    auto vkSampler   = samplerImpl ? samplerImpl->GetVkHandle() : nullptr;
    // @todo If there is still no sampler, fall back to default?
    if(!vkSampler)
    {
      DALI_LOG_INFO(gLogCmdBufferFilter, Debug::Concise, "No sampler for texture binding\n");
    }

    auto image     = texture->GetImage();
    auto imageView = texture->GetImageView();

    // test if image is valid, skip invalid image
    if(!image || !image->GetVkHandle())
    {
      continue;
    }

    // Store: imageView, sampler & texture.binding for later use
    // We don't know at this point what pipeline is bound (As dali-core
    // binds the pipeline after calling this API)

    mDeferredTextureBindings.emplace_back();
    mDeferredTextureBindings.back().imageView = imageView->GetVkHandle();
    mDeferredTextureBindings.back().sampler   = vkSampler;
    mDeferredTextureBindings.back().binding   = textureBinding.binding; // zero indexed
  }
}

void CommandBufferImpl::BindSamplers(const std::vector<SamplerBinding>& samplerBindings)
{
  // Unused in core
}

vk::CommandBuffer CommandBufferImpl::GetVkHandle() const
{
  return mCommandBuffer;
}

bool CommandBufferImpl::IsPrimary() const
{
  return mAllocateInfo.level == vk::CommandBufferLevel::ePrimary;
}

void CommandBufferImpl::BeginRenderPass(vk::RenderPassBeginInfo renderPassBeginInfo, vk::SubpassContents subpassContents)
{
  mCommandBuffer.beginRenderPass(renderPassBeginInfo, subpassContents);
}

void CommandBufferImpl::EndRenderPass()
{
  mCommandBuffer.endRenderPass();
}

void CommandBufferImpl::ReadPixels(uint8_t* buffer)
{
}

void CommandBufferImpl::PipelineBarrier(
  vk::PipelineStageFlags               srcStageMask,
  vk::PipelineStageFlags               dstStageMask,
  vk::DependencyFlags                  dependencyFlags,
  std::vector<vk::MemoryBarrier>       memoryBarriers,
  std::vector<vk::BufferMemoryBarrier> bufferBarriers,
  std::vector<vk::ImageMemoryBarrier>  imageBarriers)
{
  mCommandBuffer.pipelineBarrier(srcStageMask,
                                 dstStageMask,
                                 dependencyFlags,
                                 memoryBarriers,
                                 bufferBarriers,
                                 imageBarriers);
}

void CommandBufferImpl::CopyBufferToImage(
  Vulkan::BufferImpl* srcBuffer, Vulkan::Image* dstImage, vk::ImageLayout dstLayout, const std::vector<vk::BufferImageCopy>& regions)
{
  mCommandBuffer.copyBufferToImage(srcBuffer->GetVkHandle(),
                                   dstImage->GetVkHandle(),
                                   dstLayout,
                                   regions);
}

void CommandBufferImpl::CopyImage(
  Vulkan::Image* srcImage, vk::ImageLayout srcLayout, Vulkan::Image* dstImage, vk::ImageLayout dstLayout, const std::vector<vk::ImageCopy>& regions)
{
  mCommandBuffer.copyImage(srcImage->GetVkHandle(), srcLayout, dstImage->GetVkHandle(), dstLayout, regions);
}

uint32_t CommandBufferImpl::GetPoolAllocationIndex() const
{
  return mPoolAllocationIndex;
}

void CommandBufferImpl::ResolveDeferredPipelineBinding()
{
  // Pipeline deferred state to resolve depth/stencil
  if(mDeferredPipelineToBind)
  {
    auto& impl = mDeferredPipelineToBind->GetImpl();
    // The depth stencil state doesn't match the pipeline
    vk::Pipeline pipelineToBind;
    mDepthStencilState.front = mStencilTestFrontState;
    mDepthStencilState.back  = mStencilTestBackState;
    if(!impl.ComparePipelineDepthStencilState(mDepthStencilState))
    {
      // Clone implementation for that state
      // One pipeline will hold to multiple implementations (?)
      pipelineToBind = impl.CloneInheritedVkPipeline(mDepthStencilState);
    }

    if(!pipelineToBind)
    {
      pipelineToBind = impl.GetVkPipeline(); // get default pipeline
    }

    if(pipelineToBind)
    {
      mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineToBind);
      mCurrentProgram = impl.GetProgram()->GetImplementation();
    }
    else
    {
      mCurrentProgram = nullptr;
    }
  }
}

void CommandBufferImpl::Draw(uint32_t vertexCount,
                             uint32_t instanceCount,
                             uint32_t firstVertex,
                             uint32_t firstInstance)
{
  ResolveDeferredPipelineBinding();

  if(mCurrentProgram)
  {
    auto set = mCurrentProgram->AllocateDescriptorSet(-1); // allocate from recent pool
    if(set)
    {
      BindResources(set);
    }
  }
  if(instanceCount == 0)
  {
    instanceCount = 1;
  }
  mCommandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBufferImpl::DrawIndexed(uint32_t indexCount,
                                    uint32_t instanceCount,
                                    uint32_t firstIndex,
                                    int32_t  vertexOffset,
                                    uint32_t firstInstance)
{
  ResolveDeferredPipelineBinding();

  if(mCurrentProgram)
  {
    auto set = mCurrentProgram->AllocateDescriptorSet(-1); // allocate from recent pool
    if(set)
    {
      BindResources(set);
    }
  }
  // draw here
  if(instanceCount == 0)
  {
    instanceCount = 1;
  }
  mCommandBuffer.drawIndexed(indexCount,
                             instanceCount,
                             firstIndex,
                             static_cast<int32_t>(vertexOffset),
                             firstInstance);
}

void CommandBufferImpl::DrawIndexedIndirect(BufferImpl& buffer,
                                            uint32_t    offset,
                                            uint32_t    drawCount,
                                            uint32_t    stride)
{
  mCommandBuffer.drawIndexedIndirect(buffer.GetVkHandle(), static_cast<vk::DeviceSize>(offset), drawCount, stride);
}

void CommandBufferImpl::ExecuteCommandBuffers(std::vector<vk::CommandBuffer>& commandBuffers)
{
  mCommandBuffer.executeCommands(commandBuffers);
}

void CommandBufferImpl::SetScissor(Rect2D value)
{
  mCommandBuffer.setScissor(0, 1, reinterpret_cast<vk::Rect2D*>(&value));
}

void CommandBufferImpl::SetViewport(Viewport value)
{
  mCommandBuffer.setViewport(0, 1, reinterpret_cast<vk::Viewport*>(&value));
}

void CommandBufferImpl::SetStencilTestEnable(bool stencilEnable)
{
  mDepthStencilState.setStencilTestEnable(stencilEnable);
  if(!stencilEnable)
  {
    // Reset
    mStencilTestFrontState.setCompareMask(0x00);
    mStencilTestFrontState.setWriteMask(0x00);
    mStencilTestFrontState.setReference(0x00);
    mStencilTestFrontState.setCompareOp(vk::CompareOp::eLess);
    mStencilTestFrontState.setFailOp(vk::StencilOp::eKeep);
    mStencilTestFrontState.setPassOp(vk::StencilOp::eKeep);
    mStencilTestFrontState.setDepthFailOp(vk::StencilOp::eKeep);

    mStencilTestBackState.setCompareMask(0x00);
    mStencilTestBackState.setWriteMask(0x00);
    mStencilTestBackState.setReference(0x00);
    mStencilTestBackState.setCompareOp(vk::CompareOp::eLess);
    mStencilTestBackState.setFailOp(vk::StencilOp::eKeep);
    mStencilTestBackState.setPassOp(vk::StencilOp::eKeep);
    mStencilTestBackState.setDepthFailOp(vk::StencilOp::eKeep);
    mDepthStencilState.setFront(mStencilTestFrontState);
    mDepthStencilState.setBack(mStencilTestBackState);
  }
}

void CommandBufferImpl::SetStencilWriteMask(vk::StencilFaceFlags faceMask, uint32_t writeMask)
{
  auto f = faceMask & vk::StencilFaceFlagBits::eFront ? 1 : 0;
  auto b = faceMask & vk::StencilFaceFlagBits::eBack ? 2 : 0;
  mStencilTestStates[f].setWriteMask(writeMask);
  mStencilTestStates[b].setWriteMask(writeMask);
}

void CommandBufferImpl::SetStencilCompareMask(vk::StencilFaceFlags faceMask, uint32_t compareMask)
{
  auto f = faceMask & vk::StencilFaceFlagBits::eFront ? 1 : 0;
  auto b = faceMask & vk::StencilFaceFlagBits::eBack ? 2 : 0;
  mStencilTestStates[f].setCompareMask(compareMask);
  mStencilTestStates[b].setCompareMask(compareMask);
}

void CommandBufferImpl::SetStencilReference(vk::StencilFaceFlags faceMask, uint32_t reference)
{
  auto f = faceMask & vk::StencilFaceFlagBits::eFront ? 1 : 0;
  auto b = faceMask & vk::StencilFaceFlagBits::eBack ? 2 : 0;
  mStencilTestStates[f].setReference(reference);
  mStencilTestStates[b].setReference(reference);
}

void CommandBufferImpl::SetStencilOp(vk::StencilFaceFlags faceMask, vk::StencilOp failOp, vk::StencilOp passOp, vk::StencilOp depthFailOp, vk::CompareOp compareOp)
{
  auto f = faceMask & vk::StencilFaceFlagBits::eFront ? 1 : 0;
  auto b = faceMask & vk::StencilFaceFlagBits::eBack ? 2 : 0;
  mStencilTestStates[f].setFailOp(failOp);
  mStencilTestStates[f].setDepthFailOp(depthFailOp);
  mStencilTestStates[f].setPassOp(passOp);
  mStencilTestStates[f].setCompareOp(compareOp);
  mStencilTestStates[b].setFailOp(failOp);
  mStencilTestStates[b].setDepthFailOp(depthFailOp);
  mStencilTestStates[b].setPassOp(passOp);
  mStencilTestStates[b].setCompareOp(compareOp);
}

void CommandBufferImpl::SetDepthTestEnable(bool depthTestEnable)
{
  mDepthStencilState.setDepthTestEnable(depthTestEnable);
  mDepthStencilState.setDepthBoundsTestEnable(false);
  mDepthStencilState.setMinDepthBounds(0.0f);
  mDepthStencilState.setMaxDepthBounds(1.0f);
}

void CommandBufferImpl::SetDepthWriteEnable(bool depthWriteEnable)
{
  mDepthStencilState.setDepthWriteEnable(depthWriteEnable);
}

void CommandBufferImpl::SetDepthCompareOp(vk::CompareOp op)
{
  mDepthStencilState.setDepthCompareOp(op);
}

void CommandBufferImpl::BindResources(vk::DescriptorSet descriptorSet)
{
  auto& reflection = mCurrentProgram->GetReflection();
  auto& samplers   = reflection.GetSamplers();

  static Dali::Vector<vk::DescriptorImageInfo>  imageInfos;
  static Dali::Vector<vk::DescriptorBufferInfo> bufferInfos;
  static Dali::Vector<vk::WriteDescriptorSet>   descriptorWrites;

  bufferInfos.Reserve(mDeferredUniformBindings.size() + 1);
  descriptorWrites.Reserve(mDeferredUniformBindings.size() + samplers.size() + 1);

  imageInfos.Clear();
  bufferInfos.Clear();
  descriptorWrites.Clear();

  // Deferred uniform buffer bindings:
  for(auto& uniformBinding : mDeferredUniformBindings)
  {
    auto bufferInfo = vk::DescriptorBufferInfo{}
                        .setOffset(uniformBinding.offset)
                        .setRange(uniformBinding.range)
                        .setBuffer(uniformBinding.buffer);
    bufferInfos.PushBack(bufferInfo);

    auto writeDescriptorSet = vk::WriteDescriptorSet{}
                                .setPBufferInfo(&bufferInfos[bufferInfos.Size() - 1])
                                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                                .setDescriptorCount(1)
                                .setDstSet(descriptorSet)
                                .setDstBinding(uniformBinding.binding)
                                .setDstArrayElement(0);
    descriptorWrites.PushBack(writeDescriptorSet);
  }

  // Deferred texture bindings:
  if(!samplers.empty()) // Ignore any texture bindings if the program is not expecting them
  {
    imageInfos.Reserve(samplers.size() + 1);
    for(auto& info : samplers)
    {
      bool     found   = false;
      uint32_t binding = 1;
      for(auto& textureBinding : mDeferredTextureBindings)
      {
        if(info.location == textureBinding.binding)
        {
          found   = true;
          binding = info.binding;

          auto imageInfo = vk::DescriptorImageInfo{}
                             .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                             .setImageView(textureBinding.imageView)
                             .setSampler(textureBinding.sampler);
          imageInfos.PushBack(imageInfo);

          break;
        }
      }
      if(found)
      {
        auto writeDescriptorSet = vk::WriteDescriptorSet{}
                                    .setPImageInfo(&imageInfos[imageInfos.Size() - 1])
                                    .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                                    .setDescriptorCount(1)
                                    .setDstSet(descriptorSet)
                                    .setDstBinding(binding)
                                    .setDstArrayElement(0);
        descriptorWrites.PushBack(writeDescriptorSet);
      }
    }
  }
  if(!descriptorWrites.Empty())
  {
    mGraphicsDevice->GetLogicalDevice().updateDescriptorSets(uint32_t(descriptorWrites.Size()), &descriptorWrites[0], 0, nullptr);
  }

  auto pipelineLayout = reflection.GetVkPipelineLayout();

  mCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                    pipelineLayout,
                                    0,
                                    1,
                                    &descriptorSet, // @note - old impl could use multiple sets (possibly)
                                    0,
                                    nullptr);
  mDeferredTextureBindings.clear();
  mDeferredUniformBindings.clear();
}

} // namespace Vulkan
} // namespace Graphics
} // namespace Dali
