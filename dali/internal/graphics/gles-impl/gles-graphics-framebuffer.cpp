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

// CLASS HEADER
#include "gles-graphics-framebuffer.h"

// external headers
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/gl-defines.h>

// Internal headers
#include <dali/internal/graphics/gles-impl/gles-graphics-texture.h>
#include "egl-graphics-controller.h"
#include "gles-context.h"

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

Context* gSharedContext = nullptr; ///< The bind available context

} // anonymous namespace

void Framebuffer::SetSharedContext(Context* context)
{
  gSharedContext = context;
}

Framebuffer::Framebuffer(const Graphics::FramebufferCreateInfo& createInfo, Graphics::EglGraphicsController& controller)
: FramebufferResource(createInfo, controller),
  mInitialized(false),
  mDepthBufferUsed(false),
  mStencilBufferUsed(false),
  mAttachedDepthWrite(false),
  mAttachedStencilWrite(false),
  mAttachedAttachment(GL_NONE),
  mAttachedInternalFormat(GL_NONE)
{
  // Check whether we need to consider multisampling
  if(createInfo.multiSamplingLevel > 1u && controller.GetGraphicsInterface()->IsMultisampledRenderToTextureSupported())
  {
    mMultisamples = std::min(createInfo.multiSamplingLevel, controller.GetGraphicsInterface()->GetMaxTextureSamples());
  }

  mDepthBufferUsed   = mCreateInfo.depthStencilAttachment.depthUsage == Graphics::DepthStencilAttachment::Usage::WRITE;
  mStencilBufferUsed = mCreateInfo.depthStencilAttachment.stencilUsage == Graphics::DepthStencilAttachment::Usage::WRITE;

  // Add framebuffer to the Resource queue
  mController.AddFramebuffer(*this);
}

Framebuffer::~Framebuffer() = default;

bool Framebuffer::InitializeResource()
{
  auto* gl = mController.GetGL();
  if(DALI_LIKELY(gl) && gSharedContext && !mInitialized)
  {
    DALI_ASSERT_DEBUG(gSharedContext == mController.GetCurrentContext() && "Framebuffer is create at another context!");
    mInitialized = true;

    gl->GenFramebuffers(1, &mFramebufferId);
    // @todo: Error check if FramebufferId is 0

    gl->BindFramebuffer(GL_FRAMEBUFFER, mFramebufferId);

    for(Graphics::ColorAttachment& attachment : mCreateInfo.colorAttachments)
    {
      AttachTexture(attachment.texture, COLOR_ATTACHMENTS[attachment.attachmentId], attachment.layerId, attachment.levelId);
    }

    // @todo is this per framebuffer, or more immediate state that needs setting when framebuffer changed?
    gl->DrawBuffers(mCreateInfo.colorAttachments.size(), COLOR_ATTACHMENTS);

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

    gl->BindFramebuffer(GL_FRAMEBUFFER, 0);

    // Update framebuffer state cache here.
    auto& glStateCache          = gSharedContext->GetGLStateCache();
    auto& framebufferStateCache = glStateCache.mFrameBufferStateCache;

    framebufferStateCache.FrameBufferCreated(mFramebufferId);
    framebufferStateCache.SetCurrentFrameBuffer(mFramebufferId);
    framebufferStateCache.DrawOperation(glStateCache.mColorMask,
                                        glStateCache.DepthBufferWriteEnabled(),
                                        glStateCache.StencilBufferWriteEnabled());
    framebufferStateCache.SetCurrentFrameBuffer(0);
  }

  return mInitialized;
}

void Framebuffer::DestroyResource()
{
  if(DALI_LIKELY(!EglGraphicsController::IsShuttingDown()))
  {
    auto* gl = mController.GetGL();
    if(DALI_LIKELY(gl) && mInitialized)
    {
      if(mDepthBufferId)
      {
        gl->DeleteRenderbuffers(1, &mDepthBufferId);
        mDepthBufferId = 0u;
      }
      if(mStencilBufferId)
      {
        gl->DeleteRenderbuffers(1, &mStencilBufferId);
        mStencilBufferId = 0u;
      }

      if(mFramebufferId != 0u)
      {
        gl->DeleteFramebuffers(1, &mFramebufferId);
      }

      if(gSharedContext)
      {
        // Update framebuffer state cache here.
        auto& framebufferStateCache = gSharedContext->GetGLStateCache().mFrameBufferStateCache;
        framebufferStateCache.FrameBufferDeleted(mFramebufferId);
      }

      mFramebufferId = 0u;
      mInitialized   = false;

      // Reset attached renderbuffer state
      mAttachedDepthWrite     = false;
      mAttachedStencilWrite   = false;
      mAttachedAttachment     = GL_NONE;
      mAttachedInternalFormat = GL_NONE;
    }
  }
}

void Framebuffer::DiscardResource()
{
  mController.DiscardResource(this);
}

