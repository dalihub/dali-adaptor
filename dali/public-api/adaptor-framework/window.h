#ifndef DALI_WINDOW_H
#define DALI_WINDOW_H

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
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/common/extents.h>
#include <dali/public-api/events/gesture-enumerations.h>
#include <dali/public-api/events/wheel-event.h>
#include <dali/public-api/math/int-pair.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/math/vector4.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/callback.h>
#include <dali/public-api/signals/dali-signal.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/mouse-in-out-event.h>
#include <dali/public-api/adaptor-framework/window-data.h>
#include <dali/public-api/adaptor-framework/window-definitions.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

typedef Dali::BoundsInteger PositionSize;

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class Window;
}
} //namespace Internal DALI_INTERNAL

class Actor;
class Layer;
class RenderTaskList;
class TouchEvent;
class KeyEvent;
class HoverEvent;

/**
 * @brief The window class is used internally for drawing.
 *
 * A Window has an orientation and indicator properties.
 * You can get a valid Window handle by calling Dali::Application::GetWindow().
 * @SINCE_1_0.0
 */
class DALI_ADAPTOR_API Window : public BaseHandle
{
public:
  using WindowSize     = Int32Pair; ///< Window size type @SINCE_1_2.60
  using WindowPosition = Int32Pair; ///< Window position type @SINCE_2_1.45

  using FocusChangeSignalType       = Signal<void(Window, bool)>; ///< Window focus signal type @SINCE_1_4.35
  using VisibilityChangedSignalType = Signal<void(Window, bool)>; ///< Visibility changed signal type @SINCE_2_5.28

  using ResizeSignalType             = Signal<void(Window, WindowSize)>;        ///< Window resized signal type @SINCE_1_4.35
  using MovedSignalType              = Signal<void(Window, WindowPosition)>;    ///< Window moved signal type @SINCE_2_5.28
  using OrientationChangedSignalType = Signal<void(Window, WindowOrientation)>; ///< Window orientation changed signal type @SINCE_2_5.28
  using MoveCompletedSignalType      = Signal<void(Window, WindowPosition)>;    ///< Window moved by server signal type @SINCE_2_5.28
  using ResizeCompletedSignalType    = Signal<void(Window, WindowSize)>;        ///< Window resized by server signal type @SINCE_2_5.28

  using KeyEventSignalType        = Signal<void(Window, KeyEvent)>;               ///< Key event signal type @SINCE_1_9.21
  using TouchEventSignalType      = Signal<void(Window, TouchEvent)>;             ///< Touch signal type @SINCE_1_9.28
  using WheelEventSignalType      = Signal<void(Window, WheelEvent)>;             ///< Wheel signal type @SINCE_2_5.28
  using MouseInOutEventSignalType = Signal<void(Window, const MouseInOutEvent&)>; ///< Mouse in/out event signal type @SINCE_2_5.28

  using InsetsChangedSignalType = Signal<void(Window, const WindowInsetsInfo&)>; ///< Insets changed signal type @SINCE_2_5.28

public:
  // Methods

  /**
   * @brief Creates an initialized handle to a new Window.
   * @SINCE_1_0.0
   * @param[in] windowPosition The position and size of the Window
   * @param[in] name The Window title
   * @param[in] isTransparent Whether Window is transparent
   * @return A new window
   * @note This creates an extra window in addition to the default main window
   */
  static Window New(PositionSize windowPosition, const Dali::String& name, bool isTransparent = false);

  /**
   * @brief Creates an initialized handle to a new Window.
   * @SINCE_2_2.33
   * @param[in] name The Window title
   * @param[in] className The Window class name
   * @param[in] windowData The window data
   * @note This creates an extra window in addition to the default main window
   * @return A new Window
   */
  static Window New(const Dali::String& name, const Dali::String& className, const WindowData& windowData);

  /**
   * @brief Creates an uninitialized handle.
   *
   * This can be initialized using Dali::Application::GetWindow() or
   * Dali::Window::New().
   * @SINCE_1_0.0
   */
  Window();

  /**
   * @brief Destructor.
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   * @SINCE_1_0.0
   */
  ~Window();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @SINCE_1_0.0
   * @param[in] handle A reference to the copied handle
   */
  Window(const Window& handle);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @SINCE_1_0.0
   * @param[in] rhs A reference to the copied handle
   * @return A reference to this
   */
  Window& operator=(const Window& rhs);

