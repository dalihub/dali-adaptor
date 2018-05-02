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
#include <dali/internal/window-system/tizen-wayland/ecore-wl2/window-render-surface-ecore-wl2.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles20/egl-implementation.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/integration-api/trigger-event-factory-interface.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

namespace
{

const int MINIMUM_DIMENSION_CHANGE( 1 ); ///< Minimum change for window to be considered to have moved

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowRenderSurfaceLogFilter = Debug::Filter::New(Debug::Verbose, false, "LOG_WINDOW_RENDER_SURFACE_ECORE_WL2");
#endif

} // unnamed namespace

WindowRenderSurfaceEcoreWl2::WindowRenderSurfaceEcoreWl2( Dali::PositionSize positionSize,
                                                        Any surface,
                                                        const std::string& name,
                                                        bool isTransparent)
: mTitle( name ),
  mPositionSize( positionSize ),
  mWlWindow( NULL ),
  mWlSurface( NULL ),
  mEglWindow( NULL ),
  mThreadSynchronization( NULL ),
  mRenderNotification( NULL ),
  mRotationTrigger( NULL ),
  mColorDepth( isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24 ),
  mRotationAngle( 0 ),
  mScreenRotationAngle( 0 ),
  mOwnSurface( false ),
  mRotationSupported( false ),
  mRotationFinished( true ),
  mScreenRotationFinished( true ),
  mResizeFinished( true )
{
  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "Creating Window\n" );
  Initialize( surface );
}

WindowRenderSurfaceEcoreWl2::~WindowRenderSurfaceEcoreWl2()
{
  if( mEglWindow != NULL )
  {
    wl_egl_window_destroy(mEglWindow);
    mEglWindow = NULL;
  }

  if( mOwnSurface )
  {
    ecore_wl2_window_free( mWlWindow );
  }

  if( mRotationTrigger )
  {
    delete mRotationTrigger;
  }

  if( mOwnSurface )
  {
    ecore_wl2_shutdown();
  }
}

void WindowRenderSurfaceEcoreWl2::Initialize( Any surface )
{
  // see if there is a surface in Any surface
  unsigned int surfaceId = GetSurfaceId( surface );

  // if the surface is empty, create a new one.
  if( surfaceId == 0 )
  {
    // we own the surface about to created
    ecore_wl2_init();
    mOwnSurface = true;
    CreateRenderable();
  }
  else
  {
    // XLib should already be initialized so no point in calling XInitThreads
    UseExistingRenderable( surfaceId );
  }
}

Ecore_Wl2_Window* WindowRenderSurfaceEcoreWl2::GetWlWindow()
{
  return mWlWindow;
}

void WindowRenderSurfaceEcoreWl2::OutputTransformed()
{
  int transform;

  if( ecore_wl2_window_ignore_output_transform_get( mWlWindow ) )
  {
    transform = 0;
  }
  else
  {
    transform = ecore_wl2_output_transform_get( ecore_wl2_window_output_find( mWlWindow ) );
  }

  mScreenRotationAngle = transform * 90;
  mScreenRotationFinished = false;

  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurfaceEcoreWl2::OutputTransformed: angle = %d screen rotation = %d\n", mRotationAngle, mScreenRotationAngle );
}

Any WindowRenderSurfaceEcoreWl2::GetWindow()
{
  return mWlWindow;
}

void WindowRenderSurfaceEcoreWl2::Map()
{
  ecore_wl2_window_show( mWlWindow );
}

void WindowRenderSurfaceEcoreWl2::SetRenderNotification( TriggerEventInterface* renderNotification )
{
  mRenderNotification = renderNotification;
}

void WindowRenderSurfaceEcoreWl2::SetTransparency( bool transparent )
{
  ecore_wl2_window_alpha_set( mWlWindow, transparent );
}

