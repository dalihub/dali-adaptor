/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-executor.h>

// INTERNAL HEADERS
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-pool-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-pipeline.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-program-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-target.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-stored-command-buffer.h>

#include <dali/internal/graphics/vulkan/vulkan-device.h>

#include <dali/integration-api/debug.h>
#include <dali/internal/window-system/common/window-render-surface.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gLogCmdBufferFilter;
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{
template<typename VT, typename GT>
VT* ConstGraphicsCast(const GT* object)
{
  return const_cast<VT*>(static_cast<const VT*>(object));
}

CommandBufferExecutor::CommandBufferExecutor(VulkanGraphicsController& controller)
: mController(controller)
{
}

CommandBufferExecutor::~CommandBufferExecutor() = default;

void CommandBufferExecutor::ProcessCommandBuffer(StoredCommandBuffer* storedCommandBuffer,
                                                 CommandBufferImpl*   commandBufferImpl)
{
  auto count    = 0u;
  auto commands = storedCommandBuffer->GetCommands(count);

  for(uint32_t i = 0u; i < count; ++i)
  {
    auto& cmd = commands[i];
    switch(cmd.type)
    {
      case Vulkan::CommandType::NULL_COMMAND:
      {
        break;
      }
      case Vulkan::CommandType::BEGIN:
      {
        Begin(commandBufferImpl, cmd.begin.beginInfo);
        break;
      }
      case Vulkan::CommandType::END:
      {
        End(commandBufferImpl);
        break;
      }
      case Vulkan::CommandType::BEGIN_RENDERPASS:
      {
        BeginRenderPass(commandBufferImpl, cmd.beginRenderPass);
        break;
      }
      case Vulkan::CommandType::END_RENDERPASS:
      {
        commandBufferImpl->EndRenderPass();
        break;
      }
      case Vulkan::CommandType::BIND_TEXTURES:
      {
        auto& textureBindings = cmd.bindTextures.textureBindings;
        commandBufferImpl->BindTextures(textureBindings,
                                        cmd.bindTextures.textureBindingsCount);
        break;
      }
      case Vulkan::CommandType::BIND_SAMPLERS:
      {
        break;
      }
      case Vulkan::CommandType::BIND_VERTEX_BUFFERS:
      {
        commandBufferImpl->BindVertexBuffers(cmd.bindVertexBuffers.firstBinding,
                                             cmd.bindVertexBuffers.vertexBufferBindings,
                                             cmd.bindVertexBuffers.vertexBufferBindingsCount);
        break;
      }
      case Vulkan::CommandType::BIND_INDEX_BUFFER:
      {
        auto indexBuffer = const_cast<Buffer*>(cmd.bindIndexBuffer.buffer);
        DALI_ASSERT_DEBUG(indexBuffer && indexBuffer->GetImpl());
        commandBufferImpl->BindIndexBuffer(*indexBuffer->GetImpl(), cmd.bindIndexBuffer.offset, cmd.bindIndexBuffer.format);
        break;
      }
      case Vulkan::CommandType::BIND_UNIFORM_BUFFER:
      {
        commandBufferImpl->BindUniformBuffers(cmd.bindUniformBuffers.uniformBufferBindings,
                                              cmd.bindUniformBuffers.uniformBufferBindingsCount);
        break;
      }
      case Vulkan::CommandType::BIND_PUSH_CONSTANTS:
      {
        break;
      }
      case Vulkan::CommandType::BIND_PIPELINE:
      {
        commandBufferImpl->BindPipeline(cmd.bindPipeline.pipeline);
        break;
      }
      case Vulkan::CommandType::DRAW:
      {
        commandBufferImpl->Draw(cmd.draw.draw.vertexCount,
                                cmd.draw.draw.instanceCount,
                                cmd.draw.draw.firstVertex,
                                cmd.draw.draw.firstInstance);
        break;
      }
      case Vulkan::CommandType::DRAW_INDEXED:
      {
        commandBufferImpl->DrawIndexed(cmd.draw.drawIndexed.indexCount,
                                       cmd.draw.drawIndexed.instanceCount,
                                       cmd.draw.drawIndexed.firstIndex,
                                       cmd.draw.drawIndexed.vertexOffset,
                                       cmd.draw.drawIndexed.firstInstance);
        break;
      }
      case Vulkan::CommandType::DRAW_INDEXED_INDIRECT:
      {
        auto buffer = ConstGraphicsCast<Buffer, Graphics::Buffer>(cmd.draw.drawIndexedIndirect.buffer)->GetImpl();
        commandBufferImpl->DrawIndexedIndirect(*buffer,
                                               cmd.draw.drawIndexedIndirect.offset,
                                               cmd.draw.drawIndexedIndirect.drawCount,
                                               cmd.draw.drawIndexedIndirect.stride);
        break;
      }
      case Vulkan::CommandType::DRAW_NATIVE:
      {
        break;
      }
      case Vulkan::CommandType::SET_SCISSOR:
      {
        auto value = *const_cast<Rect2D*>(&cmd.scissor.region);
        commandBufferImpl->SetScissor(value);
        break;
      }
      case Vulkan::CommandType::SET_SCISSOR_TEST:
      {
        break;
      }
      case Vulkan::CommandType::SET_VIEWPORT:
      {
        commandBufferImpl->SetViewport(cmd.viewport.region);
        break;
      }
      case Vulkan::CommandType::SET_STENCIL_TEST_ENABLE:
      {
        bool enabled = cmd.stencilTest.enabled;
        SetStencilTest(commandBufferImpl, enabled);
        break;
      }
      case Vulkan::CommandType::SET_STENCIL_WRITE_MASK:
      {
        uint32_t mask = cmd.stencilWriteMask.mask;
        commandBufferImpl->SetStencilWriteMask(mask);
        break;
      }
      case Vulkan::CommandType::SET_STENCIL_STATE:
      {
        SetStencilState(commandBufferImpl, cmd.stencilState.compareOp, cmd.stencilState.reference, cmd.stencilState.compareMask, cmd.stencilState.failOp, cmd.stencilState.passOp, cmd.stencilState.depthFailOp);
        break;
      }
      case Vulkan::CommandType::SET_DEPTH_COMPARE_OP:
      {
        Graphics::CompareOp compareOp = cmd.depth.compareOp;
        SetDepthCompare(commandBufferImpl, compareOp);
        break;
      }
      case Vulkan::CommandType::SET_DEPTH_TEST_ENABLE:
      {
        SetDepthTest(commandBufferImpl, cmd.depth.testEnabled);
        break;
      }
      case Vulkan::CommandType::SET_DEPTH_WRITE_ENABLE:
      {
        SetDepthWrite(commandBufferImpl, cmd.depth.writeEnabled);
        break;
      }
      case Vulkan::CommandType::SET_COLOR_WRITE_MASK:
      {
        commandBufferImpl->SetColorMask(cmd.colorMask.enabled);
        break;
      }
      case Vulkan::CommandType::SET_COLOR_BLEND_ENABLE:
      {
        commandBufferImpl->SetColorBlendEnable(0, cmd.colorBlend.enabled);
        break;
      }
      case Vulkan::CommandType::SET_COLOR_BLEND_EQUATION:
      {
        commandBufferImpl->SetColorBlendEquation(0, cmd.colorBlend.equation);
        break;
      }
      case Vulkan::CommandType::SET_COLOR_BLEND_ADVANCED:
      {
        commandBufferImpl->SetColorBlendAdvanced(0, cmd.colorBlend.advanced.srcPremultiplied, cmd.colorBlend.advanced.dstPremultiplied, cmd.colorBlend.advanced.blendOp);
        break;
      }
    }
  }
}

void CommandBufferExecutor::Reset()
{
  mRenderTarget = nullptr;
}

void CommandBufferExecutor::Begin(CommandBufferImpl* commandBufferImpl, const Graphics::CommandBufferBeginInfo& info)
{
  mRenderTarget = ConstGraphicsCast<Vulkan::RenderTarget, Graphics::RenderTarget>(info.renderTarget);

  DALI_LOG_INFO(gLogCmdBufferFilter, Debug::Verbose, "vkHandle:%p bufferIndex=%d\n", commandBufferImpl->GetVkHandle(), mController.GetGraphicsDevice().GetCurrentBufferIndex());

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
    SetDepthTest(commandBufferImpl, false);
    SetDepthWrite(commandBufferImpl, false);
    SetDepthCompare(commandBufferImpl, Graphics::CompareOp::LESS);
    SetStencilTest(commandBufferImpl, false);
  }
}

