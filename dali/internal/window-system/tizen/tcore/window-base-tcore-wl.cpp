/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

// C style cast suppression
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

// CLASS HEADER
#include <dali/internal/window-system/tizen/tcore/window-base-tcore-wl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/string-utils.h>
#include <dali/integration-api/trace.h>
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/public-api/common/dali-utility.h>
#include <dali/public-api/events/mouse-button.h>
#include <dali/public-api/object/any.h>

#include <tizen-core-wl/tizen_core_wl_internal.h>
#include <vector>

#if defined(VCONF_ENABLED)
#include <vconf-keys.h>
#include <vconf.h>
#endif

#include <wayland-egl-tizen.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/egl-include.h>
#include <dali/internal/input/common/key-impl.h>
#include <dali/internal/system/common/time-service.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-system.h>

using Dali::Integration::ToDaliString;
using Dali::Integration::ToStdString;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{

static uint32_t windowId = 1;

#if defined(DEBUG_ENABLED)
Debug::Filter* gWindowBaseLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_WINDOW_BASE");
#endif

DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_PERFORMANCE_MARKER, false);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
DALI_INIT_TIME_CHECKER_FILTER_WITH_DEFAULT_THRESHOLD(gTimeCheckerFilter, DALI_TCORE_WL_PERFORMANCE_LOG_THRESHOLD_TIME, 0);
#pragma GCC diagnostic pop

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

void GetBaseData(tizen_core_wl_event_input_base_h ev, tizen_core_wl_event_type_e* type, unsigned int* timestamp, char** dev)
{
  if(type)
  {
    tizen_core_wl_error_e ret = tizen_core_wl_event_input_base_get_type(ev, type);
    DALI_LOG_RELEASE_INFO("GetBaseData::get_type ret=%d type=%d\n", ret, static_cast<int>(*type));
  }
  if(timestamp)
  {
    tizen_core_wl_error_e ret = tizen_core_wl_event_input_base_get_timestamp(ev, timestamp);
    DALI_LOG_RELEASE_INFO("GetBaseData::get_timestamp ret=%d timestamp=%u\n", ret, *timestamp);
  }
  if(dev)
  {
    tizen_core_wl_error_e ret = tizen_core_wl_event_input_base_get_device_identifier(ev, dev);
    DALI_LOG_RELEASE_INFO("GetBaseData::get_device_identifier ret=%d dev=%s\n", ret, (*dev ? *dev : "(null)"));
  }
}

void GetMouseData(tizen_core_wl_event_input_base_h ev, int* x, int* y, unsigned int* buttons, unsigned int* touchId, unsigned int* modifiers, bool move)
{
  int tmpX = 0, tmpY = 0;
  if(move)
  {
    tizen_core_wl_error_e ret = tizen_core_wl_event_mouse_move_get_position(ev, &tmpX, &tmpY);
    DALI_LOG_RELEASE_INFO("GetMouseData::mouse_move_get_position ret=%d x=%d y=%d\n", ret, tmpX, tmpY);
  }
  else
  {
    tizen_core_wl_error_e ret = tizen_core_wl_event_mouse_button_get_position(ev, &tmpX, &tmpY);
    DALI_LOG_RELEASE_INFO("GetMouseData::mouse_button_get_position ret=%d x=%d y=%d\n", ret, tmpX, tmpY);
  }
  if(x)
  {
    *x = tmpX;
    DALI_LOG_RELEASE_INFO("GetMouseData::out_x=%d\n", *x);
  }
  if(y)
  {
    *y = tmpY;
    DALI_LOG_RELEASE_INFO("GetMouseData::out_y=%d\n", *y);
  }
  if(buttons)
  {
    tizen_core_wl_error_e ret = tizen_core_wl_event_mouse_button_get_buttons(ev, buttons);
    DALI_LOG_RELEASE_INFO("GetMouseData::mouse_button_get_buttons ret=%d buttons=%u\n", ret, *buttons);
  }
  if(touchId)
  {
    tizen_core_wl_error_e ret = tizen_core_wl_event_mouse_button_get_touch_id(ev, touchId);
    DALI_LOG_RELEASE_INFO("GetMouseData::mouse_button_get_touch_id ret=%d touchId=%u\n", ret, *touchId);
  }
  if(modifiers)
  {
    tizen_core_wl_error_e ret = tizen_core_wl_event_mouse_button_get_modifiers(ev, modifiers);
    DALI_LOG_RELEASE_INFO("GetMouseData::mouse_button_get_modifiers ret=%d modifiers=%u\n", ret, *modifiers);
  }
}

inline void DisconnectAndDestroyDisplay(tizen_core_wl_display_h& display)
{
  if(!display)
  {
    return;
  }

  tizen_core_wl_display_disconnect(display);
  tizen_core_wl_display_destroy(display);
  display = nullptr;
}

void GetKeyData(tizen_core_wl_event_input_base_h ev, char** keyname, char** compose, char** symbol, unsigned int* keycode, unsigned int* modifiers, unsigned int* flags)
{
  if(keyname)
  {
    tizen_core_wl_error_e ret = tizen_core_wl_event_key_get_keyname(ev, keyname);
    DALI_LOG_RELEASE_INFO("GetKeyData::key_get_keyname ret=%d keyname=%s\n", ret, (*keyname ? *keyname : "(null)"));
  }
  if(compose)
  {
    tizen_core_wl_error_e ret = tizen_core_wl_event_key_get_compose(ev, compose);
    DALI_LOG_RELEASE_INFO("GetKeyData::key_get_compose ret=%d compose=%s\n", ret, (*compose ? *compose : "(null)"));
  }
  if(symbol)
  {
    tizen_core_wl_error_e ret = tizen_core_wl_event_key_get_keysymbol(ev, symbol);
    DALI_LOG_RELEASE_INFO("GetKeyData::key_get_keysymbol ret=%d symbol=%s\n", ret, (*symbol ? *symbol : "(null)"));
  }
  if(keycode)
  {
    tizen_core_wl_error_e ret = tizen_core_wl_event_key_get_keycode(ev, keycode);
    DALI_LOG_RELEASE_INFO("GetKeyData::key_get_keycode ret=%d keycode=%u\n", ret, *keycode);
  }
  if(modifiers)
  {
    tizen_core_wl_error_e ret = tizen_core_wl_event_key_get_modifiers(ev, modifiers);
    DALI_LOG_RELEASE_INFO("GetKeyData::key_get_modifiers ret=%d modifiers=%u\n", ret, *modifiers);
  }
  if(flags)
  {
    tizen_core_wl_error_e ret = tizen_core_wl_event_key_get_event_flags(ev, flags);
    DALI_LOG_RELEASE_INFO("GetKeyData::key_get_event_flags ret=%d flags=%u\n", ret, *flags);
  }
}

void GetDeviceInfo(tizen_core_wl_display_h display, char* identifier, const char** name, tizen_core_wl_device_class_e* tizenClass, tizen_core_wl_device_subclass_e* tizenSubclass)
{
  tizen_core_wl_seat_h seat = nullptr;
  if(!display)
  {
    DALI_LOG_RELEASE_INFO("GetDeviceInfo: mTcoreDisplay is null (no display bind)\n");
    return;
  }
  if(tizen_core_wl_display_get_default_seat(display, &seat) == TIZEN_CORE_WL_ERROR_NONE && seat)
  {
    GList* list = NULL;
    if(tizen_core_wl_seat_get_input_device_list(seat, &list) == TIZEN_CORE_WL_ERROR_NONE && list)
    {
      int idx = 0;
      for(GList* l = list; l; l = l->next)
      {
        tizen_core_wl_input_device_h dev = static_cast<tizen_core_wl_input_device_h>(l->data);
        if(dev)
        {
          const char* devIdentifier = nullptr;
          if(tizen_core_wl_input_device_get_identifier(dev, &devIdentifier) == TIZEN_CORE_WL_ERROR_NONE && devIdentifier)
          {
            if(identifier && strcmp(identifier, devIdentifier) == 0)
            {
              // Found matching device, get its information
              if(name)
              {
                tizen_core_wl_input_device_get_name(dev, name);
              }
              if(tizenClass)
              {
                tizen_core_wl_input_device_get_class(dev, tizenClass);
              }
              if(tizenSubclass)
              {
                tizen_core_wl_input_device_get_subclass(dev, tizenSubclass);
              }
              break;
            }
          }
        }
        idx++;
      }
      // Free the device list
      g_list_free(list);
    }
  }
}
/** Same layout as tizen_core_wl_region_s / */
struct InputRect
{
  int x, y, w, h;
};

/**
 * Get wl_surface from tizen_core_wl_window_h for tizen_policy/tizen_display_policy APIs.
 */
[[maybe_unused]] static struct wl_surface* GetWlSurfaceFromTizenWindow(tizen_core_wl_window_h window)
{
  struct wl_surface* surface = nullptr;
  if(window && tizen_core_wl_window_private_get_wl_surface(window, &surface) == TIZEN_CORE_WL_ERROR_NONE)
  {
    return surface;
  }
  return nullptr;
}

bool RegisterTizenCoreEventListener(tizen_core_event_h                           event,
                                    tizen_core_wl_event_type_e                   eventType,
                                    tizen_core_wl_event_cb                       callback,
                                    void*                                        userData,
                                    std::vector<tizen_core_wl_event_listener_h>& listeners)
{
  if(!event || !callback)
  {
    return false;
  }

  tizen_core_wl_event_listener_h handle = nullptr;
  tizen_core_wl_error_e          err    = tizen_core_wl_event_add_listener(event, eventType, callback, userData, &handle);
  if(err == TIZEN_CORE_WL_ERROR_NONE && handle)
  {
    listeners.push_back(handle);
    return true;
  }
  return false;
}

/**
 * Map Tizen Core WL device class to Dali Device::Class
 */
void GetDeviceClass(tizen_core_wl_device_class_e tizenClass, Device::Class::Type& daliClass)
{
  switch(tizenClass)
  {
    case TIZEN_CORE_WL_DEVICE_CLASS_SEAT:
      daliClass = Device::Class::USER;
      break;
    case TIZEN_CORE_WL_DEVICE_CLASS_KEYBOARD:
      daliClass = Device::Class::KEYBOARD;
      break;
    case TIZEN_CORE_WL_DEVICE_CLASS_MOUSE:
      daliClass = Device::Class::MOUSE;
      break;
    case TIZEN_CORE_WL_DEVICE_CLASS_TOUCH:
      daliClass = Device::Class::TOUCH;
      break;
    default:
      daliClass = Device::Class::NONE;
      break;
  }
}

/**
 * Map Tizen Core WL device subclass to Dali Device::Subclass
 */
void GetDeviceSubclass(tizen_core_wl_device_subclass_e tizenSubclass, Device::Subclass::Type& daliSubclass)
{
  switch(tizenSubclass)
  {
    case TIZEN_CORE_WL_DEVICE_SUBCLASS_REMOCON:
      daliSubclass = Device::Subclass::REMOCON;
      break;
    case TIZEN_CORE_WL_DEVICE_SUBCLASS_VIRTUAL_KEYBOARD:
      daliSubclass = Device::Subclass::VIRTUAL_KEYBOARD;
      break;
    case TIZEN_CORE_WL_DEVICE_SUBCLASS_VIRTUAL_REMOCON:
      daliSubclass = Device::Subclass::VIRTUAL_REMOCON;
      break;
    case TIZEN_CORE_WL_DEVICE_SUBCLASS_VIRTUAL_MOUSE:
      daliSubclass = Device::Subclass::VIRTUAL_MOUSE;
      break;
    default:
      daliSubclass = Device::Subclass::NONE;
      break;
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
static void TcoreWlEventWindowIconifyStateChanged(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnIconifyStateChanged(user_data, event_type, event);
  }
}

/// Called when the window gains focus
static void TcoreWlEventWindowFocusIn(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnFocusIn(user_data, event_type, event);
  }
}

/// Called when the window loses focus
static void TcoreWlEventWindowFocusOut(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnFocusOut(user_data, event_type, event);
  }
}

/**
 * Called when rotate event is recevied.
 */
static void TcoreWlEventRotate(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::TcoreWlEventRotate\n");
    windowBase->OnRotation(user_data, event_type, event);
  }
}

/**
 * Called when configure event is recevied.
 */
static void TcoreWlEventConfigure(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnConfiguration(user_data, event_type, event);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Touch Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a touch down is received.
 */
static void TcoreWlEventMouseButtonDown(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnMouseButtonDown(user_data, event_type, event);
  }
}

/**
 * Called when a touch up is received.
 */
static void TcoreWlEventMouseButtonUp(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnMouseButtonUp(user_data, event_type, event);
  }
}

/**
 * Called when a touch motion is received.
 */
static void TcoreWlEventMouseButtonMove(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnMouseButtonMove(user_data, event_type, event);
  }
}

/**
 * Called when a touch motion is received.
 */
static void TcoreWlEventMouseFrame(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnMouseFrame(user_data, event_type, event);
  }
}

/**
 * Called when a touch motion is received.
 */
static void TcoreWlEventMouseButtonRelativeMove(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnMouseButtonRelativeMove(user_data, event_type, event);
  }
}

/**
 * Called when a touch is canceled.
 */
static void TcoreWlEventMouseButtonCancel(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnMouseButtonCancel(user_data, event_type, event);
  }
}

/**
 * Called when pointer constraints event is recevied.
 */
static void TcoreWlEventPointerConstraints(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnPointerConstraints(user_data, event_type, event);
  }
}

/**
 * Called when a mouse wheel is received.
 */
static void TcoreWlEventMouseWheel(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnMouseWheel(user_data, event_type, event);
  }
}

/**
 * Called when a mouse in is received.
 */
static void TcoreWlEventMouseIn(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnMouseInOut(user_data, event_type, event);
  }
}

/**
 * Called when a mouse out is received.
 */
static void TcoreWlEventMouseOut(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnMouseButtonCancel(user_data, event_type, event);
    windowBase->OnMouseInOut(user_data, event_type, event);
  }
}

/**
 * Called when a detent rotation event is recevied.
 */
static void TcoreWlEventDetentRotation(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnDetentRotation(user_data, event_type, event);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Key Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when a key down is received.
 */
static void TcoreWlEventKeyDown(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnKeyDown(user_data, event_type, event);
  }
}

/**
 * Called when a key up is received.
 */
static void TcoreWlEventKeyUp(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnKeyUp(user_data, event_type, event);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Selection Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Called when the source window notifies us the content in clipboard is selected.
 */
static void TcoreWlEventDataSend(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnDataSend(user_data, event_type, event);
  }
}

/**
 * Called when the source window sends us about the selected content.
 * For example, when item is selected in the clipboard.
 */
static void TcoreWlEventDataReceive(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnDataReceive(user_data, event_type, event);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Effect Start/End Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

static void TcoreWlEventWindowEffectStart(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnTransitionEffectEvent(user_data, static_cast<int>(event_type), event);
  }
}

static void TcoreWlEventWindowEffectEnd(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnTransitionEffectEvent(user_data, static_cast<int>(event_type), event);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Keymap Changed Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

static void TcoreWlEventSeatKeymapChanged(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->KeymapChanged(user_data, event_type, event);
  }
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
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(data);
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
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(data);
  if(windowBase)
  {
    windowBase->OnFontSizeChanged();
  }
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
// Window Redraw Request Event Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

// [TCORE_WL_MIGRATION] No direct Tizen Core WL equivalent for WINDOW_REDRAW_REQUEST
// static Eina_Bool EcoreEventWindowRedrawRequest(void* data, int type, void* event)
// {
//   WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(data);
//   DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseTcoreWl::EcoreEventWindowRedrawRequest, window[ %d ]\n", static_cast<Ecore_Wl2_Event_Window_Redraw_Request*>(event)->win);
//   if(windowBase)
//   {
//     windowBase->OnEcoreEventWindowRedrawRequest();
//   }
//
//   return ECORE_CALLBACK_RENEW;
// }

/////////////////////////////////////////////////////////////////////////////////////////////////
// Window Auxiliary Message Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

static void TcoreWlEventWindowAuxiliaryMessage(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnWindowAuxiliaryMessage(user_data, static_cast<int>(event_type), event);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Window is Moved/Resized By Server Callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////

static void TcoreWlEventWindowInteractiveMoveDone(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnMoveCompleted(user_data, static_cast<int>(event_type), event);
  }
}

static void TcoreWlEventWindowInteractiveResizeDone(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnResizeCompleted(user_data, static_cast<int>(event_type), event);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Conformant Change Callback
/////////////////////////////////////////////////////////////////////////////////////////////////
static void TcoreWlEventConformantChange(void* event, tizen_core_wl_event_type_e event_type, void* user_data)
{
  WindowBaseTcoreWl* windowBase = static_cast<WindowBaseTcoreWl*>(user_data);
  if(windowBase)
  {
    windowBase->OnConformantChange(user_data, event_type, event);
  }
}

} // unnamed namespace

WindowBaseTcoreWl::WindowBaseTcoreWl(Dali::PositionSize positionSize, Any surface, bool isTransparent)
: mTcoreEventListeners(),
  mTcoreDisplay(nullptr),
  mTcoreWindow(nullptr),
  mDisplayRegion(nullptr),
  mTcoreEvent(nullptr),
  mScreen(nullptr),
  mWlSurface(nullptr),
  mWlInputPanel(nullptr),
  mWlOutput(nullptr),
  mWlInputPanelSurface(nullptr),
  mEglWindow(nullptr),
  mDisplay(nullptr),
  mSupportedAuxiliaryHints(),
  mWindowPositionSize(positionSize),
  mAuxiliaryHints(),
  mType(WindowType::NORMAL),
  mWindowRotationAngle(0),
  mScreenRotationAngle(0),
  mSupportedPreProtation(0),
  mWinId(0),
  mLastSubmittedMoveResizeSerial(0),
  mMoveResizeSerial(0),
  mVisible(true),
  mOwnSurface(false),
  mIsFrontBufferRendering(false),
  mIsIMEWindowInitialized(false),
  mIsBottom(false),
  mIsFloating(false),
  mIsAlwaysTop(false),
  mPendingRestoreResizeOnUnmaximize(false),
  mSetRestoreWindowUnmaximize(false),
  mWindowStateStatus(WindowStateStatus::NORMAL),
  mRestoreWindowWidth(positionSize.width),
  mRestoreWindowHeight(positionSize.height)
{
  Initialize(positionSize, surface, isTransparent);
}

WindowBaseTcoreWl::~WindowBaseTcoreWl()
{
  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::~WindowBaseTcoreWl: start cleanup this=%p mOwnSurface=%d\n",
                        static_cast<void*>(this),
                        mOwnSurface ? 1 : 0);

#if defined(VCONF_ENABLED)
  vconf_ignore_key_changed(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, VconfNotifyFontSizeChanged);
  vconf_ignore_key_changed(DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_NAME, VconfNotifyFontNameChanged);
#endif

  if(mTcoreEvent)
  {
    for(auto& listener : mTcoreEventListeners)
    {
      tizen_core_wl_event_remove_listener(mTcoreEvent, listener);
    }
  }
  mTcoreEventListeners.clear();

  if(mWlInputPanel)
  {
    wl_proxy_destroy((struct wl_proxy*)mWlInputPanel);
    mWlInputPanel = nullptr;
  }

  if(mWlOutput)
  {
    wl_output_destroy(mWlOutput);
    mWlOutput = nullptr;
  }

  mSupportedAuxiliaryHints.clear();
  mAuxiliaryHints.clear();

  if(mEglWindow != nullptr)
  {
    wl_egl_window_destroy(mEglWindow);
    mEglWindow = NULL;
  }

  if(mOwnSurface)
  {
    if(mTcoreWindow)
    {
      DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::~WindowBaseTcoreWl: tizen_core_wl_window_destroy %p\n",
                            static_cast<void*>(mTcoreWindow));
      tizen_core_wl_window_destroy(mTcoreWindow);
      mTcoreWindow = nullptr;
    }
  }

  if(mTcoreDisplay && mDisplayRegion)
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::~WindowBaseTcoreWl: tizen_core_wl_display_destroy_region %p\n",
                          static_cast<void*>(mDisplayRegion));
    tizen_core_wl_display_destroy_region(mTcoreDisplay, mDisplayRegion);
    mDisplayRegion = nullptr;
  }

  if(mTcoreDisplay)
  {
    if(mOwnSurface)
    {
      // We created and connected the display in CreateInternalWindow();
      // disconnect AND destroy so the handle from tizen_core_wl_display_create()
      // is freed (matches DisconnectAndDestroyDisplay() helper above).
      DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::~WindowBaseTcoreWl: DisconnectAndDestroyDisplay %p\n",
                            static_cast<void*>(mTcoreDisplay));
      DisconnectAndDestroyDisplay(mTcoreDisplay);
    }
    else
    {
      // External surface case: the display was obtained from
      // tizen_core_wl_window_get_display(); ownership belongs to the
      // caller, so just drop our reference.
      mTcoreDisplay = nullptr;
    }
  }

  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::~WindowBaseTcoreWl: cleanup done\n");
}

