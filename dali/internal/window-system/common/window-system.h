#ifndef DALI_INTERNAL_WINDOW_SYSTEM_COMMON_WINDOW_SYSTEM_H
#define DALI_INTERNAL_WINDOW_SYSTEM_COMMON_WINDOW_SYSTEM_H

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

// EXTERNAL_HEADERS
#include <dali/public-api/object/any.h>
#include <cstdint>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace WindowSystem
{
/**
 * @brief Initialize a window system
 */
void Initialize();

/**
 * @brief Shutdown a window system
 */
void Shutdown();

/**
 * @brief Set the DPI of the target screen.
 *
 * @note Multiple screens are not currently supported.
 * @param[in] horizontalDpi The horizontal resolution in DPI.
 * @param[in] verticalDpi The vertical resolution in DPI.
 */
void SetDpi(uint32_t dpiHorizontal, uint32_t dpiVertical);

/**
 * @brief Retrieves the DPI previously set to the target screen.
 *
 * @note Multiple screens are not currently supported.
 * @param[out] horizontalDpi The horizontal resolution in DPI.
 * @param[out] verticalDpi The vertical resolution in DPI.
 */
void GetDpi(uint32_t& dpiHorizontal, uint32_t& dpiVertical);

/**
 * @brief Get the screen size
 */
void GetScreenSize(int32_t& width, int32_t& height);

/**
 * @brief Update the screen size
 * @note The screen size may be updated while application is running. So update the stored size.
 */
void UpdateScreenSize();

/**
 * @copydoc Dali::Keyboard::SetRepeatInfo()
 */
bool SetKeyboardRepeatInfo(float rate, float delay);

/**
 * @copydoc Dali::Keyboard::GetRepeatInfo()
 */
bool GetKeyboardRepeatInfo(float& rate, float& delay);

/**
 * @copydoc Dali::Keyboard::SetHorizontalRepeatInfo()
 */
bool SetKeyboardHorizontalRepeatInfo(float rate, float delay);

/**
 * @copydoc Dali::Keyboard::GetHorizontalRepeatInfo()
 */
bool GetKeyboardHorizontalRepeatInfo(float& rate, float& delay);

/**
 * @copydoc Dali::Keyboard::SetVerticalRepeatInfo()
 */
bool SetKeyboardVerticalRepeatInfo(float rate, float delay);

/**
 * @copydoc Dali::Keyboard::GetVerticalRepeatInfo()
 */
bool GetKeyboardVerticalRepeatInfo(float& rate, float& delay);

/**
 * @brief Sets whether the processes using geometry event propagation touch and hover events.
 *
 * @param[in] enabled True if the processes using geometry event propagation touch and hover events.
 */
void SetGeometryHittestEnabled(bool enabled);

/**
 * @brief Queries whether the scene using geometry event propagation touch and hover events.
 *
 * @return True if the scene using geometry event propagation touch and hover events.
 */
bool IsGeometryHittestEnabled();

} // namespace WindowSystem

/**
 * Base class for window system. The minimal set of events and properties that should be received/set on the window
 */
class WindowSystemBase
{
public:
  /**
   * Event types that can be received from the window manager for a given window
   */
  enum class Event
  {
    PROPERTY_NOTIFY,
    DELETE_REQUEST,
    MOVE_RESIZE_REQUEST,
    FOCUS_IN,
    FOCUS_OUT,
    DAMAGE,
    MOUSE_WHEEL,
    MOUSE_MOVE,
    MOUSE_BUTTON_DOWN,
    MOUSE_BUTTON_UP,
    MOUSE_OUT,
    KEY_DOWN,
    KEY_UP,
    SELECTION_CLEAR,
    SELECTION_NOTIFY,
    CONFIGURE_NOTIFY,
  };

  /**
   * Base ptr for events - implementation can downcast to platform specific event structure
   */
  struct EventBase
  {
  };

  /**
   * Callback function signature. Platform implementation can call generic handler on a given window
   */
  using EventHandlerCallback = bool (*)(void* data, Event eventType, EventBase* event);

  /**
   * Struct to define an event handler in a window implementation
   */
  struct EventHandler
  {
    EventHandlerCallback callback;  ///< User callback.
    void*                data;      ///< user data
    Event                event;     ///< Event the handler is listening to
    int                  handlerId; ///< Id of the handler
  };

  /**
   * @return Get the current display of this application
   */
  virtual Dali::Any GetDisplay() = 0;

  /**
   * Add an event handler to the window system
   * @param event The window system event to listen for
   * @param callback A callback to handle the event
   * @param data User data to pass to the callback
   * @return A handler object that may be passed to DeleteEventHandler.
   *
   * When the callback is executed, if it returns true, then the invoker will stop calling
   * other event handlers that have registered with that event type. If it returns false,
   * then it will continue with other registered handlers.
   */
  virtual EventHandler* AddEventHandler(Event event, EventHandlerCallback callback, void* data) = 0;

  /**
   * Delete an event handler from the window system.
   * @param eventHandler The event handler to delete.
   */
  virtual void DeleteEventHandler(EventHandler* eventHandler) = 0;

  /**
   * Get the screen size for this window system.
   *
   * @param[out] width The width of the screen
   * @param[out] height The height of the screen
   */
  virtual void GetScreenSize(int32_t& width, int32_t& height) = 0;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_WINDOW_SYSTEM_COMMON_WINDOW_SYSTEM_H
