/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
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
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer.h>

// INTERNAL HEADERS
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-pool-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-program-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

#include <dali/integration-api/debug.h>
#include <dali/internal/window-system/common/window-render-surface.h>

#if defined(DEBUG_ENABLED)
Debug::Filter*        gLogCmdBufferFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VK_COMMAND_BUFFER");
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{
const uint32_t EXCESS_BUFFER_COUNT = 8;

template<typename VT, typename GT>
VT* ConstGraphicsCast(const GT* object)
{
  return const_cast<VT*>(static_cast<const VT*>(object));
}

CommandBuffer::CommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, VulkanGraphicsController& controller)
: CommandBufferResource(createInfo, controller),
  mDynamicStateMask(CommandBuffer::INITIAL_DYNAMIC_MASK_VALUE)
{
  AllocateCommandBuffers();
}

CommandBuffer::~CommandBuffer() = default;

void CommandBuffer::DestroyResource()
{
  // Don't delete the impl, it's pool allocated and should have been
  // returned to the command pool for re-use.
  mCommandBufferImpl.clear();
}

ResourceBase::InitializationResult CommandBuffer::InitializeResource()
{
  return InitializationResult::INITIALIZED;
}

void CommandBuffer::DiscardResource()
{
  mController.DiscardResource(this);
}

void CommandBuffer::Begin(const Graphics::CommandBufferBeginInfo& info)
{
  mDynamicStateMask = CommandBuffer::INITIAL_DYNAMIC_MASK_VALUE;
  mRenderTarget     = ConstGraphicsCast<Vulkan::RenderTarget, Graphics::RenderTarget>(info.renderTarget);

  auto commandBufferImpl = GetImpl();
  DALI_LOG_INFO(gLogCmdBufferFilter, Debug::Verbose, "CommandBuffer::Begin: ptr:%p bufferIndex=%d\n", GetImpl(), mController.GetGraphicsDevice().GetCurrentBufferIndex());
  mCmdCount++; // Debug info

  if(commandBufferImpl)
  {
    vk::CommandBufferInheritanceInfo inheritanceInfo{};
    if(info.renderPass)
    {
      inheritanceInfo.renderPass         = mRenderTarget->GetRenderPass(info.renderPass)->GetVkHandle();
      inheritanceInfo.subpass            = 0;
      inheritanceInfo.framebuffer        = mRenderTarget->GetCurrentFramebufferImpl()->GetVkHandle();
      inheritanceInfo.queryFlags         = static_cast<vk::QueryControlFlags>(0);
      inheritanceInfo.pipelineStatistics = static_cast<vk::QueryPipelineStatisticFlags>(0);
    }
    commandBufferImpl->Begin(static_cast<vk::CommandBufferUsageFlags>(info.usage), &inheritanceInfo);

    // Default depth/stencil should be off:
    SetDepthTestEnable(false);
    SetDepthWriteEnable(false);
    SetDepthCompareOp(Graphics::CompareOp::LESS);
    SetStencilTestEnable(false);
  }
}

void CommandBuffer::End()
{
  CommandBufferImpl* commandBufferImpl = GetImpl();
  DALI_LOG_INFO(gLogCmdBufferFilter, Debug::Verbose, "CommandBuffer::End: ptr:%p bufferIndex=%d\n", commandBufferImpl, mController.GetGraphicsDevice().GetCurrentBufferIndex());

  mCmdCount++; // Debug info
  commandBufferImpl->End();
}

void CommandBuffer::Reset()
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();

  if(bufferIndex >= EXCESS_BUFFER_COUNT)
  {
    DALI_LOG_ERROR("ERROR: bufferIndex %u exceeds EXCESS_BUFFER_COUNT\n", bufferIndex);
  }
  DALI_ASSERT_DEBUG(bufferIndex < EXCESS_BUFFER_COUNT);

  if(bufferIndex >= mCommandBufferImpl.size())
  {
    // Handle (odd) case where swapchain is re-created with a different number of min images
    AllocateCommandBuffers();
  }

  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size());
  mCommandBufferImpl[bufferIndex]->Reset();

  mDynamicStateMask = CommandBuffer::INITIAL_DYNAMIC_MASK_VALUE;
  mRenderTarget     = nullptr;
  mCmdCount         = 1;
}

