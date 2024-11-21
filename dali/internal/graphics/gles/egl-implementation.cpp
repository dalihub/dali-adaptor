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
#include <dali/internal/graphics/gles/egl-implementation.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/trace.h>
#include <dali/public-api/common/dali-vector.h>
#include <limits>
#include <sstream>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/graphics/gles/egl-debug.h>
#include <dali/internal/graphics/gles/gl-implementation.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/common/time-service.h>
#include <dali/public-api/dali-adaptor-common.h>

// EGL constants use C style casts
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

namespace
{
#ifndef DALI_PROFILE_UBUNTU
const uint32_t THRESHOLD_SWAPBUFFER_COUNT = 20;
#else
const uint32_t THRESHOLD_SWAPBUFFER_COUNT = 5;
#endif
const uint32_t CHECK_EXTENSION_NUMBER                  = 4;
const uint32_t EGL_VERSION_SUPPORT_SURFACELESS_CONTEXT = 15;
const char*    EGL_KHR_SURFACELESS_CONTEXT             = "EGL_KHR_surfaceless_context";
const char*    EGL_KHR_CREATE_CONTEXT                  = "EGL_KHR_create_context";
const char*    EGL_KHR_PARTIAL_UPDATE                  = "EGL_KHR_partial_update";
const char*    EGL_KHR_SWAP_BUFFERS_WITH_DAMAGE        = "EGL_KHR_swap_buffers_with_damage";

#ifndef DALI_PROFILE_TV // Avoid HWC log printing in TV
DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_EGL, true);
#else
DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_EGL, false);
#endif

DALI_INIT_TIME_CHECKER_FILTER(gTimeCheckerFilter, DALI_EGL_PERFORMANCE_LOG_THRESHOLD_TIME);
} // namespace

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
#define TEST_EGL_ERROR(lastCommand)                        \
  {                                                        \
    EGLint err = eglGetError();                            \
    if(err != EGL_SUCCESS)                                 \
    {                                                      \
      DALI_LOG_ERROR("EGL error after %s\n", lastCommand); \
      Egl::PrintError(err);                                \
      DALI_ASSERT_ALWAYS(0 && "EGL error");                \
    }                                                      \
  }

EglImplementation::EglImplementation(int                                 multiSamplingLevel,
                                     Integration::DepthBufferAvailable   depthBufferRequired,
                                     Integration::StencilBufferAvailable stencilBufferRequired,
                                     Integration::PartialUpdateAvailable partialUpdateRequired)
: mContextAttribs(),
  mEglNativeDisplay(0),
  mEglNativeWindow(0),
  mCurrentEglNativePixmap(0),
  mEglDisplay(0),
  mEglConfig(0),
  mEglContext(0),
  mCurrentEglSurface(0),
  mCurrentEglContext(EGL_NO_CONTEXT),
  mMultiSamplingLevel(multiSamplingLevel),
  mGlesVersion(30),
  mColorDepth(COLOR_DEPTH_24),
  mGlesInitialized(false),
  mIsOwnSurface(true),
  mIsWindow(true),
  mDepthBufferRequired(depthBufferRequired == Integration::DepthBufferAvailable::TRUE),
  mStencilBufferRequired(stencilBufferRequired == Integration::StencilBufferAvailable::TRUE),
  mPartialUpdateRequired(partialUpdateRequired == Integration::PartialUpdateAvailable::TRUE),
  mIsSurfacelessContextSupported(false),
  mIsKhrCreateContextSupported(false),
  mSwapBufferCountAfterResume(0),
  mEglSetDamageRegionKHR(0),
  mEglSwapBuffersWithDamageKHR(0)
{
}

EglImplementation::~EglImplementation()
{
  TerminateGles();
}

