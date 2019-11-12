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
#include <dali/internal/window-system/tizen-wayland/ecore-wl/window-base-ecore-wl.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-system.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/input/common/key-impl.h>

// EXTERNAL_HEADERS
#include <dali/public-api/object/any.h>
#include <dali/integration-api/debug.h>
#include <Ecore_Input.h>
#include <vconf.h>
#include <vconf-keys.h>

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

/////////////////////////////////////////////////////////////////////////////////////////////////
// Window Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/// Called when the window iconify state is changed.
static Eina_Bool EcoreEventWindowIconifyStateChanged( void* data, int type, void* event )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    return windowBase->OnIconifyStateChanged( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the window gains focus
static Eina_Bool EcoreEventWindowFocusIn( void* data, int type, void* event )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    return windowBase->OnFocusIn( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the window loses focus
static Eina_Bool EcoreEventWindowFocusOut( void* data, int type, void* event )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    return windowBase->OnFocusOut( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the output is transformed
static Eina_Bool EcoreEventOutputTransform( void* data, int type, void* event )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    return windowBase->OnOutputTransform( data, type, event );
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the output transform should be ignored
static Eina_Bool EcoreEventIgnoreOutputTransform( void* data, int type, void* event )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    windowBase->OnRotation( data, type, event );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    windowBase->OnDataReceive( data, type, event );
  }
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    windowBase->OnFontSizeChanged();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// ElDBus Accessibility Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DALI_ELDBUS_AVAILABLE
// Callback for Ecore ElDBus accessibility events.
static void EcoreElDBusAccessibilityNotification( void* context, const Eldbus_Message* message )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( context );
  if( windowBase )
  {
    windowBase->OnEcoreElDBusAccessibilityNotification( context, message );
  }
}
#endif // DALI_ELDBUS_AVAILABLE

static void RegistryGlobalCallback( void* data, struct wl_registry *registry, uint32_t name, const char* interface, uint32_t version )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
  if( windowBase )
  {
    windowBase->RegistryGlobalCallback( data, registry, name, interface, version );
  }
}

static void RegistryGlobalCallbackRemove( void* data, struct wl_registry* registry, uint32_t id )
{
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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
  WindowBaseEcoreWl* windowBase = static_cast< WindowBaseEcoreWl* >( data );
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

WindowBaseEcoreWl::WindowBaseEcoreWl( Dali::PositionSize positionSize, Any surface, bool isTransparent )
: mEcoreEventHandler(),
  mEcoreWindow( NULL ),
  mWlSurface( NULL ),
  mEglWindow( NULL ),
  mDisplay( NULL ),
  mEventQueue( NULL ),
  mTizenPolicy( NULL ),
  mTizenDisplayPolicy( NULL ),
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
  mOwnSurface( false )
#ifdef DALI_ELDBUS_AVAILABLE
  , mSystemConnection( NULL )
#endif
{
  Initialize( positionSize, surface, isTransparent );
}

WindowBaseEcoreWl::~WindowBaseEcoreWl()
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
    ecore_wl_window_free( mEcoreWindow );

    WindowSystem::Shutdown();
  }
}

void WindowBaseEcoreWl::Initialize( PositionSize positionSize, Any surface, bool isTransparent )
{
  if( surface.Empty() == false )
  {
    // check we have a valid type
    DALI_ASSERT_ALWAYS( ( surface.GetType() == typeid (Ecore_Wl_Window *) ) && "Surface type is invalid" );

    mEcoreWindow = AnyCast< Ecore_Wl_Window* >( surface );
  }
  else
  {
    // we own the surface about to created
    WindowSystem::Initialize();

    mOwnSurface = true;
    CreateWindow( positionSize );
  }

  mWlSurface = ecore_wl_window_surface_create( mEcoreWindow );

  SetTransparency( isTransparent );

  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL_EVENT_WINDOW_ICONIFY_STATE_CHANGE, EcoreEventWindowIconifyStateChanged, this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL_EVENT_FOCUS_IN,                    EcoreEventWindowFocusIn,             this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL_EVENT_FOCUS_OUT,                   EcoreEventWindowFocusOut,            this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL_EVENT_OUTPUT_TRANSFORM,            EcoreEventOutputTransform,           this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL_EVENT_IGNORE_OUTPUT_TRANSFORM,     EcoreEventIgnoreOutputTransform,     this ) );

  // Register Rotate event
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL_EVENT_WINDOW_ROTATE,               EcoreEventRotate,                    this ) );

  // Register Touch events
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_BUTTON_DOWN,              EcoreEventMouseButtonDown,           this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_BUTTON_UP,                EcoreEventMouseButtonUp,             this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_MOVE,                     EcoreEventMouseButtonMove,           this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_BUTTON_CANCEL,            EcoreEventMouseButtonCancel,         this ) );

  // Register Mouse wheel events
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_MOUSE_WHEEL,                    EcoreEventMouseWheel,                this ) );

  // Register Detent event
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_DETENT_ROTATE,                  EcoreEventDetentRotation,            this ) );

  // Register Key events
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_KEY_DOWN,                       EcoreEventKeyDown,                   this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_EVENT_KEY_UP,                         EcoreEventKeyUp,                     this ) );

  // Register Selection event - clipboard selection
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL_EVENT_DATA_SOURCE_SEND,            EcoreEventDataSend,                  this ) );
  mEcoreEventHandler.PushBack( ecore_event_handler_add( ECORE_WL_EVENT_SELECTION_DATA_READY,        EcoreEventDataReceive,               this ) );

  // Register Vconf notify - font name and size
  vconf_notify_key_changed( DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_NAME, VconfNotifyFontNameChanged, this );
  vconf_notify_key_changed( VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, VconfNotifyFontSizeChanged, this );

  InitializeEcoreElDBus();

  mDisplay = ecore_wl_display_get();

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

  // get auxiliary hint
  Eina_List* hints = ecore_wl_window_aux_hints_supported_get( mEcoreWindow );
  if( hints )
  {
    Eina_List* l = NULL;
    char* hint = NULL;

    for( l = hints, ( hint =  static_cast< char* >( eina_list_data_get(l) ) ); l; l = eina_list_next(l), ( hint = static_cast< char* >( eina_list_data_get(l) ) ) )
    {
      mSupportedAuxiliaryHints.push_back( hint );

      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::Initialize: %s\n", hint );
    }
  }
}

