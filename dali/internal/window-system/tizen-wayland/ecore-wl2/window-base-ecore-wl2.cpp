/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/internal/input/common/key-impl.h>
#include <dali/internal/system/common/time-service.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-system.h>

// EXTERNAL_HEADERS
#include <Ecore_Input.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/trace.h>
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/public-api/events/mouse-button.h>
#include <dali/public-api/object/any.h>

#if defined(VCONF_ENABLED)
#include <vconf-keys.h>
#include <vconf.h>
#endif

#define START_DURATION_CHECK()                               \
  uint32_t durationMilliSeconds = static_cast<uint32_t>(-1); \
  uint32_t startTime, endTime;                               \
  startTime = TimeService::GetMilliSeconds();

#define FINISH_DURATION_CHECK(functionName)                                             \
  endTime              = TimeService::GetMilliSeconds();                                \
  durationMilliSeconds = endTime - startTime;                                           \
  if(durationMilliSeconds > 0)                                                          \
  {                                                                                     \
    DALI_LOG_DEBUG_INFO("%s : duration [%u ms]\n", functionName, durationMilliSeconds); \
  }

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
Debug::Filter* gWindowBaseLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_WINDOW_BASE");
#endif

DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_PERFORMANCE_MARKER, false);

/**
 * @brief Enumeration of location for window resized by display server.
 */
enum class ResizeLocation
{
  INVALID      = 0,  ///< Invalid value
  TOP_LEFT     = 5,  ///< Start resizing window to the top-left edge.
  LEFT         = 4,  ///< Start resizing window to the left side.
  BOTTOM_LEFT  = 6,  ///< Start resizing window to the bottom-left edge.
  BOTTOM       = 2,  ///< Start resizing window to the bottom side.
  BOTTOM_RIGHT = 10, ///< Start resizing window to the bottom-right edge.
  RIGHT        = 8,  ///< Start resizing window to the right side.
  TOP_RIGHT    = 9,  ///< Start resizing window to the top-right edge.
  TOP          = 1   ///< Start resizing window to the top side.
};

const uint32_t       MAX_TIZEN_CLIENT_VERSION = 7;
const unsigned int   PRIMARY_TOUCH_BUTTON_ID  = 1;
const ResizeLocation RESIZE_LOCATIONS[]       = {ResizeLocation::TOP_LEFT, ResizeLocation::LEFT, ResizeLocation::BOTTOM_LEFT, ResizeLocation::BOTTOM, ResizeLocation::BOTTOM_RIGHT, ResizeLocation::RIGHT, ResizeLocation::TOP_RIGHT, ResizeLocation::TOP, ResizeLocation::INVALID};

#if defined(VCONF_ENABLED)
const char* DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_NAME = "db/setting/accessibility/font_name"; // It will be update at vconf-key.h and replaced.
#endif

struct KeyCodeMap
{
  xkb_keysym_t  keySym;
  xkb_keycode_t keyCode;
  bool          isKeyCode;
};

/**
 * Get the device name from the provided ecore key event
 */
void GetDeviceName(Ecore_Event_Key* keyEvent, std::string& result)
{
  const char* ecoreDeviceName = ecore_device_name_get(keyEvent->dev);

  if(ecoreDeviceName)
  {
    result = ecoreDeviceName;
  }
}

/**
 * Get the device class from the provided ecore event
 */
void GetDeviceClass(Ecore_Device_Class ecoreDeviceClass, Device::Class::Type& deviceClass)
{
  switch(ecoreDeviceClass)
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

void GetDeviceSubclass(Ecore_Device_Subclass ecoreDeviceSubclass, Device::Subclass::Type& deviceSubclass)
{
  switch(ecoreDeviceSubclass)
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

void FindKeyCode(struct xkb_keymap* keyMap, xkb_keycode_t key, void* data)
{
  KeyCodeMap* foundKeyCode = static_cast<KeyCodeMap*>(data);
  if(foundKeyCode->isKeyCode)
  {
    return;
  }

  xkb_keysym_t        keySym  = foundKeyCode->keySym;
  int                 nsyms   = 0;
  const xkb_keysym_t* symsOut = NULL;

  nsyms = xkb_keymap_key_get_syms_by_level(keyMap, key, 0, 0, &symsOut);

  if(nsyms && symsOut)
  {
    if(*symsOut == keySym)
    {
      foundKeyCode->keyCode   = key;
      foundKeyCode->isKeyCode = true;
    }
  }
}

/**
 * Return the recalculated window resizing location according to the current orientation.
 */
ResizeLocation RecalculateLocationToCurrentOrientation(WindowResizeDirection direction, int windowRotationAngle)
{
  int index = 8;
  switch(direction)
  {
    case WindowResizeDirection::TOP_LEFT:
    {
      index = 0;
      break;
    }
    case WindowResizeDirection::LEFT:
    {
      index = 1;
      break;
    }
    case WindowResizeDirection::BOTTOM_LEFT:
    {
      index = 2;
      break;
    }
    case WindowResizeDirection::BOTTOM:
    {
      index = 3;
      break;
    }
    case WindowResizeDirection::BOTTOM_RIGHT:
    {
      index = 4;
      break;
    }
    case WindowResizeDirection::RIGHT:
    {
      index = 5;
      break;
    }
    case WindowResizeDirection::TOP_RIGHT:
    {
      index = 6;
      break;
    }
    case WindowResizeDirection::TOP:
    {
      index = 7;
      break;
    }
    default:
    {
      index = 8;
      break;
    }
  }

  if(index != 8 && windowRotationAngle != 0)
  {
    index = (index + (windowRotationAngle / 90) * 2) % 8;
  }

  return RESIZE_LOCATIONS[index];
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Window Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/// Called when the window iconify state is changed.
static Eina_Bool EcoreEventWindowIconifyStateChanged(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    return windowBase->OnIconifyStateChanged(data, type, event);
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the window gains focus
static Eina_Bool EcoreEventWindowFocusIn(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    return windowBase->OnFocusIn(data, type, event);
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the window loses focus
static Eina_Bool EcoreEventWindowFocusOut(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    return windowBase->OnFocusOut(data, type, event);
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the output is transformed
static Eina_Bool EcoreEventOutputTransform(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    return windowBase->OnOutputTransform(data, type, event);
  }

  return ECORE_CALLBACK_PASS_ON;
}

/// Called when the output transform should be ignored
static Eina_Bool EcoreEventIgnoreOutputTransform(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    return windowBase->OnIgnoreOutputTransform(data, type, event);
  }

  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when rotate event is recevied.
 */
static Eina_Bool EcoreEventRotate(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    DALI_LOG_RELEASE_INFO("WindowBaseEcoreWl2::EcoreEventRotate\n");
    windowBase->OnRotation(data, type, event);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when configure event is recevied.
 */
static Eina_Bool EcoreEventConfigure(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnConfiguration(data, type, event);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Touch Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a touch down is received.
 */
static Eina_Bool EcoreEventMouseButtonDown(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnMouseButtonDown(data, type, event);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a touch up is received.
 */
static Eina_Bool EcoreEventMouseButtonUp(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnMouseButtonUp(data, type, event);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a touch motion is received.
 */
static Eina_Bool EcoreEventMouseButtonMove(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnMouseButtonMove(data, type, event);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a touch motion is received.
 */
static Eina_Bool EcoreEventMouseButtonRelativeMove(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnMouseButtonRelativeMove(data, type, event);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a touch is canceled.
 */
static Eina_Bool EcoreEventMouseButtonCancel(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnMouseButtonCancel(data, type, event);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a mouse wheel is received.
 */
static Eina_Bool EcoreEventMouseWheel(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnMouseWheel(data, type, event);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a mouse in is received.
 */
static Eina_Bool EcoreEventMouseIn(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnMouseInOut(data, type, event, Dali::DevelWindow::MouseInOutEvent::Type::IN);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a mouse out is received.
 */
static Eina_Bool EcoreEventMouseOut(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    // When the mouse is out, the previous mouse must be canceled.
    windowBase->OnMouseButtonCancel(data, type, event);
    windowBase->OnMouseInOut(data, type, event, Dali::DevelWindow::MouseInOutEvent::Type::OUT);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a detent rotation event is recevied.
 */
static Eina_Bool EcoreEventDetentRotation(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnDetentRotation(data, type, event);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Key Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a key down is received.
 */
static Eina_Bool EcoreEventKeyDown(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnKeyDown(data, type, event);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when a key up is received.
 */
static Eina_Bool EcoreEventKeyUp(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnKeyUp(data, type, event);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Selection Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when the source window notifies us the content in clipboard is selected.
 */
static Eina_Bool EcoreEventDataSend(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnDataSend(data, type, event);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when the source window sends us about the selected content.
 * For example, when item is selected in the clipboard.
 */
static Eina_Bool EcoreEventDataReceive(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnDataReceive(data, type, event);
  }
  return ECORE_CALLBACK_PASS_ON;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Effect Start/End Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when transition animation of the window's shown/hidden is started by window manager.
 */
static Eina_Bool EcoreEventEffectStart(void* data, int type, void* event)
{
  WindowBaseEcoreWl2*           windowBase  = static_cast<WindowBaseEcoreWl2*>(data);
  Ecore_Wl2_Event_Effect_Start* effectStart = static_cast<Ecore_Wl2_Event_Effect_Start*>(event);
  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::EcoreEventEffectStart, effect type[ %d ]\n", effectStart->type);
  if(windowBase)
  {
    if(effectStart->type < 3) // only under restack
    {
      windowBase->OnTransitionEffectEvent(WindowEffectState::START, static_cast<WindowEffectType>(effectStart->type));
    }
  }
  return ECORE_CALLBACK_PASS_ON;
}

/**
 * Called when transition animation of the window's shown/hidden is ended by window manager.
 */
static Eina_Bool EcoreEventEffectEnd(void* data, int type, void* event)
{
  Ecore_Wl2_Event_Effect_Start* effectEnd  = static_cast<Ecore_Wl2_Event_Effect_Start*>(event);
  WindowBaseEcoreWl2*           windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::EcoreEventEffectEnd, effect type[ %d ]\n", effectEnd->type);
  if(windowBase)
  {
    if(effectEnd->type < 3) // only under restack
    {
      windowBase->OnTransitionEffectEvent(WindowEffectState::END, static_cast<WindowEffectType>(effectEnd->type));
    }
  }
  return ECORE_CALLBACK_PASS_ON;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Keyboard Repeat Settings Changed Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

static Eina_Bool EcoreEventSeatKeyboardRepeatChanged(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::EcoreEventSeatKeyboardRepeatChanged, id[ %d ]\n", static_cast<Ecore_Wl2_Event_Seat_Keyboard_Repeat_Changed*>(event)->id);
  if(windowBase)
  {
    windowBase->OnKeyboardRepeatSettingsChanged();
  }

  return ECORE_CALLBACK_RENEW;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Keymap Changed Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

static Eina_Bool EcoreEventSeatKeymapChanged(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->KeymapChanged(data, type, event);
  }

  return ECORE_CALLBACK_RENEW;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Font Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(VCONF_ENABLED)
/**
 * Called when a font name is changed.
 */
static void VconfNotifyFontNameChanged(keynode_t* node, void* data)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnFontNameChanged();
  }
}

/**
 * Called when a font size is changed.
 */
static void VconfNotifyFontSizeChanged(keynode_t* node, void* data)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnFontSizeChanged();
  }
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
// Window Redraw Request Event Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

static Eina_Bool EcoreEventWindowRedrawRequest(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::EcoreEventWindowRedrawRequest, window[ %d ]\n", static_cast<Ecore_Wl2_Event_Window_Redraw_Request*>(event)->win);
  if(windowBase)
  {
    windowBase->OnEcoreEventWindowRedrawRequest();
  }

  return ECORE_CALLBACK_RENEW;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Window Auxiliary Message Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////
static Eina_Bool EcoreEventWindowAuxiliaryMessage(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnEcoreEventWindowAuxiliaryMessage(event);
  }
  return ECORE_CALLBACK_RENEW;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Window is Moved/Resized By Server Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////
static Eina_Bool EcoreEventWindowMoveCompleted(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnMoveCompleted(event);
  }
  return ECORE_CALLBACK_RENEW;
}

static Eina_Bool EcoreEventWindowResizeCompleted(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnResizeCompleted(event);
  }
  return ECORE_CALLBACK_RENEW;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Conformant Change Callback
/////////////////////////////////////////////////////////////////////////////////////////////////
static Eina_Bool EcoreEventConformantChange(void* data, int type, void* event)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->OnEcoreEventConformantChange(event);
  }
  return ECORE_CALLBACK_RENEW;
}

static void RegistryGlobalCallback(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->RegistryGlobalCallback(data, registry, name, interface, version);
  }
}

static void RegistryGlobalCallbackRemove(void* data, struct wl_registry* registry, uint32_t id)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->RegistryGlobalCallbackRemove(data, registry, id);
  }
}

static void TizenPolicyConformant(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t isConformant)
{
}

static void TizenPolicyConformantArea(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t conformantPart, uint32_t state, int32_t x, int32_t y, int32_t w, int32_t h)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->TizenPolicyConformantArea(data, tizenPolicy, surface, conformantPart, state, x, y, w, h);
  }
}

static void TizenPolicyNotificationChangeDone(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, int32_t level, uint32_t state)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->TizenPolicyNotificationChangeDone(data, tizenPolicy, surface, level, state);
  }
}

static void TizenPolicyTransientForDone(void* data, struct tizen_policy* tizenPolicy, uint32_t childId)
{
}

static void TizenPolicyScreenModeChangeDone(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t mode, uint32_t state)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->TizenPolicyScreenModeChangeDone(data, tizenPolicy, surface, mode, state);
  }
}

static void TizenPolicyIconifyStateChanged(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t iconified, uint32_t force)
{
}

static void TizenPolicySupportedAuxiliaryHints(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, struct wl_array* hints, uint32_t numNints)
{
}

static void TizenPolicyAllowedAuxiliaryHint(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, int id)
{
}

static void TizenPolicyAuxiliaryMessage(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, const char* key, const char* val, struct wl_array* options)
{
}

static void TizenPolicyConformantRegion(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t conformantPart, uint32_t state, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t serial)
{
}

static void DisplayPolicyBrightnessChangeDone(void* data, struct tizen_display_policy* displayPolicy, struct wl_surface* surface, int32_t brightness, uint32_t state)
{
  WindowBaseEcoreWl2* windowBase = static_cast<WindowBaseEcoreWl2*>(data);
  if(windowBase)
  {
    windowBase->DisplayPolicyBrightnessChangeDone(data, displayPolicy, surface, brightness, state);
  }
}

const struct wl_registry_listener registryListener =
  {
    RegistryGlobalCallback,
    RegistryGlobalCallbackRemove};

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
    TizenPolicyConformantRegion};

const struct tizen_display_policy_listener tizenDisplayPolicyListener =
  {
    DisplayPolicyBrightnessChangeDone};

} // unnamed namespace

WindowBaseEcoreWl2::WindowBaseEcoreWl2(Dali::PositionSize positionSize, Any surface, bool isTransparent)
: mEcoreEventHandler(),
  mEcoreWindow(nullptr),
  mWlSurface(nullptr),
  mWlInputPanel(nullptr),
  mWlOutput(nullptr),
  mWlInputPanelSurface(nullptr),
  mEglWindow(nullptr),
  mDisplay(nullptr),
  mEventQueue(nullptr),
  mTizenPolicy(nullptr),
  mTizenDisplayPolicy(nullptr),
  mKeyMap(nullptr),
  mSupportedAuxiliaryHints(),
  mWindowPositionSize(positionSize),
  mAuxiliaryHints(),
  mType(WindowType::NORMAL),
  mNotificationLevel(-1),
  mScreenOffMode(0),
  mBrightness(0),
  mWindowRotationAngle(0),
  mScreenRotationAngle(0),
  mSupportedPreProtation(0),
  mNotificationChangeState(0),
  mScreenOffModeChangeState(0),
  mBrightnessChangeState(0),
  mLastSubmittedMoveResizeSerial(0),
  mMoveResizeSerial(0),
  mNotificationLevelChangeDone(true),
  mScreenOffModeChangeDone(true),
  mVisible(true),
  mOwnSurface(false),
  mBrightnessChangeDone(true)
{
  Initialize(positionSize, surface, isTransparent);
}

WindowBaseEcoreWl2::~WindowBaseEcoreWl2()
{
#if defined(VCONF_ENABLED)
  vconf_ignore_key_changed(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, VconfNotifyFontSizeChanged);
  vconf_ignore_key_changed(DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_NAME, VconfNotifyFontNameChanged);
#endif

  for(Dali::Vector<Ecore_Event_Handler*>::Iterator iter = mEcoreEventHandler.Begin(), endIter = mEcoreEventHandler.End(); iter != endIter; ++iter)
  {
    ecore_event_handler_del(*iter);
  }
  mEcoreEventHandler.Clear();

  if(mEventQueue)
  {
    wl_event_queue_destroy(mEventQueue);
  }

  mSupportedAuxiliaryHints.clear();
  mAuxiliaryHints.clear();

  if(mEglWindow != NULL)
  {
    wl_egl_window_destroy(mEglWindow);
    mEglWindow = NULL;
  }

  if(mOwnSurface)
  {
    ecore_wl2_window_free(mEcoreWindow);
  }
}

void WindowBaseEcoreWl2::Initialize(PositionSize positionSize, Any surface, bool isTransparent)
{
  if(surface.Empty() == false)
  {
    // check we have a valid type
    DALI_ASSERT_ALWAYS((surface.GetType() == typeid(Ecore_Wl2_Window*)) && "Surface type is invalid");

    mEcoreWindow = AnyCast<Ecore_Wl2_Window*>(surface);
  }
  else
  {
    // we own the surface about to created
    mOwnSurface = true;
    CreateWindow(positionSize);
  }

  mWlSurface = ecore_wl2_window_surface_get(mEcoreWindow);

  SetTransparency(isTransparent);

  Ecore_Wl2_Display* display      = ecore_wl2_connected_display_get(NULL);
  Ecore_Wl2_Input*   ecoreWlInput = ecore_wl2_input_default_input_get(display);

  if(ecoreWlInput)
  {
    mKeyMap = ecore_wl2_input_keymap_get(ecoreWlInput);

    mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_SEAT_KEYMAP_CHANGED, EcoreEventSeatKeymapChanged, this));
  }

  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_WINDOW_ICONIFY_STATE_CHANGE, EcoreEventWindowIconifyStateChanged, this));
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_FOCUS_IN, EcoreEventWindowFocusIn, this));
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_FOCUS_OUT, EcoreEventWindowFocusOut, this));
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_OUTPUT_TRANSFORM, EcoreEventOutputTransform, this));
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_IGNORE_OUTPUT_TRANSFORM, EcoreEventIgnoreOutputTransform, this));

  // Register Rotate event
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_WINDOW_ROTATE, EcoreEventRotate, this));

  // Register Configure event
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_WINDOW_CONFIGURE, EcoreEventConfigure, this));

  // Register Touch events
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, EcoreEventMouseButtonDown, this));
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, EcoreEventMouseButtonUp, this));
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, EcoreEventMouseButtonMove, this));
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_CANCEL, EcoreEventMouseButtonCancel, this));
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_EVENT_MOUSE_RELATIVE_MOVE, EcoreEventMouseButtonRelativeMove, this));

  // Register Mouse wheel events
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_EVENT_MOUSE_WHEEL, EcoreEventMouseWheel, this));

  // Register Mouse IO events
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_EVENT_MOUSE_IN, EcoreEventMouseIn, this));
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_EVENT_MOUSE_OUT, EcoreEventMouseOut, this));

  // Register Detent event
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_EVENT_DETENT_ROTATE, EcoreEventDetentRotation, this));

  // Register Key events
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, EcoreEventKeyDown, this));
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_EVENT_KEY_UP, EcoreEventKeyUp, this));

  // Register Selection event - clipboard selection
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_DATA_SOURCE_SEND, EcoreEventDataSend, this));
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_OFFER_DATA_READY, EcoreEventDataReceive, this));

  // Register Effect Start/End event
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_EFFECT_START, EcoreEventEffectStart, this));
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_EFFECT_END, EcoreEventEffectEnd, this));

  // Register Keyboard repeat event
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_SEAT_KEYBOARD_REPEAT_CHANGED, EcoreEventSeatKeyboardRepeatChanged, this));

  // Register Window redraw request event
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_WINDOW_REDRAW_REQUEST, EcoreEventWindowRedrawRequest, this));

  // Register Window auxiliary event
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_AUX_MESSAGE, EcoreEventWindowAuxiliaryMessage, this));

  // Register Conformant change event
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_CONFORMANT_CHANGE, EcoreEventConformantChange, this));

