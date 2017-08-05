/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <native-render-surface.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/debug.h>
#include <dali/devel-api/threading/conditional-wait.h>

#include <Ecore_Wayland.h>
#include <tbm_bufmgr.h>
#include <tbm_surface_queue.h>
#include <tbm_surface_internal.h>

// INTERNAL INCLUDES
#include <trigger-event.h>
#include <gl/egl-implementation.h>
#include <base/display-connection.h>
#include <integration-api/thread-synchronization-interface.h>

namespace Dali
{

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gRenderSurfaceLogFilter;
#endif

struct NativeRenderSurface::Impl
{
  Impl( Dali::PositionSize positionSize, const std::string& name, bool isTransparent )
  : mPosition( positionSize ),
    mTitle( name ),
    mRenderNotification( NULL ),
    mColorDepth( isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24 ),
    mTbmFormat( isTransparent ? TBM_FORMAT_ARGB8888 : TBM_FORMAT_RGB888 ),
    mOwnSurface( false ),
    mDrawableCompleted( false ),
    mTbmQueue( NULL ),
    mConsumeSurface( NULL ),
    mThreadSynchronization( NULL )
  {
  }

  PositionSize mPosition;
  std::string mTitle;
  TriggerEventInterface* mRenderNotification;
  ColorDepth mColorDepth;
  tbm_format mTbmFormat;
  bool mOwnSurface;
  bool mDrawableCompleted;

  tbm_surface_queue_h mTbmQueue;
  tbm_surface_h mConsumeSurface;
  ThreadSynchronizationInterface* mThreadSynchronization;     ///< A pointer to the thread-synchronization
  ConditionalWait mTbmSurfaceCondition;
};

NativeRenderSurface::NativeRenderSurface(Dali::PositionSize positionSize,
                                         const std::string& name,
                                         bool isTransparent)
: mImpl( new Impl( positionSize, name, isTransparent ) )
{
  ecore_wl_init(NULL);
  CreateNativeRenderable();
  setenv( "EGL_PLATFORM", "tbm", 1 );
}

NativeRenderSurface::~NativeRenderSurface()
{
  // release the surface if we own one
  if( mImpl->mOwnSurface )
  {
    ReleaseDrawable();

    if( mImpl->mTbmQueue )
    {
      tbm_surface_queue_destroy( mImpl->mTbmQueue );
    }

    delete mImpl;

    DALI_LOG_INFO( gRenderSurfaceLogFilter, Debug::General, "Own tbm surface queue destroy\n" );
  }
}

void NativeRenderSurface::SetRenderNotification( TriggerEventInterface* renderNotification )
{
  mImpl->mRenderNotification = renderNotification;
}

tbm_surface_h NativeRenderSurface::GetDrawable()
{
  return mImpl->mConsumeSurface;
}

Any NativeRenderSurface::GetSurface()
{
  return Any( NULL );
}

void NativeRenderSurface::InitializeEgl( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );
  unsetenv( "EGL_PLATFORM" );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );

  eglImpl.ChooseConfig( true, mImpl->mColorDepth );
}

void NativeRenderSurface::CreateEglSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );

  eglImpl.CreateSurfaceWindow( (EGLNativeWindowType)mImpl->mTbmQueue, mImpl->mColorDepth );
}

void NativeRenderSurface::DestroyEglSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );
  eglImpl.DestroySurface();
}

bool NativeRenderSurface::ReplaceEGLSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  if( !mImpl->mTbmQueue )
  {
    return false;
  }

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );

  return eglImpl.ReplaceSurfaceWindow( (EGLNativeWindowType)mImpl->mTbmQueue ); // reinterpret_cast does not compile
}

void NativeRenderSurface::StartRender()
{
}

bool NativeRenderSurface::PreRender( EglInterface&, Integration::GlAbstraction&, bool )
{
  // nothing to do for pixmaps
  return true;
}

void NativeRenderSurface::PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, DisplayConnection* displayConnection, bool replacingSurface, bool resizingSurface )
{
  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );
  eglImpl.SwapBuffers();

  if( mImpl->mThreadSynchronization )
  {
    mImpl->mThreadSynchronization->PostRenderStarted();
  }

  if( tbm_surface_queue_can_acquire( mImpl->mTbmQueue, 1 ) )
  {
    if( tbm_surface_queue_acquire( mImpl->mTbmQueue, &mImpl->mConsumeSurface ) != TBM_SURFACE_QUEUE_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Failed to aquire a tbm_surface\n" );
      return;
    }
  }

  tbm_surface_internal_ref( mImpl->mConsumeSurface );

  if( replacingSurface )
  {
    ConditionalWait::ScopedLock lock( mImpl->mTbmSurfaceCondition );
    mImpl->mDrawableCompleted = true;
    mImpl->mTbmSurfaceCondition.Notify( lock );
  }

 // create damage for client applications which wish to know the update timing
  if( !replacingSurface && mImpl->mRenderNotification )
  {
    // use notification trigger
    // Tell the event-thread to render the tbm_surface
    mImpl->mRenderNotification->Trigger();
  }

  if( mImpl->mThreadSynchronization )
  {
    // wait until the event-thread completed to use the tbm_surface
    mImpl->mThreadSynchronization->PostRenderWaitForCompletion();
  }

  // release the consumed surface after post render was completed
  ReleaseDrawable();
}

void NativeRenderSurface::StopRender()
{
  ReleaseLock();
}

PositionSize NativeRenderSurface::GetPositionSize() const
{
  return mImpl->mPosition;
}

void NativeRenderSurface::MoveResize( Dali::PositionSize positionSize )
{
}

void NativeRenderSurface::SetViewMode( ViewMode viewMode )
{
}

void NativeRenderSurface::SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization )
{
  mImpl->mThreadSynchronization = &threadSynchronization;
}

RenderSurface::Type NativeRenderSurface::GetSurfaceType()
{
  return RenderSurface::NATIVE_RENDER_SURFACE;
}

void NativeRenderSurface::CreateNativeRenderable()
{
  // check we're creating one with a valid size
  DALI_ASSERT_ALWAYS( mImpl->mPosition.width > 0 && mImpl->mPosition.height > 0 && "tbm_surface size is invalid" );

  mImpl->mTbmQueue = tbm_surface_queue_create( 3, mImpl->mPosition.width, mImpl->mPosition.height, mImpl->mTbmFormat, TBM_BO_DEFAULT );

  if( mImpl->mTbmQueue )
  {
    mImpl->mOwnSurface = true;
  }
  else
  {
    mImpl->mOwnSurface = false;
  }
}

void NativeRenderSurface::ReleaseLock()
{
  if( mImpl->mThreadSynchronization )
  {
    mImpl->mThreadSynchronization->PostRenderComplete();
  }
}

void NativeRenderSurface::WaitUntilSurfaceReplaced()
{
  ConditionalWait::ScopedLock lock( mImpl->mTbmSurfaceCondition );
  while( !mImpl->mDrawableCompleted )
  {
    mImpl->mTbmSurfaceCondition.Wait( lock );
  }

  mImpl->mDrawableCompleted = false;
}

void NativeRenderSurface::ReleaseDrawable()
{
  if( mImpl->mConsumeSurface )
  {
    tbm_surface_internal_unref( mImpl->mConsumeSurface );

    if( tbm_surface_internal_is_valid( mImpl->mConsumeSurface ) )
    {
      tbm_surface_queue_release( mImpl->mTbmQueue, mImpl->mConsumeSurface );
    }
    mImpl->mConsumeSurface = NULL;
  }
}

} // namespace Dali
