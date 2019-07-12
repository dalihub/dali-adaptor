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
#include <dali/internal/window-system/ubuntu-x11/pixmap-render-surface-ecore-x.h>

// EXTERNAL INCLUDES
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/extensions/Xfixes.h> // for damage notify
#include <X11/extensions/Xdamage.h> // for damage notify

#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/debug.h>
#include <dali/devel-api/threading/mutex.h>

// INTERNAL INCLUDES
#include <dali/integration-api/thread-synchronization-interface.h>
#include <dali/internal/system/common/trigger-event.h>
#include <dali/internal/window-system/common/display-connection.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/adaptor-internal-services.h>


namespace Dali
{
namespace Internal
{
namespace Adaptor
{

#if defined(DEBUG_ENABLED)
Debug::Filter* gPixmapRenderSurfaceLogFilter = Debug::Filter::New(Debug::Verbose, false, "LOG_PIXMAP_RENDER_SURFACE_ECORE_X");
#endif

namespace
{
static const int INITIAL_PRODUCE_BUFFER_INDEX = 0;
static const int INITIAL_CONSUME_BUFFER_INDEX = 1;
}

PixmapRenderSurfaceEcoreX::PixmapRenderSurfaceEcoreX( Dali::PositionSize positionSize, Any surface, bool isTransparent )
: mGraphics( nullptr ),
  mDisplayConnection( nullptr ),
  mPosition( positionSize ),
  mRenderNotification( NULL ),
  mColorDepth( isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24 ),
  mOwnSurface( false ),
  mProduceBufferIndex( INITIAL_PRODUCE_BUFFER_INDEX ),
  mConsumeBufferIndex( INITIAL_CONSUME_BUFFER_INDEX ),
  mX11Pixmaps(),
  mEglSurfaces(),
  mThreadSynchronization( nullptr ),
  mPixmapCondition()
{
  for( int i = 0; i != BUFFER_COUNT; ++i )
  {
    mX11Pixmaps[i] = 0;
    mEglSurfaces[i] = 0;
  }

  Initialize( surface );
}

PixmapRenderSurfaceEcoreX::~PixmapRenderSurfaceEcoreX()
{
  DestroySurface();

  // release the surface if we own one
  if( mOwnSurface )
  {
    for (int i = 0; i < BUFFER_COUNT; ++i)
    {
      Ecore_X_Pixmap pixmap = mX11Pixmaps[i];

      // if we did create the pixmap, delete the pixmap
      DALI_LOG_INFO( gPixmapRenderSurfaceLogFilter, Debug::General, "Own pixmap (%x) freed\n", pixmap );
      ecore_x_pixmap_free( pixmap );
    }
  }
}

void PixmapRenderSurfaceEcoreX::Initialize( Any surface )
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

Any PixmapRenderSurfaceEcoreX::GetSurface()
{
  Ecore_X_Pixmap pixmap = 0;
  {
    ConditionalWait::ScopedLock lock( mPixmapCondition );
    pixmap = mX11Pixmaps[mProduceBufferIndex];
  }

  return Any( pixmap );
}

void PixmapRenderSurfaceEcoreX::SetRenderNotification(TriggerEventInterface* renderNotification)
{
  mRenderNotification = renderNotification;
}

PositionSize PixmapRenderSurfaceEcoreX::GetPositionSize() const
{
  return mPosition;
}

void PixmapRenderSurfaceEcoreX::GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical )
{
  // calculate DPI
  float xres, yres;

  // 1 inch = 25.4 millimeters
  xres = ecore_x_dpi_get();
  yres = ecore_x_dpi_get();

  dpiHorizontal = int( xres + 0.5f );  // rounding
  dpiVertical   = int( yres + 0.5f );
}

void PixmapRenderSurfaceEcoreX::InitializeGraphics()
{
  mGraphics = &mAdaptor->GetGraphicsInterface();
  mDisplayConnection = &mAdaptor->GetDisplayConnectionInterface();


  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();
  eglImpl.ChooseConfig(false, mColorDepth);
}

void PixmapRenderSurfaceEcoreX::CreateSurface()
{
  DALI_LOG_TRACE_METHOD( gPixmapRenderSurfaceLogFilter );

  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    // create the EGL surface
    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    XPixmap pixmap = static_cast<XPixmap>( mX11Pixmaps[i] );
    mEglSurfaces[i] = eglImpl.CreateSurfacePixmap( EGLNativePixmapType( pixmap ), mColorDepth ); // reinterpret_cast does not compile
  }
}

void PixmapRenderSurfaceEcoreX::DestroySurface()
{
  DALI_LOG_TRACE_METHOD( gPixmapRenderSurfaceLogFilter );

  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);

  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    XPixmap pixmap = static_cast<XPixmap>( mX11Pixmaps[i] );
    eglImpl.MakeCurrent( EGLNativePixmapType( pixmap ), mEglSurfaces[i] );
    eglImpl.DestroySurface( mEglSurfaces[i] );
  }
}

