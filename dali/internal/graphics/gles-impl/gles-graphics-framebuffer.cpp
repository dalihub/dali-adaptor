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

// CLASS HEADER
#include "gles-graphics-framebuffer.h"

// external headers
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>

// Internal headers
#include <dali/internal/graphics/gles-impl/gles-graphics-texture.h>
#include "egl-graphics-controller.h"

namespace Dali::Graphics::GLES
{
namespace
{
const GLenum COLOR_ATTACHMENTS[] =
  {
    GL_COLOR_ATTACHMENT0,
    GL_COLOR_ATTACHMENT1,
    GL_COLOR_ATTACHMENT2,
    GL_COLOR_ATTACHMENT3,
    GL_COLOR_ATTACHMENT4,
    GL_COLOR_ATTACHMENT5,
    GL_COLOR_ATTACHMENT6,
    GL_COLOR_ATTACHMENT7,
};

struct DEPTH_STENCIL_ATTACHMENT_TYPE
{
  constexpr explicit DEPTH_STENCIL_ATTACHMENT_TYPE(Graphics::Format textureFormat)
  {
    switch(textureFormat)
    {
      case Graphics::Format::D16_UNORM:
      case Graphics::Format::D32_SFLOAT:
      case Graphics::Format::X8_D24_UNORM_PACK32:
      {
        attachment = GL_DEPTH_ATTACHMENT;
        break;
      }

      case Graphics::Format::S8_UINT: // Probably won't work as a standalone texture.
      {
        attachment = GL_STENCIL_ATTACHMENT;
        break;
      }

      case Graphics::Format::D16_UNORM_S8_UINT:
      case Graphics::Format::D24_UNORM_S8_UINT:
      case Graphics::Format::D32_SFLOAT_S8_UINT:
      {
        attachment = GL_DEPTH_STENCIL_ATTACHMENT;
        break;
      }
      default:
      {
        attachment = GL_NONE;
        break;
      }
    }
  }
  Dali::GLenum attachment{GL_NONE};
};

} // anonymous namespace

Framebuffer::Framebuffer(const Graphics::FramebufferCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
: FramebufferResource(createInfo, controller)
{
  // Check whether we need to consider multisampling
  if(createInfo.multiSamplingLevel > 1u && controller.GetGraphicsInterface()->IsMultisampledRenderToTextureSupported())
  {
    mMultisamples = std::min(createInfo.multiSamplingLevel, controller.GetGraphicsInterface()->GetMaxTextureSamples());
  }

  // Add framebuffer to the Resource queue
  mController.AddFramebuffer(*this);
}

Framebuffer::~Framebuffer() = default;

bool Framebuffer::InitializeResource()
{
  auto context = mController.GetCurrentContext();
  auto gl      = mController.GetGL();
  if(gl && context && !mInitialized)
  {
    mInitialized = true;

    context->GenFramebuffers(1, &mFramebufferId);
    context->BindFrameBuffer(GL_FRAMEBUFFER, mFramebufferId);

    for(Graphics::ColorAttachment& attachment : mCreateInfo.colorAttachments)
    {
      AttachTexture(attachment.texture, COLOR_ATTACHMENTS[attachment.attachmentId], attachment.layerId, attachment.levelId);
    }

    // @todo is this per framebuffer, or more immediate state that needs setting when framebuffer changed?
    context->DrawBuffers(mCreateInfo.colorAttachments.size(), COLOR_ATTACHMENTS);

    // @todo Currently, we don't assume that GL_EXT_PACKED_DEPTH_STENCIL valid.
    // We will assume that stencilTexture / stencilBufferId always mean depth-stencil.
    if(mCreateInfo.depthStencilAttachment.stencilTexture)
    {
      // bind depth 24 bits + stencil 8 bits texture, or 8 tencil texture.
      auto stencilTexture = static_cast<const GLES::Texture*>(mCreateInfo.depthStencilAttachment.stencilTexture);
      auto attachmentId   = DEPTH_STENCIL_ATTACHMENT_TYPE(stencilTexture->GetCreateInfo().format).attachment;

      if(attachmentId != GL_DEPTH_STENCIL_ATTACHMENT)
      {
        DALI_LOG_ERROR("Current Depth/Stencil Texture Type doesn't support. Please check depth/stencil texture's pixel format");
      }

      AttachTexture(stencilTexture, attachmentId, 0, mCreateInfo.depthStencilAttachment.stencilLevel);
    }
    else if(mCreateInfo.depthStencilAttachment.depthTexture)
    {
      // bind depth texture.
      auto depthTexture = static_cast<const GLES::Texture*>(mCreateInfo.depthStencilAttachment.depthTexture);
      auto attachmentId = DEPTH_STENCIL_ATTACHMENT_TYPE(depthTexture->GetCreateInfo().format).attachment;

      if(attachmentId != GL_DEPTH_STENCIL_ATTACHMENT && mCreateInfo.depthStencilAttachment.stencilUsage == Graphics::DepthStencilAttachment::Usage::WRITE)
      {
        DALI_LOG_ERROR("Current Depth Texture Type doesn't support to store Stencil. Please check depth texture's pixel format");
      }

      AttachTexture(depthTexture, attachmentId, 0, mCreateInfo.depthStencilAttachment.depthLevel);
    }
    else
    {
      const bool depthWrite   = mCreateInfo.depthStencilAttachment.depthUsage == Graphics::DepthStencilAttachment::Usage::WRITE;
      const bool stencilWrite = mCreateInfo.depthStencilAttachment.stencilUsage == Graphics::DepthStencilAttachment::Usage::WRITE;

      // Check whether we need to use RenderBuffer
      if(depthWrite || stencilWrite)
      {
        // if stencil is write, use renderbuffer as mStencilBufferId.
        uint32_t&  bufferId       = stencilWrite ? mStencilBufferId : mDepthBufferId;
        const auto internalFormat = depthWrite ? (stencilWrite ? GL_DEPTH24_STENCIL8 : GL_DEPTH_COMPONENT16) : GL_STENCIL_INDEX8;
        const auto attachment     = depthWrite ? (stencilWrite ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT) : GL_STENCIL_ATTACHMENT;

        gl->GenRenderbuffers(1, &bufferId);
        gl->BindRenderbuffer(GL_RENDERBUFFER, bufferId);

        if(mMultisamples <= 1u)
        {
          gl->RenderbufferStorage(GL_RENDERBUFFER, internalFormat, mCreateInfo.size.width, mCreateInfo.size.height);
        }
        else
        {
          gl->RenderbufferStorageMultisample(GL_RENDERBUFFER, mMultisamples, internalFormat, mCreateInfo.size.width, mCreateInfo.size.height);
        }
        gl->FramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, bufferId);
      }
    }

    context->BindFrameBuffer(GL_FRAMEBUFFER, 0);
  }

  return mInitialized;
}

void Framebuffer::DestroyResource()
{
  if(DALI_LIKELY(!EglGraphicsController::IsShuttingDown()))
  {
    auto context = mController.GetCurrentContext();
    auto gl      = mController.GetGL();
    if(gl && context && mInitialized)
    {
      if(mDepthBufferId)
      {
        gl->DeleteRenderbuffers(1, &mDepthBufferId);
      }
      if(mStencilBufferId)
      {
        gl->DeleteRenderbuffers(1, &mStencilBufferId);
      }

      context->DeleteFramebuffers(1, &mFramebufferId);

      mFramebufferId = 0u;
      mInitialized   = false;
    }
  }
}

void Framebuffer::DiscardResource()
{
  mController.DiscardResource(this);
}

void Framebuffer::Bind() const
{
  auto context = mController.GetCurrentContext();
  auto gl      = mController.GetGL();

  if(gl && context)
  {
    context->BindFrameBuffer(GL_FRAMEBUFFER, mFramebufferId);
  }
}

void Framebuffer::AttachTexture(const Graphics::Texture* texture, uint32_t attachmentId, uint32_t layerId, uint32_t levelId)
{
  auto gl = mController.GetGL();
  if(gl)
  {
    auto graphicsTexture = static_cast<const GLES::Texture*>(texture);
    auto textarget       = (graphicsTexture->GetCreateInfo().textureType == Graphics::TextureType::TEXTURE_2D) ? graphicsTexture->GetGlTarget() : GL_TEXTURE_CUBE_MAP_POSITIVE_X + layerId;
    if(mMultisamples <= 1u)
    {
      gl->FramebufferTexture2D(GL_FRAMEBUFFER, attachmentId, textarget, graphicsTexture->GetGLTexture(), levelId);
    }
    else
    {
      gl->FramebufferTexture2DMultisample(GL_FRAMEBUFFER, attachmentId, textarget, graphicsTexture->GetGLTexture(), levelId, mMultisamples);
    }
  }
}

uint32_t Framebuffer::GetGlFramebufferId() const
{
  return mFramebufferId;
}

uint32_t Framebuffer::GetGlDepthBufferId() const
{
  return mDepthBufferId;
}

uint32_t Framebuffer::GetGlStencilBufferId() const
{
  return mStencilBufferId;
}

} //namespace Dali::Graphics::GLES
