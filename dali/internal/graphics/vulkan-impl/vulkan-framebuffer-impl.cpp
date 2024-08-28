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

#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-view-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

#include <dali/integration-api/debug.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{
FramebufferAttachment* FramebufferAttachment::NewColorAttachment(std::unique_ptr<ImageView>& imageView,
                                                                 vk::ClearColorValue         clearColorValue,
                                                                 bool                        presentable)
{
  assert(imageView->GetImage()->GetUsageFlags() & vk::ImageUsageFlagBits::eColorAttachment);

  auto attachment = new FramebufferAttachment(imageView,
                                              clearColorValue,
                                              AttachmentType::COLOR,
                                              presentable);
  return attachment;
}

FramebufferAttachment* FramebufferAttachment::NewDepthAttachment(
  std::unique_ptr<ImageView>& imageView,
  vk::ClearDepthStencilValue  clearDepthStencilValue)
{
  assert(imageView->GetImage()->GetUsageFlags() & vk::ImageUsageFlagBits::eDepthStencilAttachment);

  auto attachment = new FramebufferAttachment(imageView,
                                              clearDepthStencilValue,
                                              AttachmentType::DEPTH_STENCIL,
                                              false /* presentable */);

  return attachment;
}

FramebufferAttachment::FramebufferAttachment(std::unique_ptr<ImageView>& imageView,
                                             vk::ClearValue              clearColor,
                                             AttachmentType              type,
                                             bool                        presentable)
: mClearValue(clearColor),
  mType(type)
{
  mImageView.swap(imageView);
  auto image = mImageView->GetImage();

  auto sampleCountFlags = image->GetSampleCount();

  mDescription.setSamples(sampleCountFlags);

  mDescription.setLoadOp(vk::AttachmentLoadOp::eClear);
  mDescription.setStoreOp(vk::AttachmentStoreOp::eStore);
  mDescription.setStencilLoadOp(vk::AttachmentLoadOp::eClear);
  mDescription.setStencilStoreOp(vk::AttachmentStoreOp::eStore);
  mDescription.setFormat(image->GetFormat());
  mDescription.setInitialLayout(vk::ImageLayout::eUndefined);

  if(type == AttachmentType::DEPTH_STENCIL)
  {
    mDescription.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
  }
  else
  {
    mDescription.finalLayout = presentable ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eShaderReadOnlyOptimal;
  }
}

ImageView* FramebufferAttachment::GetImageView() const
{
  return mImageView.get();
}

const vk::AttachmentDescription& FramebufferAttachment::GetDescription() const
{
  return mDescription;
}

const vk::ClearValue& FramebufferAttachment::GetClearValue() const
{
  return mClearValue;
}

AttachmentType FramebufferAttachment::GetType() const
{
  return mType;
}

bool FramebufferAttachment::IsValid() const
{
  return mImageView != nullptr;
}

// FramebufferImpl -------------------------------

FramebufferImpl* FramebufferImpl::New(
  Vulkan::Device&   device,
  RenderPassImpl*   renderPass,
  OwnedAttachments& attachments,
  uint32_t          width,
  uint32_t          height,
  bool              hasDepthAttachments)
{
  DALI_ASSERT_ALWAYS(renderPass != nullptr && "You require more render passes!");

  std::vector<vk::ImageView> imageViewAttachments;
  for(auto& attachment : attachments)
  {
    imageViewAttachments.emplace_back(attachment->GetImageView()->GetVkHandle());
  }

  auto framebufferCreateInfo = vk::FramebufferCreateInfo{}
                                 .setRenderPass(renderPass->GetVkHandle())
                                 .setPAttachments(imageViewAttachments.data())
                                 .setLayers(1)
                                 .setWidth(width)
                                 .setHeight(height)
                                 .setAttachmentCount(U32(attachments.size()));

  auto vkFramebuffer = VkAssert(device.GetLogicalDevice().createFramebuffer(framebufferCreateInfo, device.GetAllocator()));

  return new FramebufferImpl(device,
                             attachments,
                             vkFramebuffer,
                             *renderPass,
                             width,
                             height,
                             hasDepthAttachments);
}

