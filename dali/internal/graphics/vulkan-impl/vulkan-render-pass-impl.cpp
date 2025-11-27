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
RenderPassHandle RenderPassImpl::New(
  Vulkan::Device&             device,
  const SharedAttachments&    colorAttachments,
  FramebufferAttachmentHandle depthAttachment)
{
  auto renderPass = new RenderPassImpl(device, colorAttachments, depthAttachment);
  return RenderPassHandle(renderPass);
}

RenderPassHandle RenderPassImpl::New(
  Vulkan::Device&                   device,
  const RenderPassImpl::CreateInfo& createInfo)
{
  auto renderPass = new RenderPassImpl(device, createInfo);
  return RenderPassHandle(renderPass);
}

RenderPassImpl::RenderPassImpl(Vulkan::Device&             device,
                               const SharedAttachments&    colorAttachments,
                               FramebufferAttachmentHandle depthAttachment)
: mGraphicsDevice(&device),
  mHasDepthAttachment(bool(depthAttachment))
{
  // Default case is creating render pass for swapchain.
  CreateCompatibleCreateInfo(mCreateInfo, colorAttachments, depthAttachment, false);
  CreateRenderPass();
}

RenderPassImpl::RenderPassImpl(Vulkan::Device&                   device,
                               const RenderPassImpl::CreateInfo& createInfo)
: mGraphicsDevice(&device),
  mCreateInfo(createInfo)
{
  CreateRenderPass();
}

RenderPassImpl::~RenderPassImpl() = default;

bool RenderPassImpl::OnDestroy()
{
  if(mVkRenderPass)
  {
    auto device     = mGraphicsDevice->GetLogicalDevice();
    auto allocator  = &mGraphicsDevice->GetAllocator();
    auto renderPass = mVkRenderPass;

    DALI_LOG_INFO(gVulkanFilter, Debug::General, "Destroying render pass: %p\n", static_cast<VkRenderPass>(renderPass));
    device.destroyRenderPass(renderPass, allocator);

    mVkRenderPass = nullptr;
  }
  return false;
}

vk::RenderPass RenderPassImpl::GetVkHandle()
{
  return mVkRenderPass;
}

size_t RenderPassImpl::GetAttachmentCount()
{
  return mHasDepthAttachment + mCreateInfo.colorAttachmentReferences.size();
}

bool RenderPassImpl::IsCompatible(RenderPassHandle rhs)
{
  if((mCreateInfo.colorAttachmentReferences.size() == rhs->mCreateInfo.colorAttachmentReferences.size()) &&
     (mHasDepthAttachment == rhs->mHasDepthAttachment))
  {
    bool equal = true;
    for(size_t i = 0; i < mCreateInfo.attachmentHandles.size(); ++i)
    {
      if(mCreateInfo.attachmentHandles[i]->GetType() != rhs->mCreateInfo.attachmentHandles[i]->GetType())
      {
        equal = false;
        break;
      }
      if(mCreateInfo.attachmentHandles[i]->GetDescription().format != rhs->mCreateInfo.attachmentHandles[i]->GetDescription().format)
      {
        equal = false;
        break;
      }
      if(mCreateInfo.attachmentHandles[i]->GetDescription().flags != rhs->mCreateInfo.attachmentHandles[i]->GetDescription().flags)
      {
        equal = false;
        break;
      }
    }
    return equal;
  }
  return false;
}

void RenderPassImpl::CreateCompatibleCreateInfo(
  CreateInfo&                        createInfo,
  const SharedAttachments&           colorAttachments,
  const FramebufferAttachmentHandle& depthAttachment,
  bool                               subpassForOffscreen)
{
  auto hasDepth = false;
  if(depthAttachment)
  {
    hasDepth = depthAttachment->IsValid();
    assert(hasDepth && "Invalid depth attachment! The attachment has no ImageView");
  }

  // The total number of attachments
  auto totalAttachmentCount = hasDepth ? colorAttachments.size() + 1 : colorAttachments.size();

  createInfo.attachmentHandles.reserve(colorAttachments.size() + depthAttachment);
  for(auto& handle : colorAttachments)
  {
    createInfo.attachmentHandles.push_back(handle);
  }
  if(depthAttachment)
  {
    createInfo.attachmentHandles.push_back(depthAttachment);
  }

  // This vector stores the attachment references
  createInfo.colorAttachmentReferences.reserve(colorAttachments.size());

  // This vector stores the attachment descriptions
  createInfo.attachmentDescriptions.reserve(totalAttachmentCount);

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
    createInfo.colorAttachmentReferences.push_back(vk::AttachmentReference{}.setLayout(imageLayout).setAttachment(U32(i)));

    createInfo.attachmentDescriptions.push_back(colorAttachments[i]->GetDescription());
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

    createInfo.depthAttachmentReference.setLayout(imageLayout);
    createInfo.depthAttachmentReference.setAttachment(U32(createInfo.colorAttachmentReferences.size()));

    createInfo.attachmentDescriptions.push_back(depthAttachment->GetDescription());
  }

  createInfo.subpassDesc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
  createInfo.subpassDesc.setColorAttachmentCount(U32(colorAttachments.size()));
  if(hasDepth)
  {
    createInfo.subpassDesc.setPDepthStencilAttachment(&createInfo.depthAttachmentReference);
  }
  createInfo.subpassDesc.setPColorAttachments(createInfo.colorAttachmentReferences.data());

  auto dependencyCount = CreateSubPassDependencies(createInfo, hasDepth, subpassForOffscreen);

  createInfo.createInfo
    .setAttachmentCount(U32(createInfo.attachmentDescriptions.size()))
    .setPAttachments(createInfo.attachmentDescriptions.data())
    .setPSubpasses(&createInfo.subpassDesc)
    .setSubpassCount(1)
    .setDependencyCount(dependencyCount)
    .setPDependencies(createInfo.subpassDependencies.data());
}