void WindowBaseTcoreWl::Initialize(PositionSize positionSize, Any surface, bool isTransparent)
{
  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [0] start pos=[%d,%d %dx%d] transparent=%d surface_empty=%d\n",
                        positionSize.x,
                        positionSize.y,
                        positionSize.width,
                        positionSize.height,
                        isTransparent ? 1 : 0,
                        surface.Empty() ? 1 : 0);

  if(surface.Empty() == false)
  {
    // Prefer Tizen Core WL window handle; fallback to tizen_core_wl_window_h* for backward compatibility
    if(surface.IsType<tizen_core_wl_window_h>())
    {
      mTcoreWindow = AnyCast<tizen_core_wl_window_h>(surface);
      if(mTcoreWindow && tizen_core_wl_window_get_display(mTcoreWindow, &mTcoreDisplay) != TIZEN_CORE_WL_ERROR_NONE)
      {
        mTcoreDisplay = nullptr;
      }
      DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [1] external surface path mTcoreWindow=%p mTcoreDisplay=%p\n",
                            static_cast<void*>(mTcoreWindow),
                            static_cast<void*>(mTcoreDisplay));
    }
    if(!surface.Empty() && !surface.IsType<tizen_core_wl_window_h>())
    {
      DALI_ASSERT_ALWAYS(false && "Surface type is invalid: expected tizen_core_wl_window_h (tizen_core_wl_window_h* path commented out)");
    }
  }
  else
  {
    // we own the surface about to created
    mOwnSurface = true;
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [1] own surface path CreateInternalWindow\n");
    CreateInternalWindow(positionSize);
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [1] CreateInternalWindow done mTcoreWindow=%p\n", static_cast<void*>(mTcoreWindow));
  }

  if(mTcoreWindow)
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [2] window valid, get wl_surface winId next\n");
    struct wl_surface* surface = nullptr;
    if(tizen_core_wl_window_private_get_wl_surface(mTcoreWindow, &surface) == TIZEN_CORE_WL_ERROR_NONE)
    {
      mWlSurface = surface;
      DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [2] wl_surface=%p\n", static_cast<void*>(mWlSurface));
    }
    else
    {
      DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [2] tizen_core_wl_window_private_get_wl_surface failed\n");
    }
    mDisplay = nullptr;

    mWinId = windowId++;

    if(mTcoreDisplay)
    {
      DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [3] mTcoreDisplay set, bind Wayland display + event\n");
      if(tizen_core_wl_display_private_get_wl_display(mTcoreDisplay, &mDisplay) != TIZEN_CORE_WL_ERROR_NONE)
      {
        DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [3] tizen_core_wl_display_private_get_wl_display failed\n");
      }

      // Fetch the tizen-core event handle associated with this display.
      tizen_core_event_h event = nullptr;
      if(tizen_core_wl_display_get_event(mTcoreDisplay, &event) == TIZEN_CORE_WL_ERROR_NONE && event)
      {
        mTcoreEvent = event;
        DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [3] mTcoreEvent=%p mDisplay(wl)=%p\n",
                              static_cast<void*>(mTcoreEvent),
                              static_cast<void*>(mDisplay));
      }
      else
      {
        DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [3] tizen_core_wl_display_get_event failed or null\n");
      }

      SetTransparency(isTransparent);
      DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [4] SetTransparency(%d) done\n", isTransparent ? 1 : 0);

      tizen_core_wl_window_set_conformant(mTcoreWindow, true);

      // Seat/keymap related listeners
      if(mTcoreEvent)
      {
        tizen_core_wl_seat_h seat = nullptr;
        if(tizen_core_wl_display_get_default_seat(mTcoreDisplay, &seat) == TIZEN_CORE_WL_ERROR_NONE && seat)
        {
          DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [5] seat ok, set cursor theme/name\n");
          tizen_core_wl_seat_set_cursor_theme(seat, "default");
          tizen_core_wl_seat_set_cursor_name(seat, "left_ptr");
          RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_SEAT_KEYMAP_CHANGED, TcoreWlEventSeatKeymapChanged, this, mTcoreEventListeners);
        }
        else
        {
          DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [5] seat failed, set cursor theme/name\n");
        }

        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_WINDOW_ICONIFY_STATE_CHANGE, TcoreWlEventWindowIconifyStateChanged, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_FOCUS_IN, TcoreWlEventWindowFocusIn, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_FOCUS_OUT, TcoreWlEventWindowFocusOut, this, mTcoreEventListeners);

        // [TCORE_WL_MIGRATION] No direct Tizen Core WL equivalent for OUTPUT_TRANSFORM
        // [TCORE_WL_MIGRATION] No direct Tizen Core WL equivalent for IGNORE_OUTPUT_TRANSFORM

        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_WINDOW_ROTATION, TcoreWlEventRotate, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_WINDOW_CONFIGURE, TcoreWlEventConfigure, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_WINDOW_INTERACTIVE_MOVE_DONE, TcoreWlEventWindowInteractiveMoveDone, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_WINDOW_INTERACTIVE_RESIZE_DONE, TcoreWlEventWindowInteractiveResizeDone, this, mTcoreEventListeners);

        // Touch events
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_MOUSE_BUTTON_DOWN, TcoreWlEventMouseButtonDown, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_MOUSE_BUTTON_UP, TcoreWlEventMouseButtonUp, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_MOUSE_MOVE, TcoreWlEventMouseButtonMove, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_MOUSE_BUTTON_CANCEL, TcoreWlEventMouseButtonCancel, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_MOUSE_RELATIVE_MOVE, TcoreWlEventMouseButtonRelativeMove, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_POINTER_LOCKED, TcoreWlEventPointerConstraints, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_MOUSE_FRAME, TcoreWlEventMouseFrame, this, mTcoreEventListeners);

        // Mouse wheel
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_MOUSE_WHEEL, TcoreWlEventMouseWheel, this, mTcoreEventListeners);

        // Mouse IO
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_MOUSE_IN, TcoreWlEventMouseIn, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_MOUSE_OUT, TcoreWlEventMouseOut, this, mTcoreEventListeners);

        // Detent
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_DETENT_ROTATE, TcoreWlEventDetentRotation, this, mTcoreEventListeners);

        // Key events
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_KEY_DOWN, TcoreWlEventKeyDown, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_KEY_UP, TcoreWlEventKeyUp, this, mTcoreEventListeners);

        // Selection/Clipboard
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_DATA_SOURCE_SEND, TcoreWlEventDataSend, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_DATA_READY, TcoreWlEventDataReceive, this, mTcoreEventListeners);

        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_WINDOW_EFFECT_START, TcoreWlEventWindowEffectStart, this, mTcoreEventListeners);
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_WINDOW_EFFECT_END, TcoreWlEventWindowEffectEnd, this, mTcoreEventListeners);

        // [TCORE_WL_MIGRATION] No direct Tizen Core WL equivalent for WINDOW_REDRAW_REQUEST
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_WINDOW_AUX_MESSAGE, TcoreWlEventWindowAuxiliaryMessage, this, mTcoreEventListeners);

        // Conformant change
        RegisterTizenCoreEventListener(mTcoreEvent, TIZEN_CORE_WL_EVENT_WINDOW_CONFORMANT_CHANGE, TcoreWlEventConformantChange, this, mTcoreEventListeners);

        DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [5] event listeners registered count=%zu\n", mTcoreEventListeners.size());

      } // mTcoreEvent

      tizen_core_wl_screen_h screen = nullptr;
      if(tizen_core_wl_display_get_preferred_screen(mTcoreDisplay, &screen) == TIZEN_CORE_WL_ERROR_NONE)
      {
        if(screen)
        {
          mScreen = screen;
          DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [7] default screen=%p\n", static_cast<void*>(mScreen));
        }
        else
        {
          DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [7] Get Default Screen() but screen is null", DALI_LOG_FORMAT_PREFIX_ARGS);
        }
      }
      else
      {
        DALI_LOG_RELEASE_INFO("Get Default Screen() but display is null", DALI_LOG_FORMAT_PREFIX_ARGS);
      }

    } // mTcoreDisplay
    else
    {
      DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [3] skipped mTcoreDisplay is null (no display bind)\n");
    }
  } // mTcoreWindow
  else
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [2] skipped mTcoreWindow is null\n");
  }
#if defined(VCONF_ENABLED)
  // Register Vconf notify - font name and size
  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [8] VCONF font name/size notify\n");
  vconf_notify_key_changed_for_ui_thread(DALI_VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_NAME, VconfNotifyFontNameChanged, this);
  vconf_notify_key_changed_for_ui_thread(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, VconfNotifyFontSizeChanged, this);
#endif

  // get auxiliary hint
  char**       auxHintList      = NULL;
  unsigned int auxHintListCount = 0;
  if(tizen_core_wl_window_get_supported_aux_hints(mTcoreWindow, &auxHintList, &auxHintListCount) == TIZEN_CORE_WL_ERROR_NONE)
  {
    if(auxHintList && auxHintListCount > 0)
    {
      for(unsigned int i = 0; i < auxHintListCount; i++)
      {
        mSupportedAuxiliaryHints.push_back(auxHintList[i]);
        DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: supported auxiliary hint: %s\n", auxHintList[i]);
      }
    }
    else
    {
      DALI_LOG_ERROR("WindowBaseTcoreWl::Initialize: [8] no supported auxiliary hints\n");
    }
  }
  else
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::Initialize: [8] failed to get supported auxiliary hints\n");
  }

  if(tizen_core_wl_display_get_preferred_screen(mTcoreDisplay, &mScreen) == TIZEN_CORE_WL_ERROR_NONE)
  {
    if(mScreen)
    {
      DALI_LOG_RELEASE_INFO("Success to get Default Screen()\n");
    }
    else
    {
      DALI_LOG_RELEASE_INFO("Get Default Screen() but screen is null");
    }
  }
  else
  {
    DALI_LOG_RELEASE_INFO("Failed to get Default Screen()");
  }

  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::Initialize: [done] window [%p], id [%d], positionSize [%d, %d, %d, %d]\n",
                        static_cast<void*>(mTcoreWindow),
                        mWinId,
                        positionSize.x,
                        positionSize.y,
                        positionSize.width,
                        positionSize.height);
}

bool WindowBaseTcoreWl::OnIconifyStateChanged(void* data, int type, void* event)
{
  tizen_core_wl_event_window_base_h baseEvent   = static_cast<tizen_core_wl_event_window_base_h>(event);
  tizen_core_wl_window_h            eventWindow = nullptr;
  if(tizen_core_wl_event_window_base_get_window(baseEvent, &eventWindow) != TIZEN_CORE_WL_ERROR_NONE)
  {
    return true;
  }

  if(eventWindow == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    bool iconified = false;
    tizen_core_wl_window_get_iconify_state(eventWindow, &iconified);

    if(iconified)
    {
      DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnIconifyStateChanged, the window (%p) is iconified\n", mTcoreWindow);
      mIconifyChangedSignal.Emit(true);
    }
    else
    {
      DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnIconifyStateChanged, the window (%p) is not iconified\n", mTcoreWindow);
      mIconifyChangedSignal.Emit(false);
    }
  }
  return true;
}

void WindowBaseTcoreWl::OnFocusIn(void* data, int type, void* event)
{
  tizen_core_wl_event_input_base_h inputEvent =
    static_cast<tizen_core_wl_event_input_base_h>(event);

  tizen_core_wl_window_h eventWindow = nullptr;
  tizen_core_wl_event_input_base_get_window(inputEvent, &eventWindow);

  if(eventWindow == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnFocusIn, Window (%p)\n", mTcoreWindow);
    mFocusChangedSignal.Emit(true);
  }
}

void WindowBaseTcoreWl::OnFocusOut(void* data, int type, void* event)
{
  tizen_core_wl_event_input_base_h inputEvent =
    static_cast<tizen_core_wl_event_input_base_h>(event);

  tizen_core_wl_window_h eventWindow = nullptr;
  tizen_core_wl_event_input_base_get_window(inputEvent, &eventWindow);

  if(eventWindow == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnFocusOut, Window (%p)\n", mTcoreWindow);
    mFocusChangedSignal.Emit(false);
  }
}

bool WindowBaseTcoreWl::OnOutputTransformChanged(void* data, int type, void* event)
{
  // It is not needed
  return true;
}

bool WindowBaseTcoreWl::OnOutputTransform(void* data, int type, void* event)
{
  // It is not needed
  return true;
}

bool WindowBaseTcoreWl::OnIgnoreOutputTransform(void* data, int type, void* event)
{
  // It is not needed
  return true;
}

void WindowBaseTcoreWl::OnRotation(void* data, int type, void* event)
{
  tizen_core_wl_event_window_base_h baseEvent = static_cast<tizen_core_wl_event_window_base_h>(event);
  if(!baseEvent)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::OnRotation: invalid base event\n");
    return;
  }

  tizen_core_wl_event_window_rotation_h rotationEv = nullptr;
  if(tizen_core_wl_event_window_base_to_window_rotation(baseEvent, &rotationEv) != TIZEN_CORE_WL_ERROR_NONE || !rotationEv)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::OnRotation: failed to convert to window rotation event\n");
    return;
  }

  tizen_core_wl_window_h eventWindow = nullptr;
  if(tizen_core_wl_event_window_base_get_window(baseEvent, &eventWindow) != TIZEN_CORE_WL_ERROR_NONE || !eventWindow)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::OnRotation: failed to get event window\n");
    return;
  }

  if(eventWindow == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    tizen_core_wl_window_angle_e wlAngle    = TIZEN_CORE_WL_WINDOW_ANGLE_0;
    int                          w          = 0;
    int                          h          = 0;
    bool                         needResize = false;

    if(tizen_core_wl_event_window_rotation_get_angle(rotationEv, &wlAngle) != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_ERROR("WindowBaseTcoreWl::OnRotation: failed to get rotation angle\n");
      return;
    }
    if(tizen_core_wl_event_window_rotation_get_size(rotationEv, &w, &h) != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_ERROR("WindowBaseTcoreWl::OnRotation: failed to get rotation size\n");
      return;
    }
    if(tizen_core_wl_event_window_rotation_get_resize(rotationEv, &needResize) != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_ERROR("WindowBaseTcoreWl::OnRotation: failed to get resize flag\n");
      return;
    }

    const int angle = static_cast<int>(wlAngle);

    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnRotation, Window (%p), angle: %d, width: %d, height: %d, needResize: %d\n", mTcoreWindow, angle, w, h, needResize ? 1 : 0);

    RotationEvent rotationEvent;
    rotationEvent.angle     = angle;
    rotationEvent.winResize = needResize ? 1 : 0;

    if(w == 0 || h == 0)
    {
      w = mWindowPositionSize.width;
      h = mWindowPositionSize.height;
    }

    mWindowRotationAngle       = angle;
    mWindowPositionSize.width  = w;
    mWindowPositionSize.height = h;

    if(tizen_core_wl_window_set_geometry(mTcoreWindow, mWindowPositionSize.x, mWindowPositionSize.y, mWindowPositionSize.width, mWindowPositionSize.height) != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_ERROR("WindowBaseTcoreWl::OnRotation: failed to set window geometry\n");
      return;
    }

    PositionSize newPositionSize = RecalculatePositionSizeToCurrentOrientation(mWindowPositionSize);
    rotationEvent.x              = newPositionSize.x;
    rotationEvent.y              = newPositionSize.y;
    rotationEvent.width          = newPositionSize.width;
    rotationEvent.height         = newPositionSize.height;

    mRotationSignal.Emit(rotationEvent);
    tizen_core_wl_window_commit(mTcoreWindow);
  }
}

void WindowBaseTcoreWl::OnWindowRotationChange(void* data, int type, void* event)
{
  // it is not needed yer.
}