#if defined(VCONF_ENABLED)
  // Register Vconf notify - font name and size
  vconf_notify_key_changed_for_ui_thread(DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_NAME, VconfNotifyFontNameChanged, this);
  vconf_notify_key_changed_for_ui_thread(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, VconfNotifyFontSizeChanged, this);
#endif

  // Register Window is moved and resized done event.
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_WINDOW_INTERACTIVE_MOVE_DONE, EcoreEventWindowMoveCompleted, this));
  mEcoreEventHandler.PushBack(ecore_event_handler_add(ECORE_WL2_EVENT_WINDOW_INTERACTIVE_RESIZE_DONE, EcoreEventWindowResizeCompleted, this));

  mDisplay = ecore_wl2_display_get(display);
  if(mDisplay)
  {
    wl_display* displayWrapper = static_cast<wl_display*>(wl_proxy_create_wrapper(mDisplay));
    if(displayWrapper)
    {
      mEventQueue = wl_display_create_queue(mDisplay);
      if(mEventQueue)
      {
        wl_proxy_set_queue(reinterpret_cast<wl_proxy*>(displayWrapper), mEventQueue);

        wl_registry* registry = wl_display_get_registry(displayWrapper);
        wl_registry_add_listener(registry, &registryListener, this);

        // To support ECORE_WL2_EVENT_CONFORMANT_CHANGE event handler
        ecore_wl2_window_conformant_set(mEcoreWindow, true);
      }

      wl_proxy_wrapper_destroy(displayWrapper);
    }
  }

  // get auxiliary hint
  Eina_List* hints = ecore_wl2_window_aux_hints_supported_get(mEcoreWindow);
  if(hints)
  {
    Eina_List* l    = NULL;
    char*      hint = NULL;

    for(l = hints, (hint = static_cast<char*>(eina_list_data_get(l))); l; l = eina_list_next(l), (hint = static_cast<char*>(eina_list_data_get(l))))
    {
      mSupportedAuxiliaryHints.push_back(hint);

      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::Initialize: %s\n", hint);
    }
  }
}

Eina_Bool WindowBaseEcoreWl2::OnIconifyStateChanged(void* data, int type, void* event)
{
  Ecore_Wl2_Event_Window_Iconify_State_Change* iconifyChangedEvent(static_cast<Ecore_Wl2_Event_Window_Iconify_State_Change*>(event));
  Eina_Bool                                    handled(ECORE_CALLBACK_PASS_ON);

  if(iconifyChangedEvent->win == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    if(iconifyChangedEvent->iconified == EINA_TRUE)
    {
      mIconifyChangedSignal.Emit(true);
    }
    else
    {
      mIconifyChangedSignal.Emit(false);
    }
    handled = ECORE_CALLBACK_DONE;
  }

  return handled;
}

Eina_Bool WindowBaseEcoreWl2::OnFocusIn(void* data, int type, void* event)
{
  Ecore_Wl2_Event_Focus_In* focusInEvent(static_cast<Ecore_Wl2_Event_Focus_In*>(event));

  if(focusInEvent->window == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Window EcoreEventWindowFocusIn\n");

    mFocusChangedSignal.Emit(true);
  }

  return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool WindowBaseEcoreWl2::OnFocusOut(void* data, int type, void* event)
{
  Ecore_Wl2_Event_Focus_Out* focusOutEvent(static_cast<Ecore_Wl2_Event_Focus_Out*>(event));

  if(focusOutEvent->window == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Window EcoreEventWindowFocusOut\n");

    mFocusChangedSignal.Emit(false);
  }

  return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool WindowBaseEcoreWl2::OnOutputTransform(void* data, int type, void* event)
{
  Ecore_Wl2_Event_Output_Transform* transformEvent(static_cast<Ecore_Wl2_Event_Output_Transform*>(event));

  if(transformEvent->output == ecore_wl2_window_output_find(mEcoreWindow))
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Window (%p) EcoreEventOutputTransform\n", mEcoreWindow);

    mScreenRotationAngle = GetScreenRotationAngle();

    mOutputTransformedSignal.Emit();
  }

  return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool WindowBaseEcoreWl2::OnIgnoreOutputTransform(void* data, int type, void* event)
{
  Ecore_Wl2_Event_Ignore_Output_Transform* ignoreTransformEvent(static_cast<Ecore_Wl2_Event_Ignore_Output_Transform*>(event));

  if(ignoreTransformEvent->win == mEcoreWindow)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "Window (%p) EcoreEventIgnoreOutputTransform\n", mEcoreWindow);

    mScreenRotationAngle = GetScreenRotationAngle();

    mOutputTransformedSignal.Emit();
  }

  return ECORE_CALLBACK_PASS_ON;
}

void WindowBaseEcoreWl2::OnRotation(void* data, int type, void* event)
{
  Ecore_Wl2_Event_Window_Rotation* ev(static_cast<Ecore_Wl2_Event_Window_Rotation*>(event));

  if(ev->win == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    DALI_LOG_RELEASE_INFO("WindowBaseEcoreWl2::OnRotation, angle: %d, width: %d, height: %d\n", ev->angle, ev->w, ev->h);

    RotationEvent rotationEvent;
    rotationEvent.angle     = ev->angle;
    rotationEvent.winResize = 0;

    if(ev->w == 0 || ev->h == 0)
    {
      // When rotation event does not have the window width or height,
      // previous DALi side window's size are used.
      ev->w = mWindowPositionSize.width;
      ev->h = mWindowPositionSize.height;
    }

    mWindowRotationAngle = ev->angle;

    mWindowPositionSize.width  = ev->w;
    mWindowPositionSize.height = ev->h;

    PositionSize newPositionSize = RecalculatePositionSizeToCurrentOrientation(mWindowPositionSize);

    rotationEvent.x      = newPositionSize.x;
    rotationEvent.y      = newPositionSize.y;
    rotationEvent.width  = newPositionSize.width;
    rotationEvent.height = newPositionSize.height;

    mRotationSignal.Emit(rotationEvent);
  }
}

