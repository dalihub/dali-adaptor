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
#include <dali/internal/window-system/ubuntu-x11/window-render-surface-ecore-x.h>

// EXTERNAL INCLUDES
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/extensions/Xfixes.h> // for damage notify
#include <X11/extensions/Xdamage.h> // for damage notify

#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/ubuntu-x11/ecore-x-types.h>
#include <dali/internal/system/common/trigger-event.h>
#include <dali/internal/graphics/gles20/egl-implementation.h>
#include <dali/internal/window-system/common/display-connection.h>

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
Debug::Filter* gWindowRenderSurfaceLogFilter = Debug::Filter::New(Debug::Verbose, false, "LOG_WINDOW_RENDER_SURFACE_ECORE_X");
#endif

} // unnamed namespace

WindowRenderSurfaceEcoreX::WindowRenderSurfaceEcoreX( Dali::PositionSize positionSize,
                                          Any surface,
                                          const std::string& name,
                                          const std::string& className,
                                          bool isTransparent)
: mTitle( name ),
  mClassName( className ),
  mPosition( positionSize ),
  mColorDepth( isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24 ),
  mX11Window( 0 ),
  mOwnSurface( false ),
  mNeedToApproveDeiconify( false )
{
  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "Creating Window\n" );
  Initialize( surface );
}

WindowRenderSurfaceEcoreX::~WindowRenderSurfaceEcoreX()
{
  if( mOwnSurface )
  {
    ecore_x_window_free( mX11Window );
  }
}

void WindowRenderSurfaceEcoreX::Initialize( Any surface )
{
  // see if there is a surface in Any surface
  unsigned int surfaceId = GetSurfaceId( surface );

  // if the surface is empty, create a new one.
  if( surfaceId == 0 )
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

Ecore_X_Window WindowRenderSurfaceEcoreX::GetXWindow()
{
  return mX11Window;
}

void WindowRenderSurfaceEcoreX::RequestToApproveDeiconify()
{
  mNeedToApproveDeiconify = true;
}

Any WindowRenderSurfaceEcoreX::GetWindow()
{
  return mX11Window;
}

void WindowRenderSurfaceEcoreX::Map()
{
  ecore_x_window_show( mX11Window );
}

void WindowRenderSurfaceEcoreX::SetRenderNotification( TriggerEventInterface* renderNotification )
{
}

void WindowRenderSurfaceEcoreX::SetTransparency( bool transparent )
{
}

void WindowRenderSurfaceEcoreX::RequestRotation( int angle, int width, int height )
{
}

PositionSize WindowRenderSurfaceEcoreX::GetPositionSize() const
{
  return mPosition;
}

void WindowRenderSurfaceEcoreX::GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical )
{
  // calculate DPI
  float xres, yres;

  // 1 inch = 25.4 millimeters
  xres = ecore_x_dpi_get();
  yres = ecore_x_dpi_get();

  dpiHorizontal = int( xres + 0.5f );  // rounding
  dpiVertical   = int( yres + 0.5f );
}

void WindowRenderSurfaceEcoreX::InitializeEgl( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );

  eglImpl.ChooseConfig(true, mColorDepth);
}

void WindowRenderSurfaceEcoreX::CreateEglSurface( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );

  // create the EGL surface
  // need to create X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
  XWindow window( mX11Window );
  eglImpl.CreateSurfaceWindow( reinterpret_cast< EGLNativeWindowType >( window ), mColorDepth );
}

void WindowRenderSurfaceEcoreX::DestroyEglSurface( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );
  eglImpl.DestroySurface();
}

bool WindowRenderSurfaceEcoreX::ReplaceEGLSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  // need to create X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
  XWindow window( mX11Window );
  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );

  return eglImpl.ReplaceSurfaceWindow( reinterpret_cast< EGLNativeWindowType >( window ) );
}

void WindowRenderSurfaceEcoreX::MoveResize( Dali::PositionSize positionSize )
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

  if( needToMove &&  needToResize)
  {
    ecore_x_window_move_resize(mX11Window, positionSize.x, positionSize.y, positionSize.width, positionSize.height);
    mPosition = positionSize;
  }
  else if(needToMove)
  {
    ecore_x_window_move(mX11Window, positionSize.x, positionSize.y);
    mPosition = positionSize;
  }
  else if (needToResize)
  {
    ecore_x_window_resize(mX11Window, positionSize.width, positionSize.height);
    mPosition = positionSize;
  }
}

