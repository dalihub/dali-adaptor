#ifndef __DALI_WINDOW_H__
#define __DALI_WINDOW_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/**
 * @addtogroup CAPI_DALI_FRAMEWORK
 * @{
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
 * The window class is used internally for drawing. It has an orientation
 * and indicator properties.
 */
class Window : public BaseHandle
{
public:

  // Enumerations

  enum WindowOrientation
  {
    PORTRAIT = 0,
    LANDSCAPE = 90,
    PORTRAIT_INVERSE = 180,
    LANDSCAPE_INVERSE = 270
  };

  enum IndicatorBgOpacity
  {
    OPAQUE = 100, // Fully opaque indicator Bg
    TRANSLUCENT = 50, // Semi translucent indicator Bg
    TRANSPARENT = 0 // Fully transparent indicator Bg
  };

  // Methods

  /**
   * Create an initialized handle to a new Window.
   * @param[in] windowPosition The position and size of the window
   * @param[in] name The window title
   * @param[in] isTransparent Whether window is transparent
   */
  static Window New(PositionSize windowPosition, std::string name, bool isTransparent = false);

  /**
   * Create an uninitalized handle. This can be initialized using
   * Dali::Application::GetWindow() or Dali::Window::New()
   */
  Window();

  /**
   * Destructor
   */
  virtual ~Window();

  /**
   * @copydoc Dali::BaseHandle::operator=
   */
  using BaseHandle::operator=;

  /**
   * This sets whether the indicator bar should be shown or not
   * @param[in] show - true if the indicator bar should be shown
   */
  void ShowIndicator( bool show );

  /**
   * This sets the opacity mode of indicator bar
   * @param[in] opacity - The opacity mode
   */
  void SetIndicatorBgOpacity( IndicatorBgOpacity opacity );

  /**
   * This sets the orientation of indicator bar. It does not
   * implicitly show the indicator if it is currently hidden.
   * @param[in] orientation The orientation
   */
  void RotateIndicator(WindowOrientation orientation);

  /**
   * Set the window name and class string
   */
  void SetClass(std::string name, std::string klass);

  /**
   * Raise window to top of window stack
   */
  void Raise();

  /**
   * Lower window to bottom of window stack
   */
  void Lower();

  /**
   * Activate window to top of window stack even it is iconified
   */
  void Activate();

  /**
   * Get the orientation class ( to allow signal connection )
   */
  Orientation GetOrientation();

  /**
   * Add an orientation to the list of available orientations
   */
  void AddAvailableOrientation( WindowOrientation orientation );

  /**
   * Remove an orientation from the list of available orientations
   */
  void RemoveAvailableOrientation( WindowOrientation orientation );

  /**
   * Set the orientations that this window can rotate to. By default, the window
   * does not change orientation.
   * @param[in] orientations The list of orientations
   */
  void SetAvailableOrientations( const std::vector<WindowOrientation>& orientations );

  /**
   * Get the list of orientations this window can rotate to.
   * @return the list of orientations
   */
  const std::vector<WindowOrientation>& GetAvailableOrientations();

  /**
   * Set a preferred orientation.
   * @pre orientation is in the list of available orientations
   * @param[in] orientation The preferred orientation
   */
  void SetPreferredOrientation( WindowOrientation orientation );

  /**
   * Get the preferred orientation
   * @return The preferred orientation if previously set, or none.
   */
  WindowOrientation GetPreferredOrientation();

  /**
   * Returns the Drag & drop detector which can be used to receive drag & drop events.
   * @return A handle to the DragAndDropDetector.
   */
  DragAndDropDetector GetDragAndDropDetector() const;

public: // Not intended for application developers
  /**
   * This constructor is used by Dali::Application::GetWindow().
   * @param[in] window A pointer to the window.
   */
  explicit Window( Internal::Adaptor::Window* window );
};

} // namespace Dali


/**
 * @}
 */
#endif // __DALI_WINDOW_H__
