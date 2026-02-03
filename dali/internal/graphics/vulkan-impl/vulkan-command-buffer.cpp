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
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer.h>

// INTERNAL HEADERS
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-executor.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-pool-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-program-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-stored-command-buffer.h>
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
  mStorageType(CommandBuffer::Storage::IMMEDIATE)
{
  AllocateCommandBuffers(true);
  mStoredCommandBuffer = std::make_unique<StoredCommandBuffer>(mCreateInfo, mCreateInfo.fixedCapacity);
}

CommandBuffer::CommandBuffer(const Graphics::CommandBufferCreateInfo& createInfo, VulkanGraphicsController& controller, CommandBuffer::Storage storageType, bool doubleBuffered)
: CommandBufferResource(createInfo, controller),
  mStorageType(storageType),
  mDoubleBuffered(doubleBuffered)
{
  AllocateCommandBuffers(mDoubleBuffered);
  if(storageType == CommandBuffer::Storage::STORED)
  {
    mStoredCommandBuffer = std::make_unique<StoredCommandBuffer>(mCreateInfo, mCreateInfo.fixedCapacity);
  }
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

void CommandBuffer::Reset()
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->Reset();
  }
  const uint32_t bufferIndex = mController.GetGraphicsDevice().GetCurrentBufferIndex();

  if(bufferIndex >= EXCESS_BUFFER_COUNT)
  {
    DALI_LOG_ERROR("ERROR: bufferIndex %u exceeds EXCESS_BUFFER_COUNT\n", bufferIndex);
  }
  DALI_ASSERT_DEBUG(bufferIndex < EXCESS_BUFFER_COUNT);

  if(bufferIndex >= mCommandBufferImpl.size())
  {
    // Handle (odd) case where swapchain is re-created with a different number of min images
    AllocateCommandBuffers(mDoubleBuffered);
  }

  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size());
  DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "Resetting cmd buf[%d]\n", bufferIndex);
  mCommandBufferImpl[bufferIndex]->Reset();

  mRenderTarget = nullptr;
}

void CommandBuffer::Begin(const Graphics::CommandBufferBeginInfo& info)
{
  mRenderTarget = ConstGraphicsCast<Vulkan::RenderTarget, Graphics::RenderTarget>(info.renderTarget);
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->Begin(info);
  }
  else
  {
    // Create temporary command buffer and execute it.
    StoredCommandBuffer commandBuffer(mCreateInfo, 1);
    commandBuffer.Begin(info);
    CommandBufferExecutor commandExecutor(mController);
    commandExecutor.ProcessCommandBuffer(&commandBuffer, GetImpl());
  }
}

void CommandBuffer::End()
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->End();
  }
  else
  {
    GetImpl()->End();
  }
}

void CommandBuffer::BindVertexBuffers(uint32_t                                    firstBinding,
                                      const std::vector<const Graphics::Buffer*>& gfxBuffers,
                                      const std::vector<uint32_t>&                offsets)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->BindVertexBuffers(firstBinding, gfxBuffers, offsets);
  }
  else
  {
    // Create temporary command buffer and execute it.
    StoredCommandBuffer commandBuffer(mCreateInfo, 1);
    commandBuffer.BindVertexBuffers(firstBinding, gfxBuffers, offsets);
    CommandBufferExecutor commandExecutor(mController);
    commandExecutor.ProcessCommandBuffer(&commandBuffer, GetImpl());
  }
}

void CommandBuffer::BindIndexBuffer(const Graphics::Buffer& gfxBuffer,
                                    uint32_t                offset,
                                    Format                  format)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->BindIndexBuffer(gfxBuffer, offset, format);
  }
  else
  {
    auto indexBuffer = ConstGraphicsCast<Buffer, Graphics::Buffer>(&gfxBuffer);
    DALI_ASSERT_DEBUG(indexBuffer && indexBuffer->GetImpl());

    GetImpl()->BindIndexBuffer(*indexBuffer->GetImpl(), offset, format);
  }
}

