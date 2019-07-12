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
#include <dali/internal/window-system/tizen-wayland/native-render-surface-ecore-wl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/debug.h>

#ifdef ECORE_WAYLAND2
#include <Ecore_Wl2.h>
#else
#include <Ecore_Wayland.h>
#endif

#include <tbm_bufmgr.h>
#include <tbm_surface_internal.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/trigger-event.h>
#include <dali/internal/graphics/gles/egl-implementation.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/window-system/common/display-connection.h>
#include <dali/internal/window-system/common/window-system.h>
#include <dali/integration-api/thread-synchronization-interface.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/adaptor-internal-services.h>

namespace Dali
{

namespace
{

#if defined(DEBUG_ENABLED)
Debug::Filter* gNativeSurfaceLogFilter = Debug::Filter::New(Debug::Verbose, false, "LOG_NATIVE_RENDER_SURFACE");
#endif

} // unnamed namespace

NativeRenderSurfaceEcoreWl::NativeRenderSurfaceEcoreWl( Dali::PositionSize positionSize, bool isTransparent )
: mPosition( positionSize ),
  mRenderNotification( NULL ),
  mGraphics( NULL ),
  mEGL( nullptr ),
  mEGLSurface( nullptr ),
  mEGLContext( nullptr ),
  mColorDepth( isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24 ),
  mTbmFormat( isTransparent ? TBM_FORMAT_ARGB8888 : TBM_FORMAT_RGB888 ),
  mOwnSurface( false ),
  mDrawableCompleted( false ),
  mTbmQueue( NULL ),
  mConsumeSurface( NULL ),
  mThreadSynchronization( NULL )
{
  Dali::Internal::Adaptor::WindowSystem::Initialize();

  CreateNativeRenderable();
  setenv( "EGL_PLATFORM", "tbm", 1 );
}

NativeRenderSurfaceEcoreWl::~NativeRenderSurfaceEcoreWl()
{
  if ( mEGLSurface )
  {
    DestroySurface();
  }

  // release the surface if we own one
  if( mOwnSurface )
  {
    ReleaseDrawable();

    if( mTbmQueue )
    {
      tbm_surface_queue_destroy( mTbmQueue );
    }

    DALI_LOG_INFO( gNativeSurfaceLogFilter, Debug::General, "Own tbm surface queue destroy\n" );
  }

  Dali::Internal::Adaptor::WindowSystem::Shutdown();
}

Any NativeRenderSurfaceEcoreWl::GetDrawable()
{
  return mConsumeSurface;
}

void NativeRenderSurfaceEcoreWl::SetRenderNotification( TriggerEventInterface* renderNotification )
{
  mRenderNotification = renderNotification;
}

void NativeRenderSurfaceEcoreWl::WaitUntilSurfaceReplaced()
{
  ConditionalWait::ScopedLock lock( mTbmSurfaceCondition );
  while( !mDrawableCompleted )
  {
    mTbmSurfaceCondition.Wait( lock );
  }

  mDrawableCompleted = false;
}

PositionSize NativeRenderSurfaceEcoreWl::GetPositionSize() const
{
  return mPosition;
}

void NativeRenderSurfaceEcoreWl::GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical )
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

  dpiHorizontal = int( xres + 0.5f );  // rounding
  dpiVertical   = int( yres + 0.5f );
}

void NativeRenderSurfaceEcoreWl::InitializeGraphics()
{
  DALI_LOG_TRACE_METHOD( gNativeSurfaceLogFilter );
  unsetenv( "EGL_PLATFORM" );

  mGraphics = &mAdaptor->GetGraphicsInterface();
  auto eglGraphics = static_cast<Internal::Adaptor::EglGraphics *>(mGraphics);

  mEGL = &eglGraphics->GetEglInterface();

  if ( mEGLContext == NULL )
  {
    // Create the OpenGL context for this window
    Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>(*mEGL);
    eglImpl.CreateWindowContext( mEGLContext );

    // Create the OpenGL surface
    CreateSurface();
  }
}

void NativeRenderSurfaceEcoreWl::CreateSurface()
{
  DALI_LOG_TRACE_METHOD( gNativeSurfaceLogFilter );

  auto eglGraphics = static_cast<Internal::Adaptor::EglGraphics *>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  mEGLSurface = eglImpl.CreateSurfaceWindow( reinterpret_cast< EGLNativeWindowType >( mTbmQueue ), mColorDepth );
}

void NativeRenderSurfaceEcoreWl::DestroySurface()
{
  DALI_LOG_TRACE_METHOD( gNativeSurfaceLogFilter );

  auto eglGraphics = static_cast<Internal::Adaptor::EglGraphics *>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  eglImpl.DestroySurface( mEGLSurface );
}