Eina_Bool WindowBaseEcoreWl::OnIconifyStateChanged( void* data, int type, void* event )
{
  Ecore_Wl_Event_Window_Iconify_State_Change* iconifyChangedEvent( static_cast< Ecore_Wl_Event_Window_Iconify_State_Change* >( event ) );
  Eina_Bool handled( ECORE_CALLBACK_PASS_ON );

  if( iconifyChangedEvent->win == static_cast< unsigned int>( ecore_wl_window_id_get( mEcoreWindow ) ) )
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

Eina_Bool WindowBaseEcoreWl::OnFocusIn( void* data, int type, void* event )
{
  Ecore_Wl_Event_Focus_In* focusInEvent( static_cast< Ecore_Wl_Event_Focus_In* >( event ) );

  if( focusInEvent->win == static_cast< unsigned int >( ecore_wl_window_id_get( mEcoreWindow ) ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window EcoreEventWindowFocusIn\n" );

    mFocusChangedSignal.Emit( true );
  }

  return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool WindowBaseEcoreWl::OnFocusOut( void* data, int type, void* event )
{
  Ecore_Wl_Event_Focus_Out* focusOutEvent( static_cast< Ecore_Wl_Event_Focus_Out* >( event ) );

  if( focusOutEvent->win == static_cast< unsigned int >( ecore_wl_window_id_get( mEcoreWindow ) ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window EcoreEventWindowFocusOut\n" );

    mFocusChangedSignal.Emit( false );
  }

  return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool WindowBaseEcoreWl::OnOutputTransform( void* data, int type, void* event )
{
  Ecore_Wl_Event_Output_Transform* transformEvent( static_cast< Ecore_Wl_Event_Output_Transform* >( event ) );

  if( transformEvent->output == ecore_wl_window_output_find( mEcoreWindow ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%p) EcoreEventOutputTransform\n", mEcoreWindow );

    mOutputTransformedSignal.Emit();
  }

  return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool WindowBaseEcoreWl::OnIgnoreOutputTransform( void* data, int type, void* event )
{
  Ecore_Wl_Event_Ignore_Output_Transform* ignoreTransformEvent( static_cast< Ecore_Wl_Event_Ignore_Output_Transform* >( event ) );

  if( ignoreTransformEvent->win == mEcoreWindow )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "Window (%p) EcoreEventIgnoreOutputTransform\n", mEcoreWindow );

    mOutputTransformedSignal.Emit();
  }

  return ECORE_CALLBACK_PASS_ON;
}

void WindowBaseEcoreWl::OnRotation( void* data, int type, void* event )
{
  Ecore_Wl_Event_Window_Rotate* ev( static_cast< Ecore_Wl_Event_Window_Rotate* >( event ) );

  if( ev->win == static_cast< unsigned int >( ecore_wl_window_id_get( mEcoreWindow ) ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl::OnRotation\n" );

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

    mRotationSignal.Emit( rotationEvent );
  }
}

void WindowBaseEcoreWl::OnMouseButtonDown( void* data, int type, void* event )
{
  Ecore_Event_Mouse_Button* touchEvent = static_cast< Ecore_Event_Mouse_Button* >( event );

  if( touchEvent->window == static_cast< unsigned int >( ecore_wl_window_id_get( mEcoreWindow ) ) )
  {
    PointState::Type state ( PointState::DOWN );

    // Check if the buttons field is set and ensure it's the primary touch button.
    // If this event was triggered by buttons other than the primary button (used for touch), then
    // just send an interrupted event to Core.
    if( touchEvent->buttons && (touchEvent->buttons != PRIMARY_TOUCH_BUTTON_ID ) )
    {
      state = PointState::INTERRUPTED;
    }

    Device::Class::Type deviceClass;
    Device::Subclass::Type deviceSubclass;

    GetDeviceClass( ecore_device_class_get( touchEvent->dev ), deviceClass );
    GetDeviceSubclass( ecore_device_subclass_get( touchEvent->dev ), deviceSubclass );

    Integration::Point point;
    point.SetDeviceId( touchEvent->multi.device );
    point.SetState( state );
    point.SetScreenPosition( Vector2( touchEvent->x, touchEvent->y ) );
    point.SetRadius( touchEvent->multi.radius, Vector2( touchEvent->multi.radius_x, touchEvent->multi.radius_y ) );
    point.SetPressure( touchEvent->multi.pressure );
    point.SetAngle( Degree( touchEvent->multi.angle ) );
    point.SetDeviceClass( deviceClass );
    point.SetDeviceSubclass( deviceSubclass );

    mTouchEventSignal.Emit( point, touchEvent->timestamp );
  }
}

void WindowBaseEcoreWl::OnMouseButtonUp( void* data, int type, void* event )
{
  Ecore_Event_Mouse_Button* touchEvent = static_cast< Ecore_Event_Mouse_Button* >( event );

  if( touchEvent->window == static_cast< unsigned int >( ecore_wl_window_id_get( mEcoreWindow ) ) )
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

    mTouchEventSignal.Emit( point, touchEvent->timestamp );
  }
}

void WindowBaseEcoreWl::OnMouseButtonMove( void* data, int type, void* event )
{
  Ecore_Event_Mouse_Move* touchEvent = static_cast< Ecore_Event_Mouse_Move* >( event );

  if( touchEvent->window == static_cast< unsigned int >( ecore_wl_window_id_get( mEcoreWindow ) ) )
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

void WindowBaseEcoreWl::OnMouseButtonCancel( void* data, int type, void* event )
{
  Ecore_Event_Mouse_Button* touchEvent = static_cast< Ecore_Event_Mouse_Button* >( event );

  if( touchEvent->window == static_cast< unsigned int >( ecore_wl_window_id_get( mEcoreWindow ) ) )
  {
    Integration::Point point;
    point.SetDeviceId( touchEvent->multi.device );
    point.SetState( PointState::INTERRUPTED );
    point.SetScreenPosition( Vector2( 0.0f, 0.0f ) );

    mTouchEventSignal.Emit( point, touchEvent->timestamp );

    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::OnMouseButtonCancel\n" );
  }
}

void WindowBaseEcoreWl::OnMouseWheel( void* data, int type, void* event )
{
  Ecore_Event_Mouse_Wheel* mouseWheelEvent = static_cast< Ecore_Event_Mouse_Wheel* >( event );

  if( mouseWheelEvent->window == static_cast< unsigned int >( ecore_wl_window_id_get( mEcoreWindow ) ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::OnMouseWheel: direction: %d, modifiers: %d, x: %d, y: %d, z: %d\n", mouseWheelEvent->direction, mouseWheelEvent->modifiers, mouseWheelEvent->x, mouseWheelEvent->y, mouseWheelEvent->z );

    WheelEvent wheelEvent( WheelEvent::MOUSE_WHEEL, mouseWheelEvent->direction, mouseWheelEvent->modifiers, Vector2( mouseWheelEvent->x, mouseWheelEvent->y ), mouseWheelEvent->z, mouseWheelEvent->timestamp );

    mWheelEventSignal.Emit( wheelEvent );
  }
}

void WindowBaseEcoreWl::OnDetentRotation( void* data, int type, void* event )
{
  Ecore_Event_Detent_Rotate* detentEvent = static_cast< Ecore_Event_Detent_Rotate* >( event );

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl::OnDetentRotation\n" );

  int direction = ( detentEvent->direction == ECORE_DETENT_DIRECTION_CLOCKWISE ) ? 1 : -1;
  int timeStamp = detentEvent->timestamp;

  WheelEvent wheelEvent( WheelEvent::CUSTOM_WHEEL, 0, 0, Vector2( 0.0f, 0.0f ), direction, timeStamp );

  mWheelEventSignal.Emit( wheelEvent );
}

void WindowBaseEcoreWl::OnKeyDown( void* data, int type, void* event )
{
  Ecore_Event_Key* keyEvent = static_cast< Ecore_Event_Key* >( event );

  if( keyEvent->window == static_cast< unsigned int >( ecore_wl_window_id_get( mEcoreWindow ) ) )
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

    int keyCode = KeyLookup::GetDaliKeyCode( keyEvent->keyname );
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

    Integration::KeyEvent keyEvent( keyName, logicalKey, keyString, keyCode, modifier, time, Integration::KeyEvent::Down, compose, deviceName, deviceClass, deviceSubclass );

     mKeyEventSignal.Emit( keyEvent );
  }
}

void WindowBaseEcoreWl::OnKeyUp( void* data, int type, void* event )
{
  Ecore_Event_Key* keyEvent = static_cast< Ecore_Event_Key* >( event );

  if( keyEvent->window == static_cast< unsigned int >( ecore_wl_window_id_get( mEcoreWindow ) ) )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::OnKeyUp\n" );

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

    int keyCode = KeyLookup::GetDaliKeyCode( keyEvent->keyname );
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

    Integration::KeyEvent keyEvent( keyName, logicalKey, keyString, keyCode, modifier, time, Integration::KeyEvent::Up, compose, deviceName, deviceClass, deviceSubclass );

     mKeyEventSignal.Emit( keyEvent );
  }
}

void WindowBaseEcoreWl::OnDataSend( void* data, int type, void* event )
{
  mSelectionDataSendSignal.Emit( event );
}

void WindowBaseEcoreWl::OnDataReceive( void* data, int type, void* event )
{
  mSelectionDataReceivedSignal.Emit( event  );
}

void WindowBaseEcoreWl::OnFontNameChanged()
{
  mStyleChangedSignal.Emit( StyleChange::DEFAULT_FONT_CHANGE );
}

void WindowBaseEcoreWl::OnFontSizeChanged()
{
  mStyleChangedSignal.Emit( StyleChange::DEFAULT_FONT_SIZE_CHANGE );
}

void WindowBaseEcoreWl::OnEcoreElDBusAccessibilityNotification( void* context, const Eldbus_Message* message )
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

void WindowBaseEcoreWl::RegistryGlobalCallback( void* data, struct wl_registry *registry, uint32_t name, const char* interface, uint32_t version )
{
  if( strcmp( interface, tizen_policy_interface.name ) == 0 )
  {
    uint32_t clientVersion = std::min( version, MAX_TIZEN_CLIENT_VERSION );

    mTizenPolicy = static_cast< tizen_policy* >( wl_registry_bind( registry, name, &tizen_policy_interface, clientVersion ) );
    if( !mTizenPolicy )
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::RegistryGlobalCallback: wl_registry_bind(tizen_policy_interface) is failed.\n" );
      return;
    }

    tizen_policy_add_listener( mTizenPolicy, &tizenPolicyListener, data );

    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::RegistryGlobalCallback: tizen_policy_add_listener is called.\n" );
  }
  else if( strcmp( interface, tizen_display_policy_interface.name ) == 0 )
  {
    mTizenDisplayPolicy = static_cast< tizen_display_policy* >( wl_registry_bind( registry, name, &tizen_display_policy_interface, version ) );
    if( !mTizenDisplayPolicy )
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::RegistryGlobalCallback: wl_registry_bind(tizen_display_policy_interface) is failed.\n" );
      return;
    }

    tizen_display_policy_add_listener( mTizenDisplayPolicy, &tizenDisplayPolicyListener, data );

    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::RegistryGlobalCallback: tizen_display_policy_add_listener is called.\n" );
  }
}

void WindowBaseEcoreWl::RegistryGlobalCallbackRemove( void* data, struct wl_registry* registry, uint32_t id )
{
  mTizenPolicy = NULL;
  mTizenDisplayPolicy = NULL;
}

void WindowBaseEcoreWl::TizenPolicyNotificationChangeDone(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, int32_t level, uint32_t state )
{
  mNotificationLevel = level;
  mNotificationChangeState = state;
  mNotificationLevelChangeDone = true;

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::TizenPolicyNotificationChangeDone: level = %d, state = %d\n", level, state );
}

void WindowBaseEcoreWl::TizenPolicyScreenModeChangeDone( void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t mode, uint32_t state )
{
  mScreenOffMode = mode;
  mScreenOffModeChangeState = state;
  mScreenOffModeChangeDone = true;

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::TizenPolicyScreenModeChangeDone: mode = %d, state = %d\n", mode, state );
}

void WindowBaseEcoreWl::DisplayPolicyBrightnessChangeDone( void* data, struct tizen_display_policy *displayPolicy, struct wl_surface* surface, int32_t brightness, uint32_t state )
{
  mBrightness = brightness;
  mBrightnessChangeState = state;
  mBrightnessChangeDone = true;

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl::DisplayPolicyBrightnessChangeDone: brightness = %d, state = %d\n", brightness, state );
}

Any WindowBaseEcoreWl::GetNativeWindow()
{
  return mEcoreWindow;
}

int WindowBaseEcoreWl::GetNativeWindowId()
{
  return ecore_wl_window_id_get( mEcoreWindow );
}

EGLNativeWindowType WindowBaseEcoreWl::CreateEglWindow( int width, int height )
{
  mEglWindow = wl_egl_window_create( mWlSurface, width, height );

  return static_cast< EGLNativeWindowType >( mEglWindow );
}

void WindowBaseEcoreWl::DestroyEglWindow()
{
  if( mEglWindow != NULL )
  {
    wl_egl_window_destroy( mEglWindow );
    mEglWindow = NULL;
  }
}

void WindowBaseEcoreWl::SetEglWindowRotation( int angle )
{
  wl_egl_window_rotation rotation;

  switch( angle )
  {
    case 0:
    {
      rotation = ROTATION_0;
      break;
    }
    case 90:
    {
      rotation = ROTATION_270;
      break;
    }
    case 180:
    {
      rotation = ROTATION_180;
      break;
    }
    case 270:
    {
      rotation = ROTATION_90;
      break;
    }
    default:
    {
      rotation = ROTATION_0;
      break;
    }
  }

  wl_egl_window_set_rotation( mEglWindow, rotation );
}

void WindowBaseEcoreWl::SetEglWindowBufferTransform( int angle )
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

  wl_egl_window_set_buffer_transform( mEglWindow, bufferTransform );
}

void WindowBaseEcoreWl::SetEglWindowTransform( int angle )
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

  wl_egl_window_set_window_transform( mEglWindow, windowTransform );
}

