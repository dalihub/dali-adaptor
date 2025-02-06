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
Debug::Filter* gLogCmdBufferFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VK_COMMAND_BUFFER");
#endif

namespace Dali::Graphics::Vulkan
{
template<typename VT, typename GT>
VT* ConstGraphicsCast(const GT* object)
{
  return const_cast<VT*>(static_cast<const VT*>(object));
}

CommandBuffer::CommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, VulkanGraphicsController& controller)
: CommandBufferResource(createInfo, controller),
  mDynamicStateMask(CommandBuffer::INITIAL_DYNAMIC_MASK_VALUE)
{
  auto& device    = controller.GetGraphicsDevice();
  bool  isPrimary = true;
  if(createInfo.level == Graphics::CommandBufferLevel::SECONDARY)
  {
    isPrimary = false;
  }
  auto commandPool = device.GetCommandPool(std::this_thread::get_id());
  for(uint32_t i = 0; i < device.GetBufferCount(); ++i)
  {
    mCommandBufferImpl.emplace_back(commandPool->NewCommandBuffer(isPrimary));
  }
}

CommandBuffer::~CommandBuffer() = default;

void CommandBuffer::DestroyResource()
{
  // Don't delete the impl, it's pool allocated and should have been
  // returned to the command pool for re-use.
  mCommandBufferImpl.clear();
}

bool CommandBuffer::InitializeResource()
{
  return true;
}

void CommandBuffer::DiscardResource()
{
  mController.DiscardResource(this);
}

void CommandBuffer::Begin(const Graphics::CommandBufferBeginInfo& info)
{
  mDynamicStateMask    = CommandBuffer::INITIAL_DYNAMIC_MASK_VALUE;
  mRenderTarget        = ConstGraphicsCast<Vulkan::RenderTarget, Graphics::RenderTarget>(info.renderTarget);
  uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size());

  if(mCommandBufferImpl[bufferIndex])
  {
    DALI_LOG_INFO(gLogCmdBufferFilter, Debug::Verbose, "CommandBuffer::Begin: ptr:%p bufferIndex=%d", this, bufferIndex);
    vk::CommandBufferInheritanceInfo inheritanceInfo{};
    if(info.renderPass)
    {
      inheritanceInfo.renderPass         = mRenderTarget->GetRenderPass(info.renderPass)->GetVkHandle();
      inheritanceInfo.subpass            = 0;
      inheritanceInfo.framebuffer        = mRenderTarget->GetCurrentFramebufferImpl()->GetVkHandle();
      inheritanceInfo.queryFlags         = static_cast<vk::QueryControlFlags>(0);
      inheritanceInfo.pipelineStatistics = static_cast<vk::QueryPipelineStatisticFlags>(0);
    }
    mCommandBufferImpl[bufferIndex]->Begin(static_cast<vk::CommandBufferUsageFlags>(info.usage), &inheritanceInfo);

    // Default depth/stencil should be off:
    SetDepthTestEnable(false);
    SetDepthWriteEnable(false);
    SetDepthCompareOp(Graphics::CompareOp::LESS);
    SetStencilTestEnable(false);
  }
}

void CommandBuffer::End()
{
  uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);
  mCommandBufferImpl[bufferIndex]->End();
}

void CommandBuffer::Reset()
{
  uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_LOG_INFO(gLogCmdBufferFilter, Debug::Verbose, "CommandBuffer::Reset: ptr:%p bufferIndex=%d", this, bufferIndex);

  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);
  mCommandBufferImpl[bufferIndex]->Reset();

  mDynamicStateMask = CommandBuffer::INITIAL_DYNAMIC_MASK_VALUE;
  mRenderTarget     = nullptr;
}

void CommandBuffer::BindVertexBuffers(uint32_t                                    firstBinding,
                                      const std::vector<const Graphics::Buffer*>& gfxBuffers,
                                      const std::vector<uint32_t>&                offsets)
{
  uint32_t                 bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  std::vector<BufferImpl*> buffers;
  buffers.reserve(gfxBuffers.size());
  for(auto& gfxBuffer : gfxBuffers)
  {
    buffers.push_back(ConstGraphicsCast<Buffer, Graphics::Buffer>(gfxBuffer)->GetImpl());
  }
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);
  mCommandBufferImpl[bufferIndex]->BindVertexBuffers(firstBinding, buffers, offsets);
}