void WindowBaseEcoreWl2::OnConfiguration(void* data, int type, void* event)
{
  Ecore_Wl2_Event_Window_Configure* ev(static_cast<Ecore_Wl2_Event_Window_Configure*>(event));

  if(ev && ev->win == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    // Note: To comply with the wayland protocol, Dali should make an ack_configure
    // by calling ecore_wl2_window_commit

    int tempWidth  = ev->w;
    int tempHeight = ev->h;

    // Initialize with previous size for skip resize when new size is 0.
    // When window is just moved or window is resized by client application,
    // The configure notification event's size will be 0.
    // If new size is 0, the resized work should be skip.
    int  newWidth    = mWindowPositionSize.width;
    int  newHeight   = mWindowPositionSize.height;
    bool windowMoved = false, windowResized = false;

    if(ev->x != mWindowPositionSize.x || ev->y != mWindowPositionSize.y)
    {
      windowMoved = true;
    }

    if(tempWidth != 0 && tempHeight != 0 && (tempWidth != mWindowPositionSize.width || tempHeight != mWindowPositionSize.height))
    {
      windowResized = true;
      newWidth      = tempWidth;
      newHeight     = tempHeight;
    }

    if(windowMoved || windowResized)
    {
      mWindowPositionSize.x      = ev->x;
      mWindowPositionSize.y      = ev->y;
      mWindowPositionSize.width  = newWidth;
      mWindowPositionSize.height = newHeight;
      DALI_LOG_RELEASE_INFO("Update position & resize signal by server, current angle [%d] x[%d] y[%d] w[%d] h[%d]\n", mWindowRotationAngle, mWindowPositionSize.x, mWindowPositionSize.y, mWindowPositionSize.width, mWindowPositionSize.height);

      ecore_wl2_window_geometry_set(mEcoreWindow, mWindowPositionSize.x, mWindowPositionSize.y, mWindowPositionSize.width, mWindowPositionSize.height);

      Dali::PositionSize newPositionSize = RecalculatePositionSizeToCurrentOrientation(mWindowPositionSize);
      mUpdatePositionSizeSignal.Emit(newPositionSize);
    }

    mMaximizeChangedSignal.Emit(static_cast<bool>(ev->states & ECORE_WL2_WINDOW_STATE_MAXIMIZED));

    ecore_wl2_window_commit(mEcoreWindow, EINA_FALSE);
  }
}

void WindowBaseEcoreWl2::OnMouseButtonDown(void* data, int type, void* event)
{
  Ecore_Event_Mouse_Button* touchEvent = static_cast<Ecore_Event_Mouse_Button*>(event);

  if(touchEvent->window == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_DOWN");

    Device::Class::Type    deviceClass;
    Device::Subclass::Type deviceSubclass;

    GetDeviceClass(ecore_device_class_get(touchEvent->dev), deviceClass);
    GetDeviceSubclass(ecore_device_subclass_get(touchEvent->dev), deviceSubclass);

    PointState::Type state(PointState::DOWN);

    if(deviceClass != Device::Class::Type::MOUSE)
    {
      // Check if the buttons field is set and ensure it's the primary touch button.
      // If this event was triggered by buttons other than the primary button (used for touch), then
      // just send an interrupted event to Core.
      if(touchEvent->buttons && (touchEvent->buttons != PRIMARY_TOUCH_BUTTON_ID))
      {
        state = PointState::INTERRUPTED;
      }
    }

    Integration::Point point;
    point.SetDeviceId(touchEvent->multi.device);
    point.SetState(state);
    point.SetScreenPosition(Vector2(touchEvent->x, touchEvent->y));
    point.SetRadius(touchEvent->multi.radius, Vector2(touchEvent->multi.radius_x, touchEvent->multi.radius_y));
    point.SetPressure(touchEvent->multi.pressure);
    point.SetAngle(Degree(touchEvent->multi.angle));
    point.SetDeviceClass(deviceClass);
    point.SetDeviceSubclass(deviceSubclass);
    point.SetMouseButton(static_cast<MouseButton::Type>(touchEvent->buttons));

    mTouchEventSignal.Emit(point, touchEvent->timestamp);
  }
}

void WindowBaseEcoreWl2::OnMouseButtonUp(void* data, int type, void* event)
{
  Ecore_Event_Mouse_Button* touchEvent = static_cast<Ecore_Event_Mouse_Button*>(event);

  if(touchEvent->window == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_UP");

    Device::Class::Type    deviceClass;
    Device::Subclass::Type deviceSubclass;

    GetDeviceClass(ecore_device_class_get(touchEvent->dev), deviceClass);
    GetDeviceSubclass(ecore_device_subclass_get(touchEvent->dev), deviceSubclass);

    Integration::Point point;
    point.SetDeviceId(touchEvent->multi.device);
    point.SetState(PointState::UP);
    point.SetScreenPosition(Vector2(touchEvent->x, touchEvent->y));
    point.SetRadius(touchEvent->multi.radius, Vector2(touchEvent->multi.radius_x, touchEvent->multi.radius_y));
    point.SetPressure(touchEvent->multi.pressure);
    point.SetAngle(Degree(touchEvent->multi.angle));
    point.SetDeviceClass(deviceClass);
    point.SetDeviceSubclass(deviceSubclass);
    point.SetMouseButton(static_cast<MouseButton::Type>(touchEvent->buttons));

    mTouchEventSignal.Emit(point, touchEvent->timestamp);
  }
}

void WindowBaseEcoreWl2::OnMouseButtonMove(void* data, int type, void* event)
{
  Ecore_Event_Mouse_Move* touchEvent = static_cast<Ecore_Event_Mouse_Move*>(event);

  if(touchEvent->window == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_MOVE");

    Device::Class::Type    deviceClass;
    Device::Subclass::Type deviceSubclass;

    GetDeviceClass(ecore_device_class_get(touchEvent->dev), deviceClass);
    GetDeviceSubclass(ecore_device_subclass_get(touchEvent->dev), deviceSubclass);

    Integration::Point point;
    point.SetDeviceId(touchEvent->multi.device);
    point.SetState(PointState::MOTION);
    point.SetScreenPosition(Vector2(touchEvent->x, touchEvent->y));
    point.SetRadius(touchEvent->multi.radius, Vector2(touchEvent->multi.radius_x, touchEvent->multi.radius_y));
    point.SetPressure(touchEvent->multi.pressure);
    point.SetAngle(Degree(touchEvent->multi.angle));
    point.SetDeviceClass(deviceClass);
    point.SetDeviceSubclass(deviceSubclass);

    mTouchEventSignal.Emit(point, touchEvent->timestamp);
  }
}

void WindowBaseEcoreWl2::OnMouseButtonRelativeMove(void* data, int type, void* event)
{
  Ecore_Event_Mouse_Relative_Move* relativeMoveEvent = static_cast<Ecore_Event_Mouse_Relative_Move*>(event);

  if(relativeMoveEvent->window == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_RELATIVE_MOVE");

    Device::Class::Type    deviceClass;
    Device::Subclass::Type deviceSubclass;

    GetDeviceClass(ecore_device_class_get(relativeMoveEvent->dev), deviceClass);
    GetDeviceSubclass(ecore_device_subclass_get(relativeMoveEvent->dev), deviceSubclass);

    Dali::DevelWindow::MouseRelativeEvent MouseRelativeEvent(Dali::DevelWindow::MouseRelativeEvent::Type::RELATIVE_MOVE, relativeMoveEvent->modifiers, relativeMoveEvent->timestamp, Vector2(relativeMoveEvent->dx, relativeMoveEvent->dy), Vector2(relativeMoveEvent->dx_unaccel, relativeMoveEvent->dy_unaccel), deviceClass, deviceSubclass);

    mMouseRelativeEventSignal.Emit(MouseRelativeEvent);
  }
}

void WindowBaseEcoreWl2::OnMouseButtonCancel(void* data, int type, void* event)
{
  Ecore_Event_Mouse_Button* touchEvent = static_cast<Ecore_Event_Mouse_Button*>(event);

  if(touchEvent->window == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_CANCEL");

    Integration::Point point;
    point.SetDeviceId(touchEvent->multi.device);
    point.SetState(PointState::INTERRUPTED);
    point.SetScreenPosition(Vector2(0.0f, 0.0f));

    mTouchEventSignal.Emit(point, touchEvent->timestamp);

    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::OnMouseButtonCancel\n");
  }
}

void WindowBaseEcoreWl2::OnMouseWheel(void* data, int type, void* event)
{
  Ecore_Event_Mouse_Wheel* mouseWheelEvent = static_cast<Ecore_Event_Mouse_Wheel*>(event);

  if(mouseWheelEvent->window == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_WHEEL");

    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::OnMouseWheel: direction: %d, modifiers: %d, x: %d, y: %d, z: %d\n", mouseWheelEvent->direction, mouseWheelEvent->modifiers, mouseWheelEvent->x, mouseWheelEvent->y, mouseWheelEvent->z);

    Integration::WheelEvent wheelEvent(Integration::WheelEvent::MOUSE_WHEEL, mouseWheelEvent->direction, mouseWheelEvent->modifiers, Vector2(mouseWheelEvent->x, mouseWheelEvent->y), mouseWheelEvent->z, mouseWheelEvent->timestamp);

    mWheelEventSignal.Emit(wheelEvent);
  }
}

void WindowBaseEcoreWl2::OnMouseInOut(void* data, int type, void* event, Dali::DevelWindow::MouseInOutEvent::Type action)
{
  Ecore_Event_Mouse_IO* mouseInOutEvent = static_cast<Ecore_Event_Mouse_IO*>(event);

  if(mouseInOutEvent->window == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_IN_OUT");

    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::OnMouseInOut: timestamp: %d, modifiers: %d, x: %d, y: %d\n", mouseInOutEvent->timestamp, mouseInOutEvent->modifiers, mouseInOutEvent->x, mouseInOutEvent->y);

    Device::Class::Type    deviceClass;
    Device::Subclass::Type deviceSubclass;

    GetDeviceClass(ecore_device_class_get(mouseInOutEvent->dev), deviceClass);
    GetDeviceSubclass(ecore_device_subclass_get(mouseInOutEvent->dev), deviceSubclass);

    Dali::DevelWindow::MouseInOutEvent inOutEvent(action, mouseInOutEvent->modifiers, Vector2(mouseInOutEvent->x, mouseInOutEvent->y), mouseInOutEvent->timestamp, deviceClass, deviceSubclass);

    mMouseInOutEventSignal.Emit(inOutEvent);
  }
}

void WindowBaseEcoreWl2::OnDetentRotation(void* data, int type, void* event)
{
  Ecore_Event_Detent_Rotate* detentEvent = static_cast<Ecore_Event_Detent_Rotate*>(event);

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl2::OnDetentRotation\n");

  int32_t clockwise = (detentEvent->direction == ECORE_DETENT_DIRECTION_CLOCKWISE) ? 1 : -1;

  Integration::WheelEvent wheelEvent(Integration::WheelEvent::CUSTOM_WHEEL, detentEvent->direction, 0, Vector2(0.0f, 0.0f), clockwise, detentEvent->timestamp);

  mWheelEventSignal.Emit(wheelEvent);
}

void WindowBaseEcoreWl2::OnKeyDown(void* data, int type, void* event)
{
  Ecore_Event_Key* keyEvent = static_cast<Ecore_Event_Key*>(event);

  if(keyEvent->window == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    std::string keyName(keyEvent->keyname);
    std::string logicalKey("");
    std::string keyString("");
    std::string compose("");

    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_KEY_DOWN");

    // Ensure key compose string is not NULL as keys like SHIFT or arrow have a null string.
    if(keyEvent->compose)
    {
      compose = keyEvent->compose;
    }

    // Ensure key symbol is not NULL as keys like SHIFT have a null string.
    if(keyEvent->key)
    {
      logicalKey = keyEvent->key;
    }

    int keyCode = 0;
    GetKeyCode(keyName, keyCode); // Get key code dynamically.

    if(keyCode == 0)
    {
      // Get a specific key code from dali key look up table.
      keyCode = KeyLookup::GetDaliKeyCode(keyEvent->keyname);
    }

    keyCode = (keyCode == -1) ? 0 : keyCode;
    int           modifier(keyEvent->modifiers);
    unsigned long time = keyEvent->timestamp;
    if(!strncmp(keyEvent->keyname, "Keycode-", 8))
    {
      keyCode = atoi(keyEvent->keyname + 8);
    }

    // Ensure key event string is not NULL as keys like SHIFT have a null string.
    if(keyEvent->string)
    {
      keyString = keyEvent->string;
    }

    std::string            deviceName;
    Device::Class::Type    deviceClass;
    Device::Subclass::Type deviceSubclass;

    GetDeviceName(keyEvent, deviceName);
    GetDeviceClass(ecore_device_class_get(keyEvent->dev), deviceClass);
    GetDeviceSubclass(ecore_device_subclass_get(keyEvent->dev), deviceSubclass);

    Integration::KeyEvent keyEvent(keyName, logicalKey, keyString, keyCode, modifier, time, Integration::KeyEvent::DOWN, compose, deviceName, deviceClass, deviceSubclass);

    mKeyEventSignal.Emit(keyEvent);
  }
}

