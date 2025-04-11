/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
  mDynamicStateMask       = CommandBufferImpl::INITIAL_DYNAMIC_MASK_VALUE;

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

  mLastBoundPipeline = VK_NULL_HANDLE;

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

  DALI_LOG_INFO(gLogCmdBufferFilter, Debug::Verbose, "this:%p Reset vk-handle: %p\n", this, mCommandBuffer);
  mCommandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
  mDeferredPipelineToBind = nullptr;
  mLastBoundPipeline      = VK_NULL_HANDLE;
  mDepthStencilState      = vk::PipelineDepthStencilStateCreateInfo();
  mColorWriteMask         = true;
  mDynamicStateMask       = CommandBufferImpl::INITIAL_DYNAMIC_MASK_VALUE;
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

void CommandBufferImpl::BindVertexBuffers(
  uint32_t                                          firstBinding,
  const IndirectPtr<VertexBufferBindingDescriptor>& bindingPtr,
  uint32_t                                          bindingCount)
{
  // update list of used resources and create an array of VkBuffers
  std::vector<vk::Buffer>     vkBuffers;
  std::vector<vk::DeviceSize> vkOffsets;
  vkBuffers.reserve(bindingCount);
  vkOffsets.reserve(bindingCount);

  for(uint32_t i = 0u; i < bindingCount; ++i)
  {
    auto& binding = bindingPtr.At(i);
    vkBuffers.emplace_back(const_cast<Buffer*>(binding.buffer)->GetImpl()->GetVkHandle());
    vkOffsets.emplace_back(static_cast<vk::DeviceSize>(binding.offset));
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
  bool standalone = true;
  for(const auto& uniformBinding : bindings)
  {
    if(standalone)
    {
      // First buffer is not used in Vulkan (it's a fake buffer in GLES)
      standalone = false;
      continue;
    }
    BindUniformBuffer(uniformBinding);
  }
}

void CommandBufferImpl::BindUniformBuffers(const IndirectPtr<UniformBufferBindingDescriptor>& uniformBindingPtr, uint32_t count)
{
  mDeferredUniformBindingDescriptor      = uniformBindingPtr;
  mDeferredUniformBindingDescriptorCount = count;
}

void CommandBufferImpl::BindUniformBuffer(const UniformBufferBinding& uniformBinding)
{
  auto buffer = const_cast<Vulkan::Buffer*>(static_cast<const Vulkan::Buffer*>(uniformBinding.buffer));

  CommandBufferImpl::DeferredUniformBinding deferredUniformBinding{};
  deferredUniformBinding.buffer  = buffer->GetImpl()->GetVkHandle();
  deferredUniformBinding.offset  = uniformBinding.offset;
  deferredUniformBinding.range   = uniformBinding.dataSize;
  deferredUniformBinding.binding = uniformBinding.binding;

  mDeferredUniformBindings.push_back(deferredUniformBinding);
}

void CommandBufferImpl::BindTextures(const std::vector<TextureBinding>& textureBindings)
{
  for(const auto& textureBinding : textureBindings)
  {
    BindTexture(textureBinding);
  }
}

void CommandBufferImpl::BindTextures(const IndirectPtr<TextureBinding>& textureBindingPtr, uint32_t count)
{
  // @todo Eliminate this copy - can pick up this ptr later.
  for(uint32_t i = 0; i < count; ++i)
  {
    if(!BindTexture(textureBindingPtr.At(i)))
      continue;
  }
}

bool CommandBufferImpl::BindTexture(const TextureBinding& textureBinding)
{
  auto texture     = const_cast<Vulkan::Texture*>(static_cast<const Vulkan::Texture*>(textureBinding.texture));
  auto sampler     = const_cast<Vulkan::Sampler*>(static_cast<const Vulkan::Sampler*>(textureBinding.sampler));
  auto samplerImpl = sampler ? sampler->GetImpl() : texture->GetDefaultSampler();
  auto vkSampler   = samplerImpl ? samplerImpl->GetVkHandle() : nullptr;
  // @todo If there is still no sampler, fall back to default?
  if(!vkSampler)
  {
    DALI_LOG_INFO(gLogCmdBufferFilter, Debug::Concise, "No sampler for texture binding\n");
  }

  // Ensure native image is prepared
  texture->PrepareTexture();

  auto image     = texture->GetImage();
  auto imageView = texture->GetImageView();

  // test if image is valid, skip invalid image
  if(!image || !image->GetVkHandle())
  {
    return false;
  }

  // Store: imageView, sampler & texture.binding for later use
  // We don't know at this point what pipeline is bound (As dali-core
  // binds the pipeline after calling this API)

  mDeferredTextureBindings.emplace_back();
  mDeferredTextureBindings.back().imageView = imageView->GetVkHandle();
  mDeferredTextureBindings.back().sampler   = vkSampler;
  mDeferredTextureBindings.back().binding   = textureBinding.binding; // zero indexed
  return true;
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
  vk::PipelineStageFlags                      srcStageMask,
  vk::PipelineStageFlags                      dstStageMask,
  vk::DependencyFlags                         dependencyFlags,
  const std::vector<vk::MemoryBarrier>&       memoryBarriers,
  const std::vector<vk::BufferMemoryBarrier>& bufferBarriers,
  const std::vector<vk::ImageMemoryBarrier>&  imageBarriers)
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

    // Check if the depth/stencil/color mask state doesn't match the pipeline
    vk::Pipeline pipelineToBind;
    mDepthStencilState.front = mStencilTestFrontState;
    mDepthStencilState.back  = mStencilTestBackState;
    if(!impl.ComparePipelineDepthStencilState(mDepthStencilState, mColorWriteMask))
    {
      // Clone implementation for that state
      // One pipeline will hold to multiple implementations (?)
      pipelineToBind = impl.CloneInheritedVkPipeline(mDepthStencilState, mColorWriteMask);
    }

    if(!pipelineToBind)
    {
      pipelineToBind = impl.GetVkPipeline(); // get default pipeline
    }

    if(pipelineToBind)
    {
      if(pipelineToBind != mLastBoundPipeline)
      {
        // Check if this pipeline uses advanced blending and insert blend barrier if needed
        const auto& pipelineCreateInfo = mDeferredPipelineToBind->GetCreateInfo();
        if(pipelineCreateInfo.colorBlendState && pipelineCreateInfo.colorBlendState->blendEnable)
        {
          auto blendOp = pipelineCreateInfo.colorBlendState->colorBlendOp;

          if(blendOp >= Graphics::ADVANCED_BLEND_OPTIONS_START)
          {
            // Insert blend barrier for advanced blending operations
            // This is equivalent to glBlendBarrier() in GLES
            vk::MemoryBarrier memoryBarrier{
              vk::AccessFlagBits::eColorAttachmentWrite,
              vk::AccessFlagBits::eColorAttachmentRead};

            mCommandBuffer.pipelineBarrier(
              vk::PipelineStageFlagBits::eColorAttachmentOutput,
              vk::PipelineStageFlagBits::eColorAttachmentOutput,
              vk::DependencyFlagBits::eByRegion,
              memoryBarrier,
              nullptr,
              nullptr);
          }
        }

        mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineToBind);
        mLastBoundPipeline = pipelineToBind;

        // Apply deferred color blend states
        ApplyDeferredColorBlendStates();
      }
      mCurrentProgram = impl.GetProgram()->GetImplementation();
    }
    else
    {
      mCurrentProgram = nullptr;
    }
  }
}

