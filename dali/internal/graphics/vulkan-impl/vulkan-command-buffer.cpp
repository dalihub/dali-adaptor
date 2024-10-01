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
  if(mCommandBufferImpl)
  {
    // Check if there is some extra information about used resources
    // if so then apply optimizations
    if(info.resourceBindings)
    {
      // update programs with descriptor pools
      for(auto& binding : *info.resourceBindings)
      {
        if(binding.type == ResourceType::PROGRAM)
        {
          auto programImpl = static_cast<Vulkan::Program*>(binding.programBinding->program)->GetImplementation();

          // Pool index is returned and we may do something with it later (storing it per cmdbuf?)
          [[maybe_unused]] auto poolIndex = programImpl->AddDescriptorPool(binding.programBinding->count, 3); // add new pool, limit pools to 3 per program
        }
      }
    }

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
  mLastSwapchain = nullptr;
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
    mLastSwapchain = swapchain;
    framebuffer    = swapchain->GetCurrentFramebuffer();
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
     !attachments->empty() &&
     attachments->front().loadOp == Graphics::AttachmentLoadOp::CLEAR)
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
                                        .setClearValueCount(uint32_t(framebuffer->GetClearValues().size())),
                                      vk::SubpassContents::eSecondaryCommandBuffers);
}

void CommandBuffer::EndRenderPass(Graphics::SyncObject* syncObject)
{
  mCommandBufferImpl->EndRenderPass();
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
  mCommandBufferImpl->SetScissor(value);
}

void CommandBuffer::SetScissorTestEnable(bool value)
{
  // Enabled by default. What does disabling test do?!
  // Probably should force pipeline to not use dynamic scissor state
}

void CommandBuffer::SetViewport(Viewport value)
{
  mCommandBufferImpl->SetViewport(value);
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
  mCommandBufferImpl->SetStencilTestEnable(stencilEnable);
}

void CommandBuffer::SetStencilWriteMask(uint32_t writeMask)
{
  mCommandBufferImpl->SetStencilWriteMask(vk::StencilFaceFlagBits::eFrontAndBack, writeMask);
}

void CommandBuffer::SetStencilState(Graphics::CompareOp compareOp,
                                    uint32_t            reference,
                                    uint32_t            compareMask,
                                    Graphics::StencilOp failOp,
                                    Graphics::StencilOp passOp,
                                    Graphics::StencilOp depthFailOp)
{
  mCommandBufferImpl->SetStencilCompareMask(vk::StencilFaceFlagBits::eFrontAndBack, compareMask);
  mCommandBufferImpl->SetStencilReference(vk::StencilFaceFlagBits::eFrontAndBack, reference);
  mCommandBufferImpl->SetStencilOp(vk::StencilFaceFlagBits::eFrontAndBack,
                                   VkStencilOpType(failOp).op,
                                   VkStencilOpType(passOp).op,
                                   VkStencilOpType(depthFailOp).op,
                                   VkCompareOpType(compareOp).op);
}

void CommandBuffer::SetDepthCompareOp(Graphics::CompareOp compareOp)
{
  // @todo Invert comparison
  // This makes depth test work, but implies that the conversion to NDC has the wrong sense.
  vk::CompareOp depthOp;
  switch(compareOp)
  {
    case Graphics::CompareOp::NEVER:
    case Graphics::CompareOp::EQUAL:
    case Graphics::CompareOp::NOT_EQUAL:
    case Graphics::CompareOp::ALWAYS:
    {
      depthOp = VkCompareOpType(compareOp).op;
      break;
    }
    case Graphics::CompareOp::LESS:
    {
      depthOp = vk::CompareOp::eGreaterOrEqual;
      break;
    }
    case Graphics::CompareOp::LESS_OR_EQUAL:
    {
      depthOp = vk::CompareOp::eGreater;
      break;
    }
    case Graphics::CompareOp::GREATER:
    {
      depthOp = vk::CompareOp::eLessOrEqual;
      break;
    }
    case Graphics::CompareOp::GREATER_OR_EQUAL:
    {
      depthOp = vk::CompareOp::eLess;
      break;
    }
  }
  depthOp = VkCompareOpType(compareOp).op;
  mCommandBufferImpl->SetDepthCompareOp(depthOp);
}

void CommandBuffer::SetDepthTestEnable(bool depthTestEnable)
{
  mCommandBufferImpl->SetDepthTestEnable(depthTestEnable);
}

void CommandBuffer::SetDepthWriteEnable(bool depthWriteEnable)
{
  mCommandBufferImpl->SetDepthWriteEnable(depthWriteEnable);
}

Swapchain* CommandBuffer::GetLastSwapchain() const
{
  return mLastSwapchain;
}

} // namespace Dali::Graphics::Vulkan
