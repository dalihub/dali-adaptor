/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/gles/gl-extensions.h>

// EXTERNAL INCLUDES
#include <dali/internal/graphics/common/egl-include.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
GlExtensions::GlExtensions()
:
#ifdef GL_EXT_discard_framebuffer
  mGlDiscardFramebuffer(nullptr),
#endif
#ifdef GL_OES_get_program_binary
  mGlGetProgramBinaryOES(nullptr),
  mGlProgramBinaryOES(nullptr),
#endif
#ifdef GL_KHR_blend_equation_advanced
  mBlendBarrierKHR(nullptr),
#endif
#ifdef GL_EXT_multisampled_render_to_texture
  mGlRenderbufferStorageMultisampleEXT(nullptr),
  mGlFramebufferTexture2DMultisampleEXT(nullptr),
#endif
  mInitialized(false)
{
}

GlExtensions::~GlExtensions()
{
}

void GlExtensions::DiscardFrameBuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
  // initialize extension on first use as on some hw platforms a context
  // has to be bound for the extensions to return correct pointer
  if(DALI_UNLIKELY(!mInitialized))
  {
    Initialize();
  }

#ifdef GL_EXT_discard_framebuffer
  if(mGlDiscardFramebuffer)
  {
    mGlDiscardFramebuffer(target, numAttachments, attachments);
  }
  else
  {
    DALI_LOG_ERROR("Error: glDiscardFramebufferEXT extension is not available\n");
  }
#endif
}

void GlExtensions::GetProgramBinaryOES(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
  // initialize extension on first use as on some hw platforms a context
  // has to be bound for the extensions to return correct pointer
  if(DALI_UNLIKELY(!mInitialized))
  {
    Initialize();
  }

#ifdef GL_OES_get_program_binary
  if(mGlGetProgramBinaryOES)
  {
    mGlGetProgramBinaryOES(program, bufSize, length, binaryFormat, binary);
  }
  else
  {
    DALI_LOG_ERROR("Error: glGetProgramBinaryOES extension is not available\n");
    DALI_ASSERT_DEBUG(0);
  }
#endif
}

void GlExtensions::ProgramBinaryOES(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLint length)
{
  // initialize extension on first use as on some hw platforms a context
  // has to be bound for the extensions to return correct pointer
  if(DALI_UNLIKELY(!mInitialized))
  {
    Initialize();
  }

#ifdef GL_OES_get_program_binary
  if(mGlProgramBinaryOES)
  {
    mGlProgramBinaryOES(program, binaryFormat, binary, length);
  }
  else
  {
    DALI_LOG_ERROR("Error: glProgramBinaryOES extension is not available\n");
    DALI_ASSERT_DEBUG(0);
  }
#endif
}

bool GlExtensions::BlendBarrierKHR()
{
  // initialize extension on first use as on some hw platforms a context
  // has to be bound for the extensions to return correct pointer
  if(DALI_UNLIKELY(!mInitialized))
  {
    Initialize();
  }

#ifdef GL_KHR_blend_equation_advanced
  if(mBlendBarrierKHR)
  {
    mBlendBarrierKHR();
    return true;
  }
  return false;
#endif

  return false;
}

void GlExtensions::RenderbufferStorageMultisampleEXT(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
  // initialize extension on first use as on some hw platforms a context
  // has to be bound for the extensions to return correct pointer
  if(DALI_UNLIKELY(!mInitialized))
  {
    Initialize();
  }

#ifdef GL_EXT_multisampled_render_to_texture
  if(mGlRenderbufferStorageMultisampleEXT)
  {
    mGlRenderbufferStorageMultisampleEXT(target, samples, internalformat, width, height);
  }
  else
  {
    DALI_LOG_ERROR("Error: glRenderbufferStorageMultisampleEXT extension is not available\n");
    DALI_ASSERT_DEBUG(0);
  }
#endif
}

void GlExtensions::FramebufferTexture2DMultisampleEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
  // initialize extension on first use as on some hw platforms a context
  // has to be bound for the extensions to return correct pointer
  if(DALI_UNLIKELY(!mInitialized))
  {
    Initialize();
  }

#ifdef GL_EXT_multisampled_render_to_texture
  if(mGlFramebufferTexture2DMultisampleEXT)
  {
    mGlFramebufferTexture2DMultisampleEXT(target, attachment, textarget, texture, level, samples);
  }
  else
  {
    DALI_LOG_ERROR("Error: glFramebufferTexture2DMultisampleEXT extension is not available\n");
    DALI_ASSERT_DEBUG(0);
  }
#endif
}

void GlExtensions::Initialize()
{
  mInitialized = true;

#ifdef GL_EXT_discard_framebuffer
  mGlDiscardFramebuffer = reinterpret_cast<PFNGLDISCARDFRAMEBUFFEREXTPROC>(eglGetProcAddress("glDiscardFramebufferEXT"));
#endif

#ifdef GL_OES_get_program_binary
  mGlGetProgramBinaryOES = reinterpret_cast<PFNGLGETPROGRAMBINARYOESPROC>(eglGetProcAddress("glGetProgramBinaryOES"));
  mGlProgramBinaryOES    = reinterpret_cast<PFNGLPROGRAMBINARYOESPROC>(eglGetProcAddress("glProgramBinaryOES"));
#endif

#ifdef GL_KHR_blend_equation_advanced
  mBlendBarrierKHR = reinterpret_cast<PFNGLBLENDBARRIERKHRPROC>(eglGetProcAddress("glBlendBarrierKHR"));
#endif

#ifdef GL_EXT_multisampled_render_to_texture
  mGlRenderbufferStorageMultisampleEXT  = reinterpret_cast<PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC>(eglGetProcAddress("glRenderbufferStorageMultisampleEXT"));
  mGlFramebufferTexture2DMultisampleEXT = reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC>(eglGetProcAddress("glFramebufferTexture2DMultisampleEXT"));
#endif
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