void CommandBufferImpl::PrepareForDraw()
{
  ResolveDeferredPipelineBinding();

  if(mCurrentProgram)
  {
    uint32_t frameIndex = mGraphicsDevice->GetCurrentBufferIndex();

    // Fetch next available descriptor set
    vk::DescriptorSet set = mCurrentProgram->GetNextDescriptorSetForFrame(frameIndex);
    if(set)
    {
      // Update and bind
      UpdateDescriptorSet(set);
      BindResources(set);
    }
  }
}

void CommandBufferImpl::Draw(uint32_t vertexCount,
                             uint32_t instanceCount,
                             uint32_t firstVertex,
                             uint32_t firstInstance)
{
  // Resolve pipeline binding and bind the appropriate descriptor set
  PrepareForDraw();

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
  // Resolve pipeline binding and bind the appropriate descriptor set
  PrepareForDraw();

  if(instanceCount == 0)
  {
    instanceCount = 1;
  }

  mCommandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, static_cast<int32_t>(vertexOffset), firstInstance);
}

void CommandBufferImpl::DrawIndexedIndirect(BufferImpl& buffer,
                                            uint32_t    offset,
                                            uint32_t    drawCount,
                                            uint32_t    stride)
{
  // Resolve pipeline binding and bind the appropriate descriptor set
  PrepareForDraw();

  mCommandBuffer.drawIndexedIndirect(buffer.GetVkHandle(), static_cast<vk::DeviceSize>(offset), drawCount, stride);
}

