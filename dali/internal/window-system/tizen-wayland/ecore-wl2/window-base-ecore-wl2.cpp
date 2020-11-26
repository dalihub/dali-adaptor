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

// Ecore is littered with C style cast
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

// CLASS HEADER
#include <dali/internal/window-system/tizen-wayland/ecore-wl2/window-base-ecore-wl2.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-system.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/input/common/key-impl.h>

// EXTERNAL_HEADERS
#include <dali/public-api/object/any.h>
#include <dali/public-api/events/mouse-button.h>
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/integration-api/debug.h>
#include <Ecore_Input.h>
#include <vconf.h>
#include <vconf-keys.h>
#include <wayland-egl-tizen.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowBaseLogFilter = Debug::Filter::New( Debug::NoLogging, false, "LOG_WINDOW_BASE" );
#endif

const uint32_t MAX_TIZEN_CLIENT_VERSION = 7;
const unsigned int PRIMARY_TOUCH_BUTTON_ID = 1;

const char* DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_NAME = "db/setting/accessibility/font_name";  // It will be update at vconf-key.h and replaced.

// DBUS accessibility
const char* BUS = "org.enlightenment.wm-screen-reader";
const char* INTERFACE = "org.tizen.GestureNavigation";
const char* PATH = "/org/tizen/GestureNavigation";

struct KeyCodeMap
{
  xkb_keysym_t keySym;
  xkb_keycode_t keyCode;
  bool isKeyCode;
};

/**
 * Get the device name from the provided ecore key event
 */
void GetDeviceName( Ecore_Event_Key* keyEvent, std::string& result )
{
  const char* ecoreDeviceName = ecore_device_name_get( keyEvent->dev );

  if( ecoreDeviceName )
  {
    result = ecoreDeviceName;
  }
}

/**
 * Get the device class from the provided ecore event
 */
void GetDeviceClass( Ecore_Device_Class ecoreDeviceClass, Device::Class::Type& deviceClass )
{
  switch( ecoreDeviceClass )
  {
    case ECORE_DEVICE_CLASS_SEAT:
    {
      deviceClass = Device::Class::USER;
      break;
    }
    case ECORE_DEVICE_CLASS_KEYBOARD:
    {
      deviceClass = Device::Class::KEYBOARD;
      break;
    }
    case ECORE_DEVICE_CLASS_MOUSE:
    {
      deviceClass = Device::Class::MOUSE;
      break;
    }
    case ECORE_DEVICE_CLASS_TOUCH:
    {
      deviceClass = Device::Class::TOUCH;
      break;
    }
    case ECORE_DEVICE_CLASS_PEN:
    {
      deviceClass = Device::Class::PEN;
      break;
    }
    case ECORE_DEVICE_CLASS_POINTER:
    {
      deviceClass = Device::Class::POINTER;
      break;
    }
    case ECORE_DEVICE_CLASS_GAMEPAD:
    {
      deviceClass = Device::Class::GAMEPAD;
      break;
    }
    default:
    {
      deviceClass = Device::Class::NONE;
      break;
    }
  }
}

