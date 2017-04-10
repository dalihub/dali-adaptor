/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <events/event-handler.h>

// EXTERNAL INCLUDES
#include <Ecore.h>
#include <Ecore_Input.h>
#include <ecore-wl-render-surface.h>
#include <cstring>

#include <sys/time.h>

#ifndef DALI_PROFILE_UBUNTU
#include <vconf.h>
#include <vconf-keys.h>
#endif // DALI_PROFILE_UBUNTU

#ifdef DALI_ELDBUS_AVAILABLE
#include <Eldbus.h>
#endif // DALI_ELDBUS_AVAILABLE

#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/events/touch-point.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/events/wheel-event.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/events/hover-event-integ.h>
#include <dali/integration-api/events/wheel-event-integ.h>

// INTERNAL INCLUDES
#include <events/gesture-manager.h>
#include <window-render-surface.h>
#include <clipboard-impl.h>
#include <key-impl.h>
#include <physical-keyboard-impl.h>
#include <style-monitor-impl.h>
#include <base/core-event-interface.h>
#include <virtual-keyboard.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

#if defined(DEBUG_ENABLED)
namespace
{
Integration::Log::Filter* gTouchEventLogFilter  = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_TOUCH");
Integration::Log::Filter* gDragAndDropLogFilter = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_DND");
Integration::Log::Filter* gImfLogging  = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_IMF");
Integration::Log::Filter* gSelectionEventLogFilter = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_SELECTION");
} // unnamed namespace
#endif


namespace
{

// DBUS accessibility
const char* BUS = "org.enlightenment.wm-screen-reader";
const char* INTERFACE = "org.tizen.GestureNavigation";
const char* PATH = "/org/tizen/GestureNavigation";

const unsigned int PRIMARY_TOUCH_BUTTON_ID( 1 );

const unsigned int BYTES_PER_CHARACTER_FOR_ATTRIBUTES = 3;

/**
 * Ecore_Event_Modifier enums in Ecore_Input.h do not match Ecore_IMF_Keyboard_Modifiers in Ecore_IMF.h.
 * This function converts from Ecore_Event_Modifier to Ecore_IMF_Keyboard_Modifiers enums.
 * @param[in] ecoreModifier the Ecore_Event_Modifier input.
 * @return the Ecore_IMF_Keyboard_Modifiers output.
 */
Ecore_IMF_Keyboard_Modifiers EcoreInputModifierToEcoreIMFModifier(unsigned int ecoreModifier)
{
   int modifier( ECORE_IMF_KEYBOARD_MODIFIER_NONE );  // If no other matches returns NONE.


   if ( ecoreModifier & ECORE_EVENT_MODIFIER_SHIFT )  // enums from ecore_input/Ecore_Input.h
   {
     modifier |= ECORE_IMF_KEYBOARD_MODIFIER_SHIFT;  // enums from ecore_imf/ecore_imf.h
   }

   if ( ecoreModifier & ECORE_EVENT_MODIFIER_ALT )
   {
     modifier |= ECORE_IMF_KEYBOARD_MODIFIER_ALT;
   }

   if ( ecoreModifier & ECORE_EVENT_MODIFIER_CTRL )
   {
     modifier |= ECORE_IMF_KEYBOARD_MODIFIER_CTRL;
   }

   if ( ecoreModifier & ECORE_EVENT_MODIFIER_WIN )
   {
     modifier |= ECORE_IMF_KEYBOARD_MODIFIER_WIN;
   }

   if ( ecoreModifier & ECORE_EVENT_MODIFIER_ALTGR )
   {
     modifier |= ECORE_IMF_KEYBOARD_MODIFIER_ALTGR;
   }

   return static_cast<Ecore_IMF_Keyboard_Modifiers>( modifier );
}


// Copied from x server
static unsigned int GetCurrentMilliSeconds(void)
{
  struct timeval tv;

  struct timespec tp;
  static clockid_t clockid;

  if (!clockid)
  {
#ifdef CLOCK_MONOTONIC_COARSE
    if (clock_getres(CLOCK_MONOTONIC_COARSE, &tp) == 0 &&
      (tp.tv_nsec / 1000) <= 1000 && clock_gettime(CLOCK_MONOTONIC_COARSE, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC_COARSE;
    }
    else
#endif
    if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC;
    }
    else
    {
      clockid = ~0L;
    }
  }
  if (clockid != ~0L && clock_gettime(clockid, &tp) == 0)
  {
    return (tp.tv_sec * 1000) + (tp.tv_nsec / 1000000L);
  }

  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

#ifndef DALI_PROFILE_UBUNTU
const char * DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE = "db/setting/accessibility/font_name";  // It will be update at vconf-key.h and replaced.
#endif // DALI_PROFILE_UBUNTU

} // unnamed namespace

// Impl to hide EFL implementation.
struct EventHandler::Impl
{
  // Construction & Destruction