FramebufferImpl* FramebufferImpl::New(
  Vulkan::Device&                         device,
  RenderPassImpl*                         renderPass,
  OwnedAttachments&                       colorAttachments,
  std::unique_ptr<FramebufferAttachment>& depthAttachment,
  uint32_t                                width,
  uint32_t                                height)
{
  assert((!colorAttachments.empty() || depthAttachment) && "Cannot create framebuffer. Please provide at least one attachment");

  auto                                colorAttachmentsValid = true;
  std::vector<FramebufferAttachment*> attachments;
  for(auto& attachment : colorAttachments)
  {
    attachments.emplace_back(attachment.get());
    if(!attachment->IsValid())
    {
      colorAttachmentsValid = false;
      break;
    }
  }

  assert(colorAttachmentsValid && "Invalid color attachment! The attachment has no ImageView");

  // Flag that indicates if the framebuffer has a depth attachment
  auto hasDepth = false;
  if(depthAttachment)
  {
    hasDepth = depthAttachment->IsValid();
    assert(hasDepth && "Invalid depth attachment! The attachment has no ImageView");
  }

  // This vector stores the attachments (vk::ImageViews)

  // Flag that indicates if the render pass is externally provided
  if(renderPass == nullptr)
  {
    // Create compatible vulkan render pass
    renderPass = RenderPassImpl::New(device, attachments, depthAttachment.get());
  }

  OwnedAttachments ownedAttachments(std::move(colorAttachments));
  if(hasDepth)
  {
    ownedAttachments.emplace_back(std::move(depthAttachment));
  }
  return FramebufferImpl::New(device, renderPass, ownedAttachments, width, height, hasDepth);
}

FramebufferImpl::FramebufferImpl(Device&               graphicsDevice,
                                 OwnedAttachments&     attachments,
                                 vk::Framebuffer       vkHandle,
                                 const RenderPassImpl& renderPassImpl,
                                 uint32_t              width,
                                 uint32_t              height,
                                 bool                  hasDepthAttachment)
: mGraphicsDevice(&graphicsDevice),
  mWidth(width),
  mHeight(height),
  mAttachments(std::move(attachments)),
  mFramebuffer(vkHandle),
  mHasDepthAttachment(hasDepthAttachment)
{
  mRenderPasses.push_back(RenderPassMapElement{nullptr, const_cast<RenderPassImpl*>(&renderPassImpl)});
}

void FramebufferImpl::Destroy()
{
  auto device = mGraphicsDevice->GetLogicalDevice();

  mRenderPasses.clear();
  mAttachments.clear();

  if(mFramebuffer)
  {
    auto allocator = &mGraphicsDevice->GetAllocator();

    DALI_LOG_INFO(gVulkanFilter, Debug::General, "Destroying Framebuffer: %p\n", static_cast<VkFramebuffer>(mFramebuffer));
    device.destroyFramebuffer(mFramebuffer, allocator);
  }
  mFramebuffer = nullptr;
}

uint32_t FramebufferImpl::GetWidth() const
{
  return mWidth;
}

uint32_t FramebufferImpl::GetHeight() const
{
  return mHeight;
}

FramebufferAttachment* FramebufferImpl::GetAttachment(AttachmentType type, uint32_t index) const
{
  switch(type)
  {
    case AttachmentType::COLOR:
    {
      return mAttachments[index].get();
    }
    case AttachmentType::DEPTH_STENCIL:
    {
      if(mHasDepthAttachment)
      {
        return mAttachments.back().get();
      }
    }
    case AttachmentType::INPUT:
    case AttachmentType::RESOLVE:
    case AttachmentType::PRESERVE:
    case AttachmentType::UNDEFINED:
      break;
  }

  return nullptr;
}