bool EglImplementation::InitializeGles(EGLNativeDisplayType display, bool isOwnSurface)
{
  if(!mGlesInitialized)
  {
    mEglNativeDisplay = display;

    {
      DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_GET_DISPLAY", [&](std::ostringstream& oss) {
        oss << "[native display:" << mEglNativeDisplay << "]";
      });
      DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
      // Try to get the display connection for the native display first
      mEglDisplay = eglGetDisplay(mEglNativeDisplay);
      DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "eglGetDisplay");
      DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_GET_DISPLAY", [&](std::ostringstream& oss) {
        oss << "[display:" << mEglDisplay << "]";
      });
    }

    if(mEglDisplay == EGL_NO_DISPLAY)
    {
      DALI_TRACE_BEGIN(gTraceFilter, "DALI_EGL_GET_DISPLAY");
      DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
      // If failed, try to get the default display connection
      mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
      DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "eglGetDisplay(default)");
      DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_GET_DISPLAY", [&](std::ostringstream& oss) {
        oss << "[display:" << mEglDisplay << "]";
      });
    }

    if(mEglDisplay == EGL_NO_DISPLAY)
    {
      // Still failed to get a display connection
      throw Dali::DaliException("", "OpenGL ES is not supported");
    }

    EGLint majorVersion = 0;
    EGLint minorVersion = 0;

    {
      DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_INITIALIZE", [&](std::ostringstream& oss) {
        oss << "[display:" << mEglDisplay << "]";
      });
      DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
      bool ret = eglInitialize(mEglDisplay, &majorVersion, &minorVersion);
      DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "eglInitialize");
      DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_INITIALIZE", [&](std::ostringstream& oss) {
        oss << "[ret:" << ret << " version:" << majorVersion << "." << minorVersion << "]";
      });

      if(!ret)
      {
        return false;
      }
    }

    {
      DALI_TRACE_SCOPE(gTraceFilter, "DALI_EGL_BIND_API");
      DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglBindAPI");
      eglBindAPI(EGL_OPENGL_ES_API);
    }

    mIsOwnSurface = isOwnSurface;
  }

  {
    DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
    const char* const versionStr   = eglQueryString(mEglDisplay, EGL_VERSION);
    const char* const extensionStr = eglQueryString(mEglDisplay, EGL_EXTENSIONS);
    const char* const vendorStr    = eglQueryString(mEglDisplay, EGL_VENDOR);
    const char* const clientStr    = eglQueryString(mEglDisplay, EGL_CLIENT_APIS);
    DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "eglQueryString");

    // Query EGL extensions to check whether required extensions are supported
    std::istringstream versionStream(versionStr);
    std::string        majorVersion, minorVersion;
    std::getline(versionStream, majorVersion, '.');
    std::getline(versionStream, minorVersion);
    uint32_t extensionCheckCount = 0;
    if(stoul(majorVersion) * 10 + stoul(minorVersion) >= EGL_VERSION_SUPPORT_SURFACELESS_CONTEXT)
    {
      mIsSurfacelessContextSupported = true;
      mIsKhrCreateContextSupported   = true;
      extensionCheckCount += 2;
    }

    std::istringstream stream(extensionStr);
    std::string        currentExtension;
    bool               isKhrPartialUpdateSupported         = false;
    bool               isKhrSwapBuffersWithDamageSupported = false;
    while(std::getline(stream, currentExtension, ' ') && extensionCheckCount < CHECK_EXTENSION_NUMBER)
    {
      if(currentExtension == EGL_KHR_SURFACELESS_CONTEXT && !mIsSurfacelessContextSupported)
      {
        mIsSurfacelessContextSupported = true;
        extensionCheckCount++;
      }
      if(currentExtension == EGL_KHR_CREATE_CONTEXT && !mIsKhrCreateContextSupported)
      {
        mIsKhrCreateContextSupported = true;
        extensionCheckCount++;
      }
      if(currentExtension == EGL_KHR_PARTIAL_UPDATE)
      {
        isKhrPartialUpdateSupported = true;
        extensionCheckCount++;
      }
      if(currentExtension == EGL_KHR_SWAP_BUFFERS_WITH_DAMAGE)
      {
        isKhrSwapBuffersWithDamageSupported = true;
        extensionCheckCount++;
      }
    }

    if(!isKhrPartialUpdateSupported || !isKhrSwapBuffersWithDamageSupported)
    {
      mPartialUpdateRequired = false;
    }

    mGlesInitialized = true;

    // We want to display this information all the time, so use the LogMessage directly
    Integration::Log::LogMessage(Integration::Log::INFO,
                                 "EGL Information\n"
                                 "            PartialUpdate  %d\n"
                                 "            Vendor:        %s\n"
                                 "            Version:       %s\n"
                                 "            Client APIs:   %s\n"
                                 "            Extensions:    %s\n",
                                 mPartialUpdateRequired,
                                 vendorStr,
                                 versionStr,
                                 clientStr,
                                 extensionStr);
  }

  return mGlesInitialized;
}