void WindowRenderSurfaceEcoreWl2::RequestRotation( int angle, int width, int height )
{
  if( !mRotationSupported )
  {
    DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurfaceEcoreWl2::Rotate: Rotation is not supported!\n" );
    return;
  }

  if( !mRotationTrigger )
  {
    TriggerEventFactoryInterface& triggerFactory = Internal::Adaptor::Adaptor::GetImplementation( Adaptor::Get() ).GetTriggerEventFactoryInterface();
    mRotationTrigger = triggerFactory.CreateTriggerEvent( MakeCallback( this, &WindowRenderSurfaceEcoreWl2::ProcessRotationRequest ), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER );
  }

  mPositionSize.width = width;
  mPositionSize.height = height;

  mRotationAngle = angle;
  mRotationFinished = false;

  ecore_wl2_window_rotation_set( mWlWindow, mRotationAngle );

  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurfaceEcoreWl2::Rotate: angle = %d screen rotation = %d\n", mRotationAngle, mScreenRotationAngle );
}

PositionSize WindowRenderSurfaceEcoreWl2::GetPositionSize() const
{
  return mPositionSize;
}

void WindowRenderSurfaceEcoreWl2::GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical )
{
  // calculate DPI
  float xres, yres;

  Ecore_Wl2_Output* output = ecore_wl2_window_output_find( mWlWindow );

  // 1 inch = 25.4 millimeters
  xres = ecore_wl2_output_dpi_get( output );
  yres = ecore_wl2_output_dpi_get( output );

  dpiHorizontal = int( xres + 0.5f );  // rounding
  dpiVertical   = int( yres + 0.5f );
}

void WindowRenderSurfaceEcoreWl2::InitializeEgl( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );

  eglImpl.ChooseConfig(true, mColorDepth);
}

void WindowRenderSurfaceEcoreWl2::CreateEglSurface( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );

  // create the EGL window
  if( mScreenRotationAngle == 0 || mScreenRotationAngle == 180 )
  {
    mEglWindow = wl_egl_window_create( mWlSurface, mPositionSize.width, mPositionSize.height );
  }
  else
  {
    mEglWindow = wl_egl_window_create( mWlSurface, mPositionSize.height, mPositionSize.width );
  }

  EGLNativeWindowType windowType( mEglWindow );
  eglImpl.CreateSurfaceWindow( windowType, mColorDepth );

  // Check capability
  wl_egl_window_capability capability = static_cast< wl_egl_window_capability >( wl_egl_window_get_capabilities( mEglWindow ) );
  if( capability == WL_EGL_WINDOW_CAPABILITY_ROTATION_SUPPORTED )
  {
    DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurfaceEcoreWl2::CreateEglSurface: capability = %d\n", capability );
    mRotationSupported = true;
  }

  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurfaceEcoreWl2::CreateEglSurface: w = %d h = %d angle = %d screen rotation = %d\n", mPositionSize.width, mPositionSize.height, mRotationAngle, mScreenRotationAngle );
}

void WindowRenderSurfaceEcoreWl2::DestroyEglSurface( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );
  eglImpl.DestroySurface();

  if( mEglWindow != NULL )
  {
    wl_egl_window_destroy(mEglWindow);
    mEglWindow = NULL;
  }
}

bool WindowRenderSurfaceEcoreWl2::ReplaceEGLSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  if( mEglWindow != NULL )
  {
    wl_egl_window_destroy(mEglWindow);
    mEglWindow = NULL;
  }

  if( mScreenRotationAngle == 0 || mScreenRotationAngle == 180 )
  {
    mEglWindow = wl_egl_window_create( mWlSurface, mPositionSize.width, mPositionSize.height );
  }
  else
  {
    mEglWindow = wl_egl_window_create( mWlSurface, mPositionSize.height, mPositionSize.width );
  }

  // Set screen rotation
  mScreenRotationFinished = false;

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );
  EGLNativeWindowType windowType( mEglWindow );
  return eglImpl.ReplaceSurfaceWindow( windowType );
}

