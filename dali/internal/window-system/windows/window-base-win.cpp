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
#include <dali/internal/window-system/windows/window-base-win.h>

// EXTERNAL_HEADERS
#include <dali/public-api/object/any.h>
#include <dali/integration-api/debug.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-system.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{

const Device::Class::Type DEFAULT_DEVICE_CLASS = Device::Class::NONE;
const Device::Subclass::Type DEFAULT_DEVICE_SUBCLASS = Device::Subclass::NONE;

const unsigned int PRIMARY_TOUCH_BUTTON_ID( 1 );

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowBaseLogFilter = Debug::Filter::New( Debug::NoLogging, false, "LOG_WINDOW_BASE" );
#endif

} // unnamed namespace

WindowBaseWin::WindowBaseWin( Dali::PositionSize positionSize, Any surface, bool isTransparent )
: mWin32Window( 0 ),
  mOwnSurface( false ),
  mIsTransparent( false ), // Should only be set to true once we actually create a transparent window regardless of what isTransparent is.
  mRotationAppSet( false )
{
  Initialize( positionSize, surface, isTransparent );
}

WindowBaseWin::~WindowBaseWin()
{
  WindowsPlatformImplementation::PostWinMessage( WM_CLOSE, 0, 0, mWin32Window );
}

void WindowBaseWin::Initialize( PositionSize positionSize, Any surface, bool isTransparent )
{
  // see if there is a surface in Any surface
  unsigned int surfaceId = GetSurfaceId( surface );

  // if the surface is empty, create a new one.
  if( surfaceId == 0 )
  {
    // we own the surface about to created
    mOwnSurface = true;
    CreateWinWindow( positionSize, isTransparent );
  }
  else
  {
    // XLib should already be initialized so no point in calling XInitThreads
    mWin32Window = static_cast< WinWindowHandle >( surfaceId );
  }

  WindowsPlatformImplementation::SetListener( MakeCallback( this, &WindowBaseWin::EventEntry ) );
}

void WindowBaseWin::OnDeleteRequest()
{
  mDeleteRequestSignal.Emit();
}

void WindowBaseWin::OnFocusIn( int type, TWinEventInfo *event )
{
}

void WindowBaseWin::OnFocusOut( int type, TWinEventInfo *event )
{
}

void WindowBaseWin::OnWindowDamaged( int type, TWinEventInfo *event )
{
  Event_Mouse_Button* windowDamagedEvent( (Event_Mouse_Button*)event );

  if( windowDamagedEvent->window == mWin32Window )
  {
    DamageArea area;
    area.x = 0;
    area.y = 0;
    WindowSystem::GetScreenSize( area.width, area.height );

    mWindowDamagedSignal.Emit( area );
  }
}

void WindowBaseWin::OnMouseButtonDown( int type, TWinEventInfo *event )
{
  Event_Mouse_Button touchEvent = *((Event_Mouse_Button*)event);
  touchEvent.timestamp = GetTickCount();
  touchEvent.x = LOWORD( event->lParam );
  touchEvent.y = HIWORD( event->lParam );
  touchEvent.multi.device = DEVICE_MOUSE;

  if( touchEvent.window == mWin32Window )
  {
    PointState::Type state ( PointState::DOWN );

    Integration::Point point;
    point.SetDeviceId( touchEvent.multi.device );
    point.SetState( state );
    point.SetScreenPosition( Vector2( touchEvent.x, touchEvent.y + WindowsPlatformImplementation::GetEdgeHeight() ) );
    point.SetRadius( touchEvent.multi.radius, Vector2( touchEvent.multi.radius_x, touchEvent.multi.radius_y ) );
    point.SetPressure( touchEvent.multi.pressure );
    point.SetAngle( Degree( touchEvent.multi.angle ) );

    mTouchEventSignal.Emit( point, touchEvent.timestamp );
  }
}

