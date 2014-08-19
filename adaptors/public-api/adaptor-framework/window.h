#ifndef __DALI_WINDOW_H__
#define __DALI_WINDOW_H__

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

namespace Dali DALI_IMPORT_API
{
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
 * It has an orientation
 * and indicator properties.
 */
class Window : public BaseHandle
{
public:

  // Enumerations

  /**
   * @brief Orientation of the window.
   */
  enum WindowOrientation
  {
    PORTRAIT = 0,
    LANDSCAPE = 90,
    PORTRAIT_INVERSE = 180,
    LANDSCAPE_INVERSE = 270
  };

  /**
   * @brief Opacity of the indicator.
   */
  enum IndicatorBgOpacity
  {
    OPAQUE = 100, // Fully opaque indicator Bg
    TRANSLUCENT = 50, // Semi translucent indicator Bg
    TRANSPARENT = 0 // Fully transparent indicator Bg
  };

  /**
   * @brief Visible mode of the indicator.
   */
  enum IndicatorVisibleMode
  {
    INVISIBLE = 0, // hide indicator
    VISIBLE = 1, // show indicator
    AUTO = 2 // hide in default, will show when necessary
  };

  /**
   * @brief Style of the indicator.
   */
  enum IndicatorStyle
  {
    FIXED_COLOR = 0, // fixed color style
    CHANGEABLE_COLOR // changeable color style
  };

  // Methods

  /**
   * @brief Create an initialized handle to a new Window.
   * @param[in] windowPosition The position and size of the window
   * @param[in] name The window title
   * @param[in] isTransparent Whether window is transparent
   * @return a new window
   */
  static Window New(PositionSize windowPosition, std::string name, bool isTransparent = false);

  /**
   * @brief Create an uninitalized handle.
   *
   * This can be initialized using Dali::Application::GetWindow() or
   * Dali::Window::New()
   */
  Window();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~Window();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param [in] handle A reference to the copied handle
   */
  Window(const Window& handle);

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param [in] rhs  A reference to the copied handle
   * @return A reference to this
   */
  Window& operator=(const Window& rhs);

  /**
   * @brief This method is defined to allow assignment of the NULL value,
   * and will throw an exception if passed any other value.
   *
   * Assigning to NULL is an alias for Reset().
   * @param [in] rhs  A NULL pointer
   * @return A reference to this handle
   */
  Window& operator=(BaseHandle::NullType* rhs);

  /**
   * @brief This sets the style of indicator
   * @param[in] style style type of the indicator
   *
   * @note This should be called before ShowIndicator()
   */
  void SetIndicatorStyle( IndicatorStyle style );

  /**
   * @brief This sets whether the indicator bar should be shown or not.
   * @param[in] visibleMode visible mode for indicator bar, VISIBLE in default
   */
  void ShowIndicator( IndicatorVisibleMode visibleMode );

  /**
   * @brief This sets the opacity mode of indicator bar.
   * @param[in] opacity - The opacity mode
   */
  void SetIndicatorBgOpacity( IndicatorBgOpacity opacity );

  /**
   * @brief This sets the orientation of indicator bar.
   *
   * It does not implicitly show the indicator if it is currently
   * hidden.
   * @param[in] orientation The orientation
   */
  void RotateIndicator(WindowOrientation orientation);

  /**
   * @brief Set the window name and class string.
   * @param[in] name The name of the window
   * @param[in] klass The class of the window
   */
  void SetClass(std::string name, std::string klass);

  /**
   * @brief Raise window to top of window stack.
   */
  void Raise();

  /**
   * @brief Lower window to bottom of window stack.
   */
  void Lower();

  /**
   * @brief Activate window to top of window stack even it is iconified.
   */
  void Activate();

  /**
   * @brief Get the orientation class ( to allow signal connection ).
   */
  Orientation GetOrientation();

  /**
   * @brief Add an orientation to the list of available orientations.
   */
  void AddAvailableOrientation( WindowOrientation orientation );

  /**
   * @brief Remove an orientation from the list of available orientations.
   */
  void RemoveAvailableOrientation( WindowOrientation orientation );

  /**
   * @brief Set the orientations that this window can rotate to.
   *
   * By default, the window does not change orientation.
   * @param[in] orientations The list of orientations
   */
  void SetAvailableOrientations( const std::vector<WindowOrientation>& orientations );

  /**
   * @brief Get the list of orientations this window can rotate to.
   * @return the list of orientations
   */
  const std::vector<WindowOrientation>& GetAvailableOrientations();

  /**
   * @brief Set a preferred orientation.
   * @pre orientation is in the list of available orientations
   * @param[in] orientation The preferred orientation
   */
  void SetPreferredOrientation( WindowOrientation orientation );

  /**
   * @brief Get the preferred orientation.
   * @return The preferred orientation if previously set, or none.
   */
  WindowOrientation GetPreferredOrientation();

  /**
   * @brief Returns the Drag & drop detector which can be used to receive drag & drop events.
   * @return A handle to the DragAndDropDetector.
   */
  DragAndDropDetector GetDragAndDropDetector() const;

public: // Not intended for application developers
  /**
   * @brief This constructor is used by Dali::Application::GetWindow().
   * @param[in] window A pointer to the window.
   */
  explicit Window( Internal::Adaptor::Window* window );
};

} // namespace Dali

#endif // __DALI_WINDOW_H__
