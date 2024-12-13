/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
  mDynamicStateMask(CommandBuffer::INITIAL_DYNAMIC_MASK_VALUE),
  mCommandBufferImpl(nullptr)
{
  auto& device    = controller.GetGraphicsDevice();
  bool  isPrimary = true;
  if(createInfo.level == Graphics::CommandBufferLevel::SECONDARY)
  {
    isPrimary = false;
  }
  auto commandPool   = device.GetCommandPool(std::this_thread::get_id());
  mCommandBufferImpl = commandPool->NewCommandBuffer(isPrimary);
}

CommandBuffer::~CommandBuffer() = default;

void CommandBuffer::DestroyResource()
{
  // Don't delete the impl, it's pool allocated and should have been
  // returned to the command pool for re-use.
  mCommandBufferImpl = nullptr;
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
  mDynamicStateMask = CommandBuffer::INITIAL_DYNAMIC_MASK_VALUE;
  if(mCommandBufferImpl)
  {
    vk::CommandBufferInheritanceInfo inheritanceInfo{};
    if(info.renderPass)
    {
      auto renderTarget                  = ConstGraphicsCast<Vulkan::RenderTarget, Graphics::RenderTarget>(info.renderTarget);
      inheritanceInfo.renderPass         = renderTarget->GetRenderPass(info.renderPass)->GetVkHandle();
      inheritanceInfo.subpass            = 0;
      inheritanceInfo.framebuffer        = renderTarget->GetCurrentFramebufferImpl()->GetVkHandle();
      inheritanceInfo.queryFlags         = static_cast<vk::QueryControlFlags>(0);
      inheritanceInfo.pipelineStatistics = static_cast<vk::QueryPipelineStatisticFlags>(0);
    }
    mCommandBufferImpl->Begin(static_cast<vk::CommandBufferUsageFlags>(info.usage), &inheritanceInfo);

    // Default depth/stencil should be off:
    SetDepthTestEnable(false);
    SetDepthWriteEnable(false);
    SetDepthCompareOp(Graphics::CompareOp::LESS);
    SetStencilTestEnable(false);
  }
}

void CommandBuffer::End()
{
  if(mCommandBufferImpl)
  {
    mCommandBufferImpl->End();
  }
}

void CommandBuffer::Reset()
{
  if(mCommandBufferImpl)
  {
    mCommandBufferImpl->Reset();
  }
  mDynamicStateMask = CommandBuffer::INITIAL_DYNAMIC_MASK_VALUE;
  mLastSwapchain    = nullptr;
}

void CommandBuffer::BindVertexBuffers(uint32_t                                    firstBinding,
                                      const std::vector<const Graphics::Buffer*>& gfxBuffers,
                                      const std::vector<uint32_t>&                offsets)
{
  std::vector<BufferImpl*> buffers;
  buffers.reserve(gfxBuffers.size());
  for(auto& gfxBuffer : gfxBuffers)
  {
    buffers.push_back(ConstGraphicsCast<Buffer, Graphics::Buffer>(gfxBuffer)->GetImpl());
  }
  mCommandBufferImpl->BindVertexBuffers(firstBinding, buffers, offsets);
}

void CommandBuffer::BindIndexBuffer(const Graphics::Buffer& gfxBuffer,
                                    uint32_t                offset,
                                    Format                  format)
{
  auto indexBuffer = ConstGraphicsCast<Buffer, Graphics::Buffer>(&gfxBuffer);
  DALI_ASSERT_DEBUG(indexBuffer && indexBuffer->GetImpl());
  mCommandBufferImpl->BindIndexBuffer(*indexBuffer->GetImpl(), offset, format);
}

void CommandBuffer::BindUniformBuffers(const std::vector<UniformBufferBinding>& bindings)
{
  mCommandBufferImpl->BindUniformBuffers(bindings);
}

void CommandBuffer::BindPipeline(const Graphics::Pipeline& pipeline)
{
  mCommandBufferImpl->BindPipeline(&pipeline);
}

void CommandBuffer::BindTextures(const std::vector<TextureBinding>& textureBindings)
{
  mCommandBufferImpl->BindTextures(textureBindings);
}

