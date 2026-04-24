/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include <dali/internal/graphics/vulkan-impl/vulkan-handle.h>
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

FramebufferImpl* FramebufferImpl::New(
  Vulkan::Device&    device,
  RenderPassHandle   renderPass,
  SharedAttachments& attachments,
  uint32_t           width,
  uint32_t           height,
  bool               hasDepthAttachments)
{
  DALI_ASSERT_ALWAYS(renderPass && "You require more render passes!");

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
                             renderPass,
                             width,
                             height,
                             hasDepthAttachments);
}

FramebufferImpl* FramebufferImpl::New(
  Vulkan::Device&             device,
  RenderPassHandle            renderPass,
  SharedAttachments&          colorAttachments,
  FramebufferAttachmentHandle depthAttachment,
  uint32_t                    width,
  uint32_t                    height)
{
  assert((!colorAttachments.empty() || depthAttachment) && "Cannot create framebuffer. Please provide at least one attachment");

  [[maybe_unused]] auto colorAttachmentsValid = true;

  SharedAttachments attachments;
  for(auto& attachment : colorAttachments)
  {
    attachments.emplace_back(attachment);
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
  if(!renderPass)
  {
    // Create compatible vulkan render pass
    renderPass = RenderPassImpl::New(device, attachments, depthAttachment);
  }

  if(hasDepth)
  {
    attachments.emplace_back(std::move(depthAttachment));
  }
  return FramebufferImpl::New(device, renderPass, attachments, width, height, hasDepth);
}

FramebufferImpl::FramebufferImpl(Device&            graphicsDevice,
                                 SharedAttachments& attachments,
                                 vk::Framebuffer    vkHandle,
                                 RenderPassHandle   renderPassImpl,
                                 uint32_t           width,
                                 uint32_t           height,
                                 bool               hasDepthAttachment)
: mGraphicsDevice(&graphicsDevice),
  mWidth(width),
  mHeight(height),
  mAttachments(attachments),
  mFramebuffer(vkHandle),
  mHasDepthAttachment(hasDepthAttachment)
{
  mRenderPasses.emplace_back(RenderPassMapElement{nullptr, renderPassImpl});
}

void FramebufferImpl::Destroy()
{
  // Deregister from any RenderPasses we observe. Each non-null entry was
  // registered via AddLifecycleObserver; RenderPass::mLifecycleObservers is
  // a set so double-removal (if two entries shared a renderPass) is a no-op.
  for(auto& element : mRenderPasses)
  {
    if(element.renderPass)
    {
      element.renderPass->RemoveLifecycleObserver(*this);
    }
  }

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

void FramebufferImpl::RenderPassInvalidated(const Vulkan::RenderPass* renderPass)
{
  // Forget the front-end RenderPass association, but keep the backend
  // RenderPassImpl handle — mRenderPasses[0] in particular is the "primary"
  // impl this framebuffer was built with and is used as the template in
  // CreateMatchingInfo; removing it would leave the vector empty and cause
  // an out-of-bounds read on the next GetImplFromRenderPass call.
  //
  // Nulling element.renderPass forces the next lookup into the match-by-impl
  // branch, which will either re-associate this impl with the reinitialized
  // front-end (if load/store ops still match) or fall through to create a
  // new impl for the new ops. Attachment-shape changes are handled separately
  // by the Vulkan swapchain rebuild path, so we don't need to evict impls
  // here.
  //
  // We intentionally stay registered with the RenderPass — on reinitialize,
  // the RenderPass keeps living, and future re-associations should also
  // receive invalidations. For the destruction path, the RenderPass
  // destructor drops its observer set on its way out.
  for(auto& element : mRenderPasses)
  {
    if(element.renderPass == renderPass)
    {
      element.renderPass = nullptr;
    }
  }
}

uint32_t FramebufferImpl::GetWidth() const
{
  return mWidth;
}

uint32_t FramebufferImpl::GetHeight() const
{
  return mHeight;
}

FramebufferAttachmentHandle FramebufferImpl::GetAttachment(AttachmentType type, uint32_t index) const
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
      {
        return mAttachments.back();
      }
    }
    case AttachmentType::INPUT:
    case AttachmentType::RESOLVE:
    case AttachmentType::PRESERVE:
    case AttachmentType::UNDEFINED:
      break;
  }

  return {};
}

