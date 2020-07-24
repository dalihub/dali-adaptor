#ifndef DALI_WINDOW_DEVEL_H
#define DALI_WINDOW_DEVEL_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/adaptor-framework/window.h>

namespace Dali
{
struct KeyEvent;
class TouchData;
struct WheelEvent;
class RenderTaskList;

namespace DevelWindow
{
/**
 * @brief Enumeration for transition effect's state.
 */
enum class EffectState
{
  NONE = 0,    ///< None state
  START,       ///< Transition effect is started.
  END          ///< Transition effect is ended.
};

/**
 * @brief Enumeration for transition effect's type.
 */
enum class  EffectType
{
  NONE = 0,    ///< None type
  SHOW,        ///< Window show effect.
  HIDE,        ///< Window hide effect.
};

typedef Signal< void () > EventProcessingFinishedSignalType;       ///< Event Processing finished signal type

typedef Signal< void (const KeyEvent&) > KeyEventSignalType;       ///< Key event signal type

typedef Signal< void (const TouchData&) > TouchSignalType;         ///< Touch signal type

typedef Signal< void (const WheelEvent&) > WheelEventSignalType;   ///< Touched signal type

typedef Signal< void ( Window, bool ) > VisibilityChangedSignalType; ///< Visibility changed signal type

typedef Signal< void (Window, EffectState, EffectType) > TransitionEffectEventSignalType; ///< Effect signal type and state

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
DALI_ADAPTOR_API void SetPositionSize( Window window, PositionSize positionSize );

/**
 * @brief Retrieve the window that the given actor is added to.
 *
 * @param[in] actor The actor
 * @return The window the actor is added to or an empty handle if the actor is not added to any window.
 */
DALI_ADAPTOR_API Window Get( Actor actor );

/**
 * @brief This signal is emitted just after the event processing is finished.
 *
 * @param[in] window The window instance
 * @return The signal to connect to
 */
DALI_ADAPTOR_API EventProcessingFinishedSignalType& EventProcessingFinishedSignal( Window window );

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
DALI_ADAPTOR_API WheelEventSignalType& WheelEventSignal( Window window );

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
DALI_ADAPTOR_API VisibilityChangedSignalType& VisibilityChangedSignal( Window window );

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
DALI_ADAPTOR_API TransitionEffectEventSignalType& TransitionEffectEventSignal( Window window );

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
DALI_ADAPTOR_API void SetParent( Window window, Window parent );

/**
 * @brief Unsets parent window of the window.
 *
 * After unsetting, the window is disconnected his parent window.
 *
 * @param[in] window The window instance
 */
DALI_ADAPTOR_API void Unparent( Window window );

/**
 * @brief Gets parent window of the window.
 *
 * @param[in] window The window instance
 * @return The parent window of the window
 */
DALI_ADAPTOR_API Window GetParent( Window window );

/**
 * @brief Downcast sceneHolder to window
 *
 * @param[in] handle The handle need to downcast
 * @return The window cast from SceneHolder
 */
DALI_ADAPTOR_API Window DownCast(  BaseHandle handle );

/**
 * @brief Gets current orientation of the window.
 *
 * @param[in] window The window instance
 * @return The current window orientation if previously set, or none
 */
DALI_ADAPTOR_API Dali::Window::WindowOrientation GetCurrentOrientation( Window window );

/**
 * @brief Sets available orientations of the window.
 *
 * This API is for setting several orientations one time.
 *
 * @param[in] window The window instance
 * @param[in] orientations The available orientation list to add
 */
DALI_ADAPTOR_API void SetAvailableOrientations( Window window, const Dali::Vector<Dali::Window::WindowOrientation>& orientations );

/**
 * @brief Gets current window ID.
 *
 * @param[in] window The window instance
 */
DALI_ADAPTOR_API int32_t GetNativeId( Window window );

/**
 * @brief Sets damaged areas of the window.
 *
 * This API is for setting static damaged areas of the window for partial update.
 *
 * @param[in] window The window instance
 * @param[in] areas The damaged areas list to set
 */
DALI_ADAPTOR_API void SetDamagedAreas(Window window, std::vector<Dali::Rect<int>>& areas);

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
DALI_ADAPTOR_API void AddFrameRenderedCallback( Window window, std::unique_ptr< CallbackBase > callback, int32_t frameId );

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
DALI_ADAPTOR_API void AddFramePresentedCallback( Window window, std::unique_ptr< CallbackBase > callback, int32_t frameId );

} // namespace DevelWindow

} // namespace Dali

#endif // DALI_WINDOW_DEVEL_H
