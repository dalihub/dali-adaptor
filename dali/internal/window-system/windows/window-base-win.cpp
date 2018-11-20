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

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>

// EXTERNAL_HEADERS
#include <dali/public-api/object/any.h>
#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{

const std::string DEFAULT_DEVICE_NAME = "";
const Device::Class::Type DEFAULT_DEVICE_CLASS = Device::Class::NONE;
const Device::Subclass::Type DEFAULT_DEVICE_SUBCLASS = Device::Subclass::NONE;

const unsigned int PRIMARY_TOUCH_BUTTON_ID( 1 );

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowBaseLogFilter = Debug::Filter::New( Debug::NoLogging, false, "LOG_WINDOW_BASE" );
#endif

static bool EcoreEventWindowPropertyChanged( void* data, int type, TWinEventInfo *event )
{
  WindowBaseWin* windowBase = static_cast< WindowBaseWin* >( data );
  if( windowBase )
  {
    return windowBase->OnWindowPropertyChanged( data, type, event );
  }

  return true;
}

/**
 * Called when the window gains focus.
 */
static bool EcoreEventWindowFocusIn( void* data, int type, TWinEventInfo *event )
{
  WindowBaseWin* windowBase = static_cast< WindowBaseWin* >( data );
  if( windowBase )
  {
    windowBase->OnFocusIn( data, type, event );
  }
  return true;
}

/**
 * Called when the window loses focus.
 */
static bool EcoreEventWindowFocusOut( void* data, int type, TWinEventInfo *event )
{
  WindowBaseWin* windowBase = static_cast< WindowBaseWin* >( data );
  if( windowBase )
  {
    windowBase->OnFocusOut( data, type, event );
  }
  return true;
}

/**
 * Called when the window is damaged.
 */