void WindowBaseWin::OnMouseButtonUp( int type, TWinEventInfo *event )
{
  Event_Mouse_Button touchEvent = *( (Event_Mouse_Button*)event );
  touchEvent.timestamp = GetTickCount();
  touchEvent.x = LOWORD( event->lParam );
  touchEvent.y = HIWORD( event->lParam );
  touchEvent.multi.device = DEVICE_MOUSE;

  if( touchEvent.window == mWin32Window )
  {
    PointState::Type state( PointState::UP );

    Integration::Point point;
    point.SetDeviceId( touchEvent.multi.device );
    point.SetState( state );
    point.SetScreenPosition( Vector2( touchEvent.x, touchEvent.y + WindowsPlatformImplementation::GetEdgeHeight() ) );
    point.SetRadius( touchEvent.multi.radius, Vector2( touchEvent.multi.radius_x, touchEvent.multi.radius_y ) );
    point.SetPressure( touchEvent.multi.pressure );
    point.SetAngle( Degree( touchEvent.multi.angle ) );

    mTouchEventSignal.Emit( point, touchEvent.timestamp );
  }
}

void WindowBaseWin::OnMouseButtonMove( int type, TWinEventInfo *event )
{
  Event_Mouse_Button touchEvent = *((Event_Mouse_Button*)event);
  touchEvent.timestamp = GetTickCount();
  touchEvent.x = LOWORD( event->lParam );
  touchEvent.y = HIWORD( event->lParam );
  touchEvent.multi.device = DEVICE_MOUSE;

  if( touchEvent.window == mWin32Window )
  {
    PointState::Type state( PointState::MOTION );

    Integration::Point point;
    point.SetDeviceId( touchEvent.multi.device );
    point.SetState( state );
    point.SetScreenPosition( Vector2( touchEvent.x, touchEvent.y + WindowsPlatformImplementation::GetEdgeHeight() ) );
    point.SetRadius( touchEvent.multi.radius, Vector2( touchEvent.multi.radius_x, touchEvent.multi.radius_y ) );
    point.SetPressure( touchEvent.multi.pressure );
    point.SetAngle( Degree( touchEvent.multi.angle ) );

    mTouchEventSignal.Emit( point, touchEvent.timestamp );
  }
}

void WindowBaseWin::OnMouseWheel( int type, TWinEventInfo *event )
{
  Event_Mouse_Wheel mouseWheelEvent = *((Event_Mouse_Wheel*)( event ));

  if( mouseWheelEvent.window == mWin32Window )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseWin::OnMouseWheel: direction: %d, modifiers: %d, x: %d, y: %d, z: %d\n", mouseWheelEvent.direction, mouseWheelEvent.modifiers, mouseWheelEvent.x, mouseWheelEvent.y, mouseWheelEvent.z );

    WheelEvent wheelEvent( WheelEvent::MOUSE_WHEEL, mouseWheelEvent.direction, mouseWheelEvent.modifiers, Vector2( mouseWheelEvent.x, mouseWheelEvent.y ), mouseWheelEvent.z, mouseWheelEvent.timestamp );

    mWheelEventSignal.Emit( wheelEvent );
  }
}

void WindowBaseWin::OnKeyDown( int type, TWinEventInfo *event )
{
  if( event->mWindow == mWin32Window )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseWin::OnKeyDown\n" );

    int keyCode = event->wParam;
    std::string keyName( WindowsPlatformImplementation::GetKeyName( keyCode ) );
    std::string keyString;
    std::string emptyString;

    int modifier( 0 );
    unsigned long time( 0 );

    // Ensure key event string is not NULL as keys like SHIFT have a null string.
    keyString.push_back( event->wParam );

    Integration::KeyEvent keyEvent( keyName, emptyString, keyString, keyCode, modifier, time, Integration::KeyEvent::Down, emptyString, emptyString, DEFAULT_DEVICE_CLASS, DEFAULT_DEVICE_SUBCLASS );

    mKeyEventSignal.Emit( keyEvent );
  }
}