void RenderPassImpl::CreateMatchingInfo(
  RenderPassHandle  renderPassImpl,
  AttachmentLoadOp  loadOp,
  AttachmentStoreOp storeOp,
  CreateInfo&       createInfo)
{
  auto rhs = renderPassImpl->GetCreateInfo();

  for(auto& attachmentHandle : rhs.attachmentHandles)
  {
    createInfo.attachmentHandles.push_back(attachmentHandle);
  }
  for(auto& colorAttachment : rhs.colorAttachmentReferences)
  {
    createInfo.colorAttachmentReferences.push_back(colorAttachment);
  }
  createInfo.depthAttachmentReference = rhs.depthAttachmentReference;
  for(auto description : rhs.attachmentDescriptions)
  {
    description.loadOp         = VkLoadOpType(loadOp).loadOp;
    description.storeOp        = VkStoreOpType(storeOp).storeOp;
    description.stencilLoadOp  = VkLoadOpType(loadOp).loadOp;
    description.stencilStoreOp = VkStoreOpType(storeOp).storeOp;
    if(loadOp == Graphics::AttachmentLoadOp::LOAD)
    {
      description.initialLayout = description.finalLayout;
    }
    createInfo.attachmentDescriptions.push_back(description);
  }

  createInfo.subpassDesc         = rhs.subpassDesc;
  createInfo.subpassDependencies = rhs.subpassDependencies;
  createInfo.createInfo
    .setAttachmentCount(U32(createInfo.attachmentDescriptions.size()))
    .setPAttachments(createInfo.attachmentDescriptions.data())
    .setPSubpasses(&createInfo.subpassDesc)
    .setSubpassCount(1)
    .setDependencyCount(createInfo.subpassDependencies.size())
    .setPDependencies(createInfo.subpassDependencies.data());
}

void RenderPassImpl::CreateRenderPass()
{
  mVkRenderPass = VkAssert(mGraphicsDevice->GetLogicalDevice().createRenderPass(mCreateInfo.createInfo, mGraphicsDevice->GetAllocator()));
}

int RenderPassImpl::CreateSubPassDependencies(CreateInfo& createInfo, bool hasDepth, bool subpassForOffscreen)
{
  int dependencyCount = 0;

  if(subpassForOffscreen)
  {
    createInfo.subpassDependencies = {
      vk::SubpassDependency{}
        .setSrcSubpass(vk::SubpassExternal)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
        .setSrcAccessMask(vk::AccessFlagBits::eNone)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite),
      vk::SubpassDependency{}
        .setSrcSubpass(0) // Self-dependency for subpass 0 to allow pipeline barriers within the same subpass
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead)
        .setDependencyFlags(vk::DependencyFlagBits::eByRegion),
      vk::SubpassDependency{}
        .setSrcSubpass(0)
        .setDstSubpass(vk::SubpassExternal)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
        .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)};

    dependencyCount = 3;
  }
  else // Subpass for swapchain
  {
    // Creating 3 subpass dependencies using VK_SUBPASS_EXTERNAL to leverage the implicit image layout
    // transitions provided by the driver, plus self-dependency for blend barrier
    vk::AccessFlags        accessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);
    vk::PipelineStageFlags stageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    if(hasDepth)
    {
      accessMask |= vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
      stageMask |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
    }

    createInfo.subpassDependencies = {
      vk::SubpassDependency{}
        .setSrcSubpass(vk::SubpassExternal)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
        .setDstStageMask(stageMask)
        .setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
        .setDstAccessMask(accessMask)
        .setDependencyFlags(vk::DependencyFlagBits::eByRegion),
      vk::SubpassDependency{}
        .setSrcSubpass(0) // Self-dependency for subpass 0 to allow pipeline barriers within the same subpass
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead)
        .setDependencyFlags(vk::DependencyFlagBits::eByRegion),
      vk::SubpassDependency{}
        .setSrcSubpass(0)
        .setDstSubpass(vk::SubpassExternal)
        .setSrcStageMask(stageMask)
        .setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
        .setSrcAccessMask(accessMask)
        .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
        .setDependencyFlags(vk::DependencyFlagBits::eByRegion)};

    dependencyCount = 3;
  }
  return dependencyCount;
}

} // namespace Dali::Graphics::Vulkan
