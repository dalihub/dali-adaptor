/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/tizen-wayland/native-render-surface-ecore-wl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-abstraction.h>

#ifdef ECORE_WAYLAND2
#include <Ecore_Wl2.h>
#else
#include <Ecore_Wayland.h>
#endif

#include <tbm_bufmgr.h>
#include <tbm_surface_internal.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/thread-synchronization-interface.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/adaptor-internal-services.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/graphics/gles/egl-implementation.h>
#include <dali/internal/system/common/trigger-event.h>
#include <dali/internal/window-system/common/display-connection.h>

namespace Dali
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gNativeSurfaceLogFilter = Debug::Filter::New(Debug::Verbose, false, "LOG_NATIVE_RENDER_SURFACE");
#endif

// The callback of tbm_surface_queue_add_acquirable_cb()
static void TbmAcquirableCallback(tbm_surface_queue_h queue, void* data)
{
  NativeRenderSurfaceEcoreWl* surface = static_cast<NativeRenderSurfaceEcoreWl*>(data);
  if(surface)
  {
    surface->TriggerFrameRenderedCallback();
  }
}

} // unnamed namespace

NativeRenderSurfaceEcoreWl::NativeRenderSurfaceEcoreWl(SurfaceSize surfaceSize, Any surface, bool isTransparent)
: mRenderNotification(NULL),
  mGraphics(NULL),
  mEGL(nullptr),
  mEGLSurface(nullptr),
  mEGLContext(nullptr),
  mOwnSurface(false),
  mTbmQueue(NULL),
  mThreadSynchronization(NULL)
{
  if(surface.Empty())
  {
    mSurfaceSize = surfaceSize;
    mColorDepth  = isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24;
    mTbmFormat   = isTransparent ? TBM_FORMAT_ARGB8888 : TBM_FORMAT_RGB888;
    CreateNativeRenderable();
  }
  else
  {
    mTbmQueue = AnyCast<tbm_surface_queue_h>(surface);

    uint16_t width  = static_cast<uint16_t>(tbm_surface_queue_get_width(mTbmQueue));
    uint16_t height = static_cast<uint16_t>(tbm_surface_queue_get_height(mTbmQueue));
    mSurfaceSize    = SurfaceSize(width, height);

    mTbmFormat = tbm_surface_queue_get_format(mTbmQueue);

    mColorDepth = (mTbmFormat == TBM_FORMAT_ARGB8888) ? COLOR_DEPTH_32 : COLOR_DEPTH_24;
  }
}

NativeRenderSurfaceEcoreWl::~NativeRenderSurfaceEcoreWl()
{
  if(mEGLSurface)
  {
    DestroySurface();
  }

  // release the surface if we own one
  if(mOwnSurface)
  {
    if(mTbmQueue)
    {
      tbm_surface_queue_destroy(mTbmQueue);
    }

    DALI_LOG_INFO(gNativeSurfaceLogFilter, Debug::General, "Own tbm surface queue destroy\n");
  }
}

void NativeRenderSurfaceEcoreWl::SetRenderNotification(TriggerEventInterface* renderNotification)
{
  mRenderNotification = renderNotification;
}

Any NativeRenderSurfaceEcoreWl::GetNativeRenderable()
{
  return mTbmQueue;
}

void NativeRenderSurfaceEcoreWl::TriggerFrameRenderedCallback()
{
  if(mFrameRenderedCallback)
  {
    mFrameRenderedCallback->Trigger();
  }
}

void NativeRenderSurfaceEcoreWl::SetFrameRenderedCallback(CallbackBase* callback)
{
  mFrameRenderedCallback = std::unique_ptr<EventThreadCallback>(new EventThreadCallback(callback));

  tbm_surface_queue_error_e result = tbm_surface_queue_add_acquirable_cb(mTbmQueue, TbmAcquirableCallback, this);
  if(result != TBM_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed calling tbm_surface_queue_add_acquirable_cb(), error : %x", result);
  }
}

PositionSize NativeRenderSurfaceEcoreWl::GetPositionSize() const
{
  return PositionSize(0, 0, static_cast<int>(mSurfaceSize.GetWidth()), static_cast<int>(mSurfaceSize.GetHeight()));
}

void NativeRenderSurfaceEcoreWl::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  // calculate DPI
  float xres, yres;

  // 1 inch = 25.4 millimeters
#ifdef ECORE_WAYLAND2
  // TODO: Application should set dpi value in wayland2
  xres = 96;
  yres = 96;
#else
  xres = ecore_wl_dpi_get();
  yres = ecore_wl_dpi_get();
#endif

  dpiHorizontal = int(xres + 0.5f); // rounding
  dpiVertical   = int(yres + 0.5f);
}

int NativeRenderSurfaceEcoreWl::GetSurfaceOrientation() const
{
  return 0;
}

int NativeRenderSurfaceEcoreWl::GetScreenOrientation() const
{
  return 0;
}

void NativeRenderSurfaceEcoreWl::InitializeGraphics()
{
  DALI_LOG_TRACE_METHOD(gNativeSurfaceLogFilter);

  mGraphics        = &mAdaptor->GetGraphicsInterface();
  auto eglGraphics = static_cast<Internal::Adaptor::EglGraphics*>(mGraphics);

  mEGL = &eglGraphics->GetEglInterface();

  if(mEGLContext == NULL)
  {
    // Create the OpenGL context for this window
    Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>(*mEGL);
    eglImpl.CreateWindowContext(mEGLContext);

    // Create the OpenGL surface
    CreateSurface();
  }
}