void GetDeviceSubclass( Ecore_Device_Subclass ecoreDeviceSubclass, Device::Subclass::Type& deviceSubclass )
{
  switch( ecoreDeviceSubclass )
  {
    case ECORE_DEVICE_SUBCLASS_FINGER:
    {
      deviceSubclass = Device::Subclass::FINGER;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_FINGERNAIL:
    {
      deviceSubclass = Device::Subclass::FINGERNAIL;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_KNUCKLE:
    {
      deviceSubclass = Device::Subclass::KNUCKLE;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_PALM:
    {
      deviceSubclass = Device::Subclass::PALM;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_HAND_SIZE:
    {
      deviceSubclass = Device::Subclass::HAND_SIDE;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_HAND_FLAT:
    {
      deviceSubclass = Device::Subclass::HAND_FLAT;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_PEN_TIP:
    {
      deviceSubclass = Device::Subclass::PEN_TIP;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_TRACKPAD:
    {
      deviceSubclass = Device::Subclass::TRACKPAD;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_TRACKPOINT:
    {
      deviceSubclass = Device::Subclass::TRACKPOINT;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_TRACKBALL:
    {
      deviceSubclass = Device::Subclass::TRACKBALL;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_REMOCON:
    {
      deviceSubclass = Device::Subclass::REMOCON;
      break;
    }
    case ECORE_DEVICE_SUBCLASS_VIRTUAL_KEYBOARD:
    {
      deviceSubclass = Device::Subclass::VIRTUAL_KEYBOARD;
      break;
    }
    default:
    {
      deviceSubclass = Device::Subclass::NONE;
      break;
    }
  }
}


void FindKeyCode( struct xkb_keymap* keyMap, xkb_keycode_t key, void* data )
{
  KeyCodeMap* foundKeyCode = static_cast< KeyCodeMap* >( data );
  if( foundKeyCode->isKeyCode )
  {
    return;
  }

  xkb_keysym_t keySym = foundKeyCode->keySym;
  int nsyms = 0;
  const xkb_keysym_t* symsOut = NULL;

  nsyms = xkb_keymap_key_get_syms_by_level( keyMap, key, 0, 0, &symsOut );

  if( nsyms && symsOut )
  {
    if( *symsOut == keySym )
    {
      foundKeyCode->keyCode = key;
      foundKeyCode->isKeyCode = true;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Window Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/// Called when the window iconify state is changed.
static Eina_Bool EcoreEventWindowIconifyStateChanged( void* data, int type, void* event )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    return windowBase->OnIconifyStateChanged( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the window gains focus
static Eina_Bool EcoreEventWindowFocusIn( void* data, int type, void* event )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    return windowBase->OnFocusIn( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the window loses focus
static Eina_Bool EcoreEventWindowFocusOut( void* data, int type, void* event )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    return windowBase->OnFocusOut( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the output is transformed
static Eina_Bool EcoreEventOutputTransform( void* data, int type, void* event )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    return windowBase->OnOutputTransform( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the output transform should be ignored
static Eina_Bool EcoreEventIgnoreOutputTransform( void* data, int type, void* event )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    return windowBase->OnIgnoreOutputTransform( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when rotate event is recevied.
 */
static Eina_Bool EcoreEventRotate( void* data, int type, void* event )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    DALI_LOG_RELEASE_INFO( "WindowBaseEcoreWl2::EcoreEventRotate\n" );
    windowBase->OnRotation( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when configure event is recevied.
 */
static Eina_Bool EcoreEventConfigure( void* data, int type, void* event )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->OnConfiguration( data, type, event );
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
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
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
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
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
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->OnMouseButtonMove( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a touch is canceled.
 */
static Eina_Bool EcoreEventMouseButtonCancel( void* data, int type, void* event )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->OnMouseButtonCancel( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a mouse wheel is received.
 */
static Eina_Bool EcoreEventMouseWheel( void* data, int type, void* event )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->OnMouseWheel( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a detent rotation event is recevied.
 */
static Eina_Bool EcoreEventDetentRotation( void* data, int type, void* event )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->OnDetentRotation( data, type, event );
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
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
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
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->OnKeyUp( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Selection Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when the source window notifies us the content in clipboard is selected.
 */
static Eina_Bool EcoreEventDataSend( void* data, int type, void* event )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->OnDataSend( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
* Called when the source window sends us about the selected content.
* For example, when item is selected in the clipboard.
*/
static Eina_Bool EcoreEventDataReceive( void* data, int type, void* event )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->OnDataReceive( data, type, event );
  }
  return ECORE_CALLBACK_PASS_ON;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Effect Start/End Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when transition animation of the window's shown/hidden is started by window manager.
 */
static Eina_Bool EcoreEventEffectStart(void *data, int type, void *event)
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  Ecore_Wl2_Event_Effect_Start *effectStart = static_cast<Ecore_Wl2_Event_Effect_Start*>( event );
  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::EcoreEventEffectStart, effect type[ %d ]\n", effectStart->type );
  if( windowBase )
  {
    if( effectStart->type < 3 ) // only under restack
    {
      windowBase->OnTransitionEffectEvent( WindowEffectState::START, static_cast<WindowEffectType>( effectStart->type ) );
    }
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when transition animation of the window's shown/hidden is ended by window manager.
 */
static Eina_Bool EcoreEventEffectEnd(void *data, int type, void *event)
{
  Ecore_Wl2_Event_Effect_Start *effectEnd = static_cast<Ecore_Wl2_Event_Effect_Start*>( event );
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::EcoreEventEffectEnd, effect type[ %d ]\n", effectEnd->type );
  if( windowBase )
  {
    if( effectEnd->type < 3 ) // only under restack
    {
      windowBase->OnTransitionEffectEvent( WindowEffectState::END, static_cast<WindowEffectType>( effectEnd->type ) );
    }
  }
  return ECORE_CALLBACK_PASS_ON;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Keyboard Repeat Settings Changed Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

static Eina_Bool EcoreEventSeatKeyboardRepeatChanged(void *data, int type, void *event)
{
   Ecore_Wl2_Event_Seat_Keyboard_Repeat_Changed *keyboardRepeat = static_cast<Ecore_Wl2_Event_Seat_Keyboard_Repeat_Changed*>( event );
   WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
   DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::EcoreEventSeatKeyboardRepeatChanged, id[ %d ]\n", keyboardRepeat->id );
   if( windowBase )
   {
     windowBase->OnKeyboardRepeatSettingsChanged();
   }

  return ECORE_CALLBACK_RENEW;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Keymap Changed Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

static Eina_Bool EcoreEventSeatKeymapChanged(void *data, int type, void *event)
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->KeymapChanged( data, type, event );
  }

  return ECORE_CALLBACK_RENEW;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Font Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a font name is changed.
 */
static void VconfNotifyFontNameChanged( keynode_t* node, void* data )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->OnFontNameChanged();
  }
}

/**
 * Called when a font size is changed.
 */
static void VconfNotifyFontSizeChanged( keynode_t* node, void* data )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->OnFontSizeChanged();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Window Redraw Request Event Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

static Eina_Bool EcoreEventWindowRedrawRequest(void *data, int type, void *event)
{
  Ecore_Wl2_Event_Window_Redraw_Request *windowRedrawRequest = static_cast<Ecore_Wl2_Event_Window_Redraw_Request *>(event);
  WindowBaseEcoreWl2 *windowBase = static_cast<WindowBaseEcoreWl2 *>(data);
  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::EcoreEventWindowRedrawRequest, window[ %d ]\n", windowRedrawRequest->win );
  if ( windowBase )
  {
    windowBase->OnEcoreEventWindowRedrawRequest();
  }

  return ECORE_CALLBACK_RENEW;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// ElDBus Accessibility Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DALI_ELDBUS_AVAILABLE
// Callback for Ecore ElDBus accessibility events.
static void EcoreElDBusAccessibilityNotification( void* context, const Eldbus_Message* message )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( context );
  if( windowBase )
  {
    windowBase->OnEcoreElDBusAccessibilityNotification( context, message );
  }
}
#endif // DALI_ELDBUS_AVAILABLE

static void RegistryGlobalCallback( void* data, struct wl_registry *registry, uint32_t name, const char* interface, uint32_t version )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->RegistryGlobalCallback( data, registry, name, interface, version );
  }
}

static void RegistryGlobalCallbackRemove( void* data, struct wl_registry* registry, uint32_t id )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->RegistryGlobalCallbackRemove( data, registry, id );
  }
}

static void TizenPolicyConformant( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t isConformant )
{
}

static void TizenPolicyConformantArea( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t conformantPart, uint32_t state, int32_t x, int32_t y, int32_t w, int32_t h )
{
}

static void TizenPolicyNotificationChangeDone(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, int32_t level, uint32_t state )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->TizenPolicyNotificationChangeDone( data, tizenPolicy, surface, level, state );
  }
}

static void TizenPolicyTransientForDone( void* data, struct tizen_policy* tizenPolicy, uint32_t childId )
{
}

static void TizenPolicyScreenModeChangeDone( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t mode, uint32_t state )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->TizenPolicyScreenModeChangeDone( data, tizenPolicy, surface, mode, state );
  }
}

static void TizenPolicyIconifyStateChanged( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t iconified, uint32_t force )
{
}

static void TizenPolicySupportedAuxiliaryHints( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, struct wl_array* hints, uint32_t numNints )
{
}

static void TizenPolicyAllowedAuxiliaryHint( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, int id )
{
}

static void TizenPolicyAuxiliaryMessage( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, const char* key, const char* val, struct wl_array* options )
{
}

static void TizenPolicyConformantRegion( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t conformantPart, uint32_t state, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t serial )
{
}

static void DisplayPolicyBrightnessChangeDone( void* data, struct tizen_display_policy *displayPolicy, struct wl_surface* surface, int32_t brightness, uint32_t state )
{
  WindowBaseEcoreWl2* windowBase = static_cast< WindowBaseEcoreWl2* >( data );
  if( windowBase )
  {
    windowBase->DisplayPolicyBrightnessChangeDone( data, displayPolicy, surface, brightness, state );
  }
}

const struct wl_registry_listener registryListener =
{
   RegistryGlobalCallback,
   RegistryGlobalCallbackRemove
};

const struct tizen_policy_listener tizenPolicyListener =
{
   TizenPolicyConformant,
   TizenPolicyConformantArea,
   TizenPolicyNotificationChangeDone,
   TizenPolicyTransientForDone,
   TizenPolicyScreenModeChangeDone,
   TizenPolicyIconifyStateChanged,
   TizenPolicySupportedAuxiliaryHints,
   TizenPolicyAllowedAuxiliaryHint,
   TizenPolicyAuxiliaryMessage,
   TizenPolicyConformantRegion
};

const struct tizen_display_policy_listener tizenDisplayPolicyListener =
{
  DisplayPolicyBrightnessChangeDone
};

} // unnamed namespace

WindowBaseEcoreWl2::WindowBaseEcoreWl2( Dali::PositionSize positionSize, Any surface, bool isTransparent )
: mEcoreEventHandler(),
  mEcoreWindow( NULL ),
  mWlSurface( NULL ),
  mEglWindow( NULL ),
  mDisplay( NULL ),
  mEventQueue( NULL ),
  mTizenPolicy( NULL ),
  mTizenDisplayPolicy( NULL ),
  mKeyMap( NULL ),
  mSupportedAuxiliaryHints(),
  mAuxiliaryHints(),
  mNotificationLevel( -1 ),
  mNotificationChangeState( 0 ),
  mNotificationLevelChangeDone( true ),
  mScreenOffMode( 0 ),
  mScreenOffModeChangeState( 0 ),
  mScreenOffModeChangeDone( true ),
  mBrightness( 0 ),
  mBrightnessChangeState( 0 ),
  mBrightnessChangeDone( true ),
  mVisible( true ),
  mWindowPositionSize( positionSize ),
  mOwnSurface( false ),
  mMoveResizeSerial( 0 ),
  mLastSubmittedMoveResizeSerial( 0 )
#ifdef DALI_ELDBUS_AVAILABLE
  , mSystemConnection( NULL )
#endif
{
  Initialize( positionSize, surface, isTransparent );
}

WindowBaseEcoreWl2::~WindowBaseEcoreWl2()
{
#ifdef DALI_ELDBUS_AVAILABLE
    // Close down ElDBus connections.
    if( mSystemConnection )
    {
      eldbus_connection_unref( mSystemConnection );
    }
#endif // DALI_ELDBUS_AVAILABLE

  vconf_ignore_key_changed( VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, VconfNotifyFontSizeChanged );
  vconf_ignore_key_changed( DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_NAME, VconfNotifyFontNameChanged );

  for( Dali::Vector< Ecore_Event_Handler* >::Iterator iter = mEcoreEventHandler.Begin(), endIter = mEcoreEventHandler.End(); iter != endIter; ++iter )
  {
    ecore_event_handler_del( *iter );
  }
  mEcoreEventHandler.Clear();

  if( mEventQueue )
  {
    wl_event_queue_destroy( mEventQueue );
  }

  mSupportedAuxiliaryHints.clear();
  mAuxiliaryHints.clear();

  if( mEglWindow != NULL )
  {
    wl_egl_window_destroy( mEglWindow );
    mEglWindow = NULL;
  }

  if( mOwnSurface )
  {
    ecore_wl2_window_free( mEcoreWindow );

    WindowSystem::Shutdown();
  }
}

void WindowBaseEcoreWl2::Initialize( PositionSize positionSize, Any surface, bool isTransparent )
{
  if( surface.Empty() == false )
  {
    // check we have a valid type
    DALI_ASSERT_ALWAYS( ( surface.GetType() == typeid (Ecore_Wl2_Window *) ) && "Surface type is invalid" );

    mEcoreWindow = AnyCast< Ecore_Wl2_Window* >( surface );
  }
  else
  {
    // we own the surface about to created
    WindowSystem::Initialize();

    mOwnSurface = true;
    CreateWindow( positionSize );
  }

  mWlSurface = ecore_wl2_window_surface_get( mEcoreWindow );

  SetTransparency( isTransparent );

  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL2_EVENT_WINDOW_ICONIFY_STATE_CHANGE,  EcoreEventWindowIconifyStateChanged, this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL2_EVENT_FOCUS_IN,                     EcoreEventWindowFocusIn,             this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL2_EVENT_FOCUS_OUT,                    EcoreEventWindowFocusOut,            this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL2_EVENT_OUTPUT_TRANSFORM,             EcoreEventOutputTransform,           this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL2_EVENT_IGNORE_OUTPUT_TRANSFORM,      EcoreEventIgnoreOutputTransform,     this ) );

  // Register Rotate event
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL2_EVENT_WINDOW_ROTATE,                EcoreEventRotate,                    this ) );

  // Register Configure event
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL2_EVENT_WINDOW_CONFIGURE,             EcoreEventConfigure,                 this ) );

  // Register Touch events
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_BUTTON_DOWN,                EcoreEventMouseButtonDown,           this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_BUTTON_UP,                  EcoreEventMouseButtonUp,             this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_MOVE,                       EcoreEventMouseButtonMove,           this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_BUTTON_CANCEL,              EcoreEventMouseButtonCancel,         this ) );

  // Register Mouse wheel events
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_WHEEL,                      EcoreEventMouseWheel,                this ) );

  // Register Detent event
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_DETENT_ROTATE,                    EcoreEventDetentRotation,            this ) );

  // Register Key events
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_KEY_DOWN,                         EcoreEventKeyDown,                   this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_KEY_UP,                           EcoreEventKeyUp,                     this ) );

  // Register Selection event - clipboard selection
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL2_EVENT_DATA_SOURCE_SEND,             EcoreEventDataSend,                  this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL2_EVENT_SELECTION_DATA_READY,         EcoreEventDataReceive,               this ) );

  // Register Effect Start/End event
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL2_EVENT_EFFECT_START,                 EcoreEventEffectStart,               this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL2_EVENT_EFFECT_END,                   EcoreEventEffectEnd,                 this ) );

  // Register Keyboard repeat event
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL2_EVENT_SEAT_KEYBOARD_REPEAT_CHANGED, EcoreEventSeatKeyboardRepeatChanged, this ) );

  // Register Window redraw request event
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL2_EVENT_WINDOW_REDRAW_REQUEST,        EcoreEventWindowRedrawRequest,       this ) );

  // Register Vconf notify - font name and size
  vconf_notify_key_changed( DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_NAME, VconfNotifyFontNameChanged, this );
  vconf_notify_key_changed( VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, VconfNotifyFontSizeChanged, this );

  InitializeEcoreElDBus();

  Ecore_Wl2_Display* display = ecore_wl2_connected_display_get( NULL );
  mDisplay = ecore_wl2_display_get( display );

  if( mDisplay )
  {
    wl_display* displayWrapper = static_cast< wl_display* >( wl_proxy_create_wrapper( mDisplay ) );
    if( displayWrapper )
    {
      mEventQueue = wl_display_create_queue( mDisplay );
      if( mEventQueue )
      {
        wl_proxy_set_queue( reinterpret_cast< wl_proxy* >( displayWrapper ), mEventQueue );

        wl_registry* registry = wl_display_get_registry( displayWrapper );
        wl_registry_add_listener( registry, &registryListener, this );
      }

      wl_proxy_wrapper_destroy( displayWrapper );
    }
  }

  Ecore_Wl2_Input* ecoreWlInput = ecore_wl2_input_default_input_get( display );

  if( ecoreWlInput )
  {
    mKeyMap = ecore_wl2_input_keymap_get( ecoreWlInput );

    mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL2_EVENT_SEAT_KEYMAP_CHANGED, EcoreEventSeatKeymapChanged, this ) );
  }

  // get auxiliary hint
  Eina_List* hints = ecore_wl2_window_aux_hints_supported_get( mEcoreWindow );
  if( hints )
  {
    Eina_List* l = NULL;
    char* hint = NULL;

    for( l = hints, ( hint =  static_cast< char* >( eina_list_data_get(l) ) ); l; l = eina_list_next(l), ( hint = static_cast< char* >( eina_list_data_get(l) ) ) )
    {
      mSupportedAuxiliaryHints.push_back( hint );

      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::Initialize: %s\n", hint );
    }
  }
}

