#ifndef DALI_INTERNAL_LONG_PRESS_GESTURE_DETECTOR_H
#define DALI_INTERNAL_LONG_PRESS_GESTURE_DETECTOR_H

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
#include <map>
#include <dali/public-api/adaptor-framework/timer.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/gesture-detector.h>

namespace Dali
{

namespace Integration
{
struct TouchEvent;
struct LongPressGestureRequest;
}

namespace Internal
{

namespace Adaptor
{

class CoreEventInterface;

/**
 * When given a set of touch events, this detector attempts to determine if a long press gesture has taken place.
 * Emits a LongPressGestureEvent (state = Started) when a long press has been detected (Touch held down for more than duration).
 * Emits a further LongPressGestureEvent (state = Finished) when a long press has been completed (Touch Release).
 */
class LongPressGestureDetector : public GestureDetector
{
public:

  /**
   * Constructor
   * @param[in] coreEventInterface Used to send events to Core.
   * @param[in] screenSize  The size of the screen.
   * @param[in] request     The long press gesture request.
   */
  LongPressGestureDetector(CoreEventInterface& coreEventInterface, Vector2 screenSize, const Integration::LongPressGestureRequest& request);

  /**
   * Virtual destructor.
   */
  virtual ~LongPressGestureDetector();

public:

  /**
   * @copydoc Dali::Internal::GestureDetector::SendEvent(const Integration::TouchEvent&)
   */
  virtual void SendEvent(const Integration::TouchEvent& event);

  /**
   * @copydoc Dali::Internal::GestureDetector::Update(const Integration::GestureRequest&)
   */
  virtual void Update(const Integration::GestureRequest& request);

private:

  /**
   * Timer Callback
   * @return will return false; one-shot timer.
   */
  bool TimerCallback();

  /**
   * Emits the long press gesture if all conditions are applicable.
   * @param[in] state The state of this gesture event.
   */
  void EmitGesture(Gesture::State state);

  /**
   * Get current system setting value for tap and hold gesture
   * @return system value for tap and hold gesture [ms]
   */
  int GetSystemValue();

private:

  /**
   * Internal state machine.
   */
  enum State
  {
    Clear,      ///< No gesture detected.
    Touched,    ///< User is touching the screen.
    Failed,     ///< Gesture has failed.
    Finished    ///< Gesture has been detected and sent.
  };

  CoreEventInterface& mCoreEventInterface; ///< Used to send events to Core.
  State mState; ///< The current state of the detector.

  unsigned int mMinimumTouchesRequired;   ///< The minimum touches required before emitting a long press.
  unsigned int mMaximumTouchesRequired;   ///< The maximum touches allowable. Any more and a long press is not emitted.

  std::map<int, Vector2> mTouchPositions; ///< A map with all the touch down positions.
  uint32_t mTouchTime;               ///< The time we first pressed down.

  Dali::Timer mTimer;                     ///< The timer used to determine a long press.
  SlotDelegate< LongPressGestureDetector > mTimerSlot;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_LONG_PRESS_GESTURE_DETECTOR_H