  /**
   * Constructor
   */
  Impl( EventHandler* handler, Ecore_Wl_Window* window )
  : mHandler( handler ),
    mEcoreEventHandler(),
    mWindow( window )
#ifdef DALI_ELDBUS_AVAILABLE
  , mSystemConnection( NULL )
#endif // DALI_ELDBUS_AVAILABLE
  {
    // Only register for touch and key events if we have a window
    if ( window != 0 )
    {
      // Register Touch events
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_MOUSE_BUTTON_DOWN,  EcoreEventMouseButtonDown, handler ) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_MOUSE_BUTTON_UP,    EcoreEventMouseButtonUp,   handler ) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_MOUSE_MOVE,         EcoreEventMouseButtonMove, handler ) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_MOUSE_OUT,          EcoreEventMouseButtonUp,   handler ) ); // process mouse out event like up event

      // Register Mouse wheel events
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_MOUSE_WHEEL,        EcoreEventMouseWheel,      handler ) );

      // Register Focus events
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WL_EVENT_FOCUS_IN,  EcoreEventWindowFocusIn,   handler ) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WL_EVENT_FOCUS_OUT, EcoreEventWindowFocusOut,  handler ) );

      // Register Key events
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_KEY_DOWN,           EcoreEventKeyDown,         handler ) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_KEY_UP,             EcoreEventKeyUp,           handler ) );

      // Register Selection event - clipboard selection
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WL_EVENT_DATA_SOURCE_SEND, EcoreEventDataSend, handler ) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WL_EVENT_SELECTION_DATA_READY, EcoreEventDataReceive, handler ) );

      // Register Rotate event
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WL_EVENT_WINDOW_ROTATE, EcoreEventRotate, handler) );

      // Register Detent event
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_DETENT_ROTATE, EcoreEventDetent, handler) );

#ifndef DALI_PROFILE_UBUNTU
      // Register Vconf notify - font name and size
      vconf_notify_key_changed( DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, VconfNotifyFontNameChanged, handler );
      vconf_notify_key_changed( VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, VconfNotifyFontSizeChanged, handler );
#endif // DALI_PROFILE_UBUNTU

#ifdef DALI_ELDBUS_AVAILABLE
      // Initialize ElDBus.
      DALI_LOG_INFO( gImfLogging, Debug::General, "Starting DBus Initialization\n" );

      // Pass in handler.
      EcoreElDBusInitialisation( handler );

      DALI_LOG_INFO( gImfLogging, Debug::General, "Finished DBus Initialization\n" );
