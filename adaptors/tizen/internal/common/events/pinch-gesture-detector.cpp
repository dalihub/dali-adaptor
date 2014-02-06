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

// CLASS HEADER
#include "pinch-gesture-detector.h"

// EXTERNAL INCLUDES
#include <cmath>

#include <dali/public-api/events/touch-point.h>
#include <dali/public-api/math/vector2.h>

#include <dali/integration-api/events/pinch-gesture-event.h>
#include <dali/integration-api/events/touch-event-integ.h>

// INTERNAL INCLUDES
#include <base/core-event-interface.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
const unsigned int MINIMUM_TOUCH_EVENTS_REQUIRED = 4;
const unsigned int MINIMUM_TOUCH_EVENTS_REQUIRED_AFTER_START = 4;
const float MINIMUM_DISTANCE_DELTA_DIVISOR = 85.0f;

const float MAXIMUM_GRADIENT_CHANGE_ALLOWED = 2.0f;
const float MAXIMUM_X_DIFF_CALCULATION_FOR_UNDEFINED_GRADIENT = 100.0f;

inline float GetDistance(const TouchPoint& point1, const TouchPoint& point2)
{
  Vector2 vector(point1.screen - point2.screen);
  return vector.Length();
}

inline float GetGradient(const TouchPoint& point1, const TouchPoint& point2)
{
  return (point2.screen.y - point1.screen.y)
         /
         (point2.screen.x - point1.screen.x);
}

inline Vector2 GetCenterPoint(const TouchPoint& point1, const TouchPoint& point2)
{
  return Vector2(point1.screen + point2.screen) * 0.5f;
}

} // unnamed namespace

PinchGestureDetector::PinchGestureDetector(CoreEventInterface& coreEventInterface, Vector2 screenSize)
: GestureDetector(screenSize, Gesture::Pinch),
  mCoreEventInterface(coreEventInterface),
  mState(Clear),
  mTouchEvents(),
  mMinimumDistanceDelta(screenSize.height / MINIMUM_DISTANCE_DELTA_DIVISOR),
  mStartingDistance(0.0f)
{
}

PinchGestureDetector::~PinchGestureDetector()
{
}