void CommandBufferExecutor::End(CommandBufferImpl* commandBufferImpl)
{
  DALI_LOG_INFO(gLogCmdBufferFilter, Debug::Verbose, "vkHandle:%p bufferIndex=%d\n", commandBufferImpl->GetVkHandle(), mController.GetGraphicsDevice().GetCurrentBufferIndex());

  commandBufferImpl->End();
}

void CommandBufferExecutor::BeginRenderPass(CommandBufferImpl* commandBufferImpl, const BeginRenderPassDescriptor& renderPassBegin)
{
  auto renderTarget = renderPassBegin.renderTarget;
  DALI_ASSERT_DEBUG(mRenderTarget == renderTarget && "RenderPass has different render target to cmd buffer Begin");

  auto             renderPass  = renderPassBegin.renderPass;
  auto             surface     = renderTarget->GetSurface();
  auto&            device      = mController.GetGraphicsDevice();
  FramebufferImpl* framebuffer = nullptr;
  RenderPassHandle renderPassImpl;
  Rect2D           renderArea = renderPassBegin.renderArea;
  if(surface)
  {
    auto window    = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface);
    auto surfaceId = window->GetSurfaceId();
    auto swapchain = device.GetSwapchainForSurfaceId(surfaceId);
    framebuffer    = swapchain->GetCurrentFramebuffer();
    renderArea.y   = framebuffer->GetHeight() - renderArea.y - renderArea.height;
    renderPassImpl = framebuffer->GetImplFromRenderPass(renderPass);
  }
  else
  {
    auto framebufferHandle = mRenderTarget->GetFramebuffer();
    framebuffer            = framebufferHandle->GetImpl();
    renderPassImpl         = framebuffer->GetImplFromRenderPass(renderPass);
  }

  std::vector<vk::ClearValue> vkClearValues;

  auto attachments = renderPass->GetCreateInfo().attachments;
  if(attachments != nullptr &&
     !attachments->empty()) // Can specify clear color even if load op is not clear.
  {
    const auto clearValues = renderPassBegin.clearValues.Ptr();
    for(auto i = 0u; i < renderPassBegin.clearValuesCount; ++i)
    {
      auto&               clearValue = *(clearValues + i);
      vk::ClearColorValue color;
      color.float32[0] = clearValue.color.r;
      color.float32[1] = clearValue.color.g;
      color.float32[2] = clearValue.color.b;
      color.float32[3] = clearValue.color.a;
      vkClearValues.emplace_back(color);
    }
  }

  commandBufferImpl->BeginRenderPass(vk::RenderPassBeginInfo{}
                                       .setFramebuffer(framebuffer->GetVkHandle())
                                       .setRenderPass(renderPassImpl->GetVkHandle())
                                       .setRenderArea({{renderArea.x, renderArea.y}, {renderArea.width, renderArea.height}})
                                       .setPClearValues(vkClearValues.data())
                                       .setClearValueCount(uint32_t(vkClearValues.size())),
                                     vk::SubpassContents::eInline);
}

