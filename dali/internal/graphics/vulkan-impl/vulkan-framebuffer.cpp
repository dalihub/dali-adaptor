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

namespace Dali::Graphics::Vulkan
{
Framebuffer::Framebuffer(const FramebufferCreateInfo& createInfo, VulkanGraphicsController& controller)
: Resource(createInfo, controller),
  mFramebufferImpl{nullptr}
{
}

Framebuffer::~Framebuffer() = default;

bool Framebuffer::InitializeResource()
{
  // Create attachments
  OwnedAttachments colorAttachments;
  // for(auto& attachment : mCreateInfo.colorAttachments)
  {
    // auto graphicsTexture = static_cast<const Vulkan::Texture*>(attachment.texture);
    // colorAttachments.push_back(FramebufferAttachment::NewColorAttachment(attachment.texture->GetVkHandle(), clearColor, AttachmentType::COLOR, false);
  }
  std::unique_ptr<FramebufferAttachment> depthStencilAttachment;
  if(mCreateInfo.depthStencilAttachment.depthTexture || mCreateInfo.depthStencilAttachment.stencilTexture)
  {
    // depthStencilAttachment = FramebufferAttachment::NewDepthAttachment();
  }

  auto& device     = mController.GetGraphicsDevice();
  mFramebufferImpl = FramebufferImpl::New(device, RenderPassHandle{}, colorAttachments, depthStencilAttachment, mCreateInfo.size.width, mCreateInfo.size.height);

  return true;
}

void Framebuffer::DestroyResource()
{
}

void Framebuffer::DiscardResource()
{
}

} // namespace Dali::Graphics::Vulkan