bool EglImplementation::CreateContext()
{
  // make sure a context isn't created twice
  DALI_ASSERT_ALWAYS((mEglContext == 0) && "EGL context recreated");

  {
    DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_CREATE_CONTEXT", [&](std::ostringstream& oss) {
      oss << "[display:" << mEglDisplay << "]";
    });
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglCreateContext");
    mEglContext = eglCreateContext(mEglDisplay, mEglConfig, NULL, &(mContextAttribs[0]));
    DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_CREATE_CONTEXT", [&](std::ostringstream& oss) {
      oss << "[context:" << mEglContext << "]";
    });
  }
  TEST_EGL_ERROR("eglCreateContext render thread");

  DALI_ASSERT_ALWAYS(EGL_NO_CONTEXT != mEglContext && "EGL context not created");

  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_VENDOR : %s ***\n", glGetString(GL_VENDOR));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_RENDERER : %s ***\n", glGetString(GL_RENDERER));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_VERSION : %s ***\n", glGetString(GL_VERSION));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_SHADING_LANGUAGE_VERSION : %s***\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** Supported Extensions ***\n%s\n\n", glGetString(GL_EXTENSIONS));

  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglGetProcAddress(\"eglSetDamageRegionKHR\")");
    mEglSetDamageRegionKHR = reinterpret_cast<PFNEGLSETDAMAGEREGIONKHRPROC>(eglGetProcAddress("eglSetDamageRegionKHR"));
    if(!mEglSetDamageRegionKHR)
    {
      DALI_LOG_ERROR("Coudn't find eglSetDamageRegionKHR!\n");
      mPartialUpdateRequired = false;
    }
  }
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglGetProcAddress(\"eglSwapBuffersWithDamageKHR\")");
    mEglSwapBuffersWithDamageKHR = reinterpret_cast<PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC>(eglGetProcAddress("eglSwapBuffersWithDamageKHR"));
    if(!mEglSwapBuffersWithDamageKHR)
    {
      DALI_LOG_ERROR("Coudn't find eglSwapBuffersWithDamageKHR!\n");
      mPartialUpdateRequired = false;
    }
  }
  return true;
}

bool EglImplementation::CreateWindowContext(EGLContext& eglContext)
{
  // make sure a context isn't created twice
  DALI_ASSERT_ALWAYS((eglContext == 0) && "EGL context recreated");

  {
    DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_CREATE_CONTEXT", [&](std::ostringstream& oss) {
      oss << "[display:" << mEglDisplay << ", share_context:" << mEglContext << "]";
    });
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglCreateContext");
    eglContext = eglCreateContext(mEglDisplay, mEglConfig, mEglContext, &(mContextAttribs[0]));
    DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_CREATE_CONTEXT", [&](std::ostringstream& oss) {
      oss << "[context:" << eglContext << "]";
    });
  }
  TEST_EGL_ERROR("eglCreateContext render thread");

  DALI_ASSERT_ALWAYS(EGL_NO_CONTEXT != eglContext && "EGL context not created");

  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_VENDOR : %s ***\n", glGetString(GL_VENDOR));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_RENDERER : %s ***\n", glGetString(GL_RENDERER));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_VERSION : %s ***\n", glGetString(GL_VERSION));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** GL_SHADING_LANGUAGE_VERSION : %s***\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  DALI_LOG_INFO(Debug::Filter::gShader, Debug::General, "*** Supported Extensions ***\n%s\n\n", glGetString(GL_EXTENSIONS));

  mEglWindowContexts.push_back(eglContext);

  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglGetProcAddress(\"eglSetDamageRegionKHR\")");
    mEglSetDamageRegionKHR = reinterpret_cast<PFNEGLSETDAMAGEREGIONKHRPROC>(eglGetProcAddress("eglSetDamageRegionKHR"));
    if(!mEglSetDamageRegionKHR)
    {
      DALI_LOG_ERROR("Coudn't find eglSetDamageRegionKHR!\n");
      mPartialUpdateRequired = false;
    }
  }
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglGetProcAddress(\"eglSwapBuffersWithDamageKHR\")");
    mEglSwapBuffersWithDamageKHR = reinterpret_cast<PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC>(eglGetProcAddress("eglSwapBuffersWithDamageKHR"));
    if(!mEglSwapBuffersWithDamageKHR)
    {
      DALI_LOG_ERROR("Coudn't find eglSwapBuffersWithDamageKHR!\n");
      mPartialUpdateRequired = false;
    }
  }
  return true;
}