  /**
   * @brief Move constructor.
   *
   * @SINCE_1_9.24
   * @param[in] rhs A reference to the moved handle
   */
  Window(Window&& rhs) noexcept;

  /**
   * @brief Move assignment operator.
   *
   * @SINCE_1_9.24
   * @param[in] rhs A reference to the moved handle
   * @return A reference to this handle
   */
  Window& operator=(Window&& rhs) noexcept;

  /**
   * @brief Downcast sceneHolder to window
   *
   * @SINCE_2_1.46
   * @param[in] handle The handle need to downcast
   * @return Whether it's a valid window or not
   */
  static Window DownCast(BaseHandle handle);

  /**
   * @brief Adds a child Actor to the Window.
   *
   * The child will be referenced.
   *
   * @SINCE_1_4.19
   * @param[in] actor The child
   * @pre The actor has been initialized.
   * @pre The actor does not have a parent.
   */
  void Add(Actor actor);

  /**
   * @brief Removes a child Actor from the Window.
   *
   * The child will be unreferenced.
   *
   * @SINCE_1_4.19
   * @param[in] actor The child
   * @pre The actor has been added to the stage.
   */
  void Remove(Actor actor);

  /**
   * @brief Sets the background color of the Window.
   *
   * @SINCE_1_4.19
   * @param[in] color The new background color
   * @note If this window is transparent, the color is converted premultiplied alpha format internally.
   */
  void SetBackgroundColor(const Vector4& color);

  /**
   * @brief Gets the background color of the Window.
   *
   * @SINCE_1_4.19
   * @return The background color
   */
  Vector4 GetBackgroundColor() const;

  /**
   * @brief Returns the root Layer of the Window.
   *
   * @SINCE_1_4.19
   * @return The root layer
   */
  Layer GetRootLayer() const;

  /**
   * @brief Returns the overlay Layer of the Window.
   * If there isn't overlay layer yet, this method create overlay layer and
   * exclusive render task internally.
   *
   * @SINCE_2_2.10
   * @return The root layer
   */
  Layer GetOverlayLayer();

  /**
   * @brief Queries the number of on-scene layers in the Window.
   *
   * Note that a default layer is always provided (count >= 1).
   *
   * @SINCE_1_4.19
   * @return The number of layers
   */
  uint32_t GetLayerCount() const;

  /**
   * @brief Retrieves the layer at a specified depth in the Window.
   *
   * @SINCE_1_4.19
   * @param[in] depth The depth
   * @return The layer found at the given depth
   * @pre Depth is less than layer count; see GetLayerCount().
   */
  Layer GetLayer(uint32_t depth) const;

  /**
   * @brief Retrieves the DPI of the window.
   *
   * @SINCE_1_9.21
   * @return The DPI of the window
   */
  Uint16Pair GetDpi() const;

  /**
   * @brief Sets the window name and class string.
   * @SINCE_1_0.0
   * @param[in] name The name of the window
   * @param[in] klass The class of the window
   */
  void SetClass(Dali::String name, Dali::String klass);

  /**
   * @brief Raises window to the top of Window stack.
   * @SINCE_1_0.0
   */
  void Raise();

  /**
   * @brief Lowers window to the bottom of Window stack.
   * @SINCE_1_0.0
   */
  void Lower();

  /**
   * @brief Activates window to the top of Window stack even it is iconified.
   * @SINCE_1_0.0
   */
  void Activate();

  /**
   * @brief Adds an orientation to the list of available orientations.
   * @SINCE_1_0.0
   * @param[in] orientation The available orientation to add
   */
  void AddAvailableOrientation(WindowOrientation orientation);

  /**
   * @brief Removes an orientation from the list of available orientations.
   * @SINCE_1_0.0
   * @param[in] orientation The available orientation to remove
   */
  void RemoveAvailableOrientation(WindowOrientation orientation);

  /**
   * @brief Sets available orientations of the window.
   *
   * This API is for setting several orientations one time.
   *
   * @SINCE_2_5.27
   * @param[in] orientations The available orientation list to add
   */
  void SetAvailableOrientations(const Dali::Vector<WindowOrientation>& orientations);

  /**
   * @brief Sets a preferred orientation.
   * @SINCE_1_0.0
   * @param[in] orientation The preferred orientation
   * @pre Orientation is in the list of available orientations.
   *
   * @note To unset the preferred orientation, orientation should be set NO_ORIENTATION_PREFERENCE.
   */
  void SetPreferredOrientation(WindowOrientation orientation);

