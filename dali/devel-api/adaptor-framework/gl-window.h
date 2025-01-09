#ifndef DALI_GL_WINDOW_H
#define DALI_GL_WINDOW_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/math/uint-16-pair.h>
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/math/vector4.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal.h>
#include <string>

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
class GlWindow;
}
} // namespace DALI_INTERNAL

class TouchEvent;
class KeyEvent;

/**
 * @brief The GlWindow class is to draw with native GLES.
 *
 * This class is the special window. It is for native GLES application.
 * So, some special funtions and type are supported.
 * In addition, basic window's functions are supported, too.
 *
 */
class DALI_ADAPTOR_API GlWindow : public BaseHandle
{
public:
  using WindowSize = Uint16Pair;

  typedef Signal<void(const KeyEvent&)>   KeyEventSignalType;          ///< GlWindow Key Event signal type
  typedef Signal<void(const TouchEvent&)> TouchEventSignalType;        ///< GlWindow Touch Event signal type
  typedef Signal<void(GlWindow, bool)>    FocusChangeSignalType;       ///< GlWindow Focus signal type
  typedef Signal<void(WindowSize)>        ResizeSignalType;            ///< GlWindow resize signal type
  typedef Signal<void(GlWindow, bool)>    VisibilityChangedSignalType; ///< GlWindow visibility change signal type

public:
  // Enumerations

  /**
   * @brief Enumeration for GLES verion
   *
   * This Enumeration is used the GLES version for EGL configuration.
   * If the device can not support GLES version 3.0 over, the version will be chosen with GLES version 2.0
   *
   */
  enum class GlesVersion
  {
    VERSION_2_0 = 0, ///< GLES version 2.0
    VERSION_3_0,     ///< GLES version 3.0
  };

  /**
   * @brief Enumeration for rendering mode
   *
   * This Enumeration is used to choose the rendering mode.
   * It has two options.
   * One of them is continuous mode. It is rendered continuously.
   * The other is on demand mode. It is rendered by application.
   */
  enum class RenderingMode
  {
    CONTINUOUS, ///< continuous mode
    ON_DEMAND   ///< on demand by application
  };

  /**
   * @brief Creates an initialized handle to a new GlWindow.
   *
   * @return A new GlWindow
   * @note This creates an extra GlWindow in addition to the default main GlWindow
   */
  static GlWindow New();

  /**
   * @brief Creates an initialized handle to a new GlWindow.
   *
   * @param[in] positionSize The position and size of the GlWindow
   * @param[in] name The GlWindow title
   * @param[in] className The GlWindow class name
   * @param[in] isTransparent Whether GlWindow is transparent
   * @note This creates an extra GlWindow in addition to the default main GlWindow
   * @return A new GlWindow
   */
  static GlWindow New(PositionSize positionSize, const std::string& name, const std::string& className, bool isTransparent = false);

  /**
   * @brief Creates an uninitialized handle.
   *
   * This can be initialized using Dali::Application::GetGlWindow() or
   * Dali::GlWindow::New().
   *
   */
  GlWindow();

  /**
   * @brief Destructor.
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   *
   */
  ~GlWindow();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param[in] handle A reference to the copied handle
   */
  GlWindow(const GlWindow& handle);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param[in] rhs A reference to the copied handle
   * @return A reference to this
   */
  GlWindow& operator=(const GlWindow& rhs);

  /**
   * @brief Move constructor.
   *
   * @param[in] rhs A reference to the moved handle
   */
  GlWindow(GlWindow&& rhs);

  /**
   * @brief Move assignment operator.
   *
   * @param[in] rhs A reference to the moved handle
   * @return A reference to this handle
   */
  GlWindow& operator=(GlWindow&& rhs);

  /**
   * @brief Sets graphics configuration for GlWindow
   *
   * @param[in] depth the flag of depth buffer. If true is set, 24bit depth buffer is enabled.
   * @param[in] stencil the flag of stencil. it true is set, 8bit stencil buffer is enabled.
   * @param[in] msaa the bit of msaa.
   * @param[in] version the GLES version
   *
   */
  void SetGraphicsConfig(bool depth, bool stencil, int msaa, GlesVersion version);

