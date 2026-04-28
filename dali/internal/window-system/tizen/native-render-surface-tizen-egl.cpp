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
#include <dali/internal/window-system/tizen/native-render-surface-tizen-egl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-abstraction.h>

#include <tbm_bufmgr.h>
#include <tbm_surface_internal.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/thread-synchronization-interface.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/adaptor-internal-services.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/graphics/gles/egl-implementation.h>
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
  NativeRenderSurfaceTizen* surface = static_cast<NativeRenderSurfaceTizen*>(data);
  if(surface)
  {
    surface->TriggerFrameRenderedCallback();
  }
}

inline bool IsColorDepth32Required(const tbm_format format)
{
  switch(format)
  {
    case TBM_FORMAT_ARGB8888:
    case TBM_FORMAT_ABGR8888:
    case TBM_FORMAT_RGBA8888:
    case TBM_FORMAT_BGRA8888:
    case TBM_FORMAT_XRGB8888:
    case TBM_FORMAT_XBGR8888:
    case TBM_FORMAT_RGBX8888:
    case TBM_FORMAT_BGRX8888:
    case TBM_FORMAT_XRGB2101010:
    case TBM_FORMAT_XBGR2101010:
    case TBM_FORMAT_RGBX1010102:
    case TBM_FORMAT_BGRX1010102:
    case TBM_FORMAT_ARGB2101010:
    case TBM_FORMAT_ABGR2101010:
    case TBM_FORMAT_RGBA1010102:
    case TBM_FORMAT_BGRA1010102:
    {
      return true;
    }
    default:
    {
      return false;
    }
  }
}

} // unnamed namespace

NativeRenderSurfaceTizen::NativeRenderSurfaceTizen(SurfaceSize surfaceSize, Any surface, bool isTransparent)
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

    mColorDepth = IsColorDepth32Required(mTbmFormat) ? COLOR_DEPTH_32 : COLOR_DEPTH_24;
  }
}

