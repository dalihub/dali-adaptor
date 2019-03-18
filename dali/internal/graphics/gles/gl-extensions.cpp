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

// CLASS HEADER
#include <dali/internal/graphics/gles/gl-extensions.h>

// EXTERNAL INCLUDES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace ECoreX
{

GlExtensions::GlExtensions()
:
#ifdef GL_EXT_discard_framebuffer
  mGlDiscardFramebuffer( NULL ),
#endif
#ifdef GL_OES_get_program_binary
  mGlGetProgramBinaryOES( NULL ),
  mGlProgramBinaryOES( NULL ),
#endif
  mInitialized( false )
{
}

GlExtensions::~GlExtensions()
{
}

void GlExtensions::DiscardFrameBuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
  // initialize extension on first use as on some hw platforms a context
  // has to be bound for the extensions to return correct pointer
  if( !mInitialized )
  {
    Initialize();
  }

#ifdef GL_EXT_discard_framebuffer
  if( mGlDiscardFramebuffer )
  {
    mGlDiscardFramebuffer(target, numAttachments, attachments);
  }
  else
  {
    DALI_LOG_ERROR("Error: glDiscardFramebufferEXT extension is not available\n");
  }
#endif
}

void GlExtensions::GetProgramBinaryOES(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary)
{
  // initialize extension on first use as on some hw platforms a context
  // has to be bound for the extensions to return correct pointer
  if( !mInitialized )
  {
    Initialize();
  }

#ifdef GL_OES_get_program_binary
  if (mGlGetProgramBinaryOES)
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

void GlExtensions::ProgramBinaryOES(GLuint program, GLenum binaryFormat, const GLvoid *binary, GLint length)
{
  // initialize extension on first use as on some hw platforms a context
  // has to be bound for the extensions to return correct pointer
  if( !mInitialized )
  {
    Initialize();
  }

#ifdef GL_OES_get_program_binary
  if (mGlProgramBinaryOES)
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

void GlExtensions::Initialize()
{
  mInitialized = true;

#ifdef GL_EXT_discard_framebuffer
  mGlDiscardFramebuffer = reinterpret_cast< PFNGLDISCARDFRAMEBUFFEREXTPROC >( eglGetProcAddress("glDiscardFramebufferEXT") );
#endif

#ifdef GL_OES_get_program_binary
  mGlGetProgramBinaryOES = reinterpret_cast< PFNGLGETPROGRAMBINARYOESPROC >( eglGetProcAddress("glGetProgramBinaryOES") );
  mGlProgramBinaryOES = reinterpret_cast< PFNGLPROGRAMBINARYOESPROC >( eglGetProcAddress("glProgramBinaryOES") );
#endif
}

} // namespace ECoreX

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