void WindowBaseEcoreWl2::OnKeyUp(void* data, int type, void* event)
{
  Ecore_Event_Key* keyEvent = static_cast<Ecore_Event_Key*>(event);

  if(keyEvent->window == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
#if defined(ECORE_VERSION_MAJOR) && (ECORE_VERSION_MAJOR >= 1) && defined(ECORE_VERSION_MINOR) && (ECORE_VERSION_MINOR >= 23)
    // Cancel processing flag is sent because this key event will combine with the previous key. So, the event should not actually perform anything.
    if(keyEvent->event_flags & ECORE_EVENT_FLAG_CANCEL)
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::OnKeyUp: This event flag indicates the event is canceled. \n");
      return;
    }
#endif // Since ecore 1.23 version

    std::string keyName(keyEvent->keyname);
    std::string logicalKey("");
    std::string keyString("");
    std::string compose("");

    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_KEY_UP");

    // Ensure key compose string is not NULL as keys like SHIFT or arrow have a null string.
    if(keyEvent->compose)
    {
      compose = keyEvent->compose;
    }

    // Ensure key symbol is not NULL as keys like SHIFT have a null string.
    if(keyEvent->key)
    {
      logicalKey = keyEvent->key;
    }

    int keyCode = 0;
    GetKeyCode(keyName, keyCode); // Get key code dynamically.

    if(keyCode == 0)
    {
      // Get a specific key code from dali key look up table.
      keyCode = KeyLookup::GetDaliKeyCode(keyEvent->keyname);
    }

    keyCode = (keyCode == -1) ? 0 : keyCode;
    int           modifier(keyEvent->modifiers);
    unsigned long time = keyEvent->timestamp;
    if(!strncmp(keyEvent->keyname, "Keycode-", 8))
    {
      keyCode = atoi(keyEvent->keyname + 8);
    }

    // Ensure key event string is not NULL as keys like SHIFT have a null string.
    if(keyEvent->string)
    {
      keyString = keyEvent->string;
    }

    std::string            deviceName;
    Device::Class::Type    deviceClass;
    Device::Subclass::Type deviceSubclass;

    GetDeviceName(keyEvent, deviceName);
    GetDeviceClass(ecore_device_class_get(keyEvent->dev), deviceClass);
    GetDeviceSubclass(ecore_device_subclass_get(keyEvent->dev), deviceSubclass);

    Integration::KeyEvent keyEvent(keyName, logicalKey, keyString, keyCode, modifier, time, Integration::KeyEvent::UP, compose, deviceName, deviceClass, deviceSubclass);

    mKeyEventSignal.Emit(keyEvent);
  }
}

void WindowBaseEcoreWl2::OnDataSend(void* data, int type, void* event)
{
  mSelectionDataSendSignal.Emit(event);
}

void WindowBaseEcoreWl2::OnDataReceive(void* data, int type, void* event)
{
  mSelectionDataReceivedSignal.Emit(event);
}

void WindowBaseEcoreWl2::OnFontNameChanged()
{
  mStyleChangedSignal.Emit(StyleChange::DEFAULT_FONT_CHANGE);
}

void WindowBaseEcoreWl2::OnFontSizeChanged()
{
  mStyleChangedSignal.Emit(StyleChange::DEFAULT_FONT_SIZE_CHANGE);
}

void WindowBaseEcoreWl2::OnTransitionEffectEvent(WindowEffectState state, WindowEffectType type)
{
  mTransitionEffectEventSignal.Emit(state, type);
}

void WindowBaseEcoreWl2::OnKeyboardRepeatSettingsChanged()
{
  mKeyboardRepeatSettingsChangedSignal.Emit();
}

void WindowBaseEcoreWl2::OnEcoreEventWindowRedrawRequest()
{
  mWindowRedrawRequestSignal.Emit();
}

void WindowBaseEcoreWl2::OnEcoreEventWindowAuxiliaryMessage(void* event)
{
  Ecore_Wl2_Event_Aux_Message* message = static_cast<Ecore_Wl2_Event_Aux_Message*>(event);
  if(message)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::OnEcoreEventWindowAuxiliaryMessage, key:%s, value:%s \n", message->key, message->val);
    std::string           key(message->key);
    std::string           value(message->val);
    Dali::Property::Array options;

    if(message->options)
    {
      Eina_List* l;
      void*      data;
      EINA_LIST_FOREACH(message->options, l, data)
      {
        DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::OnEcoreEventWindowAuxiliaryMessage, option: %s\n", (char*)data);
        std::string option(static_cast<char*>(data));
        options.Add(option);
      }
    }

    mAuxiliaryMessageSignal.Emit(key, value, options);
  }
}

void WindowBaseEcoreWl2::OnEcoreEventConformantChange(void* event)
{
  Ecore_Wl2_Event_Conformant_Change* ev = static_cast<Ecore_Wl2_Event_Conformant_Change*>(event);
  if(ev && ev->win == static_cast<unsigned int>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    WindowInsetsPartType partType = WindowInsetsPartType::STATUS_BAR;

    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    switch(ev->part_type)
    {
      case ECORE_WL2_INDICATOR_PART:
      {
        partType = WindowInsetsPartType::STATUS_BAR;
        ecore_wl2_window_indicator_geometry_get(mEcoreWindow, &x, &y, &w, &h);
        break;
      }
      case ECORE_WL2_KEYBOARD_PART:
      {
        partType = WindowInsetsPartType::KEYBOARD;
        ecore_wl2_window_keyboard_geometry_get(mEcoreWindow, &x, &y, &w, &h);
        break;
      }
      case ECORE_WL2_CLIPBOARD_PART:
      {
        partType = WindowInsetsPartType::CLIPBOARD;
        ecore_wl2_window_clipboard_geometry_get(mEcoreWindow, &x, &y, &w, &h);
        break;
      }
      default:
      {
        break;
      }
    }

    WindowInsetsPartState partState = WindowInsetsPartState::INVISIBLE;

    int left   = 0;
    int right  = 0;
    int top    = 0;
    int bottom = 0;

    // Insets are applied only if system UI(e.g. virtual keyboard) satisfies the following 2 conditions.
    // 1. System UI fits to the window width or height.
    // 2. System UI begins or ends from the edge of window.
    // Otherwise, we should not resize window content because there would be empty space around system UI.
    bool applyInsets = false;

    // Zero insets are applied if state is invisible
    if(!ev->state)
    {
      applyInsets = true;
    }
    else
    {
      partState = WindowInsetsPartState::VISIBLE;

      int winX = mWindowPositionSize.x;
      int winY = mWindowPositionSize.y;
      int winW = mWindowPositionSize.width;
      int winH = mWindowPositionSize.height;

      if((x <= winX) && (x + w >= winX + winW))
      {
        if((y <= winY) && (y + h >= winY) && (y + h <= winY + winH))
        {
          top         = y + h - winY;
          applyInsets = true;
        }
        else if((y + h >= winY + winH) && (y >= winY) && (y <= winY + winH))
        {
          bottom      = winY + winH - y;
          applyInsets = true;
        }
      }
      else if((y <= winY) && (y + h >= winY + winH))
      {
        if((x <= winX) && (x + w >= winX) && (x + w <= winX + winW))
        {
          left        = x + w - winX;
          applyInsets = true;
        }
        else if((x + w >= winX + winW) && (x >= winX) && (x <= winX + winW))
        {
          right       = winX + winW - x;
          applyInsets = true;
        }
      }
    }

    if(applyInsets)
    {
      mInsetsChangedSignal.Emit(partType, partState, Extents(left, right, top, bottom));
    }
  }
}

void WindowBaseEcoreWl2::KeymapChanged(void* data, int type, void* event)
{
  Ecore_Wl2_Event_Seat_Keymap_Changed* changed = static_cast<Ecore_Wl2_Event_Seat_Keymap_Changed*>(event);
  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::KeymapChanged, keymap id[ %d ]\n", changed->id);
  Ecore_Wl2_Input* ecoreWlInput = ecore_wl2_input_default_input_get(changed->display);
  if(ecoreWlInput)
  {
    mKeyMap = ecore_wl2_input_keymap_get(ecoreWlInput);
  }
}

void WindowBaseEcoreWl2::OnMoveCompleted(void* event)
{
  Ecore_Wl2_Event_Window_Interactive_Move_Done* movedDoneEvent = static_cast<Ecore_Wl2_Event_Window_Interactive_Move_Done*>(event);
  if(movedDoneEvent && movedDoneEvent->win == static_cast<uint32_t>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    Dali::PositionSize orgPositionSize(movedDoneEvent->x, movedDoneEvent->y, movedDoneEvent->w, movedDoneEvent->h);
    Dali::PositionSize newPositionSize = RecalculatePositionSizeToCurrentOrientation(orgPositionSize);
    Dali::Int32Pair    newPosition(newPositionSize.x, newPositionSize.y);
    DALI_LOG_RELEASE_INFO("window(%p) has been moved by server[%d, %d]\n", mEcoreWindow, newPositionSize.x, newPositionSize.y);
    mMoveCompletedSignal.Emit(newPosition);
  }
}

void WindowBaseEcoreWl2::OnResizeCompleted(void* event)
{
  Ecore_Wl2_Event_Window_Interactive_Resize_Done* resizedDoneEvent = static_cast<Ecore_Wl2_Event_Window_Interactive_Resize_Done*>(event);
  if(resizedDoneEvent && resizedDoneEvent->win == static_cast<uint32_t>(ecore_wl2_window_id_get(mEcoreWindow)))
  {
    Dali::PositionSize orgPositionSize(resizedDoneEvent->x, resizedDoneEvent->y, resizedDoneEvent->w, resizedDoneEvent->h);
    Dali::PositionSize newPositionSize = RecalculatePositionSizeToCurrentOrientation(orgPositionSize);
    Dali::Uint16Pair   newSize(newPositionSize.width, newPositionSize.height);
    DALI_LOG_RELEASE_INFO("window(%p) has been resized by server[%d, %d]\n", mEcoreWindow, newPositionSize.width, newPositionSize.height);
    mResizeCompletedSignal.Emit(newSize);
  }
}

void WindowBaseEcoreWl2::RegistryGlobalCallback(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
  if(strcmp(interface, tizen_policy_interface.name) == 0)
  {
    uint32_t clientVersion = std::min(version, MAX_TIZEN_CLIENT_VERSION);

    mTizenPolicy = static_cast<tizen_policy*>(wl_registry_bind(registry, name, &tizen_policy_interface, clientVersion));
    if(!mTizenPolicy)
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::RegistryGlobalCallback: wl_registry_bind(tizen_policy_interface) is failed.\n");
      return;
    }

    tizen_policy_add_listener(mTizenPolicy, &tizenPolicyListener, data);

    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::RegistryGlobalCallback: tizen_policy_add_listener is called.\n");
  }
  else if(strcmp(interface, tizen_display_policy_interface.name) == 0)
  {
    mTizenDisplayPolicy = static_cast<tizen_display_policy*>(wl_registry_bind(registry, name, &tizen_display_policy_interface, version));
    if(!mTizenDisplayPolicy)
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::RegistryGlobalCallback: wl_registry_bind(tizen_display_policy_interface) is failed.\n");
      return;
    }

    tizen_display_policy_add_listener(mTizenDisplayPolicy, &tizenDisplayPolicyListener, data);

    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::RegistryGlobalCallback: tizen_display_policy_add_listener is called.\n");
  }
}

void WindowBaseEcoreWl2::RegistryGlobalCallbackRemove(void* data, struct wl_registry* registry, uint32_t id)
{
  mTizenPolicy        = NULL;
  mTizenDisplayPolicy = NULL;
}