Eina_Bool WindowBaseEcoreWl2::OnIconifyStateChanged( void* data, int type, void* event )
{
  Ecore_Wl2_Event_Window_Iconify_State_Change* iconifyChangedEvent( static_cast< Ecore_Wl2_Event_Window_Iconify_State_Change* >( event ) );
  Eina_Bool handled( ECORE_CALLBACK_PASS_ON );

  if( iconifyChangedEvent->win == static_cast< unsigned int>( ecore_wl2_window_id_get( mEcoreWindow ) ) )
  {
    if( iconifyChangedEvent->iconified == EINA_TRUE )
    {
      mIconifyChangedSignal.Emit( true );
    }
    else
    {
      mIconifyChangedSignal.Emit( false );
    }
    handled = ECORE_CALLBACK_DONE;
  }

  return handled;
}

Eina_Bool WindowBaseEcoreWl2::OnFocusIn( void* data, int type, void* event )
{
  Ecore_Wl2_Event_Focus_In* focusInEvent( static_cast< Ecore_Wl2_Event_Focus_In* >( event ) );

  if( focusInEvent->window == static_cast< unsigned int >( ecore_wl2_window_id_get( mEcoreWindow ) ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window EcoreEventWindowFocusIn\n" );

    mFocusChangedSignal.Emit( true );
  }

  return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool WindowBaseEcoreWl2::OnFocusOut( void* data, int type, void* event )
{
  Ecore_Wl2_Event_Focus_Out* focusOutEvent( static_cast< Ecore_Wl2_Event_Focus_Out* >( event ) );

  if( focusOutEvent->window == static_cast< unsigned int >( ecore_wl2_window_id_get( mEcoreWindow ) ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window EcoreEventWindowFocusOut\n" );

    mFocusChangedSignal.Emit( false );
  }

  return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool WindowBaseEcoreWl2::OnOutputTransform( void* data, int type, void* event )
{
  Ecore_Wl2_Event_Output_Transform* transformEvent( static_cast< Ecore_Wl2_Event_Output_Transform* >( event ) );

  if( transformEvent->output == ecore_wl2_window_output_find( mEcoreWindow ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%p) EcoreEventOutputTransform\n", mEcoreWindow );

    mOutputTransformedSignal.Emit();
  }

  return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool WindowBaseEcoreWl2::OnIgnoreOutputTransform( void* data, int type, void* event )
{
  Ecore_Wl2_Event_Ignore_Output_Transform* ignoreTransformEvent( static_cast< Ecore_Wl2_Event_Ignore_Output_Transform* >( event ) );

  if( ignoreTransformEvent->win == mEcoreWindow )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%p) EcoreEventIgnoreOutputTransform\n", mEcoreWindow );

    mOutputTransformedSignal.Emit();
  }

  return ECORE_CALLBACK_PASS_ON;
}

void WindowBaseEcoreWl2::OnRotation( void* data, int type, void* event )
{
  Ecore_Wl2_Event_Window_Rotation* ev( static_cast< Ecore_Wl2_Event_Window_Rotation* >( event ) );

  if( ev->win == static_cast< unsigned int >( ecore_wl2_window_id_get( mEcoreWindow ) ) )
  {
    DALI_LOG_RELEASE_INFO( "WindowBaseEcoreWl2::OnRotation, angle: %d, width: %d, height: %d\n", ev->angle, ev->w, ev->h );

    RotationEvent rotationEvent;
    rotationEvent.angle = ev->angle;
    rotationEvent.winResize = 0;

    if( ev->angle == 0 || ev->angle == 180 )
    {
      rotationEvent.width = ev->w;
      rotationEvent.height = ev->h;
    }
    else
    {
      rotationEvent.width = ev->h;
      rotationEvent.height = ev->w;
    }

    mWindowPositionSize.width = rotationEvent.width;
    mWindowPositionSize.height = rotationEvent.height;

    mRotationSignal.Emit( rotationEvent );
  }
}

void WindowBaseEcoreWl2::OnConfiguration( void* data, int type, void* event )
{
  Ecore_Wl2_Event_Window_Configure* ev( static_cast< Ecore_Wl2_Event_Window_Configure* >( event ) );

  if( ev->win == static_cast< unsigned int >( ecore_wl2_window_id_get( mEcoreWindow ) ) )
  {
    // Note: To comply with the wayland protocol, Dali should make an ack_configure
    // by calling ecore_wl2_window_commit
    ecore_wl2_window_commit(mEcoreWindow, EINA_FALSE);
  }
}

void WindowBaseEcoreWl2::OnMouseButtonDown( void* data, int type, void* event )
{
  Ecore_Event_Mouse_Button* touchEvent = static_cast< Ecore_Event_Mouse_Button* >( event );

  if( touchEvent->window == static_cast< unsigned int >( ecore_wl2_window_id_get( mEcoreWindow ) ) )
  {
    Device::Class::Type deviceClass;
    Device::Subclass::Type deviceSubclass;

    GetDeviceClass( ecore_device_class_get( touchEvent->dev ), deviceClass );
    GetDeviceSubclass( ecore_device_subclass_get( touchEvent->dev ), deviceSubclass );

    PointState::Type state ( PointState::DOWN );

    if( deviceClass != Device::Class::Type::MOUSE )
    {
      // Check if the buttons field is set and ensure it's the primary touch button.
      // If this event was triggered by buttons other than the primary button (used for touch), then
      // just send an interrupted event to Core.
      if( touchEvent->buttons && (touchEvent->buttons != PRIMARY_TOUCH_BUTTON_ID ) )
      {
        state = PointState::INTERRUPTED;
      }
    }

    Integration::Point point;
    point.SetDeviceId( touchEvent->multi.device );
    point.SetState( state );
    point.SetScreenPosition( Vector2( touchEvent->x, touchEvent->y ) );
    point.SetRadius( touchEvent->multi.radius, Vector2( touchEvent->multi.radius_x, touchEvent->multi.radius_y ) );
    point.SetPressure( touchEvent->multi.pressure );
    point.SetAngle( Degree( touchEvent->multi.angle ) );
    point.SetDeviceClass( deviceClass );
    point.SetDeviceSubclass( deviceSubclass );
    point.SetMouseButton( static_cast< MouseButton::Type >( touchEvent->buttons) );

    mTouchEventSignal.Emit( point, touchEvent->timestamp );
  }
}

void WindowBaseEcoreWl2::OnMouseButtonUp( void* data, int type, void* event )
{
  Ecore_Event_Mouse_Button* touchEvent = static_cast< Ecore_Event_Mouse_Button* >( event );

  if( touchEvent->window == static_cast< unsigned int >( ecore_wl2_window_id_get( mEcoreWindow ) ) )
  {
    Device::Class::Type deviceClass;
    Device::Subclass::Type deviceSubclass;

    GetDeviceClass( ecore_device_class_get( touchEvent->dev ), deviceClass );
    GetDeviceSubclass( ecore_device_subclass_get( touchEvent->dev ), deviceSubclass );

    Integration::Point point;
    point.SetDeviceId( touchEvent->multi.device );
    point.SetState( PointState::UP );
    point.SetScreenPosition( Vector2( touchEvent->x, touchEvent->y ) );
    point.SetRadius( touchEvent->multi.radius, Vector2( touchEvent->multi.radius_x, touchEvent->multi.radius_y ) );
    point.SetPressure( touchEvent->multi.pressure );
    point.SetAngle( Degree( touchEvent->multi.angle ) );
    point.SetDeviceClass( deviceClass );
    point.SetDeviceSubclass( deviceSubclass );
    point.SetMouseButton( static_cast< MouseButton::Type >( touchEvent->buttons) );

    mTouchEventSignal.Emit( point, touchEvent->timestamp );
  }
}

void WindowBaseEcoreWl2::OnMouseButtonMove( void* data, int type, void* event )
{
  Ecore_Event_Mouse_Move* touchEvent = static_cast< Ecore_Event_Mouse_Move* >( event );

  if( touchEvent->window == static_cast< unsigned int >( ecore_wl2_window_id_get( mEcoreWindow ) ) )
  {
    Device::Class::Type deviceClass;
    Device::Subclass::Type deviceSubclass;

    GetDeviceClass( ecore_device_class_get( touchEvent->dev ), deviceClass );
    GetDeviceSubclass( ecore_device_subclass_get( touchEvent->dev ), deviceSubclass );

    Integration::Point point;
    point.SetDeviceId( touchEvent->multi.device );
    point.SetState( PointState::MOTION );
    point.SetScreenPosition( Vector2( touchEvent->x, touchEvent->y ) );
    point.SetRadius( touchEvent->multi.radius, Vector2( touchEvent->multi.radius_x, touchEvent->multi.radius_y ) );
    point.SetPressure( touchEvent->multi.pressure );
    point.SetAngle( Degree( touchEvent->multi.angle ) );
    point.SetDeviceClass( deviceClass );
    point.SetDeviceSubclass( deviceSubclass );

    mTouchEventSignal.Emit( point, touchEvent->timestamp );
  }
}

void WindowBaseEcoreWl2::OnMouseButtonCancel( void* data, int type, void* event )
{
  Ecore_Event_Mouse_Button* touchEvent = static_cast< Ecore_Event_Mouse_Button* >( event );

  if( touchEvent->window == static_cast< unsigned int >( ecore_wl2_window_id_get( mEcoreWindow ) ) )
  {
    Integration::Point point;
    point.SetDeviceId( touchEvent->multi.device );
    point.SetState( PointState::INTERRUPTED );
    point.SetScreenPosition( Vector2( 0.0f, 0.0f ) );

    mTouchEventSignal.Emit( point, touchEvent->timestamp );

    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::OnMouseButtonCancel\n" );
  }
}

void WindowBaseEcoreWl2::OnMouseWheel( void* data, int type, void* event )
{
  Ecore_Event_Mouse_Wheel* mouseWheelEvent = static_cast< Ecore_Event_Mouse_Wheel* >( event );

  if( mouseWheelEvent->window == static_cast< unsigned int >( ecore_wl2_window_id_get( mEcoreWindow ) ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::OnMouseWheel: direction: %d, modifiers: %d, x: %d, y: %d, z: %d\n", mouseWheelEvent->direction, mouseWheelEvent->modifiers, mouseWheelEvent->x, mouseWheelEvent->y, mouseWheelEvent->z );

    Integration::WheelEvent wheelEvent( Integration::WheelEvent::MOUSE_WHEEL, mouseWheelEvent->direction, mouseWheelEvent->modifiers, Vector2( mouseWheelEvent->x, mouseWheelEvent->y ), mouseWheelEvent->z, mouseWheelEvent->timestamp );

    mWheelEventSignal.Emit( wheelEvent );
  }
}

void WindowBaseEcoreWl2::OnDetentRotation( void* data, int type, void* event )
{
  Ecore_Event_Detent_Rotate* detentEvent = static_cast< Ecore_Event_Detent_Rotate* >( event );

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl::OnDetentRotation\n" );

  int direction = ( detentEvent->direction == ECORE_DETENT_DIRECTION_CLOCKWISE ) ? 1 : -1;
  int timeStamp = detentEvent->timestamp;

  Integration::WheelEvent wheelEvent( Integration::WheelEvent::CUSTOM_WHEEL, direction, 0, Vector2( 0.0f, 0.0f ), 0, timeStamp );

  mWheelEventSignal.Emit( wheelEvent );
}

void WindowBaseEcoreWl2::OnKeyDown( void* data, int type, void* event )
{
  Ecore_Event_Key* keyEvent = static_cast< Ecore_Event_Key* >( event );

  if( keyEvent->window == static_cast< unsigned int >( ecore_wl2_window_id_get( mEcoreWindow ) ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::OnKeyDown\n" );

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

    int keyCode = 0;
    GetKeyCode( keyName, keyCode ); // Get key code dynamically.

    if( keyCode == 0 )
    {
      // Get a specific key code from dali key look up table.
      keyCode = KeyLookup::GetDaliKeyCode( keyEvent->keyname );
    }

    keyCode = ( keyCode == -1 ) ? 0 : keyCode;
    int modifier( keyEvent->modifiers );
    unsigned long time = keyEvent->timestamp;
    if( !strncmp( keyEvent->keyname, "Keycode-", 8 ) )
    {
      keyCode = atoi( keyEvent->keyname + 8 );
    }

    // Ensure key event string is not NULL as keys like SHIFT have a null string.
    if( keyEvent->string )
    {
      keyString = keyEvent->string;
    }

    std::string deviceName;
    Device::Class::Type deviceClass;
    Device::Subclass::Type deviceSubclass;

    GetDeviceName( keyEvent, deviceName );
    GetDeviceClass( ecore_device_class_get( keyEvent->dev ), deviceClass );
    GetDeviceSubclass( ecore_device_subclass_get( keyEvent->dev ), deviceSubclass );

    Integration::KeyEvent keyEvent( keyName, logicalKey, keyString, keyCode, modifier, time, Integration::KeyEvent::DOWN, compose, deviceName, deviceClass, deviceSubclass );

     mKeyEventSignal.Emit( keyEvent );
  }
}

void WindowBaseEcoreWl2::OnKeyUp( void* data, int type, void* event )
{
  Ecore_Event_Key* keyEvent = static_cast< Ecore_Event_Key* >( event );

  if( keyEvent->window == static_cast< unsigned int >( ecore_wl2_window_id_get( mEcoreWindow ) ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::OnKeyUp\n" );

#if defined(ECORE_VERSION_MAJOR) && (ECORE_VERSION_MAJOR >= 1) && defined(ECORE_VERSION_MINOR) && (ECORE_VERSION_MINOR >= 23)
    // Cancel processing flag is sent because this key event will combine with the previous key. So, the event should not actually perform anything.
    if( keyEvent->event_flags & ECORE_EVENT_FLAG_CANCEL )
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::OnKeyUp: This event flag indicates the event is canceled. \n" );
      return;
    }
#endif // Since ecore 1.23 version

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

    int keyCode = 0;
    GetKeyCode( keyName, keyCode ); // Get key code dynamically.

    if( keyCode == 0 )
    {
      // Get a specific key code from dali key look up table.
      keyCode = KeyLookup::GetDaliKeyCode( keyEvent->keyname );
    }

    keyCode = ( keyCode == -1 ) ? 0 : keyCode;
    int modifier( keyEvent->modifiers );
    unsigned long time = keyEvent->timestamp;
    if( !strncmp( keyEvent->keyname, "Keycode-", 8 ) )
    {
      keyCode = atoi( keyEvent->keyname + 8 );
    }

    // Ensure key event string is not NULL as keys like SHIFT have a null string.
    if( keyEvent->string )
    {
      keyString = keyEvent->string;
    }

    std::string deviceName;
    Device::Class::Type deviceClass;
    Device::Subclass::Type deviceSubclass;

    GetDeviceName( keyEvent, deviceName );
    GetDeviceClass( ecore_device_class_get( keyEvent->dev ), deviceClass );
    GetDeviceSubclass( ecore_device_subclass_get( keyEvent->dev ), deviceSubclass );

    Integration::KeyEvent keyEvent( keyName, logicalKey, keyString, keyCode, modifier, time, Integration::KeyEvent::UP, compose, deviceName, deviceClass, deviceSubclass );

     mKeyEventSignal.Emit( keyEvent );
  }
}

void WindowBaseEcoreWl2::OnDataSend( void* data, int type, void* event )
{
  mSelectionDataSendSignal.Emit( event );
}

void WindowBaseEcoreWl2::OnDataReceive( void* data, int type, void* event )
{
  mSelectionDataReceivedSignal.Emit( event  );
}

void WindowBaseEcoreWl2::OnFontNameChanged()
{
  mStyleChangedSignal.Emit( StyleChange::DEFAULT_FONT_CHANGE );
}

void WindowBaseEcoreWl2::OnFontSizeChanged()
{
  mStyleChangedSignal.Emit( StyleChange::DEFAULT_FONT_SIZE_CHANGE );
}

void WindowBaseEcoreWl2::OnEcoreElDBusAccessibilityNotification( void* context, const Eldbus_Message* message )
{
#ifdef DALI_ELDBUS_AVAILABLE
  AccessibilityInfo info;

  // The string defines the arg-list's respective types.
  if( !eldbus_message_arguments_get( message, "iiiiiiu", &info.gestureValue, &info.startX, &info.startY, &info.endX, &info.endY, &info.state, &info.eventTime ) )
  {
    DALI_LOG_ERROR( "OnEcoreElDBusAccessibilityNotification: Error getting arguments\n" );
  }

  mAccessibilitySignal.Emit( info );
#endif
}

void WindowBaseEcoreWl2::OnTransitionEffectEvent( WindowEffectState state, WindowEffectType type )
{
  mTransitionEffectEventSignal.Emit( state, type );
}

void WindowBaseEcoreWl2::OnKeyboardRepeatSettingsChanged()
{
  mKeyboardRepeatSettingsChangedSignal.Emit();
}

void WindowBaseEcoreWl2::OnEcoreEventWindowRedrawRequest()
{
  mWindowRedrawRequestSignal.Emit();
}

void WindowBaseEcoreWl2::KeymapChanged(void *data, int type, void *event)
{
  Ecore_Wl2_Event_Seat_Keymap_Changed *changed = static_cast<Ecore_Wl2_Event_Seat_Keymap_Changed*>( event );
  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::KeymapChanged, keymap id[ %d ]\n", changed->id );
  Ecore_Wl2_Input* ecoreWlInput = ecore_wl2_input_default_input_get( changed->display );
  if( ecoreWlInput )
  {
    mKeyMap = ecore_wl2_input_keymap_get( ecoreWlInput );
  }
}

void WindowBaseEcoreWl2::RegistryGlobalCallback( void* data, struct wl_registry *registry, uint32_t name, const char* interface, uint32_t version )
{
  if( strcmp( interface, tizen_policy_interface.name ) == 0 )
  {
    uint32_t clientVersion = std::min( version, MAX_TIZEN_CLIENT_VERSION );

    mTizenPolicy = static_cast< tizen_policy* >( wl_registry_bind( registry, name, &tizen_policy_interface, clientVersion ) );
    if( !mTizenPolicy )
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::RegistryGlobalCallback: wl_registry_bind(tizen_policy_interface) is failed.\n" );
      return;
    }

    tizen_policy_add_listener( mTizenPolicy, &tizenPolicyListener, data );

    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::RegistryGlobalCallback: tizen_policy_add_listener is called.\n" );
  }
  else if( strcmp( interface, tizen_display_policy_interface.name ) == 0 )
  {
    mTizenDisplayPolicy = static_cast< tizen_display_policy* >( wl_registry_bind( registry, name, &tizen_display_policy_interface, version ) );
    if( !mTizenDisplayPolicy )
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::RegistryGlobalCallback: wl_registry_bind(tizen_display_policy_interface) is failed.\n" );
      return;
    }

    tizen_display_policy_add_listener( mTizenDisplayPolicy, &tizenDisplayPolicyListener, data );

    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::RegistryGlobalCallback: tizen_display_policy_add_listener is called.\n" );
  }
}

void WindowBaseEcoreWl2::RegistryGlobalCallbackRemove( void* data, struct wl_registry* registry, uint32_t id )
{
  mTizenPolicy = NULL;
  mTizenDisplayPolicy = NULL;
}

void WindowBaseEcoreWl2::TizenPolicyNotificationChangeDone(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, int32_t level, uint32_t state )
{
  mNotificationLevel = level;
  mNotificationChangeState = state;
  mNotificationLevelChangeDone = true;

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::TizenPolicyNotificationChangeDone: level = %d, state = %d\n", level, state );
}

void WindowBaseEcoreWl2::TizenPolicyScreenModeChangeDone( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t mode, uint32_t state )
{
  mScreenOffMode = mode;
  mScreenOffModeChangeState = state;
  mScreenOffModeChangeDone = true;

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::TizenPolicyScreenModeChangeDone: mode = %d, state = %d\n", mode, state );
}

void WindowBaseEcoreWl2::DisplayPolicyBrightnessChangeDone( void* data, struct tizen_display_policy *displayPolicy, struct wl_surface* surface, int32_t brightness, uint32_t state )
{
  mBrightness = brightness;
  mBrightnessChangeState = state;
  mBrightnessChangeDone = true;

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::DisplayPolicyBrightnessChangeDone: brightness = %d, state = %d\n", brightness, state );
}

void WindowBaseEcoreWl2::GetKeyCode( std::string keyName, int32_t& keyCode )
{
  xkb_keysym_t sym = XKB_KEY_NoSymbol;
  KeyCodeMap foundKeyCode;

  sym = xkb_keysym_from_name( keyName.c_str(), XKB_KEYSYM_NO_FLAGS );
  if( sym == XKB_KEY_NoSymbol )
  {
    DALI_LOG_ERROR( "Failed to get keysym in WindowBaseEcoreWl2\n" );
    return;
  }

  foundKeyCode.keySym = sym;
  foundKeyCode.isKeyCode = false;
  xkb_keymap_key_for_each( mKeyMap, FindKeyCode, &foundKeyCode );
  keyCode = static_cast< int32_t >( foundKeyCode.keyCode );
}

Any WindowBaseEcoreWl2::GetNativeWindow()
{
  return mEcoreWindow;
}

int WindowBaseEcoreWl2::GetNativeWindowId()
{
  return ecore_wl2_window_id_get( mEcoreWindow );
}

EGLNativeWindowType WindowBaseEcoreWl2::CreateEglWindow( int width, int height )
{
  mEglWindow = wl_egl_window_create( mWlSurface, width, height );

  return static_cast< EGLNativeWindowType >( mEglWindow );
}

void WindowBaseEcoreWl2::DestroyEglWindow()
{
  if( mEglWindow != NULL )
  {
    wl_egl_window_destroy( mEglWindow );
    mEglWindow = NULL;
  }
}

void WindowBaseEcoreWl2::SetEglWindowRotation( int angle )
{
  wl_egl_window_tizen_rotation rotation;

  switch( angle )
  {
    case 0:
    {
      rotation = WL_EGL_WINDOW_TIZEN_ROTATION_0 ;
      break;
    }
    case 90:
    {
      rotation = WL_EGL_WINDOW_TIZEN_ROTATION_270;
      break;
    }
    case 180:
    {
      rotation = WL_EGL_WINDOW_TIZEN_ROTATION_180;
      break;
    }
    case 270:
    {
      rotation = WL_EGL_WINDOW_TIZEN_ROTATION_90;
      break;
    }
    default:
    {
      rotation = WL_EGL_WINDOW_TIZEN_ROTATION_0 ;
      break;
    }
  }

  wl_egl_window_tizen_set_rotation( mEglWindow, rotation );
}

void WindowBaseEcoreWl2::SetEglWindowBufferTransform( int angle )
{
  wl_output_transform bufferTransform;

  switch( angle )
  {
    case 0:
    {
      bufferTransform = WL_OUTPUT_TRANSFORM_NORMAL;
      break;
    }
    case 90:
    {
      bufferTransform = WL_OUTPUT_TRANSFORM_90;
      break;
    }
    case 180:
    {
      bufferTransform = WL_OUTPUT_TRANSFORM_180;
      break;
    }
    case 270:
    {
      bufferTransform = WL_OUTPUT_TRANSFORM_270;
      break;
    }
    default:
    {
      bufferTransform = WL_OUTPUT_TRANSFORM_NORMAL;
      break;
    }
  }

  wl_egl_window_tizen_set_buffer_transform( mEglWindow, bufferTransform );
}

void WindowBaseEcoreWl2::SetEglWindowTransform( int angle )
{
  wl_output_transform windowTransform;

  switch( angle )
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

  wl_egl_window_tizen_set_window_transform( mEglWindow, windowTransform );
}

void WindowBaseEcoreWl2::ResizeEglWindow( PositionSize positionSize )
{
  wl_egl_window_resize( mEglWindow, positionSize.width, positionSize.height, positionSize.x, positionSize.y );

  // Note: Both "Resize" and "MoveResize" cases can reach here, but only "MoveResize" needs to submit serial number
  if( mMoveResizeSerial != mLastSubmittedMoveResizeSerial )
  {
    wl_egl_window_tizen_set_window_serial( mEglWindow, mMoveResizeSerial );
    mLastSubmittedMoveResizeSerial = mMoveResizeSerial;
  }
}

bool WindowBaseEcoreWl2::IsEglWindowRotationSupported()
{
  // Check capability
  wl_egl_window_tizen_capability capability = static_cast< wl_egl_window_tizen_capability >( wl_egl_window_tizen_get_capabilities( mEglWindow ) );
  if( capability == WL_EGL_WINDOW_TIZEN_CAPABILITY_ROTATION_SUPPORTED )
  {
    return true;
  }

  return false;
}

void WindowBaseEcoreWl2::Move( PositionSize positionSize )
{
  mWindowPositionSize = positionSize;
  ecore_wl2_window_position_set( mEcoreWindow, positionSize.x, positionSize.y );
}

void WindowBaseEcoreWl2::Resize( PositionSize positionSize )
{
  mWindowPositionSize = positionSize;
  ecore_wl2_window_geometry_set( mEcoreWindow, positionSize.x, positionSize.y, positionSize.width, positionSize.height );
}

void WindowBaseEcoreWl2::MoveResize( PositionSize positionSize )
{
  mWindowPositionSize = positionSize;
  ecore_wl2_window_sync_geometry_set( mEcoreWindow, ++mMoveResizeSerial, positionSize.x, positionSize.y, positionSize.width, positionSize.height );
}

void WindowBaseEcoreWl2::SetClass( const std::string& name, const std::string& className )
{
  ecore_wl2_window_title_set( mEcoreWindow, name.c_str() );
  ecore_wl2_window_class_set( mEcoreWindow, className.c_str() );
}

void WindowBaseEcoreWl2::Raise()
{
  // Use ecore_wl2_window_activate to prevent the window shown without rendering
  ecore_wl2_window_activate( mEcoreWindow );
}

void WindowBaseEcoreWl2::Lower()
{
  ecore_wl2_window_lower( mEcoreWindow );
}

void WindowBaseEcoreWl2::Activate()
{
  ecore_wl2_window_activate( mEcoreWindow );
}

void WindowBaseEcoreWl2::SetAvailableAnlges( const std::vector< int >& angles )
{
  int rotations[4] = { 0 };
  DALI_LOG_RELEASE_INFO( "WindowBaseEcoreWl2::SetAvailableAnlges, angle's count: %d, angles\n", angles.size() );
  for( std::size_t i = 0; i < angles.size(); ++i )
  {
    rotations[i] = static_cast< int >( angles[i] );
    DALI_LOG_RELEASE_INFO( "%d ", rotations[i] );
  }
  ecore_wl2_window_available_rotations_set( mEcoreWindow, rotations, angles.size() );
}

void WindowBaseEcoreWl2::SetPreferredAngle( int angle )
{
  ecore_wl2_window_preferred_rotation_set( mEcoreWindow, angle );
}

void WindowBaseEcoreWl2::SetAcceptFocus( bool accept )
{
  ecore_wl2_window_focus_skip_set( mEcoreWindow, !accept );
}

void WindowBaseEcoreWl2::Show()
{
  if( !mVisible )
  {
    ecore_wl2_window_geometry_set( mEcoreWindow, mWindowPositionSize.x, mWindowPositionSize.y, mWindowPositionSize.width, mWindowPositionSize.height );
  }
  mVisible = true;

  ecore_wl2_window_show( mEcoreWindow );
}

void WindowBaseEcoreWl2::Hide()
{
  mVisible = false;
  ecore_wl2_window_hide( mEcoreWindow );
}

unsigned int WindowBaseEcoreWl2::GetSupportedAuxiliaryHintCount() const
{
  return mSupportedAuxiliaryHints.size();
}

std::string WindowBaseEcoreWl2::GetSupportedAuxiliaryHint( unsigned int index ) const
{
  if( index >= GetSupportedAuxiliaryHintCount() )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetSupportedAuxiliaryHint: Invalid index! [%d]\n", index );
  }

  return mSupportedAuxiliaryHints[index];
}

unsigned int WindowBaseEcoreWl2::AddAuxiliaryHint( const std::string& hint, const std::string& value )
{
  bool supported = false;

  // Check if the hint is suppported
  for( std::vector< std::string >::iterator iter = mSupportedAuxiliaryHints.begin(); iter != mSupportedAuxiliaryHints.end(); ++iter )
  {
    if( *iter == hint )
    {
      supported = true;
      break;
    }
  }

  if( !supported )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl2::AddAuxiliaryHint: Not supported auxiliary hint [%s]\n", hint.c_str() );
    return 0;
  }

  // Check if the hint is already added
  for( unsigned int i = 0; i < mAuxiliaryHints.size(); i++ )
  {
    if( mAuxiliaryHints[i].first == hint )
    {
      // Just change the value
      mAuxiliaryHints[i].second = value;

      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::AddAuxiliaryHint: Change! hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), i + 1 );

      return i + 1;   // id is index + 1
    }
  }

  // Add the hint
  mAuxiliaryHints.push_back( std::pair< std::string, std::string >( hint, value ) );

  unsigned int id = mAuxiliaryHints.size();

  ecore_wl2_window_aux_hint_add( mEcoreWindow, static_cast< int >( id ), hint.c_str(), value.c_str() );

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::AddAuxiliaryHint: hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), id );

  return id;
}