void CommandBuffer::BindIndexBuffer(const Graphics::Buffer& gfxBuffer,
                                    uint32_t                offset,
                                    Format                  format)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  auto indexBuffer = ConstGraphicsCast<Buffer, Graphics::Buffer>(&gfxBuffer);
  DALI_ASSERT_DEBUG(indexBuffer && indexBuffer->GetImpl());

  mCommandBufferImpl[bufferIndex]->BindIndexBuffer(*indexBuffer->GetImpl(), offset, format);
}

void CommandBuffer::BindUniformBuffers(const std::vector<UniformBufferBinding>& bindings)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();

  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  mCommandBufferImpl[bufferIndex]->BindUniformBuffers(bindings);
}

void CommandBuffer::BindPipeline(const Graphics::Pipeline& pipeline)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  mCommandBufferImpl[bufferIndex]->BindPipeline(&pipeline);
}

void CommandBuffer::BindTextures(const std::vector<TextureBinding>& textureBindings)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  mCommandBufferImpl[bufferIndex]->BindTextures(textureBindings);

  mController.CheckTextureDependencies(textureBindings, mRenderTarget);
}

void CommandBuffer::BindSamplers(const std::vector<SamplerBinding>& samplerBindings)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  mCommandBufferImpl[bufferIndex]->BindSamplers(samplerBindings);
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
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();

  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

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

  std::vector<vk::ClearValue> vkClearValues;

  auto attachments = renderPass->GetCreateInfo().attachments;
  if(attachments != nullptr &&
     !attachments->empty()) // Can specify clear color even if load op is not clear.
  {
    for(auto clearValue : clearValues)
    {
      vk::ClearColorValue color;
      color.float32[0] = clearValue.color.r;
      color.float32[1] = clearValue.color.g;
      color.float32[2] = clearValue.color.b;
      color.float32[3] = clearValue.color.a;
      vkClearValues.emplace_back(color);
    }
  }

  mCommandBufferImpl[bufferIndex]->BeginRenderPass(vk::RenderPassBeginInfo{}
                                                     .setFramebuffer(framebuffer->GetVkHandle())
                                                     .setRenderPass(renderPassImpl->GetVkHandle())
                                                     .setRenderArea({{0, 0}, {renderArea.width, renderArea.height}})
                                                     .setPClearValues(vkClearValues.data())
                                                     .setClearValueCount(uint32_t(vkClearValues.size())),
                                                   vk::SubpassContents::eSecondaryCommandBuffers);
}

void CommandBuffer::EndRenderPass(Graphics::SyncObject* syncObject)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  mCommandBufferImpl[bufferIndex]->EndRenderPass();
}

void CommandBuffer::ReadPixels(uint8_t* buffer)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();

  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  mCommandBufferImpl[bufferIndex]->ReadPixels(buffer);
}

void CommandBuffer::ExecuteCommandBuffers(std::vector<const Graphics::CommandBuffer*>&& gfxCommandBuffers)
{
  uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  std::vector<vk::CommandBuffer> vkCommandBuffers;
  vkCommandBuffers.reserve(gfxCommandBuffers.size());
  for(auto& gfxCmdBuf : gfxCommandBuffers)
  {
    vkCommandBuffers.push_back(ConstGraphicsCast<CommandBuffer, Graphics::CommandBuffer>(gfxCmdBuf)->GetImpl()->GetVkHandle());
  }
  mCommandBufferImpl[bufferIndex]->ExecuteCommandBuffers(vkCommandBuffers);
}

void CommandBuffer::Draw(uint32_t vertexCount,
                         uint32_t instanceCount,
                         uint32_t firstVertex,
                         uint32_t firstInstance)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  mCommandBufferImpl[bufferIndex]->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::DrawIndexed(uint32_t indexCount,
                                uint32_t instanceCount,
                                uint32_t firstIndex,
                                int32_t  vertexOffset,
                                uint32_t firstInstance)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  mCommandBufferImpl[bufferIndex]->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::DrawIndexedIndirect(Graphics::Buffer& gfxBuffer,
                                        uint32_t          offset,
                                        uint32_t          drawCount,
                                        uint32_t          stride)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  auto buffer = ConstGraphicsCast<Buffer, Graphics::Buffer>(&gfxBuffer)->GetImpl();
  mCommandBufferImpl[bufferIndex]->DrawIndexedIndirect(*buffer, offset, drawCount, stride);
}

void CommandBuffer::DrawNative(const DrawNativeInfo* drawInfo)
{
}