void WindowBaseEcoreWl2::TizenPolicyConformantArea(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t conformantPart, uint32_t state, int32_t x, int32_t y, int32_t w, int32_t h)
{
  int  originalX, originalY, originalWidth, originalHeight;
  bool changed = false;

  if(!surface)
  {
    DALI_LOG_ERROR("Failed to get wayland surface in WindowBaseEcoreWl2::TizenPolicyConformantArea()\n");
    return;
  }

  if(conformantPart == TIZEN_POLICY_CONFORMANT_PART_INDICATOR)
  {
    ecore_wl2_window_indicator_geometry_get(mEcoreWindow, &originalX, &originalY, &originalWidth, &originalHeight);
    if((originalX != x) || (originalY != y) || (originalWidth != w) || (originalHeight != h))
    {
      ecore_wl2_window_indicator_geometry_set(mEcoreWindow, x, y, w, h);
      changed = true;
    }

    /**
     * The given state is based on the visibility value of indicator.
     * Thus we need to add 1 to it before comparing with indicator state.
     */
    Ecore_Wl2_Indicator_State indState = ecore_wl2_window_indicator_state_get(mEcoreWindow);
    if((state + 1) != indState)
    {
      ecore_wl2_window_indicator_state_set(mEcoreWindow, static_cast<Ecore_Wl2_Indicator_State>(state + 1));
      changed = true;
    }
  }
  else if(conformantPart == TIZEN_POLICY_CONFORMANT_PART_KEYBOARD)
  {
    ecore_wl2_window_keyboard_geometry_get(mEcoreWindow, &originalX, &originalY, &originalWidth, &originalHeight);
    if((originalX != x) || (originalY != y) || (originalWidth != w) || (originalHeight != h))
    {
      ecore_wl2_window_keyboard_geometry_set(mEcoreWindow, x, y, w, h);
      changed = true;
    }

    /**
     * The given state is based on the visibility value of virtual keyboard window.
     * Thus we need to add 1 to it before comparing with keyboard state.
     */
    Ecore_Wl2_Virtual_Keyboard_State kbdState = ecore_wl2_window_keyboard_state_get(mEcoreWindow);
    if((state + 1) != (kbdState))
    {
      ecore_wl2_window_keyboard_state_set(mEcoreWindow, static_cast<Ecore_Wl2_Virtual_Keyboard_State>(state + 1));
      changed = true;
    }
  }
  else if(conformantPart == TIZEN_POLICY_CONFORMANT_PART_CLIPBOARD)
  {
    ecore_wl2_window_clipboard_geometry_get(mEcoreWindow, &originalX, &originalY, &originalWidth, &originalHeight);
    if((originalX != x) || (originalY != y) || (originalWidth != w) || (originalHeight != h))
    {
      ecore_wl2_window_clipboard_geometry_set(mEcoreWindow, x, y, w, h);
      changed = true;
    }

    /**
     * The given state is based on the visibility value of clipboard window.
     * Thus we need to add 1 to it before comparing with clipboard state.
     */
    Ecore_Wl2_Clipboard_State clipState = ecore_wl2_window_clipboard_state_get(mEcoreWindow);
    if((state + 1) != clipState)
    {
      ecore_wl2_window_clipboard_state_set(mEcoreWindow, static_cast<Ecore_Wl2_Clipboard_State>(state + 1));
      changed = true;
    }
  }

  if(changed)
  {
    Ecore_Wl2_Event_Conformant_Change* ev = static_cast<Ecore_Wl2_Event_Conformant_Change*>(calloc(1, sizeof(Ecore_Wl2_Event_Conformant_Change)));

    if(!ev)
    {
      return;
    }
    ev->win       = GetNativeWindowId();
    ev->part_type = static_cast<Ecore_Wl2_Conformant_Part_Type>(conformantPart);
    ev->state     = state;
    ecore_event_add(ECORE_WL2_EVENT_CONFORMANT_CHANGE, ev, NULL, NULL);
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::TizenPolicyConformantArea: conformantPart = %u, state = %u, position = (%d, %d), size = (%d, %d)\n", conformantPart, state, x, y, w, h);
}

void WindowBaseEcoreWl2::TizenPolicyNotificationChangeDone(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, int32_t level, uint32_t state)
{
  mNotificationLevel           = level;
  mNotificationChangeState     = state;
  mNotificationLevelChangeDone = true;

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::TizenPolicyNotificationChangeDone: level = %d, state = %d\n", level, state);
}

void WindowBaseEcoreWl2::TizenPolicyScreenModeChangeDone(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t mode, uint32_t state)
{
  mScreenOffMode            = mode;
  mScreenOffModeChangeState = state;
  mScreenOffModeChangeDone  = true;

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::TizenPolicyScreenModeChangeDone: mode = %d, state = %d\n", mode, state);
}

void WindowBaseEcoreWl2::DisplayPolicyBrightnessChangeDone(void* data, struct tizen_display_policy* displayPolicy, struct wl_surface* surface, int32_t brightness, uint32_t state)
{
  mBrightness            = brightness;
  mBrightnessChangeState = state;
  mBrightnessChangeDone  = true;

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseEcoreWl2::DisplayPolicyBrightnessChangeDone: brightness = %d, state = %d\n", brightness, state);
}

void WindowBaseEcoreWl2::GetKeyCode(std::string keyName, int32_t& keyCode)
{
  xkb_keysym_t sym = XKB_KEY_NoSymbol;
  KeyCodeMap   foundKeyCode;

  sym = xkb_keysym_from_name(keyName.c_str(), XKB_KEYSYM_NO_FLAGS);
  if(sym == XKB_KEY_NoSymbol || !mKeyMap)
  {
    DALI_LOG_ERROR("Failed to get keysym or keymap in WindowBaseEcoreWl2\n");
    return;
  }

  foundKeyCode.keySym    = sym;
  foundKeyCode.isKeyCode = false;
  xkb_keymap_key_for_each(mKeyMap, FindKeyCode, &foundKeyCode);
  keyCode = static_cast<int32_t>(foundKeyCode.keyCode);
}

Any WindowBaseEcoreWl2::GetNativeWindow()
{
  return mEcoreWindow;
}

int WindowBaseEcoreWl2::GetNativeWindowId()
{
  return ecore_wl2_window_id_get(mEcoreWindow);
}

std::string WindowBaseEcoreWl2::GetNativeWindowResourceId()
{
#ifdef OVER_TIZEN_VERSION_7
  return std::to_string(ecore_wl2_window_resource_id_get(mEcoreWindow));
#else
  return std::string();
#endif
}

EGLNativeWindowType WindowBaseEcoreWl2::CreateEglWindow(int width, int height)
{
  int totalAngle = (mWindowRotationAngle + mScreenRotationAngle) % 360;

  START_DURATION_CHECK();
  if(totalAngle == 90 || totalAngle == 270)
  {
    mEglWindow = wl_egl_window_create(mWlSurface, height, width);
  }
  else
  {
    mEglWindow = wl_egl_window_create(mWlSurface, width, height);
  }
  FINISH_DURATION_CHECK("wl_egl_window_create");

  return static_cast<EGLNativeWindowType>(mEglWindow);
}

void WindowBaseEcoreWl2::DestroyEglWindow()
{
  if(mEglWindow != NULL)
  {
    START_DURATION_CHECK();
    wl_egl_window_destroy(mEglWindow);
    FINISH_DURATION_CHECK("wl_egl_window_destroy");

    mEglWindow = NULL;
  }
}

void WindowBaseEcoreWl2::SetEglWindowRotation(int angle)
{
  wl_egl_window_tizen_rotation rotation;

  switch(angle)
  {
    case 0:
    {
      rotation = WL_EGL_WINDOW_TIZEN_ROTATION_0;
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
      rotation = WL_EGL_WINDOW_TIZEN_ROTATION_0;
      break;
    }
  }

  START_DURATION_CHECK();
  wl_egl_window_tizen_set_rotation(mEglWindow, rotation);
  FINISH_DURATION_CHECK("wl_egl_window_tizen_set_rotation");
}

void WindowBaseEcoreWl2::SetEglWindowBufferTransform(int angle)
{
  wl_output_transform bufferTransform;

  switch(angle)
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

  DALI_LOG_RELEASE_INFO("wl_egl_window_tizen_set_buffer_transform() with buffer Transform [%d]\n", bufferTransform);
  START_DURATION_CHECK();
  wl_egl_window_tizen_set_buffer_transform(mEglWindow, bufferTransform);
  FINISH_DURATION_CHECK("wl_egl_window_tizen_set_buffer_transform");
}

void WindowBaseEcoreWl2::SetEglWindowTransform(int angle)
{
  wl_output_transform windowTransform;

  switch(angle)
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

  DALI_LOG_RELEASE_INFO("wl_egl_window_tizen_set_window_transform() with window Transform [%d]\n", windowTransform);
  START_DURATION_CHECK();
  wl_egl_window_tizen_set_window_transform(mEglWindow, windowTransform);
  FINISH_DURATION_CHECK("wl_egl_window_tizen_set_window_transform");
}

void WindowBaseEcoreWl2::ResizeEglWindow(PositionSize positionSize)
{
  DALI_LOG_RELEASE_INFO("wl_egl_window_resize(), (%d, %d) [%d x %d]\n", positionSize.x, positionSize.y, positionSize.width, positionSize.height);
  START_DURATION_CHECK();
  wl_egl_window_resize(mEglWindow, positionSize.width, positionSize.height, positionSize.x, positionSize.y);

  // Note: Both "Resize" and "MoveResize" cases can reach here, but only "MoveResize" needs to submit serial number
  if(mMoveResizeSerial != mLastSubmittedMoveResizeSerial)
  {
    wl_egl_window_tizen_set_window_serial(mEglWindow, mMoveResizeSerial);
    mLastSubmittedMoveResizeSerial = mMoveResizeSerial;
  }
  FINISH_DURATION_CHECK("wl_egl_window functions");
}

bool WindowBaseEcoreWl2::IsEglWindowRotationSupported()
{
  START_DURATION_CHECK();
  // Check capability
  wl_egl_window_tizen_capability capability = static_cast<wl_egl_window_tizen_capability>(wl_egl_window_tizen_get_capabilities(mEglWindow));
  FINISH_DURATION_CHECK("wl_egl_window_tizen_get_capabilities");

  if(capability == WL_EGL_WINDOW_TIZEN_CAPABILITY_ROTATION_SUPPORTED)
  {
    mSupportedPreProtation = true;
    return true;
  }
  mSupportedPreProtation = false;
  return false;
}

PositionSize WindowBaseEcoreWl2::RecalculatePositionSizeToSystem(PositionSize positionSize)
{
  PositionSize newPositionSize;
  int32_t      screenWidth, screenHeight;
  WindowSystem::GetScreenSize(screenWidth, screenHeight);

  if(mWindowRotationAngle == 90)
  {
    newPositionSize.x      = positionSize.y;
    newPositionSize.y      = screenHeight - (positionSize.x + positionSize.width);
    newPositionSize.width  = positionSize.height;
    newPositionSize.height = positionSize.width;
  }
  else if(mWindowRotationAngle == 180)
  {
    newPositionSize.x      = screenWidth - (positionSize.x + positionSize.width);
    newPositionSize.y      = screenHeight - (positionSize.y + positionSize.height);
    newPositionSize.width  = positionSize.width;
    newPositionSize.height = positionSize.height;
  }
  else if(mWindowRotationAngle == 270)
  {
    newPositionSize.x      = screenWidth - (positionSize.y + positionSize.height);
    newPositionSize.y      = positionSize.x;
    newPositionSize.width  = positionSize.height;
    newPositionSize.height = positionSize.width;
  }
  else
  {
    newPositionSize.x      = positionSize.x;
    newPositionSize.y      = positionSize.y;
    newPositionSize.width  = positionSize.width;
    newPositionSize.height = positionSize.height;
  }

  return newPositionSize;
}

PositionSize WindowBaseEcoreWl2::RecalculatePositionSizeToCurrentOrientation(PositionSize positionSize)
{
  PositionSize newPositionSize;
  int32_t      screenWidth, screenHeight;
  WindowSystem::GetScreenSize(screenWidth, screenHeight);

  if(mWindowRotationAngle == 90)
  {
    newPositionSize.x      = screenHeight - (positionSize.y + positionSize.height);
    newPositionSize.y      = positionSize.x;
    newPositionSize.width  = positionSize.height;
    newPositionSize.height = positionSize.width;
  }
  else if(mWindowRotationAngle == 180)
  {
    newPositionSize.x      = screenWidth - (positionSize.x + positionSize.width);
    newPositionSize.y      = screenHeight - (positionSize.y + positionSize.height);
    newPositionSize.width  = positionSize.width;
    newPositionSize.height = positionSize.height;
  }
  else if(mWindowRotationAngle == 270)
  {
    newPositionSize.x      = positionSize.y;
    newPositionSize.y      = screenWidth - (positionSize.x + positionSize.width);
    newPositionSize.width  = positionSize.height;
    newPositionSize.height = positionSize.width;
  }
  else
  {
    newPositionSize.x      = positionSize.x;
    newPositionSize.y      = positionSize.y;
    newPositionSize.width  = positionSize.width;
    newPositionSize.height = positionSize.height;
  }

  return newPositionSize;
}

void WindowBaseEcoreWl2::Move(PositionSize positionSize)
{
  PositionSize newPositionSize = RecalculatePositionSizeToSystem(positionSize);

  mWindowPositionSize = newPositionSize;
  DALI_LOG_RELEASE_INFO("ecore_wl2_window_position_set x[%d], y[%d]\n", newPositionSize.x, newPositionSize.y);
  START_DURATION_CHECK();
  ecore_wl2_window_position_set(mEcoreWindow, newPositionSize.x, newPositionSize.y);
  FINISH_DURATION_CHECK("ecore_wl2_window_position_set");
}

void WindowBaseEcoreWl2::Resize(PositionSize positionSize)
{
  PositionSize newPositionSize = RecalculatePositionSizeToSystem(positionSize);

  mWindowPositionSize = newPositionSize;
  DALI_LOG_RELEASE_INFO("ecore_wl2_window_sync_geometry_set, x[%d], y[%d], w{%d], h[%d]\n", newPositionSize.x, newPositionSize.y, newPositionSize.width, newPositionSize.height);
  START_DURATION_CHECK();
  ecore_wl2_window_sync_geometry_set(mEcoreWindow, ++mMoveResizeSerial, newPositionSize.x, newPositionSize.y, newPositionSize.width, newPositionSize.height);
  FINISH_DURATION_CHECK("ecore_wl2_window_sync_geometry_set");
}

void WindowBaseEcoreWl2::MoveResize(PositionSize positionSize)
{
  PositionSize newPositionSize = RecalculatePositionSizeToSystem(positionSize);

  mWindowPositionSize = newPositionSize;
  DALI_LOG_RELEASE_INFO("ecore_wl2_window_sync_geometry_set, x[%d], y[%d], w{%d], h[%d]\n", newPositionSize.x, newPositionSize.y, newPositionSize.width, newPositionSize.height);
  START_DURATION_CHECK();
  ecore_wl2_window_sync_geometry_set(mEcoreWindow, ++mMoveResizeSerial, newPositionSize.x, newPositionSize.y, newPositionSize.width, newPositionSize.height);
  FINISH_DURATION_CHECK("ecore_wl2_window_sync_geometry_set");
}

void WindowBaseEcoreWl2::SetLayout(unsigned int numCols, unsigned int numRows, unsigned int column, unsigned int row, unsigned int colSpan, unsigned int rowSpan)
{
  DALI_LOG_RELEASE_INFO("ecore_wl2_window_layout_set, numCols[%d], numRows[%d], column[%d], row[%d], colSpan[%d], rowSpan[%d]\n", numCols, numRows, column, row, colSpan, rowSpan);
  START_DURATION_CHECK();
  ecore_wl2_window_layout_set(mEcoreWindow, numCols, numRows, column, row, colSpan, rowSpan);
  FINISH_DURATION_CHECK("ecore_wl2_window_layout_set");
}

void WindowBaseEcoreWl2::SetClass(const std::string& name, const std::string& className)
{
  ecore_wl2_window_title_set(mEcoreWindow, name.c_str());
  ecore_wl2_window_class_set(mEcoreWindow, className.c_str());
}

void WindowBaseEcoreWl2::Raise()
{
  START_DURATION_CHECK();
  // Use ecore_wl2_window_activate to prevent the window shown without rendering
  ecore_wl2_window_activate(mEcoreWindow);
  FINISH_DURATION_CHECK("ecore_wl2_window_activate");
}

void WindowBaseEcoreWl2::Lower()
{
  START_DURATION_CHECK();
  ecore_wl2_window_lower(mEcoreWindow);
  FINISH_DURATION_CHECK("ecore_wl2_window_lower");
}

void WindowBaseEcoreWl2::Activate()
{
  START_DURATION_CHECK();
  ecore_wl2_window_activate(mEcoreWindow);
  FINISH_DURATION_CHECK("ecore_wl2_window_activate");
}

void WindowBaseEcoreWl2::Maximize(bool maximize)
{
  START_DURATION_CHECK();
  ecore_wl2_window_maximized_set(mEcoreWindow, maximize);
  FINISH_DURATION_CHECK("ecore_wl2_window_maximized_set");
}

bool WindowBaseEcoreWl2::IsMaximized() const
{
  return ecore_wl2_window_maximized_get(mEcoreWindow);
}

void WindowBaseEcoreWl2::SetMaximumSize(Dali::Window::WindowSize size)
{
  DALI_LOG_RELEASE_INFO("ecore_wl2_window_maximum_size_set, width: %d, height: %d\n", size.GetWidth(), size.GetHeight());
  START_DURATION_CHECK();
  ecore_wl2_window_maximum_size_set(mEcoreWindow, size.GetWidth(), size.GetHeight());
  FINISH_DURATION_CHECK("ecore_wl2_window_maximum_size_set");
  ecore_wl2_window_commit(mEcoreWindow, EINA_TRUE);
}

void WindowBaseEcoreWl2::Minimize(bool minimize)
{
  START_DURATION_CHECK();
  ecore_wl2_window_iconified_set(mEcoreWindow, minimize);
  FINISH_DURATION_CHECK("ecore_wl2_window_iconified_set");
}

bool WindowBaseEcoreWl2::IsMinimized() const
{
  return ecore_wl2_window_iconified_get(mEcoreWindow);
}

void WindowBaseEcoreWl2::SetMimimumSize(Dali::Window::WindowSize size)
{
  DALI_LOG_RELEASE_INFO("ecore_wl2_window_minimum_size_set, width: %d, height: %d\n", size.GetWidth(), size.GetHeight());
  START_DURATION_CHECK();
  ecore_wl2_window_minimum_size_set(mEcoreWindow, size.GetWidth(), size.GetHeight());
  FINISH_DURATION_CHECK("ecore_wl2_window_minimum_size_set");
  ecore_wl2_window_commit(mEcoreWindow, EINA_TRUE);
}

void WindowBaseEcoreWl2::SetAvailableAnlges(const std::vector<int>& angles)
{
  int rotations[4] = {0};
  DALI_LOG_RELEASE_INFO("WindowBaseEcoreWl2::SetAvailableAnlges, angle's count: %d, angles\n", angles.size());
  for(std::size_t i = 0; i < angles.size(); ++i)
  {
    rotations[i] = static_cast<int>(angles[i]);
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "%d ", rotations[i]);
  }

  START_DURATION_CHECK();
  ecore_wl2_window_available_rotations_set(mEcoreWindow, rotations, angles.size());
  FINISH_DURATION_CHECK("ecore_wl2_window_available_rotations_set");
}

