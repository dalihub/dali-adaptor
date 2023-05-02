#ifndef DALI_WINDOW_DEVEL_H
#define DALI_WINDOW_DEVEL_H

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

// EXTERNAL INCLUDES
#include <memory>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/mouse-in-out-event.h>
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/public-api/adaptor-framework/window.h>
#include <dali/public-api/common/vector-wrapper.h>

namespace Dali
{
class KeyEvent;
class TouchEvent;
class WheelEvent;
class RenderTaskList;
struct TouchPoint;

namespace DevelWindow
{
typedef Signal<void()>                                                               EventProcessingFinishedSignalType;       ///< Event Processing finished signal type
typedef Signal<void(const KeyEvent&)>                                                KeyEventSignalType;                      ///< Key event signal type
typedef Signal<void(const TouchEvent&)>                                              TouchEventSignalType;                    ///< Touch signal type
typedef Signal<void(const WheelEvent&)>                                              WheelEventSignalType;                    ///< Wheel signal type
typedef Signal<void(Window, bool)>                                                   VisibilityChangedSignalType;             ///< Visibility changed signal type
typedef Signal<void(Window, WindowEffectState, WindowEffectType)>                    TransitionEffectEventSignalType;         ///< Effect signal type and state
typedef Signal<void()>                                                               KeyboardRepeatSettingsChangedSignalType; ///< Keyboard repeat settings changed signal type
typedef Signal<void(const std::string&, const std::string&, const Property::Array&)> AuxiliaryMessageSignalType;              ///< Auxiliary message signal type
typedef Signal<void(Window, bool)>                                                   AccessibilityHighlightSignalType;        ///< Accessibility Highlight signal type
typedef Signal<bool(const KeyEvent&)>                                                InterceptKeyEventSignalType;             ///< Intercept Key event signal type
typedef Signal<void(Window, Dali::Window::WindowPosition)>                           MovedSignalType;                         ///< Window Moved signal type
typedef Signal<void(Window, Dali::WindowOrientation)>                                OrientationChangedSignalType;            ///< Window orientation changed signal type
typedef Signal<void(Window, const Dali::DevelWindow::MouseInOutEvent&)>              MouseInOutEventSignalType;               ///< MouseInOutEvent signal type

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
 * @brief This signal is emitted when window's auxiliary was changed then display server sent the message.
 *
 * Auxiliary message is sent by display server.
 * When client application added the window's auxiliary hint and if the auxiliary is changed,
 * display server send the auxiliary message.
 * Auxiliary message has the key, value and options.
 *
 * @param[in] window The window instance
 * @return The signal to connect to
 */
DALI_ADAPTOR_API AuxiliaryMessageSignalType& AuxiliaryMessageSignal(Window window);

/**
 * @brief This signal is emitted when the window needs to grab or clear accessibility highlight.
 * The highlight indicates that it is an object to interact with the user regardless of focus.
 * After setting the highlight on the object, you can do things that the object can do, such as
 * giving or losing focus.
 *
 * This signal is emitted by Dali::Accessibility::Component::GrabHighlight
 * and Dali::Accessibility::Component::ClearHighlight
 *
 * A callback of the following type may be connected:
 * @code
 *   void YourCallbackName( Window window, bool highlight );
 * @endcode
 *
 * @param[in] window The window instance
 * @return The signal to connect to
 */
DALI_ADAPTOR_API AccessibilityHighlightSignalType& AccessibilityHighlightSignal(Window window);

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
 * @brief Sets parent window of the window.
 *
 * After setting that, these windows do together when raise-up, lower and iconified/deiconified.
 * This function has the additional flag whether the child is located above or below of the parent.
 *
 * @param[in] window The window instance
 * @param[in] parent The parent window instance
 * @param[in] belowParent The flag is whether the child is located above or below of the parent.
 */
DALI_ADAPTOR_API void SetParent(Window window, Window parent, bool belowParent);

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

/**
 * @brief Includes input region.
 *
 * This function inlcudes input regions.
 * It can be used multiple times and supports multiple regions.
 * It means input region will be extended.
 *
 * This input is related to mouse and touch event.
 * If device has touch screen, this function is useful.
 * Otherwise device does not have that, we can use it after connecting mouse to the device.
 *
 * @param[in] window The window instance.
 * @param[in] inputRegion The added region to accept input events.
 */
DALI_ADAPTOR_API void IncludeInputRegion(Window window, const Rect<int>& inputRegion);

/**
 * @brief Excludes input region.
 *
 * This function excludes input regions.
 * It can be used multiple times and supports multiple regions.
 * It means input region will be reduced.
 * Nofice, should be set input area by IncludeInputRegion() before this function is used.
 *
 * This input is related to mouse and touch event.
 * If device has touch screen, this function is useful.
 * Otherwise device does not have that, we can use it after connecting mouse to the device.
 *
 * @param[in] window The window instance.
 * @param[in] inputRegion The subtracted region to except input events.
 */
DALI_ADAPTOR_API void ExcludeInputRegion(Window window, const Rect<int>& inputRegion);

/**
 * @brief Sets the necessary for window rotation Acknowledgement.
 * After this function called, SendRotationCompletedAcknowledgement() should be called to complete window rotation.
 *
 * This function is supprot that application has the window rotation acknowledgement's control.
 * It means display server waits when application's rotation work is finished.
 * It is useful application has the other rendering engine which works asynchronous.
 * For instance, GlView.
 * It only works on Tizen device.
 *
 * @param[in] window The window instance.
 * @param[in] needAcknowledgement the flag is true if window rotation acknowledge is sent.
 */
DALI_ADAPTOR_API void SetNeedsRotationCompletedAcknowledgement(Window window, bool needAcknowledgement);

/**
 * @brief send the Acknowledgement to complete window rotation.
 * For this function, SetNeedsRotationCompletedAcknowledgement should be already called with true.
 *
 * @param[in] window The window instance.
 */
DALI_ADAPTOR_API void SendRotationCompletedAcknowledgement(Window window);

/**
 * @brief Feed (Send) touch event to window
 * @param[in] window The window instance
 * @param[in] point The touch point
 * @param[in] timeStamp The time stamp
 */
DALI_ADAPTOR_API void FeedTouchPoint(Window window, const Dali::TouchPoint& point, int32_t timeStamp);

/**
 * @brief Feed (Send) wheel event to window
 * @param[in] window The window instance
 * @param[in] wheelEvent The wheel event
 */
DALI_ADAPTOR_API void FeedWheelEvent(Window window, const Dali::WheelEvent& wheelEvent);

/**
 * @brief Feed (Send) key event to window
 * @param[in] window The window instance
 * @param[in] keyEvent The key event holding the key information.
 */
DALI_ADAPTOR_API void FeedKeyEvent(Window window, const Dali::KeyEvent& keyEvent);

/**
 * @brief Maximizes window's size.
 * If this function is called with true, window will be resized with screen size.
 * Otherwise window will be resized with previous size.
 * It is for the window's MAX button in window's border.
 *
 * It is for client application.
 * If window border is supported by display server, it is not necessary.
 *
 * @param[in] window The window instance.
 * @param[in] maximize If window is maximized or unmaximized.
 */
DALI_ADAPTOR_API void Maximize(Window window, bool maximize);

/**
 * @brief Returns whether the window is maximized or not.
 *
 * @param[in] window The window instance.
 * @return True if the window is maximized, false otherwise.
 */
DALI_ADAPTOR_API bool IsMaximized(Window window);

/**
 * @brief Sets window's maximum size.
 *
 * It is to set the maximized size when window is maximized or the window's size is increased by RequestResizeToServer().
 * Although the size is set by this function, window's size can be increased over the limitation by SetPositionSize() or SetSize().
 *
 * After setting, if Maximize() is called, window is resized with the setting size and move the center.
 *
 * @param[in] window The window instance.
 * @param[in] size the maximum size
 */
DALI_ADAPTOR_API void SetMaximumSize(Window window, Dali::Window::WindowSize size);

/**
 * @brief Minimizes window's size.
 * If this function is called with true, window will be iconified.
 * Otherwise window will be activated.
 * It is for the window's MIN button in window border.
 *
 * It is for client application.
 * If window border is supported by display server, it is not necessary.
 *
 * @param[in] window The window instance.
 * @param[in] minimize If window is minimized or unminimized(activated).
 */
DALI_ADAPTOR_API void Minimize(Window window, bool minimize);

/**
 * @brief Returns whether the window is minimized or not.
 *
 * @param[in] window The window instance.
 * @return True if the window is minimized, false otherwise.
 */
DALI_ADAPTOR_API bool IsMinimized(Window window);

/**
 * @brief Sets window's minimum size.
 *
 * It is to set the minimum size when window's size is decreased by RequestResizeToServer().
 * Although the size is set by this function, window's size can be decreased over the limitation by SetPositionSize() or SetSize().
 *
 * @param[in] window The window instance.
 * @param[in] size the minimum size
 */
DALI_ADAPTOR_API void SetMimimumSize(Window window, Dali::Window::WindowSize size);

/**
 * @brief Query whether window is rotating or not.
 *
 * @param[in] window The window instance.
 * @return true if window is rotating, false otherwise.
 */
DALI_ADAPTOR_API bool IsWindowRotating(Window window);

/**
 * @brief Gets the last key event the window gets.
 *
 * @param[in] window The window instance.
 * @return The last key event the window gets.
 */
DALI_ADAPTOR_API const KeyEvent& GetLastKeyEvent(Window window);

/**
 * @brief Gets the last touch event the window gets.
 *
 * @param[in] window The window instance.
 * @return The last touch event the window gets.
 * @note It returns the raw event the window gets. There is no hit-actor and local position information.
 */
DALI_ADAPTOR_API const TouchEvent& GetLastTouchEvent(Window window);

/**
 * @brief The user would connect to this signal to intercept a KeyEvent at window.
 *
 * Intercepts KeyEvents in the window before dispatching KeyEvents to the control.
 * If a KeyEvent is consumed, no KeyEvent is delivered to the control.
 *
 * @param[in] window The window instance.
 * @return The signal to connect to
 */
DALI_ADAPTOR_API InterceptKeyEventSignalType& InterceptKeyEventSignal(Window window);

/**
 * @brief This signal is emitted when the window is moved.
 *
 * A callback of the following type may be connected:
 * @code
 *   void YourCallbackName( Window window, Dali::Window::WindowPosition position );
 * @endcode
 * The parameters are the moved x and y coordinates.
 * and window means this signal was called from what window
 *
 * @param[in] window The window instance.
 * @return The signal to connect to
 */
DALI_ADAPTOR_API MovedSignalType& MovedSignal(Window window);

/**
 * @brief This signal is emitted when the window orientation is changed.
 *
 * To emit Window Orientation signal, AddAvailableOrientation() or SetPreferredOrientation() should be called before device is rotated.
 * Most of cases, AddAvailableOrientation() or SetPreferredOrientation() is callled in onCreate().
 *
 * A callback of the following type may be connected:
 * @code
 *   void YourCallbackName( Window window, Dali::WindowOrientation orientation );
 * @endcode
 * The parameter is the changed window orientation.
 * and window means this signal was called from what window
 *
 * @param[in] window The window instance.
 * @return The signal to connect to
 */
DALI_ADAPTOR_API OrientationChangedSignalType& OrientationChangedSignal(Window window);

/**
 * @brief This signal is emitted when the mouse in or out event is received.
 *
 * A callback of the following type may be connected:
 * @code
 *   void YourCallbackName( Window window, Dali::MouseInOutEvent event );
 * @endcode
 *
 * @param[in] window The window instance.
 * @return The signal to connect to
 */
DALI_ADAPTOR_API MouseInOutEventSignalType& MouseInOutEventSignal(Window window);

} // namespace DevelWindow

} // namespace Dali

#endif // DALI_WINDOW_DEVEL_H