void WindowBaseTcoreWl::OnConfiguration(void* data, int type, void* event)
{
  tizen_core_wl_event_window_base_h baseEvent   = static_cast<tizen_core_wl_event_window_base_h>(event);
  tizen_core_wl_window_h            eventWindow = nullptr;
  if(tizen_core_wl_event_window_base_get_window(baseEvent, &eventWindow) != TIZEN_CORE_WL_ERROR_NONE)
  {
    return;
  }

  if(eventWindow == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    tizen_core_wl_event_window_configure_h configureEvent = nullptr;
    if(tizen_core_wl_event_window_base_to_configure(baseEvent, &configureEvent) != TIZEN_CORE_WL_ERROR_NONE || !configureEvent)
    {
      DALI_LOG_ERROR("failed to convert base event to configure event\n");
      return;
    }

    DALI_LOG_RELEASE_INFO("configure event Info --------------------------------------------------->\n");

    int          tempX = 0, tempY = 0;
    unsigned int tempWidth = 0u, tempHeight = 0u;
    if(tizen_core_wl_event_window_configure_get_geometry(configureEvent, &tempX, &tempY, &tempWidth, &tempHeight) != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_ERROR("failed to get configure geometry\n");
      return;
    }
    DALI_LOG_RELEASE_INFO("tizen_core_wl_event_window_configure_get_geometry x[%d] y[%d] w[%d] h[%d]", tempX, tempY, tempWidth, tempHeight);

    tizen_core_wl_window_state_e state = TIZEN_CORE_WL_WINDOW_STATE_NONE;
    if(tizen_core_wl_event_window_configure_get_state(configureEvent, &state) == TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_RELEASE_INFO("Window (%p), configure state [%d]\n", mTcoreWindow, state);

      if(state == TIZEN_CORE_WL_WINDOW_STATE_FULLSCREEN)
      {
        mWindowStateStatus = WindowStateStatus::FULLSCREEN;
      }
      else if(state == TIZEN_CORE_WL_WINDOW_STATE_MAXIMIZED)
      {
        mWindowStateStatus = WindowStateStatus::MAXIMIZED;
      }
      else
      {
        mWindowStateStatus = WindowStateStatus::NORMAL;
      }
    }
    else
    {
      DALI_LOG_ERROR("failed to get configure state\n");
    }

    unsigned int edges = 0;
    if(tizen_core_wl_event_window_configure_get_edges(configureEvent, &edges) == TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_RELEASE_INFO("Window (%p), edges [%d]\n", mTcoreWindow, edges);
    }
    else
    {
      DALI_LOG_ERROR("failed to get edges\n");
    }
    DALI_LOG_RELEASE_INFO("------------------------------------------------------------------------>\n");

    DALI_LOG_RELEASE_INFO("current window size x[%d] y[%d] w[%d] h[%d]", mWindowPositionSize.x, mWindowPositionSize.y, mWindowPositionSize.width, mWindowPositionSize.height);
    int  newWidth    = mWindowPositionSize.width;
    int  newHeight   = mWindowPositionSize.height;
    bool windowMoved = false, windowResized = false;

    if(tempX != mWindowPositionSize.x || tempY != mWindowPositionSize.y)
    {
      windowMoved = true;
    }

    if(tempWidth != 0u && tempHeight != 0u && (static_cast<int>(tempWidth) != mWindowPositionSize.width || static_cast<int>(tempHeight) != mWindowPositionSize.height))
    {
      windowResized = true;
      newWidth      = static_cast<int>(tempWidth);
      newHeight     = static_cast<int>(tempHeight);
    }

    if(windowMoved || windowResized)
    {
      mWindowPositionSize.x      = tempX;
      mWindowPositionSize.y      = tempY;
      mWindowPositionSize.width  = newWidth;
      mWindowPositionSize.height = newHeight;

      if(mPendingRestoreResizeOnUnmaximize && mSetRestoreWindowUnmaximize && mWindowStateStatus == WindowStateStatus::NORMAL)
      {
        mPendingRestoreResizeOnUnmaximize = false;

        mWindowPositionSize.width  = Max(1, mRestoreWindowWidth);
        mWindowPositionSize.height = Max(1, mRestoreWindowHeight);

        DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnConfiguration, apply restore geometry x[%d] y[%d] w[%d] h[%d]\n",
                              mWindowPositionSize.x,
                              mWindowPositionSize.y,
                              mWindowPositionSize.width,
                              mWindowPositionSize.height);
      }

      int tx = 0, ty = 0, tw = 0, th = 0;
      tizen_core_wl_window_get_geometry(mTcoreWindow, &tx, &ty, &tw, &th);
      DALI_LOG_RELEASE_INFO("Window (%p), tizen_core_wl_window_get_geometry x[%d] y[%d] w[%d] h[%d]\n", mTcoreWindow, tx, ty, tw, th);
      if(tx != mWindowPositionSize.x || ty != mWindowPositionSize.y || tw != mWindowPositionSize.width || th != mWindowPositionSize.height)
      {
        DALI_LOG_RELEASE_INFO("tizen_core_wl_window_set_geometry x[%d] y[%d] w[%d] h[%d]\n", mWindowPositionSize.x, mWindowPositionSize.y, mWindowPositionSize.width, mWindowPositionSize.height);
        tizen_core_wl_window_set_geometry(mTcoreWindow, mWindowPositionSize.x, mWindowPositionSize.y, mWindowPositionSize.width, mWindowPositionSize.height);
      }

      Dali::PositionSize newPositionSize = RecalculatePositionSizeToCurrentOrientation(mWindowPositionSize);
      mUpdatePositionSizeSignal.Emit(newPositionSize);
    }

    tizen_core_wl_window_commit(mTcoreWindow);
  }
}

void WindowBaseTcoreWl::OnMouseButtonDown(void* data, int type, void* event)
{
  tizen_core_wl_event_input_base_h ev = static_cast<tizen_core_wl_event_input_base_h>(event);

  tizen_core_wl_window_h window    = NULL;
  tizen_core_wl_error_e  retWindow = tizen_core_wl_event_input_base_get_window(ev, &window);
  DALI_LOG_RELEASE_INFO("OnMouseButtonDown::get_window ret=%d eventType=%d window=%p\n", retWindow, type, static_cast<void*>(window));

  if(window == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_DOWN");

    int                             x = 0, y = 0;
    unsigned int                    buttons = 0, touchId = 0, timestamp = 0;
    char*                           dev  = nullptr;
    const char*                     name = nullptr;
    tizen_core_wl_device_class_e    tizenClass;
    tizen_core_wl_device_subclass_e tizenSubclass;
    Device::Class::Type             deviceClass;
    Device::Subclass::Type          deviceSubclass;

    GetBaseData(ev, nullptr, &timestamp, &dev);
    GetDeviceInfo(mTcoreDisplay, dev, &name, &tizenClass, &tizenSubclass);
    GetMouseData(ev, &x, &y, &buttons, &touchId, nullptr, false);
    GetDeviceClass(tizenClass, deviceClass);
    GetDeviceSubclass(tizenSubclass, deviceSubclass);
    Dali::String deviceName(name);

    PointState::Type state(PointState::DOWN);

    if(deviceClass != Device::Class::Type::MOUSE)
    {
      // Check if the buttons field is set and ensure it's the primary touch button.
      // If this event was triggered by buttons other than the primary button (used for touch), then
      // just send an interrupted event to Core.
      if(buttons && (buttons != PRIMARY_TOUCH_BUTTON_ID))
      {
        state = PointState::INTERRUPTED;
      }
    }

    Integration::Point point;
    point.SetDeviceId(touchId);
    point.SetState(state);
    point.SetScreenPosition(Vector2(x, y));
    point.SetRadius(1, Vector2(1, 1));
    point.SetPressure(1.0);
    point.SetAngle(Degree(0.0));
    point.SetDeviceClass(deviceClass);
    point.SetDeviceSubclass(deviceSubclass);
    point.SetMouseButton(static_cast<MouseButton::Type>(buttons));
    point.SetDeviceName(deviceName);

    mTouchEventSignal.Emit(point, timestamp);

    if(dev)
    {
      free(dev);
    }
  }
  else
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnMouseButtonDown, Window (%p) and input (%p) comes\n", mTcoreWindow, window);
  }
}

void WindowBaseTcoreWl::OnMouseButtonUp(void* data, int type, void* event)
{
  tizen_core_wl_event_input_base_h ev = static_cast<tizen_core_wl_event_input_base_h>(event);

  tizen_core_wl_window_h window    = NULL;
  tizen_core_wl_error_e  retWindow = tizen_core_wl_event_input_base_get_window(ev, &window);
  DALI_LOG_RELEASE_INFO("OnMouseButtonUp::get_window ret=%d eventType=%d window=%p\n", retWindow, type, static_cast<void*>(window));

  if(window == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_UP");

    int                             x = 0, y = 0;
    unsigned int                    buttons = 0, touchId = 0, timestamp = 0;
    char*                           dev  = nullptr;
    const char*                     name = nullptr;
    tizen_core_wl_device_class_e    tizenClass;
    tizen_core_wl_device_subclass_e tizenSubclass;
    Device::Class::Type             deviceClass;
    Device::Subclass::Type          deviceSubclass;

    GetBaseData(ev, nullptr, &timestamp, &dev);
    GetDeviceInfo(mTcoreDisplay, dev, &name, &tizenClass, &tizenSubclass);
    GetMouseData(ev, &x, &y, &buttons, &touchId, nullptr, false);
    GetDeviceClass(tizenClass, deviceClass);
    GetDeviceSubclass(tizenSubclass, deviceSubclass);
    Dali::String deviceName(name);

    Integration::Point point;
    point.SetDeviceId(touchId);
    point.SetState(PointState::UP);
    point.SetScreenPosition(Vector2(x, y));
    point.SetRadius(1, Vector2(1, 1));
    point.SetPressure(1.0);
    point.SetAngle(Degree(0.0));
    point.SetDeviceClass(deviceClass);
    point.SetDeviceSubclass(deviceSubclass);
    point.SetMouseButton(static_cast<MouseButton::Type>(buttons));
    point.SetDeviceName(deviceName);

    mTouchEventSignal.Emit(point, timestamp);

    if(dev)
    {
      free(dev);
    }
  }
  else
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnMouseButtonUp, Window (%p) and input (%p) comes\n", mTcoreWindow, window);
  }
}

void WindowBaseTcoreWl::OnMouseButtonMove(void* data, int type, void* event)
{
  tizen_core_wl_event_input_base_h ev = static_cast<tizen_core_wl_event_input_base_h>(event);

  tizen_core_wl_window_h window    = NULL;
  tizen_core_wl_error_e  retWindow = tizen_core_wl_event_input_base_get_window(ev, &window);
  DALI_LOG_RELEASE_INFO("OnMouseButtonMove::get_window ret=%d eventType=%d window=%p\n", retWindow, type, static_cast<void*>(window));

  if(window == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_MOVE");

    int                             x = 0, y = 0;
    unsigned int                    touchId = 0, timestamp = 0;
    char*                           dev  = nullptr;
    const char*                     name = nullptr;
    tizen_core_wl_device_class_e    tizenClass;
    tizen_core_wl_device_subclass_e tizenSubclass;
    Device::Class::Type             deviceClass;
    Device::Subclass::Type          deviceSubclass;

    GetBaseData(ev, nullptr, &timestamp, &dev);
    GetDeviceInfo(mTcoreDisplay, dev, &name, &tizenClass, &tizenSubclass);
    GetMouseData(ev, &x, &y, nullptr, &touchId, nullptr, true);
    GetDeviceClass(tizenClass, deviceClass);
    GetDeviceSubclass(tizenSubclass, deviceSubclass);
    Dali::String deviceName(name);

    Integration::Point point;
    point.SetDeviceId(touchId);
    point.SetState(PointState::MOTION);
    point.SetScreenPosition(Vector2(x, y));
    point.SetRadius(1, Vector2(1, 1));
    point.SetPressure(1.0);
    point.SetAngle(Degree(0.0));
    point.SetDeviceClass(deviceClass);
    point.SetDeviceSubclass(deviceSubclass);
    point.SetDeviceName(deviceName);

    mTouchEventSignal.Emit(point, timestamp);

    if(dev)
    {
      free(dev);
    }
  }
  else
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnMouseButtonMove, Window (%p) and input (%p) comes\n", mTcoreWindow, window);
  }
}

void WindowBaseTcoreWl::OnMouseFrame(void* data, int type, void* event)
{
  tizen_core_wl_event_input_base_h ev = static_cast<tizen_core_wl_event_input_base_h>(event);

  tizen_core_wl_window_h window = NULL;
  tizen_core_wl_event_input_base_get_window(ev, &window);

  if(window == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_FRAME");
    mMouseFrameEventSignal.Emit();
  }
  else
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnMouseFrame, Window (%p) and input (%p) comes\n", mTcoreWindow, window);
  }
}

void WindowBaseTcoreWl::OnMouseButtonRelativeMove(void* data, int type, void* event)
{
  tizen_core_wl_event_input_base_h ev = static_cast<tizen_core_wl_event_input_base_h>(event);

  tizen_core_wl_window_h window = NULL;
  tizen_core_wl_event_input_base_get_window(ev, &window);

  if(window == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_RELATIVE_MOVE");

    int                             dx = 0, dy = 0, dx_unaccel = 0, dy_unaccel = 0;
    unsigned int                    timestamp = 0, modifiers = 0;
    char*                           dev = nullptr;
    tizen_core_wl_device_class_e    tizenClass;
    tizen_core_wl_device_subclass_e tizenSubclass;
    Device::Class::Type             deviceClass;
    Device::Subclass::Type          deviceSubclass;

    GetBaseData(ev, nullptr, &timestamp, &dev);
    GetDeviceInfo(mTcoreDisplay, dev, nullptr, &tizenClass, &tizenSubclass);
    GetDeviceClass(tizenClass, deviceClass);
    GetDeviceSubclass(tizenSubclass, deviceSubclass);

    tizen_core_wl_event_mouse_relative_move_get_delta(ev, &dx, &dy);
    tizen_core_wl_event_mouse_relative_move_get_delta_unaccel(ev, &dx_unaccel, &dy_unaccel);

    Dali::DevelWindow::MouseRelativeEvent mouseRelativeEvent(Dali::DevelWindow::MouseRelativeEvent::Type::RELATIVE_MOVE, modifiers, timestamp, Vector2(dx, dy), Vector2(dx_unaccel, dy_unaccel), deviceClass, deviceSubclass);

    mMouseRelativeEventSignal.Emit(mouseRelativeEvent);

    if(dev)
    {
      free(dev);
    }
  }
  else
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnMouseButtonRelativeMove, Window (%p) and input (%p) comes\n", mTcoreWindow, window);
  }
}

void WindowBaseTcoreWl::OnMouseButtonCancel(void* data, int type, void* event)
{
  tizen_core_wl_event_input_base_h ev = static_cast<tizen_core_wl_event_input_base_h>(event);

  tizen_core_wl_window_h window = NULL;
  tizen_core_wl_event_input_base_get_window(ev, &window);

  if(window == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_CANCEL");

    int                             x = 0, y = 0;
    unsigned int                    touchId = 0, timestamp = 0;
    char*                           dev  = nullptr;
    const char*                     name = nullptr;
    tizen_core_wl_device_class_e    tizenClass;
    tizen_core_wl_device_subclass_e tizenSubclass;
    Device::Class::Type             deviceClass;
    Device::Subclass::Type          deviceSubclass;

    GetBaseData(ev, nullptr, &timestamp, &dev);
    GetDeviceInfo(mTcoreDisplay, dev, &name, &tizenClass, &tizenSubclass);
    GetMouseData(ev, &x, &y, nullptr, &touchId, nullptr, false);
    GetDeviceClass(tizenClass, deviceClass);
    GetDeviceSubclass(tizenSubclass, deviceSubclass);
    Dali::String deviceName(name);

    Integration::Point point;
    point.SetDeviceId(touchId);
    point.SetState(PointState::INTERRUPTED);
    point.SetScreenPosition(Vector2(x, y));
    point.SetDeviceClass(deviceClass);
    point.SetDeviceSubclass(deviceSubclass);
    point.SetDeviceName(deviceName);

    mTouchEventSignal.Emit(point, timestamp);
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseTcoreWl::OnMouseButtonCancel\n");

    if(dev)
    {
      free(dev);
    }
  }
  else
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnMouseButtonCancel, Window (%p) and input (%p) comes\n", mTcoreWindow, window);
  }
}

void WindowBaseTcoreWl::OnPointerConstraints(void* data, int type, void* event)
{
  tizen_core_wl_event_input_base_h ev = static_cast<tizen_core_wl_event_input_base_h>(event);

  tizen_core_wl_window_h window = NULL;
  tizen_core_wl_event_input_base_get_window(ev, &window);

  if(window == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_POINTER_CONSTRAINTS");

    int  x = 0, y = 0;
    bool locked = false, confined = false;

    tizen_core_wl_event_pointer_constraints_get_position(ev, &x, &y);
    tizen_core_wl_event_pointer_constraints_get_locked(ev, &locked);
    tizen_core_wl_event_pointer_constraints_get_confined(ev, &confined);

    Dali::Int32Pair position(x, y);
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnPointerConstraints[%d, %d]\n", position.GetX(), position.GetY());

    mPointerConstraintsSignal.Emit(position, locked, confined);
  }
  else if(ev)
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnMouseButtonRelativeMove, Window (%p) and input (%p) comes\n", mTcoreWindow, window);
  }
}

void WindowBaseTcoreWl::OnMouseWheel(void* data, int type, void* event)
{
  tizen_core_wl_event_input_base_h ev = static_cast<tizen_core_wl_event_input_base_h>(event);

  tizen_core_wl_window_h window = NULL;
  tizen_core_wl_event_input_base_get_window(ev, &window);

  if(window == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_WHEEL");

    int          direction = 0, x = 0, y = 0, z = 0;
    unsigned int timestamp = 0, modifiers = 0;

    GetBaseData(ev, nullptr, &timestamp, nullptr);
    tizen_core_wl_event_mouse_wheel_get_position(ev, &x, &y);
    tizen_core_wl_event_mouse_wheel_get_z(ev, &z);
    tizen_core_wl_event_mouse_wheel_get_direction(ev, &direction);
    tizen_core_wl_event_mouse_wheel_get_modifiers(ev, &modifiers);

    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseTcoreWl::OnMouseWheel: direction: %d, modifiers: %d, x: %d, y: %d, z: %d\n", direction, modifiers, x, y, z);

    Integration::WheelEvent wheelEvent(Integration::WheelEvent::MOUSE_WHEEL, direction, modifiers, Vector2(x, y), z, timestamp);

    mWheelEventSignal.Emit(wheelEvent);
  }
  else
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnMouseWheel, Window (%p) and input (%p) comes\n", mTcoreWindow, window);
  }
}

void WindowBaseTcoreWl::OnMouseInOut(void* data, int type, void* event)
{
  tizen_core_wl_event_input_base_h ev = static_cast<tizen_core_wl_event_input_base_h>(event);

  tizen_core_wl_window_h window = NULL;
  tizen_core_wl_event_input_base_get_window(ev, &window);

  if(window == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_MOUSE_IN_OUT");

    tizen_core_wl_event_type_e      type;
    int                             x = 0, y = 0;
    unsigned int                    timestamp = 0, modifiers = 0;
    char*                           dev  = nullptr;
    const char*                     name = nullptr;
    tizen_core_wl_device_class_e    tizenClass;
    tizen_core_wl_device_subclass_e tizenSubclass;
    Device::Class::Type             deviceClass;
    Device::Subclass::Type          deviceSubclass;

    GetBaseData(ev, &type, &timestamp, &dev);
    GetDeviceInfo(mTcoreDisplay, dev, &name, &tizenClass, &tizenSubclass);
    GetMouseData(ev, &x, &y, nullptr, nullptr, &modifiers, false);
    GetDeviceClass(tizenClass, deviceClass);
    GetDeviceSubclass(tizenSubclass, deviceSubclass);
    Dali::String deviceName(name);

    Dali::DevelWindow::MouseInOutEvent::Type action = (type == TIZEN_CORE_WL_EVENT_MOUSE_IN ? Dali::DevelWindow::MouseInOutEvent::Type::IN : Dali::DevelWindow::MouseInOutEvent::Type::OUT);

    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseTcoreWl::OnMouseInOut: timestamp: %d, modifiers: %d, x: %d, y: %d\n", timestamp, modifiers, x, y);

    Dali::DevelWindow::MouseInOutEvent inOutEvent(action, modifiers, Vector2(x, y), timestamp, deviceClass, deviceSubclass);

    mMouseInOutEventSignal.Emit(inOutEvent);

    if(action == Dali::DevelWindow::MouseInOutEvent::Type::IN)
    {
      Integration::Point point;
      point.SetState(PointState::MOTION);
      point.SetScreenPosition(Vector2(x, y));
      point.SetDeviceClass(deviceClass);
      point.SetDeviceSubclass(deviceSubclass);
      point.SetDeviceName(deviceName);

      mTouchEventSignal.Emit(point, timestamp);
    }

    if(dev)
    {
      free(dev);
    }
  }
  else
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnMouseInOut, Window (%p) and input (%p) comes\n", mTcoreWindow, window);
  }
}

