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
 *
 */

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

CommandBufferImpl::~CommandBufferImpl() = default;

/** Begin recording */
void CommandBufferImpl::Begin(vk::CommandBufferUsageFlags       usageFlags,
                              vk::CommandBufferInheritanceInfo* inheritanceInfo)
{
  assert(!mRecording && "CommandBufferImpl already is in the recording state");
  auto info = vk::CommandBufferBeginInfo{};

  vk::CommandBufferInheritanceInfo defaultInheritanceInfo{};
  defaultInheritanceInfo.sType                = vk::StructureType::eCommandBufferInheritanceInfo;
  defaultInheritanceInfo.pNext                = nullptr;
  defaultInheritanceInfo.subpass              = 0;
  defaultInheritanceInfo.occlusionQueryEnable = false;
  defaultInheritanceInfo.queryFlags           = static_cast<vk::QueryControlFlags>(0);
  defaultInheritanceInfo.pipelineStatistics   = static_cast<vk::QueryPipelineStatisticFlags>(0);

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

  auto& pipelineImpl = static_cast<const Vulkan::Pipeline*>(pipeline)->GetImpl();

  // Bind if pipeline is ready (if nullptr, pipeline isn't ready).
  // If pipeline is valid, bind it early
  if(pipelineImpl.GetVkPipeline())
  {
    mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineImpl.GetVkPipeline());
  }
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
}

void CommandBufferImpl::BindTextures(const std::vector<TextureBinding>& textureBindings)
{
  for(const auto& textureBinding : textureBindings)
  {
    auto texture   = static_cast<const Vulkan::Texture*>(textureBinding.texture);
    auto sampler   = const_cast<Vulkan::Sampler*>(static_cast<const Vulkan::Sampler*>(textureBinding.sampler));
    auto vkSampler = sampler ? sampler->GetImpl()->GetVkHandle() : nullptr;

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
    mDeferredTextureBindings.back().binding   = textureBinding.binding;
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

bool CommandBufferImpl::OnDestroy()
{
  mOwnerCommandPool->ReleaseCommandBuffer(*this);
  return true;
}

void CommandBufferImpl::Draw(uint32_t vertexCount,
                             uint32_t instanceCount,
                             uint32_t firstVertex,
                             uint32_t firstInstance)
{
  mCommandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBufferImpl::DrawIndexed(uint32_t indexCount,
                                    uint32_t instanceCount,
                                    uint32_t firstIndex,
                                    int32_t  vertexOffset,
                                    uint32_t firstInstance)
{
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

} // namespace Vulkan
} // namespace Graphics
} // namespace Dali
