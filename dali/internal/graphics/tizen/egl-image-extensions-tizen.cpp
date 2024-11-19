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
#include <dali/internal/graphics/common/egl-image-extensions.h>

// EXTERNAL INCLUDES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>

#include <EGL/eglext.h>

#include <tbm_bufmgr.h>
#include <tbm_surface.h>
#include <tbm_surface_internal.h>

#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-implementation.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/common/time-service.h>

// TBM surface support
#ifndef EGL_NATIVE_SURFACE_TIZEN
#define EGL_NATIVE_SURFACE_TIZEN 0x32A1
#endif

namespace
{
// function pointers assigned in InitializeEglImageKHR
PFNEGLCREATEIMAGEKHRPROC            eglCreateImageKHRProc            = 0;
PFNEGLDESTROYIMAGEKHRPROC           eglDestroyImageKHRProc           = 0;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOESProc = 0;

const char* EGL_TIZEN_IMAGE_NATIVE_SURFACE = "EGL_TIZEN_image_native_surface";
const char* EGL_EXT_IMAGE_DMA_BUF_IMPORT   = "EGL_EXT_image_dma_buf_import";

static uint32_t gLogThreshold{0};
static bool     gLogEnabled{false};

static uint32_t GetPerformanceLogThresholdTime()
{
  auto     timeString = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_EGL_PERFORMANCE_LOG_THRESHOLD_TIME);
  uint32_t time       = timeString ? static_cast<uint32_t>(std::atoi(timeString)) : std::numeric_limits<uint32_t>::max();
  return time;
}

#define START_DURATION_CHECK()                         \
  uint64_t startTimeNanoSeconds = 0ull;                \
  uint64_t endTimeNanoSeconds   = 0ull;                \
  if(gLogEnabled)                                      \
  {                                                    \
    TimeService::GetNanoseconds(startTimeNanoSeconds); \
  }

#define FINISH_DURATION_CHECK(functionName)                                                                                                                \
  if(gLogEnabled)                                                                                                                                          \
  {                                                                                                                                                        \
    TimeService::GetNanoseconds(endTimeNanoSeconds);                                                                                                       \
    if(static_cast<uint32_t>((endTimeNanoSeconds - startTimeNanoSeconds) / 1000000ull) >= gLogThreshold)                                                   \
    {                                                                                                                                                      \
      DALI_LOG_RELEASE_INFO("%s takes long time! [%.6lf ms]\n", functionName, static_cast<double>(endTimeNanoSeconds - startTimeNanoSeconds) / 1000000.0); \
    }                                                                                                                                                      \
  }

} // unnamed namespace

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
struct EglImageExtensions::Impl
{
  bool mIsTizenImageNativeSurfaceSupported{false};
  bool mIsExtImageDmaBufImportSupported{false};
};

EglImageExtensions::EglImageExtensions(EglImplementation* eglImpl)
: mImpl(new Impl()),
  mEglImplementation(eglImpl),
  mImageKHRInitialized(false),
  mImageKHRInitializeFailed(false)
{
  DALI_ASSERT_ALWAYS(eglImpl && "EGL Implementation not instantiated");

  gLogThreshold = GetPerformanceLogThresholdTime();
  gLogEnabled   = gLogThreshold < std::numeric_limits<uint32_t>::max() ? true : false;
}

EglImageExtensions::~EglImageExtensions()
{
  delete mImpl;
}