void WindowBaseTcoreWl::OnDetentRotation(void* data, int type, void* event)
{
  tizen_core_wl_event_input_base_h ev = static_cast<tizen_core_wl_event_input_base_h>(event);

  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnDetentRotation, Window (%p)\n", mTcoreWindow);
  if(Dali::Adaptor::IsAvailable())
  {
    unsigned int                       timestamp = 0;
    tizen_core_wl_circular_direction_e direction;

    GetBaseData(ev, nullptr, &timestamp, nullptr);
    tizen_core_wl_event_detent_rotate_get_direction(ev, &direction);

    int32_t clockwise = (direction == TIZEN_CORE_WL_CIRCULAR_DIRECTION_CLOCKWISE) ? 1 : -1;

    Integration::WheelEvent wheelEvent(Integration::WheelEvent::CUSTOM_WHEEL, direction, 0, Vector2(0.0f, 0.0f), clockwise, timestamp);

    mWheelEventSignal.Emit(wheelEvent);
  }
}

void WindowBaseTcoreWl::OnKeyDown(void* data, int type, void* event)
{
  tizen_core_wl_event_input_base_h ev = static_cast<tizen_core_wl_event_input_base_h>(event);

  tizen_core_wl_window_h window = NULL;
  tizen_core_wl_event_input_base_get_window(ev, &window);

  if(window == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    char *                          name = nullptr, *compose = nullptr, *symbol = nullptr, *dev = nullptr;
    const char*                     devName = nullptr;
    unsigned int                    keyCode = 0, modifiers = 0, flags = 0, timestamp = 0;
    tizen_core_wl_device_class_e    tizenClass;
    tizen_core_wl_device_subclass_e tizenSubclass;
    Device::Class::Type             deviceClass;
    Device::Subclass::Type          deviceSubclass;

    GetBaseData(ev, nullptr, &timestamp, &dev);
    GetDeviceInfo(mTcoreDisplay, dev, &devName, &tizenClass, &tizenSubclass);
    GetKeyData(ev, &name, &compose, &symbol, &keyCode, &modifiers, &flags);
    GetDeviceClass(tizenClass, deviceClass);
    GetDeviceSubclass(tizenSubclass, deviceSubclass);
    Dali::String deviceName(devName);
    Dali::String keyName(name);
    Dali::String logicalKey(symbol);
    Dali::String keyString(compose);
    Dali::String keyCompose(compose);

    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_KEY_DOWN");
    DALI_LOG_RELEASE_INFO("name %s code %u key %s compose %s", name, keyCode, symbol, compose);

    if(keyCode == 0)
    {
      // Get a specific key code from dali key look up table.
      int dalyKeyCode = KeyLookup::GetDaliKeyCode(name);
      keyCode         = (dalyKeyCode == -1) ? 0 : dalyKeyCode;
    }

    if(!strncmp(name, "Keycode-", 8))
    {
      keyCode = atoi(name + 8);
    }

    bool isRepeat = false;
    if(flags & TIZEN_CORE_WL_EVENT_FLAG_REPEAT)
    {
      isRepeat = true;
    }

    Integration::KeyEvent keyEvent(keyName, logicalKey, keyString, static_cast<int>(keyCode), static_cast<int>(modifiers), static_cast<unsigned long>(timestamp), Integration::KeyEvent::DOWN, keyCompose, deviceName, deviceClass, deviceSubclass);
    keyEvent.isRepeat    = isRepeat;
    keyEvent.windowId    = GetNativeWindowId();
    keyEvent.receiveTime = TimeService::GetMilliSeconds();

    mKeyEventSignal.Emit(keyEvent);

    if(name)
    {
      free(name);
    }
    if(compose)
    {
      free(compose);
    }
    if(symbol)
    {
      free(symbol);
    }
    if(dev)
    {
      free(dev);
    }
  }
  else
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnKeyDown, Window (%p) and input (%p) comes\n", mTcoreWindow, window);
  }
}

void WindowBaseTcoreWl::OnKeyUp(void* data, int type, void* event)
{
  tizen_core_wl_event_input_base_h ev = static_cast<tizen_core_wl_event_input_base_h>(event);

  tizen_core_wl_window_h window = NULL;
  tizen_core_wl_event_input_base_get_window(ev, &window);

  if(window == mTcoreWindow && Dali::Adaptor::IsAvailable())
  {
    char *                          name = nullptr, *dev = nullptr, *compose = nullptr, *symbol = nullptr;
    unsigned int                    keyCode = 0, modifiers = 0, flags = 0, timestamp = 0;
    const char*                     devName = nullptr;
    tizen_core_wl_device_class_e    tizenClass;
    tizen_core_wl_device_subclass_e tizenSubclass;
    Device::Class::Type             deviceClass;
    Device::Subclass::Type          deviceSubclass;

    GetBaseData(ev, nullptr, &timestamp, &dev);
    GetDeviceInfo(mTcoreDisplay, dev, &devName, &tizenClass, &tizenSubclass);
    GetKeyData(ev, &name, &compose, &symbol, &keyCode, &modifiers, &flags);
    GetDeviceClass(tizenClass, deviceClass);
    GetDeviceSubclass(tizenSubclass, deviceSubclass);
    Dali::String deviceName(devName);

    if(flags & TIZEN_CORE_WL_EVENT_FLAG_CANCEL)
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseTcoreWl::OnKeyUp: This event flag indicates the event is canceled. \n");
      return;
    }

    Dali::String keyName(name);
    Dali::String logicalKey(symbol);
    Dali::String keyString(compose);
    Dali::String keyCompose(compose);

    DALI_TRACE_SCOPE(gTraceFilter, "DALI_ON_KEY_UP");

    if(keyCode == 0)
    {
      // Get a specific key code from dali key look up table.
      int dalyKeyCode = KeyLookup::GetDaliKeyCode(name);
      keyCode         = (dalyKeyCode == -1) ? 0 : dalyKeyCode;
    }
    if(!strncmp(name, "Keycode-", 8))
    {
      keyCode = atoi(name + 8);
    }

    Integration::KeyEvent keyEvent(keyName, logicalKey, keyString, static_cast<int>(keyCode), static_cast<int>(modifiers), static_cast<unsigned long>(timestamp), Integration::KeyEvent::UP, keyCompose, deviceName, deviceClass, deviceSubclass);
    keyEvent.windowId    = GetNativeWindowId();
    keyEvent.receiveTime = TimeService::GetMilliSeconds();

    mKeyEventSignal.Emit(keyEvent);

    if(name)
    {
      free(name);
    }
    if(compose)
    {
      free(compose);
    }
    if(symbol)
    {
      free(symbol);
    }
    if(dev)
    {
      free(dev);
    }
  }
  else
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnKeyDown, Window (%p) and input (%p) comes\n", mTcoreWindow, window);
  }
}

void WindowBaseTcoreWl::OnDataSend(void* data, int type, void* event)
{
  if(Dali::Adaptor::IsAvailable())
  {
    mSelectionDataSendSignal.Emit(event);
  }
}

void WindowBaseTcoreWl::OnDataReceive(void* data, int type, void* event)
{
  if(Dali::Adaptor::IsAvailable())
  {
    mSelectionDataReceivedSignal.Emit(event);
  }
}

void WindowBaseTcoreWl::OnFontNameChanged()
{
  if(Dali::Adaptor::IsAvailable())
  {
    mStyleChangedSignal.Emit(StyleChange::DEFAULT_FONT_CHANGE);
  }
}

void WindowBaseTcoreWl::OnFontSizeChanged()
{
  if(Dali::Adaptor::IsAvailable())
  {
    mStyleChangedSignal.Emit(StyleChange::DEFAULT_FONT_SIZE_CHANGE);
  }
}

void WindowBaseTcoreWl::OnTransitionEffectEvent(void* data, int type, void* event)
{
  if(!event || !mTcoreWindow || !Dali::Adaptor::IsAvailable())
  {
    return;
  }

  tizen_core_wl_event_window_base_h baseEvent = static_cast<tizen_core_wl_event_window_base_h>(event);

  tizen_core_wl_window_h eventWindow = nullptr;
  if(tizen_core_wl_event_window_base_get_window(baseEvent, &eventWindow) != TIZEN_CORE_WL_ERROR_NONE || eventWindow != mTcoreWindow)
  {
    return;
  }

  tizen_core_wl_event_window_effect_h effectEvent = nullptr;
  if(tizen_core_wl_event_window_base_to_effect(baseEvent, &effectEvent) != TIZEN_CORE_WL_ERROR_NONE || !effectEvent)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::OnTransitionEffectEvent: failed to convert to effect event\n");
    return;
  }

  unsigned int effectType = 0u;
  if(tizen_core_wl_event_window_effect_get_type(effectEvent, &effectType) != TIZEN_CORE_WL_ERROR_NONE)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::OnTransitionEffectEvent: failed to get effect type\n");
    return;
  }

  if(effectType >= 3u) // only under restack (same filter as ecore path)
  {
    return;
  }

  const auto        wlEventType = static_cast<tizen_core_wl_event_type_e>(type);
  WindowEffectState state       = WindowEffectState::NONE;
  if(wlEventType == TIZEN_CORE_WL_EVENT_WINDOW_EFFECT_START)
  {
    state = WindowEffectState::START;
  }
  else if(wlEventType == TIZEN_CORE_WL_EVENT_WINDOW_EFFECT_END)
  {
    state = WindowEffectState::END;
  }
  else
  {
    return;
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseTcoreWl::OnTransitionEffectEvent, effect type[ %u ] state[ %d ]\n", effectType, static_cast<int>(state));
  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnTransitionEffectEvent, Window (%p)\n", mTcoreWindow);

  mTransitionEffectEventSignal.Emit(state, static_cast<WindowEffectType>(effectType));
}

void WindowBaseTcoreWl::OnWindowRedrawRequest(void* data, int type, void* event)
{
}

void WindowBaseTcoreWl::OnWindowAuxiliaryMessage(void* data, int type, void* event)
{
  if(!Dali::Adaptor::IsAvailable())
  {
    return;
  }

  tizen_core_wl_event_window_base_h        baseEvent = static_cast<tizen_core_wl_event_window_base_h>(event);
  tizen_core_wl_event_window_aux_message_h auxEvent  = nullptr;
  if(tizen_core_wl_event_window_base_to_aux_message(baseEvent, &auxEvent) != TIZEN_CORE_WL_ERROR_NONE || !auxEvent)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::OnWindowAuxiliaryMessage: invalid aux message event\n");
    return;
  }

  tizen_core_wl_window_h eventWindow = nullptr;
  if(tizen_core_wl_event_window_base_get_window(baseEvent, &eventWindow) != TIZEN_CORE_WL_ERROR_NONE || eventWindow != mTcoreWindow)
  {
    return;
  }

  char* keyCStr = nullptr;
  char* valCStr = nullptr;

  if(tizen_core_wl_event_window_aux_message_get_key(auxEvent, &keyCStr) != TIZEN_CORE_WL_ERROR_NONE)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::OnWindowAuxiliaryMessage: failed to get key\n");
    return;
  }

  if(tizen_core_wl_event_window_aux_message_get_val(auxEvent, &valCStr) != TIZEN_CORE_WL_ERROR_NONE)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::OnWindowAuxiliaryMessage: failed to get value\n");
    if(keyCStr)
    {
      free(keyCStr);
    }
    return;
  }

  std::string           key   = keyCStr ? keyCStr : "";
  std::string           value = valCStr ? valCStr : "";
  Dali::Property::Array options;

  GList* optionList = nullptr;
  if(tizen_core_wl_event_window_aux_message_get_options(auxEvent, &optionList) == TIZEN_CORE_WL_ERROR_NONE && optionList)
  {
    for(GList* l = optionList; l; l = l->next)
    {
      if(l->data)
      {
        options.Add(String(static_cast<char*>(l->data)));
      }
    }
  }

  if(keyCStr)
  {
    free(keyCStr);
  }
  if(valCStr)
  {
    free(valCStr);
  }

  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnWindowAuxiliaryMessage, Window (%p), key:%s, value:%s\n", mTcoreWindow, key.c_str(), value.c_str());
  mAuxiliaryMessageSignal.Emit(key, value, options);
}

void WindowBaseTcoreWl::OnConformantChange(void* data, int type, void* event)
{
  if(Dali::Adaptor::IsAvailable())
  {
    tizen_core_wl_event_window_conformant_change_h confEvent =
      static_cast<tizen_core_wl_event_window_conformant_change_h>(event);

    tizen_core_wl_window_h eventWindow = nullptr;
    tizen_core_wl_event_window_base_get_window(reinterpret_cast<tizen_core_wl_event_window_base_h>(confEvent), &eventWindow);

    if(eventWindow == mTcoreWindow)
    {
      tizen_core_wl_conformant_part_e partType = TIZEN_CORE_WL_CONFORMANT_PART_UNKNOWN;
      bool                            visible  = false;
      if(tizen_core_wl_event_window_conformant_change_get_part(confEvent, &partType) != TIZEN_CORE_WL_ERROR_NONE)
      {
        return;
      }
      tizen_core_wl_event_window_conformant_change_get_visible(confEvent, &visible);

      int x = 0, y = 0, w = 0, h = 0;
      tizen_core_wl_event_window_conformant_change_get_geometry(confEvent, &x, &y, &w, &h);

      WindowInsetsPartType insetsPartType = WindowInsetsPartType::STATUS_BAR;
      switch(partType)
      {
        case TIZEN_CORE_WL_CONFORMANT_PART_INDICATOR:
          insetsPartType               = WindowInsetsPartType::STATUS_BAR;
          mLastIndicatorGeometry.x     = x;
          mLastIndicatorGeometry.y     = y;
          mLastIndicatorGeometry.w     = w;
          mLastIndicatorGeometry.h     = h;
          mLastIndicatorGeometry.state = visible ? 1 : 0;
          DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnConformantChange, indicator geometry: x[%d] y[%d] w[%d] h[%d] state[%d]\n", x, y, w, h, mLastIndicatorGeometry.state);
          break;
        case TIZEN_CORE_WL_CONFORMANT_PART_KEYBOARD:
          insetsPartType              = WindowInsetsPartType::KEYBOARD;
          mLastKeyboardGeometry.x     = x;
          mLastKeyboardGeometry.y     = y;
          mLastKeyboardGeometry.w     = w;
          mLastKeyboardGeometry.h     = h;
          mLastKeyboardGeometry.state = visible ? 1 : 0;
          DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnConformantChange, keyboard geometry: x[%d] y[%d] w[%d] h[%d] state[%d]\n", x, y, w, h, mLastKeyboardGeometry.state);
          break;
        case TIZEN_CORE_WL_CONFORMANT_PART_CLIPBOARD:
          insetsPartType               = WindowInsetsPartType::CLIPBOARD;
          mLastClipboardGeometry.x     = x;
          mLastClipboardGeometry.y     = y;
          mLastClipboardGeometry.w     = w;
          mLastClipboardGeometry.h     = h;
          mLastClipboardGeometry.state = visible ? 1 : 0;
          DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnConformantChange, clipboard geometry: x[%d] y[%d] w[%d] h[%d] state[%d]\n", x, y, w, h, mLastClipboardGeometry.state);
          break;
        default:
          break;
      }

      WindowInsetsPartState partState = visible ? WindowInsetsPartState::VISIBLE : WindowInsetsPartState::INVISIBLE;
      int                   left = 0, right = 0, top = 0, bottom = 0;
      if(visible)
      {
        int winX = mWindowPositionSize.x;
        int winY = mWindowPositionSize.y;
        int winW = mWindowPositionSize.width;
        int winH = mWindowPositionSize.height;

        if((x <= winX) && (x + w >= winX + winW))
        {
          if((y <= winY) && (y + h >= winY) && (y + h <= winY + winH))
          {
            top = y + h - winY;
          }
          else if((y + h >= winY + winH) && (y >= winY) && (y <= winY + winH))
          {
            bottom = winY + winH - y;
          }
        }
        else if((y <= winY) && (y + h >= winY + winH))
        {
          if((x <= winX) && (x + w >= winX) && (x + w <= winX + winW))
          {
            left = x + w - winX;
          }
          else if((x + w >= winX + winW) && (x >= winX) && (x <= winX + winW))
          {
            right = winX + winW - x;
          }
        }
      }

      mInsetsChangedSignal.Emit(insetsPartType, partState, Extents(left, right, top, bottom));
    }
  }
}

void WindowBaseTcoreWl::KeymapChanged(void* data, int type, void* event)
{
  tizen_core_wl_event_seat_changed_h ev = static_cast<tizen_core_wl_event_seat_changed_h>(event);
  tizen_core_wl_display_h            display;
  unsigned int                       id;

  tizen_core_wl_event_seat_changed_get_display(ev, &display);
  tizen_core_wl_event_seat_changed_get_seat_id(ev, &id);
  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::General, "WindowBaseTcoreWl::KeymapChanged, keymap id[ %d ]\n", id);
}

