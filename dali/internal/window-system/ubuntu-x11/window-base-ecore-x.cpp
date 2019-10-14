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
#include <dali/internal/window-system/ubuntu-x11/window-base-ecore-x.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/ubuntu-x11/ecore-x-types.h>

// EXTERNAL_HEADERS
#include <dali/public-api/object/any.h>
#include <dali/public-api/events/mouse-button.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/input/ubuntu-x11/dali-ecore-input.h>

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

/////////////////////////////////////////////////////////////////////////////////////////////////
// Window Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

static Eina_Bool EcoreEventWindowPropertyChanged( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    return windowBase->OnWindowPropertyChanged( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when the window receives a delete request
 */
static Eina_Bool EcoreEventWindowDeleteRequest( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    windowBase->OnDeleteRequest();
  }
  return ECORE_CALLBACK_DONE;
}

/**
 * Called when the window gains focus.
 */
static Eina_Bool EcoreEventWindowFocusIn( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    windowBase->OnFocusIn( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when the window loses focus.
 */
static Eina_Bool EcoreEventWindowFocusOut( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    windowBase->OnFocusOut( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when the window is damaged.
 */
static Eina_Bool EcoreEventWindowDamaged( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    windowBase->OnWindowDamaged( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Selection Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when the source window notifies us the content in clipboard is selected.
 */
static Eina_Bool EcoreEventSelectionClear( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    windowBase->OnSelectionClear( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when the source window sends us about the selected content.
 * For example, when dragged items are dragged INTO our window or when items are selected in the clipboard.
 */
static Eina_Bool EcoreEventSelectionNotify( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    windowBase->OnSelectionNotify( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Touch Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a touch down is received.
 */
static Eina_Bool EcoreEventMouseButtonDown( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    windowBase->OnMouseButtonDown( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a touch up is received.
 */
static Eina_Bool EcoreEventMouseButtonUp( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    windowBase->OnMouseButtonUp( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a touch motion is received.
 */
static Eina_Bool EcoreEventMouseButtonMove( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    windowBase->OnMouseButtonMove( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Wheel Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a mouse wheel is received.
 */
static Eina_Bool EcoreEventMouseWheel( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    windowBase->OnMouseWheel( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Key Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a key down is received.
 */
static Eina_Bool EcoreEventKeyDown( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    windowBase->OnKeyDown( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a key up is received.
 */
static Eina_Bool EcoreEventKeyUp( void* data, int type, void* event )
{
  WindowBaseEcoreX* windowBase = static_cast< WindowBaseEcoreX* >( data );
  if( windowBase )
  {
    windowBase->OnKeyUp( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

} // unnamed namespace

WindowBaseEcoreX::WindowBaseEcoreX( Dali::PositionSize positionSize, Any surface, bool isTransparent )
: mEcoreEventHandler(),
  mEcoreWindow( 0 ),
  mOwnSurface( false ),
  mIsTransparent( false ), // Should only be set to true once we actually create a transparent window regardless of what isTransparent is.
  mRotationAppSet( false )
{
  Initialize( positionSize, surface, isTransparent );
}

WindowBaseEcoreX::~WindowBaseEcoreX()
{
  for( Dali::Vector< Ecore_Event_Handler* >::Iterator iter = mEcoreEventHandler.Begin(), endIter = mEcoreEventHandler.End(); iter != endIter; ++iter )
  {
    ecore_event_handler_del( *iter );
  }
  mEcoreEventHandler.Clear();

  if( mOwnSurface )
  {
    ecore_x_window_free( mEcoreWindow );
  }
}

void WindowBaseEcoreX::Initialize( PositionSize positionSize, Any surface, bool isTransparent )
{
  // see if there is a surface in Any surface
  unsigned int surfaceId = GetSurfaceId( surface );

  // if the surface is empty, create a new one.
  if( surfaceId == 0 )
  {
    // we own the surface about to created
    mOwnSurface = true;
    CreateWindow( positionSize, isTransparent );
  }
  else
  {
    // XLib should already be initialized so no point in calling XInitThreads
    mEcoreWindow = static_cast< Ecore_X_Window >( surfaceId );
  }

  // set up etc properties to match with ecore-evas
  char *id = NULL;
  if( ( id = getenv("DESKTOP_STARTUP_ID") ) )
  {
    ecore_x_netwm_startup_id_set( mEcoreWindow, id );
  }

  ecore_x_icccm_hints_set( mEcoreWindow,
                           1,                                // accepts_focus
                           ECORE_X_WINDOW_STATE_HINT_NORMAL, // initial_state
                           0,                                // icon_pixmap
                           0,                                // icon_mask
                           0,                                // icon_window
                           0,                                // window_group
                           0 );                              // is_urgent

  // we SHOULD guarantee the x11 window was created in x server.
  ecore_x_sync();

  ecore_x_input_multi_select( mEcoreWindow );

  // This ensures that we catch the window close (or delete) request
  ecore_x_icccm_protocol_set( mEcoreWindow, ECORE_X_WM_PROTOCOL_DELETE_REQUEST, EINA_TRUE );

  // Enable Drag & Drop
  ecore_x_dnd_aware_set( mEcoreWindow, EINA_TRUE );

  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_X_EVENT_WINDOW_PROPERTY,       EcoreEventWindowPropertyChanged, this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_X_EVENT_WINDOW_DELETE_REQUEST, EcoreEventWindowDeleteRequest,   this ) );

  // Register window focus events
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_X_EVENT_WINDOW_FOCUS_IN,       EcoreEventWindowFocusIn,   this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_X_EVENT_WINDOW_FOCUS_OUT,      EcoreEventWindowFocusOut,  this ) );

  // Register Window damage events
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_X_EVENT_WINDOW_DAMAGE,         EcoreEventWindowDamaged,   this ) );

  // Register Touch events
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_BUTTON_DOWN,       EcoreEventMouseButtonDown, this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_BUTTON_UP,         EcoreEventMouseButtonUp,   this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_MOVE,              EcoreEventMouseButtonMove, this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_OUT,               EcoreEventMouseButtonUp,   this ) ); // process mouse out event like up event

  // Register Mouse wheel events
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_WHEEL,             EcoreEventMouseWheel,      this ) );

  // Register Key events
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_KEY_DOWN,                EcoreEventKeyDown,         this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_KEY_UP,                  EcoreEventKeyUp,           this ) );

  // Register Selection event
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_X_EVENT_SELECTION_CLEAR,       EcoreEventSelectionClear,  this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_X_EVENT_SELECTION_NOTIFY,      EcoreEventSelectionNotify, this ) );
}

Eina_Bool WindowBaseEcoreX::OnWindowPropertyChanged( void* data, int type, void* event )
{
  Ecore_X_Event_Window_Property* propertyChangedEvent = static_cast< Ecore_X_Event_Window_Property* >( event );
  Eina_Bool handled( ECORE_CALLBACK_PASS_ON );

  if( propertyChangedEvent->win == mEcoreWindow )
  {
    Ecore_X_Window_State_Hint state( ecore_x_icccm_state_get( propertyChangedEvent->win ) );

    switch( state )
    {
      case ECORE_X_WINDOW_STATE_HINT_WITHDRAWN:
      {
        // Window was hidden.
        mIconifyChangedSignal.Emit( true );
        handled = ECORE_CALLBACK_DONE;
        break;
      }
      case ECORE_X_WINDOW_STATE_HINT_ICONIC:
      {
        // Window was iconified (minimised).
        mIconifyChangedSignal.Emit( true );
        handled = ECORE_CALLBACK_DONE;
        break;
      }
      case ECORE_X_WINDOW_STATE_HINT_NORMAL:
      {
        // Window was shown.
        mIconifyChangedSignal.Emit( false );
        handled = ECORE_CALLBACK_DONE;
        break;
      }
      default:
      {
        // Ignore
        break;
      }
    }
  }

  return handled;
}

void WindowBaseEcoreX::OnDeleteRequest()
{
  mDeleteRequestSignal.Emit();
}

void WindowBaseEcoreX::OnFocusIn( void* data, int type, void* event )
{
  Ecore_X_Event_Window_Focus_In* focusInEvent = static_cast< Ecore_X_Event_Window_Focus_In* >( event );

  if( focusInEvent->win == mEcoreWindow )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window EcoreEventWindowFocusIn\n" );

    mFocusChangedSignal.Emit( true );
  }
}

void WindowBaseEcoreX::OnFocusOut( void* data, int type, void* event )
{
  Ecore_X_Event_Window_Focus_Out* focusOutEvent = static_cast< Ecore_X_Event_Window_Focus_Out* >( event );

  // If the window loses focus then hide the keyboard.
  if( focusOutEvent->win == mEcoreWindow )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window EcoreEventWindowFocusOut\n" );

    mFocusChangedSignal.Emit( false );
  }
}

void WindowBaseEcoreX::OnWindowDamaged( void* data, int type, void* event )
{
  Ecore_X_Event_Window_Damage* windowDamagedEvent = static_cast< Ecore_X_Event_Window_Damage* >( event );

  if( windowDamagedEvent->win == mEcoreWindow )
  {
    DamageArea area;
    area.x = windowDamagedEvent->x;
    area.y = windowDamagedEvent->y;
    area.width = windowDamagedEvent->w;
    area.height = windowDamagedEvent->h;

    mWindowDamagedSignal.Emit( area );
  }
}

void WindowBaseEcoreX::OnMouseButtonDown( void* data, int type, void* event )
{
  Ecore_Event_Mouse_Button* touchEvent = static_cast< Ecore_Event_Mouse_Button* >( event );

  if( touchEvent->window == mEcoreWindow )
  {
    PointState::Type state ( PointState::DOWN );

    Integration::Point point;
    point.SetDeviceId( touchEvent->multi.device );
    point.SetState( state );
    point.SetScreenPosition( Vector2( touchEvent->x, touchEvent->y ) );
    point.SetRadius( touchEvent->multi.radius, Vector2( touchEvent->multi.radius_x, touchEvent->multi.radius_y ) );
    point.SetPressure( touchEvent->multi.pressure );
    point.SetAngle( Degree( touchEvent->multi.angle ) );
    if( touchEvent->buttons)
    {
      point.SetMouseButton( static_cast< MouseButton::Type >( touchEvent->buttons) );
    }

    mTouchEventSignal.Emit( point, touchEvent->timestamp );
  }
}

void WindowBaseEcoreX::OnMouseButtonUp( void* data, int type, void* event )
{
  Ecore_Event_Mouse_Button* touchEvent = static_cast< Ecore_Event_Mouse_Button* >( event );

  if( touchEvent->window == mEcoreWindow )
  {
    Integration::Point point;
    point.SetDeviceId( touchEvent->multi.device );
    point.SetState( PointState::UP );
    point.SetScreenPosition( Vector2( touchEvent->x, touchEvent->y ) );
    point.SetRadius( touchEvent->multi.radius, Vector2( touchEvent->multi.radius_x, touchEvent->multi.radius_y ) );
    point.SetPressure( touchEvent->multi.pressure );
    point.SetAngle( Degree( static_cast<float>( touchEvent->multi.angle ) ) );
    if( touchEvent->buttons)
    {
      point.SetMouseButton( static_cast< MouseButton::Type >( touchEvent->buttons) );
    }

    mTouchEventSignal.Emit( point, touchEvent->timestamp );
  }
}

void WindowBaseEcoreX::OnMouseButtonMove( void* data, int type, void* event )
{
  Ecore_Event_Mouse_Move* touchEvent = static_cast< Ecore_Event_Mouse_Move* >( event );

  if( touchEvent->window == mEcoreWindow )
  {
    Integration::Point point;
    point.SetDeviceId( touchEvent->multi.device );
    point.SetState( PointState::MOTION );
    point.SetScreenPosition( Vector2( static_cast<float>( touchEvent->x ), static_cast<float>( touchEvent->y ) ) );
    point.SetRadius( static_cast<float>( touchEvent->multi.radius ), Vector2( static_cast<float>( touchEvent->multi.radius_x ), static_cast<float>( touchEvent->multi.radius_y ) ) );
    point.SetPressure( static_cast<float>( touchEvent->multi.pressure ) );
    point.SetAngle( Degree( static_cast<float>( touchEvent->multi.angle ) ) );

    mTouchEventSignal.Emit( point, touchEvent->timestamp );
  }
}

void WindowBaseEcoreX::OnMouseWheel( void* data, int type, void* event )
{
  Ecore_Event_Mouse_Wheel* mouseWheelEvent = static_cast< Ecore_Event_Mouse_Wheel* >( event );

  if( mouseWheelEvent->window == mEcoreWindow )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreX::OnMouseWheel: direction: %d, modifiers: %d, x: %d, y: %d, z: %d\n", mouseWheelEvent->direction, mouseWheelEvent->modifiers, mouseWheelEvent->x, mouseWheelEvent->y, mouseWheelEvent->z );

    WheelEvent wheelEvent( WheelEvent::MOUSE_WHEEL, mouseWheelEvent->direction, mouseWheelEvent->modifiers, Vector2( static_cast<float>( mouseWheelEvent->x ), static_cast<float>( mouseWheelEvent->y ) ), mouseWheelEvent->z, mouseWheelEvent->timestamp );

    mWheelEventSignal.Emit( wheelEvent );
  }
}

void WindowBaseEcoreX::OnKeyDown( void* data, int type, void* event )
{
  Ecore_Event_Key* keyEvent = static_cast< Ecore_Event_Key* >( event );

  if( keyEvent->window == mEcoreWindow )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreX::OnKeyDown\n" );

    std::string keyName( keyEvent->keyname );
    std::string logicalKey( "" );
    std::string keyString( "" );
    std::string compose( "" );

    // Ensure key compose string is not NULL as keys like SHIFT or arrow have a null string.
    if( keyEvent->compose )
    {
      compose = keyEvent->compose;
    }

    // Ensure key symbol is not NULL as keys like SHIFT have a null string.
    if( keyEvent->key )
    {
      logicalKey = keyEvent->key;
    }

    int keyCode = ecore_x_keysym_keycode_get( keyEvent->keyname );
    int modifier( keyEvent->modifiers );
    unsigned long time = keyEvent->timestamp;

    // Ensure key event string is not NULL as keys like SHIFT have a null string.
    if( keyEvent->string )
    {
      keyString = keyEvent->string;
    }

    Integration::KeyEvent keyEvent( keyName, logicalKey, keyString, keyCode, modifier, time, Integration::KeyEvent::Down, compose, DEFAULT_DEVICE_NAME, DEFAULT_DEVICE_CLASS, DEFAULT_DEVICE_SUBCLASS );

    mKeyEventSignal.Emit( keyEvent );
  }
}

void WindowBaseEcoreX::OnKeyUp( void* data, int type, void* event )
{
  Ecore_Event_Key* keyEvent = static_cast< Ecore_Event_Key* >( event );

  if ( keyEvent->window == mEcoreWindow )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, " WindowBaseEcoreX::OnKeyUp\n" );

    std::string keyName( keyEvent->keyname );
    std::string logicalKey( "" );
    std::string keyString( "" );
    std::string compose( "" );

    // Ensure key compose string is not NULL as keys like SHIFT or arrow have a null string.
    if( keyEvent->compose )
    {
      compose = keyEvent->compose;
    }
    // Ensure key symbol is not NULL as keys like SHIFT have a null string.
    if( keyEvent->key )
    {
      logicalKey = keyEvent->key;
    }

    int keyCode = ecore_x_keysym_keycode_get( keyEvent->keyname );
    int modifier( keyEvent->modifiers );
    unsigned long time( keyEvent->timestamp );

    // Ensure key event string is not NULL as keys like SHIFT have a null string.
    if( keyEvent->string )
    {
      keyString = keyEvent->string;
    }

    Integration::KeyEvent keyEvent( keyName, logicalKey, keyString, keyCode, modifier, time, Integration::KeyEvent::Up, compose, DEFAULT_DEVICE_NAME, DEFAULT_DEVICE_CLASS, DEFAULT_DEVICE_SUBCLASS );

    mKeyEventSignal.Emit( keyEvent );
  }
}

void WindowBaseEcoreX::OnSelectionClear( void* data, int type, void* event )
{
  Ecore_X_Event_Selection_Clear* selectionClearEvent = static_cast< Ecore_X_Event_Selection_Clear* >( event );

  if( selectionClearEvent->win == mEcoreWindow )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, " WindowBaseEcoreX::OnSelectionClear\n" );

    if( selectionClearEvent->selection == ECORE_X_SELECTION_SECONDARY )
    {
      // Request to get the content from Ecore.
      ecore_x_selection_secondary_request( selectionClearEvent->win, ECORE_X_SELECTION_TARGET_TEXT );
    }
  }
}

void WindowBaseEcoreX::OnSelectionNotify( void* data, int type, void* event )
{
  Ecore_X_Event_Selection_Notify* selectionNotifyEvent = static_cast< Ecore_X_Event_Selection_Notify* >( event );

  if( selectionNotifyEvent->win == mEcoreWindow )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, " WindowBaseEcoreX::OnSelectionNotify\n" );

    Ecore_X_Selection_Data* selectionData = static_cast< Ecore_X_Selection_Data* >( selectionNotifyEvent->data );
    if( selectionData->data )
    {
      if( selectionNotifyEvent->selection == ECORE_X_SELECTION_SECONDARY )
      {
        mSelectionDataReceivedSignal.Emit( event  );
      }
    }
  }
}