void WindowRenderSurfaceEcoreWl2::MoveResize( Dali::PositionSize positionSize )
{
  bool needToMove = false;
  bool needToResize = false;

  // check moving
  if( (fabs(positionSize.x - mPositionSize.x) > MINIMUM_DIMENSION_CHANGE) ||
      (fabs(positionSize.y - mPositionSize.y) > MINIMUM_DIMENSION_CHANGE) )
  {
    needToMove = true;
  }

  // check resizing
  if( (fabs(positionSize.width - mPositionSize.width) > MINIMUM_DIMENSION_CHANGE) ||
      (fabs(positionSize.height - mPositionSize.height) > MINIMUM_DIMENSION_CHANGE) )
  {
    needToResize = true;
  }

  if( needToMove || needToResize )
  {
    ecore_wl2_window_geometry_set( mWlWindow, positionSize.x, positionSize.y, positionSize.width, positionSize.height );

    if( needToResize )
    {
      mResizeFinished = false;
    }
  }

  mPositionSize = positionSize;

  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurfaceEcoreWl2::MoveResize: %d, %d, %d, %d\n", mPositionSize.x, mPositionSize.y, mPositionSize.width, mPositionSize.height );
}

void WindowRenderSurfaceEcoreWl2::SetViewMode( ViewMode viewMode )
{
}

void WindowRenderSurfaceEcoreWl2::StartRender()
{
}

bool WindowRenderSurfaceEcoreWl2::PreRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, bool resizingSurface )
{
  if( resizingSurface )
  {
#ifdef OVER_TIZEN_VERSION_4
    // Window rotate or screen rotate
    if( !mRotationFinished || !mScreenRotationFinished )
    {
      wl_egl_window_rotation rotation;
      wl_output_transform bufferTransform;
      int totalAngle = (mRotationAngle + mScreenRotationAngle) % 360;

      switch( totalAngle )
      {
        case 0:
        {
          rotation = ROTATION_0;
          bufferTransform = WL_OUTPUT_TRANSFORM_NORMAL;
          break;
        }
        case 90:
        {
          rotation = ROTATION_270;
          bufferTransform = WL_OUTPUT_TRANSFORM_90;
          break;
        }
        case 180:
        {
          rotation = ROTATION_180;
          bufferTransform = WL_OUTPUT_TRANSFORM_180;
          break;
        }
        case 270:
        {
          rotation = ROTATION_90;
          bufferTransform = WL_OUTPUT_TRANSFORM_270;
          break;
        }
        default:
        {
          rotation = ROTATION_0;
          bufferTransform = WL_OUTPUT_TRANSFORM_NORMAL;
          break;
        }
      }

      wl_egl_window_set_rotation( mEglWindow, rotation );

      wl_egl_window_set_buffer_transform( mEglWindow, bufferTransform );

      // Reset only screen rotation flag
      mScreenRotationFinished = true;

      DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurfaceEcoreWl2::PreRender: Set rotation [%d] [%d]\n", mRotationAngle, mScreenRotationAngle );
    }

    // Only window rotate
    if( !mRotationFinished )
    {
      wl_output_transform windowTransform;

      switch( mRotationAngle )
      {
        case 0:
        {
          windowTransform = WL_OUTPUT_TRANSFORM_NORMAL;
          break;
        }
        case 90:
        {
          windowTransform = WL_OUTPUT_TRANSFORM_90;
          break;
        }
        case 180:
        {
          windowTransform = WL_OUTPUT_TRANSFORM_180;
          break;
        }
        case 270:
        {
          windowTransform = WL_OUTPUT_TRANSFORM_270;
          break;
        }
        default:
        {
          windowTransform = WL_OUTPUT_TRANSFORM_NORMAL;
          break;
        }
      }

      wl_egl_window_set_window_transform( mEglWindow, windowTransform );
    }
#endif

    // Resize case
    if( !mResizeFinished )
    {
      wl_egl_window_resize( mEglWindow, mPositionSize.width, mPositionSize.height, mPositionSize.x, mPositionSize.y );
      mResizeFinished = true;

      DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurfaceEcoreWl2::PreRender: Set resize\n" );
    }
  }

  return true;
}