void WindowBaseTcoreWl::OnMoveCompleted(void* data, int type, void* event)
{
  if(!event || !mTcoreWindow || !Dali::Adaptor::IsAvailable())
  {
    return;
  }

  tizen_core_wl_event_window_base_h baseEvent = static_cast<tizen_core_wl_event_window_base_h>(event);

  tizen_core_wl_window_h eventWindow = nullptr;
  if(tizen_core_wl_event_window_base_get_window(baseEvent, &eventWindow) != TIZEN_CORE_WL_ERROR_NONE || eventWindow != mTcoreWindow)
  {
    return;
  }

  tizen_core_wl_event_window_interactive_move_done_h moveDone = nullptr;
  if(tizen_core_wl_event_window_base_to_interactive_move_done(baseEvent, &moveDone) != TIZEN_CORE_WL_ERROR_NONE || !moveDone)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::OnMoveCompleted: failed to convert to interactive move done event\n");
    return;
  }

  int32_t  x = 0, y = 0;
  uint32_t w = 0u, h = 0u;
  if(tizen_core_wl_event_window_interactive_move_done_get_geometry(moveDone, &x, &y, &w, &h) != TIZEN_CORE_WL_ERROR_NONE)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::OnMoveCompleted: failed to get geometry\n");
    return;
  }

  Dali::PositionSize orgPositionSize(x, y, static_cast<int>(w), static_cast<int>(h));
  Dali::PositionSize newPositionSize = RecalculatePositionSizeToCurrentOrientation(orgPositionSize);
  Dali::Int32Pair    newPosition(newPositionSize.x, newPositionSize.y);

  mWindowPositionSize.x = orgPositionSize.x;
  mWindowPositionSize.y = orgPositionSize.y;
  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnMoveCompleted, window(%p) has been moved by server[%d, %d]\n", static_cast<void*>(mTcoreWindow), newPositionSize.x, newPositionSize.y);
  mMoveCompletedSignal.Emit(newPosition);
}

void WindowBaseTcoreWl::OnResizeCompleted(void* data, int type, void* event)
{
  if(!event || !mTcoreWindow || !Dali::Adaptor::IsAvailable())
  {
    return;
  }

  tizen_core_wl_event_window_base_h baseEvent = static_cast<tizen_core_wl_event_window_base_h>(event);

  tizen_core_wl_window_h eventWindow = nullptr;
  if(tizen_core_wl_event_window_base_get_window(baseEvent, &eventWindow) != TIZEN_CORE_WL_ERROR_NONE || eventWindow != mTcoreWindow)
  {
    return;
  }

  tizen_core_wl_event_window_interactive_resize_done_h resizeDone = nullptr;
  if(tizen_core_wl_event_window_base_to_interactive_resize_done(baseEvent, &resizeDone) != TIZEN_CORE_WL_ERROR_NONE || !resizeDone)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::OnResizeCompleted: failed to convert to interactive resize done event\n");
    return;
  }

  int32_t  x = 0, y = 0;
  uint32_t w = 0u, h = 0u;
  if(tizen_core_wl_event_window_interactive_resize_done_get_geometry(resizeDone, &x, &y, &w, &h) != TIZEN_CORE_WL_ERROR_NONE)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::OnResizeCompleted: failed to get geometry\n");
    return;
  }

  Dali::PositionSize orgPositionSize(x, y, static_cast<int>(w), static_cast<int>(h));
  Dali::PositionSize newPositionSize = RecalculatePositionSizeToCurrentOrientation(orgPositionSize);
  SurfaceSize        newSize(newPositionSize.width, newPositionSize.height);

  mWindowPositionSize.width  = orgPositionSize.width;
  mWindowPositionSize.height = orgPositionSize.height;
  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::OnResizeCompleted, window(%p) has been resized by server[%d, %d]\n", static_cast<void*>(mTcoreWindow), newPositionSize.width, newPositionSize.height);
  mResizeCompletedSignal.Emit(newSize);
}

Any WindowBaseTcoreWl::GetNativeWindow()
{
  return mTcoreWindow;
}

void* WindowBaseTcoreWl::GetNativeRawHandle()
{
  return static_cast<void*>(mTcoreWindow);
}

int WindowBaseTcoreWl::GetNativeWindowId()
{
  if(mTcoreWindow)
  {
    return static_cast<int>(mWinId);
  }
  return 0;
}

std::string WindowBaseTcoreWl::GetNativeWindowResourceId()
{
  if(mTcoreWindow)
  {
    unsigned int resourceId = 0u;
    if(tizen_core_wl_window_private_get_resource_id(mTcoreWindow, &resourceId) == TIZEN_CORE_WL_ERROR_NONE)
    {
      return std::to_string(resourceId);
    }
  }

  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::GetNativeWindowResourceId: not supported (tizen-core-wayland API not available yet)\n");
  return std::string();
}

Dali::Any WindowBaseTcoreWl::CreateWindow(int width, int height)
{
  int totalAngle = (mWindowRotationAngle + GetScreenRotationAngle(false)) % 360;

  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "wl_egl_window_create");
  if(totalAngle == 90 || totalAngle == 270)
  {
    mEglWindow = wl_egl_window_create(mWlSurface, height, width);
  }
  else
  {
    mEglWindow = wl_egl_window_create(mWlSurface, width, height);
  }

  return static_cast<void*>(mEglWindow);
}

void WindowBaseTcoreWl::DestroyWindow()
{
  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::DestroyWindow: mEglWindow=%p\n", static_cast<void*>(mEglWindow));
  if(mEglWindow != NULL)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "wl_egl_window_destroy");
    wl_egl_window_destroy(mEglWindow);

    mEglWindow = NULL;
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::DestroyWindow: wl_egl_window_destroy done\n");
  }
  else
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::DestroyWindow: skip (no EGL window)\n");
  }
}

void WindowBaseTcoreWl::SetWindowRotation(int angle)
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

  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "wl_egl_window_tizen_set_rotation");
  wl_egl_window_tizen_set_rotation(mEglWindow, rotation);
}

void WindowBaseTcoreWl::SetWindowBufferTransform(int angle)
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
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "wl_egl_window_tizen_set_buffer_transform");
  wl_egl_window_tizen_set_buffer_transform(mEglWindow, bufferTransform);
}

void WindowBaseTcoreWl::SetWindowTransform(int angle)
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
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "wl_egl_window_tizen_set_window_transform");
  wl_egl_window_tizen_set_window_transform(mEglWindow, windowTransform);
}

void WindowBaseTcoreWl::ResizeWindow(PositionSize positionSize)
{
  DALI_LOG_RELEASE_INFO("wl_egl_window_resize(), (%d, %d) [%d x %d]\n", positionSize.x, positionSize.y, positionSize.width, positionSize.height);
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "wl_egl_window_resize");
  wl_egl_window_resize(mEglWindow, positionSize.width, positionSize.height, positionSize.x, positionSize.y);

  // Note: Both "Resize" and "MoveResize" cases can reach here, but only "MoveResize" needs to submit serial number
  if(mMoveResizeSerial != mLastSubmittedMoveResizeSerial)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "wl_egl_window_tizen_set_window_serial");
    wl_egl_window_tizen_set_window_serial(mEglWindow, mMoveResizeSerial);
    mLastSubmittedMoveResizeSerial = mMoveResizeSerial;
  }
}

bool WindowBaseTcoreWl::IsWindowRotationSupported()
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "wl_egl_window_tizen_get_capabilities");
  // Check capability
  wl_egl_window_tizen_capability capability = static_cast<wl_egl_window_tizen_capability>(wl_egl_window_tizen_get_capabilities(mEglWindow));

  if(capability == WL_EGL_WINDOW_TIZEN_CAPABILITY_ROTATION_SUPPORTED)
  {
    mSupportedPreProtation = true;
    return true;
  }
  mSupportedPreProtation = false;
  return false;
}

Rect<int> WindowBaseTcoreWl::RecalculateInputRect(const Rect<int>& rect, const Rect<int>& surfaceSize)
{
  Rect<int> newRect;

  if(mWindowRotationAngle == 90)
  {
    newRect.x      = rect.y;
    newRect.y      = surfaceSize.height - (rect.x + rect.width);
    newRect.width  = rect.height;
    newRect.height = rect.width;
  }
  else if(mWindowRotationAngle == 180)
  {
    newRect.x      = surfaceSize.width - (rect.x + rect.width);
    newRect.y      = surfaceSize.height - (rect.y + rect.height);
    newRect.width  = rect.width;
    newRect.height = rect.height;
  }
  else if(mWindowRotationAngle == 270)
  {
    newRect.x      = surfaceSize.width - (rect.y + rect.height);
    newRect.y      = rect.x;
    newRect.width  = rect.height;
    newRect.height = rect.width;
  }
  else
  {
    newRect = rect;
  }

  return newRect;
}

PositionSize WindowBaseTcoreWl::RecalculatePositionSizeToSystem(PositionSize positionSize)
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

PositionSize WindowBaseTcoreWl::RecalculatePositionSizeToCurrentOrientation(PositionSize positionSize)
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

void WindowBaseTcoreWl::Move(PositionSize positionSize)
{
  PositionSize newPositionSize = RecalculatePositionSizeToSystem(positionSize);

  mWindowPositionSize = newPositionSize;
  DALI_LOG_RELEASE_INFO("tizen_core_wl_window_set_position x[%d], y[%d]\n", newPositionSize.x, newPositionSize.y);

  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_position");
    tizen_core_wl_window_set_position(mTcoreWindow, newPositionSize.x, newPositionSize.y);
  }
}

void WindowBaseTcoreWl::Resize(PositionSize positionSize)
{
  PositionSize newPositionSize = RecalculatePositionSizeToSystem(positionSize);

  mWindowPositionSize = newPositionSize;
  DALI_LOG_RELEASE_INFO("tizen_core_wl_window_set_geometry, x[%d], y[%d], w{%d], h[%d]\n", newPositionSize.x, newPositionSize.y, newPositionSize.width, newPositionSize.height);

  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_geometry");
    tizen_core_wl_window_set_geometry(mTcoreWindow, newPositionSize.x, newPositionSize.y, newPositionSize.width, newPositionSize.height);
    tizen_core_wl_window_commit(mTcoreWindow);
  }
}

void WindowBaseTcoreWl::MoveResize(PositionSize positionSize)
{
  PositionSize newPositionSize = RecalculatePositionSizeToSystem(positionSize);

  mWindowPositionSize = newPositionSize;
  DALI_LOG_RELEASE_INFO("tizen_core_wl_window_set_geometry, x[%d], y[%d], w{%d], h[%d]\n", newPositionSize.x, newPositionSize.y, newPositionSize.width, newPositionSize.height);

  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_geometry");
    tizen_core_wl_window_set_geometry(mTcoreWindow, newPositionSize.x, newPositionSize.y, newPositionSize.width, newPositionSize.height);
    tizen_core_wl_window_commit(mTcoreWindow);
  }
}

void WindowBaseTcoreWl::SetLayout(unsigned int numCols, unsigned int numRows, unsigned int column, unsigned int row, unsigned int colSpan, unsigned int rowSpan)
{
  unsigned int transformedNumCols = numCols;
  unsigned int transformedNumRows = numRows;
  unsigned int transformedColumn  = column;
  unsigned int transformedRow     = row;
  unsigned int transformedColSpan = colSpan;
  unsigned int transformedRowSpan = rowSpan;

  int totalAngle = (mWindowRotationAngle + mScreenRotationAngle) % 360;

  // Apply rotation transformation to layout parameters
  switch(totalAngle)
  {
    case 90:
    {
      transformedNumCols = numRows;
      transformedNumRows = numCols;
      transformedColumn  = row;
      transformedRow     = (numCols - 1) - column - colSpan + 1;
      transformedColSpan = rowSpan;
      transformedRowSpan = colSpan;
      break;
    }
    case 180:
    {
      transformedColumn = (numCols - 1) - column - colSpan + 1;
      transformedRow    = (numRows - 1) - row - rowSpan + 1;
      break;
    }
    case 270:
    {
      transformedNumCols = numRows;
      transformedNumRows = numCols;
      transformedColumn  = (numRows - 1) - row - rowSpan + 1;
      transformedRow     = column;
      transformedColSpan = rowSpan;
      transformedRowSpan = colSpan;
      break;
    }
    case 0:
    default:
    {
      break;
    }
  }

  DALI_LOG_RELEASE_INFO("tizen_core_wl_window_set_layout, original: numCols[%d], numRows[%d], column[%d], row[%d], colSpan[%d], rowSpan[%d]\n",
                        numCols,
                        numRows,
                        column,
                        row,
                        colSpan,
                        rowSpan);
  DALI_LOG_RELEASE_INFO("tizen_core_wl_window_set_layout, transformed: numCols[%d], numRows[%d], column[%d], row[%d], colSpan[%d], rowSpan[%d], rotation[%d]\n",
                        transformedNumCols,
                        transformedNumRows,
                        transformedColumn,
                        transformedRow,
                        transformedColSpan,
                        transformedRowSpan,
                        totalAngle);

  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_layout");
    tizen_core_wl_window_set_layout(mTcoreWindow, transformedNumCols, transformedNumRows, transformedColumn, transformedRow, transformedColSpan, transformedRowSpan);
  }
}

void WindowBaseTcoreWl::SetClass(const std::string& name, const std::string& className)
{
  DALI_LOG_RELEASE_INFO("[SET_CLASS_TRACE] WindowBaseTcoreWl::SetClass() this[%p], tcoreWindow[%p], name[%s], className[%s]\n",
                        this,
                        mTcoreWindow,
                        name.c_str(),
                        className.c_str());
  if(mTcoreWindow)
  {
    DALI_LOG_RELEASE_INFO("tizen_core_wl_window_set_title, Set Title[%s]\n", name.c_str());
    tizen_core_wl_window_set_title(mTcoreWindow, name.c_str());
    DALI_LOG_RELEASE_INFO("tizen_core_wl_window_set_app_id, Set App ID[%s]\n", className.c_str());
    tizen_core_wl_window_set_app_id(mTcoreWindow, className.c_str());
  }
}

void WindowBaseTcoreWl::Raise()
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_activate");
    tizen_core_wl_window_raise(mTcoreWindow);
  }
}

void WindowBaseTcoreWl::Lower()
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_lower");
    tizen_core_wl_window_lower(mTcoreWindow);
  }
}

void WindowBaseTcoreWl::Activate()
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_activate");
    tizen_core_wl_window_activate(mTcoreWindow);
  }
}

void WindowBaseTcoreWl::Maximize(bool maximize)
{
  if(mTcoreWindow)
  {
    mWindowStateStatus                = maximize ? WindowStateStatus::MAXIMIZED : WindowStateStatus::NORMAL;
    mPendingRestoreResizeOnUnmaximize = !maximize;

    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_state");
    tizen_core_wl_window_set_state(mTcoreWindow, maximize ? TIZEN_CORE_WL_WINDOW_STATE_MAXIMIZED : TIZEN_CORE_WL_WINDOW_STATE_NONE);
  }
}

void WindowBaseTcoreWl::MaximizeWithRestoreSize(bool maximize, Dali::Window::WindowSize size)
{
  mRestoreWindowWidth  = size.GetWidth();
  mRestoreWindowHeight = size.GetHeight();

  mSetRestoreWindowUnmaximize = true;

  if(mTcoreWindow)
  {
    mWindowStateStatus = maximize ? WindowStateStatus::MAXIMIZED : WindowStateStatus::NORMAL;

    // On unmaximize, restore geometry is applied from OnConfiguration().
    mPendingRestoreResizeOnUnmaximize = !maximize;

    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::MaximizeWithRestoreSize, maximize [%d], restore [%d x %d]\n",
                          maximize ? 1 : 0,
                          mRestoreWindowWidth,
                          mRestoreWindowHeight);

    // Tizen Core WL has no maximized_set_with_size equivalent. Request state change first.
    tizen_core_wl_window_set_state(mTcoreWindow, maximize ? TIZEN_CORE_WL_WINDOW_STATE_MAXIMIZED : TIZEN_CORE_WL_WINDOW_STATE_NONE);
  }
}

bool WindowBaseTcoreWl::IsMaximized() const
{
  if(mTcoreWindow)
  {
    tizen_core_wl_window_state_e state = TIZEN_CORE_WL_WINDOW_STATE_NONE;
    return (tizen_core_wl_window_get_state(mTcoreWindow, &state) == TIZEN_CORE_WL_ERROR_NONE &&
            state == TIZEN_CORE_WL_WINDOW_STATE_MAXIMIZED);
  }
  return false;
}

void WindowBaseTcoreWl::SetMaximumSize(Dali::Window::WindowSize size)
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_max_size");
    tizen_core_wl_window_set_max_size(mTcoreWindow, size.GetWidth(), size.GetHeight());
  }
}

void WindowBaseTcoreWl::Minimize(bool minimize)
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_iconify_state");
    tizen_core_wl_window_set_iconify_state(mTcoreWindow, minimize);
  }
}

bool WindowBaseTcoreWl::IsMinimized() const
{
  if(mTcoreWindow)
  {
    bool iconified = false;
    return (tizen_core_wl_window_get_iconify_state(mTcoreWindow, &iconified) == TIZEN_CORE_WL_ERROR_NONE && iconified);
  }
  return false;
}

void WindowBaseTcoreWl::SetMinimumSize(Dali::Window::WindowSize size)
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_min_size");
    tizen_core_wl_window_set_min_size(mTcoreWindow, size.GetWidth(), size.GetHeight());
  }
}

void WindowBaseTcoreWl::SetAvailableAnlges(const std::vector<int>& angles)
{
  std::vector<tizen_core_wl_window_angle_e> availableAngles;
  availableAngles.reserve(angles.size());

  for(std::size_t i = 0; i < angles.size(); ++i)
  {
    availableAngles.push_back(static_cast<tizen_core_wl_window_angle_e>(angles[i]));
  }

  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_available_rotation_angle_list");
    tizen_core_wl_error_e err = tizen_core_wl_window_set_available_rotation_angle_list(
      mTcoreWindow,
      availableAngles.empty() ? nullptr : availableAngles.data(),
      availableAngles.size());
    if(err != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::SetAvailableAnlges: Error! [%d]\n", err);
    }
  }
}

void WindowBaseTcoreWl::SetPreferredAngle(int angle)
{
  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::SetPreferredAngle, angle: %d\n", angle);
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_preferred_rotation_angle");
    tizen_core_wl_error_e err = tizen_core_wl_window_set_preferred_rotation_angle(mTcoreWindow, static_cast<tizen_core_wl_window_angle_e>(angle));
    if(err != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::SetPreferredAngle: Error! [%d]\n", err);
    }
  }
}

void WindowBaseTcoreWl::SetAcceptFocus(bool accept)
{
  if(mTcoreWindow)
  {
    // Tizen Core WL has no focus_skip equivalent
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_focus_skip");
    tizen_core_wl_window_set_focus_skip(mTcoreWindow, accept);
  }
}

void WindowBaseTcoreWl::Show()
{
  // if(!mVisible)
  // {
  //   if(mTcoreWindow)
  //   {
  //     DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_geometry");
  //     tizen_core_wl_window_set_geometry(mTcoreWindow, mWindowPositionSize.x, mWindowPositionSize.y, mWindowPositionSize.width, mWindowPositionSize.height);
  //   }
  // }
  mVisible = true;

  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_show");
    tizen_core_wl_window_show(mTcoreWindow);
  }
}

void WindowBaseTcoreWl::Hide()
{
  mVisible = false;

  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_hide");
    tizen_core_wl_window_hide(mTcoreWindow);
  }
}

unsigned int WindowBaseTcoreWl::GetSupportedAuxiliaryHintCount() const
{
  return mSupportedAuxiliaryHints.size();
}

