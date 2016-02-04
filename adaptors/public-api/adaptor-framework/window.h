#ifndef __DALI_WINDOW_H__
#define __DALI_WINDOW_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/object/any.h>
#include <dali/public-api/signals/dali-signal.h>

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

/**
 * @brief The window class is used internally for drawing.
 *
 * A Window has an orientation and indicator properties.
 * You can get a valid Window handle by calling Dali::Application::GetWindow().
 * @SINCE_1_0.0
 */
class DALI_IMPORT_API Window : public BaseHandle
{
public:
  typedef Signal< void (bool) > IndicatorSignalType;

public:

  // Enumerations

  /**
   * @brief Orientation of the window.
   * @SINCE_1_0.0
   */
  enum WindowOrientation
  {
    PORTRAIT = 0,  ///< Portrait orientation @SINCE_1_0.0
    LANDSCAPE = 90,  ///< Landscape orientation @SINCE_1_0.0
    PORTRAIT_INVERSE = 180,  ///< Portrait inverse orientation @SINCE_1_0.0
    LANDSCAPE_INVERSE = 270  ///< Landscape inverse orientation @SINCE_1_0.0
  };

  /**
   * @brief Opacity of the indicator.
   * @SINCE_1_0.0
   */
  enum IndicatorBgOpacity
  {
    OPAQUE = 100, ///< Fully opaque indicator Bg @SINCE_1_0.0
    TRANSLUCENT = 50, ///< Semi translucent indicator Bg @SINCE_1_0.0
    TRANSPARENT = 0 ///< Fully transparent indicator Bg @SINCE_1_0.0
  };

  /**
   * @brief Visible mode of the indicator.
   * @SINCE_1_0.0
   */
  enum IndicatorVisibleMode
  {
    INVISIBLE = 0, ///< Hide indicator @SINCE_1_0.0
    VISIBLE = 1, ///< Show indicator @SINCE_1_0.0
    AUTO = 2 ///< Hide in default, will show when necessary @SINCE_1_0.0
  };

  // Methods

  /**
   * @brief Create an initialized handle to a new Window.
   * @SINCE_1_0.0
   * @param[in] windowPosition The position and size of the window
   * @param[in] name The window title
   * @param[in] isTransparent Whether window is transparent
   * @return a new window
   */
  static Window New(PositionSize windowPosition, const std::string& name, bool isTransparent = false);

  /**
   * @brief Create an initialized handle to a new Window.
   * @SINCE_1_0.0
   * @param[in] windowPosition The position and size of the window
   * @param[in] name The window title
   * @param[in] className The window class name
   * @param[in] isTransparent Whether window is transparent
   * @return a new window
   */
  static Window New(PositionSize windowPosition, const std::string& name, const std::string& className, bool isTransparent = false);

  /**
   * @brief Create an uninitalized handle.
   *
   * This can be initialized using Dali::Application::GetWindow() or
   * Dali::Window::New()
   * @SINCE_1_0.0
   */
  Window();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   * @SINCE_1_0.0
   */
  ~Window();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @SINCE_1_0.0
   * @param [in] handle A reference to the copied handle
   */
  Window(const Window& handle);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @SINCE_1_0.0
   * @param [in] rhs  A reference to the copied handle
   * @return A reference to this
   */
  Window& operator=(const Window& rhs);

  /**
   * @brief This sets whether the indicator bar should be shown or not.
   * @SINCE_1_0.0
   * @param[in] visibleMode Visible mode for indicator bar, VISIBLE in default
   */
  void ShowIndicator( IndicatorVisibleMode visibleMode );

  /**
   * @brief This sets the opacity mode of indicator bar.
   * @SINCE_1_0.0
   * @param[in] opacity The opacity mode
   */
  void SetIndicatorBgOpacity( IndicatorBgOpacity opacity );

  /**
   * @brief This sets the orientation of indicator bar.
   *
   * It does not implicitly show the indicator if it is currently
   * hidden.
   * @SINCE_1_0.0
   * @param[in] orientation The orientation
   */
  void RotateIndicator(WindowOrientation orientation);

  /**
   * @brief Set the window name and class string.
   * @SINCE_1_0.0
   * @param[in] name The name of the window
   * @param[in] klass The class of the window
   */
  void SetClass(std::string name, std::string klass);

  /**
   * @brief Raise window to top of window stack.
   * @SINCE_1_0.0
   */
  void Raise();

  /**
   * @brief Lower window to bottom of window stack.
   * @SINCE_1_0.0
   */
  void Lower();

  /**
   * @brief Activate window to top of window stack even it is iconified.
   * @SINCE_1_0.0
   */
  void Activate();

  /**
   * @brief Add an orientation to the list of available orientations.
   * @SINCE_1_0.0
   */
  void AddAvailableOrientation( WindowOrientation orientation );

  /**
   * @brief Remove an orientation from the list of available orientations.
   * @SINCE_1_0.0
   */
  void RemoveAvailableOrientation( WindowOrientation orientation );

  /**
   * @brief Set a preferred orientation.
   * @SINCE_1_0.0
   * @param[in] orientation The preferred orientation
   * @pre orientation is in the list of available orientations
   */
  void SetPreferredOrientation( WindowOrientation orientation );

  /**
   * @brief Get the preferred orientation.
   * @SINCE_1_0.0
   * @return The preferred orientation if previously set, or none.
   */
  WindowOrientation GetPreferredOrientation();

  /**
   * @brief Returns the Drag & drop detector which can be used to receive drag & drop events.
   * @SINCE_1_0.0
   * @return A handle to the DragAndDropDetector.
   */
  DragAndDropDetector GetDragAndDropDetector() const;

  /**
   * @brief Get the native handle of the window.
   * @SINCE_1_0.0
   * @return The native handle of the window or an empty handle.
   */
  Any GetNativeHandle() const;

public: // Signals
  /**
   * @brief The user should connect to this signal to get a timing when indicator was shown / hidden.
   * @SINCE_1_0.0
   */
  IndicatorSignalType& IndicatorVisibilityChangedSignal();

public: // Not intended for application developers
  /**
   * @brief This constructor is used by Dali::Application::GetWindow().
   * @SINCE_1_0.0
   * @param[in] window A pointer to the window.
   */
  explicit DALI_INTERNAL Window( Internal::Adaptor::Window* window );
};

/**
 * @}
 */
} // namespace Dali

#endif // __DALI_WINDOW_H__