void EglImplementation::DestroyContext(EGLContext& eglContext)
{
  if(eglContext)
  {
    // Remove from the list of contexts first.
    auto iter = std::find(mEglWindowContexts.begin(), mEglWindowContexts.end(), eglContext);
    if(iter != mEglWindowContexts.end())
    {
      mEglWindowContexts.erase(iter);
    }

    DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_DESTROY_CONTEXT", [&](std::ostringstream& oss) {
      oss << "[display:" << mEglDisplay << ", context:" << eglContext << "]";
    });

    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglDestroyContext");
    eglDestroyContext(mEglDisplay, eglContext);
    eglContext = 0;
    DALI_TRACE_END(gTraceFilter, "DALI_EGL_DESTROY_CONTEXT");
  }
}

void EglImplementation::DestroySurface(EGLSurface& eglSurface)
{
  if(mIsOwnSurface && eglSurface)
  {
    // Make context null to prevent crash in driver side
    MakeContextNull();
    DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_DESTROY_SURFACE", [&](std::ostringstream& oss) {
      oss << "[display:" << mEglDisplay << ", surface:" << eglSurface << "]";
    });
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglDestroySurface");
    eglDestroySurface(mEglDisplay, eglSurface);
    eglSurface = 0;
    DALI_TRACE_END(gTraceFilter, "DALI_EGL_DESTROY_SURFACE");
  }
}

void EglImplementation::MakeContextCurrent(EGLSurface eglSurface, EGLContext eglContext)
{
  if(mCurrentEglContext == eglContext)
  {
    return;
  }

  mCurrentEglSurface = eglSurface;

  if(mIsOwnSurface)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglMakeCurrent");
    eglMakeCurrent(mEglDisplay, eglSurface, eglSurface, eglContext);

    mCurrentEglContext = eglContext;
  }

  EGLint error = eglGetError();

  if(error != EGL_SUCCESS)
  {
    Egl::PrintError(error);

    DALI_ASSERT_ALWAYS(false && "MakeContextCurrent failed!");
  }
}

void EglImplementation::MakeCurrent(EGLNativePixmapType pixmap, EGLSurface eglSurface)
{
  if(mCurrentEglContext == mEglContext)
  {
    return;
  }

  mCurrentEglNativePixmap = pixmap;
  mCurrentEglSurface      = eglSurface;

  if(mIsOwnSurface)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglMakeCurrent(pixmap)");
    eglMakeCurrent(mEglDisplay, eglSurface, eglSurface, mEglContext);

    mCurrentEglContext = mEglContext;
  }

  EGLint error = eglGetError();

  if(error != EGL_SUCCESS)
  {
    Egl::PrintError(error);

    DALI_ASSERT_ALWAYS(false && "MakeCurrent failed!");
  }
}

void EglImplementation::MakeContextNull()
{
  // clear the current context
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglMakeCurrent(null)");
  eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  mCurrentEglContext = EGL_NO_CONTEXT;
}

