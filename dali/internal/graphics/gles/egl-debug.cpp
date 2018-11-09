/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/gles/egl-debug.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace Egl
{

void PrintError( EGLint error)
{
  switch (error)
  {
    case EGL_BAD_DISPLAY:
    {
      DALI_LOG_ERROR("EGL_BAD_DISPLAY : Display is not an EGL display connection\n");
      break;
    }
    case EGL_NOT_INITIALIZED:
    {
      DALI_LOG_ERROR("EGL_NOT_INITIALIZED : Display has not been initialized\n");
      break;
    }
    case EGL_BAD_SURFACE:
    {
      DALI_LOG_ERROR("EGL_BAD_SURFACE : Draw or read is not an EGL surface\n");
      break;
    }
    case EGL_BAD_CONTEXT:
    {
      DALI_LOG_ERROR("EGL_BAD_CONTEXT : Context is not an EGL rendering context\n");
      break;
    }
    case EGL_BAD_MATCH:
    {
      DALI_LOG_ERROR("EGL_BAD_MATCH : Draw or read are not compatible with context, or if context is set to EGL_NO_CONTEXT and draw or read are not set to EGL_NO_SURFACE, or if draw or read are set to EGL_NO_SURFACE and context is not set to EGL_NO_CONTEXT\n");
      break;
    }
    case EGL_BAD_ACCESS:
    {
      DALI_LOG_ERROR("EGL_BAD_ACCESS : Context is current to some other thread\n");
      break;
    }
    case EGL_BAD_NATIVE_PIXMAP:
    {
      DALI_LOG_ERROR("EGL_BAD_NATIVE_PIXMAP : A native pixmap underlying either draw or read is no longer valid\n");
      break;
    }
    case EGL_BAD_NATIVE_WINDOW:
    {
      DALI_LOG_ERROR("EGL_BAD_NATIVE_WINDOW : A native window underlying either draw or read is no longer valid\n");
      break;
    }
    case EGL_BAD_CURRENT_SURFACE:
    {
      DALI_LOG_ERROR("EGL_BAD_CURRENT_SURFACE : The previous context has unflushed commands and the previous surface is no longer valid\n");
      break;
    }
    case EGL_BAD_ALLOC:
    {
      DALI_LOG_ERROR("EGL_BAD_ALLOC : Allocation of ancillary buffers for draw or read were delayed until eglMakeCurrent is called, and there are not enough resources to allocate them\n");
      break;
    }
    case EGL_CONTEXT_LOST:
    {
      DALI_LOG_ERROR("EGL_CONTEXT_LOST : If a power management event has occurred. The application must destroy all contexts and reinitialise OpenGL ES state and objects to continue rendering\n");
      break;
    }
    default:
    {
      DALI_LOG_ERROR("Unknown error with code: %d\n", error);
      break;
    }
  }
}

}  // namespace Egl

}  // namespace Adaptor

}  // namespace Internal

}  // namespace Dali