void WindowBaseEcoreWl2::SetPreferredAngle(int angle)
{
  DALI_LOG_RELEASE_INFO("WindowBaseEcoreWl2::SetPreferredAngle, angle: %d\n", angle);
  START_DURATION_CHECK();
  ecore_wl2_window_preferred_rotation_set(mEcoreWindow, angle);
  FINISH_DURATION_CHECK("ecore_wl2_window_preferred_rotation_set");
}

void WindowBaseEcoreWl2::SetAcceptFocus(bool accept)
{
  START_DURATION_CHECK();
  ecore_wl2_window_focus_skip_set(mEcoreWindow, !accept);
  FINISH_DURATION_CHECK("ecore_wl2_window_focus_skip_set");
}

void WindowBaseEcoreWl2::Show()
{
  if(!mVisible)
  {
    START_DURATION_CHECK();
    ecore_wl2_window_geometry_set(mEcoreWindow, mWindowPositionSize.x, mWindowPositionSize.y, mWindowPositionSize.width, mWindowPositionSize.height);
    FINISH_DURATION_CHECK("ecore_wl2_window_geometry_set");
  }
  mVisible = true;

  ecore_wl2_window_show(mEcoreWindow);
}

void WindowBaseEcoreWl2::Hide()
{
  mVisible = false;
  ecore_wl2_window_hide(mEcoreWindow);
}

unsigned int WindowBaseEcoreWl2::GetSupportedAuxiliaryHintCount() const
{
  return mSupportedAuxiliaryHints.size();
}

std::string WindowBaseEcoreWl2::GetSupportedAuxiliaryHint(unsigned int index) const
{
  if(index >= GetSupportedAuxiliaryHintCount())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetSupportedAuxiliaryHint: Invalid index! [%d]\n", index);
  }

  return mSupportedAuxiliaryHints[index];
}

unsigned int WindowBaseEcoreWl2::AddAuxiliaryHint(const std::string& hint, const std::string& value)
{
  bool supported = false;

  // Check if the hint is suppported
  for(std::vector<std::string>::iterator iter = mSupportedAuxiliaryHints.begin(); iter != mSupportedAuxiliaryHints.end(); ++iter)
  {
    if(*iter == hint)
    {
      supported = true;
      break;
    }
  }

  if(!supported)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl2::AddAuxiliaryHint: Not supported auxiliary hint [%s]\n", hint.c_str());
    return 0;
  }

  // Check if the hint is already added
  for(unsigned int i = 0; i < mAuxiliaryHints.size(); i++)
  {
    if(mAuxiliaryHints[i].first == hint)
    {
      // Just change the value
      mAuxiliaryHints[i].second = value;

      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::AddAuxiliaryHint: Change! hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), i + 1);

      return i + 1; // id is index + 1
    }
  }

  // Add the hint
  mAuxiliaryHints.push_back(std::pair<std::string, std::string>(hint, value));

  unsigned int id = mAuxiliaryHints.size();

  START_DURATION_CHECK();
  ecore_wl2_window_aux_hint_add(mEcoreWindow, static_cast<int>(id), hint.c_str(), value.c_str());
  FINISH_DURATION_CHECK("ecore_wl2_window_aux_hint_add");

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::AddAuxiliaryHint: hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), id);

  return id;
}

bool WindowBaseEcoreWl2::RemoveAuxiliaryHint(unsigned int id)
{
  if(id == 0 || id > mAuxiliaryHints.size())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl2::RemoveAuxiliaryHint: Invalid id [%d]\n", id);
    return false;
  }

  mAuxiliaryHints[id - 1].second = std::string();

  START_DURATION_CHECK();
  ecore_wl2_window_aux_hint_del(mEcoreWindow, static_cast<int>(id));
  FINISH_DURATION_CHECK("ecore_wl2_window_aux_hint_del");

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::RemoveAuxiliaryHint: id = %d, hint = %s\n", id, mAuxiliaryHints[id - 1].first.c_str());

  return true;
}

bool WindowBaseEcoreWl2::SetAuxiliaryHintValue(unsigned int id, const std::string& value)
{
  if(id == 0 || id > mAuxiliaryHints.size())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl2::SetAuxiliaryHintValue: Invalid id [%d]\n", id);
    return false;
  }

  mAuxiliaryHints[id - 1].second = value;

  START_DURATION_CHECK();
  ecore_wl2_window_aux_hint_change(mEcoreWindow, static_cast<int>(id), value.c_str());
  FINISH_DURATION_CHECK("ecore_wl2_window_aux_hint_change");

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetAuxiliaryHintValue: id = %d, hint = %s, value = %s\n", id, mAuxiliaryHints[id - 1].first.c_str(), mAuxiliaryHints[id - 1].second.c_str());

  return true;
}

std::string WindowBaseEcoreWl2::GetAuxiliaryHintValue(unsigned int id) const
{
  if(id == 0 || id > mAuxiliaryHints.size())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Concise, "WindowBaseEcoreWl2::GetAuxiliaryHintValue: Invalid id [%d]\n", id);
    return std::string();
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetAuxiliaryHintValue: id = %d, hint = %s, value = %s\n", id, mAuxiliaryHints[id - 1].first.c_str(), mAuxiliaryHints[id - 1].second.c_str());

  return mAuxiliaryHints[id - 1].second;
}

unsigned int WindowBaseEcoreWl2::GetAuxiliaryHintId(const std::string& hint) const
{
  for(unsigned int i = 0; i < mAuxiliaryHints.size(); i++)
  {
    if(mAuxiliaryHints[i].first == hint)
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetAuxiliaryHintId: hint = %s, id = %d\n", hint.c_str(), i + 1);
      return i + 1;
    }
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetAuxiliaryHintId: Invalid hint! [%s]\n", hint.c_str());

  return 0;
}

void WindowBaseEcoreWl2::SetInputRegion(const Rect<int>& inputRegion)
{
  Rect<int> convertRegion = RecalculatePositionSizeToSystem(inputRegion);

  Eina_Rectangle rect;
  rect.x = convertRegion.x;
  rect.y = convertRegion.y;
  rect.w = convertRegion.width;
  rect.h = convertRegion.height;

  DALI_LOG_RELEASE_INFO("%p, Set input rect (%d, %d, %d x %d)\n", mEcoreWindow, rect.x, rect.y, rect.w, rect.h);
  START_DURATION_CHECK();
  ecore_wl2_window_input_rect_set(mEcoreWindow, &rect);
  FINISH_DURATION_CHECK("ecore_wl2_window_input_rect_set");
  ecore_wl2_window_commit(mEcoreWindow, EINA_TRUE);
}

void WindowBaseEcoreWl2::SetType(Dali::WindowType type)
{
  if(mType != type)
  {
    mType = type;
    Ecore_Wl2_Window_Type windowType;

    switch(type)
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
      case Dali::WindowType::IME:
      {
        windowType = ECORE_WL2_WINDOW_TYPE_NONE;
        break;
      }
      case Dali::WindowType::DESKTOP:
      {
        windowType = ECORE_WL2_WINDOW_TYPE_DESKTOP;
        break;
      }
      default:
      {
        windowType = ECORE_WL2_WINDOW_TYPE_TOPLEVEL;
        break;
      }
    }

    START_DURATION_CHECK();
    ecore_wl2_window_type_set(mEcoreWindow, windowType);
    FINISH_DURATION_CHECK("ecore_wl2_window_type_set");
  }
}

