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
#include <dali/internal/window-system/tizen-wayland/native-image-surface-impl-ecore-wl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <system_info.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-graphics-factory.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/graphics/gles/egl-implementation.h>
#include <dali/internal/system/common/environment-options.h>
#include <dali/internal/window-system/common/display-utils.h>

using namespace Dali::Internal::Adaptor;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
NativeImageSurfaceEcoreWl::NativeImageSurfaceEcoreWl(Dali::NativeImageSourceQueuePtr queue)
: mDisplayConnection(nullptr),
  mGraphics(nullptr),
  mEGL(nullptr),
  mEGLSurface(nullptr),
  mEGLContext(nullptr),
  mColorDepth(COLOR_DEPTH_32),
  mTbmFormat(0u),
  mTbmQueue(nullptr),
  mDepth(false),
  mStencil(false),
  mGLESVersion(30),
  mMSAA(0)
{
  if(queue)
  {
    mTbmQueue   = AnyCast<tbm_surface_queue_h>(queue->GetNativeImageSourceQueue());
    mTbmFormat  = tbm_surface_queue_get_format(mTbmQueue);
    mColorDepth = (mTbmFormat == TBM_FORMAT_ARGB8888) ? COLOR_DEPTH_32 : COLOR_DEPTH_24;
  }
  else
  {
    DALI_LOG_ERROR("NativeImageSourceQueue is null.");
  }
}

bool NativeImageSurfaceEcoreWl::SetGraphicsConfig(bool depth, bool stencil, int msaa, int version)
{
  // Setup the configuration
  // The GLES version support is done by the caller
  mDepth   = depth;
  mStencil = stencil;
  if(mMSAA == 0)
  {
    //EGL_DONT_CARE is -1
    mMSAA = -1;
  }
  else
  {
    mMSAA = msaa;
  }
  mGLESVersion = version;

  return true;
}

Any NativeImageSurfaceEcoreWl::GetNativeRenderable()
{
  return mTbmQueue;
}

void NativeImageSurfaceEcoreWl::InitializeGraphics()
{
  std::unique_ptr<GraphicsFactory> graphicsFactoryPtr = Utils::MakeUnique<GraphicsFactory>(*(new EnvironmentOptions()));
  auto                             graphicsFactory    = *graphicsFactoryPtr.get();

  mGraphics                      = std::unique_ptr<GraphicsInterface>(&graphicsFactory.Create());
  GraphicsInterface* graphics    = mGraphics.get();
  auto               eglGraphics = static_cast<EglGraphics*>(graphics);
  eglGraphics->Initialize(mDepth, mStencil, false, mMSAA);

  mDisplayConnection = std::unique_ptr<Dali::DisplayConnection>(Dali::DisplayConnection::New(*mGraphics, Dali::RenderSurfaceInterface::Type::NATIVE_RENDER_SURFACE));
  mDisplayConnection->Initialize();

  mEGL = &eglGraphics->GetEglInterface();

  if(mEGLContext == NULL)
  {
    Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>(*mEGL);
    eglImpl.SetGlesVersion(mGLESVersion);

    if(eglImpl.ChooseConfig(true, mColorDepth) == false)
    {
      DALI_LOG_ERROR("InitializeGraphics: Fail to choose config. Version:%d, ColorDepth:%d, depth:%d, stencil:%d, MSAA:%d",
                     mGLESVersion,
                     mColorDepth == COLOR_DEPTH_32 ? 32 : 24,
                     mDepth ? 24 : 0,
                     mStencil ? 8 : 0,
                     mMSAA);
      return;
    }

    // Create the OpenGL Surface & Context
    eglImpl.CreateWindowContext(mEGLContext);
    mEGLSurface = eglImpl.CreateSurfaceWindow(reinterpret_cast<EGLNativeWindowType>(mTbmQueue), mColorDepth);

    MakeContextCurrent();
  }
}

void NativeImageSurfaceEcoreWl::TerminateGraphics()
{
  GraphicsInterface* graphics    = mGraphics.get();
  auto               eglGraphics = static_cast<EglGraphics*>(graphics);

  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();
  if(mEGLSurface)
  {
    eglImpl.DestroySurface(mEGLSurface);
  }

  if(mEGLContext)
  {
    eglImpl.DestroyContext(mEGLContext);
  }
}

void NativeImageSurfaceEcoreWl::PreRender()
{
  MakeContextCurrent();
}

void NativeImageSurfaceEcoreWl::PostRender()
{
  GraphicsInterface* graphics    = mGraphics.get();
  auto               eglGraphics = static_cast<EglGraphics*>(graphics);
  if(eglGraphics)
  {
    Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();
    eglImpl.SwapBuffers(mEGLSurface);
  }
}

void NativeImageSurfaceEcoreWl::MakeContextCurrent()
{
  if(mEGL != nullptr)
  {
    if(mEGLSurface && mEGLContext)
    {
      mEGL->MakeContextCurrent(mEGLSurface, mEGLContext);
    }
    else
    {
      DALI_LOG_ERROR("EGLSurface(%p) or mEGLContext(%p) is null\n", mEGLSurface, mEGLContext);
    }
  }
}

bool NativeImageSurfaceEcoreWl::CanRender()
{
  return tbm_surface_queue_can_dequeue(mTbmQueue, 0);
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
