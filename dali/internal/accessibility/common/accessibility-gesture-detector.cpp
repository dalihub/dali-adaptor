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

// CLASS HEADER
#include <dali/internal/accessibility/common/accessibility-gesture-detector.h>

// EXTERNAL INCLUDES
#include <cmath>

#include <dali/public-api/events/touch-point.h>

#include <dali/integration-api/events/touch-event-integ.h>


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
  const float MINIMUM_MOTION_DISTANCE_BEFORE_PAN( 15.0f );
  const float MINIMUM_MOTION_DISTANCE_BEFORE_PAN_SQUARED( MINIMUM_MOTION_DISTANCE_BEFORE_PAN * MINIMUM_MOTION_DISTANCE_BEFORE_PAN );
  const float MINIMUM_MOTION_DISTANCE_TO_THRESHOLD_ADJUSTMENTS_RATIO( 2.0f / 3.0f );
  const unsigned long MAXIMUM_TIME_DIFF_ALLOWED( 500 );
  const unsigned long MINIMUM_TIME_BEFORE_THRESHOLD_ADJUSTMENTS( 100 );
  const unsigned int MINIMUM_MOTION_EVENTS_BEFORE_PAN(2);
  const unsigned int MINIMUM_TOUCHES_BEFORE_PAN(1);
  const unsigned int MAXIMUM_TOUCHES_BEFORE_PAN(1);
} // unnamed namespace


AccessibilityGestureDetector::AccessibilityGestureDetector()
: mState( Clear ),
  mScene(nullptr),
  mGestureHandler(nullptr),
  mPanning(false),
  mThresholdAdjustmentsRemaining( 0 ),
  mThresholdTotalAdjustments( MINIMUM_MOTION_DISTANCE_BEFORE_PAN * MINIMUM_MOTION_DISTANCE_TO_THRESHOLD_ADJUSTMENTS_RATIO ),
  mPrimaryTouchDownTime( 0 ),
  mMinimumTouchesRequired( MINIMUM_TOUCHES_BEFORE_PAN ),
  mMaximumTouchesRequired( MAXIMUM_TOUCHES_BEFORE_PAN ),
  mMinimumDistanceSquared( MINIMUM_MOTION_DISTANCE_BEFORE_PAN_SQUARED ),
  mMinimumMotionEvents( MINIMUM_MOTION_EVENTS_BEFORE_PAN ),
  mMotionEvents( 0 )
{
}

AccessibilityGestureDetector::~AccessibilityGestureDetector()
{
}

void AccessibilityGestureDetector::SetGestureHandler(AccessibilityGestureHandler& handler)
{
  mGestureHandler = &handler;
}

void AccessibilityGestureDetector::EmitPan(const AccessibilityGestureEvent gesture)
{
  if( mGestureHandler )
  {
    if(gesture.state == AccessibilityGestureEvent::Started)
    {
      mPanning = true;
    }

    if( mPanning )
    {
      mGestureHandler->HandlePanGesture(gesture);

      if( (gesture.state == AccessibilityGestureEvent::Finished) ||
          (gesture.state == AccessibilityGestureEvent::Cancelled) )
      {
        mPanning = false;
      }
    }
  }
}