  /**
   * @brief Gets the preferred orientation.
   * @SINCE_1_0.0
   * @return The preferred orientation if previously set, or none
   */
  WindowOrientation GetPreferredOrientation();

  /**
   * @brief Gets current orientation of the window.
   *
   * @SINCE_2_5.27
   * @return The current window orientation if previously set, or none
   */
  WindowOrientation GetCurrentOrientation();

  /**
   * @brief Queries whether the window's orientation change is in progress.
   *
   * @SINCE_2_5.27
   * @return true if orientation change is in progress, false otherwise
   */
  bool IsOrientationChanging() const;

  /**
   * @brief Gets the native handle of the window.
   *
   * When users call this function, it wraps the actual type used by the underlying window system.
   * @SINCE_1_0.0
   * @return The native handle of the Window or an empty handle
   */
  Any GetNativeHandle() const;

  /**
   * @brief Gets the native window id.
   *
   * @SINCE_2_5.27
   * @return The native window id
   */
  int32_t GetNativeId() const;

  /**
   * @brief Sets whether window accepts focus or not.
   *
   * @SINCE_1_2.60
   * @param[in] accept If focus is accepted or not. Default is true.
   */
  void SetAcceptFocus(bool accept);

  /**
   * @brief Returns whether window accepts focus or not.
   *
   * @SINCE_1_2.60
   * @return True if the window accept focus, false otherwise
   */
  bool IsFocusAcceptable() const;

  /**
   * @brief Shows the window if it is hidden.
   * @SINCE_1_2.60
   */
  void Show();

  /**
   * @brief Hides the window if it is showing.
   * @SINCE_1_2.60
   */
  void Hide();

  /**
   * @brief Returns whether the window is visible or not.
   * @SINCE_1_2.60
   * @return True if the window is visible, false otherwise.
   */
  bool IsVisible() const;

  /**
   * @brief Gets the count of supported auxiliary hints of the window.
   * @SINCE_1_2.60
   * @return The number of supported auxiliary hints.
   *
   * @note The window auxiliary hint is the value which is used to decide which actions should be made available to the user by the window manager.
   * If you want to set specific hint to your window, then you should check whether it exists in the supported auxiliary hints.
   */
  unsigned int GetSupportedAuxiliaryHintCount() const;

  /**
   * @brief Gets the supported auxiliary hint string of the window.
   * @SINCE_1_2.60
   * @param[in] index The index of the supported auxiliary hint lists
   * @return The auxiliary hint string of the index.
   *
   * @note The window auxiliary hint is the value which is used to decide which actions should be made available to the user by the window manager.
   * If you want to set specific hint to your window, then you should check whether it exists in the supported auxiliary hints.
   */
  Dali::String GetSupportedAuxiliaryHint(unsigned int index) const;

  /**
   * @brief Creates an auxiliary hint of the window.
   * @SINCE_1_2.60
   * @param[in] hint The auxiliary hint string.
   * @param[in] value The value string.
   * @return The ID of created auxiliary hint, or @c 0 on failure.
   */
  unsigned int AddAuxiliaryHint(const Dali::String& hint, const Dali::String& value);

  /**
   * @brief Removes an auxiliary hint of the window.
   * @SINCE_1_2.60
   * @param[in] id The ID of the auxiliary hint.
   * @return True if no error occurred, false otherwise.
   */
  bool RemoveAuxiliaryHint(unsigned int id);

  /**
   * @brief Changes a value of the auxiliary hint.
   * @SINCE_1_2.60
   * @param[in] id The auxiliary hint ID.
   * @param[in] value The value string to be set.
   * @return True if no error occurred, false otherwise.
   */
  bool SetAuxiliaryHintValue(unsigned int id, const Dali::String& value);

  /**
   * @brief Gets a value of the auxiliary hint.
   * @SINCE_1_2.60
   * @param[in] id The auxiliary hint ID.
   * @return The string value of the auxiliary hint ID, or an empty string if none exists.
   */
  Dali::String GetAuxiliaryHintValue(unsigned int id) const;

  /**
   * @brief Gets a ID of the auxiliary hint string.
   * @SINCE_1_2.60
   * @param[in] hint The auxiliary hint string.
   * @return The ID of the auxiliary hint string, or @c 0 if none exists.
   */
  unsigned int GetAuxiliaryHintId(const Dali::String& hint) const;

