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
#include <window-render-surface.h>

// EXTERNAL INCLUDES
#include <dlfcn.h>
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-defines.h>

// INTERNAL INCLUDES
#include <wl-types.h>
#include <gl/egl-implementation.h>
#include <base/display-connection.h>
#include <adaptors/common/adaptor-impl.h>
#include <integration-api/trigger-event-factory-interface.h>

namespace Dali
{

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gRenderSurfaceLogFilter;
#endif

namespace ECore
{

namespace
{

const int MINIMUM_DIMENSION_CHANGE( 1 ); ///< Minimum change for window to be considered to have moved
const char* WAYLAND_EGL_SO( "libwayland-egl.so" );

} // unnamed namespace

WindowRenderSurface::WindowRenderSurface( Dali::PositionSize positionSize,
                                          Any surface,
                                          const std::string& name,
                                          bool isTransparent)
: EcoreWlRenderSurface( positionSize, surface, name, isTransparent ),
  mEglWinGetCapabilitiesPtr( NULL ),
  mEglWinSetRotationPtr( NULL ),
  mLibHandle( NULL ),
  mWlWindow( NULL ),
  mWlSurface( NULL ),
  mEglWindow( NULL ),
  mThreadSynchronization( NULL ),
  mRotationTrigger( NULL ),
  mRotationSupported( false ),
  mRotated( false )
{
  DALI_LOG_INFO( gRenderSurfaceLogFilter, Debug::Verbose, "Creating Window\n" );
  Init( surface );
}

WindowRenderSurface::~WindowRenderSurface()
{
  if( mEglWindow != NULL )
  {
    wl_egl_window_destroy(mEglWindow);
    mEglWindow = NULL;
  }

  if( mOwnSurface )
  {
    ecore_wl_window_free( mWlWindow );
  }

  if( mRotationTrigger )
  {
    delete mRotationTrigger;
  }

  if( mLibHandle != NULL )
  {
    dlclose( mLibHandle );
  }
}

Ecore_Wl_Window* WindowRenderSurface::GetDrawable()
{
  // already an e-core type
  return mWlWindow;
}

Any WindowRenderSurface::GetSurface()
{
  // already an e-core type
  return Any( mWlWindow );
}

Ecore_Wl_Window* WindowRenderSurface::GetWlWindow()
{
  return mWlWindow;
}

void WindowRenderSurface::RequestRotation( Dali::Window::WindowOrientation orientation, int width, int height )
{
  if( !mRotationSupported )
  {
    DALI_LOG_INFO( gRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::Rotate: Rotation is not supported!\n" );
    return;
  }

  DALI_LOG_INFO( gRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::Rotate: orientation = %d\n", orientation );

  if( !mRotationTrigger )
  {
    TriggerEventFactoryInterface& triggerFactory = Internal::Adaptor::Adaptor::GetImplementation( Adaptor::Get() ).GetTriggerEventFactoryInterface();
    mRotationTrigger = triggerFactory.CreateTriggerEvent( MakeCallback( this, &WindowRenderSurface::ProcessRotationRequest ), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER );
  }

  mPosition.width = width;
  mPosition.height = height;

  mRotated = true;

  int angle;
  wl_egl_window_rotation rotation;

  switch( orientation )
  {
    case Dali::Window::PORTRAIT:
    {
      angle = 0;
      rotation = ROTATION_0;
      break;
    }
    case Dali::Window::LANDSCAPE:
    {
      angle = 90;
      rotation = ROTATION_270;
      break;
    }
    case Dali::Window::PORTRAIT_INVERSE:
    {
      angle = 180;
      rotation = ROTATION_180;
      break;
    }
    case Dali::Window::LANDSCAPE_INVERSE:
    {
      angle = 270;
      rotation = ROTATION_90;
      break;
    }
    default:
    {
      angle = 0;
      rotation = ROTATION_0;
      break;
    }
  }

  ecore_wl_window_rotation_set( mWlWindow, angle );

  if( mEglWinSetRotationPtr )
  {
    mEglWinSetRotationPtr( mEglWindow, rotation );
  }
}

void WindowRenderSurface::InitializeEgl( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );

  eglImpl.ChooseConfig(true, mColorDepth);
}

void WindowRenderSurface::CreateEglSurface( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );

  // Temporary code for opaque window. We have to modify it after wayland team finish the work.
  if( mColorDepth == COLOR_DEPTH_32 )
  {
    ecore_wl_window_alpha_set( mWlWindow, true );
  }
  else
  {
    ecore_wl_window_alpha_set( mWlWindow, false );
  }

  // create the EGL window
  mEglWindow = wl_egl_window_create( mWlSurface, mPosition.width, mPosition.height );
  EGLNativeWindowType windowType( mEglWindow );
  eglImpl.CreateSurfaceWindow( windowType, mColorDepth );

  // Check capability
  if( !mLibHandle )
  {
    mLibHandle = dlopen( WAYLAND_EGL_SO, RTLD_LAZY );

    char* error = dlerror();
    if( mLibHandle == NULL || error != NULL )
    {
      DALI_LOG_INFO( gRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::CreateEglSurface: dlopen error: %s\n", error );
      return;
    }

    mEglWinGetCapabilitiesPtr = reinterpret_cast< EglWinGetCapabilitiesFunction >( dlsym( mLibHandle, "wl_egl_window_get_capabilities" ) );
    if( !mEglWinGetCapabilitiesPtr )
    {
      DALI_LOG_INFO( gRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::CreateEglSurface: Can't load wl_egl_window_get_capabilities\n" );
      return;
    }

    mEglWinSetRotationPtr = reinterpret_cast< EglWinSetRotationFunction >( dlsym( mLibHandle, "wl_egl_window_set_rotation" ) );
    if( !mEglWinSetRotationPtr )
    {
      DALI_LOG_INFO( gRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::CreateEglSurface: Can't load wl_egl_window_set_rotation\n" );
      return;
    }
  }

  if( mEglWinGetCapabilitiesPtr )
  {
    wl_egl_window_capability capability = static_cast< wl_egl_window_capability >( mEglWinGetCapabilitiesPtr( mEglWindow ) );
    if( capability == WL_EGL_WINDOW_CAPABILITY_ROTATION_SUPPORTED )
    {
      DALI_LOG_INFO( gRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::CreateEglSurface: capability = %d\n", capability );
      mRotationSupported = true;
    }
  }
}

void WindowRenderSurface::DestroyEglSurface( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );
  eglImpl.DestroySurface();

  if( mEglWindow != NULL )
  {
    wl_egl_window_destroy(mEglWindow);
    mEglWindow = NULL;
  }
}

bool WindowRenderSurface::ReplaceEGLSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gRenderSurfaceLogFilter );

  if( mEglWindow != NULL )
  {
    wl_egl_window_destroy(mEglWindow);
    mEglWindow = NULL;
  }

  // Temporary code for opaque window. We have to modify it after wayland team finish the work.
  if( mColorDepth == COLOR_DEPTH_32 )
  {
    ecore_wl_window_alpha_set( mWlWindow, true );
  }
  else
  {
    ecore_wl_window_alpha_set( mWlWindow, false );
  }

  mEglWindow = wl_egl_window_create( mWlSurface, mPosition.width, mPosition.height );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );
  EGLNativeWindowType windowType( mEglWindow );
  return eglImpl.ReplaceSurfaceWindow( windowType );
}

void WindowRenderSurface::MoveResize( Dali::PositionSize positionSize )
{
  bool needToMove = false;
  bool needToResize = false;

  // check moving
  if( (fabs(positionSize.x - mPosition.x) > MINIMUM_DIMENSION_CHANGE) ||
      (fabs(positionSize.y - mPosition.y) > MINIMUM_DIMENSION_CHANGE) )
  {
    needToMove = true;
  }

  // check resizing
  if( (fabs(positionSize.width - mPosition.width) > MINIMUM_DIMENSION_CHANGE) ||
      (fabs(positionSize.height - mPosition.height) > MINIMUM_DIMENSION_CHANGE) )
  {
    needToResize = true;
  }

  if(needToMove)
  {
    ecore_wl_window_position_set( mWlWindow, positionSize.x, positionSize.y );
  }
  if (needToResize)
  {
    ecore_wl_window_update_size( mWlWindow, positionSize.width, positionSize.height );
  }

  mPosition = positionSize;

  wl_egl_window_resize( mEglWindow, mPosition.width, mPosition.height, mPosition.x, mPosition.y );

  DALI_LOG_INFO( gRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::MoveResize: %d, %d, %d, %d\n", mPosition.x, mPosition.y, mPosition.width, mPosition.height );
}

void WindowRenderSurface::Map()
{
  ecore_wl_window_show(mWlWindow);
}

void WindowRenderSurface::StartRender()
{
}

bool WindowRenderSurface::PreRender( EglInterface&, Integration::GlAbstraction& )
{
  // nothing to do for windows
  return true;
}

void WindowRenderSurface::PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, DisplayConnection* displayConnection, bool replacingSurface )
{
  if( mRotated )
  {
    // Check viewport size
    Dali::Vector< GLint > viewportSize;
    viewportSize.Resize( 4 );

    glAbstraction.GetIntegerv( GL_VIEWPORT, &viewportSize[0] );

    if( viewportSize[2] == mPosition.width && viewportSize[3] == mPosition.height )
    {
      DALI_LOG_INFO( gRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::PostRender: Trigger rotation event\n" );

      mRotationTrigger->Trigger();

      if( mThreadSynchronization )
      {
        // Wait until the event-thread complete the rotation event processing
        mThreadSynchronization->PostRenderWaitForCompletion();
      }
    }
  }

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );
  eglImpl.SwapBuffers();

  if( mRenderNotification )
  {
    mRenderNotification->Trigger();
  }
}

void WindowRenderSurface::StopRender()
{
}

void WindowRenderSurface::SetViewMode( ViewMode viewMode )
{
  //FIXME
}

void WindowRenderSurface::CreateWlRenderable()
{
   // if width or height are zero, go full screen.
  if ( (mPosition.width == 0) || (mPosition.height == 0) )
  {
    // Default window size == screen size
    mPosition.x = 0;
    mPosition.y = 0;

    ecore_wl_screen_size_get( &mPosition.width, &mPosition.height );
  }

  mWlWindow = ecore_wl_window_new( 0, mPosition.x, mPosition.y, mPosition.width, mPosition.height, ECORE_WL_WINDOW_BUFFER_TYPE_EGL_WINDOW );

  if ( mWlWindow == 0 )
  {
    DALI_ASSERT_ALWAYS(0 && "Failed to create Wayland window");
  }

  mWlSurface = ecore_wl_window_surface_create( mWlWindow );
}

void WindowRenderSurface::UseExistingRenderable( unsigned int surfaceId )
{
  mWlWindow = AnyCast< Ecore_Wl_Window* >( surfaceId );
}

void WindowRenderSurface::SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization )
{
  DALI_LOG_INFO( gRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::SetThreadSynchronization: called\n" );

  mThreadSynchronization = &threadSynchronization;
}

void WindowRenderSurface::ReleaseLock()
{
  // Nothing to do.
}

void WindowRenderSurface::ProcessRotationRequest()
{
  mRotated = false;

  ecore_wl_window_rotation_change_done_send( mWlWindow );

  DALI_LOG_INFO( gRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurface::ProcessRotationRequest: Rotation Done\n" );

  if( mThreadSynchronization )
  {
    mThreadSynchronization->PostRenderComplete();
  }
}

} // namespace ECore

} // namespace Dali