void AccessibilityGestureDetector::SendEvent(const Integration::TouchEvent& event)
{
  PointState::Type primaryPointState(event.points[0].GetState());

  if (primaryPointState == PointState::INTERRUPTED)
  {
    if ( ( mState == Started ) || ( mState == Possible ) )
    {
      // If our pan had started and we are interrupted, then tell Core that pan is cancelled.
      mTouchEvents.push_back(event);
      SendPan(AccessibilityGestureEvent::Cancelled, event);
    }
    mState = Clear; // We should change our state to Clear.
    mTouchEvents.clear();
  }
  else
  {
    switch (mState)
    {
      case Clear:
      {
        if ( ( primaryPointState == PointState::DOWN ) || ( primaryPointState == PointState::STATIONARY ) )
        {
          mPrimaryTouchDownLocation = event.points[0].GetScreenPosition();
          mPrimaryTouchDownTime = event.time;
          mMotionEvents = 0;
          if (event.GetPointCount() == mMinimumTouchesRequired)
          {
            // We have satisfied the minimum touches required for a pan, tell core that a gesture may be possible and change our state accordingly.
            mState = Possible;
            SendPan(AccessibilityGestureEvent::Possible, event);
          }

          mTouchEvents.push_back(event);
        }
        break;
      }

      case Possible:
      {
        unsigned int pointCount(event.GetPointCount());
        if ( (pointCount >= mMinimumTouchesRequired)&&(pointCount <= mMaximumTouchesRequired) )
        {
          if (primaryPointState == PointState::MOTION)
          {
            mTouchEvents.push_back(event);
            mMotionEvents++;

            Vector2 delta(event.points[0].GetScreenPosition() - mPrimaryTouchDownLocation);

            if ( ( mMotionEvents >= mMinimumMotionEvents ) &&
                 ( delta.LengthSquared() >= static_cast<float>( mMinimumDistanceSquared ) ) )
            {
              // If the touch point(s) have moved enough distance to be considered a pan, then tell Core that the pan gesture has started and change our state accordingly.
              mState = Started;
              SendPan(AccessibilityGestureEvent::Started, event);
            }
          }
          else if (primaryPointState == PointState::UP)
          {
            Vector2 delta(event.points[0].GetScreenPosition() - mPrimaryTouchDownLocation);
            if (delta.LengthSquared() >= static_cast<float>( mMinimumDistanceSquared ) )
            {
              SendPan(AccessibilityGestureEvent::Started, event);
              mTouchEvents.push_back(event);
              SendPan(AccessibilityGestureEvent::Finished, event);
            }
            else
            {
              // If we have lifted the primary touch point then tell core the pan is cancelled and change our state to Clear.
              SendPan(AccessibilityGestureEvent::Cancelled, event);
            }
            mState = Clear;
            mTouchEvents.clear();
          }
        }
        else
        {
          // We do not satisfy pan conditions, tell Core our Gesture has been cancelled.
          SendPan(AccessibilityGestureEvent::Cancelled, event);

          if (pointCount == 1 && primaryPointState == PointState::UP)
          {
            // If we have lifted the primary touch point, then change our state to Clear...
            mState = Clear;
            mTouchEvents.clear();
          }
          else
          {
            // ...otherwise change it to Failed.
            mState = Failed;
          }
        }
        break;
      }

      case Started:
      {
        mTouchEvents.push_back(event);

        unsigned int pointCount(event.GetPointCount());
        if ( (pointCount >= mMinimumTouchesRequired)&&(pointCount <= mMaximumTouchesRequired) )
        {
          switch (primaryPointState)
          {
            case PointState::MOTION:
              // Pan is continuing, tell Core.
              SendPan(AccessibilityGestureEvent::Continuing, event);
              break;

            case PointState::UP:
              // Pan is finally finished when our primary point is lifted, tell Core and change our state to Clear.
              SendPan(AccessibilityGestureEvent::Finished, event);
              mState = Clear;
              mTouchEvents.clear();
              break;

            case PointState::STATIONARY:
              if (pointCount == mMinimumTouchesRequired)
              {
                Integration::PointContainerConstIterator iter = event.points.begin() + 1; // We already know the state of the first point
                for(; iter != event.points.end(); ++iter)
                {
                  if(iter->GetState() == PointState::UP)
                  {
                    // The number of touch points will be less than the minimum required.  Inform core and change our state to Finished.
                    SendPan(AccessibilityGestureEvent::Finished, event);
                    mState = Finished;
                    break;
                  }
                }
              }
              break;

            default:
              break;
          }
        }
        else
        {
          // We have gone outside of the pan requirements, inform Core that the gesture is finished.
          SendPan(AccessibilityGestureEvent::Finished, event);

          if (pointCount == 1 && primaryPointState == PointState::UP)
          {
            // If this was the primary point being released, then we change our state back to Clear...
            mState = Clear;
            mTouchEvents.clear();
          }
          else
          {
            // ...otherwise we change it to Finished.
            mState = Finished;
          }
        }
        break;
      }

      case Finished:
      case Failed:
      {
        if (primaryPointState == PointState::UP)
        {
          // Change our state back to clear when the primary touch point is released.
          mState = Clear;
          mTouchEvents.clear();
        }
        break;
      }
    }
  }
}

void AccessibilityGestureDetector::SendPan(AccessibilityGestureEvent::State state, const Integration::TouchEvent& currentEvent)
{
  AccessibilityGestureEvent gesture(state);
  gesture.currentPosition = currentEvent.points[0].GetScreenPosition();
  gesture.numberOfTouches = currentEvent.GetPointCount();

  if ( mTouchEvents.size() > 1 )
  {
    // Get the second last event in the queue, the last one is the current event
    const Integration::TouchEvent& previousEvent( *( mTouchEvents.rbegin() + 1 ) );

    Vector2 previousPosition( mPreviousPosition );
    uint32_t previousTime( previousEvent.time );

    // If we've just started then we want to remove the threshold from Core calculations.
    if ( state == AccessibilityGestureEvent::Started )
    {
      previousPosition = mPrimaryTouchDownLocation;
      previousTime = mPrimaryTouchDownTime;

      // If it's a slow pan, we do not want to phase in the threshold over the first few pan-events
      // A slow pan is defined as one that starts the specified number of milliseconds after the down-event
      if ( ( currentEvent.time - previousTime ) > MINIMUM_TIME_BEFORE_THRESHOLD_ADJUSTMENTS )
      {
        mThresholdAdjustmentsRemaining = mThresholdTotalAdjustments;
        mThresholdAdjustmentPerFrame = ( gesture.currentPosition - previousPosition ) / static_cast<float>( mThresholdTotalAdjustments );
      }
      else
      {
        mThresholdAdjustmentsRemaining = 0;
        mThresholdAdjustmentPerFrame = Vector2::ZERO;
      }
    }

    gesture.previousPosition = previousPosition;
    gesture.timeDelta = currentEvent.time - previousTime;

    // Apply the threshold with a phased approach
    if ( mThresholdAdjustmentsRemaining > 0 )
    {
      --mThresholdAdjustmentsRemaining;
      gesture.currentPosition -= mThresholdAdjustmentPerFrame * static_cast<float>( mThresholdAdjustmentsRemaining );
    }

    mPreviousPosition = gesture.currentPosition;
  }
  else
  {
    gesture.previousPosition = gesture.currentPosition;
    gesture.timeDelta = 0;
  }

  gesture.time = currentEvent.time;

  EmitPan(gesture);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