std::string WindowBaseTcoreWl::GetSupportedAuxiliaryHint(unsigned int index) const
{
  if(index >= GetSupportedAuxiliaryHintCount())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::GetSupportedAuxiliaryHint: Invalid index! [%d]\n", index);
  }

  return mSupportedAuxiliaryHints[index];
}

unsigned int WindowBaseTcoreWl::AddAuxiliaryHint(const std::string& hint, const std::string& value)
{
  // Check if the hint is already added
  for(unsigned int i = 0; i < mAuxiliaryHints.size(); i++)
  {
    if(mAuxiliaryHints[i].first == hint)
    {
      unsigned int id = i + 1;
      if(mAuxiliaryHints[i].second == value)
      {
        DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::AddAuxiliaryHint: skip same hint/value. hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), id);
        return id;
      }

      mAuxiliaryHints[i].second = value;

      if(mTcoreWindow)
      {
        DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_aux_hint");
        DALI_LOG_RELEASE_INFO("tizen_core_wl_window_set_aux_hint: change hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), id);
        tizen_core_wl_window_set_aux_hint(mTcoreWindow, hint.c_str(), value.c_str());
      }

      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::AddAuxiliaryHint: Change! hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), id);

      return id;
    }
  }

  // Add the hint
  mAuxiliaryHints.push_back(std::pair<std::string, std::string>(hint, value));

  unsigned int id = mAuxiliaryHints.size();

  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_aux_hint");
    DALI_LOG_RELEASE_INFO("tizen_core_wl_window_set_aux_hint: hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), id);
    tizen_core_wl_window_set_aux_hint(mTcoreWindow, hint.c_str(), value.c_str());
  }

  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::AddAuxiliaryHint: hint = %s, value = %s, id = %d\n", hint.c_str(), value.c_str(), id);

  return id;
}

bool WindowBaseTcoreWl::RemoveAuxiliaryHint(unsigned int id)
{
  if(id == 0 || id > mAuxiliaryHints.size())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Concise, "WindowBaseTcoreWl::RemoveAuxiliaryHint: Invalid id [%d]\n", id);
    return false;
  }

  const std::string hintToRemove = mAuxiliaryHints[id - 1].first;
  mAuxiliaryHints[id - 1].second = std::string();

  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_unset_aux_hint");
    tizen_core_wl_window_unset_aux_hint(mTcoreWindow, hintToRemove.c_str());
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::RemoveAuxiliaryHint: id = %d, hint = %s\n", id, hintToRemove.c_str());

  return true;
}

bool WindowBaseTcoreWl::SetAuxiliaryHintValue(unsigned int id, const std::string& value)
{
  if(id == 0 || id > mAuxiliaryHints.size())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Concise, "WindowBaseTcoreWl::SetAuxiliaryHintValue: Invalid id [%d]\n", id);
    return false;
  }

  mAuxiliaryHints[id - 1].second = value;

  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_aux_hint");
    tizen_core_wl_window_set_aux_hint(mTcoreWindow, mAuxiliaryHints[id - 1].first.c_str(), value.c_str());
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::SetAuxiliaryHintValue: id = %d, hint = %s, value = %s\n", id, mAuxiliaryHints[id - 1].first.c_str(), mAuxiliaryHints[id - 1].second.c_str());

  return true;
}

std::string WindowBaseTcoreWl::GetAuxiliaryHintValue(unsigned int id) const
{
  if(id == 0 || id > mAuxiliaryHints.size())
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Concise, "WindowBaseTcoreWl::GetAuxiliaryHintValue: Invalid id [%d]\n", id);
    return std::string();
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::GetAuxiliaryHintValue: id = %d, hint = %s, value = %s\n", id, mAuxiliaryHints[id - 1].first.c_str(), mAuxiliaryHints[id - 1].second.c_str());

  return mAuxiliaryHints[id - 1].second;
}

unsigned int WindowBaseTcoreWl::GetAuxiliaryHintId(const std::string& hint) const
{
  for(unsigned int i = 0; i < mAuxiliaryHints.size(); i++)
  {
    if(mAuxiliaryHints[i].first == hint)
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::GetAuxiliaryHintId: hint = %s, id = %d\n", hint.c_str(), i + 1);
      return i + 1;
    }
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::GetAuxiliaryHintId: Invalid hint! [%s]\n", hint.c_str());

  return 0;
}

void WindowBaseTcoreWl::SetInputRegion(const Rect<int>& inputRegion)
{
  Rect<int> convertRegion = RecalculateInputRect(inputRegion, mWindowPositionSize);

  if(mTcoreWindow && mTcoreDisplay)
  {
    // Recreate region to replace previous input region.
    if(mDisplayRegion)
    {
      tizen_core_wl_display_destroy_region(mTcoreDisplay, mDisplayRegion);
      mDisplayRegion = nullptr;
    }

    tizen_core_wl_error_e err = tizen_core_wl_display_create_region(mTcoreDisplay, &mDisplayRegion);
    if(err != TIZEN_CORE_WL_ERROR_NONE || !mDisplayRegion)
    {
      DALI_LOG_ERROR("Failed to create input region\n");
      mDisplayRegion = nullptr;
      return;
    }

    tizen_core_wl_rect_s rect;
    rect.x = convertRegion.x;
    rect.y = convertRegion.y;
    rect.w = convertRegion.width;
    rect.h = convertRegion.height;

    err = tizen_core_wl_region_add_rect(mDisplayRegion, rect);
    if(err != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_ERROR("Failed to add rect to input region\n");
      return;
    }

    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_input_region");
    tizen_core_wl_window_set_input_region(mTcoreWindow, mDisplayRegion);
    tizen_core_wl_window_commit(mTcoreWindow);
  }
}

void WindowBaseTcoreWl::SetType(Dali::WindowType type)
{
  if(mType != type)
  {
    mType = type;
    tizen_core_wl_window_type_e windowType;

    switch(type)
    {
      case Dali::WindowType::NORMAL:
      {
        DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::SetType, Dali::WindowType::NORMAL\n");
        windowType = TIZEN_CORE_WL_WINDOW_TYPE_TOPLEVEL;
        break;
      }
      case Dali::WindowType::NOTIFICATION:
      {
        DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::SetType, Dali::WindowType::NOTIFICATION\n");
        windowType = TIZEN_CORE_WL_WINDOW_TYPE_NOTIFICATION;
        break;
      }
      case Dali::WindowType::UTILITY:
      {
        DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::SetType, Dali::WindowType::UTILITY\n");
        windowType = TIZEN_CORE_WL_WINDOW_TYPE_UTILITY;
        break;
      }
      case Dali::WindowType::DIALOG:
      {
        DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::SetType, Dali::WindowType::DIALOG\n");
        windowType = TIZEN_CORE_WL_WINDOW_TYPE_DIALOG;
        break;
      }
      case Dali::WindowType::IME:
      {
        DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::SetType, Dali::WindowType::IME\n");
        windowType = TIZEN_CORE_WL_WINDOW_TYPE_NONE;
        break;
      }
      case Dali::WindowType::DESKTOP:
      {
        DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::SetType, Dali::WindowType::DESKTOP\n");
        windowType = TIZEN_CORE_WL_WINDOW_TYPE_DESKTOP;
        break;
      }
      default:
      {
        DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::SetType, default window type\n");
        windowType = TIZEN_CORE_WL_WINDOW_TYPE_TOPLEVEL;
        break;
      }
    }

    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "window_type_set");
    if(mTcoreWindow)
    {
      tizen_core_wl_window_set_type(mTcoreWindow, windowType);
    }
  }
}

Dali::WindowType WindowBaseTcoreWl::GetType() const
{
  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::GetType(), Window (%p), DALI WindType: %d, mIsIMEWindowInitialized: %d\n", mTcoreWindow, mType, mIsIMEWindowInitialized);
  return mType;
}

Dali::WindowOperationResult WindowBaseTcoreWl::SetNotificationLevel(Dali::WindowNotificationLevel level)
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::SetNotificationLevel");
  tizen_core_wl_notification_level_e notificationLevel;

  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::SetNotificationLevel(), Window (%p), level [%d]\n", mTcoreWindow, level);
  switch(level)
  {
    case Dali::WindowNotificationLevel::NONE:
    {
      notificationLevel = TIZEN_CORE_WL_NOTIFICATION_LEVEL_NONE;
      break;
    }
    case Dali::WindowNotificationLevel::BASE:
    {
      notificationLevel = TIZEN_CORE_WL_NOTIFICATION_LEVEL_DEFAULT;
      break;
    }
    case Dali::WindowNotificationLevel::MEDIUM:
    {
      notificationLevel = TIZEN_CORE_WL_NOTIFICATION_LEVEL_MEDIUM;
      break;
    }
    case Dali::WindowNotificationLevel::HIGH:
    {
      notificationLevel = TIZEN_CORE_WL_NOTIFICATION_LEVEL_HIGH;
      break;
    }
    case Dali::WindowNotificationLevel::TOP:
    {
      notificationLevel = TIZEN_CORE_WL_NOTIFICATION_LEVEL_TOP;
      break;
    }
    default:
    {
      DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::SetNotificationLevel(), invalid level [%d]\n", level);
      notificationLevel = TIZEN_CORE_WL_NOTIFICATION_LEVEL_DEFAULT;
      break;
    }
  }

  if(mTcoreWindow)
  {
    tizen_core_wl_notification_set_level(mTcoreWindow, notificationLevel);
    return Dali::WindowOperationResult::SUCCEED;
  }

  return Dali::WindowOperationResult::UNKNOWN_ERROR;
}

Dali::WindowNotificationLevel WindowBaseTcoreWl::GetNotificationLevel() const
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::GetNotificationLevel");

  if(!mTcoreWindow)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::GetNotificationLevel: Error! [Invalid Window]\n");
    return Dali::WindowNotificationLevel::NONE;
  }

  Dali::WindowNotificationLevel      level;
  tizen_core_wl_notification_level_e notificationLevel;
  tizen_core_wl_error_e              ret = tizen_core_wl_notification_get_level(mTcoreWindow, &notificationLevel);

  if(ret != TIZEN_CORE_WL_ERROR_NONE)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::GetNotificationLevel: Error! [%d]\n", ret);
    return Dali::WindowNotificationLevel::NONE;
  }

  switch(notificationLevel)
  {
    case TIZEN_CORE_WL_NOTIFICATION_LEVEL_NONE:
    {
      level = Dali::WindowNotificationLevel::NONE;
      break;
    }
    case TIZEN_CORE_WL_NOTIFICATION_LEVEL_DEFAULT:
    {
      level = Dali::WindowNotificationLevel::BASE;
      break;
    }
    case TIZEN_CORE_WL_NOTIFICATION_LEVEL_MEDIUM:
    {
      level = Dali::WindowNotificationLevel::MEDIUM;
      break;
    }
    case TIZEN_CORE_WL_NOTIFICATION_LEVEL_HIGH:
    {
      level = Dali::WindowNotificationLevel::HIGH;
      break;
    }
    case TIZEN_CORE_WL_NOTIFICATION_LEVEL_TOP:
    {
      level = Dali::WindowNotificationLevel::TOP;
      break;
    }
    default:
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::GetNotificationLevel: invalid level [%d]\n", notificationLevel);
      level = Dali::WindowNotificationLevel::NONE;
      break;
    }
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::GetNotificationLevel: level [%d]\n", notificationLevel);

  return level;
}

void WindowBaseTcoreWl::SetOpaqueState(bool opaque)
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::SetOpaqueState");

  if(mTcoreWindow)
  {
    tizen_core_wl_window_set_opaque_state(mTcoreWindow, (opaque ? 1 : 0));
  }
}

Dali::WindowOperationResult WindowBaseTcoreWl::SetScreenOffMode(WindowScreenOffMode screenOffMode)
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::SetScreenOffMode");

  if(!mTcoreWindow)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::SetScreenOffMode: Error! [Invalid Window]\n");
    return Dali::WindowOperationResult::UNKNOWN_ERROR;
  }

  tizen_core_wl_screen_mode_e mode = TIZEN_CORE_WL_SCREEN_MODE_DEFAULT;

  switch(screenOffMode)
  {
    case WindowScreenOffMode::TIMEOUT:
    {
      mode = TIZEN_CORE_WL_SCREEN_MODE_DEFAULT;
      break;
    }
    case WindowScreenOffMode::NEVER:
    {
      mode = TIZEN_CORE_WL_SCREEN_MODE_ALWAYS_ON;
      break;
    }
  }

  tizen_core_wl_error_e ret = tizen_core_wl_window_set_screen_mode(mTcoreWindow, mode);

  if(ret != TIZEN_CORE_WL_ERROR_NONE)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::SetScreenOffMode: Error! [%d]\n", ret);
    return Dali::WindowOperationResult::UNKNOWN_ERROR;
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::SetScreenOffMode: Screen mode is changed [%d]\n", screenOffMode);

  return Dali::WindowOperationResult::SUCCEED;
}

WindowScreenOffMode WindowBaseTcoreWl::GetScreenOffMode() const
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::GetScreenOffMode");

  WindowScreenOffMode screenMode = WindowScreenOffMode::TIMEOUT;

  if(!mTcoreWindow)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::GetScreenOffMode: Error! [Invalid Window]\n");
    return screenMode;
  }

  tizen_core_wl_screen_mode_e mode;
  tizen_core_wl_error_e       ret = tizen_core_wl_window_get_screen_mode(mTcoreWindow, &mode);

  if(ret != TIZEN_CORE_WL_ERROR_NONE)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::GetScreenOffMode: Error! [%d]\n", ret);
    return screenMode;
  }

  switch(mode)
  {
    case TIZEN_CORE_WL_SCREEN_MODE_DEFAULT:
    {
      screenMode = WindowScreenOffMode::TIMEOUT;
      break;
    }
    case TIZEN_CORE_WL_SCREEN_MODE_ALWAYS_ON:
    {
      screenMode = WindowScreenOffMode::NEVER;
      break;
    }
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::GetScreenOffMode: screen mode [%d]\n", screenMode);

  return screenMode;
}

Dali::WindowOperationResult WindowBaseTcoreWl::SetBrightness(int brightness)
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::SetBrightness");

  if(!mTcoreWindow)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::SetBrightness: Error! [Invalid Window]\n");
    return Dali::WindowOperationResult::UNKNOWN_ERROR;
  }

  tizen_core_wl_error_e ret = tizen_core_wl_window_set_brightness(mTcoreWindow, brightness);

  if(ret != TIZEN_CORE_WL_ERROR_NONE)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::SetBrightness: Error! [%d]\n", ret);
    return Dali::WindowOperationResult::UNKNOWN_ERROR;
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::SetBrightness: Brightness is changed [%d]\n", brightness);

  return Dali::WindowOperationResult::SUCCEED;
}

int WindowBaseTcoreWl::GetBrightness() const
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::GetBrightness");

  if(!mTcoreWindow)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::GetBrightness: Error! [Invalid Window]\n");
    return 0;
  }

  int                   brightness = 0;
  tizen_core_wl_error_e ret        = tizen_core_wl_window_get_brightness(mTcoreWindow, &brightness);

  if(ret != TIZEN_CORE_WL_ERROR_NONE)
  {
    DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::GetBrightness: Error! [%d]\n", ret);
    return 0;
  }

  DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::GetBrightness: Brightness [%d]\n", brightness);

  return brightness;
}

bool WindowBaseTcoreWl::GrabKey(Dali::KEY key, KeyGrab::KeyGrabMode grabMode)
{
  tizen_core_wl_keygrab_mode_e mode = TIZEN_CORE_WL_KEYGRAB_UNKNOWN;

  switch(grabMode)
  {
    case KeyGrab::TOPMOST:
    {
      mode = TIZEN_CORE_WL_KEYGRAB_TOPMOST;
      break;
    }
    case KeyGrab::SHARED:
    {
      mode = TIZEN_CORE_WL_KEYGRAB_SHARED;
      break;
    }
    case KeyGrab::OVERRIDE_EXCLUSIVE:
    {
      mode = TIZEN_CORE_WL_KEYGRAB_OVERRIDE_EXCLUSIVE;
      break;
    }
    case KeyGrab::EXCLUSIVE:
    {
      mode = TIZEN_CORE_WL_KEYGRAB_EXCLUSIVE;
      break;
    }
    default:
    {
      return false;
    }
  }

  if(mTcoreWindow)
  {
    tizen_core_wl_keygrab_info_h info = nullptr;
    if(tizen_core_wl_keygrab_info_create(KeyLookup::GetKeyName(key), mode, &info) != TIZEN_CORE_WL_ERROR_NONE || !info)
    {
      return false;
    }
    GList*                list = g_list_append(nullptr, info);
    tizen_core_wl_error_e err  = tizen_core_wl_window_set_keygrab_list(mTcoreWindow, list);
    tizen_core_wl_keygrab_info_destroy(info);
    g_list_free(list);

    return (err == TIZEN_CORE_WL_ERROR_NONE);
  }
  return false;
}

bool WindowBaseTcoreWl::UngrabKey(Dali::KEY key)
{
  if(mTcoreWindow)
  {
    return (tizen_core_wl_window_set_keygrab_list(mTcoreWindow, nullptr) == TIZEN_CORE_WL_ERROR_NONE);
  }
  return false;
}

bool WindowBaseTcoreWl::GrabKeyList(const Dali::Vector<Dali::KEY>& key, const Dali::Vector<KeyGrab::KeyGrabMode>& grabMode, Dali::Vector<bool>& result)
{
  int keyCount         = key.Count();
  int keyGrabModeCount = grabMode.Count();

  if(keyCount != keyGrabModeCount || keyCount == 0)
  {
    return false;
  }
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::GrabKeyList");

  if(!mTcoreWindow)
  {
    return false;
  }

  GList* list = nullptr;
  for(int index = 0; index < keyCount; ++index)
  {
    tizen_core_wl_keygrab_mode_e tcoreMode = TIZEN_CORE_WL_KEYGRAB_UNKNOWN;
    switch(grabMode[index])
    {
      case KeyGrab::TOPMOST:
        tcoreMode = TIZEN_CORE_WL_KEYGRAB_TOPMOST;
        break;
      case KeyGrab::SHARED:
        tcoreMode = TIZEN_CORE_WL_KEYGRAB_SHARED;
        break;
      case KeyGrab::OVERRIDE_EXCLUSIVE:
        tcoreMode = TIZEN_CORE_WL_KEYGRAB_OVERRIDE_EXCLUSIVE;
        break;
      case KeyGrab::EXCLUSIVE:
        tcoreMode = TIZEN_CORE_WL_KEYGRAB_EXCLUSIVE;
        break;
      default:
        break;
    }
    tizen_core_wl_keygrab_info_h info = nullptr;
    if(tizen_core_wl_keygrab_info_create(KeyLookup::GetKeyName(key[index]), tcoreMode, &info) == TIZEN_CORE_WL_ERROR_NONE && info)
    {
      list = g_list_append(list, info);
    }
  }

  DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
  tizen_core_wl_error_e err = tizen_core_wl_window_set_keygrab_list(mTcoreWindow, list);
  DALI_TIME_CHECKER_END_WITH_MESSAGE_GENERATOR(gTimeCheckerFilter, [&](std::ostringstream& oss)
  { oss << "tizen_core_wl_window_set_keygrab_list [" << keyCount << "]"; });

  for(GList* l = list; l; l = l->next)
  {
    tizen_core_wl_keygrab_info_destroy(static_cast<tizen_core_wl_keygrab_info_h>(l->data));
  }
  g_list_free(list);

  result.Resize(keyCount, true);
  if(err != TIZEN_CORE_WL_ERROR_NONE)
  {
    for(int index = 0; index < keyCount; ++index)
    {
      result[index] = false;
    }
    return false;
  }
  return true;
}

