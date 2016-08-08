/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include "pixmap-render-surface.h"

// EXTERNAL INCLUDES
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/extensions/Xfixes.h> // for damage notify
#include <X11/extensions/Xdamage.h> // for damage notify

#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/debug.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/devel-api/threading/conditional-wait.h>

// INTERNAL INCLUDES

#include <integration-api/thread-synchronization-interface.h>
#include <ecore-x-types.h>
#include <trigger-event.h>
#include <gl/egl-implementation.h>
#include <base/display-connection.h>

namespace Dali
{

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gRenderSurfaceLogFilter;
#endif

namespace ECore
{

namespace
{
static const int INITIAL_PRODUCE_BUFFER_INDEX = 0;
static const int INITIAL_CONSUME_BUFFER_INDEX = 1;
static const int BUFFER_COUNT = 2;
}

struct PixmapRenderSurface::Impl
{
  Impl()
  : mProduceBufferIndex( INITIAL_PRODUCE_BUFFER_INDEX ),
    mConsumeBufferIndex( INITIAL_CONSUME_BUFFER_INDEX ),
    mThreadSynchronization(NULL)
  {
    for (int i = 0; i != BUFFER_COUNT; ++i)
    {
      mX11Pixmaps[i] = 0;
      mEglSurfaces[i] = 0;
    }
  }

  int                             mProduceBufferIndex;
  int                             mConsumeBufferIndex;
  XPixmap                         mX11Pixmaps[BUFFER_COUNT];  ///< X-Pixmap
  EGLSurface                      mEglSurfaces[BUFFER_COUNT];
  ThreadSynchronizationInterface* mThreadSynchronization;     ///< A pointer to the thread-synchronization
  ConditionalWait                 mPixmapCondition;           ///< condition to share pixmap
};

PixmapRenderSurface::PixmapRenderSurface(Dali::PositionSize positionSize,
                                         Any surface,
                                         const std::string& name,
                                         bool isTransparent)
: EcoreXRenderSurface( positionSize, surface, name, isTransparent ),
  mImpl( new Impl )
{
  Init( surface );
}

PixmapRenderSurface::~PixmapRenderSurface()
{
  // release the surface if we own one
  if( mOwnSurface )
  {
    for (int i = 0; i < BUFFER_COUNT; ++i)
    {
      Ecore_X_Pixmap pixmap = mImpl->mX11Pixmaps[i];

      // if we did create the pixmap, delete the pixmap
      DALI_LOG_INFO( gRenderSurfaceLogFilter, Debug::General, "Own pixmap (%x) freed\n", pixmap );
      ecore_x_pixmap_free( pixmap );
    }
  }

  delete mImpl;
}

Ecore_X_Drawable PixmapRenderSurface::GetDrawable()
{
  Ecore_X_Pixmap pixmap = 0;
  {
    ConditionalWait::ScopedLock lock( mImpl->mPixmapCondition );
    pixmap = mImpl->mX11Pixmaps[mImpl->mConsumeBufferIndex];
  }

  return Ecore_X_Drawable( pixmap );
}

Any PixmapRenderSurface::GetSurface()
{
  Ecore_X_Pixmap pixmap = 0;
  {
    ConditionalWait::ScopedLock lock( mImpl->mPixmapCondition );
    pixmap = mImpl->mX11Pixmaps[mImpl->mProduceBufferIndex];
  }

  return Any( pixmap );
}

void PixmapRenderSurface::InitializeEgl( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );

  eglImpl.ChooseConfig(false, mColorDepth);
}

void PixmapRenderSurface::CreateEglSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    // create the EGL surface
    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    XPixmap pixmap = static_cast<XPixmap>( mImpl->mX11Pixmaps[i] );
    mImpl->mEglSurfaces[i] = eglImpl.CreateSurfacePixmap( EGLNativePixmapType( pixmap ), mColorDepth ); // reinterpret_cast does not compile
  }
}

void PixmapRenderSurface::DestroyEglSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    XPixmap pixmap = static_cast<XPixmap>( mImpl->mX11Pixmaps[i] );
    eglImpl.MakeCurrent( EGLNativePixmapType( pixmap ), mImpl->mEglSurfaces[i] );
    eglImpl.DestroySurface();
  }
}

