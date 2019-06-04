#ifndef DALI_WINDOW_H
#define DALI_WINDOW_H

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

// EXTERNAL INCLUDES
#include <string>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/math/uint-16-pair.h>
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/math/vector4.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/signals/dali-signal.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

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
}

class DragAndDropDetector;
class Orientation;
class Actor;
class Layer;

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

  typedef Uint16Pair WindowSize;          ///< Window size type @SINCE_1_2.60
  typedef Uint16Pair WindowPosition;      ///< Window position type @SINCE_1_2.60

  typedef Signal< void (bool) > IndicatorSignalType;  ///< @DEPRECATED_1_4.9 @brief Indicator state signal type @SINCE_1_0.0
  typedef Signal< void (bool) > FocusSignalType;         ///< Window focus signal type @SINCE_1_2.60
  typedef Signal< void (WindowSize) > ResizedSignalType; ///< Window resized signal type @SINCE_1_2.60

public:

  // Enumerations

  /**
   * @brief Enumeration for orientation of the window is the way in which a rectangular page is oriented for normal viewing.
   * @SINCE_1_0.0
   */
  enum WindowOrientation
  {
    PORTRAIT = 0,  ///< Portrait orientation. The height of the display area is greater than the width. @SINCE_1_0.0
    LANDSCAPE = 90,  ///< Landscape orientation. A wide view area is needed. @SINCE_1_0.0
    PORTRAIT_INVERSE = 180,  ///< Portrait inverse orientation @SINCE_1_0.0
    LANDSCAPE_INVERSE = 270  ///< Landscape inverse orientation @SINCE_1_0.0
  };

  /**
   * @DEPRECATED_1_4.9
   * @brief Enumeration for opacity of the indicator.
   * @SINCE_1_0.0
   */
  enum IndicatorBgOpacity
  {
    OPAQUE = 100, ///< @DEPRECATED_1_4.9 @brief Fully opaque indicator Bg @SINCE_1_0.0
    TRANSLUCENT = 50, ///< @DEPRECATED_1_4.9 @brief Semi translucent indicator Bg @SINCE_1_0.0
    TRANSPARENT = 0 ///< @DEPRECATED_1_4.9 @brief Fully transparent indicator Bg @SINCE_1_0.0
  };

  /**
   * @DEPRECATED_1_4.9
   * @brief Enumeration for visible mode of the indicator.
   * @SINCE_1_0.0
   */
  enum IndicatorVisibleMode
  {
    INVISIBLE = 0, ///< @DEPRECATED_1_4.9 @brief Hide indicator @SINCE_1_0.0
    VISIBLE = 1, ///< @DEPRECATED_1_4.9 @brief Show indicator @SINCE_1_0.0
    AUTO = 2 ///< @DEPRECATED_1_4.9 @brief Hide in default, will show when necessary @SINCE_1_0.0
  };

  /**
   * @brief An enum of Window types.
   * @SINCE_1_2.60
   */
  enum Type
  {
    NORMAL,           ///< A default window type. Indicates a normal, top-level window. Almost every window will be created with this type. @SINCE_1_2.60
    NOTIFICATION,     ///< A notification window, like a warning about battery life or a new E-Mail received. @SINCE_1_2.60
    UTILITY,          ///< A persistent utility window, like a toolbox or palette. @SINCE_1_2.60
    DIALOG            ///< Used for simple dialog windows. @SINCE_1_2.60
  };

  /**
   * @brief An enum of screen mode.
   * @SINCE_1_2.60
   */
  struct NotificationLevel
  {
    /**
     * @brief An enum of screen mode.
     * @SINCE_1_2.60
     */
    enum Type
    {
      NONE   = -1,    ///< No notification level. Default level. This value makes the notification window place in the layer of the normal window. @SINCE_1_2.60
      BASE   = 10,    ///< Base notification level. @SINCE_1_2.60
      MEDIUM = 20,    ///< Higher notification level than base. @SINCE_1_2.60
      HIGH   = 30,    ///< Higher notification level than medium. @SINCE_1_2.60
      TOP    = 40     ///< The highest notification level. @SINCE_1_2.60
    };
  };

  /**
   * @brief An enum of screen mode.
   * @SINCE_1_2.60
   */
  struct ScreenOffMode
  {
    /**
     * @brief An enum of screen mode.
     * @SINCE_1_2.60
     */
    enum Type
    {
      TIMEOUT,              ///< The mode which turns the screen off after a timeout. @SINCE_1_2.60
      NEVER,                ///< The mode which keeps the screen turned on. @SINCE_1_2.60
    };

    static constexpr Type DEFAULT { TIMEOUT }; ///< The default mode. @SINCE_1_2.60
  };

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
   * @brief Adds a child Actor to the Window.
   *
   * The child will be referenced.
   *
   * @SINCE_1_4.19
   * @param[in] actor The child
   * @pre The actor has been initialized.
   * @pre The actor does not have a parent.
   */
  void Add( Actor actor );

  /**
   * @brief Removes a child Actor from the Window.
   *
   * The child will be unreferenced.
   *
   * @SINCE_1_4.19
   * @param[in] actor The child
   * @pre The actor has been added to the stage.
   */
  void Remove( Actor actor );

  /**
   * @brief Sets the background color of the Window.
   *
   * @SINCE_1_4.19
   * @param[in] color The new background color
   */
  void SetBackgroundColor( const Vector4& color );

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
  Layer GetLayer( uint32_t depth ) const;

  /**
   * @DEPRECATED_1_4.9
   * @brief This sets whether the indicator bar should be shown or not.
   * @SINCE_1_0.0
   * @param[in] visibleMode Visible mode for indicator bar, VISIBLE in default
   */
  void ShowIndicator( IndicatorVisibleMode visibleMode ) DALI_DEPRECATED_API;

  /**
   * @DEPRECATED_1_4.9
   * @brief This sets the opacity mode of indicator bar.
   * @SINCE_1_0.0
   * @param[in] opacity The opacity mode
   */
  void SetIndicatorBgOpacity( IndicatorBgOpacity opacity ) DALI_DEPRECATED_API;

  /**
   * @DEPRECATED_1_4.9
   * @brief This sets the orientation of indicator bar.
   *
   * It does not implicitly show the indicator if it is currently hidden.
   * @SINCE_1_0.0
   * @param[in] orientation The orientation
   */
  void RotateIndicator(WindowOrientation orientation) DALI_DEPRECATED_API;

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
  void AddAvailableOrientation( WindowOrientation orientation );

  /**
   * @brief Removes an orientation from the list of available orientations.
   * @SINCE_1_0.0
   * @param[in] orientation The available orientation to remove
   */
  void RemoveAvailableOrientation( WindowOrientation orientation );

  /**
   * @brief Sets a preferred orientation.
   * @SINCE_1_0.0
   * @param[in] orientation The preferred orientation
   * @pre Orientation is in the list of available orientations.
   */
  void SetPreferredOrientation( WindowOrientation orientation );

  /**
   * @brief Gets the preferred orientation.
   * @SINCE_1_0.0
   * @return The preferred orientation if previously set, or none
   */
  WindowOrientation GetPreferredOrientation();

  /**
   * @DEPRECATED_1_4.19 Was not intended for Application developers
   * @brief Returns an empty handle.
   * @note  Not intended for application developers.
   * @SINCE_1_0.0
   * @return An empty handle
   */
  DragAndDropDetector GetDragAndDropDetector() const DALI_DEPRECATED_API;

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
  void SetAcceptFocus( bool accept );

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
  std::string GetSupportedAuxiliaryHint( unsigned int index ) const;

  /**
   * @brief Creates an auxiliary hint of the window.
   * @SINCE_1_2.60
   * @param[in] hint The auxiliary hint string.
   * @param[in] value The value string.
   * @return The ID of created auxiliary hint, or @c 0 on failure.
   */
  unsigned int AddAuxiliaryHint( const std::string& hint, const std::string& value );

  /**
   * @brief Removes an auxiliary hint of the window.
   * @SINCE_1_2.60
   * @param[in] id The ID of the auxiliary hint.
   * @return True if no error occurred, false otherwise.
   */
  bool RemoveAuxiliaryHint( unsigned int id );

  /**
   * @brief Changes a value of the auxiliary hint.
   * @SINCE_1_2.60
   * @param[in] id The auxiliary hint ID.
   * @param[in] value The value string to be set.
   * @return True if no error occurred, false otherwise.
   */
  bool SetAuxiliaryHintValue( unsigned int id, const std::string& value );

  /**
   * @brief Gets a value of the auxiliary hint.
   * @SINCE_1_2.60
   * @param[in] id The auxiliary hint ID.
   * @return The string value of the auxiliary hint ID, or an empty string if none exists.
   */
  std::string GetAuxiliaryHintValue( unsigned int id ) const;

  /**
   * @brief Gets a ID of the auxiliary hint string.
   * @SINCE_1_2.60
   * @param[in] hint The auxiliary hint string.
   * @return The ID of the auxiliary hint string, or @c 0 if none exists.
   */
  unsigned int GetAuxiliaryHintId( const std::string& hint ) const;

  /**
   * @brief Sets a region to accept input events.
   * @SINCE_1_2.60
   * @param[in] inputRegion The region to accept input events.
   */
  void SetInputRegion( const Rect< int >& inputRegion );

  /**
   * @brief Sets a window type.
   * @SINCE_1_2.60
   * @param[in] type The window type.
   * @remarks The default window type is NORMAL.
   */
  void SetType( Type type );

  /**
   * @brief Gets a window type.
   * @SINCE_1_2.60
   * @return A window type.
   */
  Type GetType() const;

  /**
   * @brief Sets a priority level for the specified notification window.
   * @SINCE_1_2.60
   * @param[in] level The notification window level.
   * @return True if no error occurred, false otherwise.
   * @PRIVLEVEL_PUBLIC
   * @PRIVILEGE_WINDOW_PRIORITY
   * @remarks This can be used for a notification type window only. The default level is NotificationLevel::NONE.
   */
  bool SetNotificationLevel( NotificationLevel::Type level );

  /**
   * @brief Gets a priority level for the specified notification window.
   * @SINCE_1_2.60
   * @return The notification window level.
   * @remarks This can be used for a notification type window only.
   */
  NotificationLevel::Type GetNotificationLevel() const;

  /**
   * @brief Sets a transparent window's visual state to opaque.
   * @details If a visual state of a transparent window is opaque,
   * then the window manager could handle it as an opaque window when calculating visibility.
   * @SINCE_1_2.60
   * @param[in] opaque Whether the window's visual state is opaque.
   * @remarks This will have no effect on an opaque window.
   * It doesn't change transparent window to opaque window but lets the window manager know the visual state of the window.
   */
  void SetOpaqueState( bool opaque );

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
   * If the application sets the screen mode to #::Dali::Window::ScreenOffMode::NEVER to its window and the window is shown,
   * the window manager requests the display system to keep the display on as long as the window is shown.
   * If the window is no longer shown, then the window manager requests the display system to go back to normal operation.
   * @SINCE_1_2.60
   * @param[in] screenOffMode The screen mode.
   * @return True if no error occurred, false otherwise.
   * @PRIVLEVEL_PUBLIC
   * @PRIVILEGE_DISPLAY
   */
  bool SetScreenOffMode(ScreenOffMode::Type screenOffMode);

  /**
   * @brief Gets a screen off mode of the window.
   * @SINCE_1_2.60
   * @return The screen off mode.
   */
  ScreenOffMode::Type GetScreenOffMode() const;

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
  bool SetBrightness( int brightness );

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
  void SetSize( WindowSize size );

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
  void SetPosition( WindowPosition position );

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
  void SetTransparency( bool transparent );

public: // Signals
  /**
   * @DEPRECATED_1_4.9
   * @brief The user should connect to this signal to get a timing when indicator was shown / hidden.
   * @SINCE_1_0.0
   * @return The signal to connect to
   */
  IndicatorSignalType& IndicatorVisibilityChangedSignal() DALI_DEPRECATED_API;

  /**
   * @brief The user should connect to this signal to get a timing when window gains focus or loses focus.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName( bool focusIn );
   * @endcode
   * The parameter is true if window gains focus, otherwise false.
   *
   * @SINCE_1_2.60
   * @return The signal to connect to
   */
  FocusSignalType& FocusChangedSignal();

  /**
   * @brief This signal is emitted when the window is resized.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName( int width, int height );
   * @endcode
   * The parameters are the resized width and height.
   *
   * @SINCE_1_2.60
   * @return The signal to connect to
   */
  ResizedSignalType& ResizedSignal();

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief This constructor is used by Dali::Application::GetWindow().
   * @SINCE_1_0.0
   * @param[in] window A pointer to the Window
   */
  explicit DALI_INTERNAL Window( Internal::Adaptor::Window* window );
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // __DALI_WINDOW_H__
