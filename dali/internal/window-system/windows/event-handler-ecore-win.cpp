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
#include <dali/internal/window-system/common/event-handler.h>

// EXTERNAL INCLUDES
#include <cstring>
#include <dali/public-api/adaptor-framework/window.h>

#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/events/touch-point.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/events/wheel-event.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <dali/integration-api/events/hover-event-integ.h>
#include <dali/integration-api/events/wheel-event-integ.h>
#include <WindowsEventSystem.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/gesture-manager.h>
#include <dali/internal/window-system/windows/window-render-surface-ecore-win.h>
#include <dali/internal/clipboard/common/clipboard-impl.h>
#include <dali/internal/input/common/key-impl.h>
#include <dali/internal/input/common/physical-keyboard-impl.h>
#include <dali/internal/styling/common/style-monitor-impl.h>
#include <dali/internal/system/common/core-event-interface.h>

#include <Windows.h>

namespace Dali
{

namespace Internal
{

using namespace Win32System;

namespace Adaptor
{

#if defined(DEBUG_ENABLED)
namespace
{
Integration::Log::Filter* gTouchEventLogFilter  = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_TOUCH");
Integration::Log::Filter* gClientMessageLogFilter  = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_CLIENT_MESSAGE");
Integration::Log::Filter* gDragAndDropLogFilter = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_DND");
Integration::Log::Filter* gImfLogging  = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_IMF");
Integration::Log::Filter* gSelectionEventLogFilter = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_SELECTION");
} // unnamed namespace
#endif

namespace
{

const std::string DEFAULT_DEVICE_NAME = "";
const Device::Class::Type DEFAULT_DEVICE_CLASS = Device::Class::NONE;
const Device::Subclass::Type DEFAULT_DEVICE_SUBCLASS = Device::Subclass::NONE;

const unsigned int PRIMARY_TOUCH_BUTTON_ID( 1 );

#ifndef DALI_PROFILE_UBUNTU
const char * CLIPBOARD_ATOM                = "CBHM_MSG";
const char * CLIPBOARD_SET_OWNER_MESSAGE   = "SET_OWNER";
#endif // DALI_PROFILE_UBUNTU

const unsigned int BYTES_PER_CHARACTER_FOR_ATTRIBUTES = 3;

/**
 * Ecore_Event_Modifier enums in Ecore_Input.h do not match Ecore_IMF_Keyboard_Modifiers in Ecore_IMF.h.
 * This function converts from Ecore_Event_Modifier to Ecore_IMF_Keyboard_Modifiers enums.
 * @param[in] ecoreModifier the Ecore_Event_Modifier input.
 * @return the Ecore_IMF_Keyboard_Modifiers output.
 */

static bool IsDeviceButton(int keyCode)
{
    bool ret = false;
    switch (keyCode)
    {
    case VK_BACK:
    case VK_ESCAPE:
    case VK_LEFT:
    case VK_RIGHT:
    case VK_SHIFT:
        ret = true;
        break;

    default:
        break;
    }

    return ret;
}

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
// Copied from x server

void GetNanoseconds(uint64_t& timeInNanoseconds)
{
    Win32WindowSystem::GetNanoseconds(timeInNanoseconds);
}

// Copied from x server
unsigned int GetCurrentMilliSeconds(void)
{
    return Win32WindowSystem::GetCurrentMilliSeconds();
}
} // unnamed namespace

// Impl to hide EFL implementation.
struct EventHandler::Impl
{
  // Construction & Destruction

  /**
   * Constructor
   */
  Impl( EventHandler* handler, Ecore_Win_Window window )
  : mHandler( handler ),
    mEcoreEventHandler(),
    mWindow( window ),
    mXiDeviceId( 0 )
#ifdef DALI_ELDBUS_AVAILABLE
  , mSessionConnection( NULL ),
    mA11yConnection( NULL )
#endif
  {
    // Only register for touch and key events if we have a window
    if ( window != 0 )
    {
      // Register Touch events
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_MOUSE_BUTTON_DOWN,  EcoreEventMouseButtonDown, handler ) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_MOUSE_BUTTON_UP,    EcoreEventMouseButtonUp,   handler ) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_MOUSE_MOVE,         EcoreEventMouseButtonMove, handler ) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_MOUSE_OUT,          EcoreEventMouseOut,   handler ) ); // process mouse out event like up event