void CommandBufferImpl::SetScissor(Rect2D value)
{
  if(SetDynamicState(mDynamicState.scissor, value, DynamicStateMaskBits::SCISSOR))
  {
    mCommandBuffer.setScissor(0, 1, reinterpret_cast<vk::Rect2D*>(&value));
  }
}

void CommandBufferImpl::SetViewport(Viewport value)
{
  if(SetDynamicState(mDynamicState.viewport, value, DynamicStateMaskBits::VIEWPORT))
  {
    mCommandBuffer.setViewport(0, 1, reinterpret_cast<vk::Viewport*>(&value));
  }
}

void CommandBufferImpl::SetStencilTestEnable(bool stencilEnable)
{
  if(SetDynamicState(mDynamicState.stencilTest, stencilEnable, DynamicStateMaskBits::STENCIL_TEST))
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
}

void CommandBufferImpl::SetStencilWriteMask(uint32_t writeMask)
{
  if(SetDynamicState(mDynamicState.stencilWriteMask, writeMask, DynamicStateMaskBits::STENCIL_WRITE_MASK))
  {
    auto faceMask = vk::StencilFaceFlagBits::eFrontAndBack;
    auto f        = faceMask & vk::StencilFaceFlagBits::eFront ? 1 : 0;
    auto b        = faceMask & vk::StencilFaceFlagBits::eBack ? 2 : 0;
    mStencilTestStates[f].setWriteMask(writeMask);
    mStencilTestStates[b].setWriteMask(writeMask);
  }
}

void CommandBufferImpl::SetStencilCompareMask(uint32_t compareMask)
{
  if(SetDynamicState(mDynamicState.stencilCompareMask, compareMask, DynamicStateMaskBits::STENCIL_COMP_MASK))
  {
    auto faceMask = vk::StencilFaceFlagBits::eFrontAndBack;
    auto f        = faceMask & vk::StencilFaceFlagBits::eFront ? 1 : 0;
    auto b        = faceMask & vk::StencilFaceFlagBits::eBack ? 2 : 0;
    mStencilTestStates[f].setCompareMask(compareMask);
    mStencilTestStates[b].setCompareMask(compareMask);
  }
}

void CommandBufferImpl::SetStencilReference(uint32_t reference)
{
  if(SetDynamicState(mDynamicState.stencilReference, reference, DynamicStateMaskBits::STENCIL_REF))
  {
    auto faceMask = vk::StencilFaceFlagBits::eFrontAndBack;
    auto f        = faceMask & vk::StencilFaceFlagBits::eFront ? 1 : 0;
    auto b        = faceMask & vk::StencilFaceFlagBits::eBack ? 2 : 0;
    mStencilTestStates[f].setReference(reference);
    mStencilTestStates[b].setReference(reference);
  }
}

void CommandBufferImpl::SetStencilOp(
  Graphics::StencilOp failOp,
  Graphics::StencilOp passOp,
  Graphics::StencilOp depthFailOp,
  Graphics::CompareOp compareOp)
{
  int result = 0;
  result |= SetDynamicState(mDynamicState.stencilFailOp, failOp, DynamicStateMaskBits::STENCIL_OP_FAIL);
  result |= SetDynamicState(mDynamicState.stencilPassOp, passOp, DynamicStateMaskBits::STENCIL_OP_PASS);
  result |= SetDynamicState(mDynamicState.stencilDepthFailOp, depthFailOp, DynamicStateMaskBits::STENCIL_OP_DEPTH_FAIL);
  result |= SetDynamicState(mDynamicState.stencilCompareOp, compareOp, DynamicStateMaskBits::STENCIL_OP_COMP);
  if(result)
  {
    auto faceMask = vk::StencilFaceFlagBits::eFrontAndBack;
    auto f        = faceMask & vk::StencilFaceFlagBits::eFront ? 1 : 0;
    auto b        = faceMask & vk::StencilFaceFlagBits::eBack ? 2 : 0;

    mStencilTestStates[f].setFailOp(VkStencilOpType(failOp).op);
    mStencilTestStates[f].setDepthFailOp(VkStencilOpType(depthFailOp).op);
    mStencilTestStates[f].setPassOp(VkStencilOpType(passOp).op);
    mStencilTestStates[f].setCompareOp(VkCompareOpType(compareOp).op);
    mStencilTestStates[b].setFailOp(VkStencilOpType(failOp).op);
    mStencilTestStates[b].setDepthFailOp(VkStencilOpType(depthFailOp).op);
    mStencilTestStates[b].setPassOp(VkStencilOpType(passOp).op);
    mStencilTestStates[b].setCompareOp(VkCompareOpType(compareOp).op);
  }
}

