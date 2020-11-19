#ifndef DALI_WINDOW_H
#define DALI_WINDOW_H

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
#include <dali/public-api/math/rect.h>
#include <dali/public-api/math/uint-16-pair.h>
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/math/vector4.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal.h>
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <string>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

#undef OPAQUE
#undef TRANSPARENT

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

typedef Dali::Rect<int> PositionSize;

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class Window;
}
} // namespace DALI_INTERNAL

class DragAndDropDetector;
class Orientation;
class Actor;
class Layer;
class RenderTaskList;
class TouchEvent;
class KeyEvent;

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
  using WindowSize     = Uint16Pair; ///< Window size type @SINCE_1_2.60
  using WindowPosition = Uint16Pair; ///< Window position type @SINCE_1_2.60

  using FocusChangeSignalType = Signal<void(Window, bool)>;       ///< Window focus signal type @SINCE_1_4.35
  using ResizeSignalType      = Signal<void(Window, WindowSize)>; ///< Window resized signal type @SINCE_1_4.35
  using KeyEventSignalType    = Signal<void(const KeyEvent&)>;    ///< Key event signal type @SINCE_1_9.21
  using TouchEventSignalType  = Signal<void(const TouchEvent&)>;  ///< Touch signal type @SINCE_1_9.28

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
  static Window New(PositionSize windowPosition, const std::string& name, bool isTransparent = false);

  /**
   * @brief Creates an initialized handle to a new Window.
   * @SINCE_1_0.0
   * @param[in] windowPosition The position and size of the Window
   * @param[in] name The Window title
   * @param[in] className The Window class name
   * @param[in] isTransparent Whether Window is transparent
   * @note This creates an extra window in addition to the default main window
   * @return A new Window
   */
  static Window New(PositionSize windowPosition, const std::string& name, const std::string& className, bool isTransparent = false);

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
  Window(Window&& rhs);

  /**
   * @brief Move assignment operator.
   *
   * @SINCE_1_9.24
   * @param[in] rhs A reference to the moved handle
   * @return A reference to this handle
   */
  Window& operator=(Window&& rhs);

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
  void SetClass(std::string name, std::string klass);

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
   * @brief Gets the native handle of the window.
   *
   * When users call this function, it wraps the actual type used by the underlying window system.
   * @SINCE_1_0.0
   * @return The native handle of the Window or an empty handle
   */
  Any GetNativeHandle() const;

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
  std::string GetSupportedAuxiliaryHint(unsigned int index) const;

  /**
   * @brief Creates an auxiliary hint of the window.
   * @SINCE_1_2.60
   * @param[in] hint The auxiliary hint string.
   * @param[in] value The value string.
   * @return The ID of created auxiliary hint, or @c 0 on failure.
   */
  unsigned int AddAuxiliaryHint(const std::string& hint, const std::string& value);

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
  bool SetAuxiliaryHintValue(unsigned int id, const std::string& value);

  /**
   * @brief Gets a value of the auxiliary hint.
   * @SINCE_1_2.60
   * @param[in] id The auxiliary hint ID.
   * @return The string value of the auxiliary hint ID, or an empty string if none exists.
   */
  std::string GetAuxiliaryHintValue(unsigned int id) const;

  /**
   * @brief Gets a ID of the auxiliary hint string.
   * @SINCE_1_2.60
   * @param[in] hint The auxiliary hint string.
   * @return The ID of the auxiliary hint string, or @c 0 if none exists.
   */
  unsigned int GetAuxiliaryHintId(const std::string& hint) const;

  /**
   * @brief Sets a region to accept input events.
   * @SINCE_1_2.60
   * @param[in] inputRegion The region to accept input events.
   */
  void SetInputRegion(const Rect<int>& inputRegion);

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
   * @brief Sets a priority level for the specified notification window.
   * @@SINCE_2_0.0
   * @param[in] level The notification window level.
   * @return True if no error occurred, false otherwise.
   * @PRIVLEVEL_PUBLIC
   * @PRIVILEGE_WINDOW_PRIORITY
   * @remarks This can be used for a notification type window only. The default level is NotificationLevel::NONE.
   */
  bool SetNotificationLevel(WindowNotificationLevel level);

  /**
   * @brief Gets a priority level for the specified notification window.
   * @@SINCE_2_0.0
   * @return The notification window level.
   * @remarks This can be used for a notification type window only.
   */
  WindowNotificationLevel GetNotificationLevel() const;

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
   * @return True if no error occurred, false otherwise.
   * @PRIVLEVEL_PUBLIC
   * @PRIVILEGE_DISPLAY
   */
  bool SetScreenOffMode(WindowScreenOffMode screenOffMode);

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
   * @return True if no error occurred, false otherwise.
   * @PRIVLEVEL_PUBLIC
   * @PRIVILEGE_DISPLAY
   */
  bool SetBrightness(int brightness);

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
   * @brief This signal is emitted when key event is received.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(const KeyEvent& event);
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
   *   void YourCallbackName(const TouchEvent& event);
   * @endcode
   *
   * @SINCE_1_9.28
   * @return The touch signal to connect to
   *
   * @note Motion events are not emitted.
   */
  TouchEventSignalType& TouchedSignal();

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