Any WindowBaseEcoreX::GetNativeWindow()
{
  return mEcoreWindow;
}

int WindowBaseEcoreX::GetNativeWindowId()
{
  return mEcoreWindow;
}

EGLNativeWindowType WindowBaseEcoreX::CreateEglWindow( int width, int height )
{
  // need to create X handle as in 64bit system ECore handle is 32 bit whereas EGLnative and XWindow are 64 bit
  XWindow window( mEcoreWindow );
  return reinterpret_cast< EGLNativeWindowType >( window );
}

void WindowBaseEcoreX::DestroyEglWindow()
{
}

void WindowBaseEcoreX::SetEglWindowRotation( int angle )
{
}

void WindowBaseEcoreX::SetEglWindowBufferTransform( int angle )
{
}

void WindowBaseEcoreX::SetEglWindowTransform( int angle )
{
}

void WindowBaseEcoreX::ResizeEglWindow( PositionSize positionSize )
{
}

bool WindowBaseEcoreX::IsEglWindowRotationSupported()
{
  return false;
}

void WindowBaseEcoreX::Move( PositionSize positionSize )
{
  ecore_x_window_move( mEcoreWindow, positionSize.x, positionSize.y );
}

void WindowBaseEcoreX::Resize( PositionSize positionSize )
{
  ecore_x_window_resize( mEcoreWindow, positionSize.width, positionSize.height );
}

