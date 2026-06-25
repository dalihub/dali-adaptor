#ifndef DALI_WINDOW_DEVEL_H
#define DALI_WINDOW_DEVEL_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/events/gesture-enumerations.h>
#include <string>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/mouse-relative-event.h>
#include <dali/devel-api/adaptor-framework/pointer-constraints-event.h>
#include <dali/devel-api/adaptor-framework/window-blur-info.h>
#include <dali/devel-api/common/vector-wrapper.h>
#include <dali/public-api/adaptor-framework/window-definitions.h>
#include <dali/public-api/adaptor-framework/window.h>

namespace Dali
{
class KeyEvent;
class TouchEvent;
class HoverEvent;
class WheelEvent;
class RenderTaskList;
struct TouchPoint;

namespace DevelWindow
{
typedef Signal<void()> EventProcessingFinishedSignalType; ///< Event Processing finished signal type

typedef Signal<void(Window, KeyEvent)>                                               KeyEventSignalType;               ///< Key event signal type
typedef Signal<void(Window, WindowEffectState, WindowEffectType)>                    TransitionEffectEventSignalType;  ///< Effect signal type and state
typedef Signal<void(const std::string&, const std::string&, const Property::Array&)> AuxiliaryMessageSignalType;       ///< Auxiliary message signal type
typedef Signal<void(Window, bool)>                                                   AccessibilityHighlightSignalType; ///< Accessibility Highlight signal type
typedef Signal<bool(Window, KeyEvent)>                                               InterceptKeyEventSignalType;      ///< Intercept Key event signal type
typedef Signal<void(Window, const Dali::DevelWindow::MouseRelativeEvent&)>           MouseRelativeEventSignalType;     ///< MouseRelativeEvent signal type
typedef Signal<void(Window, const Dali::DevelWindow::PointerConstraintsEvent&)>      PointerConstraintsSignalType;     ///< PointerConstraintsEvent signal type

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
 * @brief Sets the maximized state of a given window with specific size
 *
 * If this function is called with true, window will be resized with screen size.
 * Otherwise window will be resized with restore size.
 * It is for the window's MAX button in window's border.
 *
 * It is for client application.
 * If window border is supported by display server, it is not necessary.
 *
 * @param[in] window The window instance.
 * @param[in] maximize If window is maximized or unmaximized.
 * @param[in] restoreSize This is the size used when unmaximized.
 */
DALI_ADAPTOR_API void MaximizeWithRestoreSize(Window window, bool maximize, Dali::Window::WindowSize restoreSize);

/**
 * @brief Sets the pointer constraints lock.
 *
 * @param[in] window The window instance.
 * @return Returns true if PointerConstraintsLock succeeds.
 */
DALI_ADAPTOR_API bool PointerConstraintsLock(Window window);

/**
 * @brief Sets the pointer constraints unlock.
 *
 * @param[in] window The window instance.
 * @return Returns true if PointerConstraintsUnlock succeeds.
 */
DALI_ADAPTOR_API bool PointerConstraintsUnlock(Window window);

/**
 * @brief Sets the locked pointer region
 *
 * @param[in] window The window instance.
 * @param[in] x The x position.
 * @param[in] y The y position.
 * @param[in] width The width.
 * @param[in] height The height
 */
DALI_ADAPTOR_API void LockedPointerRegionSet(Window window, int32_t x, int32_t y, int32_t width, int32_t height);

/**
 * @brief Sets the locked pointer cursor position hintset
 *
 * @param[in] window The window instance.
 * @param[in] x The x position.
 * @param[in] y The y position.
 */
DALI_ADAPTOR_API void LockedPointerCursorPositionHintSet(Window window, int32_t x, int32_t y);

/**
 * @brief Sets the pointer warp. The pointer moves to the set coordinates.
 *
 * @param[in] window The window instance.
 * @param[in] x The x position.
 * @param[in] y The y position.
 * @return Returns true if PointerWarp succeeds.
 */
DALI_ADAPTOR_API bool PointerWarp(Window window, int32_t x, int32_t y);

/**
 * @brief Sets visibility on/off of cursor
 *
 * @param[in] window The window instance.
 * @param[in] visible The visibility of cursor
 */
DALI_ADAPTOR_API void CursorVisibleSet(Window window, bool visible);

/**
 * @brief Requests grab key events according to the requested device subtype
 *
 * @param[in] window The window instance.
 * @param[in] deviceSubclass The deviceSubclass type.
 * @return Returns true if KeyboardGrab succeeds.
 */
DALI_ADAPTOR_API bool KeyboardGrab(Window window, Device::Subclass::Type deviceSubclass);

/**
 * @brief Requests ungrab key events
 *
 * @param[in] window The window instance.
 * @return Returns true if KeyboardUnGrab succeeds.
 */
DALI_ADAPTOR_API bool KeyboardUnGrab(Window window);

/**
 * @brief Sets full screen sized window.
 * If full screen size is set for the window,
 * window will be resized with full screen size.
 * In addition, the full screen sized window's z-order is the highest.
 *
 * @param[in] window The window instance.
 * @param[in] fullscreen true to set fullscreen, false to unset.
 */
DALI_ADAPTOR_API void SetFullScreen(Window window, bool fullscreen);

/**
 * @brief Gets whether the full screen sized window or not
 *
 * @param[in] window The window instance.
 * @return Returns true if the full screen sized window is.
 */
DALI_ADAPTOR_API bool GetFullScreen(Window window);

/**
 * @brief Enables or disables the modal of window.
 *
 * @param[in] window The window instance.
 * @param[in] modal true to enable the modal of window, false to disable.
 */
DALI_ADAPTOR_API void SetModal(Window window, bool modal);

/**
 * @brief Returns whether the window has a modal or not.
 *
 * @param[in] window The window instance.
 * @return True if the window is modal, false otherwise.
 */
DALI_ADAPTOR_API bool IsModal(Window window);

/**
 * @brief Enables or disables the window's layer is changed to bottom.
 *
 * If the enable flag is true, this window will be placed below other windows.
 * Otherwise, if it's called with a false value, it will be located above other windows.
 *
 * @param[in] window The window instance.
 * @param[in] enable true to change the window layer to the bottom.
 */
DALI_ADAPTOR_API void SetBottom(Window window, bool enable);

/**
 * @brief Returns whether the window layer is the bottom or not.
 *
 * @param[in] window The window instance.
 * @return True if the window layer is the bottom, false otherwise.
 */
DALI_ADAPTOR_API bool IsBottom(Window window);

/**
 * @brief Gets the native buffer of the window.
 *
 * When users call this function, it wraps the actual type used by the underlying window system.
 * @return The native buffer of the Window or an empty handle
 */
DALI_ADAPTOR_API Any GetNativeBuffer(Window window);

/**
 * @brief Requests relative motion grab
 *
 * @param[in] window The window instance.
 * @param[in] boundary The Pointer edge boundary for grab.
 * @return True if the request was successful, false otherwise.
 */
DALI_ADAPTOR_API bool RelativeMotionGrab(Window window, uint32_t boundary);

/**
 * @brief Requests relative motion ungrab
 *
 * @param[in] window The window instance.
 * @return True if the request was successful, false otherwise.
 */
DALI_ADAPTOR_API bool RelativeMotionUnGrab(Window window);

/**
 * @brief Sets the window blur.
 *
 * @param[in] window The window instance.
 * @param[in] blurInfo the window blur's informations.
 */
DALI_ADAPTOR_API void SetBlur(Window window, const WindowBlurInfo& blurInfo);

/**
 * @brief Gets the window blur.
 *
 * @param[in] window The window instance.
 * @return Current window blur information.
 */
DALI_ADAPTOR_API WindowBlurInfo GetBlur(Window window);

/**
 * @brief Gets the window insets for all parts of the system UI.
 *
 * @param[in] window The window instance.
 * @return The window insets from all parts.
 */
DALI_ADAPTOR_API Extents GetInsets(Window window);

/**
 * @brief Gets the combined window insets for the specified parts of the system UI.
 *
 * @param[in] window The window instance.
 * @param[in] insetsFlags A bitwise combination of WindowInsetsPartFlags values specifying which window insets parts to include.
 * @return The combined window insets from the specified parts.
 */
DALI_ADAPTOR_API Extents GetInsets(Window window, WindowInsetsPartFlags insetsFlags);

/**
 * @brief Request to rendering forcibly for this window
 *
 * @param[in] window The window instance
 * @param[in] frameCount The number of frames to render forcibly.
 */
DALI_ADAPTOR_API void SetForceRendering(Window window, uint32_t frameCount);

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
 * @brief This signal is emitted when the mouse relative event is received.
 *
 * A callback of the following type may be connected:
 * @code
 *   void YourCallbackName( Window window, Dali::MouseRelativeEvent event );
 * @endcode
 *
 * @param[in] window The window instance.
 * @return The signal to connect to
 */
DALI_ADAPTOR_API MouseRelativeEventSignalType& MouseRelativeEventSignal(Window window);

/**
 * @brief This signal is emitted when pointer is locked/unlocked
 *
 * @param[in] window The window instance
 * @return The signal to connect to
 */
DALI_ADAPTOR_API PointerConstraintsSignalType& PointerConstraintsSignal(Window window);

/**
 * @brief This signal is emitted when the key event monitor is received.
 * @param[in] window The window instance
 * @return The signal to connect to
 */
DALI_ADAPTOR_API KeyEventSignalType& KeyEventMonitorSignal(Window window);

} // namespace DevelWindow

} // namespace Dali

#endif // DALI_WINDOW_DEVEL_H