void WindowBaseEcoreWl::ResizeEglWindow( PositionSize positionSize )
{
  wl_egl_window_resize( mEglWindow, positionSize.width, positionSize.height, positionSize.x, positionSize.y );
}

bool WindowBaseEcoreWl::IsEglWindowRotationSupported()
{
  // Check capability
  wl_egl_window_capability capability = static_cast< wl_egl_window_capability >( wl_egl_window_get_capabilities( mEglWindow ) );
  if( capability == WL_EGL_WINDOW_CAPABILITY_ROTATION_SUPPORTED )
  {
    return true;
  }

  return false;
}

void WindowBaseEcoreWl::Move( PositionSize positionSize )
{
  ecore_wl_window_position_set( mEcoreWindow, positionSize.x, positionSize.y );
}

void WindowBaseEcoreWl::Resize( PositionSize positionSize )
{
  ecore_wl_window_update_size( mEcoreWindow, positionSize.width, positionSize.height );
}

void WindowBaseEcoreWl::MoveResize( PositionSize positionSize )
{
  ecore_wl_window_position_set( mEcoreWindow, positionSize.x, positionSize.y );
  ecore_wl_window_update_size( mEcoreWindow, positionSize.width, positionSize.height );
}

void WindowBaseEcoreWl::SetClass( const std::string& name, const std::string& className )
{
  ecore_wl_window_title_set( mEcoreWindow, name.c_str() );
  ecore_wl_window_class_name_set( mEcoreWindow, className.c_str() );
}