void* EglImageExtensions::CreateImageKHR(EGLClientBuffer clientBuffer)
{
  if(mImageKHRInitialized == false)
  {
    InitializeEglImageKHR();
  }

  if(mImageKHRInitialized == false)
  {
    return NULL;
  }

  EGLImageKHR eglImage = EGL_NO_IMAGE_KHR;

  // Use the EGL image extension
  if(mImpl->mIsTizenImageNativeSurfaceSupported)
  {
    // If EGL_TIZEN_image_native_surface is supported
    const EGLint attribs[] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE};

    START_DURATION_CHECK();
    eglImage = eglCreateImageKHRProc(mEglImplementation->GetDisplay(),
                                     EGL_NO_CONTEXT,
                                     EGL_NATIVE_SURFACE_TIZEN,
                                     clientBuffer,
                                     attribs);
    FINISH_DURATION_CHECK("eglCreateImageKHRProc(EGL_NATIVE_SURFACE_TIZEN)");
  }
  else if(mImpl->mIsExtImageDmaBufImportSupported)
  {
    // Else then use EGL_EXT_image_dma_buf_import
    tbm_surface_info_s info;
    tbm_surface_h      tbmSurface = reinterpret_cast<tbm_surface_h>(clientBuffer);

    if(tbm_surface_get_info(tbmSurface, &info) != TBM_SURFACE_ERROR_NONE)
    {
      return NULL;
    }

    // We support only 1 plane
    tbm_bo tbmBo = tbm_surface_internal_get_bo(tbmSurface, tbm_surface_internal_get_plane_bo_idx(tbmSurface, 0));

    // clang-format off
    const EGLint attribs[] = {EGL_WIDTH, static_cast<EGLint>(info.width),
                              EGL_HEIGHT, static_cast<EGLint>(info.height),
                              EGL_LINUX_DRM_FOURCC_EXT, static_cast<EGLint>(info.format),
                              EGL_DMA_BUF_PLANE0_FD_EXT, static_cast<EGLint>(reinterpret_cast<size_t>(tbm_bo_get_handle(tbmBo, TBM_DEVICE_3D).ptr)),
                              EGL_DMA_BUF_PLANE0_OFFSET_EXT, static_cast<EGLint>(info.planes[0].offset),
                              EGL_DMA_BUF_PLANE0_PITCH_EXT, static_cast<EGLint>(info.planes[0].stride),
                              EGL_NONE};
    // clang-format on

    START_DURATION_CHECK();
    eglImage = eglCreateImageKHRProc(mEglImplementation->GetDisplay(),
                                     EGL_NO_CONTEXT,
                                     EGL_LINUX_DMA_BUF_EXT,
                                     nullptr,
                                     attribs);
    FINISH_DURATION_CHECK("eglCreateImageKHRProc(tbmBo)");
  }
  else
  {
    DALI_LOG_ERROR("Not supported\n");
    return EGL_NO_IMAGE_KHR;
  }

  if(EGL_NO_IMAGE_KHR == eglImage)
  {
    EGLint error = eglGetError();
    switch(error)
    {
      case EGL_SUCCESS:
      {
        break;
      }
      case EGL_BAD_DISPLAY:
      {
        DALI_LOG_ERROR("EGL_BAD_DISPLAY: Invalid EGLDisplay object\n");
        break;
      }
      case EGL_BAD_CONTEXT:
      {
        DALI_LOG_ERROR("EGL_BAD_CONTEXT: Invalid EGLContext object\n");
        break;
      }
      case EGL_BAD_PARAMETER:
      {
        DALI_LOG_ERROR("EGL_BAD_PARAMETER: Invalid target parameter or attribute in attrib_list\n");
        break;
      }
      case EGL_BAD_MATCH:
      {
        DALI_LOG_ERROR("EGL_BAD_MATCH: attrib_list does not match target\n");
        break;
      }
      case EGL_BAD_ACCESS:
      {
        DALI_LOG_ERROR("EGL_BAD_ACCESS: Previously bound off-screen, or EGLImage sibling error\n");
        break;
      }
      case EGL_BAD_ALLOC:
      {
        DALI_LOG_ERROR("EGL_BAD_ALLOC: Insufficient memory is available\n");
        break;
      }
      default:
      {
        DALI_LOG_ERROR("Unknown error. eglGetError return[0x%x]\n", error);
        break;
      }
    }
  }
  DALI_ASSERT_DEBUG(EGL_NO_IMAGE_KHR != eglImage && "EglImageExtensions::CreateImageKHR: eglCreateImageKHR failed!\n");

  return eglImage;
}