bool PixmapRenderSurfaceEcoreX::ReplaceGraphicsSurface()
{
  DALI_LOG_TRACE_METHOD( gPixmapRenderSurfaceLogFilter );

  bool contextLost = false;

  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);

  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    // a new surface for the new pixmap
    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    XPixmap pixmap = static_cast<XPixmap>( mX11Pixmaps[i] );
    contextLost = eglImpl.ReplaceSurfacePixmap( EGLNativePixmapType( pixmap ), mEglSurfaces[i] ); // reinterpret_cast does not compile
  }

  // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
  XPixmap pixmap = static_cast<XPixmap>( mX11Pixmaps[mProduceBufferIndex] );
  eglImpl.MakeCurrent( EGLNativePixmapType( pixmap ), mEglSurfaces[mProduceBufferIndex] );

  return contextLost;
}

void PixmapRenderSurfaceEcoreX::StartRender()
{
}

bool PixmapRenderSurfaceEcoreX::PreRender( bool )
{
  // Nothing to do for pixmaps
  return true;
}

void PixmapRenderSurfaceEcoreX::PostRender( bool renderToFbo, bool replacingSurface, bool resizingSurface )
{
  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);

  // flush gl instruction queue
  Integration::GlAbstraction& glAbstraction = eglGraphics->GetGlAbstraction();
  glAbstraction.Flush();

  if( mThreadSynchronization )
  {
    mThreadSynchronization->PostRenderStarted();
  }

  {
    ConditionalWait::ScopedLock lock( mPixmapCondition );
    mConsumeBufferIndex = __sync_fetch_and_xor( &mProduceBufferIndex, 1 ); // Swap buffer indexes.

    Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    XPixmap pixmap = static_cast<XPixmap>( mX11Pixmaps[mProduceBufferIndex] );
    eglImpl.MakeCurrent( EGLNativePixmapType( pixmap ), mEglSurfaces[mProduceBufferIndex] );
  }

  // create damage for client applications which wish to know the update timing
  if( mRenderNotification )
  {
    // use notification trigger
    // Tell the event-thread to render the pixmap
    mRenderNotification->Trigger();
  }
  else
  {
    // as a fallback, send damage event.
    Ecore_X_Drawable drawable = Ecore_X_Drawable( mX11Pixmaps[mProduceBufferIndex] );

    if( drawable )
    {
      XRectangle rect;
      XserverRegion region;

      rect.x = 0;
      rect.y = 0;
      rect.width = mPosition.width;
      rect.height = mPosition.height;

      XDisplay* display = AnyCast<XDisplay*>(mDisplayConnection->GetDisplay());

      // make a fixes region as updated area
      region = XFixesCreateRegion( display, &rect, 1 );
      // add damage event to updated drawable
      Drawable xdrawable( drawable ); // ecore type is unsigned int whereas in 64bit linux Drawable is long unsigned int
      XDamageAdd( display, xdrawable, region );
      XFixesDestroyRegion( display, region );

      XFlush( display );
    }
  }

  if( mThreadSynchronization )
  {
    mThreadSynchronization->PostRenderWaitForCompletion();
  }
}

void PixmapRenderSurfaceEcoreX::StopRender()
{
  ReleaseLock();
}

void PixmapRenderSurfaceEcoreX::SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization )
{
  mThreadSynchronization = &threadSynchronization;
}

void PixmapRenderSurfaceEcoreX::ReleaseLock()
{
  if( mThreadSynchronization )
  {
    mThreadSynchronization->PostRenderComplete();
  }
}

Integration::RenderSurface::Type PixmapRenderSurfaceEcoreX::GetSurfaceType()
{
  return Integration::RenderSurface::PIXMAP_RENDER_SURFACE;
}

void PixmapRenderSurfaceEcoreX::MakeContextCurrent()
{
}

void PixmapRenderSurfaceEcoreX::CreateRenderable()
{
  // check we're creating one with a valid size
  DALI_ASSERT_ALWAYS( mPosition.width > 0 && mPosition.height > 0 && "Pixmap size is invalid" );

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    // create the pixmap
    mX11Pixmaps[i] = ecore_x_pixmap_new(0, mPosition.width, mPosition.height, mColorDepth);

    // clear the pixmap
    unsigned int foreground;
    Ecore_X_GC gc;
    foreground = 0;
    gc = ecore_x_gc_new( mX11Pixmaps[i],
                         ECORE_X_GC_VALUE_MASK_FOREGROUND,
                         &foreground );

    DALI_ASSERT_ALWAYS( gc && "CreateRenderable(): failed to get gc" );

    ecore_x_drawable_rectangle_fill( mX11Pixmaps[i], gc, 0, 0, mPosition.width, mPosition.height );

    DALI_ASSERT_ALWAYS( mX11Pixmaps[i] && "Failed to create X pixmap" );

    // we SHOULD guarantee the xpixmap/x11 window was created in x server.
    ecore_x_sync();

    ecore_x_gc_free(gc);
  }
}

void PixmapRenderSurfaceEcoreX::UseExistingRenderable( unsigned int surfaceId )
{
}

unsigned int PixmapRenderSurfaceEcoreX::GetSurfaceId( Any surface ) const
{
  unsigned int surfaceId = 0;

  if ( surface.Empty() == false )
  {
    // check we have a valid type
    DALI_ASSERT_ALWAYS( ( (surface.GetType() == typeid (XWindow) ) ||
                          (surface.GetType() == typeid (Ecore_X_Window) ) )
                        && "Surface type is invalid" );

    if ( surface.GetType() == typeid (Ecore_X_Window) )
    {
      surfaceId = AnyCast<Ecore_X_Window>( surface );
    }
    else
    {
      surfaceId = AnyCast<XWindow>( surface );
    }
  }
  return surfaceId;
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
