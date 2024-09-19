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
 */

#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-view-impl.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

#include <dali/integration-api/debug.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{
RenderPassImpl* RenderPassImpl::New(
  Vulkan::Device&                            device,
  const std::vector<FramebufferAttachment*>& colorAttachments,
  FramebufferAttachment*                     depthAttachment)
{
  auto renderPass = new RenderPassImpl(device, colorAttachments, depthAttachment);
  return renderPass;
}

RenderPassImpl::RenderPassImpl(Vulkan::Device&                            device,
                               const std::vector<FramebufferAttachment*>& colorAttachments,
                               FramebufferAttachment*                     depthAttachment)
: mGraphicsDevice(&device)
{
  CreateCompatibleCreateInfo(colorAttachments, depthAttachment);
  CreateRenderPass();
}

RenderPassImpl::~RenderPassImpl() = default;

vk::RenderPass RenderPassImpl::GetVkHandle()
{
  return mVkRenderPass;
}

bool RenderPassImpl::OnDestroy()
{
  if(mVkRenderPass)
  {
    auto device     = mGraphicsDevice->GetLogicalDevice();
    auto allocator  = &mGraphicsDevice->GetAllocator();
    auto renderPass = mVkRenderPass;
    mGraphicsDevice->DiscardResource([device, renderPass, allocator]()
                                     {
      DALI_LOG_INFO(gVulkanFilter, Debug::General, "Invoking deleter function: swap chain->%p\n", static_cast<VkRenderPass>(renderPass))
      device.destroyRenderPass(renderPass, allocator); });

    mVkRenderPass = nullptr;
  }
  return false;
}

std::vector<vk::ImageView>& RenderPassImpl::GetAttachments()
{
  return mAttachments;
}

void RenderPassImpl::CreateCompatibleCreateInfo(
  const std::vector<FramebufferAttachment*>& colorAttachments,
  FramebufferAttachment*                     depthAttachment)
{
  auto hasDepth = false;
  if(depthAttachment)
  {
    hasDepth = depthAttachment->IsValid();
    assert(hasDepth && "Invalid depth attachment! The attachment has no ImageView");
  }

  // The total number of attachments
  auto totalAttachmentCount = hasDepth ? colorAttachments.size() + 1 : colorAttachments.size();
  mAttachments.clear();
  mAttachments.reserve(totalAttachmentCount);

  // This vector stores the attachment references
  mCreateInfo.colorAttachmentReferences.reserve(colorAttachments.size());

  // This vector stores the attachment descriptions
  mCreateInfo.attachmentDescriptions.reserve(totalAttachmentCount);

  // For each color attachment...
  for(auto i = 0u; i < colorAttachments.size(); ++i)
  {
    // Get the image layout
    auto imageLayout = colorAttachments[i]->GetImageView()->GetImage()->GetImageLayout();

    // If the layout is undefined...
    if(imageLayout == vk::ImageLayout::eUndefined)
    {
      // Set it to color attachment optimal
      imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    }

    // Any other case should be invalid
    assert(imageLayout == vk::ImageLayout::eColorAttachmentOptimal);

    // Add a reference and a descriptions and image views to their respective vectors
    mCreateInfo.colorAttachmentReferences.push_back(vk::AttachmentReference{}.setLayout(imageLayout).setAttachment(U32(i)));

    mCreateInfo.attachmentDescriptions.push_back(colorAttachments[i]->GetDescription());

    mAttachments.push_back(colorAttachments[i]->GetImageView()->GetVkHandle());
  }

  // Follow the exact same procedure as color attachments
  if(hasDepth)
  {
    auto imageLayout = depthAttachment->GetImageView()->GetImage()->GetImageLayout();

    if(imageLayout == vk::ImageLayout::eUndefined)
    {
      imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }

    assert(imageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal);

    mCreateInfo.depthAttachmentReference.setLayout(imageLayout);
    mCreateInfo.depthAttachmentReference.setAttachment(U32(mCreateInfo.colorAttachmentReferences.size()));

    mCreateInfo.attachmentDescriptions.push_back(depthAttachment->GetDescription());

    mAttachments.push_back(depthAttachment->GetImageView()->GetVkHandle());
  }

  // Creating a single subpass per framebuffer
  mCreateInfo.subpassDesc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
  mCreateInfo.subpassDesc.setColorAttachmentCount(U32(colorAttachments.size()));
  if(hasDepth)
  {
    mCreateInfo.subpassDesc.setPDepthStencilAttachment(&mCreateInfo.depthAttachmentReference);
  }
  mCreateInfo.subpassDesc.setPColorAttachments(mCreateInfo.colorAttachmentReferences.data());

  // Creating 2 subpass dependencies using VK_SUBPASS_EXTERNAL to leverage the implicit image layout
  // transitions provided by the driver
  mCreateInfo.subpassDependencies = {
    vk::SubpassDependency{}
      .setSrcSubpass(VK_SUBPASS_EXTERNAL)
      .setDstSubpass(0)
      .setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
      .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
      .setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
      .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
      .setDependencyFlags(vk::DependencyFlagBits::eByRegion),

    vk::SubpassDependency{}
      .setSrcSubpass(0)
      .setDstSubpass(VK_SUBPASS_EXTERNAL)
      .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
      .setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
      .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
      .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
      .setDependencyFlags(vk::DependencyFlagBits::eByRegion)};

  mCreateInfo.createInfo
    .setAttachmentCount(U32(mCreateInfo.attachmentDescriptions.size()))
    .setPAttachments(mCreateInfo.attachmentDescriptions.data())
    .setPSubpasses(&mCreateInfo.subpassDesc)
    .setSubpassCount(1)
    .setDependencyCount(2)
    .setPDependencies(mCreateInfo.subpassDependencies.data());
}

void RenderPassImpl::CreateRenderPass()
{
  mVkRenderPass = VkAssert(mGraphicsDevice->GetLogicalDevice().createRenderPass(mCreateInfo.createInfo, mGraphicsDevice->GetAllocator()));
}

} // namespace Dali::Graphics::Vulkan