void CommandBuffer::BindUniformBuffers(const std::vector<UniformBufferBinding>& bindings)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->BindUniformBuffers(bindings);
  }
  else
  {
    GetImpl()->BindUniformBuffers(bindings);
  }
}

void CommandBuffer::BindPipeline(const Graphics::Pipeline& pipeline)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->BindPipeline(pipeline);
  }
  else
  {
    GetImpl()->BindPipeline(&pipeline);
  }
}

void CommandBuffer::BindTextures(const std::vector<TextureBinding>& textureBindings)
{
  mController.CheckTextureDependencies(textureBindings, mRenderTarget);
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->BindTextures(textureBindings);
  }
  else
  {
    GetImpl()->BindTextures(textureBindings);
  }
}

void CommandBuffer::BindSamplers(const std::vector<SamplerBinding>& samplerBindings)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->BindSamplers(samplerBindings);
  }
  else
  {
    GetImpl()->BindSamplers(samplerBindings);
  }
}

void CommandBuffer::BindPushConstants(void*    data,
                                      uint32_t size,
                                      uint32_t binding)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->BindPushConstants(data, size, binding);
  }
  else
  {
    //GetImpl()->BindPushConstants(data, size, binding);
  }
}

void CommandBuffer::BeginRenderPass(Graphics::RenderPass*          gfxRenderPass,
                                    Graphics::RenderTarget*        gfxRenderTarget,
                                    Rect2D                         renderArea,
                                    const std::vector<ClearValue>& clearValues)
{
  if(mStoredCommandBuffer)
  {

    mStoredCommandBuffer->BeginRenderPass(gfxRenderPass, gfxRenderTarget, renderArea, clearValues);
  }
  else
  {
    CommandBufferExecutor     commandExecutor(mController);
    BeginRenderPassDescriptor descriptor;
    descriptor.renderPass       = static_cast<Vulkan::RenderPass*>(gfxRenderPass);
    descriptor.renderTarget     = static_cast<Vulkan::RenderTarget*>(gfxRenderTarget);
    descriptor.renderArea       = renderArea;
    descriptor.clearValuesCount = clearValues.size();
    descriptor.clearValues      = const_cast<ClearValue*>(clearValues.data());
    commandExecutor.BeginRenderPass(GetImpl(), descriptor);
  }

  auto surface = mRenderTarget->GetSurface();
  if(!surface)
  {
    mController.AddTextureDependencies(mRenderTarget);
  }
}

void CommandBuffer::EndRenderPass(Graphics::SyncObject* syncObject)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->EndRenderPass(syncObject);
  }
  else
  {
    GetImpl()->EndRenderPass();
  }
}

void CommandBuffer::ReadPixels(uint8_t* buffer)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->ReadPixels(buffer);
  }
  else
  {
    GetImpl()->ReadPixels(buffer);
  }
}

void CommandBuffer::ExecuteCommandBuffers(std::vector<const Graphics::CommandBuffer*>&& commandBuffers)
{
  DALI_LOG_ERROR("Secondary cmd buffers no longer supported\n");
}

void CommandBuffer::Draw(uint32_t vertexCount,
                         uint32_t instanceCount,
                         uint32_t firstVertex,
                         uint32_t firstInstance)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
  }
  else
  {
    GetImpl()->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
  }
}

void CommandBuffer::DrawIndexed(uint32_t indexCount,
                                uint32_t instanceCount,
                                uint32_t firstIndex,
                                int32_t  vertexOffset,
                                uint32_t firstInstance)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
  }
  else
  {
    GetImpl()->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
  }
}

void CommandBuffer::DrawIndexedIndirect(Graphics::Buffer& gfxBuffer,
                                        uint32_t          offset,
                                        uint32_t          drawCount,
                                        uint32_t          stride)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->DrawIndexedIndirect(gfxBuffer, offset, drawCount, stride);
  }
  else
  {
    GetImpl()->DrawIndexedIndirect(*static_cast<Vulkan::Buffer*>(&gfxBuffer)->GetImpl(), offset, drawCount, stride);
  }
}