void WindowBaseEcoreX::MoveResize( PositionSize positionSize )
{
  ecore_x_window_move_resize( mEcoreWindow, positionSize.x, positionSize.y, positionSize.width, positionSize.height );
}

void WindowBaseEcoreX::SetClass( const std::string& name, const std::string& className )
{
  ecore_x_icccm_title_set( mEcoreWindow, name.c_str() );
  ecore_x_netwm_name_set( mEcoreWindow, name.c_str() );
  ecore_x_icccm_name_class_set( mEcoreWindow, name.c_str(), className.c_str() );
}

void WindowBaseEcoreX::Raise()
{
  ecore_x_window_raise( mEcoreWindow );
}

void WindowBaseEcoreX::Lower()
{
  ecore_x_window_lower( mEcoreWindow );
}

void WindowBaseEcoreX::Activate()
{
  ecore_x_netwm_client_active_request( ecore_x_window_root_get( mEcoreWindow ), mEcoreWindow, 1 /* request type, 1:application, 2:pager */, 0 );
}

void WindowBaseEcoreX::SetAvailableOrientations( const std::vector< Dali::Window::WindowOrientation >& orientations )
{
}

void WindowBaseEcoreX::SetPreferredOrientation( Dali::Window::WindowOrientation orientation )
{
}

