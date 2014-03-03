#ifndef __DALI_DRAG_AND_DROP_DETECTOR_H__
#define __DALI_DRAG_AND_DROP_DETECTOR_H__

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
#include <boost/function.hpp>

#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal-v2.h>

namespace Dali DALI_IMPORT_API
{

namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class DragAndDropDetector;
}
}

/**
 * The DragAndDropDetectors provides signals when draggable objects are dragged into our window.
 * It provides signals for when the draggable object enters our window, moves around in our window,
 * leaves our window and when it is finally dropped into our window.
 * The basic usage is shown below:
 *
 * @code
 *
 *  void Example()
 *  {
 *    DragAndDropDetector detector( window::GetDragAndDropDetector() );
 *
 *    // Get notifications when the draggable item enters our window
 *    detector.EnteredSignal().Connect( &OnEntered );
 *
 *    // Get notifications when the draggable item leaves our window
 *    detector.ExitedSignal().Connect( &OnExited );
 *
 *    // Get notifications when the draggable item is moved within our window
 *    detector.MovedSignal().Connect( &OnMoved );
 *
 *    // Get notifications when the draggable item is dropped
 *    detector.DroppedSignal().Connect( &OnDropped );
 *  }
 *
 *  void OnEntered( DragAndDropDetector detector )
 *  {
 *    // Change mode as required
 *  }
 *
 *  void OnExited( DragAndDropDetector detector )
 *  {
 *    // Change mode as required
 *  }
 *
 *  void OnMoved( DragAndDropDetector detector )
 *  {
 *    // Query the new values
 *    std::cout << "Position = " << detector.GetCurrentScreenPosition() << std::endl;
 *  }
 *
 *  void OnDropped( DragAndDropDetector detector )
 *  {
 *    // Query the new values
 *    std::cout << "Position = " << detector.GetCurrentScreenPosition() << ", Content = " << detector.GetContent() << std::endl;
 *  }
 *
 * @endcode
 */
class DragAndDropDetector : public BaseHandle
{
public:

  // Typedefs

  // Drag & Drop
  typedef SignalV2< void ( DragAndDropDetector ) > DragAndDropSignalV2;

  // Signal Names
  static const char* const SIGNAL_ENTERED;
  static const char* const SIGNAL_EXITED;
  static const char* const SIGNAL_MOVED;
  static const char* const SIGNAL_DROPPED;

  /**
   * Create an uninitialized handle.
   * This can be initialized by calling getting the detector from Dali::Window.
   */
  DragAndDropDetector();

  /**
   * Virtual Destructor.
   */
  virtual ~DragAndDropDetector();

  /**
   * Returns the dropped content.
   * @return A reference to the string representing the dropped content.
   */
  const std::string& GetContent() const;

  /**
   * Returns the current position of the dragged object.  This is the dropped position when an object
   * is dropped.
   * @return The current screen position.
   */
  Vector2 GetCurrentScreenPosition() const;

public:  // Signals

  /**
   * This is emitted when a dragged object enters a DALi window.
   * A callback of the following type may be connected:
   * @code
   *   void YourCallback( DragAndDropDetector detector );
   * @endcode
   * @return The signal to connect to.
   */
  DragAndDropSignalV2& EnteredSignal();

  /**
   * This is emitted when a dragged object leaves a DALi window.
   * A callback of the following type may be connected:
   * @code
   *   void YourCallback( DragAndDropDetector detector );
   * @endcode
   * @return The signal to connect to.
   */
  DragAndDropSignalV2& ExitedSignal();

  /**
   * This is emitted when a dragged object is moved within the DALi window.
   * A callback of the following type may be connected:
   * @code
   *   void YourCallback( DragAndDropDetector detector );
   * @endcode
   * This will be replaced by a property notification system once that is in place.
   * @return The signal to connect to.
   */
  DragAndDropSignalV2& MovedSignal();

  /**
   * This is emitted when a dragged object is dropped within a DALi window.
   * A callback of the following type may be connected:
   * @code
   *   void YourCallback( DragAndDropDetector detector );
   * @endcode
   * @return The signal to connect to.
   */
  DragAndDropSignalV2& DroppedSignal();

public: // Not intended for application developers

  /**
   * This constructor is used by DragAndDropDetector::Get().
   * @param[in] detector A pointer to the drag and drop detector.
   */
  DragAndDropDetector( Internal::Adaptor::DragAndDropDetector* detector );
};

} // namespace Dali

/**
 * @}
 */
#endif // __DALI_DRAG_AND_DROP_DETECTOR_H__
