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
#include <dali/internal/graphics/linux/egl-image-extensions-linux-dma-buf.h>

// EXTERNAL INCLUDES
#include <dali/internal/graphics/common/egl-include.h> ///< for EGL/egl.h, which eglext.h needs first

#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <dali/devel-api/threading/mutex.h>
#include <dali/integration-api/debug.h>
#include <cstring> ///< for strstr
#include <string>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-implementation.h>

// EGL_EXT_device_drm_render_node was added to the EGL headers after the
// extension itself became available in some drivers.
#ifndef EGL_DRM_RENDER_NODE_FILE_EXT
#define EGL_DRM_RENDER_NODE_FILE_EXT 0x3377
#endif

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
namespace
{
// Resolved once, on the render thread.
PFNEGLCREATEIMAGEKHRPROC            gEglCreateImageKHRProc            = 0;
PFNEGLDESTROYIMAGEKHRPROC           gEglDestroyImageKHRProc           = 0;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC gGlEGLImageTargetTexture2DOESProc = 0;

bool gInitialized = false;
bool gSupported   = false;

/// Written by OnDisplayInitialized() on the render thread and read from producer
/// threads, so it needs guarding however short-lived the race would be.
Dali::Mutex gRenderNodeMutex;
std::string gRenderNodePath;

void Initialize(EglImplementation& impl)
{
  if(gInitialized)
  {
    return;
  }
  gInitialized = true;

  const char* extensions = eglQueryString(impl.GetDisplay(), EGL_EXTENSIONS);
  if(!extensions || !strstr(extensions, "EGL_EXT_image_dma_buf_import"))
  {
    DALI_LOG_ERROR("EGL_EXT_image_dma_buf_import is not available\n");
    return;
  }

  gEglCreateImageKHRProc            = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
  gEglDestroyImageKHRProc           = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
  gGlEGLImageTargetTexture2DOESProc = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));

  gSupported = (gEglCreateImageKHRProc && gEglDestroyImageKHRProc && gGlEGLImageTargetTexture2DOESProc);
}
} // unnamed namespace

bool EglImageExtensionsLinuxDmaBuf::IsSupported(EglImplementation& impl)
{
  Initialize(impl);
  return gSupported;
}

void EglImageExtensionsLinuxDmaBuf::OnDisplayInitialized(EglImplementation& impl)
{
  // Caches the render node path, so the threads that need it never touch EGL.
  auto queryDisplayAttrib = reinterpret_cast<PFNEGLQUERYDISPLAYATTRIBEXTPROC>(eglGetProcAddress("eglQueryDisplayAttribEXT"));
  auto queryDeviceString  = reinterpret_cast<PFNEGLQUERYDEVICESTRINGEXTPROC>(eglGetProcAddress("eglQueryDeviceStringEXT"));
  if(!queryDisplayAttrib || !queryDeviceString)
  {
    return;
  }

  EGLAttrib device = 0;
  if(!queryDisplayAttrib(impl.GetDisplay(), EGL_DEVICE_EXT, &device))
  {
    return;
  }

  const auto  eglDevice        = reinterpret_cast<EGLDeviceEXT>(device);
  const char* deviceExtensions = queryDeviceString(eglDevice, EGL_EXTENSIONS);
  if(!deviceExtensions || !strstr(deviceExtensions, "EGL_EXT_device_drm_render_node"))
  {
    return;
  }

  const char* path = queryDeviceString(eglDevice, EGL_DRM_RENDER_NODE_FILE_EXT);
  if(!path)
  {
    // No DRM device, which is the honest answer for a software renderer.
    return;
  }

  Dali::Mutex::ScopedLock lock(gRenderNodeMutex);
  gRenderNodePath = path;
}

std::string EglImageExtensionsLinuxDmaBuf::GetRenderNodePath()
{
  Dali::Mutex::ScopedLock lock(gRenderNodeMutex);
  return gRenderNodePath;
}

void* EglImageExtensionsLinuxDmaBuf::CreateImage(EglImplementation& impl, const DmaBufDescriptor& descriptor)
{
  if(!IsSupported(impl))
  {
    return nullptr;
  }

  const EGLint attribs[] =
    {
      EGL_WIDTH, static_cast<EGLint>(descriptor.width),
      EGL_HEIGHT, static_cast<EGLint>(descriptor.height),
      EGL_LINUX_DRM_FOURCC_EXT, static_cast<EGLint>(descriptor.fourcc),
      EGL_DMA_BUF_PLANE0_FD_EXT, descriptor.fd,
      EGL_DMA_BUF_PLANE0_OFFSET_EXT, static_cast<EGLint>(descriptor.offset),
      EGL_DMA_BUF_PLANE0_PITCH_EXT, static_cast<EGLint>(descriptor.stride),
      EGL_NONE};

  // EGL_LINUX_DMA_BUF_EXT describes the buffer entirely through the attribute
  // list, so the client buffer argument must be NULL.
  EGLImageKHR eglImage = gEglCreateImageKHRProc(impl.GetDisplay(),
                                                EGL_NO_CONTEXT,
                                                EGL_LINUX_DMA_BUF_EXT,
                                                NULL,
                                                attribs);

  if(EGL_NO_IMAGE_KHR == eglImage)
  {
    DALI_LOG_ERROR("eglCreateImageKHR(EGL_LINUX_DMA_BUF_EXT) failed. fourcc[0x%x] %ux%u stride[%u] eglGetError[0x%x]\n",
                   descriptor.fourcc,
                   descriptor.width,
                   descriptor.height,
                   descriptor.stride,
                   eglGetError());
    return nullptr;
  }

  return eglImage;
}

void EglImageExtensionsLinuxDmaBuf::DestroyImage(EglImplementation& impl, void* eglImage)
{
  if(!gSupported || eglImage == nullptr)
  {
    return;
  }

  if(EGL_FALSE == gEglDestroyImageKHRProc(impl.GetDisplay(), static_cast<EGLImageKHR>(eglImage)))
  {
    DALI_LOG_ERROR("eglDestroyImageKHR failed. eglGetError[0x%x]\n", eglGetError());
  }
}

void EglImageExtensionsLinuxDmaBuf::TargetExternalTexture(void* eglImage)
{
  if(!gSupported || eglImage == nullptr)
  {
    return;
  }

  gGlEGLImageTargetTexture2DOESProc(GL_TEXTURE_EXTERNAL_OES, reinterpret_cast<GLeglImageOES>(eglImage));

#ifdef EGL_ERROR_CHECKING
  GLint glError = glGetError();
  if(GL_NO_ERROR != glError)
  {
    DALI_LOG_ERROR(" glEGLImageTargetTexture2DOES(EXTERNAL_OES) returned error 0x%04x\n", glError);
  }
#endif
}

} // namespace Adaptor

} // namespace Internal

} //namespace DALI_NAMESPACE