void CommandBuffer::BindVertexBuffers(uint32_t                                    firstBinding,
                                      const std::vector<const Graphics::Buffer*>& gfxBuffers,
                                      const std::vector<uint32_t>&                offsets)
{
  mCmdCount++; // Debug info
  std::vector<BufferImpl*> buffers;
  buffers.reserve(gfxBuffers.size());
  for(auto& gfxBuffer : gfxBuffers)
  {
    buffers.push_back(ConstGraphicsCast<Buffer, Graphics::Buffer>(gfxBuffer)->GetImpl());
  }

  CommandBufferImpl* commandBufferImpl = GetImpl();
  commandBufferImpl->BindVertexBuffers(firstBinding, buffers, offsets);
}

void CommandBuffer::BindIndexBuffer(const Graphics::Buffer& gfxBuffer,
                                    uint32_t                offset,
                                    Format                  format)
{
  mCmdCount++; // Debug info
  auto indexBuffer = ConstGraphicsCast<Buffer, Graphics::Buffer>(&gfxBuffer);
  DALI_ASSERT_DEBUG(indexBuffer && indexBuffer->GetImpl());

  CommandBufferImpl* commandBufferImpl = GetImpl();
  commandBufferImpl->BindIndexBuffer(*indexBuffer->GetImpl(), offset, format);
}

void CommandBuffer::BindUniformBuffers(const std::vector<UniformBufferBinding>& bindings)
{
  mCmdCount++; // Debug info
  CommandBufferImpl* commandBufferImpl = GetImpl();
  commandBufferImpl->BindUniformBuffers(bindings);
}

void CommandBuffer::BindPipeline(const Graphics::Pipeline& pipeline)
{
  mCmdCount++; // Debug info
  CommandBufferImpl* commandBufferImpl = GetImpl();
  commandBufferImpl->BindPipeline(&pipeline);
}

void CommandBuffer::BindTextures(const std::vector<TextureBinding>& textureBindings)
{
  mCmdCount++; // Debug info
  CommandBufferImpl* commandBufferImpl = GetImpl();
  commandBufferImpl->BindTextures(textureBindings);

  mController.CheckTextureDependencies(textureBindings, mRenderTarget);
}

void CommandBuffer::BindSamplers(const std::vector<SamplerBinding>& samplerBindings)
{
  mCmdCount++; // Debug info
  CommandBufferImpl* commandBufferImpl = GetImpl();
  commandBufferImpl->BindSamplers(samplerBindings);
}

void CommandBuffer::BindPushConstants(void*    data,
                                      uint32_t size,
                                      uint32_t binding)
{
}