void CommandBuffer::DrawNative(const DrawNativeInfo* drawInfo)
{
  // No implementation in Vulkan
}

void CommandBuffer::SetScissor(Rect2D value)
{
  Rect2D correctedValue = value;

  // Invert the Y coord for surface only, as we've flipped
  // projection matrix Y scale for framebuffers, and only if it's
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

  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->SetScissor(correctedValue);
  }
  else
  {
    GetImpl()->SetScissor(correctedValue);
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

  // Invert Y of viewport for surface only.
  auto surface = mRenderTarget->GetSurface();
  if(surface)
  {
    correctedValue.y = mRenderTarget->GetSurface()->GetPositionSize().height - correctedValue.height - correctedValue.y;
  }

  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->SetViewport(correctedValue);
  }
  else
  {
    GetImpl()->SetViewport(correctedValue);
  }
}

void CommandBuffer::SetViewportEnable(bool value)
{
  // Enabled by default. What does disabling test do?!
  // Probably should force pipeline to not use dynamic viewport state
}

void CommandBuffer::SetColorMask(bool enabled)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->SetColorMask(enabled);
  }
  else
  {
    GetImpl()->SetColorMask(enabled);
  }
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
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->SetStencilTestEnable(stencilEnable);
  }
  else
  {
    GetImpl()->SetStencilTestEnable(stencilEnable);
  }
}

void CommandBuffer::SetStencilWriteMask(uint32_t writeMask)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->SetStencilWriteMask(writeMask);
  }
  else
  {
    CommandBufferImpl* commandBufferImpl = GetImpl();
    commandBufferImpl->SetStencilWriteMask(writeMask);
  }
}

void CommandBuffer::SetStencilState(Graphics::CompareOp compareOp,
                                    uint32_t            reference,
                                    uint32_t            compareMask,
                                    Graphics::StencilOp failOp,
                                    Graphics::StencilOp passOp,
                                    Graphics::StencilOp depthFailOp)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->SetStencilState(compareOp, reference, compareMask, failOp, passOp, depthFailOp);
  }
  else
  {
    CommandBufferImpl* commandBufferImpl = GetImpl();

    commandBufferImpl->SetStencilCompareMask(compareMask);
    commandBufferImpl->SetStencilReference(reference);
    commandBufferImpl->SetStencilOp(failOp, passOp, depthFailOp, compareOp);
  }
}

void CommandBuffer::SetDepthCompareOp(Graphics::CompareOp compareOp)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->SetDepthCompareOp(compareOp);
  }
  else
  {
    CommandBufferImpl* commandBufferImpl = GetImpl();
    commandBufferImpl->SetDepthCompareOp(compareOp);
  }
}

void CommandBuffer::SetDepthTestEnable(bool depthTestEnable)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->SetDepthTestEnable(depthTestEnable);
  }
  else
  {
    CommandBufferImpl* commandBufferImpl = GetImpl();
    commandBufferImpl->SetDepthTestEnable(depthTestEnable);
  }
}
void CommandBuffer::SetDepthWriteEnable(bool depthWriteEnable)
{
  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->SetDepthWriteEnable(depthWriteEnable);
  }
  else
  {
    CommandBufferImpl* commandBufferImpl = GetImpl();
    commandBufferImpl->SetDepthWriteEnable(depthWriteEnable);
  }
}

void CommandBuffer::SetColorBlendEnable(uint32_t attachment, bool enabled)
{
  if(attachment == 0)
  {
    if(mStoredCommandBuffer)
    {
      mStoredCommandBuffer->SetColorBlendEnable(attachment, enabled);
    }
    else
    {
      CommandBufferImpl* commandBufferImpl = GetImpl();
      commandBufferImpl->SetColorBlendEnable(attachment, enabled);
    }
  }
}

