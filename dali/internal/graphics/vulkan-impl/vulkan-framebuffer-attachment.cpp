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
 */

#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-attachment.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-view-impl.h>
#include <atomic>

namespace Dali::Graphics::Vulkan
{
FramebufferAttachment* FramebufferAttachment::NewColorAttachment(
  std::unique_ptr<ImageView>&            imageView,
  vk::ClearColorValue                    clearColorValue,
  const Graphics::AttachmentDescription* description,
  bool                                   presentable)
{
  assert(imageView->GetImage()->GetUsageFlags() & vk::ImageUsageFlagBits::eColorAttachment);

  auto attachment = new FramebufferAttachment(imageView,
                                              clearColorValue,
                                              description,
                                              AttachmentType::COLOR,
                                              presentable);
  return attachment;
}

FramebufferAttachment* FramebufferAttachment::NewDepthAttachment(
  std::unique_ptr<ImageView>&            imageView,
  vk::ClearDepthStencilValue             clearDepthStencilValue,
  const Graphics::AttachmentDescription* description)
{
  assert(imageView->GetImage()->GetUsageFlags() & vk::ImageUsageFlagBits::eDepthStencilAttachment);

  auto attachment = new FramebufferAttachment(imageView,
                                              clearDepthStencilValue,
                                              description,
                                              AttachmentType::DEPTH_STENCIL,
                                              false /* presentable */);

  return attachment;
}

FramebufferAttachment::FramebufferAttachment(
  std::unique_ptr<ImageView>&            imageView,
  vk::ClearValue                         clearColor,
  const Graphics::AttachmentDescription* description,
  AttachmentType                         type,
  bool                                   presentable)
: mClearValue(clearColor),
  mType(type)
{
  mImageView.swap(imageView);
  auto image = mImageView->GetImage();

  auto sampleCountFlags = image->GetSampleCount();

  mDescription.setSamples(sampleCountFlags);
  mDescription.setFormat(image->GetFormat());
  mDescription.setInitialLayout(vk::ImageLayout::eUndefined);
  if(description == nullptr)
  {
    mDescription.setLoadOp(vk::AttachmentLoadOp::eClear);
    mDescription.setStoreOp(vk::AttachmentStoreOp::eStore);
    mDescription.setStencilLoadOp(vk::AttachmentLoadOp::eClear);
    mDescription.setStencilStoreOp(vk::AttachmentStoreOp::eStore);
  }
  else
  {
    mDescription.setLoadOp(VkLoadOpType(description->loadOp).loadOp);
    mDescription.setStoreOp(VkStoreOpType(description->storeOp).storeOp);
    mDescription.setStencilLoadOp(VkLoadOpType(description->stencilLoadOp).loadOp);
    mDescription.setStencilStoreOp(VkStoreOpType(description->stencilStoreOp).storeOp);
  }

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
} //namespace Dali::Graphics::Vulkan
