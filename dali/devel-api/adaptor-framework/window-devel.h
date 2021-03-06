#ifndef DALI_WINDOW_DEVEL_H
#define DALI_WINDOW_DEVEL_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <memory>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/common/vector-wrapper.h>

namespace Dali
{
class KeyEvent;
class TouchEvent;
class WheelEvent;
class RenderTaskList;

namespace DevelWindow
{
typedef Signal<void()> EventProcessingFinishedSignalType; ///< Event Processing finished signal type

typedef Signal<void(const KeyEvent&)> KeyEventSignalType; ///< Key event signal type

typedef Signal<void(const TouchEvent&)> TouchEventSignalType; ///< Touch signal type

typedef Signal<void(const WheelEvent&)> WheelEventSignalType; ///< Touched signal type

typedef Signal<void(Window, bool)> VisibilityChangedSignalType; ///< Visibility changed signal type

typedef Signal<void(Window, WindowEffectState, WindowEffectType)> TransitionEffectEventSignalType; ///< Effect signal type and state

typedef Signal<void()> KeyboardRepeatSettingsChangedSignalType; ///< Keyboard repeat settings changed signal type

/**
 * @brief Creates an initialized handle to a new Window.
 *
 * @param[in] surface Can be a window or pixmap.
 * @param[in] windowPosition The position and size of the Window
 * @param[in] name The Window title
 * @param[in] isTransparent Whether Window is transparent
 * @return A new window
 * @note This creates an extra window in addition to the default main window
*/
DALI_ADAPTOR_API Window New(Any surface, PositionSize windowPosition, const std::string& name, bool isTransparent = false);

/**
 * @brief Creates an initialized handle to a new Window.
 *
 * @param[in] surface Can be a window or pixmap.
 * @param[in] windowPosition The position and size of the Window
 * @param[in] name The Window title
 * @param[in] className The Window class name
 * @param[in] isTransparent Whether Window is transparent
 * @note This creates an extra window in addition to the default main window
 * @return A new Window
 */
DALI_ADAPTOR_API Window New(Any surface, PositionSize windowPosition, const std::string& name, const std::string& className, bool isTransparent = false);

/**
 * @brief Sets position and size of the window. This API guarantees that both moving and resizing of window will appear on the screen at once.
 *
 * @param[in] window The window instance
 * @param[in] positionSize The new window position and size
 */
DALI_ADAPTOR_API void SetPositionSize(Window window, PositionSize positionSize);

/**
 * @brief Retrieve the window that the given actor is added to.
 *
 * @param[in] actor The actor
 * @return The window the actor is added to or an empty handle if the actor is not added to any window.
 */
DALI_ADAPTOR_API Window Get(Actor actor);

/**
 * @brief This signal is emitted just after the event processing is finished.
 *
 * @param[in] window The window instance
 * @return The signal to connect to
 */
DALI_ADAPTOR_API EventProcessingFinishedSignalType& EventProcessingFinishedSignal(Window window);

/**
 * @brief This signal is emitted when wheel event is received.
 *
 * A callback of the following type may be connected:
 * @code
 *   void YourCallbackName(const WheelEvent& event);
 * @endcode
 * @param[in] window The window instance
 * @return The signal to connect to
 */
DALI_ADAPTOR_API WheelEventSignalType& WheelEventSignal(Window window);

/**
 * @brief This signal is emitted when the window is shown or hidden.
 *
 * A callback of the following type may be connected:
 * @code
 *   void YourCallbackName( Window window, bool visible );
 * @endcode
 * @param[in] window The window instance
 * @return The signal to connect to
 */
DALI_ADAPTOR_API VisibilityChangedSignalType& VisibilityChangedSignal(Window window);

/**
 * @brief This signal is emitted for transition effect.
 *
 * The transition animation is appeared when the window is shown/hidden.
 * When the animation is started, START signal is emitted.
 * Then the animation is ended, END signal is emitted, too.
 * A callback of the following type may be connected:
 * @code
 *   void YourCallbackName( Window window, EffectState state, EffectType type );
 * @endcode
 * @param[in] window The window instance
 * @return The signal to connect to
 */
DALI_ADAPTOR_API TransitionEffectEventSignalType& TransitionEffectEventSignal(Window window);

/**
 * @brief This signal is emitted just after the keyboard repeat setting is changed globally.
 *
 * @param[in] window The window instance
 * @return The signal to connect to
 */
DALI_ADAPTOR_API KeyboardRepeatSettingsChangedSignalType& KeyboardRepeatSettingsChangedSignal(Window window);

/**
 * @brief Sets parent window of the window.
 *
 * After setting that, these windows do together when raise-up, lower and iconified/deiconified.
 * Initially, the window is located on top of the parent. The window can go below parent by calling Lower().
 * If parent's window stack is changed by calling Raise() or Lower(), child windows are located on top of the parent again.
 *
 * @param[in] window The window instance
 * @param[in] parent The parent window instance
 */
DALI_ADAPTOR_API void SetParent(Window window, Window parent);

/**
 * @brief Unsets parent window of the window.
 *
 * After unsetting, the window is disconnected his parent window.
 *
 * @param[in] window The window instance
 */
DALI_ADAPTOR_API void Unparent(Window window);

/**
 * @brief Gets parent window of the window.
 *
 * @param[in] window The window instance
 * @return The parent window of the window
 */
DALI_ADAPTOR_API Window GetParent(Window window);

/**
 * @brief Downcast sceneHolder to window
 *
 * @param[in] handle The handle need to downcast
 * @return The window cast from SceneHolder
 */
DALI_ADAPTOR_API Window DownCast(BaseHandle handle);

/**
 * @brief Gets current orientation of the window.
 *
 * @param[in] window The window instance
 * @return The current window orientation if previously set, or none
 */
DALI_ADAPTOR_API WindowOrientation GetCurrentOrientation(Window window);

/**
 * @brief Gets current physical orientation of the window.
 *
 * It means current physical rotation angle of the window.
 * If the height of the display device's area is greater than the width,
 * default current orientation is PORTRAIT and current physical orientation angle is 0.
 * If the width of the display device's area is greater than the height,
 * default current orientation is LANDSCAPE and current physical orientation angle is 0.
 *
 * @param[in] window The window instance
 * @return The current physical orientation degree of the window. It is one of them as 0, 90, 180 and 270.
 */
DALI_ADAPTOR_API int GetPhysicalOrientation(Window window);

/**
 * @brief Sets available orientations of the window.
 *
 * This API is for setting several orientations one time.
 *
 * @param[in] window The window instance
 * @param[in] orientations The available orientation list to add
 */
DALI_ADAPTOR_API void SetAvailableOrientations(Window window, const Dali::Vector<WindowOrientation>& orientations);

/**
 * @brief Gets current window ID.
 *
 * @param[in] window The window instance
 */
DALI_ADAPTOR_API int32_t GetNativeId(Window window);

/**
 * @brief Adds a callback that is called when the frame rendering is done by the graphics driver.
 *
 * @param[in] window The window instance
 * @param[in] callback The function to call
 * @param[in] frameId The Id to specify the frame. It will be passed when the callback is called.
 *
 * @note A callback of the following type may be used:
 * @code
 *   void MyFunction( int frameId );
 * @endcode
 * This callback will be deleted once it is called.
 *
 * @note Ownership of the callback is passed onto this class.
 */
DALI_ADAPTOR_API void AddFrameRenderedCallback(Window window, std::unique_ptr<CallbackBase> callback, int32_t frameId);

/**
 * @brief Adds a callback that is called when the frame is displayed on the display.
 *
 * @param[in] window The window instance
 * @param[in] callback The function to call
 * @param[in] frameId The Id to specify the frame. It will be passed when the callback is called.
 *
 * @note A callback of the following type may be used:
 * @code
 *   void MyFunction( int frameId );
 * @endcode
 * This callback will be deleted once it is called.
 *
 * @note Ownership of the callback is passed onto this class.
 */
DALI_ADAPTOR_API void AddFramePresentedCallback(Window window, std::unique_ptr<CallbackBase> callback, int32_t frameId);

/**
 * @brief Sets window position and size for specific orientation.
 * This api reserves the position and size per orientation to display server.
 * When the device is rotated, the window is moved/resized with the reserved position/size by display server.
 *
 * @param[in] window The window instance
 * @param[in] positionSize The reserved position and size for the orientation
 * @param[in] orientation The orientation
 *
 * @note Currently, it only works when the window's type is WindowType::IME.
 * @note To set WindowType::IME, use Application New(... WindowType type), not Window::SetType().
 * @note This function is only useful in Tizen world.
 */
DALI_ADAPTOR_API void SetPositionSizeWithOrientation(Window window, PositionSize positionSize, WindowOrientation orientation);

/**
 * @brief Requests to display server for the window is moved by display server.
 *
 * This function should be called in mouse down event callback function.
 * After this function is called in mouse down event callback function, the window is moved with mouse move event.
 * When mouse up event happens, the window moved work is finished.
 *
 * @param[in] window The window instance
 */
DALI_ADAPTOR_API void RequestMoveToServer(Window window);

/**
 * @brief Requests to display server for the window is resized by display server.
 *
 * This function should be called in mouse down event callback function.
 * After this function is called in mouse down event callback function, the window is resized with mouse move event.
 * The direction is selected one of eight ways.
 * When mouse up event happens, the window resized work is finished.
 *
 * @param[in] window The window instance
 * @param[in] direction it is indicated the window's side or edge for starting point.
 */
DALI_ADAPTOR_API void RequestResizeToServer(Window window, WindowResizeDirection direction);

/**
 * @brief Enables the floating mode of window.
 *
 * The floating mode is to support making partial size window easliy.
 * It is useful to make popup style window and this window is always upper than the other normal window.
 * In addition, it is easy to change between popup style and normal style window.
 *
 * A special display server(as a Tizen display server) supports this mode.
 *
 * @param[in] window The window instance.
 * @param[in] enable Enable floating mode or not.
 */
DALI_ADAPTOR_API void EnableFloatingMode(Window window, bool enable);

} // namespace DevelWindow

} // namespace Dali

#endif // DALI_WINDOW_DEVEL_H
