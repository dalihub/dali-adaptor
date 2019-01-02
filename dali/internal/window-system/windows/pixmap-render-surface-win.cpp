/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/windows/pixmap-render-surface-win.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/debug.h>
#include <dali/devel-api/threading/mutex.h>

// INTERNAL INCLUDES
#include <dali/integration-api/thread-synchronization-interface.h>
#include <dali/internal/system/common/trigger-event.h>
#include <dali/internal/window-system/common/display-connection.h>
#include <dali/internal/window-system/windows/platform-implement-win.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

#if defined(DEBUG_ENABLED)
Debug::Filter* gPixmapRenderSurfaceLogFilter = Debug::Filter::New(Debug::Verbose, false, "LOG_PIXMAP_RENDER_SURFACE_WIN");
#endif

namespace
{
static const int INITIAL_PRODUCE_BUFFER_INDEX = 0;
static const int INITIAL_CONSUME_BUFFER_INDEX = 1;
}

PixmapRenderSurfaceEcoreWin::PixmapRenderSurfaceEcoreWin( Dali::PositionSize positionSize, Any surface, bool isTransparent )
: mPosition( positionSize ),
  mRenderNotification( NULL ),
  mColorDepth( isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24 ),
  mOwnSurface( false ),
  mProduceBufferIndex( INITIAL_PRODUCE_BUFFER_INDEX ),
  mConsumeBufferIndex( INITIAL_CONSUME_BUFFER_INDEX ),
  mThreadSynchronization(NULL)
{
  for( int i = 0; i != BUFFER_COUNT; ++i )
  {
    mWinPixmaps[i] = 0;
    mEglSurfaces[i] = 0;
  }

  Initialize( surface );
}

PixmapRenderSurfaceEcoreWin::~PixmapRenderSurfaceEcoreWin()
{
  // release the surface if we own one
  //if( mOwnSurface )
  //{
  //  for (int i = 0; i < BUFFER_COUNT; ++i)
  //  {
  //    Ecore_X_Pixmap pixmap = mWinPixmaps[i];

  //    // if we did create the pixmap, delete the pixmap
  //    DALI_LOG_INFO( gPixmapRenderSurfaceLogFilter, Debug::General, "Own pixmap (%x) freed\n", pixmap );
  //    ecore_x_pixmap_free( pixmap );
  //  }
  //}
}

void PixmapRenderSurfaceEcoreWin::Initialize( Any surface )
{
  // see if there is a surface in Any surface
  unsigned int surfaceId  = GetSurfaceId( surface );

  // if the surface is empty, create a new one.
  if ( surfaceId == 0 )
  {
    // we own the surface about to created
    mOwnSurface = true;
    CreateRenderable();
  }
  else
  {
    // XLib should already be initialized so no point in calling XInitThreads
    UseExistingRenderable( surfaceId );
  }
}

Any PixmapRenderSurfaceEcoreWin::GetSurface()
{
  //Ecore_X_Pixmap pixmap = 0;
  //{
  //  ConditionalWait::ScopedLock lock( mPixmapCondition );
  //  pixmap = mWinPixmaps[mProduceBufferIndex];
  //}

  return Any( 0 );
}

void PixmapRenderSurfaceEcoreWin::SetRenderNotification(TriggerEventInterface* renderNotification)
{
  mRenderNotification = renderNotification;
}

PositionSize PixmapRenderSurfaceEcoreWin::GetPositionSize() const
{
  return mPosition;
}

void PixmapRenderSurfaceEcoreWin::GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical )
{
  // calculate DPI 
  float xres, yres;

  //// 1 inch = 25.4 millimeters
  WindowsPlatformImplement::GetDPI( xres, yres );

  xres *= 1.5;
  yres *= 1.5;

  dpiHorizontal = int( xres + 0.5f );  // rounding
  dpiVertical = int( yres + 0.5f );
}

void PixmapRenderSurfaceEcoreWin::InitializeEgl( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gPixmapRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );

  eglImpl.ChooseConfig(false, mColorDepth);
}

void PixmapRenderSurfaceEcoreWin::CreateEglSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gPixmapRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    // create the EGL surface
    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    WinPixmap pixmap = static_cast<WinPixmap>( mWinPixmaps[i] );
    mEglSurfaces[i] = eglImpl.CreateSurfacePixmap( EGLNativePixmapType( pixmap ), mColorDepth ); // reinterpret_cast does not compile
  }
}

void PixmapRenderSurfaceEcoreWin::DestroyEglSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gPixmapRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    WinPixmap pixmap = static_cast<WinPixmap>( mWinPixmaps[i] );
    eglImpl.MakeCurrent( EGLNativePixmapType( pixmap ), mEglSurfaces[i] );
    eglImpl.DestroySurface();
  }
}

bool PixmapRenderSurfaceEcoreWin::ReplaceEGLSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gPixmapRenderSurfaceLogFilter );

  bool contextLost = false;

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    // a new surface for the new pixmap
    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    WinPixmap pixmap = static_cast<WinPixmap>( mWinPixmaps[i] );
    contextLost = eglImpl.ReplaceSurfacePixmap( EGLNativePixmapType( pixmap ), mEglSurfaces[i] ); // reinterpret_cast does not compile
  }

  // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
  WinPixmap pixmap = static_cast<WinPixmap>( mWinPixmaps[mProduceBufferIndex] );
  eglImpl.MakeCurrent( EGLNativePixmapType( pixmap ), mEglSurfaces[mProduceBufferIndex] );

  return contextLost;
}

void PixmapRenderSurfaceEcoreWin::StartRender()
{
}

bool PixmapRenderSurfaceEcoreWin::PreRender( EglInterface& egl, Integration::GlAbstraction&, bool )
{
  // Nothing to do for pixmaps
  return true;
}

void PixmapRenderSurfaceEcoreWin::PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, Dali::DisplayConnection* displayConnection, bool replacingSurface, bool resizingSurface )
{
}

void PixmapRenderSurfaceEcoreWin::StopRender()
{
  ReleaseLock();
}

void PixmapRenderSurfaceEcoreWin::SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization )
{
  mThreadSynchronization = &threadSynchronization;
}

void PixmapRenderSurfaceEcoreWin::ReleaseLock()
{
  if( mThreadSynchronization )
  {
    mThreadSynchronization->PostRenderComplete();
  }
}

RenderSurface::Type PixmapRenderSurfaceEcoreWin::GetSurfaceType()
{
  return RenderSurface::PIXMAP_RENDER_SURFACE;
}

void PixmapRenderSurfaceEcoreWin::CreateRenderable()
{
}

void PixmapRenderSurfaceEcoreWin::UseExistingRenderable( unsigned int surfaceId )
{
}

unsigned int PixmapRenderSurfaceEcoreWin::GetSurfaceId( Any surface ) const
{
  return 0;
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