  /**
   * @brief Raises GlWindow to the top of GlWindow stack.
   *
   */
  void Raise();

  /**
   * @brief Lowers GlWindow to the bottom of GlWindow stack.
   *
   */
  void Lower();

  /**
   * @brief Activates GlWindow to the top of GlWindow stack even it is iconified.
   *
   */
  void Activate();

  /**
   * @brief Shows the GlWindow if it is hidden.
   *
   */
  void Show();

  /**
   * @brief Hides the GlWindow if it is showing.
   *
   */
  void Hide();

  /**
   * @brief Sets a position of the GlWindow.
   *
   * @param[in] positionSize The new GlWindow position
   */
  void SetPositionSize(PositionSize positionSize);

  /**
   * @brief Gets a position of the GlWindow.
   *
   * @return The position of the GlWindow
   */
  PositionSize GetPositionSize() const;

  /**
   * @brief Gets the count of supported auxiliary hints of the window.
   *
   * @return The number of supported auxiliary hints.
   *
   * @note The window auxiliary hint is the value which is used to decide which actions should be made available to the user by the window manager.
   * If you want to set specific hint to your window, then you should check whether it exists in the supported auxiliary hints.
   */
  unsigned int GetSupportedAuxiliaryHintCount() const;

  /**
   * @brief Gets the supported auxiliary hint string of the window.
   *
   * @param[in] index The index of the supported auxiliary hint lists
   * @return The auxiliary hint string of the index.
   *
   * @note The window auxiliary hint is the value which is used to decide which actions should be made available to the user by the window manager.
   * If you want to set specific hint to your window, then you should check whether it exists in the supported auxiliary hints.
   */
  std::string GetSupportedAuxiliaryHint(unsigned int index) const;

  /**
   * @brief Creates an auxiliary hint of the window.
   *
   * @param[in] hint The auxiliary hint string.
   * @param[in] value The value string.
   * @return The ID of created auxiliary hint, or @c 0 on failure.
   */
  unsigned int AddAuxiliaryHint(const std::string& hint, const std::string& value);

  /**
   * @brief Removes an auxiliary hint of the window.
   *
   * @param[in] id The ID of the auxiliary hint.
   * @return True if no error occurred, false otherwise.
   */
  bool RemoveAuxiliaryHint(unsigned int id);

  /**
   * @brief Changes a value of the auxiliary hint.
   *
   * @param[in] id The auxiliary hint ID.
   * @param[in] value The value string to be set.
   * @return True if no error occurred, false otherwise.
   */
  bool SetAuxiliaryHintValue(unsigned int id, const std::string& value);

  /**
   * @brief Gets a value of the auxiliary hint.
   *
   * @param[in] id The auxiliary hint ID.
   * @return The string value of the auxiliary hint ID, or an empty string if none exists.
   */
  std::string GetAuxiliaryHintValue(unsigned int id) const;

  /**
   * @brief Gets a ID of the auxiliary hint string.
   *
   * @param[in] hint The auxiliary hint string.
   * @return The ID of the auxiliary hint string, or @c 0 if none exists.
   */
  unsigned int GetAuxiliaryHintId(const std::string& hint) const;

  /**
   * @brief Sets a region to accept input events.
   *
   * @param[in] inputRegion The region to accept input events.
   */
  void SetInputRegion(const Rect<int>& inputRegion);

  /**
   * @brief Sets a transparent window's visual state to opaque.
   * @details If a visual state of a transparent window is opaque,
   * then the window manager could handle it as an opaque window when calculating visibility.
   *
   * @param[in] opaque Whether the window's visual state is opaque.
   * @remarks This will have no effect on an opaque window.
   * It doesn't change transparent window to opaque window but lets the window manager know the visual state of the window.
   */
  void SetOpaqueState(bool opaque);

  /**
   * @brief Returns whether a transparent window's visual state is opaque or not.
   *
   * @return True if the window's visual state is opaque, false otherwise.
   * @remarks The return value has no meaning on an opaque window.
   */
  bool IsOpaqueState() const;

