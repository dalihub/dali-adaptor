#ifndef DALI_INTERNAL_ACCESSIBILITY_GESTURE_DETECTOR_H
#define DALI_INTERNAL_ACCESSIBILITY_GESTURE_DETECTOR_H

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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-gesture-handler.h>
#include <dali/devel-api/adaptor-framework/accessibility-gesture-event.h>
#include <dali/integration-api/scene.h>
#include <dali/public-api/events/touch-event.h>

namespace Dali
{

namespace Integration
{
struct TouchEvent;
}

namespace Internal
{

namespace Adaptor
{

/**
 * Detects an accessibility pan gesture and sends it to the gesture handler.
 */
class AccessibilityGestureDetector : public RefObject
{
public:

  /**
   * Constructor
   */
  AccessibilityGestureDetector();

  /**
   * Virtual destructor.
   */
  virtual ~AccessibilityGestureDetector();

  /**
   * Set the handler to handle accessibility gestures.
   * @param[in] handler The Accessibility gesture handler.
   * @note Handlers should remove themselves when they are destroyed.
   */
  void SetGestureHandler(AccessibilityGestureHandler& handler);

  void SendEvent(const Integration::TouchEvent& event);

  void SendEvent(Integration::Scene& scene, const Integration::TouchEvent& event)
  {
    mScene = &scene;
    SendEvent(event);
  }

private:

  /**
   * Emits the pan gesture event (performs some smoothing operation).
   * @param[in]  state         The state of the pan.
   * @param[in]  currentEvent  The latest touch event.
   */
  void SendPan(AccessibilityGestureEvent::State state, const Integration::TouchEvent& currentEvent);

  /**
   * Emits the pan gesture event to the gesture handler.
   * @param[in] gesture The pan gesture event.
   */
  void EmitPan(const AccessibilityGestureEvent gesture);

private:

  /**
   * Internal state machine.
   */
  enum State
  {
    Clear,    ///< No gesture detected.
    Possible, ///< The current touch event data suggests that a gesture is possible.
    Started,  ///< A gesture has been detected.
    Finished, ///< A previously started pan gesture has finished.
    Failed,   ///< Current touch event data suggests a pan gesture is not possible.
  };

  State mState; ///< The current state of the detector.

  Integration::Scene* mScene;
  AccessibilityGestureHandler* mGestureHandler; ///< The pointer of accessibility gesture handler
  bool mPanning;    ///< Keep track of panning state, when panning is occuring, this is true.

  std::vector<Integration::TouchEvent> mTouchEvents;     ///< A container of all touch events after an initial down event.

  Vector2 mPrimaryTouchDownLocation;    ///< The initial touch down point.
  Vector2 mThresholdAdjustmentPerFrame; ///< The adjustment per frame at the start of a slow pan.
  Vector2 mPreviousPosition;            ///< The previous position.

  unsigned int mThresholdAdjustmentsRemaining; ///< No. of threshold adjustments still to apply (for a slow-pan).
  unsigned int mThresholdTotalAdjustments;     ///< The total number of adjustments required.

  uint32_t mPrimaryTouchDownTime;       ///< The initial touch down time.
  unsigned int mMinimumTouchesRequired; ///< The minimum touches required before a pan should be emitted.
  unsigned int mMaximumTouchesRequired; ///< The maximum touches after which a pan should not be emitted.
  unsigned int mMinimumDistanceSquared; ///< The minimum distance squared before pan should start.
  unsigned int mMinimumMotionEvents;    ///< The minimum motion events before pan should start.
  unsigned int mMotionEvents;           ///< The motion events received so far (before pan is emitted).
};

using AccessibilityGestureDetectorPtr = Dali::IntrusivePtr<AccessibilityGestureDetector>;

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ACCESSIBILITY_GESTURE_DETECTOR_H