Dali::WindowType WindowBaseEcoreWl2::GetType() const
{
  return mType;
}

Dali::WindowOperationResult WindowBaseEcoreWl2::SetNotificationLevel(Dali::WindowNotificationLevel level)
{
  START_DURATION_CHECK();
  while(!mTizenPolicy)
  {
    wl_display_dispatch_queue(mDisplay, mEventQueue);
  }

  int notificationLevel;

  switch(level)
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
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetNotificationLevel: invalid level [%d]\n", level);
      notificationLevel = TIZEN_POLICY_LEVEL_DEFAULT;
      break;
    }
  }

  mNotificationLevelChangeDone = false;
  mNotificationChangeState     = TIZEN_POLICY_ERROR_STATE_NONE;

  tizen_policy_set_notification_level(mTizenPolicy, ecore_wl2_window_surface_get(mEcoreWindow), notificationLevel);

  int count = 0;

  while(!mNotificationLevelChangeDone && count < 3)
  {
    ecore_wl2_display_flush(ecore_wl2_connected_display_get(NULL));
    wl_display_dispatch_queue(mDisplay, mEventQueue);
    count++;
  }
  FINISH_DURATION_CHECK("ecore_wl2 & wl_display");

  if(!mNotificationLevelChangeDone)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetNotificationLevel: Level change is failed [%d, %d]\n", level, mNotificationChangeState);
    return Dali::WindowOperationResult::UNKNOWN_ERROR;
  }
  else if(mNotificationChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetNotificationLevel: Permission denied! [%d]\n", level);
    return Dali::WindowOperationResult::PERMISSION_DENIED;
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetNotificationLevel: Level is changed [%d]\n", mNotificationLevel);

  return Dali::WindowOperationResult::SUCCEED;
}

Dali::WindowNotificationLevel WindowBaseEcoreWl2::GetNotificationLevel() const
{
  while(!mTizenPolicy)
  {
    wl_display_dispatch_queue(mDisplay, mEventQueue);
  }

  int count = 0;

  while(!mNotificationLevelChangeDone && count < 3)
  {
    ecore_wl2_display_flush(ecore_wl2_connected_display_get(NULL));
    wl_display_dispatch_queue(mDisplay, mEventQueue);
    count++;
  }

  if(!mNotificationLevelChangeDone)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetNotificationLevel: Error! [%d]\n", mNotificationChangeState);
    return Dali::WindowNotificationLevel::NONE;
  }

  Dali::WindowNotificationLevel level;

  switch(mNotificationLevel)
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
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetNotificationLevel: invalid level [%d]\n", mNotificationLevel);
      level = Dali::WindowNotificationLevel::NONE;
      break;
    }
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetNotificationLevel: level [%d]\n", mNotificationLevel);

  return level;
}

void WindowBaseEcoreWl2::SetOpaqueState(bool opaque)
{
  while(!mTizenPolicy)
  {
    wl_display_dispatch_queue(mDisplay, mEventQueue);
  }

  tizen_policy_set_opaque_state(mTizenPolicy, ecore_wl2_window_surface_get(mEcoreWindow), (opaque ? 1 : 0));
}

Dali::WindowOperationResult WindowBaseEcoreWl2::SetScreenOffMode(WindowScreenOffMode screenOffMode)
{
  while(!mTizenPolicy)
  {
    wl_display_dispatch_queue(mDisplay, mEventQueue);
  }

  mScreenOffModeChangeDone  = false;
  mScreenOffModeChangeState = TIZEN_POLICY_ERROR_STATE_NONE;

  unsigned int mode = 0;

  switch(screenOffMode)
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

  tizen_policy_set_window_screen_mode(mTizenPolicy, ecore_wl2_window_surface_get(mEcoreWindow), mode);

  int count = 0;

  while(!mScreenOffModeChangeDone && count < 3)
  {
    ecore_wl2_display_flush(ecore_wl2_connected_display_get(NULL));
    wl_display_dispatch_queue(mDisplay, mEventQueue);
    count++;
  }

  if(!mScreenOffModeChangeDone)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetScreenOffMode: Screen mode change is failed [%d, %d]\n", screenOffMode, mScreenOffModeChangeState);
    return Dali::WindowOperationResult::UNKNOWN_ERROR;
  }
  else if(mScreenOffModeChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetScreenOffMode: Permission denied! [%d]\n", screenOffMode);
    return Dali::WindowOperationResult::PERMISSION_DENIED;
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetScreenOffMode: Screen mode is changed [%d]\n", mScreenOffMode);

  return Dali::WindowOperationResult::SUCCEED;
}

WindowScreenOffMode WindowBaseEcoreWl2::GetScreenOffMode() const
{
  while(!mTizenPolicy)
  {
    wl_display_dispatch_queue(mDisplay, mEventQueue);
  }

  int count = 0;

  while(!mScreenOffModeChangeDone && count < 3)
  {
    ecore_wl2_display_flush(ecore_wl2_connected_display_get(NULL));
    wl_display_dispatch_queue(mDisplay, mEventQueue);
    count++;
  }

  if(!mScreenOffModeChangeDone)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetScreenOffMode: Error! [%d]\n", mScreenOffModeChangeState);
    return WindowScreenOffMode::TIMEOUT;
  }

  WindowScreenOffMode screenMode = WindowScreenOffMode::TIMEOUT;

  switch(mScreenOffMode)
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

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetScreenOffMode: screen mode [%d]\n", mScreenOffMode);

  return screenMode;
}

Dali::WindowOperationResult WindowBaseEcoreWl2::SetBrightness(int brightness)
{
  START_DURATION_CHECK();
  while(!mTizenDisplayPolicy)
  {
    wl_display_dispatch_queue(mDisplay, mEventQueue);
  }

  mBrightnessChangeDone  = false;
  mBrightnessChangeState = TIZEN_POLICY_ERROR_STATE_NONE;

  tizen_display_policy_set_window_brightness(mTizenDisplayPolicy, ecore_wl2_window_surface_get(mEcoreWindow), brightness);

  int count = 0;

  while(!mBrightnessChangeDone && count < 3)
  {
    ecore_wl2_display_flush(ecore_wl2_connected_display_get(NULL));
    wl_display_dispatch_queue(mDisplay, mEventQueue);
    count++;
  }
  FINISH_DURATION_CHECK("ecore_wl2_display_flush");

  if(!mBrightnessChangeDone)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetBrightness: Brightness change is failed [%d, %d]\n", brightness, mBrightnessChangeState);
    return Dali::WindowOperationResult::UNKNOWN_ERROR;
  }
  else if(mBrightnessChangeState == TIZEN_POLICY_ERROR_STATE_PERMISSION_DENIED)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetBrightness: Permission denied! [%d]\n", brightness);
    return Dali::WindowOperationResult::PERMISSION_DENIED;
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::SetBrightness: Brightness is changed [%d]\n", mBrightness);

  return Dali::WindowOperationResult::SUCCEED;
}

int WindowBaseEcoreWl2::GetBrightness() const
{
  START_DURATION_CHECK();
  while(!mTizenDisplayPolicy)
  {
    wl_display_dispatch_queue(mDisplay, mEventQueue);
  }

  int count = 0;

  while(!mBrightnessChangeDone && count < 3)
  {
    ecore_wl2_display_flush(ecore_wl2_connected_display_get(NULL));
    wl_display_dispatch_queue(mDisplay, mEventQueue);
    count++;
  }
  FINISH_DURATION_CHECK("ecore_wl2_display_flush");

  if(!mBrightnessChangeDone)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetBrightness: Error! [%d]\n", mBrightnessChangeState);
    return 0;
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseEcoreWl2::GetBrightness: Brightness [%d]\n", mBrightness);

  return mBrightness;
}

bool WindowBaseEcoreWl2::GrabKey(Dali::KEY key, KeyGrab::KeyGrabMode grabMode)
{
  Ecore_Wl2_Window_Keygrab_Mode mode;

  switch(grabMode)
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

  return ecore_wl2_window_keygrab_set(mEcoreWindow, KeyLookup::GetKeyName(key), 0, 0, 0, mode);
}

bool WindowBaseEcoreWl2::UngrabKey(Dali::KEY key)
{
  return ecore_wl2_window_keygrab_unset(mEcoreWindow, KeyLookup::GetKeyName(key), 0, 0);
}

bool WindowBaseEcoreWl2::GrabKeyList(const Dali::Vector<Dali::KEY>& key, const Dali::Vector<KeyGrab::KeyGrabMode>& grabMode, Dali::Vector<bool>& result)
{
  int keyCount         = key.Count();
  int keyGrabModeCount = grabMode.Count();

  if(keyCount != keyGrabModeCount || keyCount == 0)
  {
    return false;
  }

  eina_init();

  Eina_List*                     keyList = NULL;
  Ecore_Wl2_Window_Keygrab_Info* info    = new Ecore_Wl2_Window_Keygrab_Info[keyCount];

  for(int index = 0; index < keyCount; ++index)
  {
    info[index].key = const_cast<char*>(KeyLookup::GetKeyName(key[index]));

    switch(grabMode[index])
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

    keyList = eina_list_append(keyList, &info);
  }

  START_DURATION_CHECK();
  Eina_List* grabList = ecore_wl2_window_keygrab_list_set(mEcoreWindow, keyList);
  FINISH_DURATION_CHECK("ecore_wl2_window_keygrab_list_set");

  result.Resize(keyCount, true);

  Eina_List* l        = NULL;
  Eina_List* m        = NULL;
  void*      listData = NULL;
  void*      data     = NULL;
  if(grabList != NULL)
  {
    EINA_LIST_FOREACH(grabList, m, data)
    {
      int index = 0;
      EINA_LIST_FOREACH(keyList, l, listData)
      {
        if(static_cast<Ecore_Wl2_Window_Keygrab_Info*>(listData)->key == NULL)
        {
          DALI_LOG_ERROR("input key list has null data!");
          break;
        }

        if(strcmp(static_cast<char*>(data), static_cast<Ecore_Wl2_Window_Keygrab_Info*>(listData)->key) == 0)
        {
          result[index] = false;
        }
        ++index;
      }
    }
  }

  delete[] info;

  eina_list_free(keyList);
  eina_list_free(grabList);
  eina_shutdown();

  return true;
}

bool WindowBaseEcoreWl2::UngrabKeyList(const Dali::Vector<Dali::KEY>& key, Dali::Vector<bool>& result)
{
  int keyCount = key.Count();
  if(keyCount == 0)
  {
    return false;
  }

  eina_init();

  Eina_List*                     keyList = NULL;
  Ecore_Wl2_Window_Keygrab_Info* info    = new Ecore_Wl2_Window_Keygrab_Info[keyCount];

  for(int index = 0; index < keyCount; ++index)
  {
    info[index].key = const_cast<char*>(KeyLookup::GetKeyName(key[index]));
    keyList         = eina_list_append(keyList, &info);
  }

  START_DURATION_CHECK();
  Eina_List* ungrabList = ecore_wl2_window_keygrab_list_unset(mEcoreWindow, keyList);
  FINISH_DURATION_CHECK("ecore_wl2_window_keygrab_list_unset");

  result.Resize(keyCount, true);

  Eina_List* l        = NULL;
  Eina_List* m        = NULL;
  void*      listData = NULL;
  void*      data     = NULL;

  if(ungrabList != NULL)
  {
    EINA_LIST_FOREACH(ungrabList, m, data)
    {
      int index = 0;
      EINA_LIST_FOREACH(keyList, l, listData)
      {
        if(strcmp(static_cast<char*>(data), static_cast<Ecore_Wl2_Window_Keygrab_Info*>(listData)->key) == 0)
        {
          result[index] = false;
        }
        ++index;
      }
    }
  }

  delete[] info;

  eina_list_free(keyList);
  eina_list_free(ungrabList);
  eina_shutdown();

  return true;
}

void WindowBaseEcoreWl2::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  // calculate DPI
  float xres, yres;

  Ecore_Wl2_Output* output = ecore_wl2_window_output_find(mEcoreWindow);

  // 1 inch = 25.4 millimeters
  xres = ecore_wl2_output_dpi_get(output);
  yres = ecore_wl2_output_dpi_get(output);

  dpiHorizontal = int(xres + 0.5f); // rounding
  dpiVertical   = int(yres + 0.5f);
}

int WindowBaseEcoreWl2::GetWindowRotationAngle() const
{
  int orientation = mWindowRotationAngle;
  if(mSupportedPreProtation)
  {
    orientation = 0;
  }
  return orientation;
}

int WindowBaseEcoreWl2::GetScreenRotationAngle()
{
  if(mSupportedPreProtation)
  {
    DALI_LOG_RELEASE_INFO("Support PreRotation and return 0\n");
    return 0;
  }
  int transform;
  if(ecore_wl2_window_ignore_output_transform_get(mEcoreWindow))
  {
    transform = 0;
  }
  else
  {
    transform = ecore_wl2_output_transform_get(ecore_wl2_window_output_find(mEcoreWindow));
  }
  mScreenRotationAngle = transform * 90;
  return mScreenRotationAngle;
}