void EglImplementation::TerminateGles()
{
  if(mGlesInitialized)
  {
    // Make context null to prevent crash in driver side
    MakeContextNull();

    if(mIsOwnSurface)
    {
      for(auto eglSurface : mEglWindowSurfaces)
      {
        if(eglSurface)
        {
          DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_DESTROY_SURFACE", [&](std::ostringstream& oss) {
            oss << "[display:" << mEglDisplay << ", surface:" << eglSurface << "]";
          });
          DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglDestroySurface");
          eglDestroySurface(mEglDisplay, eglSurface);
          DALI_TRACE_END(gTraceFilter, "DALI_EGL_DESTROY_SURFACE");
        }
      }
    }
    {
      DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_DESTROY_CONTEXT", [&](std::ostringstream& oss) {
        oss << "[display:" << mEglDisplay << ", context:" << mEglContext << "]";
      });
      DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglDestroyContext");
      eglDestroyContext(mEglDisplay, mEglContext);
      DALI_TRACE_END(gTraceFilter, "DALI_EGL_DESTROY_CONTEXT");
    }
    for(auto eglContext : mEglWindowContexts)
    {
      DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_DESTROY_CONTEXT", [&](std::ostringstream& oss) {
        oss << "[display:" << mEglDisplay << ", context:" << eglContext << "]";
      });
      DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglDestroyContext");
      eglDestroyContext(mEglDisplay, eglContext);
      DALI_TRACE_END(gTraceFilter, "DALI_EGL_DESTROY_CONTEXT");
    }

    {
      DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_TERMINATE", [&](std::ostringstream& oss) {
        oss << "[display:" << mEglDisplay << "]";
      });
      DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglTerminate");
      eglTerminate(mEglDisplay);
      DALI_TRACE_END(gTraceFilter, "DALI_EGL_TERMINATE");
    }

    mEglDisplay        = NULL;
    mEglConfig         = NULL;
    mEglContext        = NULL;
    mCurrentEglSurface = NULL;
    mCurrentEglContext = EGL_NO_CONTEXT;

    mGlesInitialized = false;
  }
}

bool EglImplementation::IsGlesInitialized() const
{
  return mGlesInitialized;
}

void EglImplementation::SwapBuffers(EGLSurface& eglSurface)
{
  if(eglSurface != EGL_NO_SURFACE) // skip if using surfaceless context
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglSwapBuffers");

#ifndef DALI_PROFILE_UBUNTU
    if(mSwapBufferCountAfterResume < THRESHOLD_SWAPBUFFER_COUNT)
    {
      DALI_LOG_RELEASE_INFO("EglImplementation::eglSwapBuffers started. eglSurface(%p)\n", eglSurface);
    }
    DALI_TRACE_BEGIN(gTraceFilter, "DALI_EGL_SWAP_BUFFERS");
#endif //DALI_PROFILE_UBUNTU

    // DALI_LOG_ERROR("EglImplementation::SwapBuffers()\n");
    eglSwapBuffers(mEglDisplay, eglSurface);

#ifndef DALI_PROFILE_UBUNTU
    DALI_TRACE_END(gTraceFilter, "DALI_EGL_SWAP_BUFFERS");
    if(mSwapBufferCountAfterResume < THRESHOLD_SWAPBUFFER_COUNT)
    {
      DALI_LOG_RELEASE_INFO("EglImplementation::eglSwapBuffers finished.\n");
      mSwapBufferCountAfterResume++;
    }
#endif //DALI_PROFILE_UBUNTU
  }
}

int EglImplementation::GetBufferAge(EGLSurface& eglSurface) const
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglQuerySurface");

  int age = 0;
  eglQuerySurface(mEglDisplay, eglSurface, EGL_BUFFER_AGE_EXT, &age);
  if(age < 0)
  {
    DALI_LOG_ERROR("eglQuerySurface(%d)\n", eglGetError());
    age = 0;
  }

  return age;
}