void CommandBuffer::BeginRenderPass(Graphics::RenderPass*          gfxRenderPass,
                                    Graphics::RenderTarget*        gfxRenderTarget,
                                    Rect2D                         renderArea,
                                    const std::vector<ClearValue>& clearValues)
{
  auto renderTarget = static_cast<Vulkan::RenderTarget*>(gfxRenderTarget);
  DALI_ASSERT_DEBUG(mRenderTarget == renderTarget && "RenderPass has different render target to cmd buffer Begin");

  auto             renderPass  = static_cast<Vulkan::RenderPass*>(gfxRenderPass);
  auto             surface     = mRenderTarget->GetSurface();
  auto&            device      = mController.GetGraphicsDevice();
  FramebufferImpl* framebuffer = nullptr;
  RenderPassHandle renderPassImpl;
  if(surface)
  {
    auto window    = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface);
    auto surfaceId = window->GetSurfaceId();
    auto swapchain = device.GetSwapchainForSurfaceId(surfaceId);
    framebuffer    = swapchain->GetCurrentFramebuffer();
    renderPassImpl = framebuffer->GetImplFromRenderPass(renderPass);
  }
  else
  {
    auto framebufferHandle = mRenderTarget->GetFramebuffer();
    framebuffer            = framebufferHandle->GetImpl();
    renderPassImpl         = framebuffer->GetImplFromRenderPass(renderPass);
    mController.AddTextureDependencies(renderTarget);
  }

  // Use standalone buffer to avoid memory allocation.
  static std::vector<vk::ClearValue> vkClearValues;

  uint32_t clearValuesCount = 0u;

  auto attachments = renderPass->GetCreateInfo().attachments;
  if(attachments != nullptr &&
     !attachments->empty()) // Can specify clear color even if load op is not clear.
  {
    clearValuesCount = static_cast<uint32_t>(clearValues.size());
    if(DALI_UNLIKELY(vkClearValues.size() < clearValuesCount))
    {
      vkClearValues.resize(clearValuesCount);
    }
    auto clearValuesIter    = clearValues.cbegin();
    auto clearValuesEndIter = clearValues.cend();
    for(auto vkClearValuesIter = vkClearValues.begin(); clearValuesIter != clearValuesEndIter;)
    {
      vk::ClearColorValue color;
      const auto&         clearValue = *(clearValuesIter++);

      color.float32[0] = clearValue.color.r;
      color.float32[1] = clearValue.color.g;
      color.float32[2] = clearValue.color.b;
      color.float32[3] = clearValue.color.a;

      *(vkClearValuesIter++) = color;
    }
  }

  mCmdCount++; // Debug info
  CommandBufferImpl* commandBufferImpl = GetImpl();
  commandBufferImpl->BeginRenderPass(vk::RenderPassBeginInfo{}
                                       .setFramebuffer(framebuffer->GetVkHandle())
                                       .setRenderPass(renderPassImpl->GetVkHandle())
                                       .setRenderArea({{0, 0}, {renderArea.width, renderArea.height}})
                                       .setPClearValues(vkClearValues.data())
                                       .setClearValueCount(clearValuesCount),
                                     vk::SubpassContents::eSecondaryCommandBuffers);
}

void CommandBuffer::EndRenderPass(Graphics::SyncObject* syncObject)
{
  mCmdCount++; // Debug info
  CommandBufferImpl* commandBufferImpl = GetImpl();
  commandBufferImpl->EndRenderPass();
}

void CommandBuffer::ReadPixels(uint8_t* buffer)
{
  mCmdCount++; // Debug info
  CommandBufferImpl* commandBufferImpl = GetImpl();
  commandBufferImpl->ReadPixels(buffer);
}

void CommandBuffer::ExecuteCommandBuffers(std::vector<const Graphics::CommandBuffer*>&& gfxCommandBuffers)
{
  std::vector<vk::CommandBuffer> vkCommandBuffers;
  vkCommandBuffers.reserve(gfxCommandBuffers.size());
  for(auto& gfxCmdBuf : gfxCommandBuffers)
  {
    vkCommandBuffers.emplace_back(ConstGraphicsCast<CommandBuffer, Graphics::CommandBuffer>(gfxCmdBuf)->GetImpl()->GetVkHandle());
  }
  mCmdCount++; // Debug info
  CommandBufferImpl* commandBufferImpl = GetImpl();
  commandBufferImpl->ExecuteCommandBuffers(vkCommandBuffers);
}

