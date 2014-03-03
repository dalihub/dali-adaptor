//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// CLASS HEADER
#include "gl-extensions.h"

// EXTERNAL INCLUDES
#include <EGL/egl.h>
#include <EGL/eglext.h>

// INTERNAL INCLUDES
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
  : mInitialized( false )
{
}

GlExtensions::~GlExtensions()
{
}

#if DALI_GLES_VERSION < 30

void GlExtensions::DiscardFrameBuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
  // initialize extension on first use as on some hw platforms a context
  // has to be bound for the extensions to return correct pointer
  if( !mInitialized )
  {
    Initialize();
  }

#ifdef PFNGLDISCARDFRAMEBUFFEREXTPROC
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

#ifdef PFNGLGETPROGRAMBINARYOESPROC
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

#ifdef PFNGLGETPROGRAMBINARYOESPROC
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

#ifdef PFNGLDISCARDFRAMEBUFFEREXTPROC
  mGlDiscardFramebuffer = (PFNGLDISCARDFRAMEBUFFEREXTPROC) eglGetProcAddress("glDiscardFramebufferEXT");
#endif

#ifdef PFNGLGETPROGRAMBINARYOESPROC
  mGlGetProgramBinaryOES = (PFNGLGETPROGRAMBINARYOESPROC) eglGetProcAddress("glGetProgramBinaryOES");
  mGlProgramBinaryOES = (PFNGLPROGRAMBINARYOESPROC) eglGetProcAddress("glProgramBinaryOES");
#endif

}

#endif // DALI_GLES_VERSION < 30

} // namespace ECoreX

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