bool NativeRenderSurfaceEcoreWl::ReplaceGraphicsSurface()
{
  DALI_LOG_TRACE_METHOD( gNativeSurfaceLogFilter );

  if( !mTbmQueue )
  {
    return false;
  }

  auto eglGraphics = static_cast<Internal::Adaptor::EglGraphics *>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  return eglImpl.ReplaceSurfaceWindow( reinterpret_cast< EGLNativeWindowType >( mTbmQueue ), mEGLSurface, mEGLContext );
}

void NativeRenderSurfaceEcoreWl::MoveResize( Dali::PositionSize positionSize )
{
  tbm_surface_queue_error_e error = TBM_SURFACE_QUEUE_ERROR_NONE;

  error = tbm_surface_queue_reset( mTbmQueue, positionSize.width, positionSize.height, mTbmFormat );

  if( error != TBM_SURFACE_QUEUE_ERROR_NONE )
  {
    DALI_LOG_ERROR( "Failed to resize tbm_surface_queue" );
  }

  mPosition = positionSize;
}

void NativeRenderSurfaceEcoreWl::StartRender()
{
}

bool NativeRenderSurfaceEcoreWl::PreRender( bool )
{
  // nothing to do for pixmaps
  return true;
}

void NativeRenderSurfaceEcoreWl::PostRender( bool renderToFbo, bool replacingSurface, bool resizingSurface )
{
  auto eglGraphics = static_cast<Internal::Adaptor::EglGraphics *>(mGraphics);
  if ( eglGraphics )
  {
    Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

    eglImpl.SwapBuffers( mEGLSurface );
  }

  if( mThreadSynchronization )
  {
    mThreadSynchronization->PostRenderStarted();
  }

  if( tbm_surface_queue_can_acquire( mTbmQueue, 1 ) )
  {
    if( tbm_surface_queue_acquire( mTbmQueue, &mConsumeSurface ) != TBM_SURFACE_QUEUE_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Failed to acquire a tbm_surface\n" );
      return;
    }
  }

  tbm_surface_internal_ref( mConsumeSurface );

  if( replacingSurface )
  {
    ConditionalWait::ScopedLock lock( mTbmSurfaceCondition );
    mDrawableCompleted = true;
    mTbmSurfaceCondition.Notify( lock );
  }

 // create damage for client applications which wish to know the update timing
  if( !replacingSurface && mRenderNotification )
  {
    // use notification trigger
    // Tell the event-thread to render the tbm_surface
    mRenderNotification->Trigger();
  }

  if( mThreadSynchronization )
  {
    // wait until the event-thread completed to use the tbm_surface
    mThreadSynchronization->PostRenderWaitForCompletion();
  }

  // release the consumed surface after post render was completed
  ReleaseDrawable();
}

void NativeRenderSurfaceEcoreWl::StopRender()
{
  ReleaseLock();
}

void NativeRenderSurfaceEcoreWl::SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization )
{
  mThreadSynchronization = &threadSynchronization;
}

Integration::RenderSurface::Type NativeRenderSurfaceEcoreWl::GetSurfaceType()
{
  return Integration::RenderSurface::NATIVE_RENDER_SURFACE;
}

void NativeRenderSurfaceEcoreWl::MakeContextCurrent()
{
  if ( mEGL != nullptr )
  {
    mEGL->MakeContextCurrent( mEGLSurface, mEGLContext );
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
  if( mThreadSynchronization )
  {
    mThreadSynchronization->PostRenderComplete();
  }
}

void NativeRenderSurfaceEcoreWl::CreateNativeRenderable()
{
  // check we're creating one with a valid size
  DALI_ASSERT_ALWAYS( mPosition.width > 0 && mPosition.height > 0 && "tbm_surface size is invalid" );

  mTbmQueue = tbm_surface_queue_create( 3, mPosition.width, mPosition.height, mTbmFormat, TBM_BO_DEFAULT );

  if( mTbmQueue )
  {
    mOwnSurface = true;
  }
  else
  {
    mOwnSurface = false;
  }
}

void NativeRenderSurfaceEcoreWl::ReleaseDrawable()
{
  if( mConsumeSurface )
  {
    tbm_surface_internal_unref( mConsumeSurface );

    if( tbm_surface_internal_is_valid( mConsumeSurface ) )
    {
      tbm_surface_queue_release( mTbmQueue, mConsumeSurface );
    }
    mConsumeSurface = NULL;
  }
}

} // namespace Dali