void WindowBaseEcoreX::SetAcceptFocus( bool accept )
{
}

void WindowBaseEcoreX::Show()
{
  ecore_x_window_show( mEcoreWindow );
}

void WindowBaseEcoreX::Hide()
{
  ecore_x_window_hide( mEcoreWindow );
}

unsigned int WindowBaseEcoreX::GetSupportedAuxiliaryHintCount() const
{
  return 0;
}

std::string WindowBaseEcoreX::GetSupportedAuxiliaryHint( unsigned int index ) const
{
  return std::string();
}

unsigned int WindowBaseEcoreX::AddAuxiliaryHint( const std::string& hint, const std::string& value )
{
  return 0;
}

bool WindowBaseEcoreX::RemoveAuxiliaryHint( unsigned int id )
{
  return false;
}

bool WindowBaseEcoreX::SetAuxiliaryHintValue( unsigned int id, const std::string& value )
{
  return false;
}

std::string WindowBaseEcoreX::GetAuxiliaryHintValue( unsigned int id ) const
{
  return std::string();
}

unsigned int WindowBaseEcoreX::GetAuxiliaryHintId( const std::string& hint ) const
{
  return 0;
}

void WindowBaseEcoreX::SetInputRegion( const Rect< int >& inputRegion )
{
}

void WindowBaseEcoreX::SetType( Dali::Window::Type type )
{
}