#endif // DALI_ELDBUS_AVAILABLE
    }
  }

  /**
   * Destructor
   */
  ~Impl()
  {
#ifndef DALI_PROFILE_UBUNTU
    vconf_ignore_key_changed( VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, VconfNotifyFontSizeChanged );
    vconf_ignore_key_changed( DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, VconfNotifyFontNameChanged );
#endif // DALI_PROFILE_UBUNTU

    for( std::vector<Ecore_Event_Handler*>::iterator iter = mEcoreEventHandler.begin(), endIter = mEcoreEventHandler.end(); iter != endIter; ++iter )
    {
      ecore_event_handler_del( *iter );
    }

#ifdef DALI_ELDBUS_AVAILABLE
    // Close down ElDBus connections.
    if( mSystemConnection )
    {
      eldbus_connection_unref( mSystemConnection );
    }
#endif // DALI_ELDBUS_AVAILABLE
  }

  // Static methods

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // Touch Callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * Called when a touch down is received.
   */
  static Eina_Bool EcoreEventMouseButtonDown( void* data, int type, void* event )
  {
    Ecore_Event_Mouse_Button *touchEvent( (Ecore_Event_Mouse_Button*)event );
    EventHandler* handler( (EventHandler*)data );

    if ( touchEvent->window == (unsigned int)ecore_wl_window_id_get(handler->mImpl->mWindow) )
    {
      PointState::Type state ( PointState::DOWN );

      // Check if the buttons field is set and ensure it's the primary touch button.
      // If this event was triggered by buttons other than the primary button (used for touch), then
      // just send an interrupted event to Core.
      if ( touchEvent->buttons && (touchEvent->buttons != PRIMARY_TOUCH_BUTTON_ID ) )
      {
        state = PointState::INTERRUPTED;
      }

      Integration::Point point;
      point.SetDeviceId( touchEvent->multi.device );
      point.SetState( state );
      point.SetScreenPosition( Vector2( touchEvent->x, touchEvent->y ) );
      point.SetRadius( touchEvent->multi.radius, Vector2( touchEvent->multi.radius_x, touchEvent->multi.radius_y ) );
      point.SetPressure( touchEvent->multi.pressure );
      point.SetAngle( Degree( touchEvent->multi.angle ) );
      handler->SendEvent( point, touchEvent->timestamp );
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a touch up is received.
   */
  static Eina_Bool EcoreEventMouseButtonUp( void* data, int type, void* event )
  {
    Ecore_Event_Mouse_Button *touchEvent( (Ecore_Event_Mouse_Button*)event );
    EventHandler* handler( (EventHandler*)data );

    if ( touchEvent->window == (unsigned int)ecore_wl_window_id_get(handler->mImpl->mWindow) )
    {
      Integration::Point point;
      point.SetDeviceId( touchEvent->multi.device );
      point.SetState( PointState::UP );
      point.SetScreenPosition( Vector2( touchEvent->x, touchEvent->y ) );
      point.SetRadius( touchEvent->multi.radius, Vector2( touchEvent->multi.radius_x, touchEvent->multi.radius_y ) );
      point.SetPressure( touchEvent->multi.pressure );
      point.SetAngle( Degree( touchEvent->multi.angle ) );
      handler->SendEvent( point, touchEvent->timestamp );
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a touch up is received.
   */
  static Eina_Bool EcoreEventMouseWheel( void* data, int type, void* event )
  {
    Ecore_Event_Mouse_Wheel *mouseWheelEvent( (Ecore_Event_Mouse_Wheel*)event );

    DALI_LOG_INFO( gImfLogging, Debug::General, "EVENT Ecore_Event_Mouse_Wheel: direction: %d, modifiers: %d, x: %d, y: %d, z: %d\n", mouseWheelEvent->direction, mouseWheelEvent->modifiers, mouseWheelEvent->x, mouseWheelEvent->y, mouseWheelEvent->z);

    EventHandler* handler( (EventHandler*)data );
    if ( mouseWheelEvent->window == (unsigned int)ecore_wl_window_id_get(handler->mImpl->mWindow) )
    {
      WheelEvent wheelEvent( WheelEvent::MOUSE_WHEEL, mouseWheelEvent->direction, mouseWheelEvent->modifiers, Vector2(mouseWheelEvent->x, mouseWheelEvent->y), mouseWheelEvent->z, mouseWheelEvent->timestamp );
      handler->SendWheelEvent( wheelEvent );
    }
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a touch motion is received.
   */
  static Eina_Bool EcoreEventMouseButtonMove( void* data, int type, void* event )
  {
    Ecore_Event_Mouse_Move *touchEvent( (Ecore_Event_Mouse_Move*)event );
    EventHandler* handler( (EventHandler*)data );

    if ( touchEvent->window == (unsigned int)ecore_wl_window_id_get(handler->mImpl->mWindow) )
    {
      Integration::Point point;
      point.SetDeviceId( touchEvent->multi.device );
      point.SetState( PointState::MOTION );
      point.SetScreenPosition( Vector2( touchEvent->x, touchEvent->y ) );
      point.SetRadius( touchEvent->multi.radius, Vector2( touchEvent->multi.radius_x, touchEvent->multi.radius_y ) );
      point.SetPressure( touchEvent->multi.pressure );
      point.SetAngle( Degree( touchEvent->multi.angle ) );
      handler->SendEvent( point, touchEvent->timestamp );
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
    DALI_LOG_INFO( gImfLogging, Debug::General, "EVENT >>EcoreEventKeyDown \n" );

    EventHandler* handler( (EventHandler*)data );
    Ecore_Event_Key *keyEvent( (Ecore_Event_Key*)event );
    bool eventHandled( false );

    // If a device key then skip ecore_imf_context_filter_event.
    if ( ! KeyLookup::IsDeviceButton( keyEvent->keyname ) )
    {
      Ecore_IMF_Context* imfContext = NULL;
      Dali::ImfManager imfManager( ImfManager::Get() );
      if ( imfManager )
      {
        imfContext = ImfManager::GetImplementation( imfManager ).GetContext();
      }

      if ( imfContext )
      {
        // We're consuming key down event so we have to pass to IMF so that it can parse it as well.
        Ecore_IMF_Event_Key_Down ecoreKeyDownEvent;
        ecoreKeyDownEvent.keyname   = keyEvent->keyname;
        ecoreKeyDownEvent.key       = keyEvent->key;
        ecoreKeyDownEvent.string    = keyEvent->string;
        ecoreKeyDownEvent.compose   = keyEvent->compose;
        ecoreKeyDownEvent.timestamp = keyEvent->timestamp;
        ecoreKeyDownEvent.modifiers = EcoreInputModifierToEcoreIMFModifier ( keyEvent->modifiers );
        ecoreKeyDownEvent.locks     = (Ecore_IMF_Keyboard_Locks) ECORE_IMF_KEYBOARD_LOCK_NONE;
#ifdef ECORE_IMF_1_13
        ecoreKeyDownEvent.dev_name  = "";
        ecoreKeyDownEvent.dev_class = ECORE_IMF_DEVICE_CLASS_KEYBOARD;
        ecoreKeyDownEvent.dev_subclass = ECORE_IMF_DEVICE_SUBCLASS_NONE;
#endif // ECORE_IMF_1_13

        eventHandled = ecore_imf_context_filter_event( imfContext,
                                                       ECORE_IMF_EVENT_KEY_DOWN,
                                                       (Ecore_IMF_Event *) &ecoreKeyDownEvent );

        // If the event has not been handled by IMF then check if we should reset our IMF context
        if( !eventHandled )
        {
          if ( !strcmp( keyEvent->keyname, "Escape"   ) ||
               !strcmp( keyEvent->keyname, "Return"   ) ||
               !strcmp( keyEvent->keyname, "KP_Enter" ) )
          {
            ecore_imf_context_reset( imfContext );
          }
        }
      }
    }

    // If the event wasn't handled then we should send a key event.
    if ( !eventHandled )
    {
      if ( keyEvent->window == (unsigned int)ecore_wl_window_id_get(handler->mImpl->mWindow) )
      {
        std::string keyName( keyEvent->keyname );
        std::string keyString( "" );
        int keyCode = KeyLookup::GetDaliKeyCode( keyEvent->keyname);
        keyCode = (keyCode == -1) ? 0 : keyCode;
        int modifier( keyEvent->modifiers );
        unsigned long time = keyEvent->timestamp;
        if (!strncmp(keyEvent->keyname, "Keycode-", 8))
          keyCode = atoi(keyEvent->keyname + 8);

        // Ensure key event string is not NULL as keys like SHIFT have a null string.
        if ( keyEvent->string )
        {
          keyString = keyEvent->string;
        }

        Integration::KeyEvent keyEvent(keyName, keyString, keyCode, modifier, time, Integration::KeyEvent::Down );
        handler->SendEvent( keyEvent );
      }
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a key up is received.
   */
  static Eina_Bool EcoreEventKeyUp( void* data, int type, void* event )
  {
    DALI_LOG_INFO( gImfLogging, Debug::General, "EVENT >>EcoreEventKeyUp\n" );

    EventHandler* handler( (EventHandler*)data );
    Ecore_Event_Key *keyEvent( (Ecore_Event_Key*)event );
    bool eventHandled( false );

    // Device keys like Menu, home, back button must skip ecore_imf_context_filter_event.
    if ( ! KeyLookup::IsDeviceButton( keyEvent->keyname ) )
    {
      Ecore_IMF_Context* imfContext = NULL;
      Dali::ImfManager imfManager( ImfManager::Get() );
      if ( imfManager )
      {
        imfContext = ImfManager::GetImplementation( imfManager ).GetContext();
      }

      if ( imfContext )
      {
        // We're consuming key up event so we have to pass to IMF so that it can parse it as well.
        Ecore_IMF_Event_Key_Up ecoreKeyUpEvent;
        ecoreKeyUpEvent.keyname   = keyEvent->keyname;
        ecoreKeyUpEvent.key       = keyEvent->key;
        ecoreKeyUpEvent.string    = keyEvent->string;
        ecoreKeyUpEvent.compose   = keyEvent->compose;
        ecoreKeyUpEvent.timestamp = keyEvent->timestamp;
        ecoreKeyUpEvent.modifiers = EcoreInputModifierToEcoreIMFModifier ( keyEvent->modifiers );
        ecoreKeyUpEvent.locks     = (Ecore_IMF_Keyboard_Locks) ECORE_IMF_KEYBOARD_LOCK_NONE;
#ifdef ECORE_IMF_1_13
        ecoreKeyUpEvent.dev_name  = "";
        ecoreKeyUpEvent.dev_class = ECORE_IMF_DEVICE_CLASS_KEYBOARD;
        ecoreKeyUpEvent.dev_subclass = ECORE_IMF_DEVICE_SUBCLASS_NONE;
#endif // ECORE_IMF_1_13

        eventHandled = ecore_imf_context_filter_event( imfContext,
                                                       ECORE_IMF_EVENT_KEY_UP,
                                                       (Ecore_IMF_Event *) &ecoreKeyUpEvent );
      }
    }

    // If the event wasn't handled then we should send a key event.
    if ( !eventHandled )
    {
      if ( keyEvent->window == (unsigned int)ecore_wl_window_id_get(handler->mImpl->mWindow) )
      {
        std::string keyName( keyEvent->keyname );
        std::string keyString( "" );
        int keyCode = KeyLookup::GetDaliKeyCode( keyEvent->keyname);
        keyCode = (keyCode == -1) ? 0 : keyCode;
        int modifier( keyEvent->modifiers );
        unsigned long time = keyEvent->timestamp;
        if (!strncmp(keyEvent->keyname, "Keycode-", 8))
          keyCode = atoi(keyEvent->keyname + 8);

        // Ensure key event string is not NULL as keys like SHIFT have a null string.
        if ( keyEvent->string )
        {
          keyString = keyEvent->string;
        }
        Integration::KeyEvent keyEvent(keyName, keyString, keyCode, modifier, time, Integration::KeyEvent::Up );
        handler->SendEvent( keyEvent );
      }
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // Window Callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * Called when the window gains focus.
   */
  static Eina_Bool EcoreEventWindowFocusIn( void* data, int type, void* event )
  {
    Ecore_Wl_Event_Focus_In* focusInEvent( (Ecore_Wl_Event_Focus_In*)event );
    EventHandler* handler( (EventHandler*)data );

    DALI_LOG_INFO( gImfLogging, Debug::General, "EVENT >>EcoreEventWindowFocusIn \n" );

    // If the window gains focus and we hid the keyboard then show it again.
    if ( focusInEvent->win == (unsigned int)ecore_wl_window_id_get(handler->mImpl->mWindow) )
    {
      DALI_LOG_INFO( gImfLogging, Debug::General, "EVENT EcoreEventWindowFocusIn - >>WindowFocusGained \n" );

      if ( ImfManager::IsAvailable() /* Only get the ImfManager if it's available as we do not want to create it */ )
      {
        Dali::ImfManager imfManager( ImfManager::Get() );
        if ( imfManager )
        {
          ImfManager& imfManagerImpl( ImfManager::GetImplementation( imfManager ) );
          if( imfManagerImpl.RestoreAfterFocusLost() )
          {
            imfManagerImpl.Activate();
          }
        }
      }
      Dali::Clipboard clipboard = Clipboard::Get();
      clipboard.HideClipboard();
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when the window loses focus.
   */
  static Eina_Bool EcoreEventWindowFocusOut( void* data, int type, void* event )
  {
    Ecore_Wl_Event_Focus_Out* focusOutEvent( (Ecore_Wl_Event_Focus_Out*)event );
    EventHandler* handler( (EventHandler*)data );

    DALI_LOG_INFO( gImfLogging, Debug::General, "EVENT >>EcoreEventWindowFocusOut \n" );

    // If the window loses focus then hide the keyboard.
    if ( focusOutEvent->win == (unsigned int)ecore_wl_window_id_get(handler->mImpl->mWindow) )
    {
      if ( ImfManager::IsAvailable() /* Only get the ImfManager if it's available as we do not want to create it */ )
      {
        Dali::ImfManager imfManager( ImfManager::Get() );
        if ( imfManager )
        {
          ImfManager& imfManagerImpl( ImfManager::GetImplementation( imfManager ) );
          if( imfManagerImpl.RestoreAfterFocusLost() )
          {
            imfManagerImpl.Deactivate();
          }
        }
      }

      // Hiding clipboard event will be ignored once because window focus out event is always received on showing clipboard
      Dali::Clipboard clipboard = Clipboard::Get();
      if ( clipboard )
      {
        Clipboard& clipBoardImpl( GetImplementation( clipboard ) );
        clipBoardImpl.HideClipboard(true);
      }
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when the window is damaged.
   */
  static Eina_Bool EcoreEventWindowDamaged(void *data, int type, void *event)
  {
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when the window properties are changed.
   * We are only interested in the font change.
   */


  /////////////////////////////////////////////////////////////////////////////////////////////////
  // Drag & Drop Callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * Called when a dragged item enters our window's bounds.
   * This is when items are dragged INTO our window.
   */
  static Eina_Bool EcoreEventDndEnter( void* data, int type, void* event )
  {
    DALI_LOG_INFO( gDragAndDropLogFilter, Debug::Concise, "EcoreEventDndEnter\n" );

    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a dragged item is moved within our window.
   * This is when items are dragged INTO our window.
   */
  static Eina_Bool EcoreEventDndPosition( void* data, int type, void* event )
  {
    DALI_LOG_INFO(gDragAndDropLogFilter, Debug::Concise, "EcoreEventDndPosition\n" );

    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a dragged item leaves our window's bounds.
   * This is when items are dragged INTO our window.
   */
  static Eina_Bool EcoreEventDndLeave( void* data, int type, void* event )
  {
    DALI_LOG_INFO(gDragAndDropLogFilter, Debug::Concise, "EcoreEventDndLeave\n" );

    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when the dragged item is dropped within our window's bounds.
   * This is when items are dragged INTO our window.
   */
  static Eina_Bool EcoreEventDndDrop( void* data, int type, void* event )
  {
    DALI_LOG_INFO(gDragAndDropLogFilter, Debug::Concise, "EcoreEventDndDrop\n" );

    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a dragged item is moved from our window and the target window has done processing it.
   * This is when items are dragged FROM our window.
   */
  static Eina_Bool EcoreEventDndFinished( void* data, int type, void* event )
  {
    DALI_LOG_INFO(gDragAndDropLogFilter, Debug::Concise, "EcoreEventDndFinished\n" );
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a dragged item is moved from our window and the target window has sent us a status.
   * This is when items are dragged FROM our window.
   */
  static Eina_Bool EcoreEventDndStatus( void* data, int type, void* event )
  {
    DALI_LOG_INFO(gDragAndDropLogFilter, Debug::Concise, "EcoreEventDndStatus\n" );
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when the client messages (i.e. the accessibility events) are received.
   */
  static Eina_Bool EcoreEventClientMessage( void* data, int type, void* event )
  {
    return ECORE_CALLBACK_PASS_ON;
  }


  /////////////////////////////////////////////////////////////////////////////////////////////////
  // ElDBus Accessibility Callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DALI_ELDBUS_AVAILABLE
  // Callback for Ecore ElDBus accessibility events.
  static void OnEcoreElDBusAccessibilityNotification( void *context EINA_UNUSED, const Eldbus_Message *message )
  {
    EventHandler* handler = static_cast< EventHandler* >( context );
    // Ignore any accessibility events when paused.
    if( handler->mPaused )
    {
      return;
    }

    if( !handler->mAccessibilityAdaptor )
    {
      DALI_LOG_ERROR( "Invalid accessibility adaptor\n" );
      return;
    }

    AccessibilityAdaptor* accessibilityAdaptor( &AccessibilityAdaptor::GetImplementation( handler->mAccessibilityAdaptor ) );
    if( !accessibilityAdaptor )
    {
      DALI_LOG_ERROR( "Cannot access accessibility adaptor\n" );
      return;
    }

    int gestureValue;
    int xS, yS, xE, yE;
    int state; // 0 - begin, 1 - ongoing, 2 - ended, 3 - aborted
    int eventTime;

    // The string defines the arg-list's respective types.
    if( !eldbus_message_arguments_get( message, "iiiiiiu", &gestureValue, &xS, &yS, &xE, &yE, &state, &eventTime ) )
    {
      DALI_LOG_ERROR( "OnEcoreElDBusAccessibilityNotification: Error getting arguments\n" );
    }

    DALI_LOG_INFO( gImfLogging, Debug::General, "Got gesture: Name: %d  Args: %d,%d,%d,%d  State: %d\n", gestureValue, xS, yS, xE, yE );

    // Create a touch point object.
    TouchPoint::State touchPointState( TouchPoint::Down );
    if( state == 0 )
    {
      touchPointState = TouchPoint::Down; // Mouse down.
    }
    else if( state == 1 )
    {
      touchPointState = TouchPoint::Motion; // Mouse move.
    }
    else if( state == 2 )
    {
      touchPointState = TouchPoint::Up; // Mouse up.
    }
    else
    {
      touchPointState = TouchPoint::Interrupted; // Error.
    }

    // Send touch event to accessibility adaptor.
    TouchPoint point( 0, touchPointState, (float)xS, (float)yS );

    // Perform actions based on received gestures.
    // Note: This is seperated from the reading so we can have other input readers without changing the below code.
    switch( gestureValue )
    {
      case 0: // OneFingerHover
      {
        // Focus, read out.
        accessibilityAdaptor->HandleActionReadEvent( (unsigned int)xS, (unsigned int)yS, true /* allow read again */ );
        break;
      }
      case 1: // TwoFingersHover
      {
        // In accessibility mode, scroll action should be handled when the currently focused actor is contained in scrollable control
        accessibilityAdaptor->HandleActionScrollEvent( point, GetCurrentMilliSeconds() );
        break;
      }
      case 2: // ThreeFingersHover
      {
        // Read from top item on screen continuously.
        accessibilityAdaptor->HandleActionReadFromTopEvent();
        break;
      }
      case 3: // OneFingerFlickLeft
      {
        // Move to previous item.
        accessibilityAdaptor->HandleActionReadPreviousEvent();
        break;
      }
      case 4: // OneFingerFlickRight
      {
        // Move to next item.
        accessibilityAdaptor->HandleActionReadNextEvent();
        break;
      }
      case 5: // OneFingerFlickUp
      {
        // Move to previous item.
        accessibilityAdaptor->HandleActionPreviousEvent();
        break;
      }
      case 6: // OneFingerFlickDown
      {
        // Move to next item.
        accessibilityAdaptor->HandleActionNextEvent();
        break;
      }
      case 7: // TwoFingersFlickUp
      {
        // Scroll up the list.
        accessibilityAdaptor->HandleActionScrollUpEvent();
        break;
      }
      case 8: // TwoFingersFlickDown
      {
        // Scroll down the list.
        accessibilityAdaptor->HandleActionScrollDownEvent();
        break;
      }
      case 9: // TwoFingersFlickLeft
      {
        // Scroll left to the previous page
        accessibilityAdaptor->HandleActionPageLeftEvent();
        break;
      }
      case 10: // TwoFingersFlickRight
      {
        // Scroll right to the next page
        accessibilityAdaptor->HandleActionPageRightEvent();
        break;
      }
      case 11: // ThreeFingersFlickLeft
      {
        // Not exist yet
        break;
      }
      case 12: // ThreeFingersFlickRight
      {
        // Not exist yet
        break;
      }
      case 13: // ThreeFingersFlickUp
      {
        // Not exist yet
        break;
      }
      case 14: // ThreeFingersFlickDown
      {
        // Not exist yet
        break;
      }
      case 15: // OneFingerSingleTap
      {
        // Focus, read out.
        accessibilityAdaptor->HandleActionReadEvent( (unsigned int)xS, (unsigned int)yS, true /* allow read again */ );
        break;
      }
      case 16: // OneFingerDoubleTap
      {
        // Activate selected item / active edit mode.
        accessibilityAdaptor->HandleActionActivateEvent();
        break;
      }
      case 17: // OneFingerTripleTap
      {
        // Zoom
        accessibilityAdaptor->HandleActionZoomEvent();
        break;
      }
      case 18: // TwoFingersSingleTap
      {
        // Pause/Resume current speech
        accessibilityAdaptor->HandleActionReadPauseResumeEvent();
        break;
      }
      case 19: // TwoFingersDoubleTap
      {
        // Start/Stop current action
        accessibilityAdaptor->HandleActionStartStopEvent();
        break;
      }
      case 20: // TwoFingersTripleTap
      {
        // Read information from indicator
        accessibilityAdaptor->HandleActionReadIndicatorInformationEvent();
        break;
      }
      case 21: // ThreeFingersSingleTap
      {
        // Read from top item on screen continuously.
        accessibilityAdaptor->HandleActionReadFromTopEvent();
        break;
      }
      case 22: // ThreeFingersDoubleTap
      {
        // Read from next item continuously.
        accessibilityAdaptor->HandleActionReadFromNextEvent();
        break;
      }
      case 23: // ThreeFingersTripleTap
      {
        // Not exist yet
        break;
      }
      case 24: // OneFingerFlickLeftReturn
      {
        // Scroll up to the previous page
        accessibilityAdaptor->HandleActionPageUpEvent();
        break;
      }
      case 25: // OneFingerFlickRightReturn
      {
        // Scroll down to the next page
        accessibilityAdaptor->HandleActionPageDownEvent();
        break;
      }
      case 26: // OneFingerFlickUpReturn
      {
        // Move to the first item on screen
        accessibilityAdaptor->HandleActionMoveToFirstEvent();
        break;
      }
      case 27: // OneFingerFlickDownReturn
      {
        // Move to the last item on screen
        accessibilityAdaptor->HandleActionMoveToLastEvent();
        break;
      }
      case 28: // TwoFingersFlickLeftReturn
      {
        // Not exist yet
        break;
      }
      case 29: // TwoFingersFlickRightReturn
      {
        // Not exist yet
        break;
      }
      case 30: // TwoFingersFlickUpReturn
      {
        // Not exist yet
        break;
      }
      case 31: // TwoFingersFlickDownReturn
      {
        // Not exist yet
        break;
      }
      case 32: // ThreeFingersFlickLeftReturn
      {
        // Not exist yet
        break;
      }
      case 33: // ThreeFingersFlickRightReturn
      {
        // Not exist yet
        break;
      }
      case 34: // ThreeFingersFlickUpReturn
      {
        // Not exist yet
        break;
      }
      case 35: // ThreeFingersFlickDownReturn
      {
        // Not exist yet
        break;
      }
    }
  }

  void EcoreElDBusInitialisation( void *handle )
  {
    Eldbus_Object *object;
    Eldbus_Proxy *manager;

    if( !( mSystemConnection = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM) ) )
    {
      DALI_LOG_ERROR( "Unable to get system bus\n" );
    }

    object = eldbus_object_get( mSystemConnection, BUS, PATH );
    if( !object )
    {
      DALI_LOG_ERROR( "Getting object failed\n" );
      return;
    }

    manager = eldbus_proxy_get( object, INTERFACE );
    if( !manager )
    {
      DALI_LOG_ERROR( "Getting proxy failed\n" );
      return;
    }

    if( !eldbus_proxy_signal_handler_add( manager, "GestureDetected", OnEcoreElDBusAccessibilityNotification, handle ) )
    {
      DALI_LOG_ERROR( "No signal handler returned\n" );
    }
  }
#endif // DALI_ELDBUS_AVAILABLE

  /**
   * Called when the source window notifies us the content in clipboard is selected.
   */
  static Eina_Bool EcoreEventSelectionClear( void* data, int type, void* event )
  {
    DALI_LOG_INFO(gSelectionEventLogFilter, Debug::Concise, "EcoreEventSelectionClear\n" );
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when the source window sends us about the selected content.
   * For example, when dragged items are dragged INTO our window or when items are selected in the clipboard.
   */
  static Eina_Bool EcoreEventSelectionNotify( void* data, int type, void* event )
  {
    DALI_LOG_INFO(gSelectionEventLogFilter, Debug::Concise, "EcoreEventSelectionNotify\n" );
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
  * Called when the source window notifies us the content in clipboard is selected.
  */
  static Eina_Bool EcoreEventDataSend( void* data, int type, void* event )
  {
    DALI_LOG_INFO(gSelectionEventLogFilter, Debug::Concise, "EcoreEventDataSend\n" );

    Dali::Clipboard clipboard = Clipboard::Get();
    if ( clipboard )
    {
      Clipboard& clipBoardImpl( GetImplementation( clipboard ) );
      clipBoardImpl.ExcuteBuffered( true, event );
    }
    return ECORE_CALLBACK_PASS_ON;
  }

   /**
    * Called when the source window sends us about the selected content.
    * For example, when item is selected in the clipboard.
    */
   static Eina_Bool EcoreEventDataReceive( void* data, int type, void* event )
   {
     DALI_LOG_INFO(gSelectionEventLogFilter, Debug::Concise, "EcoreEventDataReceive\n" );

     EventHandler* handler( (EventHandler*)data );
      Dali::Clipboard clipboard = Clipboard::Get();
      char *selectionData = NULL;
      if ( clipboard )
      {
        Clipboard& clipBoardImpl( GetImplementation( clipboard ) );
        selectionData = clipBoardImpl.ExcuteBuffered( false, event );
      }
      if ( selectionData && handler->mClipboardEventNotifier )
      {
        ClipboardEventNotifier& clipboardEventNotifier( ClipboardEventNotifier::GetImplementation( handler->mClipboardEventNotifier ) );
        std::string content( selectionData, strlen(selectionData) );

        clipboardEventNotifier.SetContent( content );
        clipboardEventNotifier.EmitContentSelectedSignal();
      }
     return ECORE_CALLBACK_PASS_ON;
   }

  /*
  * Called when rotate event is recevied
  */
  static Eina_Bool EcoreEventRotate( void* data, int type, void* event )
  {
    DALI_LOG_INFO( gSelectionEventLogFilter, Debug::Concise, "EcoreEventRotate\n" );

    EventHandler* handler( (EventHandler*)data );
    Ecore_Wl_Event_Window_Rotate* ev( (Ecore_Wl_Event_Window_Rotate*)event );

    if( ev->win != (unsigned int)ecore_wl_window_id_get( handler->mImpl->mWindow ) )
    {
      return ECORE_CALLBACK_PASS_ON;
    }

    RotationEvent rotationEvent;
    rotationEvent.angle = ev->angle;
    rotationEvent.winResize = 0;
    rotationEvent.width = ev->w;
    rotationEvent.height = ev->h;
    handler->SendRotationPrepareEvent( rotationEvent );

    return ECORE_CALLBACK_PASS_ON;
  }

  /*
  * Called when detent event is recevied
  */
  static Eina_Bool EcoreEventDetent( void* data, int type, void* event )
  {
    DALI_LOG_INFO(gSelectionEventLogFilter, Debug::Concise, "EcoreEventDetent\n" );
    EventHandler* handler( (EventHandler*)data );
    Ecore_Event_Detent_Rotate *e((Ecore_Event_Detent_Rotate *)event);
    int direction = (e->direction == ECORE_DETENT_DIRECTION_CLOCKWISE) ? 1 : -1;
    int timeStamp = e->timestamp;

    WheelEvent wheelEvent( WheelEvent::CUSTOM_WHEEL, 0, 0, Vector2(0.0f, 0.0f), direction, timeStamp );
    handler->SendWheelEvent( wheelEvent );
    return ECORE_CALLBACK_PASS_ON;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // Font Callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /**
   * Called when a font name is changed.
   */
  static void VconfNotifyFontNameChanged( keynode_t* node, void* data )
  {
    EventHandler* handler = static_cast<EventHandler*>( data );
    handler->SendEvent( StyleChange::DEFAULT_FONT_CHANGE );
  }

  /**
   * Called when a font size is changed.
   */
  static void VconfNotifyFontSizeChanged( keynode_t* node, void* data )
  {
    EventHandler* handler = static_cast<EventHandler*>( data );
    handler->SendEvent( StyleChange::DEFAULT_FONT_SIZE_CHANGE );
  }

  // Data
  EventHandler* mHandler;
  std::vector<Ecore_Event_Handler*> mEcoreEventHandler;
  Ecore_Wl_Window* mWindow;
#ifdef DALI_ELDBUS_AVAILABLE
  Eldbus_Connection* mSystemConnection;
#endif // DALI_ELDBUS_AVAILABLE
};

EventHandler::EventHandler( RenderSurface* surface, CoreEventInterface& coreEventInterface, GestureManager& gestureManager, DamageObserver& damageObserver, DragAndDropDetectorPtr dndDetector )
: mCoreEventInterface( coreEventInterface ),
  mGestureManager( gestureManager ),
  mStyleMonitor( StyleMonitor::Get() ),
  mDamageObserver( damageObserver ),
  mRotationObserver( NULL ),
  mDragAndDropDetector( dndDetector ),
  mAccessibilityAdaptor( AccessibilityAdaptor::Get() ),
  mClipboardEventNotifier( ClipboardEventNotifier::Get() ),
  mClipboard( Clipboard::Get() ),
  mImpl( NULL ),
  mPaused( false )
{
  Ecore_Wl_Window* window = 0;

  // this code only works with the Ecore RenderSurface so need to downcast
  ECore::WindowRenderSurface* ecoreSurface = dynamic_cast< ECore::WindowRenderSurface* >( surface );
  if( ecoreSurface )
  {
    window = ecoreSurface->GetWlWindow();
  }

  mImpl = new Impl(this, window);
}

EventHandler::~EventHandler()
{
  if(mImpl)
  {
    delete mImpl;
  }

  mGestureManager.Stop();
}

void EventHandler::SendEvent(Integration::Point& point, unsigned long timeStamp)
{
  if(timeStamp < 1)
  {
    timeStamp = GetCurrentMilliSeconds();
  }

  Integration::TouchEvent touchEvent;
  Integration::HoverEvent hoverEvent;
  Integration::TouchEventCombiner::EventDispatchType type = mCombiner.GetNextTouchEvent(point, timeStamp, touchEvent, hoverEvent);
  if(type != Integration::TouchEventCombiner::DispatchNone )
  {
    DALI_LOG_INFO(gTouchEventLogFilter, Debug::General, "%d: Device %d: Button state %d (%.2f, %.2f)\n", timeStamp, point.GetDeviceId(), point.GetState(), point.GetLocalPosition().x, point.GetLocalPosition().y);

    // First the touch and/or hover event & related gesture events are queued
    if(type == Integration::TouchEventCombiner::DispatchTouch || type == Integration::TouchEventCombiner::DispatchBoth)
    {
      mCoreEventInterface.QueueCoreEvent( touchEvent );
      mGestureManager.SendEvent(touchEvent);
    }

    if(type == Integration::TouchEventCombiner::DispatchHover || type == Integration::TouchEventCombiner::DispatchBoth)
    {
      mCoreEventInterface.QueueCoreEvent( hoverEvent );
    }

    // Next the events are processed with a single call into Core
    mCoreEventInterface.ProcessCoreEvents();
  }
}

void EventHandler::SendEvent(Integration::KeyEvent& keyEvent)
{
  Dali::PhysicalKeyboard physicalKeyboard = PhysicalKeyboard::Get();
  if ( physicalKeyboard )
  {
    if ( ! KeyLookup::IsDeviceButton( keyEvent.keyName.c_str() ) )
    {
      GetImplementation( physicalKeyboard ).KeyReceived( keyEvent.time > 1 );
    }
  }

  // Create send KeyEvent to Core.
  mCoreEventInterface.QueueCoreEvent( keyEvent );
  mCoreEventInterface.ProcessCoreEvents();
}

void EventHandler::SendWheelEvent( WheelEvent& wheelEvent )
{
  // Create WheelEvent and send to Core.
  Integration::WheelEvent event( static_cast< Integration::WheelEvent::Type >(wheelEvent.type), wheelEvent.direction, wheelEvent.modifiers, wheelEvent.point, wheelEvent.z, wheelEvent.timeStamp );
  mCoreEventInterface.QueueCoreEvent( event );
  mCoreEventInterface.ProcessCoreEvents();
}

void EventHandler::SendEvent( StyleChange::Type styleChange )
{
  DALI_ASSERT_DEBUG( mStyleMonitor && "StyleMonitor Not Available" );
  GetImplementation( mStyleMonitor ).StyleChanged(styleChange);
}

void EventHandler::SendEvent( const DamageArea& area )
{
  mDamageObserver.OnDamaged( area );
}

void EventHandler::SendRotationPrepareEvent( const RotationEvent& event )
{
  if( mRotationObserver != NULL )
  {
    mRotationObserver->OnRotationPrepare( event );
    mRotationObserver->OnRotationRequest();
  }
}

void EventHandler::SendRotationRequestEvent( )
{
  // No need to separate event into prepare and request in wayland
}

void EventHandler::FeedTouchPoint( TouchPoint& point, int timeStamp)
{
  Integration::Point convertedPoint( point );
  SendEvent(convertedPoint, timeStamp);
}

void EventHandler::FeedWheelEvent( WheelEvent& wheelEvent )
{
  SendWheelEvent( wheelEvent );
}

void EventHandler::FeedKeyEvent( KeyEvent& event )
{
  Integration::KeyEvent convertedEvent( event );
  SendEvent( convertedEvent );
}

void EventHandler::FeedEvent( Integration::Event& event )
{
  mCoreEventInterface.QueueCoreEvent( event );
  mCoreEventInterface.ProcessCoreEvents();
}

void EventHandler::Reset()
{
  mCombiner.Reset();

  // Any touch listeners should be told of the interruption.
  Integration::TouchEvent event;
  Integration::Point point;
  point.SetState( PointState::INTERRUPTED );
  event.AddPoint( point );

  // First the touch event & related gesture events are queued
  mCoreEventInterface.QueueCoreEvent( event );
  mGestureManager.SendEvent( event );

  // Next the events are processed with a single call into Core
  mCoreEventInterface.ProcessCoreEvents();
}

void EventHandler::Pause()
{
  mPaused = true;
  Reset();
}

void EventHandler::Resume()
{
  mPaused = false;
  Reset();
}

void EventHandler::SetDragAndDropDetector( DragAndDropDetectorPtr detector )
{
  mDragAndDropDetector = detector;
}

void EventHandler::SetRotationObserver( RotationObserver* observer )
{
  mRotationObserver = observer;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