void WindowBaseEcoreWl2::SetWindowRotationAngle(int degree)
{
  mWindowRotationAngle = degree;
  ecore_wl2_window_rotation_set(mEcoreWindow, degree);
}

void WindowBaseEcoreWl2::WindowRotationCompleted(int degree, int width, int height)
{
  START_DURATION_CHECK();
  ecore_wl2_window_rotation_change_done_send(mEcoreWindow, degree, width, height);
  FINISH_DURATION_CHECK("ecore_wl2_window_rotation_change_done_send");
}

void WindowBaseEcoreWl2::SetTransparency(bool transparent)
{
  START_DURATION_CHECK();
  ecore_wl2_window_alpha_set(mEcoreWindow, transparent);
  FINISH_DURATION_CHECK("ecore_wl2_window_alpha_set");
}

void WindowBaseEcoreWl2::CreateWindow(PositionSize positionSize)
{
  Ecore_Wl2_Display* display = ecore_wl2_display_connect(NULL);
  if(!display)
  {
    DALI_ASSERT_ALWAYS(0 && "Failed to get display");
  }

  START_DURATION_CHECK();
  ecore_wl2_display_sync(display);

  mEcoreWindow = ecore_wl2_window_new(display, NULL, positionSize.x, positionSize.y, positionSize.width, positionSize.height);

  if(mEcoreWindow == 0)
  {
    DALI_ASSERT_ALWAYS(0 && "Failed to create Wayland window");
  }

  // Set default type
  ecore_wl2_window_type_set(mEcoreWindow, ECORE_WL2_WINDOW_TYPE_TOPLEVEL);
  FINISH_DURATION_CHECK("ecore_wl2 functions");
}

void WindowBaseEcoreWl2::SetParent(WindowBase* parentWinBase, bool belowParent)
{
  Ecore_Wl2_Window* ecoreParent = NULL;
  if(parentWinBase)
  {
    WindowBaseEcoreWl2* winBaseEcore2 = static_cast<WindowBaseEcoreWl2*>(parentWinBase);
    ecoreParent                       = winBaseEcore2->mEcoreWindow;
  }

  START_DURATION_CHECK();
  ecore_wl2_window_transient_parent_set(mEcoreWindow, ecoreParent, belowParent);
  FINISH_DURATION_CHECK("ecore_wl2_window_transient_parent_set");
}

int WindowBaseEcoreWl2::CreateFrameRenderedSyncFence()
{
  return wl_egl_window_tizen_create_commit_sync_fd(mEglWindow);
}

int WindowBaseEcoreWl2::CreateFramePresentedSyncFence()
{
  return wl_egl_window_tizen_create_presentation_sync_fd(mEglWindow);
}

void WindowBaseEcoreWl2::SetPositionSizeWithAngle(PositionSize positionSize, int angle)
{
  DALI_LOG_RELEASE_INFO("WindowBaseEcoreWl2::SetPositionSizeWithAngle, angle: %d, x: %d, y: %d, w: %d, h: %d\n", angle, positionSize.x, positionSize.y, positionSize.width, positionSize.height);
  START_DURATION_CHECK();
  ecore_wl2_window_rotation_geometry_set(mEcoreWindow, angle, positionSize.x, positionSize.y, positionSize.width, positionSize.height);
  FINISH_DURATION_CHECK("ecore_wl2_window_rotation_geometry_set");
}

void WindowBaseEcoreWl2::InitializeIme()
{
  Eina_Iterator*      globals;
  struct wl_registry* registry;
  Ecore_Wl2_Global*   global;
  Ecore_Wl2_Display*  ecoreWl2Display;

  if(!(ecoreWl2Display = ecore_wl2_connected_display_get(NULL)))
  {
    DALI_LOG_ERROR("WindowBaseEcoreWl2::InitializeIme(), fail to get ecore_wl2 connected display\n");
    return;
  }

  DALI_LOG_RELEASE_INFO("InitializeIme:  Ecore_Wl2_Display: %p, ecore wl window: %p\n", ecoreWl2Display, mEcoreWindow);

  if(!(registry = ecore_wl2_display_registry_get(ecoreWl2Display)))
  {
    DALI_LOG_ERROR("WindowBaseEcoreWl2::InitializeIme(), fail to get ecore_wl2 display registry\n");
    return;
  }

  if(!(globals = ecore_wl2_display_globals_get(ecoreWl2Display)))
  {
    DALI_LOG_ERROR("WindowBaseEcoreWl2::InitializeIme(), fail to get ecore_wl2 globals\n");
    return;
  }

  START_DURATION_CHECK();
  EINA_ITERATOR_FOREACH(globals, global)
  {
#ifdef OVER_TIZEN_VERSION_7
    if(strcmp(global->interface, "zwp_input_panel_v1") == 0)
    {
      mWlInputPanel = (zwp_input_panel_v1*)wl_registry_bind(registry, global->id, &zwp_input_panel_v1_interface, 1);
    }
#else
    if(strcmp(global->interface, "wl_input_panel") == 0)
    {
      mWlInputPanel = (wl_input_panel*)wl_registry_bind(registry, global->id, &wl_input_panel_interface, 1);
    }
#endif
    else if(strcmp(global->interface, "wl_output") == 0)
    {
      mWlOutput = (wl_output*)wl_registry_bind(registry, global->id, &wl_output_interface, 1);
    }
  }

  if(!mWlInputPanel)
  {
    DALI_LOG_ERROR("WindowBaseEcoreWl2::InitializeIme(), fail to get wayland input panel interface\n");
    return;
  }

  if(!mWlOutput)
  {
    DALI_LOG_ERROR("WindowBaseEcoreWl2::InitializeIme(), fail to get wayland output panel interface\n");
    return;
  }
#ifdef OVER_TIZEN_VERSION_7
  mWlInputPanelSurface = zwp_input_panel_v1_get_input_panel_surface(mWlInputPanel, mWlSurface);
#else
  mWlInputPanelSurface = wl_input_panel_get_input_panel_surface(mWlInputPanel, mWlSurface);
#endif
  if(!mWlInputPanelSurface)
  {
    DALI_LOG_ERROR("WindowBaseEcoreWl2::InitializeIme(), fail to get wayland input panel surface\n");
    return;
  }
#ifdef OVER_TIZEN_VERSION_7
  zwp_input_panel_surface_v1_set_toplevel(mWlInputPanelSurface, mWlOutput, ZWP_INPUT_PANEL_SURFACE_V1_POSITION_CENTER_BOTTOM);
#else
  wl_input_panel_surface_set_toplevel(mWlInputPanelSurface, mWlOutput, WL_INPUT_PANEL_SURFACE_POSITION_CENTER_BOTTOM);
#endif
  FINISH_DURATION_CHECK("zwp_input_panel_surface_v1_set_toplevel");
}

void WindowBaseEcoreWl2::ImeWindowReadyToRender()
{
  if(!mWlInputPanelSurface)
  {
    DALI_LOG_ERROR("WindowBaseEcoreWl2::ImeWindowReadyToRender(), wayland input panel surface is null\n");
    return;
  }

  START_DURATION_CHECK();
#ifdef OVER_TIZEN_VERSION_7
  zwp_input_panel_surface_v1_set_ready(mWlInputPanelSurface, 1);
#else
  wl_input_panel_surface_set_ready(mWlInputPanelSurface, 1);
#endif
  FINISH_DURATION_CHECK("zwp_input_panel_surface_v1_set_ready");
}

void WindowBaseEcoreWl2::RequestMoveToServer()
{
  Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(NULL);
  if(!display)
  {
    DALI_LOG_ERROR("WindowBaseEcoreWl2::RequestMoveToServer, Fail to get ecore_wl2_display\n");
    return;
  }

  Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(display);
  if(!input)
  {
    DALI_LOG_ERROR("WindowBaseEcoreWl2::RequestMoveToServer, Fail to get default Ecore_Wl2_Input\n");
    return;
  }

  START_DURATION_CHECK();
  ecore_wl2_window_move(mEcoreWindow, input);
  FINISH_DURATION_CHECK("ecore_wl2_window_move");
  DALI_LOG_RELEASE_INFO("WindowBaseEcoreWl2::RequestMoveToServer, starts the window[%p] is moved by server\n", mEcoreWindow);
}

void WindowBaseEcoreWl2::RequestResizeToServer(WindowResizeDirection direction)
{
  Ecore_Wl2_Display* display = ecore_wl2_connected_display_get(NULL);
  if(!display)
  {
    DALI_LOG_ERROR("WindowBaseEcoreWl2::RequestResizeToServer, Fail to get ecore_wl2_display\n");
    return;
  }

  Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get(display);
  if(!input)
  {
    DALI_LOG_ERROR("WindowBaseEcoreWl2::RequestResizeToServer, Fail to get default Ecore_Wl2_Input\n");
    return;
  }

  ResizeLocation location = RecalculateLocationToCurrentOrientation(direction, mWindowRotationAngle);

  START_DURATION_CHECK();
  ecore_wl2_window_resize(mEcoreWindow, input, static_cast<int>(location));
  FINISH_DURATION_CHECK("ecore_wl2_window_resize");
  DALI_LOG_RELEASE_INFO("WindowBaseEcoreWl2::RequestResizeToServer, starts the window[%p] is resized by server, direction:%d oriention:%d mode:%d\n", mEcoreWindow, direction, mWindowRotationAngle, location);
}

void WindowBaseEcoreWl2::EnableFloatingMode(bool enable)
{
  DALI_LOG_RELEASE_INFO("WindowBaseEcoreWl2::EnableFloatingMode, floating mode flag: [%p], enable [%d]\n", mEcoreWindow, enable);
  START_DURATION_CHECK();
  if(enable == true)
  {
    ecore_wl2_window_floating_mode_set(mEcoreWindow, EINA_TRUE);
  }
  else
  {
    ecore_wl2_window_floating_mode_set(mEcoreWindow, EINA_FALSE);
  }
  FINISH_DURATION_CHECK("ecore_wl2_window_floating_mode_set");
}

bool WindowBaseEcoreWl2::IsFloatingModeEnabled() const
{
  return ecore_wl2_window_floating_mode_get(mEcoreWindow);
}

void WindowBaseEcoreWl2::IncludeInputRegion(const Rect<int>& inputRegion)
{
  Rect<int>      convertRegion = RecalculatePositionSizeToSystem(inputRegion);
  Eina_Rectangle rect;

  rect.x = convertRegion.x;
  rect.y = convertRegion.y;
  rect.w = convertRegion.width;
  rect.h = convertRegion.height;

  DALI_LOG_RELEASE_INFO("%p, Add input_rect(%d, %d, %d x %d)\n", mEcoreWindow, rect.x, rect.y, rect.w, rect.h);
  START_DURATION_CHECK();
  ecore_wl2_window_input_rect_add(mEcoreWindow, &rect);
  FINISH_DURATION_CHECK("ecore_wl2_window_input_rect_add");
  ecore_wl2_window_commit(mEcoreWindow, EINA_TRUE);
}

void WindowBaseEcoreWl2::ExcludeInputRegion(const Rect<int>& inputRegion)
{
  Rect<int>      convertRegion = RecalculatePositionSizeToSystem(inputRegion);
  Eina_Rectangle rect;

  rect.x = convertRegion.x;
  rect.y = convertRegion.y;
  rect.w = convertRegion.width;
  rect.h = convertRegion.height;

  DALI_LOG_RELEASE_INFO("%p, Subtract input_rect(%d, %d, %d x %d)\n", mEcoreWindow, rect.x, rect.y, rect.w, rect.h);
  START_DURATION_CHECK();
  ecore_wl2_window_input_rect_subtract(mEcoreWindow, &rect);
  FINISH_DURATION_CHECK("ecore_wl2_window_input_rect_subtract");
  ecore_wl2_window_commit(mEcoreWindow, EINA_TRUE);
}

bool WindowBaseEcoreWl2::PointerConstraintsLock()
{
  return ecore_wl2_window_pointer_constraints_lock_pointer(mEcoreWindow);
}

bool WindowBaseEcoreWl2::PointerConstraintsUnlock()
{
  return ecore_wl2_window_pointer_constraints_unlock_pointer(mEcoreWindow);
}

void WindowBaseEcoreWl2::LockedPointerRegionSet(int32_t x, int32_t y, int32_t width, int32_t height)
{
  ecore_wl2_window_locked_pointer_region_set(mEcoreWindow, x, y, width, height);
}

void WindowBaseEcoreWl2::LockedPointerCursorPositionHintSet(int32_t x, int32_t y)
{
  ecore_wl2_window_locked_pointer_cursor_position_hint_set(mEcoreWindow, x, y);
}

bool WindowBaseEcoreWl2::PointerWarp(int32_t x, int32_t y)
{
  return ecore_wl2_window_pointer_warp(mEcoreWindow, x, y);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