void EglImageExtensions::DestroyImageKHR(void* eglImageKHR)
{
  DALI_ASSERT_DEBUG(mImageKHRInitialized);

  if(!mImageKHRInitialized)
  {
    return;
  }

  if(eglImageKHR == NULL)
  {
    return;
  }

  EGLImageKHR eglImage = static_cast<EGLImageKHR>(eglImageKHR);

  START_DURATION_CHECK();
  EGLBoolean result = eglDestroyImageKHRProc(mEglImplementation->GetDisplay(), eglImage);
  FINISH_DURATION_CHECK("eglDestroyImageKHRProc");

  if(EGL_FALSE == result)
  {
    EGLint error = eglGetError();
    switch(error)
    {
      case EGL_BAD_DISPLAY:
      {
        DALI_LOG_ERROR("EGL_BAD_DISPLAY: Invalid EGLDisplay object\n");
        break;
      }
      case EGL_BAD_PARAMETER:
      {
        DALI_LOG_ERROR("EGL_BAD_PARAMETER: eglImage is not a valid EGLImageKHR object created with respect to EGLDisplay\n");
        break;
      }
      case EGL_BAD_ACCESS:
      {
        DALI_LOG_ERROR("EGL_BAD_ACCESS: EGLImage sibling error\n");
        break;
      }
      default:
      {
        DALI_LOG_ERROR("Unknown error. eglGetError return[0x%x]\n", error);
        break;
      }
    }
  }
}

void EglImageExtensions::TargetTextureKHR(void* eglImageKHR)
{
  DALI_ASSERT_DEBUG(mImageKHRInitialized);

  if(eglImageKHR != NULL)
  {
    EGLImageKHR eglImage = static_cast<EGLImageKHR>(eglImageKHR);

#ifdef EGL_ERROR_CHECKING
    GLint glError = glGetError();
    if(GL_NO_ERROR != glError)
    {
      DALI_LOG_ERROR(" before glEGLImageTargetTexture2DOES returned error 0x%04x\n", glError);
    }
#endif

    START_DURATION_CHECK();
    glEGLImageTargetTexture2DOESProc(GL_TEXTURE_EXTERNAL_OES, reinterpret_cast<GLeglImageOES>(eglImage));
    FINISH_DURATION_CHECK("glEGLImageTargetTexture2DOESProc");

#ifdef EGL_ERROR_CHECKING
    glError = glGetError();
    if(GL_NO_ERROR != glError)
    {
      DALI_LOG_ERROR(" glEGLImageTargetTexture2DOES returned error 0x%04x\n", glError);
    }
#endif
  }
}

void EglImageExtensions::InitializeEglImageKHR()
{
  // avoid trying to reload extended KHR functions, if it fails the first time
  if(!mImageKHRInitializeFailed)
  {
    START_DURATION_CHECK();
    eglCreateImageKHRProc            = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
    eglDestroyImageKHRProc           = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
    glEGLImageTargetTexture2DOESProc = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));
    FINISH_DURATION_CHECK("eglGetProcAddress");
  }

  if(eglCreateImageKHRProc && eglDestroyImageKHRProc && glEGLImageTargetTexture2DOESProc)
  {
    mImageKHRInitialized = true;
  }
  else
  {
    mImageKHRInitializeFailed = true;
  }

  START_DURATION_CHECK();
  std::string extensionStr = eglQueryString(mEglImplementation->GetDisplay(), EGL_EXTENSIONS);
  FINISH_DURATION_CHECK("eglQueryString(EGL_EXTENSIONS)");

  auto found = extensionStr.find(EGL_TIZEN_IMAGE_NATIVE_SURFACE);
  if(found != std::string::npos)
  {
    mImpl->mIsTizenImageNativeSurfaceSupported = true;
  }

  found = extensionStr.find(EGL_EXT_IMAGE_DMA_BUF_IMPORT);
  if(found != std::string::npos)
  {
    mImpl->mIsExtImageDmaBufImportSupported = true;
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