std::vector<FramebufferAttachment*> FramebufferImpl::GetAttachments(AttachmentType type) const
{
  auto retval = std::vector<FramebufferAttachment*>{};
  switch(type)
  {
    case AttachmentType::COLOR:
    {
      auto numColorAttachments = mHasDepthAttachment ? mAttachments.size() - 1 : mAttachments.size();
      retval.reserve(numColorAttachments);
      for(size_t i = 0; i < numColorAttachments; ++i)
      {
        retval.emplace_back(mAttachments[i].get());
      }
      break;
    }
    case AttachmentType::DEPTH_STENCIL:
    {
      if(mHasDepthAttachment)
      {
        retval.reserve(1);
        retval.emplace_back(mAttachments.back().get());
      }
      break;
    }
    case AttachmentType::INPUT:
    case AttachmentType::RESOLVE:
    case AttachmentType::PRESERVE:
    case AttachmentType::UNDEFINED:
    {
      break;
    }
  }
  return retval;
}

uint32_t FramebufferImpl::GetAttachmentCount(AttachmentType type) const
{
  switch(type)
  {
    case AttachmentType::COLOR:
    {
      return U32(mAttachments.size() - mHasDepthAttachment);
    }
    case AttachmentType::DEPTH_STENCIL:
    {
      return mHasDepthAttachment;
    }
    case AttachmentType::INPUT:
    case AttachmentType::RESOLVE:
    case AttachmentType::PRESERVE:
    case AttachmentType::UNDEFINED:
      return 0u;
  }
  return 0u;
}

uint32_t FramebufferImpl::GetRenderPassCount() const
{
  return uint32_t(mRenderPasses.size());
}

RenderPassImpl* FramebufferImpl::GetRenderPass(uint32_t index) const
{
  if(index < mRenderPasses.size())
  {
    return mRenderPasses[index].renderPassImpl;
  }
  return nullptr;
}

RenderPassImpl* FramebufferImpl::GetImplFromRenderPass(RenderPass* renderPass)
{
  auto attachments  = renderPass->GetCreateInfo().attachments;
  auto matchLoadOp  = attachments->front().loadOp;
  auto matchStoreOp = attachments->front().storeOp;

  for(auto& element : mRenderPasses)
  {
    // Test renderpass first
    if(element.renderPass != nullptr)
    {
      auto firstAttachment = element.renderPass->GetCreateInfo().attachments->front();
      if(firstAttachment.loadOp == matchLoadOp &&
         firstAttachment.storeOp == matchStoreOp)
      {
        return element.renderPassImpl;
      }
    }
    else
    {
      DALI_ASSERT_DEBUG(element.renderPassImpl != nullptr && "Render pass list doesn't contain impl");
      auto createInfo = element.renderPassImpl->GetCreateInfo();

      if(createInfo.attachmentDescriptions[0].loadOp == VkLoadOpType(matchLoadOp).loadOp &&
         createInfo.attachmentDescriptions[0].storeOp == VkStoreOpType(matchStoreOp).storeOp)
      {
        // Point at passed in render pass... should be a weak ptr... What's lifecycle?!
        element.renderPass = renderPass;
        return element.renderPassImpl;
      }
    }
  }

  // @todo create new render pass from existing + load/store op, add it to mRenderPasses, and return it.
  // @todo Need to reconsider swapchain/fbo/renderpass creation model.
  // This framebuffer may belong to a swapchain, in which case, there are multiple framebuffers
  // that could share render passes.
  // A) Need to detect this situation - keep owner info?
  // B) Sharing render passes means we
  //    1) need to ref-count to ensure safe ownership, or
  //    2) move ownership of renderpass to swapchain.
  //       Onus is on client to determine which interface to use, if it's a surface, use swapchain;
  //       if it's an offscreen, use framebuffer. (Kinda need a core interface to wrap surface/offscreen)
  return mRenderPasses[0].renderPassImpl;
}

vk::Framebuffer FramebufferImpl::GetVkHandle() const
{
  return mFramebuffer;
}

std::vector<vk::ClearValue> FramebufferImpl::GetClearValues() const
{
  auto result = std::vector<vk::ClearValue>{};

  // @todo & color clear enabled / depth clear enabled
  for(auto& attachment : mAttachments)
  {
    result.emplace_back(attachment->GetClearValue());
  }

  return result;
}

} // namespace Dali::Graphics::Vulkan

// Namespace Graphics

// Namespace Dali