bool WindowBaseTcoreWl::UngrabKeyList(const Dali::Vector<Dali::KEY>& key, Dali::Vector<bool>& result)
{
  int keyCount = key.Count();
  if(keyCount == 0)
  {
    return false;
  }
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::UngrabKeyList");

  if(!mTcoreWindow)
  {
    return false;
  }

  DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
  tizen_core_wl_error_e err = tizen_core_wl_window_set_keygrab_list(mTcoreWindow, nullptr);
  DALI_TIME_CHECKER_END_WITH_MESSAGE_GENERATOR(gTimeCheckerFilter, [&](std::ostringstream& oss)
  { oss << "tizen_core_wl_window_set_keygrab_list(clear) [" << keyCount << "]"; });

  result.Resize(keyCount, true);
  return (err == TIZEN_CORE_WL_ERROR_NONE);
}

void WindowBaseTcoreWl::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::GetDpi");
  dpiHorizontal = 96;
  dpiVertical   = 96;
  if(mTcoreWindow)
  {
    tizen_core_wl_display_h display = nullptr;
    if(tizen_core_wl_window_get_display(mTcoreWindow, &display) == TIZEN_CORE_WL_ERROR_NONE && display)
    {
      GList* outputs = nullptr;
      if(tizen_core_wl_display_get_output_device_list(display, &outputs) == TIZEN_CORE_WL_ERROR_NONE && outputs)
      {
        tizen_core_wl_output_h output = static_cast<tizen_core_wl_output_h>(outputs->data);
        int                    dpi    = 96;
        if(output && tizen_core_wl_output_device_get_dpi(output, &dpi) == TIZEN_CORE_WL_ERROR_NONE)
        {
          dpiHorizontal = static_cast<unsigned int>(dpi);
          dpiVertical   = static_cast<unsigned int>(dpi);
        }
        g_list_free(outputs);
      }
    }
  }
}

int WindowBaseTcoreWl::GetWindowRotationAngle() const
{
  int orientation = mWindowRotationAngle;
  if(mSupportedPreProtation)
  {
    orientation = 0;
  }
  return orientation;
}

bool WindowBaseTcoreWl::UpdateScreenRotationAngle()
{
  int  transform = 0, newAngle = 0;
  bool isChanged = false;
  if(mSupportedPreProtation)
  {
    DALI_LOG_RELEASE_INFO("Support PreRotation, so return 0, Window(%p), WindowId(%d)\n", mTcoreWindow, GetNativeWindowId());
    return 0;
  }

  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::UpdateScreenRotationAngle");
    tizen_core_wl_display_h display = nullptr;
    if(tizen_core_wl_window_get_display(mTcoreWindow, &display) == TIZEN_CORE_WL_ERROR_NONE && display)
    {
      GList* outputs = nullptr;
      if(tizen_core_wl_display_get_output_device_list(display, &outputs) == TIZEN_CORE_WL_ERROR_NONE && outputs)
      {
        tizen_core_wl_output_h output = static_cast<tizen_core_wl_output_h>(outputs->data);
        if(output && tizen_core_wl_output_device_get_transform(output, &transform) == TIZEN_CORE_WL_ERROR_NONE)
        {
          newAngle = (transform * 90);
        }
        g_list_free(outputs);
      }
    }
  }
  if(newAngle != mScreenRotationAngle)
  {
    DALI_LOG_RELEASE_INFO("Change Screen Rotation Angle, Pre(%d), New(%d), Window(%p), WindowId(%d)\n", mScreenRotationAngle, newAngle, mTcoreWindow, GetNativeWindowId());
    mScreenRotationAngle = newAngle;
    isChanged            = true;
  }
  DALI_LOG_RELEASE_INFO("UpdateScreenRotationAngle's (%d), isChanged(%d), Window(%p), WindowId(%d)\n", mScreenRotationAngle, isChanged, mTcoreWindow, GetNativeWindowId());
  return isChanged;
}

int WindowBaseTcoreWl::GetScreenRotationAngle(const bool update)
{
  if(mSupportedPreProtation)
  {
    DALI_LOG_RELEASE_INFO("Support PreRotation, so return 0, Window(%p), WindowId(%d)\n", mTcoreWindow, GetNativeWindowId());
    return 0;
  }

  if(update)
  {
    UpdateScreenRotationAngle();
    DALI_LOG_RELEASE_INFO("ScreenRotationAngle (%d), Window(%p), WindowId(%d)\n", mScreenRotationAngle, mTcoreWindow, GetNativeWindowId());
  }

  return mScreenRotationAngle;
}

void WindowBaseTcoreWl::SetWindowRotationAngle(int degree)
{
  mWindowRotationAngle = degree;
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_rotation_angle");
    tizen_core_wl_error_e err = tizen_core_wl_window_set_rotation_angle(mTcoreWindow, static_cast<tizen_core_wl_window_angle_e>(mWindowRotationAngle));
    if(err != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::SetWindowRotationAngle: Error! [%d]\n", err);
    }
  }
}

void WindowBaseTcoreWl::WindowRotationCompleted(int degree, int width, int height)
{
  (void)width;
  (void)height;
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_send_rotation_change_done");
    tizen_core_wl_window_send_rotation_change_done(mTcoreWindow, degree);
  }
}

void WindowBaseTcoreWl::SetTransparency(bool transparent)
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_alpha");
    tizen_core_wl_window_set_alpha(mTcoreWindow, transparent);
  }
  else
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "window_alpha_set");
    if(mTcoreWindow)
    {
      tizen_core_wl_window_set_alpha(mTcoreWindow, transparent);
    }
  }
}

void WindowBaseTcoreWl::CreateInternalWindow(PositionSize positionSize)
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::CreateInternalWindow");

  // 1) Create & connect Wayland display using Tizen Core WL
  tizen_core_wl_error_e wlError = tizen_core_wl_display_create(&mTcoreDisplay);
  if(wlError != TIZEN_CORE_WL_ERROR_NONE || !mTcoreDisplay)
  {
    DALI_LOG_ERROR("Failed to create Tizen Core WL display: %d\n", wlError);
    DALI_ASSERT_ALWAYS(0 && "Failed to create Tizen Core WL display");
  }

  wlError = tizen_core_wl_display_connect(mTcoreDisplay, nullptr);
  if(wlError != TIZEN_CORE_WL_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed to connect Tizen Core WL display: %d\n", wlError);
    DALI_ASSERT_ALWAYS(0 && "Failed to connect Tizen Core WL display");
  }

  // 2) Create window via Tizen Core WL
  wlError = tizen_core_wl_create_window(mTcoreDisplay, nullptr, positionSize.x, positionSize.y, positionSize.width, positionSize.height, &mTcoreWindow);
  if(wlError != TIZEN_CORE_WL_ERROR_NONE || !mTcoreWindow)
  {
    DALI_LOG_ERROR("Failed to create Tizen Core WL window: %d\n", wlError);
    DALI_ASSERT_ALWAYS(0 && "Failed to create Tizen Core WL window");
  }

  // 3) Get underlying wl_surface from the Tizen Core WL window
  wl_surface* surface = nullptr;
  wlError             = tizen_core_wl_window_private_get_wl_surface(mTcoreWindow, &surface);
  if(wlError != TIZEN_CORE_WL_ERROR_NONE || !surface)
  {
    DALI_LOG_ERROR("Failed to get wl_surface from Tizen Core WL window: %d\n", wlError);
    DALI_ASSERT_ALWAYS(0 && "Failed to get wl_surface from Tizen Core WL window");
  }

  mWlSurface = surface;
}

void WindowBaseTcoreWl::SetParent(WindowBase* parentWinBase, bool belowParent)
{
  if(mTcoreWindow)
  {
    tizen_core_wl_window_h parentWindow = nullptr;
    if(parentWinBase)
    {
      WindowBaseTcoreWl* baseTcoreWl = static_cast<WindowBaseTcoreWl*>(parentWinBase);
      parentWindow                   = baseTcoreWl->mTcoreWindow;
    }
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_transient_for_parent_window");
    tizen_core_wl_window_set_transient_for_parent_window(mTcoreWindow, parentWindow, belowParent);
  }
}

int WindowBaseTcoreWl::CreateFrameRenderedSyncFence()
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "wl_egl_window_tizen_create_commit_sync_fd");
  return wl_egl_window_tizen_create_commit_sync_fd(mEglWindow);
}

int WindowBaseTcoreWl::CreateFramePresentedSyncFence()
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "wl_egl_window_tizen_create_presentation_sync_fd");
  return wl_egl_window_tizen_create_presentation_sync_fd(mEglWindow);
}

void WindowBaseTcoreWl::SetPositionSizeWithAngle(PositionSize positionSize, int angle)
{
  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::SetPositionSizeWithAngle, angle: %d, x: %d, y: %d, w: %d, h: %d\n", angle, positionSize.x, positionSize.y, positionSize.width, positionSize.height);
  mWindowRotationAngle = angle;
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_geometry_hint_of_rotation_angle");
    tizen_core_wl_error_e err = tizen_core_wl_window_set_geometry_hint_of_rotation_angle(mTcoreWindow, static_cast<tizen_core_wl_window_angle_e>(angle), positionSize.x, positionSize.y, positionSize.width, positionSize.height);
    if(err != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::SetPositionSizeWithAngle: Error! [%d]\n", err);
    }
    tizen_core_wl_window_commit(mTcoreWindow);
  }
}

void WindowBaseTcoreWl::InitializeIme()
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::InitializeIme");
  bool createdDisplay = false;

  if(mIsIMEWindowInitialized)
  {
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::InitializeIme, IME Window is already initialized\n");
    return;
  }

  if(!mTcoreDisplay)
  {
    if(tizen_core_wl_display_create(&mTcoreDisplay) != TIZEN_CORE_WL_ERROR_NONE || !mTcoreDisplay)
    {
      DALI_LOG_ERROR("WindowBaseTcoreWl::InitializeIme, fail to create tizen core wl display\n");
      return;
    }

    if(tizen_core_wl_display_connect(mTcoreDisplay, nullptr) != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_ERROR("WindowBaseTcoreWl::InitializeIme, fail to connect tizen core wl display\n");
      tizen_core_wl_display_destroy(mTcoreDisplay);
      mTcoreDisplay = nullptr;
      return;
    }
    createdDisplay = true;
  }

  if(tizen_core_wl_window_private_get_wl_surface(mTcoreWindow, &mWlSurface) != TIZEN_CORE_WL_ERROR_NONE || !mWlSurface)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::InitializeIme, fail to get wl_surface from tizen core wl window\n");
    if(createdDisplay)
    {
      DisconnectAndDestroyDisplay(mTcoreDisplay);
    }
    return;
  }

  // Bind zwp_input_panel_v1 intferface using tizen-core-wayland API
  tizen_core_wl_error_e err = tizen_core_wl_display_private_bind_interface(mTcoreDisplay, "zwp_input_panel_v1", 1, &zwp_input_panel_v1_interface, (void**)&mWlInputPanel);
  if(err != TIZEN_CORE_WL_ERROR_NONE || !mWlInputPanel)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::InitializeIme, fail to bind zwp_input_panel_v1 interface\n");
    if(createdDisplay)
    {
      DisconnectAndDestroyDisplay(mTcoreDisplay);
    }
    return;
  }

  // Bind wl_output intferface using tizen-core-wayland API
  err = tizen_core_wl_display_private_bind_interface(mTcoreDisplay, "wl_output", 1, &wl_output_interface, (void**)&mWlOutput);
  if(err != TIZEN_CORE_WL_ERROR_NONE || !mWlOutput)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::InitializeIme, fail to bind wl_output interface\n");
    if(createdDisplay)
    {
      DisconnectAndDestroyDisplay(mTcoreDisplay);
    }
    return;
  }

  // Set Window type to NONE
  err = tizen_core_wl_window_set_type(mTcoreWindow, TIZEN_CORE_WL_WINDOW_TYPE_NONE);
  if(err != TIZEN_CORE_WL_ERROR_NONE)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::InitializeIme, fail to set window type\n");
    if(createdDisplay)
    {
      DisconnectAndDestroyDisplay(mTcoreDisplay);
    }
    return;
  }

  // Get input panel surface
  mWlInputPanelSurface = zwp_input_panel_v1_get_input_panel_surface(mWlInputPanel, mWlSurface);
  if(!mWlInputPanelSurface)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::InitializeIme, fail to get input panel surface\n");
    if(createdDisplay)
    {
      DisconnectAndDestroyDisplay(mTcoreDisplay);
    }
    return;
  }

  // Set input panel to bottom center position
  zwp_input_panel_surface_v1_set_toplevel(mWlInputPanelSurface, mWlOutput, ZWP_INPUT_PANEL_SURFACE_V1_POSITION_CENTER_BOTTOM);

  mIsIMEWindowInitialized = true;
}

void WindowBaseTcoreWl::ImeWindowReadyToRender()
{
  if(!mWlInputPanelSurface)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::ImeWindowReadyToRender(), wayland input panel surface is null\n");
    return;
  }

  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "zwp_input_panel_surface_v1_set_ready");
  zwp_input_panel_surface_v1_set_ready(mWlInputPanelSurface, 1);
}

void WindowBaseTcoreWl::RequestMoveToServer()
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::RequestMoveToServer");
  if(!mTcoreWindow || !mTcoreDisplay)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::RequestMoveToServer, invalid window or display\n");
    return;
  }

  tizen_core_wl_seat_h seat = nullptr;
  if(tizen_core_wl_display_get_default_seat(mTcoreDisplay, &seat) != TIZEN_CORE_WL_ERROR_NONE || !seat)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::RequestMoveToServer, Fail to get default seat\n");
    return;
  }

  DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
  tizen_core_wl_window_start_drag_move(mTcoreWindow, seat);
  DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "tizen_core_wl_window_start_drag_move");
  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::RequestMoveToServer, starts the window[%p] is moved by server\n", mTcoreWindow);
}

void WindowBaseTcoreWl::RequestResizeToServer(WindowResizeDirection direction)
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "WindowBaseTcoreWl::RequestResizeToServer");
  if(!mTcoreWindow || !mTcoreDisplay)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::RequestResizeToServer, invalid window or display\n");
    return;
  }

  tizen_core_wl_seat_h seat = nullptr;
  if(tizen_core_wl_display_get_default_seat(mTcoreDisplay, &seat) != TIZEN_CORE_WL_ERROR_NONE || !seat)
  {
    DALI_LOG_ERROR("WindowBaseTcoreWl::RequestResizeToServer, Fail to get default seat\n");
    return;
  }

  ResizeLocation                     location   = RecalculateLocationToCurrentOrientation(direction, mWindowRotationAngle);
  tizen_core_wl_window_resize_mode_e resizeMode = TIZEN_CORE_WL_WINDOW_RESIZE_MODE_TOP_LEFT;
  switch(location)
  {
    case ResizeLocation::TOP:
      resizeMode = TIZEN_CORE_WL_WINDOW_RESIZE_MODE_TOP;
      break;
    case ResizeLocation::BOTTOM:
      resizeMode = TIZEN_CORE_WL_WINDOW_RESIZE_MODE_BOTTOM;
      break;
    case ResizeLocation::LEFT:
      resizeMode = TIZEN_CORE_WL_WINDOW_RESIZE_MODE_LEFT;
      break;
    case ResizeLocation::TOP_LEFT:
      resizeMode = TIZEN_CORE_WL_WINDOW_RESIZE_MODE_TOP_LEFT;
      break;
    case ResizeLocation::BOTTOM_LEFT:
      resizeMode = TIZEN_CORE_WL_WINDOW_RESIZE_MODE_BOTTOM_LEFT;
      break;
    case ResizeLocation::RIGHT:
      resizeMode = TIZEN_CORE_WL_WINDOW_RESIZE_MODE_RIGHT;
      break;
    case ResizeLocation::TOP_RIGHT:
      resizeMode = TIZEN_CORE_WL_WINDOW_RESIZE_MODE_TOP_RIGHT;
      break;
    case ResizeLocation::BOTTOM_RIGHT:
      resizeMode = TIZEN_CORE_WL_WINDOW_RESIZE_MODE_BOTTOM_RIGHT;
      break;
    default:
      break;
  }

  DALI_TIME_CHECKER_BEGIN(gTimeCheckerFilter);
  tizen_core_wl_window_start_drag_resize(mTcoreWindow, seat, resizeMode);
  DALI_TIME_CHECKER_END_WITH_MESSAGE(gTimeCheckerFilter, "tizen_core_wl_window_start_drag_resize");
  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::RequestResizeToServer, starts the window[%p] is resized by server, direction:%d oriention:%d mode:%d\n", mTcoreWindow, direction, mWindowRotationAngle, location);
}

void WindowBaseTcoreWl::EnableFloatingMode(bool enable)
{
  DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::EnableFloatingMode, floating mode flag: [%p], enable [%d]\n", mTcoreWindow, enable);
  tizen_core_wl_window_type_e windowType;
  if(mTcoreWindow)
  {
    // Tizen Core WL has no floating_mode equivalent
    if(enable)
    {
      mIsFloating = true;
      windowType  = TIZEN_CORE_WL_WINDOW_TYPE_FLOATING;
    }
    else
    {
      mIsFloating = false;
      switch(mType)
      {
        case Dali::WindowType::NORMAL:
        {
          DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::EnableFloatingMode, Dali::WindowType::NORMAL\n");
          windowType = TIZEN_CORE_WL_WINDOW_TYPE_TOPLEVEL;
          break;
        }
        case Dali::WindowType::NOTIFICATION:
        {
          DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::EnableFloatingMode, Dali::WindowType::NOTIFICATION\n");
          windowType = TIZEN_CORE_WL_WINDOW_TYPE_NOTIFICATION;
          break;
        }
        case Dali::WindowType::UTILITY:
        {
          DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::EnableFloatingMode, Dali::WindowType::UTILITY\n");
          windowType = TIZEN_CORE_WL_WINDOW_TYPE_UTILITY;
          break;
        }
        case Dali::WindowType::DIALOG:
        {
          DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::EnableFloatingMode, Dali::WindowType::DIALOG\n");
          windowType = TIZEN_CORE_WL_WINDOW_TYPE_DIALOG;
          break;
        }
        case Dali::WindowType::IME:
        {
          DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::EnableFloatingMode, Dali::WindowType::IME\n");
          windowType = TIZEN_CORE_WL_WINDOW_TYPE_NONE;
          break;
        }
        case Dali::WindowType::DESKTOP:
        {
          DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::EnableFloatingMode, Dali::WindowType::DESKTOP\n");
          windowType = TIZEN_CORE_WL_WINDOW_TYPE_DESKTOP;
          break;
        }
        default:
        {
          DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::EnableFloatingMode, default window type\n");
          windowType = TIZEN_CORE_WL_WINDOW_TYPE_TOPLEVEL;
          break;
        }
      }
    }
    tizen_core_wl_window_set_type(mTcoreWindow, windowType);
  }
}

