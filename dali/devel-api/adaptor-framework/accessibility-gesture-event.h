#ifndef DALI_INTEGRAION_ACCESSIBILITY_GESTURE_STRUCTS_H
#define DALI_INTEGRAION_ACCESSIBILITY_GESTURE_STRUCTS_H

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

// INTERNAL INCLUDES
#include <dali/public-api/events/gesture.h>
#include <dali/public-api/math/vector2.h>

namespace Dali
{

struct AccessibilityGestureEvent
{
  // Construction & Destruction

  /**
   * Virtual destructor
   */
  ~AccessibilityGestureEvent()
  {
  }

  // Data

  /**
   * The previous touch position of the primary touch point in screen coordinates.
   */
  Vector2 previousPosition;

  /**
   * This current touch position of the primary touch point in screen coordinates.
   */
  Vector2 currentPosition;

  /**
   * The time difference between the previous and latest touch motion events (in ms).
   */
  unsigned long timeDelta;

  /**
   * The total number of fingers touching the screen in a pan gesture.
   */
  unsigned int numberOfTouches;

  enum State
  {
    Clear,      ///< There is no state associated with this gesture. @SINCE_1_0.0
    Started,    ///< The touched points on the screen have moved enough to be considered a gesture. @SINCE_1_0.0
    Continuing, ///< The gesture is continuing. @SINCE_1_0.0
    Finished,   ///< The user has lifted a finger or touched an additional point on the screen. @SINCE_1_0.0
    Cancelled,  ///< The gesture has been cancelled. @SINCE_1_0.0
    Possible    ///< A gesture is possible. @SINCE_1_0.0
  };

  State state;

  uint32_t time;

  /**
   * Default Constructor
   * @param[in]  state  The state of the gesture
   */
  AccessibilityGestureEvent(AccessibilityGestureEvent::State state)
  : timeDelta( 0 ),
  numberOfTouches( 0 ),
  state( state ),
  time( 0 )
  {}
};

} // namespace Dali

#endif // DALI_INTEGRAION_ACCESSIBILITY_GESTURE_STRUCTS_H