void WindowBaseEcoreWl::Raise()
{
  // Use ecore_wl_window_activate to prevent the window shown without rendering
  ecore_wl_window_activate( mEcoreWindow );
}

void WindowBaseEcoreWl::Lower()
{
  ecore_wl_window_lower( mEcoreWindow );
}

void WindowBaseEcoreWl::Activate()
{
  ecore_wl_window_activate( mEcoreWindow );
}

void WindowBaseEcoreWl::SetAvailableOrientations( const std::vector< Dali::Window::WindowOrientation >& orientations )
{
  int rotations[4] = { 0 };
  for( std::size_t i = 0; i < orientations.size(); ++i )
  {
    rotations[i] = static_cast< int >( orientations[i] );
  }
  ecore_wl_window_rotation_available_rotations_set( mEcoreWindow, rotations, orientations.size() );
}

void WindowBaseEcoreWl::SetPreferredOrientation( Dali::Window::WindowOrientation orientation )
{
  ecore_wl_window_rotation_preferred_rotation_set( mEcoreWindow, orientation );
}

void WindowBaseEcoreWl::SetAcceptFocus( bool accept )
{
  ecore_wl_window_focus_skip_set( mEcoreWindow, !accept );
}

void WindowBaseEcoreWl::Show()
{
  ecore_wl_window_show( mEcoreWindow );
}

