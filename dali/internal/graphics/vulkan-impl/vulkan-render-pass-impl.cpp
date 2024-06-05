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

#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-view-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

#include <dali/integration-api/debug.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{

namespace {

vk::RenderPass CreateCompatibleRenderPass(
  Vulkan::Device& device,
  const std::vector< FramebufferAttachment* >& colorAttachments,
  FramebufferAttachment* depthAttachment,
  std::vector< vk::ImageView >& attachments)
{
  auto hasDepth = false;
  if( depthAttachment )
  {
    hasDepth = depthAttachment->IsValid();
    assert( hasDepth && "Invalid depth attachment! The attachment has no ImageView" );
  }

  // The total number of attachments
  auto totalAttachmentCount = hasDepth ? colorAttachments.size() + 1 : colorAttachments.size();
  attachments.clear();
  attachments.reserve( totalAttachmentCount );

  // This vector stores the attachment references
  auto colorAttachmentReferences = std::vector< vk::AttachmentReference >{};
  colorAttachmentReferences.reserve( colorAttachments.size() );

  // This vector stores the attachment descriptions
  auto attachmentDescriptions = std::vector< vk::AttachmentDescription >{};
  attachmentDescriptions.reserve( totalAttachmentCount );

  // For each color attachment...
  for( auto i = 0u; i < colorAttachments.size(); ++i )
  {
    // Get the image layout
    auto imageLayout = colorAttachments[i]->GetImageView()->GetImage()->GetImageLayout();

    // If the layout is undefined...
    if( imageLayout == vk::ImageLayout::eUndefined )
    {
      // Set it to color attachment optimal
      imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    }

    // Any other case should be invalid
    assert( imageLayout == vk::ImageLayout::eColorAttachmentOptimal );

    // Add a reference and a descriptions and image views to their respective vectors
    colorAttachmentReferences.push_back( vk::AttachmentReference{}.setLayout( imageLayout )
                                                                  .setAttachment( U32( i ) ) );

    attachmentDescriptions.push_back( colorAttachments[i]->GetDescription() );

    attachments.push_back( colorAttachments[i]->GetImageView()->GetVkHandle() );
  }


  // Follow the exact same procedure as color attachments
  auto depthAttachmentReference = vk::AttachmentReference{};
  if( hasDepth )
  {
    auto imageLayout = depthAttachment->GetImageView()->GetImage()->GetImageLayout();

    if( imageLayout == vk::ImageLayout::eUndefined )
    {
      imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }

    assert( imageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal );

    depthAttachmentReference.setLayout( imageLayout );
    depthAttachmentReference.setAttachment( U32( colorAttachmentReferences.size() ) );

    attachmentDescriptions.push_back( depthAttachment->GetDescription() );

    attachments.push_back( depthAttachment->GetImageView()->GetVkHandle() );
  }

  // Creating a single subpass per framebuffer
  auto subpassDesc = vk::SubpassDescription{};
  subpassDesc.setPipelineBindPoint( vk::PipelineBindPoint::eGraphics );
  subpassDesc.setColorAttachmentCount( U32( colorAttachments.size()));
  if( hasDepth )
  {
    subpassDesc.setPDepthStencilAttachment( &depthAttachmentReference );
  }
  subpassDesc.setPColorAttachments( colorAttachmentReferences.data() );

  // Creating 2 subpass dependencies using VK_SUBPASS_EXTERNAL to leverage the implicit image layout
  // transitions provided by the driver
  std::array< vk::SubpassDependency, 2 > subpassDependencies{

    vk::SubpassDependency{}.setSrcSubpass( VK_SUBPASS_EXTERNAL )
                           .setDstSubpass( 0 )
                           .setSrcStageMask( vk::PipelineStageFlagBits::eBottomOfPipe )
                           .setDstStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput )
                           .setSrcAccessMask( vk::AccessFlagBits::eMemoryRead )
                           .setDstAccessMask( vk::AccessFlagBits::eColorAttachmentRead |
                                              vk::AccessFlagBits::eColorAttachmentWrite )
                           .setDependencyFlags( vk::DependencyFlagBits::eByRegion ),

    vk::SubpassDependency{}.setSrcSubpass( 0 )
                           .setDstSubpass( VK_SUBPASS_EXTERNAL )
                           .setSrcStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput )
                           .setDstStageMask( vk::PipelineStageFlagBits::eBottomOfPipe )
                           .setSrcAccessMask( vk::AccessFlagBits::eColorAttachmentRead |
                                              vk::AccessFlagBits::eColorAttachmentWrite )
                           .setDstAccessMask( vk::AccessFlagBits::eMemoryRead )
                           .setDependencyFlags( vk::DependencyFlagBits::eByRegion )

  };

  // Create the render pass
  auto renderPassCreateInfo = vk::RenderPassCreateInfo{}.setAttachmentCount( U32( attachmentDescriptions.size() ) )
                                                        .setPAttachments( attachmentDescriptions.data() )
                                                        .setPSubpasses( &subpassDesc )
                                                        .setSubpassCount( 1 )
                                                        .setPDependencies( subpassDependencies.data() );

  auto vkRenderPass = VkAssert(device.GetLogicalDevice().createRenderPass( renderPassCreateInfo, device.GetAllocator()));
  return vkRenderPass;
}

} // namespace