void EglImplementation::SetDamageRegion(EGLSurface& eglSurface, std::vector<Rect<int>>& damagedRects)
{
  if(!mPartialUpdateRequired)
  {
    return;
  }

  if(eglSurface != EGL_NO_SURFACE) // skip if using surfaceless context
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglSetDamageRegionKHR");
    EGLBoolean result = mEglSetDamageRegionKHR(mEglDisplay, eglSurface, reinterpret_cast<int*>(damagedRects.data()), 1);
    if(result == EGL_FALSE)
    {
      DALI_LOG_ERROR("eglSetDamageRegionKHR(0x%x)\n", eglGetError());
    }
  }
}

void EglImplementation::SwapBuffers(EGLSurface& eglSurface, const std::vector<Rect<int>>& damagedRects)
{
  if(eglSurface != EGL_NO_SURFACE) // skip if using surfaceless context
  {
    if(!mPartialUpdateRequired)
    {
      SwapBuffers(eglSurface);
      return;
    }

    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglSwapBuffersWithDamageKHR");

#ifndef DALI_PROFILE_UBUNTU
    if(mSwapBufferCountAfterResume < THRESHOLD_SWAPBUFFER_COUNT)
    {
      DALI_LOG_RELEASE_INFO("EglImplementation::eglSwapBuffersWithDamageKHR started. eglSurface(%p)\n", eglSurface);
    }
    DALI_TRACE_BEGIN(gTraceFilter, "DALI_EGL_SWAP_BUFFERS_KHR");
#endif //DALI_PROFILE_UBUNTU

    EGLBoolean result = mEglSwapBuffersWithDamageKHR(mEglDisplay, eglSurface, reinterpret_cast<int*>(const_cast<std::vector<Rect<int>>&>(damagedRects).data()), damagedRects.size());
    if(result == EGL_FALSE)
    {
      DALI_LOG_ERROR("eglSwapBuffersWithDamageKHR(%d)\n", eglGetError());
    }

#ifndef DALI_PROFILE_UBUNTU
    DALI_TRACE_END(gTraceFilter, "DALI_EGL_SWAP_BUFFERS_KHR");
    if(mSwapBufferCountAfterResume < THRESHOLD_SWAPBUFFER_COUNT)
    {
      DALI_LOG_RELEASE_INFO("EglImplementation::eglSwapBuffersWithDamageKHR finished.\n");
      mSwapBufferCountAfterResume++;
    }
#endif //DALI_PROFILE_UBUNTU
  }
}

void EglImplementation::CopyBuffers(EGLSurface& eglSurface)
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglCopyBuffers");
  eglCopyBuffers(mEglDisplay, eglSurface, mCurrentEglNativePixmap);
}

void EglImplementation::WaitGL()
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglWaitGL");
  eglWaitGL();
}