void CommandBufferExecutor::SetStencilState(CommandBufferImpl*  commandBufferImpl,
                                            Graphics::CompareOp compareOp,
                                            uint32_t            reference,
                                            uint32_t            compareMask,
                                            Graphics::StencilOp failOp,
                                            Graphics::StencilOp passOp,
                                            Graphics::StencilOp depthFailOp)
{
  commandBufferImpl->SetStencilCompareMask(compareMask);
  commandBufferImpl->SetStencilReference(reference);
  commandBufferImpl->SetStencilOp(failOp, passOp, depthFailOp, compareOp);
}

void CommandBufferExecutor::SetStencilTest(CommandBufferImpl* commandBufferImpl, bool enabled)
{
  commandBufferImpl->SetStencilTestEnable(enabled);
}

void CommandBufferExecutor::SetDepthCompare(CommandBufferImpl* commandBufferImpl, Graphics::CompareOp compareOp)
{
  commandBufferImpl->SetDepthCompareOp(compareOp);
}

void CommandBufferExecutor::SetDepthTest(CommandBufferImpl* commandBufferImpl, bool enabled)
{
  commandBufferImpl->SetDepthTestEnable(enabled);
}

void CommandBufferExecutor::SetDepthWrite(CommandBufferImpl* commandBufferImpl, bool enabled)
{
  commandBufferImpl->SetDepthWriteEnable(enabled);
}

} // namespace Dali::Graphics::Vulkan
