#ifndef DALI_INTERNAL_TAP_GESTURE_DETECTOR_H
#define DALI_INTERNAL_TAP_GESTURE_DETECTOR_H

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
#include <cstdint> // uint32_t
#include <dali/integration-api/events/tap-gesture-event.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/integration-api/events/point.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/gesture-detector.h>

namespace Dali
{

namespace Integration
{
struct TouchEvent;
struct TapGestureRequest;
}

namespace Internal
{

namespace Adaptor
{

class CoreEventInterface;

/**
 * When given a set of touch events, this detector attempts to determine if a tap gesture has taken place.
 */
class TapGestureDetector : public GestureDetector
{
public:

  /**
   * Constructor
   * @param[in] coreEventInterface Used to send events to Core.
   * @param[in]  screenSize  The size of the screen.
   * @param[in]  request     The tap gesture request.
   */
  TapGestureDetector(CoreEventInterface& coreEventInterface, Vector2 screenSize, const Integration::TapGestureRequest& request);

  /**
   * Virtual destructor.
   */
  virtual ~TapGestureDetector();

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
   * Checks if registered taps are within required bounds and emits tap gesture if they are.
   *
   * @param[in] state current state of incomplete gesture
   * @param[in] time time of this latest touch event
   */
  void EmitGesture( Gesture::State state, uint32_t time );

  /**
   * Initialises tap gesture detector for next tap sequence
   *
   * @param[in] event registered touch event
   * @param[in] point position touch event occurred
   */
  void SetupForTouchDown( const Integration::TouchEvent& event, const Integration::Point& point );

  /**
   * Emit a touch down event for hit testing
   *
   * @param[in] event registered touch event
   */
  void EmitPossibleState( const Integration::TouchEvent& event );

  /**
   * Force a touch event sequence to be treated as a single tap
   *
   * @param[in] time time of this latest touch event
   * @param[in] point position touch event occurred
    */
  void EmitSingleTap( uint32_t time, const Integration::Point& point );

  /**
   * Emit a tap event
   *
   * @param[in] time time of this latest touch event
   * @param[in] event registered touch event
   */
  void EmitTap( uint32_t time, Integration::TapGestureEvent& event );

private:

  /**
   * Internal state machine.
   */
  enum State
  {
    Clear,      ///< No gesture detected.
    Touched,    ///< User is touching the screen.
    Registered, ///< At least one tap has been registered.
    Failed,     ///< Gesture has failed.
  };

  CoreEventInterface& mCoreEventInterface; ///< Used to send events to Core.
  State mState; ///< Current state of the detector.

  int mMinimumTapsRequired; ///< Minimum number of taps required.
  int mMaximumTapsRequired; ///< Maximum number of taps required.
  int mTapsRegistered;      ///< In current detection, the number of taps registered.

  Vector2 mTouchPosition;   ///< The initial touch down position.
  uint32_t mTouchTime;      ///< The initial touch down time.
  uint32_t mLastTapTime;    ///< Time last tap gesture was registered

};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_TAP_GESTURE_DETECTOR_H