void WindowBaseEcoreWl::Hide()
{
  ecore_wl_window_hide( mEcoreWindow );
}

unsigned int WindowBaseEcoreWl::GetSupportedAuxiliaryHintCount() const
{
  return mSupportedAuxiliaryHints.size();
}

std::string WindowBaseEcoreWl::GetSupportedAuxiliaryHint( unsigned int index ) const
{
  if( index >= GetSupportedAuxiliaryHintCount() )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetSupportedAuxiliaryHint: Invalid index! [%d]\n", index );
  }

  return mSupportedAuxiliaryHints[index];
}

unsigned int WindowBaseEcoreWl::AddAuxiliaryHint( const std::string& hint, const std::string& value )
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
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl::AddAuxiliaryHint: Not supported auxiliary hint [%s]\n", hint.c_str() );
    return 0;
  }

  // Check if the hint is already added
  for( unsigned int i = 0; i < mAuxiliaryHints.size(); i++ )
  {
    if( mAuxiliaryHints[i].first == hint )
    {
      // Just change the value
      mAuxiliaryHints[i].second = value;

      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::AddAuxiliaryHint: Change! hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), i + 1 );

      return i + 1;   // id is index + 1
    }
  }

  // Add the hint
  mAuxiliaryHints.push_back( std::pair< std::string, std::string >( hint, value ) );

  unsigned int id = mAuxiliaryHints.size();

  ecore_wl_window_aux_hint_add( mEcoreWindow, static_cast< int >( id ), hint.c_str(), value.c_str() );

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::AddAuxiliaryHint: hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), id );

  return id;
}

bool WindowBaseEcoreWl::RemoveAuxiliaryHint( unsigned int id )
{
  if( id == 0 || id > mAuxiliaryHints.size() )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl::RemoveAuxiliaryHint: Invalid id [%d]\n", id );
    return false;
  }

  mAuxiliaryHints[id - 1].second = std::string();

  ecore_wl_window_aux_hint_del( mEcoreWindow, static_cast< int >( id ) );

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::RemoveAuxiliaryHint: id = %d, hint = %s\n", id, mAuxiliaryHints[id - 1].first.c_str() );

  return true;
}

bool WindowBaseEcoreWl::SetAuxiliaryHintValue( unsigned int id, const std::string& value )
{
  if( id == 0 || id > mAuxiliaryHints.size() )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl::SetAuxiliaryHintValue: Invalid id [%d]\n", id );
    return false;
  }

  mAuxiliaryHints[id - 1].second = value;

  ecore_wl_window_aux_hint_change( mEcoreWindow, static_cast< int >( id ), value.c_str() );

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetAuxiliaryHintValue: id = %d, hint = %s, value = %s\n", id, mAuxiliaryHints[id - 1].first.c_str(), mAuxiliaryHints[id - 1].second.c_str() );

  return true;
}