void WindowBaseWin::OnKeyUp( int type, TWinEventInfo *event )
{
  if( event->mWindow == mWin32Window )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseWin::OnKeyDown\n" );

    int keyCode = event->wParam;
    std::string keyName( WindowsPlatformImplementation::GetKeyName( keyCode ) );
    std::string keyString;
    std::string emptyString;

    int modifier( 0 );
    unsigned long time( 0 );

    // Ensure key event string is not NULL as keys like SHIFT have a null string.
    keyString.push_back( event->wParam );

    Integration::KeyEvent keyEvent( keyName, emptyString, keyString, keyCode, modifier, time, Integration::KeyEvent::Down, emptyString, emptyString, DEFAULT_DEVICE_CLASS, DEFAULT_DEVICE_SUBCLASS );

    mKeyEventSignal.Emit( keyEvent );
  }
}

Any WindowBaseWin::GetNativeWindow()
{
  return mWin32Window;
}

int WindowBaseWin::GetNativeWindowId()
{
  return mWin32Window;
}

EGLNativeWindowType WindowBaseWin::CreateEglWindow( int width, int height )
{
  return reinterpret_cast< EGLNativeWindowType >( mWin32Window );
}

void WindowBaseWin::DestroyEglWindow()
{
}

void WindowBaseWin::SetEglWindowRotation( int angle )
{
}

void WindowBaseWin::SetEglWindowBufferTransform( int angle )
{
}

void WindowBaseWin::SetEglWindowTransform( int angle )
{
}

void WindowBaseWin::ResizeEglWindow( PositionSize positionSize )
{
}

bool WindowBaseWin::IsEglWindowRotationSupported()
{
  return false;
}

void WindowBaseWin::Move( PositionSize positionSize )
{
}

void WindowBaseWin::Resize( PositionSize positionSize )
{
}

void WindowBaseWin::MoveResize( PositionSize positionSize )
{
}

void WindowBaseWin::SetClass( const std::string& name, const std::string& className )
{
}

void WindowBaseWin::Raise()
{
}

void WindowBaseWin::Lower()
{
}

void WindowBaseWin::Activate()
{
}

void WindowBaseWin::SetAvailableOrientations( const std::vector< Dali::Window::WindowOrientation >& orientations )
{
}

void WindowBaseWin::SetPreferredOrientation( Dali::Window::WindowOrientation orientation )
{
}

void WindowBaseWin::SetAcceptFocus( bool accept )
{
}

void WindowBaseWin::Show()
{
}

void WindowBaseWin::Hide()
{
}

unsigned int WindowBaseWin::GetSupportedAuxiliaryHintCount() const
{
  return 0;
}

std::string WindowBaseWin::GetSupportedAuxiliaryHint( unsigned int index ) const
{
  return std::string();
}

unsigned int WindowBaseWin::AddAuxiliaryHint( const std::string& hint, const std::string& value )
{
  return 0;
}

bool WindowBaseWin::RemoveAuxiliaryHint( unsigned int id )
{
  return false;
}

bool WindowBaseWin::SetAuxiliaryHintValue( unsigned int id, const std::string& value )
{
  return false;
}

std::string WindowBaseWin::GetAuxiliaryHintValue( unsigned int id ) const
{
  return std::string();
}

unsigned int WindowBaseWin::GetAuxiliaryHintId( const std::string& hint ) const
{
  return 0;
}

void WindowBaseWin::SetInputRegion( const Rect< int >& inputRegion )
{
}

void WindowBaseWin::SetType( Dali::Window::Type type )
{
}

bool WindowBaseWin::SetNotificationLevel( Dali::Window::NotificationLevel::Type level )
{
  return false;
}

Dali::Window::NotificationLevel::Type WindowBaseWin::GetNotificationLevel() const
{
  return Dali::Window::NotificationLevel::NONE;
}

void WindowBaseWin::SetOpaqueState( bool opaque )
{
}