bool WindowBaseEcoreWl2::RemoveAuxiliaryHint( unsigned int id )
{
  if( id == 0 || id > mAuxiliaryHints.size() )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl2::RemoveAuxiliaryHint: Invalid id [%d]\n", id );
    return false;
  }

  mAuxiliaryHints[id - 1].second = std::string();

  ecore_wl2_window_aux_hint_del( mEcoreWindow, static_cast< int >( id ) );

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::RemoveAuxiliaryHint: id = %d, hint = %s\n", id, mAuxiliaryHints[id - 1].first.c_str() );

  return true;
}

bool WindowBaseEcoreWl2::SetAuxiliaryHintValue( unsigned int id, const std::string& value )
{
  if( id == 0 || id > mAuxiliaryHints.size() )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl2::SetAuxiliaryHintValue: Invalid id [%d]\n", id );
    return false;
  }

  mAuxiliaryHints[id - 1].second = value;

  ecore_wl2_window_aux_hint_change( mEcoreWindow, static_cast< int >( id ), value.c_str() );

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetAuxiliaryHintValue: id = %d, hint = %s, value = %s\n", id, mAuxiliaryHints[id - 1].first.c_str(), mAuxiliaryHints[id - 1].second.c_str() );

  return true;
}

std::string WindowBaseEcoreWl2::GetAuxiliaryHintValue( unsigned int id ) const
{
  if( id == 0 || id > mAuxiliaryHints.size() )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl2::GetAuxiliaryHintValue: Invalid id [%d]\n", id );
    return std::string();
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetAuxiliaryHintValue: id = %d, hint = %s, value = %s\n", id, mAuxiliaryHints[id - 1].first.c_str(), mAuxiliaryHints[id - 1].second.c_str() );

  return mAuxiliaryHints[id - 1].second;
}