RenderPassImpl* RenderPassImpl::NewRenderPass(
  Vulkan::Device& device,
  const std::vector< FramebufferAttachment* >& colorAttachments,
  FramebufferAttachment* depthAttachment)
{
  std::vector< vk::ImageView > attachments{};
  auto vkRenderPass = CreateCompatibleRenderPass(device, colorAttachments, depthAttachment, attachments);

  auto renderPass = new RenderPassImpl(device, vkRenderPass);
  renderPass->SetAttachments(attachments);
  return renderPass;
}

RenderPassImpl::RenderPassImpl(Vulkan::Device& device, vk::RenderPass renderPass)
: mGraphicsDevice(&device),
  mVkRenderPass(renderPass)
{
}

RenderPassImpl::RenderPassImpl(const Graphics::RenderPassCreateInfo& createInfo,
  VulkanGraphicsController& controller,
  std::vector<FramebufferAttachment*>& colorAttachments,
  FramebufferAttachment* depthAttachment)
: mGraphicsDevice(&controller.GetGraphicsDevice())
{
  // @todo Do mostly as CreateCompatibleRenderPass above, but instead, get the attachment
  // descriptions from the createInfo passed in here.
  mVkRenderPass = CreateCompatibleRenderPass(*mGraphicsDevice, colorAttachments, depthAttachment, mAttachments);
}

RenderPassImpl::~RenderPassImpl() = default;

vk::RenderPass RenderPassImpl::GetVkHandle()
{
  return mVkRenderPass;
}

bool RenderPassImpl::OnDestroy()
{
  if( mVkRenderPass )
  {
    auto device = mGraphicsDevice->GetLogicalDevice();
    auto allocator = &mGraphicsDevice->GetAllocator();
    auto renderPass = mVkRenderPass;
    mGraphicsDevice->DiscardResource( [ device, renderPass, allocator ]() {
      DALI_LOG_INFO( gVulkanFilter, Debug::General, "Invoking deleter function: swap chain->%p\n",
                     static_cast< VkRenderPass >(renderPass) )
      device.destroyRenderPass( renderPass, allocator );
    } );

    mVkRenderPass = nullptr;
  }
  return false;
}

std::vector< vk::ImageView >& RenderPassImpl::GetAttachments()
{
  return mAttachments;
}

void RenderPassImpl::SetAttachments(std::vector< vk::ImageView >& attachments)
{
  mAttachments = std::move(attachments);
}

} // namespace Dali::Graphics::Vulkan