NativeRenderSurfaceTizen::~NativeRenderSurfaceTizen()
{
  if(mEGLSurface)
  {
    DestroySurface();
  }

  if(mEGLContext)
  {
    DestroyContext();
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

void NativeRenderSurfaceTizen::SetRenderNotification(TriggerEventInterface* renderNotification)
{
  mRenderNotification = renderNotification;
}

Any NativeRenderSurfaceTizen::GetNativeRenderable()
{
  return mTbmQueue;
}

void NativeRenderSurfaceTizen::TriggerFrameRenderedCallback()
{
  if(mFrameRenderedCallback)
  {
    mFrameRenderedCallback->Trigger();
  }
}

void NativeRenderSurfaceTizen::SetFrameRenderedCallback(CallbackBase* callback)
{
  mFrameRenderedCallback = std::unique_ptr<EventThreadCallback>(new EventThreadCallback(callback));
  DALI_LOG_DEBUG_INFO("SetFrameRenderedCallback Trigger Id(%d)\n", mFrameRenderedCallback->GetId());

  tbm_surface_queue_error_e result = tbm_surface_queue_add_acquirable_cb(mTbmQueue, TbmAcquirableCallback, this);
  if(result != TBM_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed calling tbm_surface_queue_add_acquirable_cb(), error : %x", result);
  }
}

PositionSize NativeRenderSurfaceTizen::GetPositionSize() const
{
  return PositionSize(0, 0, static_cast<int>(mSurfaceSize.GetWidth()), static_cast<int>(mSurfaceSize.GetHeight()));
}

void NativeRenderSurfaceTizen::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  // calculate DPI
  float xres, yres;

  // 1 inch = 25.4 millimeters
  xres = 96;
  yres = 96;

  dpiHorizontal = int(xres + 0.5f); // rounding
  dpiVertical   = int(yres + 0.5f);
}

int NativeRenderSurfaceTizen::GetSurfaceOrientation() const
{
  return 0;
}

int NativeRenderSurfaceTizen::GetScreenOrientation() const
{
  return 0;
}

void NativeRenderSurfaceTizen::InitializeGraphics()
{
  DALI_LOG_TRACE_METHOD(gNativeSurfaceLogFilter);

  mGraphics        = &mAdaptor->GetGraphicsInterface();
  auto eglGraphics = static_cast<Internal::Adaptor::EglGraphics*>(mGraphics);

  mEGL = &eglGraphics->GetEglInterface();

  if(mEGLContext == NULL)
  {
    CreateContext();
  }

  // Create the OpenGL surface
  if(mEGLSurface == NULL)
  {
    CreateSurface();
  }
}

void NativeRenderSurfaceTizen::CreateSurface()
{
  DALI_LOG_TRACE_METHOD(gNativeSurfaceLogFilter);

  auto                                  eglGraphics = static_cast<Internal::Adaptor::EglGraphics*>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl     = eglGraphics->GetEglImplementation();

  mEGLSurface = eglImpl.CreateSurfaceWindow(reinterpret_cast<EGLNativeWindowType>(mTbmQueue), mColorDepth);
  DALI_LOG_RELEASE_INFO("NativeRenderSurfaceTizen::CreateSurface mTbmQueue(%p), mOwnSurface(%d), create surface: %p\n", mTbmQueue, mOwnSurface, mEGLSurface);
}

void NativeRenderSurfaceTizen::DestroySurface()
{
  DALI_LOG_TRACE_METHOD(gNativeSurfaceLogFilter);

  auto                                  eglGraphics = static_cast<Internal::Adaptor::EglGraphics*>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl     = eglGraphics->GetEglImplementation();

  DALI_LOG_RELEASE_INFO("NativeRenderSurfaceTizen::DestroySurface mTbmQueue(%p), mOwnSurface(%d), surface: %p\n", mTbmQueue, mOwnSurface, mEGLSurface);
  eglImpl.DestroySurface(mEGLSurface);
  mEGLSurface = NULL;

  // TODO : We'd better call this API for more clear way.
  if(mEGLContext != NULL)
  {
    DestroyContext();
  }
}

void NativeRenderSurfaceTizen::CreateContext()
{
  DALI_LOG_TRACE_METHOD(gNativeSurfaceLogFilter);

  auto                                  eglGraphics = static_cast<Internal::Adaptor::EglGraphics*>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl     = eglGraphics->GetEglImplementation();

  eglImpl.CreateWindowContext(mEGLContext);
  DALI_LOG_RELEASE_INFO("NativeRenderSurfaceTizen::CreateContext mTbmQueue(%p), mOwnSurface(%d), create context: %p\n", mTbmQueue, mOwnSurface, mEGLContext);
}

void NativeRenderSurfaceTizen::DestroyContext()
{
  DALI_LOG_TRACE_METHOD(gNativeSurfaceLogFilter);

  auto                                  eglGraphics = static_cast<Internal::Adaptor::EglGraphics*>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl     = eglGraphics->GetEglImplementation();

  DALI_LOG_RELEASE_INFO("NativeRenderSurfaceTizen::DestroyContext mTbmQueue(%p), mOwnSurface(%d), destroy context: %p\n", mTbmQueue, mOwnSurface, mEGLContext);
  eglImpl.DestroyContext(mEGLContext);
  mEGLContext = NULL;
}

bool NativeRenderSurfaceTizen::ReplaceGraphicsSurface()
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

void NativeRenderSurfaceTizen::MoveResize(Dali::PositionSize positionSize)
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

void NativeRenderSurfaceTizen::Resize(Dali::Uint16Pair size)
{
  MoveResize(PositionSize(0, 0, size.GetWidth(), size.GetHeight()));
}

void NativeRenderSurfaceTizen::StartRender()
{
}

bool NativeRenderSurfaceTizen::PreRender(bool resizingSurface, const std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect)
{
  // Not support partial update
  clippingRect = Rect<int32_t>(0, 0, mSurfaceSize.GetWidth(), mSurfaceSize.GetHeight());

  // Discard old surface if we cannot enqueue to tbm buffer.
  // If we don't acquire & release any buffer, it will be dead lock when we call
  // any glClear, or glFlush, glDraw, etc.
  if(DALI_UNLIKELY(mTbmQueue && !tbm_surface_queue_can_dequeue(mTbmQueue, 0)))
  {
    if(tbm_surface_queue_can_acquire(mTbmQueue, 0))
    {
      tbm_surface_h surface;

      auto ret = tbm_surface_queue_acquire(mTbmQueue, &surface);

      if(ret != TBM_SURFACE_QUEUE_ERROR_NONE)
      {
        DALI_LOG_ERROR("Failed to aquire a tbm_surface. error : 0x%x. Deadlock might be occured!!\n", ret);
      }
      else
      {
        if(tbm_surface_internal_is_valid(surface))
        {
          ret = tbm_surface_queue_release(mTbmQueue, surface);
          if(ret != TBM_SURFACE_QUEUE_ERROR_NONE)
          {
            DALI_LOG_ERROR("Failed to release a tbm_surface[%p]. error : 0x%x. Deadlock might be occured!!\n", surface, ret);
          }
        }
        else
        {
          DALI_LOG_ERROR("tbm_surface[%p] is not valid!. Deadlock might be occured!!\n", surface);
        }
      }
    }
  }
  return true;
}

void NativeRenderSurfaceTizen::PostRender()
{
  auto eglGraphics = static_cast<Internal::Adaptor::EglGraphics*>(mGraphics);
  if(eglGraphics)
  {
    Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();
    eglImpl.SwapBuffers(mEGLSurface);
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

void NativeRenderSurfaceTizen::StopRender()
{
  ReleaseLock();
}

void NativeRenderSurfaceTizen::SetThreadSynchronization(ThreadSynchronizationInterface& threadSynchronization)
{
  mThreadSynchronization = &threadSynchronization;
}

Dali::Integration::RenderSurfaceInterface::Type NativeRenderSurfaceTizen::GetSurfaceType()
{
  return Dali::Integration::RenderSurfaceInterface::NATIVE_RENDER_SURFACE;
}

void NativeRenderSurfaceTizen::MakeContextCurrent()
{
  if(mEGL != nullptr)
  {
    mEGL->MakeContextCurrent(mEGLSurface, mEGLContext);
  }
}

void NativeRenderSurfaceTizen::ReleaseLock()
{
  if(mThreadSynchronization)
  {
    mThreadSynchronization->PostRenderComplete();
  }
}

void NativeRenderSurfaceTizen::CreateNativeRenderable()
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