void CommandBuffer::SetColorBlendEquation(uint32_t attachment,
                                         BlendFactor srcColorBlendFactor,
                                         BlendFactor dstColorBlendFactor,
                                         BlendOp colorBlendOp,
                                         BlendFactor srcAlphaBlendFactor,
                                         BlendFactor dstAlphaBlendFactor,
                                         BlendOp alphaBlendOp)
{
  // For now, only support attachment 0 (single color attachment)
  if(attachment != 0)
  {
    return;
  }

  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->SetColorBlendEquation(attachment,
                                                srcColorBlendFactor,
                                                dstColorBlendFactor,
                                                colorBlendOp,
                                                srcAlphaBlendFactor,
                                                dstAlphaBlendFactor,
                                                alphaBlendOp);
  }
  else
  {
    ColorBlendEquation equation{
      srcColorBlendFactor,
      dstColorBlendFactor,
      colorBlendOp,
      srcAlphaBlendFactor,
      dstAlphaBlendFactor,
      alphaBlendOp
    };

    CommandBufferImpl* commandBufferImpl = GetImpl();
    commandBufferImpl->SetColorBlendEquation(attachment, equation);
  }
}

void CommandBuffer::SetColorBlendAdvanced(uint32_t attachment,
                                          bool srcPremultiplied,
                                          bool dstPremultiplied,
                                          Graphics::BlendOp blendOp)
{
  // For now, only support attachment 0 (single color attachment)
  if(attachment != 0)
  {
    return;
  }

  if(mStoredCommandBuffer)
  {
    mStoredCommandBuffer->SetColorBlendAdvanced(attachment, srcPremultiplied, dstPremultiplied, blendOp);
  }
  else
  {
    CommandBufferImpl* commandBufferImpl = GetImpl();
    commandBufferImpl->SetColorBlendAdvanced(attachment, srcPremultiplied, dstPremultiplied, blendOp);
  }
}

Vulkan::RenderTarget* CommandBuffer::GetRenderTarget() const
{
  // Gets the render target from the Begin() cmd.
  return mRenderTarget;
}

[[nodiscard]] Vulkan::CommandBufferImpl* CommandBuffer::GetImpl() const
{
  uint32_t bufferIndex = mDoubleBuffered ? mController.GetGraphicsDevice().GetCurrentBufferIndex() : 0;

  DALI_ASSERT_ALWAYS(bufferIndex < mCommandBufferImpl.size());
  auto impl = mCommandBufferImpl[bufferIndex];

  DALI_LOG_INFO(gLogCmdBufferFilter, Debug::Verbose, "Resource:%p DoubleBuffered:%s currentBufferIndex=%d  appliedIndex:%d impl:%p\n", this, mDoubleBuffered ? "T" : "F", mController.GetGraphicsDevice().GetCurrentBufferIndex(), bufferIndex, impl);

  return impl;
}

void CommandBuffer::AllocateCommandBuffers(bool doubleBuffered)
{
  auto& device    = mController.GetGraphicsDevice();
  bool  isPrimary = true;
  if(mCreateInfo.level == Graphics::CommandBufferLevel::SECONDARY)
  {
    isPrimary = false;
  }
  auto commandPool = device.GetCommandPool(std::this_thread::get_id());
  auto bufferCount = doubleBuffered ? device.GetBufferCount() : 1;

  DALI_LOG_INFO(gVulkanFilter, Debug::General, "Allocating %d new cmd buffers\n", bufferCount - mCommandBufferImpl.size());

  for(uint32_t i = mCommandBufferImpl.size(); i < bufferCount; ++i)
  {
    mCommandBufferImpl.emplace_back(commandPool->NewCommandBuffer(isPrimary));
  }
}

void CommandBuffer::Process() const
{
  DALI_LOG_INFO(gLogCmdBufferFilter, Debug::Verbose, "Resource:%p\n", this);
  CommandBufferExecutor commandExecutor(mController);
  commandExecutor.ProcessCommandBuffer(mStoredCommandBuffer.get(), GetImpl());
}

} // namespace Dali::Graphics::Vulkan
