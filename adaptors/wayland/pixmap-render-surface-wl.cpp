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
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <ecore-wl-types.h>
#include <trigger-event.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gRenderSurfaceLogFilter;
#endif

namespace ECore
{

PixmapRenderSurface::PixmapRenderSurface( Dali::PositionSize positionSize,
                              Any surface,
                              Any display,
                              const std::string& name,
                              bool isTransparent)
: RenderSurface( Dali::RenderSurface::PIXMAP, positionSize, surface, display, name, isTransparent )
{
  Init( surface );
}

PixmapRenderSurface::~PixmapRenderSurface()
{
  // release the surface if we own one
  if( mOwnSurface )
  {
    // if we did create the pixmap, delete the pixmap
    DALI_LOG_INFO( gRenderSurfaceLogFilter, Debug::General, "Own pixmap (%x) freed\n", mX11Pixmap );
  }
}

Dali::RenderSurface::SurfaceType PixmapRenderSurface::GetType()
{
  return Dali::RenderSurface::PIXMAP;
}

Any PixmapRenderSurface::GetSurface()
{
  return Any( NULL );
}

void PixmapRenderSurface::InitializeEgl( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  EglImplementation& eglImpl = static_cast<EglImplementation&>( eglIf );
  eglImpl.InitializeGles( reinterpret_cast< EGLNativeDisplayType >( mMainDisplay ) );

  eglImpl.ChooseConfig(false, mColorDepth);
}

void PixmapRenderSurface::CreateEglSurface( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  //EglImplementation& eglImpl = static_cast<EglImplementation&>( eglIf );

  // create the EGL surface
  // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
  // FIXME
}

void PixmapRenderSurface::DestroyEglSurface( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  EglImplementation& eglImpl = static_cast<EglImplementation&>( eglIf );
  eglImpl.DestroySurface();
}

bool PixmapRenderSurface::ReplaceEGLSurface( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  EglImplementation& eglImpl = static_cast<EglImplementation&>( eglIf );
  eglImpl.InitializeGles( reinterpret_cast< EGLNativeDisplayType >( mMainDisplay ) );

  // a new surface for the new pixmap
  // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
  // FIXME
  return false;
}

bool PixmapRenderSurface::PreRender( EglInterface&, Integration::GlAbstraction& )
{
  // nothing to do for pixmaps
  return true;
}

void PixmapRenderSurface::PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, unsigned int timeDelta, SyncMode syncMode )
{
  // flush gl instruction queue
  glAbstraction.Flush();

  // create damage for client applications which wish to know the update timing
  if( mRenderNotification )
  {
    // use notification trigger
    // Tell the event-thread to render the pixmap
    mRenderNotification->Trigger();
  }
  else
  {
    // as a fallback, send damage event. This is needed until livebox is fixed to
    // stop using damage events for render
    // FIXME
  }

  // Do render synchronisation
  DoRenderSync( timeDelta, syncMode );
}

void PixmapRenderSurface::CreateWlRenderable()
{
  // check we're creating one with a valid size
  DALI_ASSERT_ALWAYS( mPosition.width > 0 && mPosition.height > 0 && "Pixmap size is invalid" );

  // FIXME
}

void PixmapRenderSurface::UseExistingRenderable( unsigned int surfaceId )
{
}

void PixmapRenderSurface::RenderSync()
{
  {
    boost::unique_lock< boost::mutex > lock( mSyncMutex );
    mSyncReceived = true;
  }

  // wake render thread if it was waiting for the notify
  mSyncNotify.notify_all();
}

} // namespace ECore

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