  /**
   * @brief Gets current rotation angle of the window.
   *
   * @return The current GlWindow rotation angle if previously set, or none
   */
  WindowOrientation GetCurrentOrientation() const;

  /**
    * @brief Sets available orientations of the window.
    *
    * This API is for setting several orientations one time.
    *
    * @param[in] orientations The available orientations list to add
    */
  void SetAvailableOrientations(const Dali::Vector<WindowOrientation>& orientations);

  /**
   * @brief Sets a preferred orientation.
   *
   * @param[in] orientation The preferred orientation
   * @pre angle is in the list of available orientation.
   *
   * @note To unset the preferred orientation, angle should be set NO_ORIENTATION_PREFERENCE.
   */
  void SetPreferredOrientation(WindowOrientation orientation);

  /**
   * @brief Registers a GL callback function for application.
   *
   * @param[in] initCallback  the callback function for application initialize
   * @param[in] renderFrameCallback the callback function to render for the frame.
   * @param[in] terminateCallback the callback function to clean-up application GL resource.
   *
   * @note Function must be called on idle time
   *
   * A initCallback of the following type should be used:
   * @code
   *   void intializeGL();
   * @endcode
   * This callback will be called before renderFrame callback is called at once.
   *
   * A renderFrameCallback of the following type should be used:
   * @code
   *   int renderFrameGL();
   * @endcode
   * This callback's return value is not 0, the eglSwapBuffers() will be called.
   *
   * A terminateCallback of the following type should be used:
   * @code
   *   void terminateGL();
   * @endcode
   * This callback is called when GlWindow is deleted.
   */
  void RegisterGlCallbacks(CallbackBase* initCallback, CallbackBase* renderFrameCallback, CallbackBase* terminateCallback);

  /**
   * @brief Renders once more even if GL render functions are not added to idler.
   * @note Will not work if the window is hidden or GL render functions are added to idler
   *
   */
  void RenderOnce();

  /**
   * @brief Sets rendering mode.
   *
   * @param[in] mode the rendering mode for GlWindow
   *
   * @note The default Rendering mode is continuous.
   * If OnDemand mode is set, it is rendered by RenderOnce()
   */
  void SetRenderingMode(RenderingMode mode);

  /**
   * @brief Gets rendering mode.
   *
   * @return current rendering mode in this GlWindow
   *
   * @note The default Rendering mode is continuous.
   * If OnDemand mode is set, it is rendered by RenderOnce()
   */
  RenderingMode GetRenderingMode() const;

public: // Signals
  /**
   * @brief The user should connect to this signal to get a timing when GlWindow gains focus or loses focus.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName( GlWindow GlWindow, bool focusIn );
   * @endcode
   * The parameter is true if GlWindow gains focus, otherwise false.
   * and GlWindow means this signal was called from what GlWindow
   *
   * @return The signal to connect to
   */
  FocusChangeSignalType& FocusChangeSignal();

  /**
   * @brief This signal is emitted when the GlWindow is resized.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName( GlWindow GlWindow, int width, int height );
   * @endcode
   * The parameters are the resized width and height.
   * and GlWindow means this signal was called from what GlWindow
   *
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
   * @return The touch signal to connect to
   *
   * @note Motion events are not emitted.
   */
  TouchEventSignalType& TouchedSignal();

  /**
   * @brief This signal is emitted when the window is shown or hidden.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName( Window window, bool visible );
   * @endcode
   *
   * @return The signal to connect to
   */
  VisibilityChangedSignalType& VisibilityChangedSignal();

public: // Not intended for application developers
  /// @cond internal
  /**
   * @brief This constructor is used by Dali::Application::GetGlWindow().
   * @param[in] GlWindow A pointer to the GlWindow
   */
  explicit GlWindow(Internal::Adaptor::GlWindow* GlWindow);
  /// @endcond
};

/**
 * @}
 */
} // namespace Dali

#endif // DALI_GL_WINDOW_H