  /**
   * @brief Sets a region to accept input events.
   * @SINCE_1_2.60
   * @param[in] inputRegion The region to accept input events.
   */
  void SetInputRegion(const BoundsInteger& inputRegion);

  /**
   * @brief Includes input region.
   *
   * This function includes input regions.
   * It can be used multiple times and supports multiple regions.
   * It means input region will be extended.
   *
   * This input is related to mouse and touch event.
   * If device has touch screen, this function is useful.
   * Otherwise device does not have that, we can use it after connecting mouse to the device.
   *
   * @SINCE_2_5.27
   * @param[in] inputRegion The added region to accept input events.
   */
  void IncludeInputRegion(const BoundsInteger& inputRegion);

  /**
   * @brief Excludes input region.
   *
   * This function excludes input regions.
   * It can be used multiple times and supports multiple regions.
   * It means input region will be reduced.
   * Notice, should be set input area by IncludeInputRegion() before this function is used.
   *
   * This input is related to mouse and touch event.
   * If device has touch screen, this function is useful.
   * Otherwise device does not have that, we can use it after connecting mouse to the device.
   *
   * @SINCE_2_5.27
   * @param[in] inputRegion The region to exclude from accepting input events.
   */
  void ExcludeInputRegion(const BoundsInteger& inputRegion);

  /**
   * @brief Sets a window type.
   * @@SINCE_2_0.0
   * @param[in] type The window type.
   * @remarks The default window type is NORMAL.
   */
  void SetType(WindowType type);

  /**
   * @brief Gets a window type.
   * @@SINCE_2_0.0
   * @return A window type.
   */
  WindowType GetType() const;

  /**
   * @brief Enables the floating mode of window.
   *
   * The floating mode is to support making partial size window easily.
   * It is useful for creating popup-style windows that are always above other normal windows.
   * It also allows easy switching between popup style and normal style.
   *
   * A special display server (e.g., Tizen display server) supports this mode.
   *
   * @SINCE_2_5.27
   * @param[in] enable Enable floating mode or not.
   */
  void EnableFloatingMode(bool enable);

  /**
   * @brief Returns whether the window is in floating mode or not.
   *
   * @SINCE_2_5.27
   * @return True if floating mode is enabled for the window, false otherwise.
   */
  bool IsFloatingModeEnabled() const;

  /**
   * @brief Sets a priority level for the specified notification window.
   * @@SINCE_2_0.0
   * @param[in] level The notification window level.
   * @return The result of the window operation.
   * @PRIVLEVEL_PUBLIC
   * @PRIVILEGE_WINDOW_PRIORITY
   * @remarks This can be used for a notification type window only. The default level is NotificationLevel::NONE.
   */
  WindowOperationResult SetNotificationLevel(WindowNotificationLevel level);

  /**
   * @brief Gets a priority level for the specified notification window.
   * @@SINCE_2_0.0
   * @return The notification window level.
   * @remarks This can be used for a notification type window only.
   */
  WindowNotificationLevel GetNotificationLevel() const;

  /**
   * @brief Enables or disables the window always being on top.
   *
   * This is valid between windows that have no notification level or a notification level of 'none'.
   * If it has a notification level, this will not do anything.
   *
   * @SINCE_2_5.27
   * @param[in] alwaysOnTop True to enable the window always on top, false to disable.
   */
  void SetAlwaysOnTop(bool alwaysOnTop);

  /**
   * @brief Returns whether the window is always on top.
   *
   * @SINCE_2_5.27
   * @return True if the window is always on top, false otherwise.
   */
  bool IsAlwaysOnTop() const;

  /**
   * @brief Sets a transparent window's visual state to opaque.
   * @details If a visual state of a transparent window is opaque,
   * then the window manager could handle it as an opaque window when calculating visibility.
   * @SINCE_1_2.60
   * @param[in] opaque Whether the window's visual state is opaque.
   * @remarks This will have no effect on an opaque window.
   * It doesn't change transparent window to opaque window but lets the window manager know the visual state of the window.
   */
  void SetOpaqueState(bool opaque);

  /**
   * @brief Returns whether a transparent window's visual state is opaque or not.
   * @SINCE_1_2.60
   * @return True if the window's visual state is opaque, false otherwise.
   * @remarks The return value has no meaning on an opaque window.
   */
  bool IsOpaqueState() const;