void Framebuffer::Bind()
{
  auto* gl = mController.GetGL();
  if(DALI_LIKELY(gl) && gSharedContext)
  {
    DALI_ASSERT_DEBUG(gSharedContext == mController.GetCurrentContext() && "Framebuffer is bound to another context!");
    gl->BindFramebuffer(GL_FRAMEBUFFER, mFramebufferId);

    PrepareRenderBuffer();

    // Update framebuffer state cache here.
    auto& framebufferStateCache = gSharedContext->GetGLStateCache().mFrameBufferStateCache;
    framebufferStateCache.SetCurrentFrameBuffer(mFramebufferId);
  }
}

void Framebuffer::AttachTexture(const Graphics::Texture* texture, uint32_t attachmentId, uint32_t layerId, uint32_t levelId)
{
  auto* gl = mController.GetGL();
  if(DALI_LIKELY(gl))
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

void Framebuffer::InvalidateDepthStencilRenderBuffers()
{
  auto* gl = mController.GetGL();
  if(DALI_LIKELY(gl))
  {
    switch(mAttachedAttachment)
    {
      case GL_DEPTH_STENCIL_ATTACHMENT:
      {
        if(mStencilBufferId != 0u)
        {
          GLenum attachments[] = {GL_DEPTH, GL_STENCIL};
          gl->InvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments);
        }
        break;
      }
      case GL_STENCIL_ATTACHMENT:
      {
        if(mStencilBufferId != 0u)
        {
          GLenum attachment = GL_STENCIL;
          gl->InvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);
        }
        break;
      }
      case GL_DEPTH_ATTACHMENT:
      {
        if(mDepthBufferId != 0u)
        {
          GLenum attachment = GL_DEPTH;
          gl->InvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);
        }
        break;
      }
      default:
      {
        // Do nothing.
        break;
      }
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

void Framebuffer::PrepareRenderBuffer()
{
  auto* gl = mController.GetGL();
  if(DALI_LIKELY(gl) && !mCreateInfo.depthStencilAttachment.stencilTexture && !mCreateInfo.depthStencilAttachment.depthTexture)
  {
    const bool depthWrite   = (mCreateInfo.depthStencilAttachment.depthUsage == Graphics::DepthStencilAttachment::Usage::WRITE) && mDepthBufferUsed;
    const bool stencilWrite = (mCreateInfo.depthStencilAttachment.stencilUsage == Graphics::DepthStencilAttachment::Usage::WRITE) && mStencilBufferUsed;

    // Check if the state has changed
    if(DALI_LIKELY(depthWrite == mAttachedDepthWrite && stencilWrite == mAttachedStencilWrite))
    {
      return; // No change, nothing to do
    }

    // State has changed, need to update renderbuffer

    // Delete old renderbuffers
    if(mDepthBufferId != 0u)
    {
      gl->DeleteRenderbuffers(1, &mDepthBufferId);
      mDepthBufferId = 0u;
    }
    if(mStencilBufferId != 0u)
    {
      gl->DeleteRenderbuffers(1, &mStencilBufferId);
      mStencilBufferId = 0u;
    }

    // Reset previous framebuffer attachment
    if(mAttachedAttachment != GL_NONE)
    {
      gl->FramebufferRenderbuffer(GL_FRAMEBUFFER, mAttachedAttachment, GL_RENDERBUFFER, 0);
      mAttachedAttachment = GL_NONE;
    }

    if(depthWrite || stencilWrite)
    {
      // Determine attachment and internal format
      const auto attachment     = depthWrite ? (stencilWrite ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT) : GL_STENCIL_ATTACHMENT;
      const auto internalFormat = depthWrite ? (stencilWrite ? GL_DEPTH24_STENCIL8 : GL_DEPTH_COMPONENT16) : GL_STENCIL_INDEX8;

      // Choose which buffer ID to use
      uint32_t& bufferId = stencilWrite ? mStencilBufferId : mDepthBufferId;

      // Create new renderbuffer
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

      // Attach to framebuffer
      gl->FramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, bufferId);

      // Update attached state
      mAttachedAttachment     = attachment;
      mAttachedInternalFormat = internalFormat;
    }
    else
    {
      // No renderbuffer needed
      mAttachedAttachment     = GL_NONE;
      mAttachedInternalFormat = GL_NONE;
    }

    // Update attached write states
    mAttachedDepthWrite   = depthWrite;
    mAttachedStencilWrite = stencilWrite;
  }
}

void Framebuffer::UpdateDepthStencilState(const Graphics::DepthStencilState& depthStencilState)
{
  if(mCreateInfo.depthStencilAttachment.depthUsage == Graphics::DepthStencilAttachment::Usage::WRITE)
  {
    mDepthBufferUsed = depthStencilState.depthTestEnable;
  }
  if(mCreateInfo.depthStencilAttachment.stencilUsage == Graphics::DepthStencilAttachment::Usage::WRITE)
  {
    mStencilBufferUsed = depthStencilState.stencilTestEnable;
  }
}

} // namespace Dali::Graphics::GLES