void WindowRenderSurfaceEcoreWl2::PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, DisplayConnection* displayConnection, bool replacingSurface, bool resizingSurface )
{
  if( resizingSurface )
  {
    if( !mRotationFinished )
    {
      DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurfaceEcoreWl2::PostRender: Trigger rotation event\n" );

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

void WindowRenderSurfaceEcoreWl2::StopRender()
{
}

void WindowRenderSurfaceEcoreWl2::SetThreadSynchronization( ThreadSynchronizationInterface& threadSynchronization )
{
  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurfaceEcoreWl2::SetThreadSynchronization: called\n" );

  mThreadSynchronization = &threadSynchronization;
}

void WindowRenderSurfaceEcoreWl2::ReleaseLock()
{
  // Nothing to do.
}

RenderSurface::Type WindowRenderSurfaceEcoreWl2::GetSurfaceType()
{
  return RenderSurface::WINDOW_RENDER_SURFACE;
}

void WindowRenderSurfaceEcoreWl2::CreateRenderable()
{
  Ecore_Wl2_Display* display = ecore_wl2_display_connect( NULL );
  if ( !display )
  {
    DALI_ASSERT_ALWAYS(0 && "Failed to get display");
  }

   // if width or height are zero, go full screen.
  if ( (mPositionSize.width == 0) || (mPositionSize.height == 0) )
  {
    // Default window size == screen size
    mPositionSize.x = 0;
    mPositionSize.y = 0;

    ecore_wl2_display_screen_size_get( display, &mPositionSize.width, &mPositionSize.height );
  }

  mWlWindow = ecore_wl2_window_new( display, NULL, mPositionSize.x, mPositionSize.y, mPositionSize.width, mPositionSize.height );

  if ( mWlWindow == 0 )
  {
    DALI_ASSERT_ALWAYS(0 && "Failed to create Wayland window");
  }

  mWlSurface = ecore_wl2_window_surface_get( mWlWindow );

  if( mColorDepth == COLOR_DEPTH_32 )
  {
    ecore_wl2_window_alpha_set( mWlWindow, true );
  }
  else
  {
    ecore_wl2_window_alpha_set( mWlWindow, false );
  }

  ecore_wl2_window_type_set( mWlWindow, ECORE_WL2_WINDOW_TYPE_TOPLEVEL );

  // Get output transform
  if( !ecore_wl2_window_ignore_output_transform_get( mWlWindow ) )
  {
    Ecore_Wl2_Output* output = ecore_wl2_window_output_find( mWlWindow );

    int transform = ecore_wl2_output_transform_get( output );

    mScreenRotationAngle = transform * 90;
    mScreenRotationFinished = false;
  }
}

void WindowRenderSurfaceEcoreWl2::UseExistingRenderable( unsigned int surfaceId )
{
  mWlWindow = AnyCast< Ecore_Wl2_Window* >( surfaceId );
}

unsigned int WindowRenderSurfaceEcoreWl2::GetSurfaceId( Any surface ) const
{
  unsigned int surfaceId = 0;

  if( surface.Empty() == false )
  {
    // check we have a valid type
    DALI_ASSERT_ALWAYS( ( (surface.GetType() == typeid (Ecore_Wl2_Window *) ) )
                        && "Surface type is invalid" );

    surfaceId = AnyCast<unsigned int>( surface );
  }
  return surfaceId;
}

void WindowRenderSurfaceEcoreWl2::ProcessRotationRequest()
{
  mRotationFinished = true;

  ecore_wl2_window_rotation_change_done_send( mWlWindow, mRotationAngle, mPositionSize.width, mPositionSize.height );

  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "WindowRenderSurfaceEcoreWl2::ProcessRotationRequest: Rotation Done\n" );

  if( mThreadSynchronization )
  {
    mThreadSynchronization->PostRenderComplete();
  }
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