  /**
   * @brief Sets a window's screen off mode.
   * @details This API is useful when the application needs to keep the display turned on.
   * If the application sets the screen mode to #::Dali::WindowScreenOffMode::NEVER to its window and the window is shown,
   * the window manager requests the display system to keep the display on as long as the window is shown.
   * If the window is no longer shown, then the window manager requests the display system to go back to normal operation.
   * @@SINCE_2_0.0
   * @param[in] screenOffMode The screen mode.
   * @return The result of the window operation.
   * @PRIVLEVEL_PUBLIC
   * @PRIVILEGE_DISPLAY
   */
  WindowOperationResult SetScreenOffMode(WindowScreenOffMode screenOffMode);

  /**
   * @brief Gets a screen off mode of the window.
   * @@SINCE_2_0.0
   * @return The screen off mode.
   */
  WindowScreenOffMode GetScreenOffMode() const;

  /**
   * @brief Sets preferred brightness of the window.
   * @details This API is useful when the application needs to change the brightness of the screen when it is appeared on the screen.
   * If the brightness has been set and the window is shown, the window manager requests the display system to change the brightness to the provided value.
   * If the window is no longer shown, then the window manager requests the display system to go back to default brightness.
   * A value less than 0 results in default brightness and a value greater than 100 results in maximum brightness.
   * @SINCE_1_2.60
   * @param[in] brightness The preferred brightness (0 to 100).
   * @return The result of the window operation.
   * @PRIVLEVEL_PUBLIC
   * @PRIVILEGE_DISPLAY
   */
  WindowOperationResult SetBrightness(int brightness);

  /**
   * @brief Gets preferred brightness of the window.
   * @SINCE_1_2.60
   * @return The preferred brightness.
   */
  int GetBrightness() const;

  /**
   * @brief Sets a size of the window.
   *
   * @SINCE_1_2.60
   * @param[in] size The new window size
   */
  void SetSize(WindowSize size);

  /**
   * @brief Gets a size of the window.
   *
   * @SINCE_1_2.60
   * @return The size of the window
   */
  WindowSize GetSize() const;

  /**
   * @brief Sets a position of the window.
   *
   * @SINCE_1_2.60
   * @param[in] position The new window position
   */
  void SetPosition(WindowPosition position);

  /**
   * @brief Gets a position of the window.
   *
   * @SINCE_1_2.60
   * @return The position of the window
   */
  WindowPosition GetPosition() const;

  /**
   * @brief Requests to display server for the window is moved by display server.
   *
   * This function should be called in mouse down event callback function.
   * After this function is called in mouse down event callback function, the window is moved with mouse move event.
   * When mouse up event happens, the window moved work is finished.
   *
   * @SINCE_2_5.27
   */
  void RequestMoveToServer();

  /**
   * @brief Sets the layout of the window.
   *
   * This method sets the layout of the window based on the specified number of columns and rows,
   * as well as the position and size of the window within that layout.
   *
   * @param numCols The number of columns in the layout.
   * @param numRows The number of rows in the layout.
   * @param column The column number of the window within the layout.
   * @param row The row number of the window within the layout.
   * @param colSpan The number of columns the window should span within the layout.
   * @param rowSpan The number of rows the window should span within the layout.
   * @SINCE_2_2.20
   */
  void SetLayout(unsigned int numCols, unsigned int numRows, unsigned int column, unsigned int row, unsigned int colSpan, unsigned int rowSpan);

  /**
   * @brief Sets whether the window is transparent or not.
   *
   * @SINCE_1_2.60
   * @param[in] transparent Whether the window is transparent
   */
  void SetTransparency(bool transparent);

  /**
   * @brief Retrieves the list of render-tasks in the window.
   *
   * @SINCE_1_9.21
   * @return A valid handle to a RenderTaskList
   */
  RenderTaskList GetRenderTaskList();

  /**
   * @brief Keeps rendering for at least the given amount of time.
   *
   * By default, Dali will stop rendering when no Actor positions are being set, and when no animations are running etc.
   * This method is useful to force screen refreshes.
   *
   * @SINCE_2_2.29
   * @param[in] durationSeconds Time to keep rendering, 0 means render at least one more frame
   */
  void KeepRendering(float durationSeconds);

  /**
   * @brief Sets whether the window has a depth buffer or not.
   *
   * @SINCE_2_5.20
   * @param[in] enabled True if a depth buffer should be created.
   * @note This is an alternative to setting the environment variable.
   * There is no longer a global setting; the environment variable will
   * only affect the main window (the automatically generated window).
   */
  void SetDepthBufferEnabled(bool enabled);