bool WindowBaseTcoreWl::IsFloatingModeEnabled() const
{
  if(mTcoreWindow)
  {
    // Tizen Core WL has no floating_mode equivalent
    return mIsFloating;
  }
  return false;
}

void WindowBaseTcoreWl::IncludeInputRegion(const Rect<int>& inputRegion)
{
  Rect<int> convertRegion = RecalculateInputRect(inputRegion, mWindowPositionSize);

  if(mTcoreWindow && mTcoreDisplay)
  {
    if(!mDisplayRegion)
    {
      tizen_core_wl_error_e err = tizen_core_wl_display_create_region(mTcoreDisplay, &mDisplayRegion);
      if(err != TIZEN_CORE_WL_ERROR_NONE || !mDisplayRegion)
      {
        DALI_LOG_ERROR("Failed to create input region\n");
        mDisplayRegion = nullptr;
        return;
      }
    }

    tizen_core_wl_rect_s rect;
    rect.x = convertRegion.x;
    rect.y = convertRegion.y;
    rect.w = convertRegion.width;
    rect.h = convertRegion.height;

    tizen_core_wl_error_e err = tizen_core_wl_region_add_rect(mDisplayRegion, rect);
    if(err != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_ERROR("Failed to add rect to input region\n");
      return;
    }

    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_input_region");
    tizen_core_wl_window_set_input_region(mTcoreWindow, mDisplayRegion);
    tizen_core_wl_window_commit(mTcoreWindow);
  }
}

void WindowBaseTcoreWl::ExcludeInputRegion(const Rect<int>& inputRegion)
{
  Rect<int> convertRegion = RecalculateInputRect(inputRegion, mWindowPositionSize);

  if(mTcoreWindow && mTcoreDisplay && mDisplayRegion)
  {
    tizen_core_wl_rect_s rect;
    rect.x = convertRegion.x;
    rect.y = convertRegion.y;
    rect.w = convertRegion.width;
    rect.h = convertRegion.height;

    tizen_core_wl_error_e err = tizen_core_wl_region_subtract_rect(mDisplayRegion, rect);
    if(err != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_ERROR("Failed to subtract rect from input region\n");
      return;
    }

    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_input_region");
    tizen_core_wl_window_set_input_region(mTcoreWindow, mDisplayRegion);
    tizen_core_wl_window_commit(mTcoreWindow);
  }
}

bool WindowBaseTcoreWl::PointerConstraintsLock()
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_lock_pointer");

    tizen_core_wl_error_e ret = tizen_core_wl_window_lock_pointer(mTcoreWindow);
    if(ret != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::PointerConstraintsLock: Error! [%d]\n", ret);
      return false;
    }
    return true;
  }
  return false;
}

bool WindowBaseTcoreWl::PointerConstraintsUnlock()
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_unlock_pointer");

    tizen_core_wl_error_e ret = tizen_core_wl_window_unlock_pointer(mTcoreWindow);
    if(ret != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::PointerConstraintsUnlock: Error! [%d]\n", ret);
      return false;
    }
    return true;
  }
  return false;
}

void WindowBaseTcoreWl::LockedPointerRegionSet(int32_t x, int32_t y, int32_t width, int32_t height)
{
  if(mTcoreWindow && mTcoreDisplay)
  {
    tizen_core_wl_region_h region = NULL;
    tizen_core_wl_error_e  ret    = tizen_core_wl_display_create_region(mTcoreDisplay, &region);
    if(ret != TIZEN_CORE_WL_ERROR_NONE)
    {
      return;
    }

    tizen_core_wl_rect_s rect = {x, y, width, height};
    ret                       = tizen_core_wl_region_add_rect(region, rect);
    if(ret != TIZEN_CORE_WL_ERROR_NONE)
    {
      return;
    }

    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_locked_pointer_set_region");
    ret = tizen_core_wl_window_locked_pointer_set_region(mTcoreWindow, region);
    if(ret != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::LockedPointerRegionSet: Error! [%d]\n", ret);
    }
  }
}

void WindowBaseTcoreWl::LockedPointerCursorPositionHintSet(int32_t x, int32_t y)
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_locked_pointer_set_cursor_position_hint");
    tizen_core_wl_error_e ret = tizen_core_wl_window_locked_pointer_set_cursor_position_hint(mTcoreWindow, x, y);
    if(ret != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_INFO(gWindowBaseLogFilter, Debug::Verbose, "WindowBaseTcoreWl::LockedPointerCursorPositionHintSet: Error! [%d]\n", ret);
    }
  }
}

bool WindowBaseTcoreWl::PointerWarp(int32_t x, int32_t y)
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_warp_pointer");
    return (tizen_core_wl_window_warp_pointer(mTcoreWindow, x, y) == TIZEN_CORE_WL_ERROR_NONE);
  }
  return false;
}

void WindowBaseTcoreWl::CursorVisibleSet(bool visible)
{
  if(mTcoreWindow && mTcoreDisplay)
  {
    tizen_core_wl_seat_h seat = nullptr;
    if(tizen_core_wl_display_get_default_seat(mTcoreDisplay, &seat) == TIZEN_CORE_WL_ERROR_NONE && seat)
    {
      DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_seat_set_cursor_visible");
      tizen_core_wl_seat_set_cursor_visible(seat, visible);
    }
  }
}

// Request grab key events according to the requested device subtype
bool WindowBaseTcoreWl::KeyboardGrab(Device::Subclass::Type deviceSubclass)
{
  if(mTcoreWindow)
  {
    tizen_core_wl_device_subclass_e subtype;
    switch(deviceSubclass)
    {
      case Device::Subclass::NONE:
      {
        subtype = TIZEN_CORE_WL_DEVICE_SUBCLASS_NONE;
        break;
      }
      case Device::Subclass::REMOCON:
      {
        subtype = TIZEN_CORE_WL_DEVICE_SUBCLASS_REMOCON;
        break;
      }
      case Device::Subclass::VIRTUAL_KEYBOARD:
      {
        subtype = TIZEN_CORE_WL_DEVICE_SUBCLASS_VIRTUAL_KEYBOARD;
        break;
      }
      default:
      {
        DALI_LOG_ERROR("deviceSubclass : %d type is not support, subtype could be 'NONE', 'REMOCON', 'VIRTUAL_KEYBOARD'\n");
        return false;
      }
    }
    return (tizen_core_wl_window_grab_keyboard(mTcoreWindow, subtype) == TIZEN_CORE_WL_ERROR_NONE);
  }
  return false;
}

// Request ungrab key events
bool WindowBaseTcoreWl::KeyboardUnGrab()
{
  if(mTcoreWindow)
  {
    return (tizen_core_wl_window_ungrab_keyboard(mTcoreWindow) == TIZEN_CORE_WL_ERROR_NONE);
  }
  return false;
}

void WindowBaseTcoreWl::SetFullScreen(bool fullscreen)
{
  DALI_LOG_RELEASE_INFO("SetFullScreen, window: [%p], fullscreen [%d]\n", mTcoreWindow, fullscreen);
  if(mTcoreWindow)
  {
    mPendingRestoreResizeOnUnmaximize = !fullscreen;

    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_state");
    tizen_core_wl_window_set_state(mTcoreWindow, fullscreen ? TIZEN_CORE_WL_WINDOW_STATE_FULLSCREEN : TIZEN_CORE_WL_WINDOW_STATE_NONE);
  }
}

bool WindowBaseTcoreWl::GetFullScreen()
{
  if(mTcoreWindow)
  {
    tizen_core_wl_window_state_e state = TIZEN_CORE_WL_WINDOW_STATE_NONE;
    return (tizen_core_wl_window_get_state(mTcoreWindow, &state) == TIZEN_CORE_WL_ERROR_NONE &&
            state == TIZEN_CORE_WL_WINDOW_STATE_FULLSCREEN);
  }
  return false;
}

void WindowBaseTcoreWl::SetFrontBufferRenderingEnabled(bool enable)
{
  mIsFrontBufferRendering = enable;
}

bool WindowBaseTcoreWl::IsFrontBufferRenderingEnabled() const
{
  return mIsFrontBufferRendering;
}

void WindowBaseTcoreWl::SetWindowFrontBufferMode(bool enable)
{
  DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "wl_egl_window_tizen_set_frontbuffer_mode");
  wl_egl_window_tizen_set_frontbuffer_mode(mEglWindow, enable);
}

void WindowBaseTcoreWl::SetModal(bool modal)
{
  DALI_LOG_RELEASE_INFO("SetModal, window: [%p], flag [%d]\n", mTcoreWindow, modal);
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_modal");
    if(tizen_core_wl_window_set_modal(mTcoreWindow, modal) != TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::SetModal: tizen_core_wl_window_set_modal failed\n");
      return;
    }
    tizen_core_wl_window_commit(mTcoreWindow);
  }
}

bool WindowBaseTcoreWl::IsModal()
{
  if(mTcoreWindow)
  {
    bool modal = false;
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_get_modal");
    if(tizen_core_wl_window_get_modal(mTcoreWindow, &modal) == TIZEN_CORE_WL_ERROR_NONE)
    {
      DALI_LOG_RELEASE_INFO("tizen_core_wl_window_get_modal, window: [%p], flag [%d]\n", mTcoreWindow, modal ? 1 : 0);
      return modal;
    }
    DALI_LOG_RELEASE_INFO("WindowBaseTcoreWl::IsModal: tizen_core_wl_window_get_modal failed\n");
  }
  return false;
}

void WindowBaseTcoreWl::SetAlwaysOnTop(bool alwaysOnTop)
{
  DALI_LOG_RELEASE_INFO("SetAlwaysOnTop, window: [%p], flag [%d]\n", mTcoreWindow, alwaysOnTop);
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_stack_mode");
    mIsAlwaysTop = alwaysOnTop;
    tizen_core_wl_window_set_pin_mode(mTcoreWindow, alwaysOnTop);
    tizen_core_wl_window_commit(mTcoreWindow);
  }
}

bool WindowBaseTcoreWl::IsAlwaysOnTop() const
{
  if(mTcoreWindow)
  {
    // Tizen Core WL has no pin_mode_get; assume false
    return mIsAlwaysTop;
  }
  return false;
}

void WindowBaseTcoreWl::SetBottom(bool enable)
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_stack_mode");
    mIsBottom = enable;
    tizen_core_wl_window_set_stack_mode(mTcoreWindow, mIsBottom ? TIZEN_CORE_WL_WINDOW_STACK_MODE_BELOW : TIZEN_CORE_WL_WINDOW_STACK_MODE_NONE);
    tizen_core_wl_window_commit(mTcoreWindow);
  }
}

bool WindowBaseTcoreWl::IsBottom()
{
  return mIsBottom;
}

Any WindowBaseTcoreWl::GetNativeBuffer() const
{
  DALI_LOG_RELEASE_INFO("Get wl_egl_window, tcore_window: [%p], wl_egl_window [%p]\n", mTcoreWindow, mEglWindow);
  return mEglWindow;
}

bool WindowBaseTcoreWl::RelativeMotionGrab(uint32_t boundary)
{
  if(mTcoreWindow)
  {
    tizen_core_wl_pointer_boundary_e wlBoundary = static_cast<tizen_core_wl_pointer_boundary_e>(boundary);

    return (tizen_core_wl_window_grab_pointer_relative_move(mTcoreWindow, wlBoundary) == TIZEN_CORE_WL_ERROR_NONE);
  }
  return false;
}

bool WindowBaseTcoreWl::RelativeMotionUnGrab()
{
  if(mTcoreWindow)
  {
    return (tizen_core_wl_window_ungrab_pointer_relative_move(mTcoreWindow) == TIZEN_CORE_WL_ERROR_NONE);
  }
  return false;
}

void WindowBaseTcoreWl::SetBackgroundBlur(int blurRadius, int cornerRadius)
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_background_blur");
    DALI_LOG_RELEASE_INFO("tizen_core_wl_window_set_background_blur, window: [%p], blur radius [%d], corner radius [%d]\n", mTcoreWindow, blurRadius, cornerRadius);
    tizen_core_wl_window_set_background_blur(mTcoreWindow, blurRadius, cornerRadius, cornerRadius);
  }
}

int WindowBaseTcoreWl::GetBackgroundBlur()
{
  // It is not needed
  return 0;
}

void WindowBaseTcoreWl::SetBehindBlur(int blurRadius)
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_behind_blur");
    DALI_LOG_RELEASE_INFO("tizen_core_wl_window_set_behind_blur, window: [%p], blur radius [%d]\n", mTcoreWindow, blurRadius);
    tizen_core_wl_window_set_behind_blur(mTcoreWindow, blurRadius);
  }
}

int WindowBaseTcoreWl::GetBehindBlur()
{
  // It is not needed
  return 0;
}

void WindowBaseTcoreWl::SetBehindBlurDim(bool enable, Vector4& color)
{
  if(mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_behind_dim");
    DALI_LOG_RELEASE_INFO("tizen_core_wl_window_set_behind_dim, window: [%p], enable [%d], DALI dim color[%f, %f, %f, %f]\n", mTcoreWindow, enable, color.r, color.g, color.b, color.a);
    int convertR = static_cast<int>(color.r * 255.0f);
    int convertG = static_cast<int>(color.g * 255.0f);
    int convertB = static_cast<int>(color.b * 255.0f);
    int convertA = static_cast<int>(color.a * 255.0f);
    DALI_LOG_RELEASE_INFO("tizen_core_wl_window_set_behind_dim, window: [%p], enable [%d], rgba [%d,%d,%d,%d]\n", mTcoreWindow, static_cast<int>(enable), convertR, convertG, convertB, convertA);
    tizen_core_wl_window_set_behind_dim(mTcoreWindow, static_cast<int>(enable), convertR, convertG, convertB, convertA);
  }
}

Vector4 WindowBaseTcoreWl::GetBehindBlurDim(bool& enable)
{
  // It is not needed
  return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
}

Extents WindowBaseTcoreWl::GetInsets()
{
  return GetInsets(WindowInsetsPartFlags::STATUS_BAR | WindowInsetsPartFlags::KEYBOARD | WindowInsetsPartFlags::CLIPBOARD);
}

Extents WindowBaseTcoreWl::GetInsets(WindowInsetsPartFlags insetsFlags)
{
  int left   = 0;
  int right  = 0;
  int top    = 0;
  int bottom = 0;

  int winX = mWindowPositionSize.x;
  int winY = mWindowPositionSize.y;
  int winW = mWindowPositionSize.width;
  int winH = mWindowPositionSize.height;

  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;

  if(insetsFlags == WindowInsetsPartFlags::NONE)
  {
    return Extents(left, right, top, bottom);
  }

  for(int i = 0; i < 3; i++)
  {
    if(i == 0)
    {
      if(!HasFlag(insetsFlags, WindowInsetsPartFlags::STATUS_BAR) || mLastIndicatorGeometry.state == 0)
      {
        continue;
      }
      x = mLastIndicatorGeometry.x;
      y = mLastIndicatorGeometry.y;
      w = mLastIndicatorGeometry.w;
      h = mLastIndicatorGeometry.h;
    }
    else if(i == 1)
    {
      if(!HasFlag(insetsFlags, WindowInsetsPartFlags::KEYBOARD) || mLastKeyboardGeometry.state == 0)
      {
        continue;
      }
      x = mLastKeyboardGeometry.x;
      y = mLastKeyboardGeometry.y;
      w = mLastKeyboardGeometry.w;
      h = mLastKeyboardGeometry.h;
    }
    else
    {
      if(!HasFlag(insetsFlags, WindowInsetsPartFlags::CLIPBOARD) || mLastClipboardGeometry.state == 0)
      {
        continue;
      }
      x = mLastClipboardGeometry.x;
      y = mLastClipboardGeometry.y;
      w = mLastClipboardGeometry.w;
      h = mLastClipboardGeometry.h;
    }

    if((x <= winX) && (x + w >= winX + winW))
    {
      if((y <= winY) && (y + h >= winY) && (y + h <= winY + winH))
      {
        top = y + h - winY;
        winY += top;
        winH -= top;
      }
      else if((y + h >= winY + winH) && (y >= winY) && (y <= winY + winH))
      {
        bottom = winY + winH - y;
        winH -= bottom;
      }
    }
    else if((y <= winY) && (y + h >= winY + winH))
    {
      if((x <= winX) && (x + w >= winX) && (x + w <= winX + winW))
      {
        left = x + w - winX;
        winX += left;
        winW -= left;
      }
      else if((x + w >= winX + winW) && (x >= winX) && (x <= winX + winW))
      {
        right = winX + winW - x;
        winW -= right;
      }
    }
  }

  return Extents(left, right, top, bottom);
}

void WindowBaseTcoreWl::SetScreen(const std::string& screenName)
{
  tizen_core_wl_screen_h* screenList = nullptr;
  int                     count      = 0;
  if(mTcoreDisplay && mTcoreWindow)
  {
    DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_display_get_screen_list");
    if(tizen_core_wl_display_get_screen_list(mTcoreDisplay, &screenList, &count) == TIZEN_CORE_WL_ERROR_NONE && screenList && count > 0)
    {
      for(int i = 0; i < count; i++)
      {
        char* tempScreenName = NULL;
        if(tizen_core_wl_screen_get_name(screenList[i], &tempScreenName) == TIZEN_CORE_WL_ERROR_NONE)
        {
          if(tempScreenName && screenName.compare(tempScreenName) == 0)
          {
            DALI_LOG_RELEASE_INFO("Get tizen_core_wl_window_set_screen, tcore_window: [%p], screen name [%s]\n", mTcoreWindow, tempScreenName);
            mScreen = screenList[i];
            DALI_TIME_CHECKER_SCOPE(gTimeCheckerFilter, "tizen_core_wl_window_set_screen");
            tizen_core_wl_window_set_screen(mTcoreWindow, mScreen);
            break;
          }
        }
        if(tempScreenName)
        {
          free(tempScreenName);
        }
      }
      if(screenList)
      {
        free(screenList);
      }
    }
  }
}

std::string WindowBaseTcoreWl::GetScreen() const
{
  char*       currentScreenName = NULL;
  std::string screenName{};
  if(tizen_core_wl_screen_get_name(mScreen, &currentScreenName) == TIZEN_CORE_WL_ERROR_NONE)
  {
    if(currentScreenName)
    {
      screenName = currentScreenName;
      free(currentScreenName);
    }
  }

  return screenName;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