std::string WindowBaseEcoreWl::GetAuxiliaryHintValue( unsigned int id ) const
{
  if( id == 0 || id > mAuxiliaryHints.size() )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl::GetAuxiliaryHintValue: Invalid id [%d]\n", id );
    return std::string();
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetAuxiliaryHintValue: id = %d, hint = %s, value = %s\n", id, mAuxiliaryHints[id - 1].first.c_str(), mAuxiliaryHints[id - 1].second.c_str() );

  return mAuxiliaryHints[id - 1].second;
}

unsigned int WindowBaseEcoreWl::GetAuxiliaryHintId( const std::string& hint ) const
{
  for( unsigned int i = 0; i < mAuxiliaryHints.size(); i++ )
  {
    if( mAuxiliaryHints[i].first == hint )
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetAuxiliaryHintId: hint = %s, id = %d\n", hint.c_str(), i + 1 );
      return i + 1;
    }
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetAuxiliaryHintId: Invalid hint! [%s]\n", hint.c_str() );

  return 0;
}

void WindowBaseEcoreWl::SetInputRegion( const Rect< int >& inputRegion )
{
  ecore_wl_window_input_region_set( mEcoreWindow, inputRegion.x, inputRegion.y, inputRegion.width, inputRegion.height );
}

void WindowBaseEcoreWl::SetType( Dali::Window::Type type )
{
  Ecore_Wl_Window_Type windowType;

  switch( type )
  {
    case Dali::Window::NORMAL:
    {
      windowType = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
      break;
    }
    case Dali::Window::NOTIFICATION:
    {
      windowType = ECORE_WL_WINDOW_TYPE_NOTIFICATION;
      break;
    }
    case Dali::Window::UTILITY:
    {
      windowType = ECORE_WL_WINDOW_TYPE_UTILITY;
      break;
    }
    case Dali::Window::DIALOG:
    {
      windowType = ECORE_WL_WINDOW_TYPE_DIALOG;
      break;
    }
    default:
    {
      windowType = ECORE_WL_WINDOW_TYPE_TOPLEVEL;
      break;
    }
  }

  ecore_wl_window_type_set( mEcoreWindow, windowType );
}

bool WindowBaseEcoreWl::SetNotificationLevel( Dali::Window::NotificationLevel::Type level )
{
  while( !mTizenPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  int notificationLevel;

  switch( level )
  {
    case Dali::Window::NotificationLevel::NONE:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_NONE;
      break;
    }
    case Dali::Window::NotificationLevel::BASE:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_DEFAULT;
      break;
    }
    case Dali::Window::NotificationLevel::MEDIUM:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_MEDIUM;
      break;
    }
    case Dali::Window::NotificationLevel::HIGH:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_HIGH;
      break;
    }
    case Dali::Window::NotificationLevel::TOP:
    {
      notificationLevel = TIZEN_POLICY_LEVEL_TOP;
      break;
    }
    default:
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetNotificationLevel: invalid level [%d]\n", level );
      notificationLevel = TIZEN_POLICY_LEVEL_DEFAULT;
      break;
    }
  }

  mNotificationLevelChangeDone = false;
  mNotificationChangeState = TIZEN_POLICY_ERROR_STATE_NONE;

  tizen_policy_set_notification_level( mTizenPolicy, ecore_wl_window_surface_get( mEcoreWindow ), notificationLevel );

  int count = 0;

  while( !mNotificationLevelChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mNotificationLevelChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetNotificationLevel: Level change is failed [%d, %d]\n", level, mNotificationChangeState );
    return false;
  }
  else if( mNotificationChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetNotificationLevel: Permission denied! [%d]\n", level );
    return false;
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetNotificationLevel: Level is changed [%d]\n", mNotificationLevel );

  return true;
}