  /**
   * @brief Queries whether the window has a depth buffer
   *
   * @SINCE_2_5.20
   * @return True if the window has a depth buffer
   */
  bool IsDepthBufferEnabled() const;

  /**
   * @brief Sets whether the window has a depth buffer or not.
   *
   * @SINCE_2_5.20
   * @param[in] enabled True if a depth buffer should be created.
   * @note This is an alternative to setting the environment variable.
   * There is no longer a global setting; the environment variable will
   * only affect the main window (the automatically generated window).
   */
  void SetStencilBufferEnabled(bool enabled);

  /**
   * @brief Queries whether the window has a stencil buffer
   *
   * @SINCE_2_5.20
   * @return True if the window has a stencil buffer
   */
  bool IsStencilBufferEnabled() const;

  /**
   * @brief Sets whether the window has MSAA.
   *
   * @SINCE_2_5.20
   * @param[in] enabled True if MSAA should be initialized
   */
  void SetMultiSampledAntiAliasingEnabled(bool enabled);

  /**
   * @brief Queries whether the window has MSAA
   *
   * @SINCE_2_5.20
   * @return True if the window has MSAA
   */
  bool IsMultiSampledAntiAliasingEnabled() const;

  /**
   * @brief Sets whether the window will update partial area or full area.
   *
   * @SINCE_2_2.33
   * @param[in] enabled True if the window should update partial area
   * @note This is an alternative to setting the environment variable.
   * There is no longer a global setting; the environment variable will
   * only affect the main window (the automatically generated window)
   */
  void SetPartialUpdateEnabled(bool enabled);

  /**
   * @brief Queries whether the window will update partial area.
   *
   * @SINCE_2_2.33
   * @return True if the window should update partial area
   */
  bool IsPartialUpdateEnabled() const;

  /**
   * @brief Enables or disables front buffer rendering.
   *
   * @SINCE_2_5.27
   * @param[in] enable True to enable front buffer rendering, false to disable.
   */
  void SetFrontBufferRenderingEnabled(bool enable);

  /**
   * @brief Returns whether front buffer rendering is enabled or not.
   *
   * @SINCE_2_5.27
   * @return True if front buffer rendering is enabled, false otherwise.
   */
  bool IsFrontBufferRenderingEnabled() const;

  /**
   * @brief Maximizes window's size.
   *
   * If this function is called with true, window will be resized with screen size.
   * Otherwise window will be resized with previous size.
   * It is for the window's MAX button in window's border.
   *
   * It is for client application.
   * If window border is supported by display server, it is not necessary.
   *
   * @SINCE_2_5.27
   * @param[in] maximize True to maximize the window, false to restore to previous size.
   */
  void Maximize(bool maximize);

  /**
   * @brief Returns whether the window is maximized or not.
   *
   * @SINCE_2_5.27
   * @return True if the window is maximized, false otherwise.
   */
  bool IsMaximized() const;

  /**
   * @brief Sets window's maximum size.
   *
   * It is to set the maximized size when window is maximized.
   * Although the size is set by this function, window's size can be increased beyond the limitation by SetSize().
   *
   * After setting, if Maximize() is called, window is resized with the setting size and centered.
   *
   * @SINCE_2_5.27
   * @param[in] size The maximum size.
   */
  void SetMaximumSize(WindowSize size);

  /**
   * @brief Minimizes window's size.
   *
   * If this function is called with true, window will be iconified.
   * Otherwise window will be activated.
   * It is for the window's MIN button in window border.
   *
   * It is for client application.
   * If window border is supported by display server, it is not necessary.
   *
   * @SINCE_2_5.27
   * @param[in] minimize True to minimize the window, false to activate it.
   */
  void Minimize(bool minimize);

  /**
   * @brief Returns whether the window is minimized or not.
   *
   * @SINCE_2_5.27
   * @return True if the window is minimized, false otherwise.
   */
  bool IsMinimized() const;

  /**
   * @brief Sets window's minimum size.
   *
   * It is to set the minimum size when window's size is decreased by the display server.
   * Although the size is set by this function, window's size can be decreased below the limitation by SetSize().
   *
   * @SINCE_2_5.27
   * @param[in] size The minimum size.
   */
  void SetMinimumSize(WindowSize size);