bool WindowBaseEcoreX::SetNotificationLevel( Dali::Window::NotificationLevel::Type level )
{
  return false;
}

Dali::Window::NotificationLevel::Type WindowBaseEcoreX::GetNotificationLevel() const
{
  return Dali::Window::NotificationLevel::NONE;
}

void WindowBaseEcoreX::SetOpaqueState( bool opaque )
{
}

bool WindowBaseEcoreX::SetScreenOffMode(Dali::Window::ScreenOffMode::Type screenOffMode)
{
  return false;
}

Dali::Window::ScreenOffMode::Type WindowBaseEcoreX::GetScreenOffMode() const
{
  return Dali::Window::ScreenOffMode::TIMEOUT;
}

bool WindowBaseEcoreX::SetBrightness( int brightness )
{
  return false;
}

int WindowBaseEcoreX::GetBrightness() const
{
  return 0;
}

bool WindowBaseEcoreX::GrabKey( Dali::KEY key, KeyGrab::KeyGrabMode grabMode )
{
  return false;
}

bool WindowBaseEcoreX::UngrabKey( Dali::KEY key )
{
  return false;
}

bool WindowBaseEcoreX::GrabKeyList( const Dali::Vector< Dali::KEY >& key, const Dali::Vector< KeyGrab::KeyGrabMode >& grabMode, Dali::Vector< bool >& result )
{
  return false;
}