bool EglImplementation::ChooseConfig(bool isWindowType, ColorDepth depth)
{
  if(mEglConfig && isWindowType == mIsWindow && mColorDepth == depth)
  {
    return true;
  }

  mColorDepth = depth;
  mIsWindow   = isWindowType;

  EGLint         numConfigs;
  Vector<EGLint> configAttribs;
  configAttribs.Reserve(31);

  if(isWindowType)
  {
    configAttribs.PushBack(EGL_SURFACE_TYPE);
    configAttribs.PushBack(EGL_WINDOW_BIT);
  }
  else
  {
    configAttribs.PushBack(EGL_SURFACE_TYPE);
    configAttribs.PushBack(EGL_PIXMAP_BIT);
  }

  configAttribs.PushBack(EGL_RENDERABLE_TYPE);

  if(mGlesVersion >= 30)
  {
    configAttribs.PushBack(EGL_OPENGL_ES3_BIT_KHR);
  }
  else
  {
    configAttribs.PushBack(EGL_OPENGL_ES2_BIT);
  }

  // TODO: enable this flag when it becomes supported
  //  configAttribs.PushBack( EGL_CONTEXT_FLAGS_KHR );
  //  configAttribs.PushBack( EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR );

  configAttribs.PushBack(EGL_RED_SIZE);
  configAttribs.PushBack(8);
  configAttribs.PushBack(EGL_GREEN_SIZE);
  configAttribs.PushBack(8);
  configAttribs.PushBack(EGL_BLUE_SIZE);
  configAttribs.PushBack(8);

  //  For underlay video playback, we also need to set the alpha value of the 24/32bit window.
  // TODO: When the tbm queue of GlView is 24bit, do we have to set the alpha size??
  configAttribs.PushBack(EGL_ALPHA_SIZE);
  configAttribs.PushBack(8);

  configAttribs.PushBack(EGL_DEPTH_SIZE);
  configAttribs.PushBack(mDepthBufferRequired ? 24 : 0);
  configAttribs.PushBack(EGL_STENCIL_SIZE);
  configAttribs.PushBack(mStencilBufferRequired ? 8 : 0);

  if(mMultiSamplingLevel != EGL_DONT_CARE)
  {
    configAttribs.PushBack(EGL_SAMPLES);
    configAttribs.PushBack(mMultiSamplingLevel);
    configAttribs.PushBack(EGL_SAMPLE_BUFFERS);
    configAttribs.PushBack(1);
  }

  configAttribs.PushBack(EGL_NONE);

  DALI_TRACE_BEGIN(gTraceFilter, "DALI_EGL_CHOOSE_CONFIG");
  DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
  auto ret = eglChooseConfig(mEglDisplay, &(configAttribs[0]), &mEglConfig, 1, &numConfigs);
  DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "eglChooseConfig");
  DALI_TRACE_END(gTraceFilter, "DALI_EGL_CHOOSE_CONFIG");

  // Ensure number of configs is set to 1 as on some drivers,
  // eglChooseConfig succeeds but does not actually create a proper configuration.
  if((ret != EGL_TRUE) ||
     (numConfigs != 1))
  {
    if(mGlesVersion >= 30)
    {
      mEglConfig = NULL;
      DALI_LOG_ERROR("Fail to use OpenGL es 3.0. Retrying to use OpenGL es 2.0.");
      return false;
    }

    if(numConfigs != 1)
    {
      DALI_LOG_ERROR("No configurations found.\n");

      TEST_EGL_ERROR("eglChooseConfig");
    }

    EGLint error = eglGetError();
    switch(error)
    {
      case EGL_BAD_DISPLAY:
      {
        DALI_LOG_ERROR("Display is not an EGL display connection\n");
        break;
      }
      case EGL_BAD_ATTRIBUTE:
      {
        DALI_LOG_ERROR("The parameter configAttribs contains an invalid frame buffer configuration attribute or an attribute value that is unrecognized or out of range\n");
        break;
      }
      case EGL_NOT_INITIALIZED:
      {
        DALI_LOG_ERROR("Display has not been initialized\n");
        break;
      }
      case EGL_BAD_PARAMETER:
      {
        DALI_LOG_ERROR("The parameter numConfig is NULL\n");
        break;
      }
      default:
      {
        DALI_LOG_ERROR("Unknown error. eglGetError return[0x%x]\n", error);
      }
    }
    DALI_ASSERT_ALWAYS(false && "eglChooseConfig failed!");
    return false;
  }
  Integration::Log::LogMessage(Integration::Log::INFO, "Using OpenGL es %d.%d.\n", mGlesVersion / 10, mGlesVersion % 10);

  mContextAttribs.Clear();
  if(mIsKhrCreateContextSupported)
  {
    mContextAttribs.Reserve(5);
    mContextAttribs.PushBack(EGL_CONTEXT_MAJOR_VERSION_KHR);
    mContextAttribs.PushBack(mGlesVersion / 10);
    mContextAttribs.PushBack(EGL_CONTEXT_MINOR_VERSION_KHR);
    mContextAttribs.PushBack(mGlesVersion % 10);
  }
  else
  {
    mContextAttribs.Reserve(3);
    mContextAttribs.PushBack(EGL_CONTEXT_CLIENT_VERSION);
    mContextAttribs.PushBack(mGlesVersion / 10);
  }
  mContextAttribs.PushBack(EGL_NONE);

  return true;
}