  /**
   * @brief Sets the parent window of this window.
   *
   * After setting, these windows move together when raised, lowered, or iconified/deiconified.
   * Initially, the child window is located on top of the parent.
   * If @a belowParent is true, the child is placed below the parent instead.
   *
   * @SINCE_2_5.27
   * @param[in] parent The parent window instance.
   * @param[in] belowParent If true, the child window is placed below the parent; otherwise above. Defaults to false.
   */
  void SetParent(Window parent, bool belowParent = false);

  /**
   * @brief Unsets the parent window of this window.
   *
   * After unsetting, the window is disconnected from its parent window.
   *
   * @SINCE_2_5.27
   */
  void Unparent();

  /**
   * @brief Gets the parent window of this window.
   *
   * @SINCE_2_5.27
   * @return The parent window, or an empty handle if no parent is set.
   */
  Window GetParent();

  /**
   * @brief Adds a callback that is called when the frame rendering is done by the graphics driver.
   *
   * @SINCE_2_5.27
   * @param[in] callback The function to call.
   * @param[in] frameId The ID to specify the frame. It will be passed when the callback is called.
   *
   * @note A callback of the following type may be used:
   * @code
   *   void MyFunction( int32_t frameId );
   * @endcode
   * This callback will be deleted once it is called.
   *
   * @note Ownership of the callback is passed onto this class.
   */
  void AddFrameRenderedCallback(CallbackBase* callback, int32_t frameId);

  /**
   * @brief Adds a callback that is called when the frame is displayed on the display.
   *
   * @SINCE_2_5.27
   * @param[in] callback The function to call.
   * @param[in] frameId The ID to specify the frame. It will be passed when the callback is called.
   *
   * @note A callback of the following type may be used:
   * @code
   *   void MyFunction( int32_t frameId );
   * @endcode
   * This callback will be deleted once it is called.
   *
   * @note Ownership of the callback is passed onto this class.
   */
  void AddFramePresentedCallback(CallbackBase* callback, int32_t frameId);

  /**
   * @brief Sets the screen for this window.
   *
   * The window will be moved to the specified screen.
   *
   * @SINCE_2_5.27
   * @param[in] screenName The name of the screen.
   */
  void SetScreen(const Dali::String& screenName);

  /**
   * @brief Gets the screen of the window.
   *
   * This is for multi-screen environments.
   *
   * @SINCE_2_5.27
   * @return The name of the screen.
   */
  Dali::String GetScreen() const;

  /**
   * @brief Feeds a touch event to the window.
   *
   * @SINCE_2_5.27
   * @param[in] touchEvent The touch event
   */
  void FeedTouchEvent(const Dali::TouchEvent& touchEvent);

  /**
   * @brief Feeds a wheel event to the window.
   *
   * @SINCE_2_5.27
   * @param[in] wheelEvent The wheel event
   */
  void FeedWheelEvent(Dali::WheelEvent wheelEvent);

  /**
   * @brief Feeds a key event to the window.
   *
   * @SINCE_2_5.27
   * @param[in] keyEvent The key event.
   */
  void FeedKeyEvent(const Dali::KeyEvent& keyEvent);

  /**
   * @brief Feeds a hover event to the window.
   *
   * @SINCE_2_5.27
   * @param[in] hoverEvent The hover event
   */
  void FeedHoverEvent(const Dali::HoverEvent& hoverEvent);

  /**
   * @brief Returns the last key event received by the window.
   *
   * @SINCE_2_5.27
   * @return The last key event received by the window.
   */
  const KeyEvent& GetLastKeyEvent() const;

  /**
   * @brief Returns the last touch event received by the window.
   *
   * @SINCE_2_5.27
   * @return The last touch event received by the window.
   * @note Returns the raw event; hit-actor and local position information are not available.
   */
  const TouchEvent& GetLastTouchEvent() const;

  /**
   * @brief Returns the last hover event received by the window.
   *
   * @SINCE_2_5.27
   * @return The last hover event received by the window.
   * @note Returns the raw event; hit-actor and local position information are not available.
   */
  const HoverEvent& GetLastHoverEvent() const;

  /**
   * @brief Returns the last pan gesture state received by the window.
   *
   * @SINCE_2_5.27
   * @return The last pan gesture state received by the window.
   */
  GestureState GetLastPanGestureState() const;

public: // Signals
  /**
   * @brief The user should connect to this signal to get a timing when window gains focus or loses focus.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName( Window window, bool focusIn );
   * @endcode
   * The parameter is true if window gains focus, otherwise false.
   * and window means this signal was called from what window
   *
   * @SINCE_1_4.35
   * @return The signal to connect to
   */
  FocusChangeSignalType& FocusChangeSignal();