void CommandBuffer::SetScissor(Rect2D value)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  // @todo Vulkan accepts array of scissors... add to API

  if(SetDynamicState(mDynamicState.scissor, value, DynamicStateMaskBits::SCISSOR))
  {
    mCommandBufferImpl[bufferIndex]->SetScissor(value);
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
  auto     surface        = mRenderTarget->GetSurface();
  if(!surface)
  {
    correctedValue.height = -value.height;
    correctedValue.y      = value.height;
  }

  uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  if(SetDynamicState(mDynamicState.viewport, correctedValue, DynamicStateMaskBits::VIEWPORT))
  {
    mCommandBufferImpl[bufferIndex]->SetViewport(correctedValue);
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
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  if(SetDynamicState(mDynamicState.stencilTest, stencilEnable, DynamicStateMaskBits::STENCIL_TEST))
  {
    mCommandBufferImpl[bufferIndex]->SetStencilTestEnable(stencilEnable);
  }
}

void CommandBuffer::SetStencilWriteMask(uint32_t writeMask)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  if(SetDynamicState(mDynamicState.stencilWriteMask, writeMask, DynamicStateMaskBits::STENCIL_WRITE_MASK))
  {
    mCommandBufferImpl[bufferIndex]->SetStencilWriteMask(vk::StencilFaceFlagBits::eFrontAndBack, writeMask);
  }
}

void CommandBuffer::SetStencilState(Graphics::CompareOp compareOp,
                                    uint32_t            reference,
                                    uint32_t            compareMask,
                                    Graphics::StencilOp failOp,
                                    Graphics::StencilOp passOp,
                                    Graphics::StencilOp depthFailOp)
{
  uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);

  if(SetDynamicState(mDynamicState.stencilCompareMask, compareMask, DynamicStateMaskBits::STENCIL_COMP_MASK))
  {
    mCommandBufferImpl[bufferIndex]->SetStencilCompareMask(vk::StencilFaceFlagBits::eFrontAndBack, compareMask);
  }
  if(SetDynamicState(mDynamicState.stencilReference, reference, DynamicStateMaskBits::STENCIL_REF))
  {
    mCommandBufferImpl[bufferIndex]->SetStencilReference(vk::StencilFaceFlagBits::eFrontAndBack, reference);
  }

  if(SetDynamicState(mDynamicState.stencilFailOp, failOp, DynamicStateMaskBits::STENCIL_OP_FAIL) ||
     SetDynamicState(mDynamicState.stencilPassOp, passOp, DynamicStateMaskBits::STENCIL_OP_PASS) ||
     SetDynamicState(mDynamicState.stencilDepthFailOp, depthFailOp, DynamicStateMaskBits::STENCIL_OP_DEPTH_FAIL) ||
     SetDynamicState(mDynamicState.stencilCompareOp, compareOp, DynamicStateMaskBits::STENCIL_OP_COMP))
  {
    mCommandBufferImpl[bufferIndex]->SetStencilOp(vk::StencilFaceFlagBits::eFrontAndBack,
                                                  VkStencilOpType(failOp).op,
                                                  VkStencilOpType(passOp).op,
                                                  VkStencilOpType(depthFailOp).op,
                                                  VkCompareOpType(compareOp).op);
  }
}

void CommandBuffer::SetDepthCompareOp(Graphics::CompareOp compareOp)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);
  if(SetDynamicState(mDynamicState.depthCompareOp, compareOp, DynamicStateMaskBits::DEPTH_OP_COMP))
  {
    mCommandBufferImpl[bufferIndex]->SetDepthCompareOp(VkCompareOpType(compareOp).op);
  }
}

void CommandBuffer::SetDepthTestEnable(bool depthTestEnable)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);
  if(SetDynamicState(mDynamicState.depthTest, depthTestEnable, DynamicStateMaskBits::DEPTH_TEST))
  {
    mCommandBufferImpl[bufferIndex]->SetDepthTestEnable(depthTestEnable);
  }
}

void CommandBuffer::SetDepthWriteEnable(bool depthWriteEnable)
{
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);
  if(SetDynamicState(mDynamicState.depthWrite, depthWriteEnable, DynamicStateMaskBits::DEPTH_WRITE))
  {
    mCommandBufferImpl[bufferIndex]->SetDepthWriteEnable(depthWriteEnable);
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
  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size() && mCommandBufferImpl[bufferIndex]);
  return mCommandBufferImpl[bufferIndex];
}

} // namespace Dali::Graphics::Vulkan
