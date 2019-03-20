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
#include <dali/internal/window-system/android/pixmap-render-surface-android.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/debug.h>
#include <dali/devel-api/threading/mutex.h>

#include <native_window.h>

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
Debug::Filter* gPixmapRenderSurfaceLogFilter = Debug::Filter::New(Debug::Verbose, false, "LOG_PIXMAP_RENDER_SURFACE_ANDROID");
#endif

namespace
{
static const int INITIAL_PRODUCE_BUFFER_INDEX = 0;
static const int INITIAL_CONSUME_BUFFER_INDEX = 1;
}

PixmapRenderSurfaceAndroid::PixmapRenderSurfaceAndroid( Dali::PositionSize positionSize, Any surface, bool isTransparent )
: mGraphics( nullptr ),
  mDisplayConnection( nullptr ),
  mPosition( positionSize ),
  mRenderNotification( NULL ),
  mColorDepth( isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24 ),
  mOwnSurface( false ),
  mProduceBufferIndex( INITIAL_PRODUCE_BUFFER_INDEX ),
  mConsumeBufferIndex( INITIAL_CONSUME_BUFFER_INDEX ),
  mEglSurfaces(),
  mThreadSynchronization( nullptr ),
  mPixmapCondition()
{
  for( int i = 0; i != BUFFER_COUNT; ++i )
  {
    mEglSurfaces[i] = 0;
  }

  Initialize( surface );
}

PixmapRenderSurfaceAndroid::~PixmapRenderSurfaceAndroid()
{
  // release the surface if we own one
  if( mOwnSurface )
  {
    for (int i = 0; i < BUFFER_COUNT; ++i)
    {
    }
  }
}

void PixmapRenderSurfaceAndroid::Initialize( Any surface )
{
  ANativeWindow* window  = GetWindow( surface );

  if( window == nullptr )
  {
    // we own the surface about to created
    mOwnSurface = true;
    CreateRenderable();
  }
  else
  {
    // UseExistingRenderable( surfaceId );
  }
}

Any PixmapRenderSurfaceAndroid::GetSurface()
{
  return Any( nullptr );
}

void PixmapRenderSurfaceAndroid::SetRenderNotification(TriggerEventInterface* renderNotification)
{
  mRenderNotification = renderNotification;
}

PositionSize PixmapRenderSurfaceAndroid::GetPositionSize() const
{
  return mPosition;
}

void PixmapRenderSurfaceAndroid::GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical )
{
  // calculate DPI
  float xres, yres;

  // 1 inch = 25.4 millimeters
  xres = 0;
  yres = 0;

  dpiHorizontal = int( xres + 0.5f );  // rounding
  dpiVertical   = int( yres + 0.5f );
}

void PixmapRenderSurfaceAndroid::InitializeGraphics()
{
  mGraphics = &mAdaptor->GetGraphicsInterface();
  mDisplayConnection = &mAdaptor->GetDisplayConnectionInterface();

  auto eglGraphics = static_cast<EglGraphics *>( mGraphics );
  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();
  eglImpl.ChooseConfig(false, mColorDepth);
}

void PixmapRenderSurfaceAndroid::CreateSurface()
{
  DALI_LOG_TRACE_METHOD( gPixmapRenderSurfaceLogFilter );

  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);
  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    // create the EGL surface
    // mEglSurfaces[i] = eglImpl.CreateSurfacePixmap( EGLNativePixmapType( pixmap ), mColorDepth ); // reinterpret_cast does not compile
  }
}

void PixmapRenderSurfaceAndroid::DestroySurface()
{
  DALI_LOG_TRACE_METHOD( gPixmapRenderSurfaceLogFilter );

  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);

  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    // eglImpl.MakeCurrent( EGLNativePixmapType( pixmap ), mEglSurfaces[i] );
    // eglImpl.DestroySurface( mEglSurfaces[i] );
  }
}

bool PixmapRenderSurfaceAndroid::ReplaceGraphicsSurface()
{
  DALI_LOG_TRACE_METHOD( gPixmapRenderSurfaceLogFilter );

  bool contextLost = false;

  auto eglGraphics = static_cast<EglGraphics *>( mGraphics );

  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
    // a new surface for the new pixmap
    // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
    // contextLost = eglImpl.ReplaceSurfacePixmap( EGLNativePixmapType( pixmap ), mEglSurfaces[i] ); // reinterpret_cast does not compile
  }

  // need to cast to X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
  // eglImpl.MakeCurrent( EGLNativePixmapType( pixmap ), mEglSurfaces[mProduceBufferIndex] );

  return contextLost;
}

void PixmapRenderSurfaceAndroid::StartRender()
{
}

bool PixmapRenderSurfaceAndroid::PreRender( bool )
{
  // Nothing to do for pixmaps
  return true;
}

void PixmapRenderSurfaceAndroid::PostRender( bool renderToFbo, bool replacingSurface, bool resizingSurface )
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
    // eglImpl.MakeCurrent( EGLNativePixmapType( pixmap ), mEglSurfaces[mProduceBufferIndex] );
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
  }

  if( mThreadSynchronization )
  {
    mThreadSynchronization->PostRenderWaitForCompletion();
  }
}

void PixmapRenderSurfaceAndroid::StopRender()
{
  ReleaseLock();
}

void PixmapRenderSurfaceAndroid::SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization )
{
  mThreadSynchronization = &threadSynchronization;
}

void PixmapRenderSurfaceAndroid::ReleaseLock()
{
  if( mThreadSynchronization )
  {
    mThreadSynchronization->PostRenderComplete();
  }
}

Integration::RenderSurface::Type PixmapRenderSurfaceAndroid::GetSurfaceType()
{
  return Integration::RenderSurface::PIXMAP_RENDER_SURFACE;
}

void PixmapRenderSurfaceAndroid::MakeContextCurrent()
{
}

void PixmapRenderSurfaceAndroid::CreateRenderable()
{
  // check we're creating one with a valid size
  DALI_ASSERT_ALWAYS( mPosition.width > 0 && mPosition.height > 0 && "Pixmap size is invalid" );

  for (int i = 0; i < BUFFER_COUNT; ++i)
  {
  }
}

void PixmapRenderSurfaceAndroid::UseExistingRenderable( unsigned int surfaceId  )
{
}

ANativeWindow* PixmapRenderSurfaceAndroid::GetWindow( Any surface ) const
{
  ANativeWindow* window = nullptr;

  if( surface.Empty() == false )
  {
    window = AnyCast<ANativeWindow*>( surface );
  }
  return window;
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