void NativeRenderSurfaceEcoreWl::CreateSurface()
{
  DALI_LOG_TRACE_METHOD(gNativeSurfaceLogFilter);

  auto                                  eglGraphics = static_cast<Internal::Adaptor::EglGraphics*>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl     = eglGraphics->GetEglImplementation();

  mEGLSurface = eglImpl.CreateSurfaceWindow(reinterpret_cast<EGLNativeWindowType>(mTbmQueue), mColorDepth);
}

void NativeRenderSurfaceEcoreWl::DestroySurface()
{
  DALI_LOG_TRACE_METHOD(gNativeSurfaceLogFilter);

  auto                                  eglGraphics = static_cast<Internal::Adaptor::EglGraphics*>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl     = eglGraphics->GetEglImplementation();

  eglImpl.DestroySurface(mEGLSurface);
}

bool NativeRenderSurfaceEcoreWl::ReplaceGraphicsSurface()
{
  DALI_LOG_TRACE_METHOD(gNativeSurfaceLogFilter);

  if(!mTbmQueue)
  {
    return false;
  }

  auto                                  eglGraphics = static_cast<Internal::Adaptor::EglGraphics*>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl     = eglGraphics->GetEglImplementation();

  return eglImpl.ReplaceSurfaceWindow(reinterpret_cast<EGLNativeWindowType>(mTbmQueue), mEGLSurface, mEGLContext);
}

void NativeRenderSurfaceEcoreWl::MoveResize(Dali::PositionSize positionSize)
{
  tbm_surface_queue_error_e error = TBM_SURFACE_QUEUE_ERROR_NONE;

  error = tbm_surface_queue_reset(mTbmQueue, positionSize.width, positionSize.height, mTbmFormat);

  if(error != TBM_SURFACE_QUEUE_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed to resize tbm_surface_queue");
  }

  mSurfaceSize.SetWidth(static_cast<uint16_t>(positionSize.width));
  mSurfaceSize.SetHeight(static_cast<uint16_t>(positionSize.height));
}

void NativeRenderSurfaceEcoreWl::StartRender()
{
}

bool NativeRenderSurfaceEcoreWl::PreRender(bool resizingSurface, const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect)
{
  if(!clippingRect.IsEmpty())
  {
    mDamagedRects.assign(damagedRects.begin(), damagedRects.end());
  }
  else
  {
    mDamagedRects.clear();
  }

  // TODO: Need to support partial update
  //  This is now done when the render pass for the render surface begins
  //   MakeContextCurrent();
  return true;
}

void NativeRenderSurfaceEcoreWl::PostRender()
{
  auto eglGraphics = static_cast<Internal::Adaptor::EglGraphics*>(mGraphics);
  if(eglGraphics)
  {
    Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();
    eglImpl.SwapBuffers(mEGLSurface, mDamagedRects);
  }

  if(mRenderNotification)
  {
    if(mThreadSynchronization)
    {
      mThreadSynchronization->PostRenderStarted();
    }

    // Tell the event-thread to render the tbm_surface
    mRenderNotification->Trigger();

    if(mThreadSynchronization)
    {
      // wait until the event-thread completed to use the tbm_surface
      mThreadSynchronization->PostRenderWaitForCompletion();
    }
  }
}

void NativeRenderSurfaceEcoreWl::StopRender()
{
  ReleaseLock();
}

void NativeRenderSurfaceEcoreWl::SetThreadSynchronization(ThreadSynchronizationInterface& threadSynchronization)
{
  mThreadSynchronization = &threadSynchronization;
}

Dali::RenderSurfaceInterface::Type NativeRenderSurfaceEcoreWl::GetSurfaceType()
{
  return Dali::RenderSurfaceInterface::NATIVE_RENDER_SURFACE;
}

void NativeRenderSurfaceEcoreWl::MakeContextCurrent()
{
  if(mEGL != nullptr)
  {
    mEGL->MakeContextCurrent(mEGLSurface, mEGLContext);
  }
}

Integration::DepthBufferAvailable NativeRenderSurfaceEcoreWl::GetDepthBufferRequired()
{
  return mGraphics ? mGraphics->GetDepthBufferRequired() : Integration::DepthBufferAvailable::FALSE;
}

Integration::StencilBufferAvailable NativeRenderSurfaceEcoreWl::GetStencilBufferRequired()
{
  return mGraphics ? mGraphics->GetStencilBufferRequired() : Integration::StencilBufferAvailable::FALSE;
}

void NativeRenderSurfaceEcoreWl::ReleaseLock()
{
  if(mThreadSynchronization)
  {
    mThreadSynchronization->PostRenderComplete();
  }
}

void NativeRenderSurfaceEcoreWl::CreateNativeRenderable()
{
  int width  = static_cast<int>(mSurfaceSize.GetWidth());
  int height = static_cast<int>(mSurfaceSize.GetHeight());

  // check we're creating one with a valid size
  DALI_ASSERT_ALWAYS(width > 0 && height > 0 && "tbm_surface size is invalid");

  mTbmQueue = tbm_surface_queue_create(3, width, height, mTbmFormat, TBM_BO_DEFAULT);

  if(mTbmQueue)
  {
    mOwnSurface = true;
  }
  else
  {
    mOwnSurface = false;
  }
}

} // namespace Dali
