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
#include <dali/internal/graphics/vulkan-impl/vulkan-render-target.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-queue-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/internal/window-system/common/window-render-surface.h>

namespace Dali::Graphics::Vulkan
{
RenderTarget::RenderTarget(const Graphics::RenderTargetCreateInfo& createInfo, VulkanGraphicsController& controller)
: RenderTargetResource(createInfo, controller)
{
  if(createInfo.surface)
  {
    // Do creation stuff!
    //   Create Swapchain?!
  }
  else
  {
    // Non-surface render targets use own semaphore to signal cmd buffer completion.
    auto& graphicsDevice = controller.GetGraphicsDevice();
    mSubmitSemaphore     = graphicsDevice.GetLogicalDevice().createSemaphore({}, graphicsDevice.GetAllocator()).value;
  }
}

RenderTarget::~RenderTarget() = default;

void RenderTarget::DestroyResource()
{
}

void RenderTarget::DiscardResource()
{
  mController.DiscardResource(this);
  mController.RemoveRenderTarget(this); // Remove from dependency graph

  // The surface context should be deleted now
  if(mCreateInfo.surface)
  {
    mCreateInfo.surface = nullptr;
  }
}

Vulkan::Framebuffer* RenderTarget::GetFramebuffer() const
{
  return static_cast<Vulkan::Framebuffer*>(mCreateInfo.framebuffer);
}

Integration::RenderSurfaceInterface* RenderTarget::GetSurface() const
{
  return mCreateInfo.surface;
}

Vulkan::FramebufferImpl* RenderTarget::GetCurrentFramebufferImpl() const
{
  auto framebuffer = GetFramebuffer();
  auto surface     = GetSurface();

  FramebufferImpl* fbImpl = nullptr;
  if(surface)
  {
    auto& gfxDevice = mController.GetGraphicsDevice();
    auto  surfaceId = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface)->GetSurfaceId();
    auto  swapchain = gfxDevice.GetSwapchainForSurfaceId(surfaceId);
    fbImpl          = swapchain->GetCurrentFramebuffer();
  }
  else if(framebuffer)
  {
    fbImpl = framebuffer->GetImpl();
  }
  return fbImpl;
}

Vulkan::RenderPassHandle RenderTarget::GetRenderPass(const Graphics::RenderPass* gfxRenderPass) const
{
  auto renderPass      = const_cast<Vulkan::RenderPass*>(static_cast<const Vulkan::RenderPass*>(gfxRenderPass));
  auto framebufferImpl = GetCurrentFramebufferImpl();
  return framebufferImpl->GetImplFromRenderPass(renderPass);
}

void RenderTarget::CreateSubmissionData(
  const CommandBuffer*         cmdBuffer,
  std::vector<SubmissionData>& submissionData)
{
  auto surface = GetSurface();

  std::vector<vk::Semaphore> waitSemaphores;
  for(auto renderTarget : mDependencies)
  {
    // Only use semaphore if dependency render target was submitted
    // already this frame and not already waited on.
    if(renderTarget->mSubmitted && !renderTarget->mSemaphoreWaited)
    {
      waitSemaphores.push_back(renderTarget->mSubmitSemaphore);
      renderTarget->mSemaphoreWaited = true;
    }
  }
  std::vector<vk::PipelineStageFlags> waitDstStageMask{waitSemaphores.size(), vk::PipelineStageFlagBits::eColorAttachmentOutput};

  if(!surface)
  {
    submissionData.emplace_back(SubmissionData{waitSemaphores, waitDstStageMask, {cmdBuffer->GetImpl()}, {mSubmitSemaphore}});
  }
  else
  {
    auto surfaceId = static_cast<Internal::Adaptor::WindowRenderSurface*>(surface)->GetSurfaceId();
    auto swapchain = mController.GetGraphicsDevice().GetSwapchainForSurfaceId(surfaceId);
    swapchain->CreateSubmissionData(cmdBuffer->GetImpl(), waitSemaphores, waitDstStageMask, submissionData);
  }
  mSubmitted = true;
}

} // namespace Dali::Graphics::Vulkan