Dali::Window::NotificationLevel::Type WindowBaseEcoreWl::GetNotificationLevel() const
{
  while( !mTizenPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  int count = 0;

  while( !mNotificationLevelChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mNotificationLevelChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetNotificationLevel: Error! [%d]\n", mNotificationChangeState );
    return Dali::Window::NotificationLevel::NONE;
  }

  Dali::Window::NotificationLevel::Type level;

  switch( mNotificationLevel )
  {
    case TIZEN_POLICY_LEVEL_NONE:
    {
      level = Dali::Window::NotificationLevel::NONE;
      break;
    }
    case TIZEN_POLICY_LEVEL_DEFAULT:
    {
      level = Dali::Window::NotificationLevel::BASE;
      break;
    }
    case TIZEN_POLICY_LEVEL_MEDIUM:
    {
      level = Dali::Window::NotificationLevel::MEDIUM;
      break;
    }
    case TIZEN_POLICY_LEVEL_HIGH:
    {
      level = Dali::Window::NotificationLevel::HIGH;
      break;
    }
    case TIZEN_POLICY_LEVEL_TOP:
    {
      level = Dali::Window::NotificationLevel::TOP;
      break;
    }
    default:
    {
      DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetNotificationLevel: invalid level [%d]\n", mNotificationLevel );
      level = Dali::Window::NotificationLevel::NONE;
      break;
    }
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetNotificationLevel: level [%d]\n", mNotificationLevel );

  return level;
}

void WindowBaseEcoreWl::SetOpaqueState( bool opaque )
{
  while( !mTizenPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  tizen_policy_set_opaque_state( mTizenPolicy, ecore_wl_window_surface_get( mEcoreWindow ), ( opaque ? 1 : 0 ) );
}

bool WindowBaseEcoreWl::SetScreenOffMode(Dali::Window::ScreenOffMode::Type screenOffMode)
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
    case Dali::Window::ScreenOffMode::TIMEOUT:
    {
      mode = 0;
      break;
    }
    case Dali::Window::ScreenOffMode::NEVER:
    {
      mode = 1;
      break;
    }
  }

  tizen_policy_set_window_screen_mode( mTizenPolicy, ecore_wl_window_surface_get( mEcoreWindow ), mode );

  int count = 0;

  while( !mScreenOffModeChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mScreenOffModeChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetScreenOffMode: Screen mode change is failed [%d, %d]\n", screenOffMode, mScreenOffModeChangeState );
    return false;
  }
  else if( mScreenOffModeChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetScreenOffMode: Permission denied! [%d]\n", screenOffMode );
    return false;
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetScreenOffMode: Screen mode is changed [%d]\n", mScreenOffMode );

  return true;
}

Dali::Window::ScreenOffMode::Type WindowBaseEcoreWl::GetScreenOffMode() const
{
  while( !mTizenPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  int count = 0;

  while( !mScreenOffModeChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mScreenOffModeChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetScreenOffMode: Error! [%d]\n", mScreenOffModeChangeState );
    return Dali::Window::ScreenOffMode::TIMEOUT;
  }

  Dali::Window::ScreenOffMode::Type screenMode = Dali::Window::ScreenOffMode::TIMEOUT;

  switch( mScreenOffMode )
  {
    case 0:
    {
      screenMode = Dali::Window::ScreenOffMode::TIMEOUT;
      break;
    }
    case 1:
    {
      screenMode = Dali::Window::ScreenOffMode::NEVER;
      break;
    }
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetScreenOffMode: screen mode [%d]\n", mScreenOffMode );

  return screenMode;
}

bool WindowBaseEcoreWl::SetBrightness( int brightness )
{
  while( !mTizenDisplayPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  mBrightnessChangeDone = false;
  mBrightnessChangeState = TIZEN_POLICY_ERROR_STATE_NONE;

  tizen_display_policy_set_window_brightness( mTizenDisplayPolicy, ecore_wl_window_surface_get( mEcoreWindow ), brightness );

  int count = 0;

  while( !mBrightnessChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mBrightnessChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetBrightness: Brightness change is failed [%d, %d]\n", brightness, mBrightnessChangeState );
    return false;
  }
  else if( mBrightnessChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetBrightness: Permission denied! [%d]\n", brightness );
    return false;
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::SetBrightness: Brightness is changed [%d]\n", mBrightness );

  return true;
}

int WindowBaseEcoreWl::GetBrightness() const
{
  while( !mTizenDisplayPolicy )
  {
    wl_display_dispatch_queue( mDisplay, mEventQueue );
  }

  int count = 0;

  while( !mBrightnessChangeDone && count < 3 )
  {
    ecore_wl_flush();
    wl_display_dispatch_queue( mDisplay, mEventQueue );
    count++;
  }

  if( !mBrightnessChangeDone )
  {
    DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetBrightness: Error! [%d]\n", mBrightnessChangeState );
    return 0;
  }

  DALI_LOG_INFO( gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl::GetBrightness: Brightness [%d]\n", mBrightness );

  return mBrightness;
}

bool WindowBaseEcoreWl::GrabKey( Dali::KEY key, KeyGrab::KeyGrabMode grabMode )
{
  Ecore_Wl_Window_Keygrab_Mode mode;

  switch( grabMode )
  {
    case KeyGrab::TOPMOST:
    {
      mode = ECORE_WL_WINDOW_KEYGRAB_TOPMOST;
      break;
    }
    case KeyGrab::SHARED:
    {
      mode = ECORE_WL_WINDOW_KEYGRAB_SHARED;
      break;
    }
    case KeyGrab::OVERRIDE_EXCLUSIVE:
    {
      mode = ECORE_WL_WINDOW_KEYGRAB_OVERRIDE_EXCLUSIVE;
      break;
    }
    case KeyGrab::EXCLUSIVE:
    {
      mode = ECORE_WL_WINDOW_KEYGRAB_EXCLUSIVE;
      break;
    }
    default:
    {
      return false;
    }
  }

  return ecore_wl_window_keygrab_set( mEcoreWindow, KeyLookup::GetKeyName( key ), 0, 0, 0, mode );
}

bool WindowBaseEcoreWl::UngrabKey( Dali::KEY key )
{
  return ecore_wl_window_keygrab_unset( mEcoreWindow, KeyLookup::GetKeyName( key ), 0, 0 );
}

bool WindowBaseEcoreWl::GrabKeyList( const Dali::Vector< Dali::KEY >& key, const Dali::Vector< KeyGrab::KeyGrabMode >& grabMode, Dali::Vector< bool >& result )
{
  int keyCount = key.Count();
  int keyGrabModeCount = grabMode.Count();

  if( keyCount != keyGrabModeCount || keyCount == 0 )
  {
    return false;
  }

  eina_init();

  Eina_List* keyList = NULL;
  Ecore_Wl_Window_Keygrab_Info* info = new Ecore_Wl_Window_Keygrab_Info[keyCount];

  for( int index = 0; index < keyCount; ++index )
  {
    info[index].key = const_cast< char* >( KeyLookup::GetKeyName( key[index] ) );

    switch( grabMode[index] )
    {
      case KeyGrab::TOPMOST:
      {
        info[index].mode = ECORE_WL_WINDOW_KEYGRAB_TOPMOST;
        break;
      }
      case KeyGrab::SHARED:
      {
        info[index].mode = ECORE_WL_WINDOW_KEYGRAB_SHARED;
        break;
      }
      case KeyGrab::OVERRIDE_EXCLUSIVE:
      {
        info[index].mode = ECORE_WL_WINDOW_KEYGRAB_OVERRIDE_EXCLUSIVE;
        break;
      }
      case KeyGrab::EXCLUSIVE:
      {
        info[index].mode = ECORE_WL_WINDOW_KEYGRAB_EXCLUSIVE;
        break;
      }
      default:
      {
        info[index].mode = ECORE_WL_WINDOW_KEYGRAB_UNKNOWN;
        break;
      }
    }

    keyList = eina_list_append( keyList, &info );
  }

  Eina_List* grabList = ecore_wl_window_keygrab_list_set( mEcoreWindow, keyList );

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
        if( static_cast< Ecore_Wl_Window_Keygrab_Info* >( listData )->key == NULL )
        {
          DALI_LOG_ERROR( "input key list has null data!" );
          break;
        }

        if( strcmp( static_cast< char* >( data ), static_cast< Ecore_Wl_Window_Keygrab_Info* >( listData )->key ) == 0 )
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

bool WindowBaseEcoreWl::UngrabKeyList( const Dali::Vector< Dali::KEY >& key, Dali::Vector< bool >& result )
{
  int keyCount = key.Count();
  if( keyCount == 0 )
  {
    return false;
  }

  eina_init();

  Eina_List* keyList = NULL;
  Ecore_Wl_Window_Keygrab_Info* info = new Ecore_Wl_Window_Keygrab_Info[keyCount];

  for( int index = 0; index < keyCount; ++index )
  {
    info[index].key = const_cast< char* >( KeyLookup::GetKeyName( key[index] ) );
    keyList = eina_list_append( keyList, &info );
  }

  Eina_List* ungrabList = ecore_wl_window_keygrab_list_unset( mEcoreWindow, keyList );

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
        if( strcmp( static_cast< char* >( data ), static_cast< Ecore_Wl_Window_Keygrab_Info* >( listData )->key ) == 0 )
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

void WindowBaseEcoreWl::GetDpi( unsigned int& dpiHorizontal, unsigned int& dpiVertical )
{
  // calculate DPI
  float xres, yres;

  // 1 inch = 25.4 millimeters
  xres = ecore_wl_dpi_get();
  yres = ecore_wl_dpi_get();

  dpiHorizontal = int( xres + 0.5f );  // rounding
  dpiVertical   = int( yres + 0.5f );
}

int WindowBaseEcoreWl::GetScreenRotationAngle()
{
  int transform = 0;

  if( ecore_wl_window_ignore_output_transform_get( mEcoreWindow ) )
  {
    transform = 0;
  }
  else
  {
    transform = ecore_wl_output_transform_get( ecore_wl_window_output_find( mEcoreWindow ) );
  }

  return transform * 90;
}

void WindowBaseEcoreWl::SetWindowRotationAngle( int degree )
{
  ecore_wl_window_rotation_set( mEcoreWindow, degree );
}

void WindowBaseEcoreWl::WindowRotationCompleted( int degree, int width, int height )
{
  ecore_wl_window_rotation_change_done_send( mEcoreWindow );
}

void WindowBaseEcoreWl::SetTransparency( bool transparent )
{
  ecore_wl_window_alpha_set( mEcoreWindow, transparent );
}

void WindowBaseEcoreWl::InitializeEcoreElDBus()
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

void WindowBaseEcoreWl::CreateWindow( PositionSize positionSize )
{
  mEcoreWindow = ecore_wl_window_new( 0, positionSize.x, positionSize.y, positionSize.width, positionSize.height, ECORE_WL_WINDOW_BUFFER_TYPE_EGL_WINDOW );

  if ( mEcoreWindow == 0 )
  {
    DALI_ASSERT_ALWAYS( 0 && "Failed to create Wayland window" );
  }
}

void WindowBaseEcoreWl::SetParent( Any parent )
{
  Ecore_Wl_Window* mEcoreParent;
  if( parent.Empty() == false )
  {
    // check we have a valid type
    DALI_ASSERT_ALWAYS( ( parent.GetType() == typeid (Ecore_Wl_Window *) ) && "Parent's surface type is invalid" );
    mEcoreParent = AnyCast< Ecore_Wl_Window* >( parent );
  }
  else
  {
    mEcoreParent = NULL;
  }
  ecore_wl_window_parent_set( mEcoreWindow, mEcoreParent );
}

bool WindowBaseEcoreWl::IsMatchedWindow( Any window )
{
  bool ret = false;
  if ( window.Empty() == false )
  {
    // check we have a valid type
    DALI_ASSERT_ALWAYS( ( window.GetType() == typeid (Ecore_Wl_Window *) ) && "Window's surface type is invalid" );
    if ( AnyCast< Ecore_Wl_Window* >( window ) == mEcoreWindow )
    {
      ret = true;
    }
  }
  return ret;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