void WindowRenderSurfaceEcoreX::SetViewMode( ViewMode viewMode )
{
  Ecore_X_Atom viewModeAtom( ecore_x_atom_get( "_E_COMP_3D_APP_WIN" ) );

  if( viewModeAtom != None )
  {
    unsigned int value( static_cast<unsigned int>( viewMode ) );
    ecore_x_window_prop_card32_set( mX11Window, viewModeAtom, &value, 1 );
  }
}

void WindowRenderSurfaceEcoreX::StartRender()
{
}

bool WindowRenderSurfaceEcoreX::PreRender( EglInterface&, Integration::GlAbstraction&, bool )
{
  // nothing to do for windows
  return true;
}

void WindowRenderSurfaceEcoreX::PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, Dali::DisplayConnection* displayConnection, bool replacingSurface, bool resizingSurface )
{
  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );
  eglImpl.SwapBuffers();

  // When the window is deiconified, it approves the deiconify operation to window manager after rendering
  if(mNeedToApproveDeiconify)
  {
    // SwapBuffer is desychronized. So make sure to sychronize when window is deiconified.
    //glAbstraction.Finish();

    XDisplay* display = AnyCast<XDisplay *>(displayConnection->GetDisplay());

#ifndef DALI_PROFILE_UBUNTU
    /* client sends immediately reply message using value 1 */
    XEvent xev;

    xev.xclient.window = mX11Window;
    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = ECORE_X_ATOM_E_DEICONIFY_APPROVE;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = mX11Window;
    xev.xclient.data.l[1] = 1;
    xev.xclient.data.l[2] = 0;
    xev.xclient.data.l[3] = 0;
    xev.xclient.data.l[4] = 0;

    XSendEvent(display, mX11Window, false, ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, &xev);
#endif // DALI_PROFILE_UBUNTU

    XSync(display, false);

    mNeedToApproveDeiconify = false;
  }
}

void WindowRenderSurfaceEcoreX::StopRender()
{
}

void WindowRenderSurfaceEcoreX::SetThreadSynchronization( ThreadSynchronizationInterface& /* threadSynchronization */ )
{
  // Nothing to do.
}

void WindowRenderSurfaceEcoreX::ReleaseLock()
{
  // Nothing to do.
}

RenderSurface::Type WindowRenderSurfaceEcoreX::GetSurfaceType()
{
  return WINDOW_RENDER_SURFACE;
}

void WindowRenderSurfaceEcoreX::CreateRenderable()
{
   // if width or height are zero, go full screen.
  if ( (mPosition.width == 0) || (mPosition.height == 0) )
  {
    // Default window size == screen size
    mPosition.x = 0;
    mPosition.y = 0;

    ecore_x_screen_size_get( ecore_x_default_screen_get(), &mPosition.width, &mPosition.height );
  }

  if(mColorDepth == COLOR_DEPTH_32)
  {
    // create 32 bit window
    mX11Window = ecore_x_window_argb_new( 0, mPosition.x, mPosition.y, mPosition.width, mPosition.height );
  }
  else
  {
    // create 24 bit window
    mX11Window = ecore_x_window_new( 0, mPosition.x, mPosition.y, mPosition.width, mPosition.height );
  }

  if ( mX11Window == 0 )
  {
      DALI_ASSERT_ALWAYS(0 && "Failed to create X window");
  }

  // set up window title which will be helpful for debug utitilty
  ecore_x_icccm_title_set( mX11Window, mTitle.c_str() );
  ecore_x_netwm_name_set( mX11Window, mTitle.c_str() );
  ecore_x_icccm_name_class_set( mX11Window, mTitle.c_str(), mClassName.c_str() );

  // set up etc properties to match with ecore-evas
  char *id = NULL;
  if( ( id = getenv("DESKTOP_STARTUP_ID") ) )
  {
    ecore_x_netwm_startup_id_set( mX11Window, id );
  }

  ecore_x_icccm_hints_set( mX11Window,
                           1,                                // accepts_focus
                           ECORE_X_WINDOW_STATE_HINT_NORMAL, // initial_state
                           0,                                // icon_pixmap
                           0,                                // icon_mask
                           0,                                // icon_window
                           0,                                // window_group
                           0 );                              // is_urgent

  // we SHOULD guarantee the x11 window was created in x server.
  ecore_x_sync();
}

void WindowRenderSurfaceEcoreX::UseExistingRenderable( unsigned int surfaceId )
{
  mX11Window = static_cast< Ecore_X_Window >( surfaceId );
}

unsigned int WindowRenderSurfaceEcoreX::GetSurfaceId( Any surface ) const
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