unsigned int WindowBaseEcoreWl2::GetAuxiliaryHintId( const std::string& hint ) const
{
  for( unsigned int i = 0; i < mAuxiliaryHints.size(); i++ )
  {
    if( mAuxiliaryHints[i].first == hint )
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetAuxiliaryHintId: hint = %s, id = %d\n", hint.c_str(), i + 1 );
      return i + 1;
    }
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetAuxiliaryHintId: Invalid hint! [%s]\n", hint.c_str() );

  return 0;
}

void WindowBaseEcoreWl2::SetInputRegion( const Rect< int >& inputRegion )
{
  ecore_wl2_window_input_region_set( mEcoreWindow, inputRegion.x, inputRegion.y, inputRegion.width, inputRegion.height );
}

void WindowBaseEcoreWl2::SetType( Dali::WindowType type )
{
  Ecore_Wl2_Window_Type windowType;

  switch( type )
  {
    case Dali::WindowType::NORMAL:
    {
      windowType = ECORE_WL2_WINDOW_TYPE_TOPLEVEL;
      break;
    }
    case Dali::WindowType::NOTIFICATION:
    {
      windowType = ECORE_WL2_WINDOW_TYPE_NOTIFICATION;
      break;
    }
    case Dali::WindowType::UTILITY:
    {
      windowType = ECORE_WL2_WINDOW_TYPE_UTILITY;
      break;
    }
    case Dali::WindowType::DIALOG:
    {
      windowType = ECORE_WL2_WINDOW_TYPE_DIALOG;
      break;
    }
    default:
    {
      windowType = ECORE_WL2_WINDOW_TYPE_TOPLEVEL;
      break;
    }
  }

  ecore_wl2_window_type_set( mEcoreWindow, windowType );
}

