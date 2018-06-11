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
#include <dali/internal/window-system/windows/window-render-surface-ecore-win.h>

// EXTERNAL INCLUDES
//#include <X11/Xatom.h>
//#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//
//#include <X11/extensions/Xfixes.h> // for damage notify
//#include <X11/extensions/Xdamage.h> // for damage notify

#include <dali/integration-api/gl-abstraction.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/windows/ecore-win-types.h>
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

WindowRenderSurfaceEcoreWin::WindowRenderSurfaceEcoreWin( Dali::PositionSize positionSize,
                                          Any surface,
                                          const std::string& name,
                                          const std::string& className,
                                          bool isTransparent)
: mTitle( name ),
  mClassName( className ),
  mPosition( positionSize ),
  mColorDepth( isTransparent ? COLOR_DEPTH_32 : COLOR_DEPTH_24 ),
  mWinWindow( 0 ),
  mOwnSurface( false ),
  mNeedToApproveDeiconify( false )
{
  DALI_LOG_INFO( gWindowRenderSurfaceLogFilter, Debug::Verbose, "Creating Window\n" );
  Initialize( surface );
}

WindowRenderSurfaceEcoreWin::~WindowRenderSurfaceEcoreWin()
{
  if( mOwnSurface )
  {
    //ecore_x_window_free( mWinWindow );
  }
}

void WindowRenderSurfaceEcoreWin::Initialize( Any surface )
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

Ecore_Win_Window WindowRenderSurfaceEcoreWin::GetWinWindow()
{
  return mWinWindow;
}

void WindowRenderSurfaceEcoreWin::RequestToApproveDeiconify()
{
  mNeedToApproveDeiconify = true;
}

Any WindowRenderSurfaceEcoreWin::GetWindow()
{
  return mWinWindow;
}

void WindowRenderSurfaceEcoreWin::Map()
{
  //ecore_x_window_show( mWinWindow );
}

void WindowRenderSurfaceEcoreWin::SetRenderNotification( TriggerEventInterface* renderNotification )
{
}

void WindowRenderSurfaceEcoreWin::SetTransparency( bool transparent )
{
}

void WindowRenderSurfaceEcoreWin::RequestRotation( int angle, int width, int height )
{
}

PositionSize WindowRenderSurfaceEcoreWin::GetPositionSize() const
{
  return mPosition;
}

void WindowRenderSurfaceEcoreWin::GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical )
{
  // calculate DPI 
  float xres, yres;

  //// 1 inch = 25.4 millimeters
  Win32WindowSystem::GetDPI( xres, yres );

  xres *= 1.5;
  yres *= 1.5;

  dpiHorizontal = int( xres + 0.5f );  // rounding
  dpiVertical = int( yres + 0.5f );
}

void WindowRenderSurfaceEcoreWin::InitializeEgl( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );

  eglImpl.ChooseConfig(true, mColorDepth);
}

void WindowRenderSurfaceEcoreWin::CreateEglSurface( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );

  // create the EGL surface
  // need to create X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
  XWindow window( mWinWindow );
  eglImpl.CreateSurfaceWindow( reinterpret_cast< EGLNativeWindowType >( window ), mColorDepth );
}

void WindowRenderSurfaceEcoreWin::DestroyEglSurface( EglInterface& eglIf )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( eglIf );
  eglImpl.DestroySurface();
}

bool WindowRenderSurfaceEcoreWin::ReplaceEGLSurface( EglInterface& egl )
{
  DALI_LOG_TRACE_METHOD( gWindowRenderSurfaceLogFilter );

  // need to create X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
  XWindow window( mWinWindow );
  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );

  return eglImpl.ReplaceSurfaceWindow( reinterpret_cast< EGLNativeWindowType >( window ) );
}

void WindowRenderSurfaceEcoreWin::MoveResize( Dali::PositionSize positionSize )
{
  SetWindowPos( (HWND)mWinWindow, NULL, positionSize.x, positionSize.y, positionSize.width, positionSize.height, SWP_NOZORDER | SWP_NOACTIVATE );
}

