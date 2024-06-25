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

#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{
Framebuffer::Framebuffer(const FramebufferCreateInfo& createInfo, VulkanGraphicsController& controller)
: Resource(createInfo, controller),
  mFramebufferImpl{nullptr}
{
  // mController.AddFramebuffer(*this)
}

Framebuffer::~Framebuffer() = default;

bool Framebuffer::InitializeResource()
{
  /*
   * Renderpass handling.
   * We get passed VulkanRenderPass
   * For actual framebuffer creation, we need at least the first VulkanRenderPass to have a VulkanRenderPassImpl created
   * and for subsequent VulkanRenderPasses to be compatible with the first (and can be created on the fly)
   */

  // Create attachments
  auto renderPass = static_cast<Vulkan::RenderPass*>(mCreateInfo.renderPasses[0]);

  auto renderPassImpl = renderPass->GetImpl(); // Only generate actual render pass if needed
  if(!renderPassImpl)
  {
    renderPass->InitializeResource();
  }

  auto&                               device = mController.GetGraphicsDevice();
  std::vector<FramebufferAttachment*> colorAttachments;
  FramebufferAttachment*              depthStencilAttachment{nullptr};
  //@todo FINISH ME! (Needs texture -> image view bindings)
  mFramebufferImpl = FramebufferImpl::New(device, renderPassImpl, colorAttachments, depthStencilAttachment, mCreateInfo.size.width, mCreateInfo.size.height);

  //@todo Store all the render passes here. Will be used later to generate compatible render pass impls.
  return true;
}

void Framebuffer::DestroyResource()
{
}

void Framebuffer::DiscardResource()
{
}

} // namespace Vulkan
} // namespace Graphics
} // namespace Dali