EGLSurface EglImplementation::CreateSurfaceWindow(EGLNativeWindowType window, ColorDepth depth)
{
  mEglNativeWindow = window;
  mColorDepth      = depth;
  mIsWindow        = true;

  // egl choose config
  ChooseConfig(mIsWindow, mColorDepth);

  {
    DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_CREATE_SURFACE", [&](std::ostringstream& oss) {
      oss << "[display:" << mEglDisplay << "]";
    });
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglCreateWindowSurface");
    mCurrentEglSurface = eglCreateWindowSurface(mEglDisplay, mEglConfig, mEglNativeWindow, NULL);
    DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_CREATE_SURFACE", [&](std::ostringstream& oss) {
      oss << "[window surface:" << mCurrentEglSurface << "]";
    });
  }

  TEST_EGL_ERROR("eglCreateWindowSurface");

  DALI_ASSERT_ALWAYS(mCurrentEglSurface && "Create window surface failed");

  return mCurrentEglSurface;
}

EGLSurface EglImplementation::CreateSurfacePixmap(EGLNativePixmapType pixmap, ColorDepth depth)
{
  mCurrentEglNativePixmap = pixmap;
  mColorDepth             = depth;
  mIsWindow               = false;

  // egl choose config
  ChooseConfig(mIsWindow, mColorDepth);

  {
    DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_CREATE_SURFACE", [&](std::ostringstream& oss) {
      oss << "[display:" << mEglDisplay << "]";
    });
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglCreatePixmapSurface");
    mCurrentEglSurface = eglCreatePixmapSurface(mEglDisplay, mEglConfig, mCurrentEglNativePixmap, NULL);
    DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_EGL_CREATE_SURFACE", [&](std::ostringstream& oss) {
      oss << "[pixmap surface:" << mCurrentEglSurface << "]";
    });
  }

  TEST_EGL_ERROR("eglCreatePixmapSurface");

  DALI_ASSERT_ALWAYS(mCurrentEglSurface && "Create pixmap surface failed");

  return mCurrentEglSurface;
}

bool EglImplementation::ReplaceSurfaceWindow(EGLNativeWindowType window, EGLSurface& eglSurface, EGLContext& eglContext)
{
  bool contextLost = false;

  // display connection has not changed, then we can just create a new surface
  //  the surface is bound to the context, so set the context to null
  MakeContextNull();

  if(eglSurface)
  {
    // destroy the surface
    DestroySurface(eglSurface);
  }

  // create the EGL surface
  EGLSurface newEglSurface = CreateSurfaceWindow(window, mColorDepth);

  // set the context to be current with the new surface
  MakeContextCurrent(newEglSurface, eglContext);

  return contextLost;
}

bool EglImplementation::ReplaceSurfacePixmap(EGLNativePixmapType pixmap, EGLSurface& eglSurface)
{
  bool contextLost = false;

  // display connection has not changed, then we can just create a new surface
  // create the EGL surface
  eglSurface = CreateSurfacePixmap(pixmap, mColorDepth);

  // set the eglSurface to be current
  MakeCurrent(pixmap, eglSurface);

  return contextLost;
}

void EglImplementation::SetGlesVersion(const int32_t glesVersion)
{
  mGlesVersion = glesVersion;
}

void EglImplementation::SetFirstFrameAfterResume()
{
  mSwapBufferCountAfterResume = 0;
}

EGLDisplay EglImplementation::GetDisplay() const
{
  return mEglDisplay;
}

EGLContext EglImplementation::GetContext() const
{
  return mEglContext;
}

int32_t EglImplementation::GetGlesVersion() const
{
  return mGlesVersion;
}

bool EglImplementation::IsSurfacelessContextSupported() const
{
  return mIsSurfacelessContextSupported;
}

void EglImplementation::WaitClient()
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "eglWaitClient");
  // Wait for EGL to finish executing all rendering calls for the current context
  if(eglWaitClient() != EGL_TRUE)
  {
    TEST_EGL_ERROR("eglWaitClient");
  }
}

bool EglImplementation::IsPartialUpdateRequired() const
{
  return mPartialUpdateRequired;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