bool WindowBaseWin::SetScreenOffMode(Dali::Window::ScreenOffMode::Type screenOffMode)
{
  return false;
}

Dali::Window::ScreenOffMode::Type WindowBaseWin::GetScreenOffMode() const
{
  return Dali::Window::ScreenOffMode::TIMEOUT;
}

bool WindowBaseWin::SetBrightness( int brightness )
{
  return false;
}

int WindowBaseWin::GetBrightness() const
{
  return 0;
}

bool WindowBaseWin::GrabKey( Dali::KEY key, KeyGrab::KeyGrabMode grabMode )
{
  return false;
}

bool WindowBaseWin::UngrabKey( Dali::KEY key )
{
  return false;
}

bool WindowBaseWin::GrabKeyList( const Dali::Vector< Dali::KEY >& key, const Dali::Vector< KeyGrab::KeyGrabMode >& grabMode, Dali::Vector< bool >& result )
{
  return false;
}

bool WindowBaseWin::UngrabKeyList( const Dali::Vector< Dali::KEY >& key, Dali::Vector< bool >& result )
{
  return false;
}

void WindowBaseWin::GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical )
{
  // calculate DPI
  float xres, yres;

  //// 1 inch = 25.4 millimeters
  WindowsPlatformImplementation::GetDPI( mWin32Window, xres, yres );

  xres *= 1.5f;
  yres *= 1.5f;

  dpiHorizontal = static_cast<int>( xres + 0.5f );  // rounding
  dpiVertical = static_cast<int>( yres + 0.5f );
}

int WindowBaseWin::GetScreenRotationAngle()
{
  return 0;
}

void WindowBaseWin::SetWindowRotationAngle( int degree )
{
}

void WindowBaseWin::WindowRotationCompleted( int degree, int width, int height )
{
}

void WindowBaseWin::SetTransparency( bool transparent )
{
}

unsigned int WindowBaseWin::GetSurfaceId( Any surface ) const
{
  unsigned int surfaceId = 0;

  if ( surface.Empty() == false )
  {
    // check we have a valid type
    DALI_ASSERT_ALWAYS( (surface.GetType() == typeid ( WinWindowHandle ) )
                        && "Surface type is invalid" );

    surfaceId = AnyCast< WinWindowHandle >( surface );
  }
  return surfaceId;
}

void WindowBaseWin::CreateWinWindow( PositionSize positionSize, bool isTransparent )
{
  long hWnd = WindowsPlatformImplementation::CreateHwnd( "Demo", "Demo", positionSize.x, positionSize.y, positionSize.width, positionSize.height, NULL );

  WindowsPlatformImplementation::ShowWindow( hWnd );

  mWin32Window = (WinWindowHandle)hWnd;
  DALI_ASSERT_ALWAYS( mWin32Window != 0 && "There is no Windows window" );
}

void WindowBaseWin::EventEntry( TWinEventInfo *event )
{
  unsigned int uMsg = event->uMsg;

  switch( uMsg )
  {
  case WM_SETFOCUS:
  {
    OnFocusIn( uMsg, event );
    break;
  }

  case WM_KILLFOCUS:
  {
    OnFocusOut( uMsg, event );
    break;
  }

  case WM_PAINT:
  {
    OnWindowDamaged( uMsg, event );
    break;
  }

  case WM_LBUTTONDOWN:
  {
    OnMouseButtonDown( uMsg, event );
    break;
  }

  case WM_LBUTTONUP:
  {
    OnMouseButtonUp( uMsg, event );
    break;
  }

  case WM_MOUSEMOVE:
  {
    OnMouseButtonMove( uMsg, event );
    break;
  }

  case WM_MOUSEWHEEL:
  {
    OnMouseWheel( uMsg, event );
    break;
  }

  case WM_KEYDOWN:
  {
    OnKeyDown( uMsg, event );
    break;
  }

  case WM_KEYUP:
  {
    OnKeyUp( uMsg, event );
    break;
  }

  default:
    break;
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

