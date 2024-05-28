/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-view-impl.h>

#include <dali/integration-api/debug.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{

FramebufferAttachment* FramebufferAttachment::NewColorAttachment(ImageView* imageView,
                                                                 vk::ClearColorValue clearColorValue,
                                                                 bool presentable )
{
  assert( imageView->GetImage()->GetUsageFlags() & vk::ImageUsageFlagBits::eColorAttachment );

  auto attachment = new FramebufferAttachment( imageView,
                                               clearColorValue,
                                               AttachmentType::COLOR,
                                               presentable );
  return attachment;
}

FramebufferAttachment* FramebufferAttachment::NewDepthAttachment(
  ImageView* imageView,
  vk::ClearDepthStencilValue clearDepthStencilValue )
{
  assert( imageView->GetImage()->GetUsageFlags() & vk::ImageUsageFlagBits::eDepthStencilAttachment );

  auto attachment = new FramebufferAttachment( imageView,
                                               clearDepthStencilValue,
                                               AttachmentType::DEPTH_STENCIL,
                                               false /* presentable */ );

  return attachment;
}

FramebufferAttachment::FramebufferAttachment( ImageView* imageView,
                                              vk::ClearValue clearColor,
                                              AttachmentType type,
                                              bool presentable )
: mImageView( imageView ),
  mClearValue( clearColor ),
  mType( type )
{
  auto image = imageView->GetImage();

  auto sampleCountFlags = image->GetSampleCount();

  mDescription.setSamples( sampleCountFlags );

  mDescription.setStoreOp( vk::AttachmentStoreOp::eStore );
  mDescription.setStencilStoreOp( vk::AttachmentStoreOp::eStore );
  mDescription.setStencilLoadOp( vk::AttachmentLoadOp::eClear );
  mDescription.setFormat( image->GetFormat() );
  mDescription.setInitialLayout( vk::ImageLayout::eUndefined );
  mDescription.setLoadOp( vk::AttachmentLoadOp::eClear );

  if( type == AttachmentType::DEPTH_STENCIL )
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


//FramebufferImpl -------------------------------
FramebufferImpl::FramebufferImpl( Device& graphicsDevice,
                          const std::vector< FramebufferAttachment* >& colorAttachments,
                          FramebufferAttachment* depthAttachment,
                          vk::Framebuffer vkHandle,
                          vk::RenderPass renderPass,
                          uint32_t width,
                          uint32_t height,
                          bool externalRenderPass )
: mGraphicsDevice( &graphicsDevice ),
  mWidth( width ),
  mHeight( height ),
  mColorAttachments( colorAttachments ),
  mDepthAttachment( depthAttachment ),
  mFramebuffer( vkHandle ),
  mRenderPass( renderPass ),
  mExternalRenderPass( externalRenderPass )
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

FramebufferAttachment* FramebufferImpl::GetAttachment( AttachmentType type, uint32_t index ) const
{
  switch( type )
  {
    case AttachmentType::COLOR:
    {
      return mColorAttachments[index];
    }
    case AttachmentType::DEPTH_STENCIL:
    {
      return mDepthAttachment;
    }
    case AttachmentType::INPUT:
    case AttachmentType::RESOLVE:
    case AttachmentType::PRESERVE:
    case AttachmentType::UNDEFINED:
      break;
  }

  return nullptr;
}

std::vector< FramebufferAttachment* > FramebufferImpl::GetAttachments( AttachmentType type ) const
{
  auto retval = std::vector< FramebufferAttachment* >{};
  switch( type )
  {
    case AttachmentType::COLOR:
    {
      retval.reserve( mColorAttachments.size() );
      retval.insert( retval.end(), mColorAttachments.begin(), mColorAttachments.end() );
      break;
    }
    case AttachmentType::DEPTH_STENCIL:
    {
      retval.reserve( 1 );
      retval.push_back( mDepthAttachment );
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

uint32_t FramebufferImpl::GetAttachmentCount( AttachmentType type ) const
{
  switch( type )
  {
    case AttachmentType::COLOR:
    {
      return U32( mColorAttachments.size() );
    }
    case AttachmentType::DEPTH_STENCIL:
    {
      return mDepthAttachment->IsValid() ? 1u : 0u;
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

std::vector< vk::ClearValue > FramebufferImpl::GetClearValues() const
{
  auto result = std::vector< vk::ClearValue >{};

  std::transform( mColorAttachments.begin(), // @todo & color clear enabled
                  mColorAttachments.end(),
                  std::back_inserter( result ),
                  []( FramebufferAttachment* attachment ) {
                    return attachment->GetClearValue();
                  } );

  if( mDepthAttachment && mDepthAttachment->IsValid() ) // @todo & depth clear enabled
  {
    result.push_back( mDepthAttachment->GetClearValue() );
  }

  return result;
}

bool FramebufferImpl::OnDestroy()
{
  auto device = mGraphicsDevice->GetDevice();
  auto frameBuffer = mFramebuffer;

  vk::RenderPass renderPass = mExternalRenderPass ? vk::RenderPass{} : mRenderPass;

  auto allocator = &mGraphicsDevice->GetAllocator();

  mGraphicsDevice->DiscardResource( [ device, frameBuffer, renderPass, allocator ]() {

    DALI_LOG_INFO( gVulkanFilter, Debug::General, "Invoking deleter function: framebuffer->%p\n",
                   static_cast< VkFramebuffer >(frameBuffer) )
    device.destroyFramebuffer( frameBuffer, allocator );

    if( renderPass )
    {
      DALI_LOG_INFO( gVulkanFilter, Debug::General, "Invoking deleter function: render pass->%p\n",
                     static_cast< VkRenderPass >(renderPass) )
      device.destroyRenderPass( renderPass, allocator );
    }

  } );

  return false;
}

} // Namespace Vulkan

// Namespace Graphics

// Namespace Dali