  /**
   * @brief This signal is emitted when the window is shown or hidden.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(Window window, bool visible);
   * @endcode
   *
   * @SINCE_2_5.28
   * @return The signal to connect to
   */
  VisibilityChangedSignalType& VisibilityChangedSignal();

  /**
   * @brief This signal is emitted when the window is resized.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName( Window window, int width, int height );
   * @endcode
   * The parameters are the resized width and height.
   * and window means this signal was called from what window
   *
   * @SINCE_1_4.35
   * @return The signal to connect to
   */
  ResizeSignalType& ResizeSignal();

  /**
   * @brief This signal is emitted when the window is moved.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(Window window, Dali::Window::WindowPosition position);
   * @endcode
   * The parameters are the moved x and y coordinates.
   *
   * @SINCE_2_5.28
   * @return The signal to connect to
   */
  MovedSignalType& MovedSignal();

  /**
   * @brief This signal is emitted when the window orientation is changed.
   *
   * To emit this signal, AddAvailableOrientation() or SetPreferredOrientation() should be called before the device is rotated.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(Window window, Dali::WindowOrientation orientation);
   * @endcode
   * The parameter is the changed window orientation.
   *
   * @SINCE_2_5.28
   * @return The signal to connect to
   */
  OrientationChangedSignalType& OrientationChangedSignal();

  /**
   * @brief This signal is emitted when the window has been moved by the display server.
   *
   * To trigger a server-side move, call RequestMoveToServer().
   * After the move is completed, this signal will be emitted.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(Window window, Dali::Window::WindowPosition position);
   * @endcode
   * The parameters are the final x and y coordinates after moving.
   *
   * @SINCE_2_5.28
   * @return The signal to connect to
   */
  MoveCompletedSignalType& MoveCompletedSignal();

  /**
   * @brief This signal is emitted when the window has been resized by the display server.
   *
   * To trigger a server-side resize, call RequestResizeToServer().
   * After the resize is completed, this signal will be emitted.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(Window window, Dali::Window::WindowSize size);
   * @endcode
   * The parameters are the final width and height after resizing.
   *
   * @SINCE_2_5.28
   * @return The signal to connect to
   */
  ResizeCompletedSignalType& ResizeCompletedSignal();

  /**
   * @brief This signal is emitted when key event is received.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(Window window, KeyEvent event);
   * @endcode
   *
   * @SINCE_1_9.21
   * @return The signal to connect to
   */
  KeyEventSignalType& KeyEventSignal();

  /**
   * @brief This signal is emitted when the screen is touched and when the touch ends
   * (i.e. the down & up touch events only).
   *
   * If there are multiple touch points, then this will be emitted when the first touch occurs and
   * then when the last finger is lifted.
   * An interrupted event will also be emitted (if it occurs).
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(Window window, TouchEvent event);
   * @endcode
   *
   * @SINCE_1_9.28
   * @return The touch signal to connect to
   *
   * @note Motion events are not emitted.
   */
  TouchEventSignalType& TouchedSignal();

  /**
   * @brief This signal is emitted when a wheel event is received.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(Window window, WheelEvent event);
   * @endcode
   *
   * @SINCE_2_5.28
   * @return The signal to connect to
   */
  WheelEventSignalType& WheelEventSignal();

  /**
   * @brief This signal is emitted when a mouse in or out event is received.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(Window window, Dali::MouseInOutEvent event);
   * @endcode
   *
   * @SINCE_2_5.28
   * @return The signal to connect to
   */
  MouseInOutEventSignalType& MouseInOutEventSignal();

  /**
   * @brief This signal is emitted when window insets change due to the indicator, virtual keyboard, or clipboard appearing or disappearing.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(Window window, const WindowInsetsInfo& insetsInfo);
   * @endcode
   * The parameter contains the insets information including the part type, part state, and the extents (left, right, top, bottom).
   *
   * @SINCE_2_5.28
   * @return The signal to connect to
   */
  InsetsChangedSignalType& InsetsChangedSignal();

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief This constructor is used by Dali::Application::GetWindow().
   * @SINCE_1_0.0
   * @param[in] window A pointer to the Window
   */
  explicit DALI_INTERNAL Window(Internal::Adaptor::Window* window);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // __DALI_WINDOW_H__
