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

#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-pool-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/internal/window-system/common/window-render-surface.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-impl.h>

namespace Dali::Graphics::Vulkan
{
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
}

bool CommandBuffer::InitializeResource()
{
  return true;
}

void CommandBuffer::DiscardResource()
{
}

void CommandBuffer::Begin(const Graphics::CommandBufferBeginInfo& info)
{
  if(mCommandBufferImpl)
  {
    mCommandBufferImpl->Begin(static_cast<vk::CommandBufferUsageFlags>(info.usage), nullptr);
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
                                      const std::vector<const Graphics::Buffer*>& buffers,
                                      const std::vector<uint32_t>&                offsets)
{
}

void CommandBuffer::BindUniformBuffers(const std::vector<UniformBufferBinding>& bindings)
{
}

void CommandBuffer::BindPipeline(const Graphics::Pipeline& pipeline)
{
}

void CommandBuffer::BindTextures(const std::vector<TextureBinding>& textureBindings)
{
}

void CommandBuffer::BindSamplers(const std::vector<SamplerBinding>& samplerBindings)
{
}

void CommandBuffer::BindPushConstants(void*    data,
                                      uint32_t size,
                                      uint32_t binding)
{
}

void CommandBuffer::BindIndexBuffer(const Graphics::Buffer& buffer,
                                    uint32_t                offset,
                                    Format                  format)
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
  RenderPassImpl*  renderPassImpl;
  if(surface)
  {
    auto window    = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface);
    auto surfaceId = window->GetSurfaceId();
    auto swapchain = device.GetSwapchainForSurfaceId(surfaceId);
    mLastSwapchain = swapchain;
    framebuffer    = swapchain->GetCurrentFramebuffer();
    renderPassImpl = framebuffer->GetRenderPass(renderPass);
  }
  else
  {
    auto coreFramebuffer = renderTarget->GetFramebuffer();
    framebuffer          = coreFramebuffer->GetImpl();
    renderPassImpl       = framebuffer->GetRenderPass(renderPass);
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
                                      vk::SubpassContents::eInline);
}

void CommandBuffer::EndRenderPass(Graphics::SyncObject* syncObject)
{
  mCommandBufferImpl->EndRenderPass();
}

void CommandBuffer::ExecuteCommandBuffers(std::vector<const Graphics::CommandBuffer*>&& commandBuffers)
{
}

void CommandBuffer::Draw(uint32_t vertexCount,
                         uint32_t instanceCount,
                         uint32_t firstVertex,
                         uint32_t firstInstance)
{
}

void CommandBuffer::DrawIndexed(uint32_t indexCount,
                                uint32_t instanceCount,
                                uint32_t firstIndex,
                                int32_t  vertexOffset,
                                uint32_t firstInstance)
{
}

void CommandBuffer::DrawIndexedIndirect(Graphics::Buffer& buffer,
                                        uint32_t          offset,
                                        uint32_t          drawCount,
                                        uint32_t          stride)
{
}

void CommandBuffer::DrawNative(const DrawNativeInfo* drawInfo)
{
}

void CommandBuffer::SetScissor(Rect2D value)
{
}

void CommandBuffer::SetScissorTestEnable(bool value)
{
}

void CommandBuffer::SetViewport(Viewport value)
{
}

void CommandBuffer::SetViewportEnable(bool value)
{
}

void CommandBuffer::SetColorMask(bool enabled)
{
}

void CommandBuffer::ClearStencilBuffer()
{
}

void CommandBuffer::ClearDepthBuffer()
{
}

void CommandBuffer::SetStencilTestEnable(bool stencilEnable)
{
}

void CommandBuffer::SetStencilWriteMask(uint32_t writeMask)
{
}

void CommandBuffer::SetStencilFunc(Graphics::CompareOp compareOp,
                                   uint32_t            reference,
                                   uint32_t            compareMask)
{
}

void CommandBuffer::SetStencilOp(Graphics::StencilOp failOp,
                                 Graphics::StencilOp passOp,
                                 Graphics::StencilOp depthFailOp)
{
}

void CommandBuffer::SetDepthCompareOp(Graphics::CompareOp compareOp)
{
}

void CommandBuffer::SetDepthTestEnable(bool depthTestEnable)
{
}

void CommandBuffer::SetDepthWriteEnable(bool depthWriteEnable)
{
}

Swapchain* CommandBuffer::GetLastSwapchain() const
{
  return mLastSwapchain;
}

} // namespace Dali::Graphics::Vulkan