void CommandBuffer::Draw(uint32_t vertexCount,
                         uint32_t instanceCount,
                         uint32_t firstVertex,
                         uint32_t firstInstance)
{
  mCmdCount++; // Debug info
  CommandBufferImpl* commandBufferImpl = GetImpl();
  commandBufferImpl->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::DrawIndexed(uint32_t indexCount,
                                uint32_t instanceCount,
                                uint32_t firstIndex,
                                int32_t  vertexOffset,
                                uint32_t firstInstance)
{
  mCmdCount++; // Debug info
  CommandBufferImpl* commandBufferImpl = GetImpl();
  commandBufferImpl->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::DrawIndexedIndirect(Graphics::Buffer& gfxBuffer,
                                        uint32_t          offset,
                                        uint32_t          drawCount,
                                        uint32_t          stride)
{
  mCmdCount++; // Debug info
  auto               buffer            = ConstGraphicsCast<Buffer, Graphics::Buffer>(&gfxBuffer)->GetImpl();
  CommandBufferImpl* commandBufferImpl = GetImpl();
  commandBufferImpl->DrawIndexedIndirect(*buffer, offset, drawCount, stride);
}

void CommandBuffer::DrawNative(const DrawNativeInfo* drawInfo)
{
}

void CommandBuffer::SetScissor(Rect2D value)
{
  // @todo Vulkan accepts array of scissors... add to API
  Rect2D correctedValue = value;

  // First, invert the Y coord. But, only for surface, as we've flipped
  // viewport coordinate system for framebuffers, below, and only if it's
  // smaller than current height of render target (which implies it's been
  // set in Layer API to crop, rather than set to viewport if no clip, as
  // per render-algorithms)
  if(mRenderTarget && mRenderTarget->GetSurface() != nullptr)
  {
    // Origin is in GL coords (bottom left)
    auto height = mRenderTarget->GetCreateInfo().extent.height;
    if(correctedValue.height < height)
    {
      correctedValue.y = height - value.y - value.height;
    }
  }

  if(SetDynamicState(mDynamicState.scissor, correctedValue, DynamicStateMaskBits::SCISSOR))
  {
    mCmdCount++; // Debug info

    CommandBufferImpl* commandBufferImpl = GetImpl();
    commandBufferImpl->SetScissor(correctedValue);
  }
}

void CommandBuffer::SetScissorTestEnable(bool value)
{
  // Enabled by default. What does disabling test do?!
  // Probably should force pipeline to not use dynamic scissor state
}

void CommandBuffer::SetViewport(Viewport value)
{
  Viewport correctedValue = value;

  // "UnCorrect" framebuffer's viewport (it's wrong way up in GLES backend, so
  // API "InvertYAxis()" exists in DALI. But that's not used for surface, and we don't want to
  // change apps).
  auto surface = mRenderTarget->GetSurface();
  if(!surface)
  {
    correctedValue.height = -value.height;
    correctedValue.y      = value.height;
  }
  if(SetDynamicState(mDynamicState.viewport, correctedValue, DynamicStateMaskBits::VIEWPORT))
  {
    mCmdCount++; // Debug info
    CommandBufferImpl* commandBufferImpl = GetImpl();
    commandBufferImpl->SetViewport(correctedValue);
  }
}

void CommandBuffer::SetViewportEnable(bool value)
{
  // Enabled by default. What does disabling test do?!
  // Probably should force pipeline to not use dynamic viewport state
}

void CommandBuffer::SetColorMask(bool enabled)
{
}

void CommandBuffer::ClearStencilBuffer()
{
  // Ignore, we should only do this in render pass
}

void CommandBuffer::ClearDepthBuffer()
{
  // Ignore, we should only do this in render pass
}

void CommandBuffer::SetStencilTestEnable(bool stencilEnable)
{
  if(SetDynamicState(mDynamicState.stencilTest, stencilEnable, DynamicStateMaskBits::STENCIL_TEST))
  {
    mCmdCount++; // Debug info
    CommandBufferImpl* commandBufferImpl = GetImpl();
    commandBufferImpl->SetStencilTestEnable(stencilEnable);
  }
}

void CommandBuffer::SetStencilWriteMask(uint32_t writeMask)
{
  if(SetDynamicState(mDynamicState.stencilWriteMask, writeMask, DynamicStateMaskBits::STENCIL_WRITE_MASK))
  {
    mCmdCount++; // Debug info
    CommandBufferImpl* commandBufferImpl = GetImpl();
    commandBufferImpl->SetStencilWriteMask(vk::StencilFaceFlagBits::eFrontAndBack, writeMask);
  }
}

void CommandBuffer::SetStencilState(Graphics::CompareOp compareOp,
                                    uint32_t            reference,
                                    uint32_t            compareMask,
                                    Graphics::StencilOp failOp,
                                    Graphics::StencilOp passOp,
                                    Graphics::StencilOp depthFailOp)
{
  CommandBufferImpl* commandBufferImpl = GetImpl();

  if(SetDynamicState(mDynamicState.stencilCompareMask, compareMask, DynamicStateMaskBits::STENCIL_COMP_MASK))
  {
    mCmdCount++; // Debug info
    commandBufferImpl->SetStencilCompareMask(vk::StencilFaceFlagBits::eFrontAndBack, compareMask);
  }
  if(SetDynamicState(mDynamicState.stencilReference, reference, DynamicStateMaskBits::STENCIL_REF))
  {
    mCmdCount++; // Debug info
    commandBufferImpl->SetStencilReference(vk::StencilFaceFlagBits::eFrontAndBack, reference);
  }

  if(SetDynamicState(mDynamicState.stencilFailOp, failOp, DynamicStateMaskBits::STENCIL_OP_FAIL) ||
     SetDynamicState(mDynamicState.stencilPassOp, passOp, DynamicStateMaskBits::STENCIL_OP_PASS) ||
     SetDynamicState(mDynamicState.stencilDepthFailOp, depthFailOp, DynamicStateMaskBits::STENCIL_OP_DEPTH_FAIL) ||
     SetDynamicState(mDynamicState.stencilCompareOp, compareOp, DynamicStateMaskBits::STENCIL_OP_COMP))
  {
    mCmdCount++; // Debug info
    commandBufferImpl->SetStencilOp(vk::StencilFaceFlagBits::eFrontAndBack,
                                    VkStencilOpType(failOp).op,
                                    VkStencilOpType(passOp).op,
                                    VkStencilOpType(depthFailOp).op,
                                    VkCompareOpType(compareOp).op);
  }
}

void CommandBuffer::SetDepthCompareOp(Graphics::CompareOp compareOp)
{
  if(SetDynamicState(mDynamicState.depthCompareOp, compareOp, DynamicStateMaskBits::DEPTH_OP_COMP))
  {
    mCmdCount++; // Debug info
    CommandBufferImpl* commandBufferImpl = GetImpl();
    commandBufferImpl->SetDepthCompareOp(VkCompareOpType(compareOp).op);
  }
}

void CommandBuffer::SetDepthTestEnable(bool depthTestEnable)
{
  if(SetDynamicState(mDynamicState.depthTest, depthTestEnable, DynamicStateMaskBits::DEPTH_TEST))
  {
    mCmdCount++; // Debug info
    CommandBufferImpl* commandBufferImpl = GetImpl();
    commandBufferImpl->SetDepthTestEnable(depthTestEnable);
  }
}

void CommandBuffer::SetDepthWriteEnable(bool depthWriteEnable)
{
  if(SetDynamicState(mDynamicState.depthWrite, depthWriteEnable, DynamicStateMaskBits::DEPTH_WRITE))
  {
    mCmdCount++; // Debug info
    CommandBufferImpl* commandBufferImpl = GetImpl();
    commandBufferImpl->SetDepthWriteEnable(depthWriteEnable);
  }
}

Vulkan::RenderTarget* CommandBuffer::GetRenderTarget() const
{
  // Gets the render target from the Begin() cmd.
  return mRenderTarget;
}

[[nodiscard]] Vulkan::CommandBufferImpl* CommandBuffer::GetImpl() const
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size());
  return mCommandBufferImpl[bufferIndex];
}

void CommandBuffer::AllocateCommandBuffers()
{
  auto& device    = mController.GetGraphicsDevice();
  bool  isPrimary = true;
  if(mCreateInfo.level == Graphics::CommandBufferLevel::SECONDARY)
  {
    isPrimary = false;
  }
  auto commandPool = device.GetCommandPool(std::this_thread::get_id());
  auto bufferCount = device.GetBufferCount();

  DALI_LOG_INFO(gVulkanFilter, Debug::General, "Allocating %d new cmd buffers\n", bufferCount - mCommandBufferImpl.size());

  for(uint32_t i = mCommandBufferImpl.size(); i < bufferCount; ++i)
  {
    mCommandBufferImpl.emplace_back(commandPool->NewCommandBuffer(isPrimary));
  }
}

} // namespace Dali::Graphics::Vulkan