void WindowRenderSurfaceEcoreWin::SetViewMode( ViewMode viewMode )
{
}

void WindowRenderSurfaceEcoreWin::StartRender()
{
}

bool WindowRenderSurfaceEcoreWin::PreRender( EglInterface&, Integration::GlAbstraction&, bool )
{
  // nothing to do for windows
  return true;
}

void WindowRenderSurfaceEcoreWin::PostRender( EglInterface& egl, Integration::GlAbstraction& glAbstraction, Dali::DisplayConnection* displayConnection, bool replacingSurface, bool resizingSurface )
{
  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( egl );
  eglImpl.SwapBuffers();

  // When the window is deiconified, it approves the deiconify operation to window manager after rendering
  if( mNeedToApproveDeiconify )
  {
    // SwapBuffer is desychronized. So make sure to sychronize when window is deiconified.
    glAbstraction.Finish();

    XDisplay* display = AnyCast<XDisplay *>( displayConnection->GetDisplay() );

    mNeedToApproveDeiconify = false;
  }
}

void WindowRenderSurfaceEcoreWin::StopRender()
{
}

void WindowRenderSurfaceEcoreWin::SetThreadSynchronization( ThreadSynchronizationInterface& /* threadSynchronization */ )
{
  // Nothing to do.
}

void WindowRenderSurfaceEcoreWin::ReleaseLock()
{
  // Nothing to do.
}

RenderSurface::Type WindowRenderSurfaceEcoreWin::GetSurfaceType()
{
  return WINDOW_RENDER_SURFACE;
}

void WindowRenderSurfaceEcoreWin::CreateRenderable()
{
  // if width or height are zero, go full screen.
  if( ( mPosition.width == 0 ) || ( mPosition.height == 0 ) )
  {
    // Default window size == screen size
    mPosition.x = 0;
    mPosition.y = 0;

    char *strWidth = std::getenv( "DALI_WINDOW_WIDTH" );
    if( NULL == strWidth )
    {
      mPosition.width = 480;
    }
    else
    {
      mPosition.width = atoi( strWidth );
    }

    char *strHeight = std::getenv( "DALI_WINDOW_HEIGHT" );
    if( NULL == strHeight )
    {
      mPosition.height = 800;
    }
    else
    {
      mPosition.height = atoi( strHeight );
    }
  }

  long pWnd1 = Win32WindowSystem::CreateWinWindow( mTitle.c_str(), mTitle.c_str(), mPosition.x, mPosition.y, mPosition.width, mPosition.height, NULL );

  //ÏÔÊ¾´°¿Ú
  Win32WindowSystem::ShowWindow( pWnd1 );

  mWinWindow = (Ecore_Win_Window)pWnd1;
  if( mWinWindow == 0 )
  {
    DALI_ASSERT_ALWAYS( 0 && "Failed to create X window" );
  }
}

void WindowRenderSurfaceEcoreWin::UseExistingRenderable( unsigned int surfaceId )
{
  mWinWindow = static_cast< Ecore_Win_Window >( surfaceId );
}

unsigned int WindowRenderSurfaceEcoreWin::GetSurfaceId( Any surface ) const
{
  unsigned int surfaceId = 0;

  if ( surface.Empty() == false )
  {
    // check we have a valid type
    DALI_ASSERT_ALWAYS( ( (surface.GetType() == typeid (XWindow) ) ||
                          (surface.GetType() == typeid (Ecore_Win_Window) ) )
                        && "Surface type is invalid" );

    if ( surface.GetType() == typeid (Ecore_Win_Window) )
    {
      surfaceId = AnyCast<Ecore_Win_Window>( surface );
    }
    else
    {
      surfaceId = AnyCast<unsigned int>(surface);
    }
  }
  return surfaceId;
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