bool WindowBaseEcoreX::UngrabKeyList( const Dali::Vector< Dali::KEY >& key, Dali::Vector< bool >& result )
{
  return false;
}

void WindowBaseEcoreX::GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical )
{
  // 1 inch = 25.4 millimeters
  // ecore does not account for differing DPI in the x and y axes, so only get for x is available

  dpiHorizontal = ecore_x_dpi_get();
  dpiVertical   = ecore_x_dpi_get();
}

int WindowBaseEcoreX::GetScreenRotationAngle()
{
  return 0;
}

void WindowBaseEcoreX::SetWindowRotationAngle( int degree )
{
}

void WindowBaseEcoreX::WindowRotationCompleted( int degree, int width, int height )
{
}

void WindowBaseEcoreX::SetTransparency( bool transparent )
{
}

unsigned int WindowBaseEcoreX::GetSurfaceId( Any surface ) const
{
  unsigned int surfaceId = 0;

  if ( surface.Empty() == false )
  {
    // check we have a valid type
    DALI_ASSERT_ALWAYS( ( (surface.GetType() == typeid (XWindow) ) || (surface.GetType() == typeid (Ecore_X_Window) ) )
                        && "Surface type is invalid" );

    if ( surface.GetType() == typeid (Ecore_X_Window) )
    {
      surfaceId = AnyCast< Ecore_X_Window >( surface );
    }
    else
    {
      surfaceId = static_cast<unsigned int>( AnyCast< XWindow >( surface ) );
    }
  }
  return surfaceId;
}

void WindowBaseEcoreX::CreateWindow( PositionSize positionSize, bool isTransparent )
{
 if( isTransparent )
 {
   // create 32 bit window
   mEcoreWindow = ecore_x_window_argb_new( 0, positionSize.x, positionSize.y, positionSize.width, positionSize.height );
   mIsTransparent = true;
 }
 else
 {
   // create 24 bit window
   mEcoreWindow = ecore_x_window_new( 0, positionSize.x, positionSize.y, positionSize.width, positionSize.height );
 }

 if ( mEcoreWindow == 0 )
 {
   DALI_ASSERT_ALWAYS( 0 && "Failed to create X window" );
 }
}

void WindowBaseEcoreX::SetParent( WindowBase* parentWinBase )
{
  Ecore_X_Window ecoreParent = 0;
  if( parentWinBase )
  {
    WindowBaseEcoreX* winBaseEcoreX = static_cast<WindowBaseEcoreX*>( parentWinBase );
    ecoreParent = winBaseEcoreX->mEcoreWindow;
    ecore_x_icccm_transient_for_set( mEcoreWindow, ecoreParent );
  }
  else
  {
    ecoreParent = 0;
    ecore_x_icccm_transient_for_unset( mEcoreWindow );
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
