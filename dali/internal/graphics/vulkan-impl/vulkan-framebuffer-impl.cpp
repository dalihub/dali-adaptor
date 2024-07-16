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
#include <dali/internal/graphics/vulkan/vulkan-device.h>

#include <dali/integration-api/debug.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{

FramebufferAttachment* FramebufferAttachment::NewColorAttachment(ImageView*          imageView,
                                                                 vk::ClearColorValue clearColorValue,
                                                                 bool                presentable)
{
  assert(imageView->GetImage()->GetUsageFlags() & vk::ImageUsageFlagBits::eColorAttachment);

  auto attachment = new FramebufferAttachment(imageView,
                                              clearColorValue,
                                              AttachmentType::COLOR,
                                              presentable);
  return attachment;
}

FramebufferAttachment* FramebufferAttachment::NewDepthAttachment(
  ImageView*                 imageView,
  vk::ClearDepthStencilValue clearDepthStencilValue)
{
  assert(imageView->GetImage()->GetUsageFlags() & vk::ImageUsageFlagBits::eDepthStencilAttachment);

  auto attachment = new FramebufferAttachment(imageView,
                                              clearDepthStencilValue,
                                              AttachmentType::DEPTH_STENCIL,
                                              false /* presentable */);

  return attachment;
}

FramebufferAttachment::FramebufferAttachment(ImageView*     imageView,
                                             vk::ClearValue clearColor,
                                             AttachmentType type,
                                             bool           presentable)
: mImageView(imageView),
  mClearValue(clearColor),
  mType(type)
{
  auto image = imageView->GetImage();

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
  return mImageView;
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
  return mImageView;
}

// FramebufferImpl -------------------------------

FramebufferImpl* FramebufferImpl::New(
  Vulkan::Device&                      device,
  RenderPassImpl*                      renderPass,
  std::vector<FramebufferAttachment*>& attachments,
  uint32_t                             width,
  uint32_t                             height,
  bool                                 hasDepthAttachments,
  bool                                 takeRenderPassOwnership)
{
  std::vector<vk::ImageView> imageViewAttachments;

  std::transform(attachments.cbegin(),
                 attachments.cend(),
                 std::back_inserter(imageViewAttachments),
                 [&](FramebufferAttachment* entry)
                 {
                   return entry->GetImageView()->GetVkHandle();
                 });

  auto framebufferCreateInfo = vk::FramebufferCreateInfo{}.setRenderPass(renderPass->GetVkHandle()).setPAttachments(imageViewAttachments.data()).setLayers(1).setWidth(width).setHeight(height).setAttachmentCount(U32(attachments.size()));

  auto vkFramebuffer = VkAssert(device.GetLogicalDevice().createFramebuffer(framebufferCreateInfo, device.GetAllocator()));

  return new FramebufferImpl(device,
                             attachments,
                             vkFramebuffer,
                             renderPass->GetVkHandle(),
                             width,
                             height,
                             hasDepthAttachments,
                             takeRenderPassOwnership);
}

FramebufferImpl* FramebufferImpl::New(
  Vulkan::Device&                            device,
  RenderPassImpl*                            renderPass,
  const std::vector<FramebufferAttachment*>& colorAttachments,
  FramebufferAttachment*                     depthAttachment,
  uint32_t                                   width,
  uint32_t                                   height)
{
  assert((!colorAttachments.empty() || depthAttachment) && "Cannot create framebuffer. Please provide at least one attachment");

  auto colorAttachmentsValid = true;
  for(auto& attachment : colorAttachments)
  {
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
  auto attachments = std::vector<FramebufferAttachment*>{};

  // Flag that indicates if the render pass is externally provided
  bool renderPassTakeOwnership = false;
  if(renderPass == nullptr)
  {
    renderPass              = RenderPassImpl::New(device, colorAttachments, depthAttachment);
    renderPassTakeOwnership = true;
  }
  attachments.reserve(colorAttachments.size());
  attachments.insert(attachments.begin(), colorAttachments.begin(), colorAttachments.end());
  if(hasDepth)
  {
    attachments.push_back(depthAttachment);
  }
  return FramebufferImpl::New(device, renderPass, attachments, width, height, hasDepth, renderPassTakeOwnership);
}

FramebufferImpl::FramebufferImpl(Device&                                    graphicsDevice,
                                 const std::vector<FramebufferAttachment*>& attachments,
                                 vk::Framebuffer                            vkHandle,
                                 vk::RenderPass                             renderPass,
                                 uint32_t                                   width,
                                 uint32_t                                   height,
                                 bool                                       hasDepthAttachment,
                                 bool                                       takeRenderPassOwnership)
: mGraphicsDevice(&graphicsDevice),
  mWidth(width),
  mHeight(height),
  mAttachments(attachments),
  mFramebuffer(vkHandle),
  mRenderPass(renderPass),
  mHasDepthAttachment(hasDepthAttachment),
  mRenderPassOwned(takeRenderPassOwnership)
{
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
      return mAttachments[index];
    }
    case AttachmentType::DEPTH_STENCIL:
    {
      if(mHasDepthAttachment)
        return mAttachments.back();
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
      retval.insert(retval.end(), mAttachments.begin(), mAttachments.begin() + numColorAttachments);
      break;
    }
    case AttachmentType::DEPTH_STENCIL:
    {
      if(mHasDepthAttachment)
      {
        retval.reserve(1);
        retval.push_back(mAttachments.back());
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

vk::RenderPass FramebufferImpl::GetRenderPass() const
{
  return mRenderPass;
}

vk::Framebuffer FramebufferImpl::GetVkHandle() const
{
  return mFramebuffer;
}

std::vector<vk::ClearValue> FramebufferImpl::GetClearValues() const
{
  auto result = std::vector<vk::ClearValue>{};

  std::transform(mAttachments.begin(), // @todo & color clear enabled / depth clear enabled
                 mAttachments.end(),
                 std::back_inserter(result),
                 [](FramebufferAttachment* attachment)
                 {
                   return attachment->GetClearValue();
                 });

  return result;
}

bool FramebufferImpl::OnDestroy()
{
  auto device      = mGraphicsDevice->GetLogicalDevice();
  auto frameBuffer = mFramebuffer;

  vk::RenderPass renderPass = mRenderPassOwned ? mRenderPass : vk::RenderPass{};

  auto allocator = &mGraphicsDevice->GetAllocator();

  mGraphicsDevice->DiscardResource([device, frameBuffer, renderPass, allocator]()
                                   {
                                     DALI_LOG_INFO(gVulkanFilter, Debug::General, "Invoking deleter function: framebuffer->%p\n", static_cast<VkFramebuffer>(frameBuffer))
                                     device.destroyFramebuffer(frameBuffer, allocator);

                                     if(renderPass)
                                     {
                                       DALI_LOG_INFO(gVulkanFilter, Debug::General, "Invoking deleter function: render pass->%p\n", static_cast<VkRenderPass>(renderPass))
                                       device.destroyRenderPass(renderPass, allocator);
                                     }
                                   });

  return false;
}

} // namespace Dali::Graphics::Vulkan

// Namespace Graphics

// Namespace Dali
