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

#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-texture.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

template<class VulkanType, class GraphicsType>
VulkanType* VulkanCast(GraphicsType* apiObject)
{
  return const_cast<VulkanType*>(static_cast<const VulkanType*>(apiObject));
}

namespace Dali::Graphics::Vulkan
{
Framebuffer::Framebuffer(const FramebufferCreateInfo& createInfo, VulkanGraphicsController& controller)
: Resource(createInfo, controller),
  mFramebufferImpl{nullptr}
{
}

Framebuffer::~Framebuffer() = default;

ResourceBase::InitializationResult Framebuffer::InitializeResource()
{
  auto& device          = mController.GetGraphicsDevice();
  bool  firstRenderPass = true;

  // There are usually 2 render passes, Clear & Load.
  // They only contain load/store ops for each of color / depth&stencil attachments.
  for(auto& gfxRenderPass : mCreateInfo.renderPasses)
  {
    auto* renderPass             = VulkanCast<Vulkan::RenderPass>(gfxRenderPass);
    auto& attachmentDescriptions = *renderPass->GetCreateInfo().attachments;

    // Each attachment description must match passed in attachments.
    size_t attachmentDescriptionIndex = 0;

    // Create attachments
    SharedAttachments colorAttachments;

    auto clearColor = vk::ClearColorValue{}.setFloat32({1.0f, 0.0f, 1.0f, 1.0f});

    for(auto& attachment : mCreateInfo.colorAttachments)
    {
      auto* graphicsTexture = VulkanCast<Vulkan::Texture>(attachment.texture);
      DALI_ASSERT_DEBUG(attachmentDescriptionIndex < attachmentDescriptions.size() &&
                        "Render pass attachment descriptions out of range");

      // FramebufferAttachment takes ownership of the image view. So, create new view onto the image.
      std::unique_ptr<ImageView> imageView = graphicsTexture->CreateImageView();
      colorAttachments.emplace_back(FramebufferAttachment::NewColorAttachment(imageView, clearColor, &attachmentDescriptions[attachmentDescriptionIndex++], false));
    }

    FramebufferAttachmentHandle depthStencilAttachment;

    auto depthClearValue = vk::ClearDepthStencilValue{}.setDepth(0.0).setStencil(STENCIL_DEFAULT_CLEAR_VALUE);

    if(mCreateInfo.depthStencilAttachment.depthTexture)
    {
      DALI_ASSERT_DEBUG(attachmentDescriptionIndex < attachmentDescriptions.size() && "Render pass attachment descriptions out of range");

      auto                       depthTexture = VulkanCast<Vulkan::Texture>(mCreateInfo.depthStencilAttachment.depthTexture);
      std::unique_ptr<ImageView> imageView    = depthTexture->CreateImageView();
      depthStencilAttachment                  = FramebufferAttachmentHandle(FramebufferAttachment::NewDepthAttachment(imageView, depthClearValue, &attachmentDescriptions[attachmentDescriptionIndex++]));
    }
    else if(mCreateInfo.depthStencilAttachment.stencilTexture)
    {
      DALI_ASSERT_DEBUG(attachmentDescriptionIndex < attachmentDescriptions.size() && "Render pass attachment descriptions out of range");
      auto                       stencilTexture = VulkanCast<Vulkan::Texture>(mCreateInfo.depthStencilAttachment.stencilTexture);
      std::unique_ptr<ImageView> imageView      = stencilTexture->CreateImageView();
      depthStencilAttachment                    = FramebufferAttachmentHandle(FramebufferAttachment::NewDepthAttachment(imageView, depthClearValue, &attachmentDescriptions[attachmentDescriptionIndex++]));
    }

    RenderPassImpl::CreateInfo createInfo;
    RenderPassImpl::CreateCompatibleCreateInfo(createInfo, colorAttachments, depthStencilAttachment, true);
    auto renderPassImpl = RenderPassHandle(RenderPassImpl::New(device, createInfo));

    if(firstRenderPass)
    {
      // Create a framebuffer using the first render pass. Subsequent render passes will be created that
      // are compatible.
      mFramebufferImpl = FramebufferImpl::New(device, renderPassImpl, colorAttachments, depthStencilAttachment, mCreateInfo.size.width, mCreateInfo.size.height);
    }

    // Add the renderPass/renderPassImpl pair to the framebuffer.
    if(mFramebufferImpl)
    {
      mFramebufferImpl->AddRenderPass(renderPass, renderPassImpl);
    }
    firstRenderPass = false;
  }

  return InitializationResult::INITIALIZED;
}

void Framebuffer::DestroyResource()
{
}

void Framebuffer::DiscardResource()
{
}

} // namespace Dali::Graphics::Vulkan