bool WindowBaseEcoreWl2::SetNotificationLevel( Dali::WindowNotificationLevel level )
{
  while( !mTizenPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  int notificationLevel;

  switch( level )
  {
    case Dali::WindowNotificationLevel::NONE:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_NONE;
      break;
    }
    case Dali::WindowNotificationLevel::BASE:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_DEFAULT;
      break;
    }
    case Dali::WindowNotificationLevel::MEDIUM:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_MEDIUM;
      break;
    }
    case Dali::WindowNotificationLevel::HIGH:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_HIGH;
      break;
    }
    case Dali::WindowNotificationLevel::TOP:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_TOP;
      break;
    }
    default:
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetNotificationLevel: invalid level [%d]\n", level );
      notificationLevel = TIZEN_POLICY_LEVEL_DEFAULT;
      break;
    }
  }

  mNotificationLevelChangeDone = false;
  mNotificationChangeState = TIZEN_POLICY_ERROR_STATE_NONE;

  tizen_policy_set_notification_level( mTizenPolicy, ecore_wl2_window_surface_get( mEcoreWindow ), notificationLevel );

  int count = 0;

  while( !mNotificationLevelChangeDone && count < 3 )
  {
    ecore_wl2_display_flush( ecore_wl2_connected_display_get( NULL ) );
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mNotificationLevelChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetNotificationLevel: Level change is failed [%d, %d]\n", level, mNotificationChangeState );
    return false;
  }
  else if( mNotificationChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetNotificationLevel: Permission denied! [%d]\n", level );
    return false;
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetNotificationLevel: Level is changed [%d]\n", mNotificationLevel );

  return true;
}

