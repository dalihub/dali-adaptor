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

#include <dali/graphics/vulkan/api/vulkan-api-framebuffer-factory.h>
#include <dali/graphics/vulkan/api/vulkan-api-framebuffer.h>
#include <dali/graphics/vulkan/vulkan-graphics.h>

namespace Dali
{
namespace Graphics
{
namespace VulkanAPI
{


FramebufferFactory::FramebufferFactory( Controller& controller )
: mController( controller ),
  mWidth(0u),
  mHeight(0u),
  mColorAttachments(),
  mDepthStencilAttachment()
{
}

FramebufferFactory::~FramebufferFactory() = default;

void FramebufferFactory::Reset()
{
  mWidth=0;
  mHeight=0;
  mColorAttachments.clear();
  mDepthStencilAttachment.texture = nullptr;
}

Dali::Graphics::FramebufferFactory& FramebufferFactory::SetSize( const Dali::Graphics::Extent2D& size )
{
  mWidth = uint32_t(size.width);
  mHeight = uint32_t(size.height);
  return *this;
}

/**
 * Set a color attachment for the framebuffer
 */
Dali::Graphics::FramebufferFactory& FramebufferFactory::SetColorAttachment(
  TextureDetails::AttachmentId attachmentIndex,
  const Texture&               texture,
  TextureDetails::LayerId      layer,
  TextureDetails::LevelId      level )
{
  if( mColorAttachments.size() <= attachmentIndex )
  {
    mColorAttachments.resize( attachmentIndex+1 );
  }
  mColorAttachments[attachmentIndex] = ColorAttachment{ &texture, layer, level };

  return *this;
}

/**
 * Set a depth attachment for the framebuffer
 */
Dali::Graphics::FramebufferFactory& FramebufferFactory::SetDepthStencilAttachment(
  const Graphics::Texture&                   texture,
  Graphics::TextureDetails::LayerId          layer,
  Graphics::TextureDetails::LevelId          level,
  Graphics::TextureDetails::DepthStencilFlag depthStencilFlag )
{
  mDepthStencilAttachment = DepthAttachment{ &texture, layer, level,depthStencilFlag };

  return *this;
}

std::unique_ptr<Graphics::Framebuffer> FramebufferFactory::Create() const
{
  auto retval = std::make_unique<VulkanAPI::Framebuffer>( mController, mWidth, mHeight );

  if( retval->Initialise( mColorAttachments, mDepthStencilAttachment ) )
  {
    return retval;
  }

  return nullptr;
}


} // namespace VulkanAPI
} // namespace Graphics
} // namespace Dali