      // Register Mouse wheel events
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_MOUSE_WHEEL,        EcoreEventMouseWheel,      handler ) );

      // Register Key events
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_KEY_DOWN,           EcoreEventKeyDown,         handler ) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_EVENT_KEY_UP,             EcoreEventKeyUp,           handler ) );

      // Register Focus events
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WIN_EVENT_WINDOW_FOCUS_IN,  EcoreEventWindowFocusIn,   handler ) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WIN_EVENT_WINDOW_FOCUS_OUT, EcoreEventWindowFocusOut,  handler ) );

      // Register Window damage events
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WIN_EVENT_WINDOW_DAMAGE,    EcoreEventWindowDamaged, handler ) );

      // Enable Drag & Drop and register DnD events
      //ecore_win_dnd_aware_set( window, EINA_TRUE );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WIN_EVENT_XDND_ENTER,       EcoreEventDndEnter,            handler) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WIN_EVENT_XDND_POSITION,    EcoreEventDndPosition,         handler) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WIN_EVENT_XDND_LEAVE,       EcoreEventDndLeave,            handler) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WIN_EVENT_XDND_DROP,        EcoreEventDndDrop,             handler) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WIN_EVENT_XDND_FINISHED,    EcoreEventDndFinished,         handler) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WIN_EVENT_XDND_STATUS,      EcoreEventDndStatus,           handler) );

      // Register Client message events - accessibility etc.
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WIN_EVENT_CLIENT_MESSAGE,  EcoreEventClientMessage, handler ) );

      // Register Selection event - clipboard selection, Drag & Drop selection etc.
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WIN_EVENT_SELECTION_CLEAR, EcoreEventSelectionClear, handler ) );
      mEcoreEventHandler.push_back( ecore_event_handler_add( ECORE_WIN_EVENT_SELECTION_NOTIFY, EcoreEventSelectionNotify, handler ) );

      Win32WindowSystem::AddListener(EventEntry);
    }
  }

  /**
   * Destructor
   */
  ~Impl()
  {
#ifndef DALI_PROFILE_UBUNTU
    //vconf_ignore_key_changed( VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, VconfNotifyFontSizeChanged );
    //vconf_ignore_key_changed( DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_NAME, VconfNotifyFontNameChanged );
#endif // DALI_PROFILE_UBUNTU

    for( std::vector<Ecore_Event_Handler>::iterator iter = mEcoreEventHandler.begin(), endIter = mEcoreEventHandler.end(); iter != endIter; ++iter )
    {
      ecore_event_handler_del( *iter );
    }

#ifdef DALI_ELDBUS_AVAILABLE
    // Close down ElDBus
    if( mA11yConnection )
    {
      eldbus_connection_unref( mA11yConnection );
    }

    if( mSessionConnection )
    {
      eldbus_connection_unref( mSessionConnection );
    }

    eldbus_shutdown();
#endif // DALI_ELDBUS_AVAILABLE
  }

  // Static methods

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // Touch Callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////
  static void EventEntry(long hWnd, unsigned int uMsg, long wParam, long lParam)
  {
      EventCallback callback = GetCallback(uMsg);
      EventHandler *handler = ( EventHandler* )GetEventHandler(uMsg);

      if (NULL != callback)
      {
          //EventHandler *handler = new EventHandler();
          TWinEventInfo eventInfo(hWnd, uMsg, wParam, lParam);
          callback(handler, uMsg, &eventInfo);
      }
  }
  /**
   * Called when a touch down is received.
   */
  static bool EcoreEventMouseButtonDown( void* data, int type, TWinEventInfo *event )
  {
    Ecore_Event_Mouse_Button touchEvent = *((Ecore_Event_Mouse_Button*)event);
    EventHandler* handler( (EventHandler*)data );

    touchEvent.x = LOWORD(event->lParam);
    touchEvent.y = HIWORD(event->lParam);
    touchEvent.multi.device = DEVICE_MOUSE;

    if ( touchEvent.window == handler->mImpl->mWindow )
    {
      PointState::Type state ( PointState::DOWN );

      // Check if the buttons field is set and ensure it's the primary touch button.
      // If this event was triggered by buttons other than the primary button (used for touch), then
      // just send an interrupted event to Core.
      if ( touchEvent.buttons && (touchEvent.buttons != PRIMARY_TOUCH_BUTTON_ID ) )
      {
        state = PointState::INTERRUPTED;
      }

      Integration::Point point;
      point.SetDeviceId( touchEvent.multi.device );
      point.SetState( state );
      point.SetScreenPosition( Vector2( touchEvent.x, touchEvent.y + Win32WindowSystem::GetEdgeHeight() ) );
      point.SetRadius( touchEvent.multi.radius, Vector2( touchEvent.multi.radius_x, touchEvent.multi.radius_y ) );
      point.SetPressure( touchEvent.multi.pressure );
      point.SetAngle( Degree( touchEvent.multi.angle ) );
      handler->SendEvent( point, touchEvent.timestamp );
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a touch up is received.
   */
  static bool EcoreEventMouseButtonUp( void* data, int type, TWinEventInfo *event )
  {
    Ecore_Event_Mouse_Button touchEvent = *((Ecore_Event_Mouse_Button*)event);
    EventHandler* handler( (EventHandler*)data );

    touchEvent.x = LOWORD(event->lParam);
    touchEvent.y = HIWORD(event->lParam);
    touchEvent.multi.device = DEVICE_MOUSE;

    if ( touchEvent.window == handler->mImpl->mWindow )
    {
      Integration::Point point;
      point.SetDeviceId(touchEvent.multi.device );
      point.SetState( PointState::UP );
      point.SetScreenPosition( Vector2(touchEvent.x, touchEvent.y + Win32WindowSystem::GetEdgeHeight() ) );
      point.SetRadius(touchEvent.multi.radius, Vector2(touchEvent.multi.radius_x, touchEvent.multi.radius_y ) );
      point.SetPressure(touchEvent.multi.pressure );
      point.SetAngle( Degree(touchEvent.multi.angle ) );
      handler->SendEvent( point, touchEvent.timestamp );
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a touch motion is received.
   */
  static bool EcoreEventMouseButtonMove( void* data, int type, TWinEventInfo *event )
  {
    Ecore_Event_Mouse_Button touchEvent = *((Ecore_Event_Mouse_Button*)event);
    touchEvent.timestamp = GetTickCount();

    EventHandler* handler( (EventHandler*)data );

    touchEvent.x = LOWORD(event->lParam);
    touchEvent.y = HIWORD(event->lParam);
    touchEvent.multi.device = DEVICE_MOUSE;

    if (touchEvent.window == handler->mImpl->mWindow )
    {
      Integration::Point point;
      point.SetDeviceId(touchEvent.multi.device );
      point.SetState( PointState::MOTION );
      point.SetScreenPosition( Vector2(touchEvent.x, touchEvent.y + Win32WindowSystem::GetEdgeHeight() ) );
      point.SetRadius(touchEvent.multi.radius, Vector2(touchEvent.multi.radius_x, touchEvent.multi.radius_y ) );
      point.SetPressure(touchEvent.multi.pressure );
      point.SetAngle( Degree(touchEvent.multi.angle ) );
      handler->SendEvent( point, touchEvent.timestamp );
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  static bool EcoreEventMouseOut( void* data, int type, TWinEventInfo *event )
  {
    return ECORE_CALLBACK_PASS_ON;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // Wheel Callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * Called when a mouse wheel is received.
   */
  static bool EcoreEventMouseWheel( void* data, int type, TWinEventInfo *event )
  {
    Ecore_Event_Mouse_Wheel mouseWheelEvent = *((Ecore_Event_Mouse_Wheel*)event);
    mouseWheelEvent.x = LOWORD(event->lParam);
    mouseWheelEvent.y = HIWORD(event->lParam);

    DALI_LOG_INFO( gImfLogging, Debug::General, "EVENT Ecore_Event_Mouse_Wheel: direction: %d, modifiers: %d, x: %d, y: %d, z: %d\n", mouseWheelEvent->direction, mouseWheelEvent->modifiers, mouseWheelEvent->x, mouseWheelEvent->y, mouseWheelEvent->z );

    EventHandler* handler( (EventHandler*)data );
    if ( mouseWheelEvent.window == handler->mImpl->mWindow )
    {
      WheelEvent wheelEvent( WheelEvent::MOUSE_WHEEL, mouseWheelEvent.direction, mouseWheelEvent.modifiers, Vector2(mouseWheelEvent.x, mouseWheelEvent.y), mouseWheelEvent.z, mouseWheelEvent.timestamp );
      handler->SendWheelEvent( wheelEvent );
    }
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a custom wheel is received.
   */
  static bool EcoreEventCustomWheel( void* data, int type, TWinEventInfo *event )
  {
    return ECORE_CALLBACK_PASS_ON;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // Key Callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * Called when a key down is received.
   */

  static bool EcoreEventKeyDown( void* data, int type, TWinEventInfo *event )
  {
    DALI_LOG_INFO( gImfLogging, Debug::General, "EVENT >>EcoreEventKeyDown \n" );

    EventHandler* handler( (EventHandler*)data );

    if( event->mWindow == handler->mImpl->mWindow )
    {
      int keyCode = event->wParam;
      std::string keyName( Win32WindowSystem::GetKeyName( keyCode ) );
      std::string keyString( "" );
      std::string compose ( "" );

      int modifier( 0 );
      unsigned long time( 0 );

      // Ensure key event string is not NULL as keys like SHIFT have a null string.
      keyString.push_back( event->wParam );

      Integration::KeyEvent keyEvent(keyName, keyString, keyCode, modifier, time, Integration::KeyEvent::Down, compose, DEFAULT_DEVICE_NAME, DEFAULT_DEVICE_CLASS, DEFAULT_DEVICE_SUBCLASS );
      handler->SendEvent( keyEvent );
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a key up is received.
   */
  static bool EcoreEventKeyUp( void* data, int type, TWinEventInfo *event )
  {
    DALI_LOG_INFO( gImfLogging, Debug::General, "EVENT >>EcoreEventKeyUp \n" );

    EventHandler* handler( (EventHandler*)data );

    if( event->mWindow == handler->mImpl->mWindow )
    {
      int keyCode = event->wParam;
      std::string keyName( Win32WindowSystem::GetKeyName( keyCode ) );
      std::string keyString( "" );
      std::string compose( "" );

      int modifier( 0/*keyEvent->modifiers*/ );
      unsigned long time( 0 );

      // Ensure key event string is not NULL as keys like SHIFT have a null string.
      keyString.push_back( event->wParam );

      Integration::KeyEvent keyEvent(keyName, keyString, keyCode, modifier, time, Integration::KeyEvent::Up, compose, DEFAULT_DEVICE_NAME, DEFAULT_DEVICE_CLASS, DEFAULT_DEVICE_SUBCLASS );

      handler->SendEvent( keyEvent );
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // Window Callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * Called when the window gains focus.
   */
  static bool EcoreEventWindowFocusIn( void* data, int type, TWinEventInfo *event )
  {
    EventHandler* handler( (EventHandler*)data );

    DALI_LOG_INFO( gImfLogging, Debug::General, "EVENT >>EcoreEventWindowFocusIn \n" );

    // If the window gains focus and we hid the keyboard then show it again.
    if (event->mWindow == handler->mImpl->mWindow )
    {
      DALI_LOG_INFO( gImfLogging, Debug::General, "EVENT EcoreEventWindowFocusIn - >>WindowFocusGained \n" );
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when the window loses focus.
   */
  static bool EcoreEventWindowFocusOut( void* data, int type, TWinEventInfo *event )
  {
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when the window is damaged.
   */
  static bool EcoreEventWindowDamaged(void *data, int type, TWinEventInfo *event)
  {
      Ecore_Event_Mouse_Button* windowDamagedEvent( (Ecore_Event_Mouse_Button*)event );
    EventHandler* handler( (EventHandler*)data );

    if( windowDamagedEvent->window == handler->mImpl->mWindow )
    {
      DamageArea area;
      area.x = 0;
      area.y = 0;
      area.width = 480;
      area.height = 800;

      handler->SendEvent( area );
    }

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
  static bool EcoreEventDndEnter( void* data, int type, TWinEventInfo *event )
  {
    DALI_LOG_INFO( gDragAndDropLogFilter, Debug::Concise, "EcoreEventDndEnter\n" );
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a dragged item is moved within our window.
   * This is when items are dragged INTO our window.
   */
  static bool EcoreEventDndPosition( void* data, int type, TWinEventInfo *event )
  {
    DALI_LOG_INFO(gDragAndDropLogFilter, Debug::Concise, "EcoreEventDndPosition\n" );
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a dragged item leaves our window's bounds.
   * This is when items are dragged INTO our window.
   */
  static bool EcoreEventDndLeave( void* data, int type, TWinEventInfo *event )
  {
    DALI_LOG_INFO(gDragAndDropLogFilter, Debug::Concise, "EcoreEventDndLeave\n" );
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when the dragged item is dropped within our window's bounds.
   * This is when items are dragged INTO our window.
   */
  static bool EcoreEventDndDrop( void* data, int type, TWinEventInfo *event )
  {
    DALI_LOG_INFO(gDragAndDropLogFilter, Debug::Concise, "EcoreEventDndDrop\n" );
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a dragged item is moved from our window and the target window has done processing it.
   * This is when items are dragged FROM our window.
   */
  static bool EcoreEventDndFinished( void* data, int type, TWinEventInfo *event )
  {
    DALI_LOG_INFO(gDragAndDropLogFilter, Debug::Concise, "EcoreEventDndFinished\n" );
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when a dragged item is moved from our window and the target window has sent us a status.
   * This is when items are dragged FROM our window.
   */
  static bool EcoreEventDndStatus( void* data, int type, TWinEventInfo *event )
  {
    DALI_LOG_INFO(gDragAndDropLogFilter, Debug::Concise, "EcoreEventDndStatus\n" );
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when the client messages (i.e. the accessibility events) are received.
   */
  static bool EcoreEventClientMessage( void* data, int type, TWinEventInfo *event )
  {
    return ECORE_CALLBACK_PASS_ON;
  }


  /////////////////////////////////////////////////////////////////////////////////////////////////
  // ElDBus Accessibility Callbacks
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * Called when the source window notifies us the content in clipboard is selected.
   */
  static bool EcoreEventSelectionClear( void* data, int type, TWinEventInfo *event )
  {
    DALI_LOG_INFO(gSelectionEventLogFilter, Debug::Concise, "EcoreEventSelectionClear\n" );
    return ECORE_CALLBACK_PASS_ON;
  }

  /**
   * Called when the source window sends us about the selected content.
   * For example, when dragged items are dragged INTO our window or when items are selected in the clipboard.
   */
  static bool EcoreEventSelectionNotify( void* data, int type, TWinEventInfo *event )
  {
    DALI_LOG_INFO(gSelectionEventLogFilter, Debug::Concise, "EcoreEventSelectionNotify\n" );
    return ECORE_CALLBACK_PASS_ON;
  }

  // Data
  EventHandler* mHandler;
  std::vector<Ecore_Event_Handler> mEcoreEventHandler;
  Ecore_Win_Window mWindow;
  int mXiDeviceId;

#ifdef DALI_ELDBUS_AVAILABLE
  Eldbus_Connection* mSessionConnection;
  Eldbus_Connection* mA11yConnection;
#endif
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
  Ecore_Win_Window window = 0;

  // this code only works with the WindowRenderSurface so need to downcast
  WindowRenderSurfaceEcoreWin* ecoreSurface = static_cast< WindowRenderSurfaceEcoreWin* >( surface );
  if( ecoreSurface )
  {
    // enable multi touch
    window = ecoreSurface->GetWinWindow();
  }

  mImpl = new Impl(this, window);
}

EventHandler::~EventHandler()
{
  delete mImpl;

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
    DALI_LOG_INFO(gTouchEventLogFilter, Debug::General, "%d: Device %d: Button state %d (%.2f, %.2f)\n", timeStamp, point.GetDeviceId(), point.GetState(), point.GetScreenPosition().x, point.GetScreenPosition().y);

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

  // Send to KeyEvent Core.
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
  }
}

void EventHandler::SendRotationRequestEvent( )
{
  if( mRotationObserver != NULL )
  {
    mRotationObserver->OnRotationRequest( );
  }
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