Dali::WindowNotificationLevel WindowBaseEcoreWl2::GetNotificationLevel() const
{
  while( !mTizenPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  int count = 0;

  while( !mNotificationLevelChangeDone && count < 3 )
  {
    ecore_wl2_display_flush( ecore_wl2_connected_display_get( NULL ) );
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mNotificationLevelChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetNotificationLevel: Error! [%d]\n", mNotificationChangeState );
    return Dali::WindowNotificationLevel::NONE;
  }

  Dali::WindowNotificationLevel level;

  switch( mNotificationLevel )
  {
    case TIZEN_POLICY_LEVEL_NONE:
    {
      level = Dali::WindowNotificationLevel::NONE;
      break;
    }
    case TIZEN_POLICY_LEVEL_DEFAULT:
    {
      level = Dali::WindowNotificationLevel::BASE;
      break;
    }
    case TIZEN_POLICY_LEVEL_MEDIUM:
    {
      level = Dali::WindowNotificationLevel::MEDIUM;
      break;
    }
    case TIZEN_POLICY_LEVEL_HIGH:
    {
      level = Dali::WindowNotificationLevel::HIGH;
      break;
    }
    case TIZEN_POLICY_LEVEL_TOP:
    {
      level = Dali::WindowNotificationLevel::TOP;
      break;
    }
    default:
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetNotificationLevel: invalid level [%d]\n", mNotificationLevel );
      level = Dali::WindowNotificationLevel::NONE;
      break;
    }
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetNotificationLevel: level [%d]\n", mNotificationLevel );

  return level;
}

void WindowBaseEcoreWl2::SetOpaqueState( bool opaque )
{
  while( !mTizenPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  tizen_policy_set_opaque_state( mTizenPolicy, ecore_wl2_window_surface_get( mEcoreWindow ), ( opaque ? 1 : 0 ) );
}

bool WindowBaseEcoreWl2::SetScreenOffMode(WindowScreenOffMode screenOffMode)
{
  while( !mTizenPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  mScreenOffModeChangeDone = false;
  mScreenOffModeChangeState = TIZEN_POLICY_ERROR_STATE_NONE;

  unsigned int mode = 0;

  switch( screenOffMode )
  {
    case WindowScreenOffMode::TIMEOUT:
    {
      mode = 0;
      break;
    }
    case WindowScreenOffMode::NEVER:
    {
      mode = 1;
      break;
    }
  }

  tizen_policy_set_window_screen_mode( mTizenPolicy, ecore_wl2_window_surface_get( mEcoreWindow ), mode );

  int count = 0;

  while( !mScreenOffModeChangeDone && count < 3 )
  {
    ecore_wl2_display_flush( ecore_wl2_connected_display_get( NULL ) );
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mScreenOffModeChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetScreenOffMode: Screen mode change is failed [%d, %d]\n", screenOffMode, mScreenOffModeChangeState );
    return false;
  }
  else if( mScreenOffModeChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetScreenOffMode: Permission denied! [%d]\n", screenOffMode );
    return false;
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetScreenOffMode: Screen mode is changed [%d]\n", mScreenOffMode );

  return true;
}

WindowScreenOffMode WindowBaseEcoreWl2::GetScreenOffMode() const
{
  while( !mTizenPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  int count = 0;

  while( !mScreenOffModeChangeDone && count < 3 )
  {
    ecore_wl2_display_flush( ecore_wl2_connected_display_get( NULL ) );
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mScreenOffModeChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetScreenOffMode: Error! [%d]\n", mScreenOffModeChangeState );
    return WindowScreenOffMode::TIMEOUT;
  }

  WindowScreenOffMode screenMode = WindowScreenOffMode::TIMEOUT;

  switch( mScreenOffMode )
  {
    case 0:
    {
      screenMode = WindowScreenOffMode::TIMEOUT;
      break;
    }
    case 1:
    {
      screenMode = WindowScreenOffMode::NEVER;
      break;
    }
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetScreenOffMode: screen mode [%d]\n", mScreenOffMode );

  return screenMode;
}

bool WindowBaseEcoreWl2::SetBrightness( int brightness )
{
  while( !mTizenDisplayPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  mBrightnessChangeDone = false;
  mBrightnessChangeState = TIZEN_POLICY_ERROR_STATE_NONE;

  tizen_display_policy_set_window_brightness( mTizenDisplayPolicy, ecore_wl2_window_surface_get( mEcoreWindow ), brightness );

  int count = 0;

  while( !mBrightnessChangeDone && count < 3 )
  {
    ecore_wl2_display_flush( ecore_wl2_connected_display_get( NULL ) );
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mBrightnessChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetBrightness: Brightness change is failed [%d, %d]\n", brightness, mBrightnessChangeState );
    return false;
  }
  else if( mBrightnessChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetBrightness: Permission denied! [%d]\n", brightness );
    return false;
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetBrightness: Brightness is changed [%d]\n", mBrightness );

  return true;
}

int WindowBaseEcoreWl2::GetBrightness() const
{
  while( !mTizenDisplayPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  int count = 0;

  while( !mBrightnessChangeDone && count < 3 )
  {
    ecore_wl2_display_flush( ecore_wl2_connected_display_get( NULL ) );
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mBrightnessChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetBrightness: Error! [%d]\n", mBrightnessChangeState );
    return 0;
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetBrightness: Brightness [%d]\n", mBrightness );

  return mBrightness;
}

bool WindowBaseEcoreWl2::GrabKey( Dali::KEY key, KeyGrab::KeyGrabMode grabMode )
{
  Ecore_Wl2_Window_Keygrab_Mode mode;

  switch( grabMode )
  {
    case KeyGrab::TOPMOST:
    {
      mode = ECORE_WL2_WINDOW_KEYGRAB_TOPMOST;
      break;
    }
    case KeyGrab::SHARED:
    {
      mode = ECORE_WL2_WINDOW_KEYGRAB_SHARED;
      break;
    }
    case KeyGrab::OVERRIDE_EXCLUSIVE:
    {
      mode = ECORE_WL2_WINDOW_KEYGRAB_OVERRIDE_EXCLUSIVE;
      break;
    }
    case KeyGrab::EXCLUSIVE:
    {
      mode = ECORE_WL2_WINDOW_KEYGRAB_EXCLUSIVE;
      break;
    }
    default:
    {
      return false;
    }
  }

  return ecore_wl2_window_keygrab_set( mEcoreWindow, KeyLookup::GetKeyName( key ), 0, 0, 0, mode );
}

bool WindowBaseEcoreWl2::UngrabKey( Dali::KEY key )
{
  return ecore_wl2_window_keygrab_unset( mEcoreWindow, KeyLookup::GetKeyName( key ), 0, 0 );
}

bool WindowBaseEcoreWl2::GrabKeyList( const Dali::Vector< Dali::KEY >& key, const Dali::Vector< KeyGrab::KeyGrabMode >& grabMode, Dali::Vector< bool >& result )
{
  int keyCount = key.Count();
  int keyGrabModeCount = grabMode.Count();

  if( keyCount != keyGrabModeCount || keyCount == 0 )
  {
    return false;
  }

  eina_init();

  Eina_List* keyList = NULL;
  Ecore_Wl2_Window_Keygrab_Info* info = new Ecore_Wl2_Window_Keygrab_Info[keyCount];

  for( int index = 0; index < keyCount; ++index )
  {
    info[index].key = const_cast< char* >( KeyLookup::GetKeyName( key[index] ) );

    switch( grabMode[index] )
    {
      case KeyGrab::TOPMOST:
      {
        info[index].mode = ECORE_WL2_WINDOW_KEYGRAB_TOPMOST;
        break;
      }
      case KeyGrab::SHARED:
      {
        info[index].mode = ECORE_WL2_WINDOW_KEYGRAB_SHARED;
        break;
      }
      case KeyGrab::OVERRIDE_EXCLUSIVE:
      {
        info[index].mode = ECORE_WL2_WINDOW_KEYGRAB_OVERRIDE_EXCLUSIVE;
        break;
      }
      case KeyGrab::EXCLUSIVE:
      {
        info[index].mode = ECORE_WL2_WINDOW_KEYGRAB_EXCLUSIVE;
        break;
      }
      default:
      {
        info[index].mode = ECORE_WL2_WINDOW_KEYGRAB_UNKNOWN;
        break;
      }
    }

    keyList = eina_list_append( keyList, &info );
  }

  Eina_List* grabList = ecore_wl2_window_keygrab_list_set( mEcoreWindow, keyList );

  result.Resize( keyCount, true );

  Eina_List* l = NULL;
  Eina_List* m = NULL;
  void* listData = NULL;
  void* data = NULL;
  if( grabList != NULL )
  {
    EINA_LIST_FOREACH( grabList, m, data )
    {
      int index = 0;
      EINA_LIST_FOREACH( keyList, l, listData )
      {
        if( static_cast< Ecore_Wl2_Window_Keygrab_Info* >( listData )->key == NULL )
        {
          DALI_LOG_ERROR( "input key list has null data!" );
          break;
        }

        if( strcmp( static_cast< char* >( data ), static_cast< Ecore_Wl2_Window_Keygrab_Info* >( listData )->key ) == 0 )
        {
          result[index] = false;
        }
        ++index;
      }
    }
  }

  delete [] info;

  eina_list_free( keyList );
  eina_list_free( grabList );
  eina_shutdown();

  return true;
}

bool WindowBaseEcoreWl2::UngrabKeyList( const Dali::Vector< Dali::KEY >& key, Dali::Vector< bool >& result )
{
  int keyCount = key.Count();
  if( keyCount == 0 )
  {
    return false;
  }

  eina_init();

  Eina_List* keyList = NULL;
  Ecore_Wl2_Window_Keygrab_Info* info = new Ecore_Wl2_Window_Keygrab_Info[keyCount];

  for( int index = 0; index < keyCount; ++index )
  {
    info[index].key = const_cast< char* >( KeyLookup::GetKeyName( key[index] ) );
    keyList = eina_list_append( keyList, &info );
  }

  Eina_List* ungrabList = ecore_wl2_window_keygrab_list_unset( mEcoreWindow, keyList );

  result.Resize( keyCount, true );

  Eina_List* l = NULL;
  Eina_List* m = NULL;
  void *listData = NULL;
  void *data = NULL;

  if( ungrabList != NULL )
  {
    EINA_LIST_FOREACH( ungrabList, m, data )
    {
      int index = 0;
      EINA_LIST_FOREACH( keyList, l, listData )
      {
        if( strcmp( static_cast< char* >( data ), static_cast< Ecore_Wl2_Window_Keygrab_Info* >( listData )->key ) == 0 )
        {
          result[index] = false;
        }
        ++index;
      }
    }
  }

  delete [] info;

  eina_list_free( keyList );
  eina_list_free( ungrabList );
  eina_shutdown();

  return true;
}

void WindowBaseEcoreWl2::GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical )
{
  // calculate DPI
  float xres, yres;

  Ecore_Wl2_Output* output = ecore_wl2_window_output_find( mEcoreWindow );

  // 1 inch = 25.4 millimeters
  xres = ecore_wl2_output_dpi_get( output );
  yres = ecore_wl2_output_dpi_get( output );

  dpiHorizontal = int( xres + 0.5f );  // rounding
  dpiVertical   = int( yres + 0.5f );
}

int WindowBaseEcoreWl2::GetScreenRotationAngle()
{
  int transform = 0;

  if( ecore_wl2_window_ignore_output_transform_get( mEcoreWindow ) )
  {
    transform = 0;
  }
  else
  {
    transform = ecore_wl2_output_transform_get( ecore_wl2_window_output_find( mEcoreWindow ) );
  }

  return transform * 90;
}

void WindowBaseEcoreWl2::SetWindowRotationAngle( int degree )
{
  ecore_wl2_window_rotation_set( mEcoreWindow, degree );
}

void WindowBaseEcoreWl2::WindowRotationCompleted( int degree, int width, int height )
{
  ecore_wl2_window_rotation_change_done_send( mEcoreWindow, degree, width, height );
}

void WindowBaseEcoreWl2::SetTransparency( bool transparent )
{
  ecore_wl2_window_alpha_set( mEcoreWindow, transparent );
}

void WindowBaseEcoreWl2::InitializeEcoreElDBus()
{
#ifdef DALI_ELDBUS_AVAILABLE
  Eldbus_Object* object;
  Eldbus_Proxy* manager;

  if( !( mSystemConnection = eldbus_connection_get( ELDBUS_CONNECTION_TYPE_SYSTEM ) ) )
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

  if( !eldbus_proxy_signal_handler_add( manager, "GestureDetected", EcoreElDBusAccessibilityNotification, this ) )
  {
    DALI_LOG_ERROR( "No signal handler returned\n" );
  }
#endif
}

void WindowBaseEcoreWl2::CreateWindow( PositionSize positionSize )
{
  Ecore_Wl2_Display* display = ecore_wl2_display_connect( NULL );
  if( !display )
  {
    DALI_ASSERT_ALWAYS( 0 && "Failed to get display" );
  }

  ecore_wl2_display_sync( display );

  mEcoreWindow = ecore_wl2_window_new( display, NULL, positionSize.x, positionSize.y, positionSize.width, positionSize.height );

  if ( mEcoreWindow == 0 )
  {
    DALI_ASSERT_ALWAYS( 0 && "Failed to create Wayland window" );
  }

  // Set default type
  ecore_wl2_window_type_set( mEcoreWindow, ECORE_WL2_WINDOW_TYPE_TOPLEVEL );
}

void WindowBaseEcoreWl2::SetParent( WindowBase* parentWinBase )
{
  Ecore_Wl2_Window* ecoreParent = NULL;
  if( parentWinBase )
  {
    WindowBaseEcoreWl2* winBaseEcore2 = static_cast<WindowBaseEcoreWl2*>( parentWinBase );
    ecoreParent = winBaseEcore2->mEcoreWindow;
  }
  ecore_wl2_window_parent_set( mEcoreWindow, ecoreParent );
}

int WindowBaseEcoreWl2::CreateFrameRenderedSyncFence()
{
  return wl_egl_window_tizen_create_commit_sync_fd( mEglWindow );
}

int WindowBaseEcoreWl2::CreateFramePresentedSyncFence()
{
  return wl_egl_window_tizen_create_presentation_sync_fd( mEglWindow );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