SharedAttachments FramebufferImpl::GetAttachments(AttachmentType type) const
{
  auto retval = SharedAttachments{};
  switch(type)
  {
    case AttachmentType::COLOR:
    {
      auto numColorAttachments = mHasDepthAttachment ? mAttachments.size() - 1 : mAttachments.size();
      retval.reserve(numColorAttachments);
      for(size_t i = 0; i < numColorAttachments; ++i)
      {
        retval.emplace_back(&*mAttachments[i]);
      }
      break;
    }
    case AttachmentType::DEPTH_STENCIL:
    {
      if(mHasDepthAttachment)
      {
        retval.reserve(1);
        retval.emplace_back(&*mAttachments.back());
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

RenderPassHandle FramebufferImpl::GetRenderPass(uint32_t index) const
{
  if(index < mRenderPasses.size())
  {
    return mRenderPasses[index].renderPassImpl;
  }
  return RenderPassHandle{};
}

RenderPassHandle FramebufferImpl::GetImplFromRenderPass(const RenderPass* renderPass)
{
  auto attachments  = renderPass->GetCreateInfo().attachments;
  auto matchLoadOp  = attachments->front().loadOp;
  auto matchStoreOp = attachments->front().storeOp;

  for(auto& element : mRenderPasses)
  {
    // Test renderpass first
    if(element.renderPass != nullptr)
    {
      auto& attachments = element.renderPass->GetCreateInfo().attachments;
      if(attachments && !attachments->empty())
      {
        auto firstAttachment = attachments->front();
        if(firstAttachment.loadOp == matchLoadOp &&
           firstAttachment.storeOp == matchStoreOp)
        {
          return element.renderPassImpl;
        }
      }
      else
      {
        DALI_LOG_ERROR("Framebuffer's Renderpass has no attachments\n");
      }
    }
    else
    {
      DALI_ASSERT_DEBUG(element.renderPassImpl && "Render pass list doesn't contain impl");
      auto createInfo = element.renderPassImpl->GetCreateInfo();

      if(createInfo.attachmentDescriptions[0].loadOp == VkLoadOpType(matchLoadOp).loadOp &&
         createInfo.attachmentDescriptions[0].storeOp == VkStoreOpType(matchStoreOp).storeOp)
      {
        element.renderPass = const_cast<RenderPass*>(renderPass);
        element.renderPass->AddLifecycleObserver(*this);
        return element.renderPassImpl;
      }
    }
  }

  RenderPassImpl::CreateInfo createInfo;
  RenderPassImpl::CreateMatchingInfo(mRenderPasses[0].renderPassImpl, matchLoadOp, matchStoreOp, createInfo);
  auto  renderPassImpl = RenderPassHandle(RenderPassImpl::New(*mGraphicsDevice, createInfo));
  auto* rawRenderPass  = const_cast<RenderPass*>(renderPass);
  mRenderPasses.emplace_back(RenderPassMapElement{rawRenderPass, renderPassImpl});
  rawRenderPass->AddLifecycleObserver(*this);

  return renderPassImpl;
}

void FramebufferImpl::AddRenderPass(RenderPass* renderPass, Vulkan::RenderPassHandle renderPassImpl)
{
  bool found = false;
  for(auto& element : mRenderPasses)
  {
    if(element.renderPassImpl == renderPassImpl && !element.renderPass)
    {
      element.renderPass = renderPass; // Update existing element with matched objects
      found              = true;
      break;
    }
  }
  if(!found)
  {
    mRenderPasses.emplace_back(RenderPassMapElement{renderPass, renderPassImpl});
  }
  if(renderPass)
  {
    renderPass->AddLifecycleObserver(*this);
  }
}

vk::Framebuffer FramebufferImpl::GetVkHandle() const
{
  return mFramebuffer;
}

std::vector<vk::ClearValue> FramebufferImpl::GetClearValues() const
{
  auto result = std::vector<vk::ClearValue>{};

  for(auto& attachment : mAttachments)
  {
    result.emplace_back(attachment->GetClearValue());
  }

  return result;
}

} // namespace Dali::Graphics::Vulkan

// Namespace Graphics

// Namespace Dali