void CommandBufferImpl::SetDepthTestEnable(bool depthTestEnable)
{
  if(SetDynamicState(mDynamicState.depthTest, depthTestEnable, DynamicStateMaskBits::DEPTH_TEST))
  {
    mDepthStencilState.setDepthTestEnable(depthTestEnable);
    mDepthStencilState.setDepthBoundsTestEnable(false);
    mDepthStencilState.setMinDepthBounds(0.0f);
    mDepthStencilState.setMaxDepthBounds(1.0f);
  }
}

void CommandBufferImpl::SetDepthWriteEnable(bool depthWriteEnable)
{
  if(SetDynamicState(mDynamicState.depthWrite, depthWriteEnable, DynamicStateMaskBits::DEPTH_WRITE))
  {
    mDepthStencilState.setDepthWriteEnable(depthWriteEnable);
  }
}

void CommandBufferImpl::SetDepthCompareOp(Graphics::CompareOp compareOp)
{
  if(SetDynamicState(mDynamicState.depthCompareOp, compareOp, DynamicStateMaskBits::DEPTH_OP_COMP))
  {
    mDepthStencilState.setDepthCompareOp(VkCompareOpType(compareOp).op);
  }
}

void CommandBufferImpl::SetColorMask(bool enable)
{
  if(SetDynamicState(mDynamicState.colorWriteMask, enable, DynamicStateMaskBits::COLOR_WRITE_MASK))
  {
    mColorWriteMask = enable;
  }
}

void CommandBufferImpl::SetColorBlendEnable(uint32_t attachment, bool enabled)
{

  if(SetDynamicState(mDynamicState.colorBlendEnable, enabled, DynamicStateMaskBits::COLOR_BLEND_ENABLE))
  {
    // Store the deferred state
    mDeferredColorBlendStates.emplace_back();
    mDeferredColorBlendStates.back().attachment = attachment;
    mDeferredColorBlendStates.back().enableSet = true;
    mDeferredColorBlendStates.back().enable = enabled;
  }
}

void CommandBufferImpl::SetColorBlendEquation(uint32_t attachment, const ColorBlendEquation& equation)
{
  if(SetDynamicState(mDynamicState.colorBlendEquation, equation, DynamicStateMaskBits::COLOR_BLEND_EQUATION))
  {
    // Store the deferred state
    mDeferredColorBlendStates.emplace_back();
    mDeferredColorBlendStates.back().attachment = attachment;
    mDeferredColorBlendStates.back().equationSet = true;
    mDeferredColorBlendStates.back().equation = equation;
  }
}

void CommandBufferImpl::SetColorBlendAdvanced(uint32_t attachment, bool srcPremultiplied, bool dstPremultiplied, BlendOp blendOp)
{
  // Store the deferred state
  mDeferredColorBlendStates.emplace_back();
  mDeferredColorBlendStates.back().attachment = attachment;
  mDeferredColorBlendStates.back().advancedSet = true;
  mDeferredColorBlendStates.back().srcPremultiplied = srcPremultiplied;
  mDeferredColorBlendStates.back().dstPremultiplied = dstPremultiplied;
  mDeferredColorBlendStates.back().blendOp = blendOp;
}