void CommandBuffer::BindSamplers(const std::vector<SamplerBinding>& samplerBindings)
{
  mCommandBufferImpl->BindSamplers(samplerBindings);
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
  auto             renderPass   = static_cast<Vulkan::RenderPass*>(gfxRenderPass);
  auto             renderTarget = static_cast<Vulkan::RenderTarget*>(gfxRenderTarget);
  auto             surface      = renderTarget->GetSurface();
  auto&            device       = mController.GetGraphicsDevice();
  FramebufferImpl* framebuffer;
  RenderPassHandle renderPassImpl;
  if(surface)
  {
    auto window    = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface);
    auto surfaceId = window->GetSurfaceId();
    auto swapchain = device.GetSwapchainForSurfaceId(surfaceId);

    // If we have swapchain then we need to acquire image
    // This is a special case:
    // We assume that:
    // - only one BeginRenderPass() happens per surface so we can acquire image here
    // - swapchain shouldn't change but in case it does hence the condition below (?)
    if(mLastSwapchain != swapchain)
    {
      mLastSwapchain = swapchain;
    }

    if(mLastSwapchain)
    {
      framebuffer = mLastSwapchain->AcquireNextFramebuffer(true);
    }

    // In case something went wrong we will try to replace swapchain once
    // before calling it a day.
    if(!framebuffer || !swapchain->IsValid())
    {
      // make sure device doesn't do any work before replacing swapchain
      device.DeviceWaitIdle();

      // replace swapchain (only once)
      swapchain = device.ReplaceSwapchainForSurface(swapchain->GetSurface(), std::move(swapchain));

      mLastSwapchain = swapchain;

      // get new valid framebuffer
      if(mLastSwapchain)
      {
        framebuffer = swapchain->AcquireNextFramebuffer(true);
      }
      assert(framebuffer && "Replacing invalid swapchain unsuccessful! Goodbye!");
    }

    renderPassImpl = framebuffer->GetImplFromRenderPass(renderPass);
  }
  else
  {
    auto coreFramebuffer = renderTarget->GetFramebuffer();
    framebuffer          = coreFramebuffer->GetImpl();
    renderPassImpl       = framebuffer->GetImplFromRenderPass(renderPass);
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

  mCommandBufferImpl->BeginRenderPass(vk::RenderPassBeginInfo{}
                                        .setFramebuffer(framebuffer->GetVkHandle())
                                        .setRenderPass(renderPassImpl->GetVkHandle())
                                        .setRenderArea({{0, 0}, {renderArea.width, renderArea.height}})
                                        .setPClearValues(vkClearValues.data())
                                        .setClearValueCount(uint32_t(vkClearValues.size())),
                                      vk::SubpassContents::eSecondaryCommandBuffers);
}

void CommandBuffer::EndRenderPass(Graphics::SyncObject* syncObject)
{
  mCommandBufferImpl->EndRenderPass();
}

void CommandBuffer::ReadPixels(uint8_t* buffer)
{
  mCommandBufferImpl->ReadPixels(buffer);
}

void CommandBuffer::ExecuteCommandBuffers(std::vector<const Graphics::CommandBuffer*>&& gfxCommandBuffers)
{
  std::vector<vk::CommandBuffer> vkCommandBuffers;
  vkCommandBuffers.reserve(gfxCommandBuffers.size());
  for(auto& gfxCmdBuf : gfxCommandBuffers)
  {
    vkCommandBuffers.push_back(ConstGraphicsCast<CommandBuffer, Graphics::CommandBuffer>(gfxCmdBuf)->GetImpl()->GetVkHandle());
  }
  mCommandBufferImpl->ExecuteCommandBuffers(vkCommandBuffers);
}