static bool EcoreEventWindowDamaged( void* data, int type, TWinEventInfo *event )
{
  WindowBaseWin* windowBase = static_cast< WindowBaseWin* >( data );
  if( windowBase )
  {
    windowBase->OnWindowDamaged( data, type, event );
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Selection Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when the source window notifies us the content in clipboard is selected.
 */
static bool EcoreEventSelectionClear( void* data, int type, TWinEventInfo *event )
{
  WindowBaseWin* windowBase = static_cast< WindowBaseWin* >( data );
  if( windowBase )
  {
    windowBase->OnSelectionClear( data, type, event );
  }
  return true;
}

/**
 * Called when the source window sends us about the selected content.
 * For example, when dragged items are dragged INTO our window or when items are selected in the clipboard.
 */
static bool EcoreEventSelectionNotify( void* data, int type, TWinEventInfo *event )
{
  WindowBaseWin* windowBase = static_cast< WindowBaseWin* >( data );
  if( windowBase )
  {
    windowBase->OnSelectionNotify( data, type, event );
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Touch Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a touch down is received.
 */
static bool EcoreEventMouseButtonDown( void* data, int type, TWinEventInfo *event )
{
  WindowBaseWin* windowBase = static_cast< WindowBaseWin* >( data );
  if( windowBase )
  {
    windowBase->OnMouseButtonDown( data, type, event );
  }
  return true;
}

/**
 * Called when a touch up is received.
 */
static bool EcoreEventMouseButtonUp( void* data, int type, TWinEventInfo *event )
{
  WindowBaseWin* windowBase = static_cast< WindowBaseWin* >( data );
  if( windowBase )
  {
    windowBase->OnMouseButtonUp( data, type, event );
  }
  return true;
}

/**
 * Called when a touch motion is received.
 */
static bool EcoreEventMouseButtonMove( void* data, int type, TWinEventInfo *event )
{
  WindowBaseWin* windowBase = static_cast< WindowBaseWin* >( data );
  if( windowBase )
  {
    windowBase->OnMouseButtonMove( data, type, event );
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Wheel Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a mouse wheel is received.
 */
static bool EcoreEventMouseWheel( void* data, int type, TWinEventInfo *event )
{
  WindowBaseWin* windowBase = static_cast< WindowBaseWin* >( data );
  if( windowBase )
  {
    windowBase->OnMouseWheel( data, type, event );
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Key Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a key down is received.
 */
static bool EcoreEventKeyDown( void* data, int type, TWinEventInfo *event )
{
  WindowBaseWin* windowBase = static_cast< WindowBaseWin* >( data );
  if( windowBase )
  {
    windowBase->OnKeyDown( data, type, event );
  }
  return true;
}

/**
 * Called when a key up is received.
 */
static bool EcoreEventKeyUp( void* data, int type, TWinEventInfo *event )
{
  WindowBaseWin* windowBase = static_cast< WindowBaseWin* >( data );
  if( windowBase )
  {
    windowBase->OnKeyUp( data, type, event );
  }
  return true;
}
} // unnamed namespace

WindowBaseWin::WindowBaseWin( Dali::PositionSize positionSize, Any surface, bool isTransparent )
: mEventHandler(),
  mWin32Window( 0 ),
  mOwnSurface( false ),
  mIsTransparent( false ), // Should only be set to true once we actually create a transparent window regardless of what isTransparent is.
  mRotationAppSet( false )
{
  Initialize( positionSize, surface, isTransparent );
}

WindowBaseWin::~WindowBaseWin()
{
  for( Dali::Vector< Event_Handler >::Iterator iter = mEventHandler.Begin(), endIter = mEventHandler.End(); iter != endIter; ++iter )
  {
    event_handler_del( *iter );
  }
  mEventHandler.Clear();

  WindowsPlatformImplement::PostWinMessage( WM_CLOSE, 0, 0, mWin32Window );
}

static void EventEntry( long hWnd, unsigned int uMsg, long wParam, long lParam )
{
  EventCallback callback = GetCallback( uMsg );
  EventHandler *handler = (EventHandler*)GetEventHandler( uMsg );

  if( NULL != callback )
  {
    //EventHandler *handler = new EventHandler();
    TWinEventInfo eventInfo( hWnd, uMsg, wParam, lParam );
    callback( handler, uMsg, &eventInfo );
  }
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
    mWin32Window = static_cast< Win_Window_Handle >( surfaceId );
  }

  // Register window focus events
  mEventHandler.PushBack( event_handler_add( WIN_EVENT_WINDOW_FOCUS_IN,       EcoreEventWindowFocusIn,   this ) );
  mEventHandler.PushBack( event_handler_add( WIN_EVENT_WINDOW_FOCUS_OUT,      EcoreEventWindowFocusOut,  this ) );

  // Register Window damage events
  mEventHandler.PushBack( event_handler_add( WIN_EVENT_WINDOW_DAMAGE,         EcoreEventWindowDamaged,   this ) );

  // Register Touch events
  mEventHandler.PushBack( event_handler_add( EVENT_MOUSE_BUTTON_DOWN,       EcoreEventMouseButtonDown, this ) );
  mEventHandler.PushBack( event_handler_add( EVENT_MOUSE_BUTTON_UP,         EcoreEventMouseButtonUp,   this ) );
  mEventHandler.PushBack( event_handler_add( EVENT_MOUSE_MOVE,              EcoreEventMouseButtonMove, this ) );
  mEventHandler.PushBack( event_handler_add( EVENT_MOUSE_OUT,               EcoreEventMouseButtonUp,   this ) ); // process mouse out event like up event

  // Register Mouse wheel events
  mEventHandler.PushBack( event_handler_add( EVENT_MOUSE_WHEEL,             EcoreEventMouseWheel,      this ) );

  // Register Key events
  mEventHandler.PushBack( event_handler_add( EVENT_KEY_DOWN,                EcoreEventKeyDown,         this ) );
  mEventHandler.PushBack( event_handler_add( EVENT_KEY_UP,                  EcoreEventKeyUp,           this ) );

  // Register Selection event
  mEventHandler.PushBack( event_handler_add( WIN_EVENT_SELECTION_CLEAR,       EcoreEventSelectionClear,  this ) );
  mEventHandler.PushBack( event_handler_add( WIN_EVENT_SELECTION_NOTIFY,      EcoreEventSelectionNotify, this ) );

  WindowsPlatformImplement::AddListener( EventEntry );
}

bool WindowBaseWin::OnWindowPropertyChanged( void* data, int type, TWinEventInfo *event )
{
  //Ecore_X_Event_Window_Property* propertyChangedEvent = static_cast< Ecore_X_Event_Window_Property* >( event );
  //bool handled( true );

  //if( propertyChangedEvent->win == mWin32Window )
  //{
  //  Ecore_X_Window_State_Hint state( ecore_x_icccm_state_get( propertyChangedEvent->win ) );

  //  switch( state )
  //  {
  //    case ECORE_X_WINDOW_STATE_HINT_WITHDRAWN:
  //    {
  //      // Window was hidden.
  //      mWindow->OnIconifyChanged( true );
  //      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%d) Withdrawn\n", mWindow );
  //      handled = ECORE_CALLBACK_DONE;
  //      break;
  //    }
  //    case ECORE_X_WINDOW_STATE_HINT_ICONIC:
  //    {
  //      // Window was iconified (minimised).
  //      mWindow->OnIconifyChanged( true );
  //      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%d) Iconfied\n", mWindow );
  //      handled = ECORE_CALLBACK_DONE;
  //      break;
  //    }
  //    case ECORE_X_WINDOW_STATE_HINT_NORMAL:
  //    {
  //      // Window was shown.
  //      mWindow->OnIconifyChanged( false );
  //      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%d) Shown\n", mWindow );
  //      handled = ECORE_CALLBACK_DONE;
  //      break;
  //    }
  //    default:
  //    {
  //      // Ignore
  //      break;
  //    }
  //  }
  //}

  //return handled;
  return true;
}

void WindowBaseWin::OnDeleteRequest()
{
  mDeleteRequestSignal.Emit();
}

void WindowBaseWin::OnFocusIn( void* data, int type, TWinEventInfo *event )
{
  //Ecore_X_Event_Window_Focus_In* focusInEvent = static_cast< Ecore_X_Event_Window_Focus_In* >( event );

  //if( focusInEvent->win == mWin32Window )
  //{
  //  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window EcoreEventWindowFocusIn\n" );

  //  mFocusChangedSignal.Emit( true );
  //}
}

void WindowBaseWin::OnFocusOut( void* data, int type, TWinEventInfo *event )
{
  //Ecore_X_Event_Window_Focus_Out* focusOutEvent = static_cast< Ecore_X_Event_Window_Focus_Out* >( event );

  //// If the window loses focus then hide the keyboard.
  //if( focusOutEvent->win == mWin32Window )
  //{
  //  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window EcoreEventWindowFocusOut\n" );

  //  mFocusChangedSignal.Emit( false );
  //}
}

void WindowBaseWin::OnWindowDamaged( void* data, int type, TWinEventInfo *event )
{
  Event_Mouse_Button* windowDamagedEvent( (Event_Mouse_Button*)event );
  EventHandler* handler( (EventHandler*)data );

  if( windowDamagedEvent->window == mWin32Window )
  {
    DamageArea area;
    area.x = 0;
    area.y = 0;
    area.width = 480;
    area.height = 800;

    //handler->SendEvent( area );
    mWindowDamagedSignal.Emit( area );
  }
}

void WindowBaseWin::OnMouseButtonDown( void* data, int type, TWinEventInfo *event )
{
  Event_Mouse_Button touchEvent = *((Event_Mouse_Button*)event);
  touchEvent.x = LOWORD( event->lParam );
  touchEvent.y = HIWORD( event->lParam );
  touchEvent.multi.device = DEVICE_MOUSE;

  if( touchEvent.window == mWin32Window )
  {
    PointState::Type state ( PointState::DOWN );

    // Check if the buttons field is set and ensure it's the primary touch button.
    // If this event was triggered by buttons other than the primary button (used for touch), then
    // just send an interrupted event to Core.
    if( touchEvent.buttons && ( touchEvent.buttons != PRIMARY_TOUCH_BUTTON_ID ) )
    {
      state = PointState::INTERRUPTED;
    }

    Integration::Point point;
    point.SetDeviceId( touchEvent.multi.device );
    point.SetState( state );
    point.SetScreenPosition( Vector2( touchEvent.x, touchEvent.y + WindowsPlatformImplement::GetEdgeHeight() ) );
    point.SetRadius( touchEvent.multi.radius, Vector2( touchEvent.multi.radius_x, touchEvent.multi.radius_y ) );
    point.SetPressure( touchEvent.multi.pressure );
    point.SetAngle( Degree( touchEvent.multi.angle ) );

    mTouchEventSignal.Emit( point, touchEvent.timestamp );
  }
}

void WindowBaseWin::OnMouseButtonUp( void* data, int type, TWinEventInfo *event )
{
  Event_Mouse_Button touchEvent = *( (Event_Mouse_Button*)event );
  touchEvent.x = LOWORD( event->lParam );
  touchEvent.y = HIWORD( event->lParam );
  touchEvent.multi.device = DEVICE_MOUSE;

  if( touchEvent.window == mWin32Window )
  {
    PointState::Type state( PointState::UP );

    // Check if the buttons field is set and ensure it's the primary touch button.
    // If this event was triggered by buttons other than the primary button (used for touch), then
    // just send an interrupted event to Core.
    if( touchEvent.buttons && ( touchEvent.buttons != PRIMARY_TOUCH_BUTTON_ID ) )
    {
      state = PointState::INTERRUPTED;
    }

    Integration::Point point;
    point.SetDeviceId( touchEvent.multi.device );
    point.SetState( state );
    point.SetScreenPosition( Vector2( touchEvent.x, touchEvent.y + WindowsPlatformImplement::GetEdgeHeight() ) );
    point.SetRadius( touchEvent.multi.radius, Vector2( touchEvent.multi.radius_x, touchEvent.multi.radius_y ) );
    point.SetPressure( touchEvent.multi.pressure );
    point.SetAngle( Degree( touchEvent.multi.angle ) );

    mTouchEventSignal.Emit( point, touchEvent.timestamp );
  }
}

void WindowBaseWin::OnMouseButtonMove( void* data, int type, TWinEventInfo *event )
{
  Event_Mouse_Button touchEvent = *((Event_Mouse_Button*)event);
  touchEvent.timestamp = GetTickCount();
  touchEvent.x = LOWORD( event->lParam );
  touchEvent.y = HIWORD( event->lParam );
  touchEvent.multi.device = DEVICE_MOUSE;

  if( touchEvent.window == mWin32Window )
  {
    PointState::Type state( PointState::MOTION );

    // Check if the buttons field is set and ensure it's the primary touch button.
    // If this event was triggered by buttons other than the primary button (used for touch), then
    // just send an interrupted event to Core.
    if( touchEvent.buttons && ( touchEvent.buttons != PRIMARY_TOUCH_BUTTON_ID ) )
    {
      state = PointState::INTERRUPTED;
    }

    Integration::Point point;
    point.SetDeviceId( touchEvent.multi.device );
    point.SetState( state );
    point.SetScreenPosition( Vector2( touchEvent.x, touchEvent.y + WindowsPlatformImplement::GetEdgeHeight() ) );
    point.SetRadius( touchEvent.multi.radius, Vector2( touchEvent.multi.radius_x, touchEvent.multi.radius_y ) );
    point.SetPressure( touchEvent.multi.pressure );
    point.SetAngle( Degree( touchEvent.multi.angle ) );

    mTouchEventSignal.Emit( point, touchEvent.timestamp );
  }
}

void WindowBaseWin::OnMouseWheel( void* data, int type, TWinEventInfo *event )
{
  Event_Mouse_Wheel mouseWheelEvent = *((Event_Mouse_Wheel*)( event ));

  if( mouseWheelEvent.window == mWin32Window )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseWin::OnMouseWheel: direction: %d, modifiers: %d, x: %d, y: %d, z: %d\n", mouseWheelEvent.direction, mouseWheelEvent.modifiers, mouseWheelEvent.x, mouseWheelEvent.y, mouseWheelEvent.z );

    WheelEvent wheelEvent( WheelEvent::MOUSE_WHEEL, mouseWheelEvent.direction, mouseWheelEvent.modifiers, Vector2( mouseWheelEvent.x, mouseWheelEvent.y ), mouseWheelEvent.z, mouseWheelEvent.timestamp );

    mWheelEventSignal.Emit( wheelEvent );
  }
}

void WindowBaseWin::OnKeyDown( void* data, int type, TWinEventInfo *event )
{
  if( event->mWindow == mWin32Window )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseWin::OnKeyDown\n" );

    int keyCode = event->wParam;
    std::string keyName( WindowsPlatformImplement::GetKeyName( keyCode ) );
    std::string key( "" );
    std::string keyString( "" );
    std::string compose( "" );

    int modifier( 0 );
    unsigned long time( 0 );

    // Ensure key event string is not NULL as keys like SHIFT have a null string.
    keyString.push_back( event->wParam );

    Integration::KeyEvent keyEvent( keyName, key, keyString, keyCode, modifier, time, Integration::KeyEvent::Down, compose, DEFAULT_DEVICE_NAME, DEFAULT_DEVICE_CLASS, DEFAULT_DEVICE_SUBCLASS );

    mKeyEventSignal.Emit( keyEvent );
  }
}

void WindowBaseWin::OnKeyUp( void* data, int type, TWinEventInfo *event )
{
  if( event->mWindow == mWin32Window )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseWin::OnKeyDown\n" );

    int keyCode = event->wParam;
    std::string keyName( WindowsPlatformImplement::GetKeyName( keyCode ) );
    std::string key( "" );
    std::string keyString( "" );
    std::string compose( "" );

    int modifier( 0 );
    unsigned long time( 0 );

    // Ensure key event string is not NULL as keys like SHIFT have a null string.
    keyString.push_back( event->wParam );

    Integration::KeyEvent keyEvent( keyName, key, keyString, keyCode, modifier, time, Integration::KeyEvent::Up, compose, DEFAULT_DEVICE_NAME, DEFAULT_DEVICE_CLASS, DEFAULT_DEVICE_SUBCLASS );

    mKeyEventSignal.Emit( keyEvent );
  }
}

void WindowBaseWin::OnSelectionClear( void* data, int type, TWinEventInfo *event )
{
  //Ecore_X_Event_Selection_Clear* selectionClearEvent = static_cast< Ecore_X_Event_Selection_Clear* >( event );

  //if( selectionClearEvent->win == mWin32Window )
  //{
  //  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, " WindowBaseWin::OnSelectionClear\n" );

  //  if( selectionClearEvent->selection == ECORE_X_SELECTION_SECONDARY )
  //  {
  //    // Request to get the content from Ecore.
  //    ecore_x_selection_secondary_request( selectionClearEvent->win, ECORE_X_SELECTION_TARGET_TEXT );
  //  }
  //}
}

void WindowBaseWin::OnSelectionNotify( void* data, int type, TWinEventInfo *event )
{
  //Ecore_X_Event_Selection_Notify* selectionNotifyEvent = static_cast< Ecore_X_Event_Selection_Notify* >( event );

  //if( selectionNotifyEvent->win == mWin32Window )
  //{
  //  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, " WindowBaseWin::OnSelectionNotify\n" );

  //  Ecore_X_Selection_Data* selectionData = static_cast< Ecore_X_Selection_Data* >( selectionNotifyEvent->data );
  //  if( selectionData->data )
  //  {
  //    if( selectionNotifyEvent->selection == ECORE_X_SELECTION_SECONDARY )
  //    {
  //      mSelectionDataReceivedSignal.Emit( event  );
  //    }
  //  }
  //}
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
  //ecore_x_window_move( mWin32Window, positionSize.x, positionSize.y );
}

void WindowBaseWin::Resize( PositionSize positionSize )
{
  //ecore_x_window_resize( mWin32Window, positionSize.width, positionSize.height );
}

void WindowBaseWin::MoveResize( PositionSize positionSize )
{
  //ecore_x_window_move_resize( mWin32Window, positionSize.x, positionSize.y, positionSize.width, positionSize.height );
}

void WindowBaseWin::ShowIndicator( Dali::Window::IndicatorVisibleMode visibleMode, Dali::Window::IndicatorBgOpacity opacityMode )
{
//  DALI_LOG_TRACE_METHOD_FMT( gWindowBaseLogFilter, "visible : %d\n", visibleMode );
//
//  if( visibleMode == Dali::Window::VISIBLE )
//  {
//    // when the indicator is visible, set proper mode for indicator server according to bg mode
//    if( opacityMode == Dali::Window::OPAQUE )
//    {
//      ecore_x_e_illume_indicator_opacity_set( mWin32Window, ECORE_X_ILLUME_INDICATOR_OPAQUE );
//    }
//    else if( opacityMode == Dali::Window::TRANSLUCENT )
//    {
//      ecore_x_e_illume_indicator_opacity_set( mWin32Window, ECORE_X_ILLUME_INDICATOR_TRANSLUCENT );
//    }
//#if defined (DALI_PROFILE_MOBILE)
//    else if( opacityMode == Dali::Window::TRANSPARENT )
//    {
//      ecore_x_e_illume_indicator_opacity_set( mWin32Window, ECORE_X_ILLUME_INDICATOR_OPAQUE );
//    }
//#endif
//  }
//  else
//  {
//    // when the indicator is not visible, set TRANSPARENT mode for indicator server
//    ecore_x_e_illume_indicator_opacity_set( mWin32Window, ECORE_X_ILLUME_INDICATOR_TRANSPARENT ); // it means hidden indicator
//  }
}

void WindowBaseWin::SetIndicatorProperties( bool isShow, Dali::Window::WindowOrientation lastOrientation )
{
  /*int show_state = static_cast< int >( isShow );
  ecore_x_window_prop_property_set( mWin32Window, ECORE_X_ATOM_E_ILLUME_INDICATOR_STATE,
                                    ECORE_X_ATOM_CARDINAL, 32, &show_state, 1 );

  if( isShow )
  {
    ecore_x_e_illume_indicator_state_set( mWin32Window, ECORE_X_ILLUME_INDICATOR_STATE_ON );
  }
  else
  {
    ecore_x_e_illume_indicator_state_set( mWin32Window, ECORE_X_ILLUME_INDICATOR_STATE_OFF );
  }*/
}

void WindowBaseWin::IndicatorTypeChanged( IndicatorInterface::Type type )
{
}

void WindowBaseWin::SetClass( const std::string& name, const std::string& className )
{
  //ecore_x_icccm_name_class_set( mWin32Window, name.c_str(), className.c_str() );
}

void WindowBaseWin::Raise()
{
  //ecore_x_window_raise( mWin32Window );
}

void WindowBaseWin::Lower()
{
  //ecore_x_window_lower( mWin32Window );
}

void WindowBaseWin::Activate()
{
  //ecore_x_netwm_client_active_request( ecore_x_window_root_get( mWin32Window ), mWin32Window, 1 /* request type, 1:application, 2:pager */, 0 );
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
  //ecore_x_window_show( mWin32Window );
}

void WindowBaseWin::Hide()
{
  //ecore_x_window_hide( mWin32Window );
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
  WindowsPlatformImplement::GetDPI( xres, yres );

  xres *= 1.5;
  yres *= 1.5;

  dpiHorizontal = int( xres + 0.5f );  // rounding
  dpiVertical = int( yres + 0.5f );
}

void WindowBaseWin::SetViewMode( ViewMode viewMode )
{
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
    DALI_ASSERT_ALWAYS( ( (surface.GetType() == typeid ( winWindow ) ) || (surface.GetType() == typeid ( Win_Window_Handle ) ) )
                        && "Surface type is invalid" );

    if ( surface.GetType() == typeid ( Win_Window_Handle ) )
    {
      surfaceId = AnyCast< Win_Window_Handle >( surface );
    }
    else
    {
      surfaceId = AnyCast< winWindow >( surface );
    }
  }
  return surfaceId;
}

void WindowBaseWin::CreateWinWindow( PositionSize positionSize, bool isTransparent )
{
  long hWnd = WindowsPlatformImplement::CreateHwnd( "Demo", "Demo", positionSize.x, positionSize.y, positionSize.width, positionSize.height, NULL );

  WindowsPlatformImplement::ShowWindow( hWnd );

  mWin32Window = (Win_Window_Handle)hWnd;
  DALI_ASSERT_ALWAYS( mWin32Window != 0 && "There is no EcoreWin window" );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