void CommandBufferImpl::ApplyDeferredColorBlendStates()
{
  if(!mGraphicsDevice->IsAdvancedBlendingSupported())
  {
    return;
  }

  auto vkDevice = mGraphicsDevice->GetLogicalDevice();

  for(const auto& state : mDeferredColorBlendStates)
  {
    if(state.enableSet)
    {
      auto func = reinterpret_cast<PFN_vkCmdSetColorBlendEnableEXT>(vkDevice.getProcAddr("vkCmdSetColorBlendEnableEXT"));
      if(func)
      {
        vk::Bool32 enable = state.enable ? VK_TRUE : VK_FALSE;
        func(mCommandBuffer, state.attachment, 1, reinterpret_cast<VkBool32*>(&enable));
      }
    }

    if(state.equationSet)
    {
      auto func = reinterpret_cast<PFN_vkCmdSetColorBlendEquationEXT>(vkDevice.getProcAddr("vkCmdSetColorBlendEquationEXT"));
      if(func)
      {
        VkColorBlendEquationEXT eq;
        eq.srcColorBlendFactor = static_cast<VkBlendFactor>(ConvBlendFactor(state.equation.srcColorBlendFactor));
        eq.dstColorBlendFactor = static_cast<VkBlendFactor>(ConvBlendFactor(state.equation.dstColorBlendFactor));
        eq.colorBlendOp        = static_cast<VkBlendOp>(ConvBlendOp(state.equation.colorBlendOp));
        eq.srcAlphaBlendFactor = static_cast<VkBlendFactor>(ConvBlendFactor(state.equation.srcAlphaBlendFactor));
        eq.dstAlphaBlendFactor = static_cast<VkBlendFactor>(ConvBlendFactor(state.equation.dstAlphaBlendFactor));
        eq.alphaBlendOp        = static_cast<VkBlendOp>(ConvBlendOp(state.equation.alphaBlendOp));

        func(mCommandBuffer, state.attachment, 1, &eq);
      }
    }

    if(state.advancedSet)
    {
      auto func = reinterpret_cast<PFN_vkCmdSetColorBlendAdvancedEXT>(vkDevice.getProcAddr("vkCmdSetColorBlendAdvancedEXT"));
      if(func)
      {
        VkColorBlendAdvancedEXT advanced;
        advanced.advancedBlendOp = static_cast<VkBlendOp>(ConvBlendOp(state.blendOp));
        advanced.srcPremultiplied = state.srcPremultiplied ? VK_TRUE : VK_FALSE;
        advanced.dstPremultiplied = state.dstPremultiplied ? VK_TRUE : VK_FALSE;
        advanced.blendOverlap     = VK_BLEND_OVERLAP_UNCORRELATED_EXT;
        advanced.clampResults = VK_TRUE;

        func(mCommandBuffer, state.attachment, 1, &advanced);
      }
    }
  }
}

void CommandBufferImpl::BindResources(vk::DescriptorSet descriptorSet)
{
  auto& reflection = mCurrentProgram->GetReflection();

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
  mDeferredColorBlendStates.clear();
}

void CommandBufferImpl::UpdateDescriptorSet(vk::DescriptorSet descriptorSet)
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
  if(!mDeferredUniformBindings.empty())
  {
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
  }
  else if(mDeferredUniformBindingDescriptorCount > 0)
  {
    for(uint32_t i = 0; i < mDeferredUniformBindingDescriptorCount; ++i)
    {
      auto&   binding = mDeferredUniformBindingDescriptor.At(i);
      Buffer* buffer  = const_cast<Buffer*>(binding.buffer);
      if(buffer)
      {
        BufferImpl* bufferImpl = buffer->GetImpl();

        auto bufferInfo = vk::DescriptorBufferInfo{}
                            .setOffset(binding.offset)
                            .setRange(binding.dataSize)
                            .setBuffer(bufferImpl->GetVkHandle());
        bufferInfos.PushBack(bufferInfo);

        auto writeDescriptorSet = vk::WriteDescriptorSet{}
                                    .setPBufferInfo(&bufferInfos[bufferInfos.Size() - 1])
                                    .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                                    .setDescriptorCount(1)
                                    .setDstSet(descriptorSet)
                                    .setDstBinding(binding.binding)
                                    .setDstArrayElement(0);
        descriptorWrites.PushBack(writeDescriptorSet);
      }
    }
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
        if(!textureBinding.imageView)
        {
          continue;
        }

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
}

} // namespace Vulkan
} // namespace Graphics
} // namespace Dali