void PinchGestureDetector::SendEvent(const Integration::TouchEvent& event)
{
  int pointCount = event.GetPointCount();

  switch (mState)
  {
    case Clear:
    {
      if (pointCount == 2)
      {
        // Change state to possible as we have two touch points.
        mState = Possible;
        mTouchEvents.push_back(event);
      }
      break;
    }

    case Possible:
    {
      if (pointCount != 2)
      {
        // We no longer have two touch points so change state back to Clear.
        mState = Clear;
        mTouchEvents.clear();
      }
      else
      {
        const TouchPoint& currentPoint1 = event.points[0];
        const TouchPoint& currentPoint2 = event.points[1];

        if (currentPoint1.state == TouchPoint::Up || currentPoint2.state == TouchPoint::Up)
        {
          // One of our touch points has an Up event so change our state back to Clear.
          mState = Clear;
          mTouchEvents.clear();
        }
        else
        {
          mTouchEvents.push_back(event);

          // We can only determine a pinch after a certain number of touch points have been collected.
          if (mTouchEvents.size() >= MINIMUM_TOUCH_EVENTS_REQUIRED)
          {
            const TouchPoint& firstPoint1 = mTouchEvents[0].points[0];
            const TouchPoint& firstPoint2 = mTouchEvents[0].points[1];

            float firstDistance = GetDistance(firstPoint1, firstPoint2);
            float currentDistance = GetDistance(currentPoint1, currentPoint2);
            float distanceChanged = firstDistance - currentDistance;

            // Check if distance has changed enough
            if (fabsf(distanceChanged) > mMinimumDistanceDelta)
            {
              // Ensure the gradient between the two points is similar
              float firstGradient = GetGradient(firstPoint1, firstPoint2);
              float currentGradient = GetGradient(currentPoint1, currentPoint2);

              float gradientDelta = firstGradient - currentGradient;

              // Need to deal with undefined gradient so ensure X difference between points is sufficiently large
              float point1XDiff = firstPoint1.screen.x - currentPoint1.screen.x;
              float point2XDiff = firstPoint2.screen.x - currentPoint2.screen.x;

              if (fabsf(gradientDelta) <= MAXIMUM_GRADIENT_CHANGE_ALLOWED ||
                  (fabsf(point1XDiff) <= MAXIMUM_X_DIFF_CALCULATION_FOR_UNDEFINED_GRADIENT &&
                   fabsf(point2XDiff) <= MAXIMUM_X_DIFF_CALCULATION_FOR_UNDEFINED_GRADIENT))
              {
                if ((point1XDiff >= 0.0f && point2XDiff <= 0.0f) ||
                    (point1XDiff <= 0.0f && point2XDiff >= 0.0f))
                {
                  float point1YDiff = firstPoint1.screen.y - currentPoint1.screen.y;
                  float point2YDiff = firstPoint2.screen.y - currentPoint2.screen.y;

                  if ((point1YDiff >= 0.0f && point2YDiff <= 0.0f) ||
                      (point1YDiff <= 0.0f && point2YDiff >= 0.0f))
                  {
                    // Remove the first few events from the vector otherwise values are exaggerated
                    mTouchEvents.erase(mTouchEvents.begin(), mTouchEvents.end() - MINIMUM_TOUCH_EVENTS_REQUIRED_AFTER_START);

                    if ( !mTouchEvents.empty() )
                    {
                      mStartingDistance = GetDistance(mTouchEvents.begin()->points[0], mTouchEvents.begin()->points[1]);

                      // Send pinch started
                      SendPinch(Gesture::Started, event);

                      mState = Started;
                    }

                    mTouchEvents.clear();
                  }
                }
              }
            }

            if (mState == Possible)
            {
              // No pinch, so restart detection
              mState = Clear;
              mTouchEvents.clear();
            }
          }
        }
      }
      break;
    }

    case Started:
    {
      if (pointCount != 2)
      {
        // Send pinch finished event
        SendPinch(Gesture::Finished, event);

        mState = Clear;
        mTouchEvents.clear();
      }
      else
      {
        const TouchPoint& currentPoint1 = event.points[0];
        const TouchPoint& currentPoint2 = event.points[1];

        if (currentPoint1.state == TouchPoint::Up || currentPoint2.state == TouchPoint::Up)
        {
          mTouchEvents.push_back(event);
          // Send pinch finished event
          SendPinch(Gesture::Finished, event);

          mState = Clear;
          mTouchEvents.clear();
        }
        else
        {
          mTouchEvents.push_back(event);

          if (mTouchEvents.size() >= MINIMUM_TOUCH_EVENTS_REQUIRED_AFTER_START)
          {
            // Send pinch continuing
            SendPinch(Gesture::Continuing, event);

            mTouchEvents.clear();
          }
        }
      }
      break;
    }
  }
}

void PinchGestureDetector::Update(const Integration::GestureRequest& request)
{
  // Nothing to do.
}

void PinchGestureDetector::SendPinch(Gesture::State state, const Integration::TouchEvent& currentEvent)
{
  Integration::PinchGestureEvent gesture(state);

  if ( !mTouchEvents.empty() )
  {
    const Integration::TouchEvent& firstEvent = mTouchEvents[0];

    // Assert if we have been holding TouchEvents that do not have 2 points
    DALI_ASSERT_DEBUG( firstEvent.GetPointCount() == 2 );

    // We should use the current event in our calculations unless it does not have two points.
    // If it does not have two points, then we should use the last point in mTouchEvents.
    Integration::TouchEvent event( currentEvent );
    if ( event.GetPointCount() != 2 )
    {
      event = *mTouchEvents.rbegin();
    }

    const TouchPoint& firstPoint1( firstEvent.points[0] );
    const TouchPoint& firstPoint2( firstEvent.points[1] );
    const TouchPoint& currentPoint1( event.points[0] );
    const TouchPoint& currentPoint2( event.points[1] );

    float firstDistance = GetDistance(firstPoint1, firstPoint2);
    float currentDistance = GetDistance(currentPoint1, currentPoint2);
    gesture.scale = currentDistance / mStartingDistance;

    float distanceDelta = fabsf(firstDistance - currentDistance);
    unsigned long timeDelta = currentEvent.time - firstEvent.time;
    gesture.speed = (distanceDelta / timeDelta) * 1000.0f;

    gesture.centerPoint = GetCenterPoint(currentPoint1, currentPoint2);
  }
  else
  {
    // Something has gone wrong, just cancel the gesture.
    gesture.state = Gesture::Cancelled;
  }

  gesture.time = currentEvent.time;

  mCoreEventInterface.QueueCoreEvent(gesture);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