bool PixmapRenderSurface::ReplaceEGLSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  bool contextLost = false;

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    // a new surface for the new pixmap
    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    XPixmap pixmap = static_cast<XPixmap>( mImpl->mX11Pixmaps[i] );
    contextLost = eglImpl.ReplaceSurfacePixmap( EGLNativePixmapType( pixmap ), mImpl->mEglSurfaces[i] ); // reinterpret_cast does not compile
  }

  // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
  XPixmap pixmap = static_cast<XPixmap>( mImpl->mX11Pixmaps[mImpl->mProduceBufferIndex] );
  eglImpl.MakeCurrent( EGLNativePixmapType( pixmap ), mImpl->mEglSurfaces[mImpl->mProduceBufferIndex] );

  return contextLost;
}

void PixmapRenderSurface::StartRender()
{
}

bool PixmapRenderSurface::PreRender( EglInterface& egl, Integration::GlAbstraction& )
{
  // Nothing to do for pixmaps
  return true;
}

void PixmapRenderSurface::PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, DisplayConnection* displayConnection, bool replacingSurface )
{
  // flush gl instruction queue
  glAbstraction.Flush();

  if( mImpl->mThreadSynchronization )
  {
    mImpl->mThreadSynchronization->PostRenderStarted();
  }

  {
    ConditionalWait::ScopedLock lock( mImpl->mPixmapCondition );
    mImpl->mConsumeBufferIndex = __sync_fetch_and_xor( &mImpl->mProduceBufferIndex, 1 ); // Swap buffer indexes.

    Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );

    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    XPixmap pixmap = static_cast<XPixmap>( mImpl->mX11Pixmaps[mImpl->mProduceBufferIndex] );
    eglImpl.MakeCurrent( EGLNativePixmapType( pixmap ), mImpl->mEglSurfaces[mImpl->mProduceBufferIndex] );
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
    Ecore_X_Drawable drawable = Ecore_X_Drawable( mImpl->mX11Pixmaps[mImpl->mProduceBufferIndex] );

    if( drawable )
    {
      XRectangle rect;
      XserverRegion region;

      rect.x = 0;
      rect.y = 0;
      rect.width = mPosition.width;
      rect.height = mPosition.height;

      XDisplay* display = AnyCast<XDisplay*>(displayConnection->GetDisplay());

      // make a fixes region as updated area
      region = XFixesCreateRegion( display, &rect, 1 );
      // add damage event to updated drawable
      XDamageAdd( display, (Drawable)drawable, region );
      XFixesDestroyRegion( display, region );

      XFlush( display );
    }
  }

  if( mImpl->mThreadSynchronization )
  {
    mImpl->mThreadSynchronization->PostRenderWaitForCompletion();
  }
}

void PixmapRenderSurface::StopRender()
{
  ReleaseLock();
}

void PixmapRenderSurface::SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization )
{
  mImpl->mThreadSynchronization = &threadSynchronization;
}

void PixmapRenderSurface::CreateXRenderable()
{
  // check we're creating one with a valid size
  DALI_ASSERT_ALWAYS( mPosition.width > 0 && mPosition.height > 0 && "Pixmap size is invalid" );

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    // create the pixmap
    mImpl->mX11Pixmaps[i] = ecore_x_pixmap_new(0, mPosition.width, mPosition.height, mColorDepth);

    // clear the pixmap
    unsigned int foreground;
    Ecore_X_GC gc;
    foreground = 0;
    gc = ecore_x_gc_new( mImpl->mX11Pixmaps[i],
                         ECORE_X_GC_VALUE_MASK_FOREGROUND,
                         &foreground );

    DALI_ASSERT_ALWAYS( gc && "CreateXRenderable(): failed to get gc" );

    ecore_x_drawable_rectangle_fill( mImpl->mX11Pixmaps[i], gc, 0, 0, mPosition.width, mPosition.height );

    DALI_ASSERT_ALWAYS( mImpl->mX11Pixmaps[i] && "Failed to create X pixmap" );

    // we SHOULD guarantee the xpixmap/x11 window was created in x server.
    ecore_x_sync();

    ecore_x_gc_free(gc);
  }
}

void PixmapRenderSurface::UseExistingRenderable( unsigned int surfaceId )
{
}

void PixmapRenderSurface::ReleaseLock()
{
  if( mImpl->mThreadSynchronization )
  {
    mImpl->mThreadSynchronization->PostRenderComplete();
  }
}

RenderSurface::Type PixmapRenderSurface::GetSurfaceType()
{
  return RenderSurface::ECORE_RENDER_SURFACE;
}

} // namespace ECore

} // namespace Dali