void CommandBuffer::Draw(uint32_t vertexCount,
                         uint32_t instanceCount,
                         uint32_t firstVertex,
                         uint32_t firstInstance)
{
  mCommandBufferImpl->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::DrawIndexed(uint32_t indexCount,
                                uint32_t instanceCount,
                                uint32_t firstIndex,
                                int32_t  vertexOffset,
                                uint32_t firstInstance)
{
  mCommandBufferImpl->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::DrawIndexedIndirect(Graphics::Buffer& gfxBuffer,
                                        uint32_t          offset,
                                        uint32_t          drawCount,
                                        uint32_t          stride)
{
  auto buffer = ConstGraphicsCast<Buffer, Graphics::Buffer>(&gfxBuffer)->GetImpl();

  mCommandBufferImpl->DrawIndexedIndirect(*buffer, offset, drawCount, stride);
}

void CommandBuffer::DrawNative(const DrawNativeInfo* drawInfo)
{
}

void CommandBuffer::SetScissor(Rect2D value)
{
  // @todo Vulkan accepts array of scissors... add to API

  if(SetDynamicState(mDynamicState.scissor, value, DynamicStateMaskBits::SCISSOR))
  {
    mCommandBufferImpl->SetScissor(value);
  }
}

void CommandBuffer::SetScissorTestEnable(bool value)
{
  // Enabled by default. What does disabling test do?!
  // Probably should force pipeline to not use dynamic scissor state
}

void CommandBuffer::SetViewport(Viewport value)
{
  if(SetDynamicState(mDynamicState.viewport, value, DynamicStateMaskBits::VIEWPORT))
  {
    mCommandBufferImpl->SetViewport(value);
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
    mCommandBufferImpl->SetStencilTestEnable(stencilEnable);
  }
}

void CommandBuffer::SetStencilWriteMask(uint32_t writeMask)
{
  if(SetDynamicState(mDynamicState.stencilWriteMask, writeMask, DynamicStateMaskBits::STENCIL_WRITE_MASK))
  {
    mCommandBufferImpl->SetStencilWriteMask(vk::StencilFaceFlagBits::eFrontAndBack, writeMask);
  }
}

void CommandBuffer::SetStencilState(Graphics::CompareOp compareOp,
                                    uint32_t            reference,
                                    uint32_t            compareMask,
                                    Graphics::StencilOp failOp,
                                    Graphics::StencilOp passOp,
                                    Graphics::StencilOp depthFailOp)
{
  if(SetDynamicState(mDynamicState.stencilCompareMask, compareMask, DynamicStateMaskBits::STENCIL_COMP_MASK))
  {
    mCommandBufferImpl->SetStencilCompareMask(vk::StencilFaceFlagBits::eFrontAndBack, compareMask);
  }
  if(SetDynamicState(mDynamicState.stencilReference, reference, DynamicStateMaskBits::STENCIL_REF))
  {
    mCommandBufferImpl->SetStencilReference(vk::StencilFaceFlagBits::eFrontAndBack, reference);
  }

  if(SetDynamicState(mDynamicState.stencilFailOp, failOp, DynamicStateMaskBits::STENCIL_OP_FAIL) ||
     SetDynamicState(mDynamicState.stencilPassOp, passOp, DynamicStateMaskBits::STENCIL_OP_PASS) ||
     SetDynamicState(mDynamicState.stencilDepthFailOp, depthFailOp, DynamicStateMaskBits::STENCIL_OP_DEPTH_FAIL) ||
     SetDynamicState(mDynamicState.stencilCompareOp, compareOp, DynamicStateMaskBits::STENCIL_OP_COMP))
  {
    mCommandBufferImpl->SetStencilOp(vk::StencilFaceFlagBits::eFrontAndBack,
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
    mCommandBufferImpl->SetDepthCompareOp(VkCompareOpType(compareOp).op);
  }
}

void CommandBuffer::SetDepthTestEnable(bool depthTestEnable)
{
  if(SetDynamicState(mDynamicState.depthTest, depthTestEnable, DynamicStateMaskBits::DEPTH_TEST))
  {
    mCommandBufferImpl->SetDepthTestEnable(depthTestEnable);
  }
}

void CommandBuffer::SetDepthWriteEnable(bool depthWriteEnable)
{
  if(SetDynamicState(mDynamicState.depthWrite, depthWriteEnable, DynamicStateMaskBits::DEPTH_WRITE))
  {
    mCommandBufferImpl->SetDepthWriteEnable(depthWriteEnable);
  }
}

Swapchain* CommandBuffer::GetLastSwapchain() const
{
  return mLastSwapchain;
}

} // namespace Dali::Graphics::Vulkan
