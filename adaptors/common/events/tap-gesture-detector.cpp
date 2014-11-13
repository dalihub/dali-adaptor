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

// CLASS HEADER
#include "tap-gesture-detector.h"

// EXTERNAL INCLUDES
#include <cmath>

#include <dali/public-api/events/touch-point.h>
#include <dali/public-api/math/vector2.h>

#include <dali/integration-api/events/gesture-requests.h>
#include <dali/integration-api/events/tap-gesture-event.h>
#include <dali/integration-api/events/touch-event-integ.h>
#include <base/core-event-interface.h>

// INTERNAL INCLUDES

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
// TODO: Set these according to DPI
const float MAXIMUM_MOTION_ALLOWED = 20.0f;
const unsigned long MAXIMUM_TIME_ALLOWED = 300u;
} // unnamed namespace

TapGestureDetector::TapGestureDetector(CoreEventInterface& coreEventInterface, Vector2 screenSize, const Integration::TapGestureRequest& request)
: GestureDetector(screenSize, Gesture::Tap),
  mCoreEventInterface(coreEventInterface),
  mState(Clear),
  mMinimumTapsRequired(request.minTaps),
  mMaximumTapsRequired(request.maxTaps),
  mTapsRegistered(0),
  mTouchPosition(),
  mTouchTime(0u),
  mTimerSlot( this )
{
  mTimer = Dali::Timer::New(MAXIMUM_TIME_ALLOWED);
  mTimer.TickSignal().Connect( mTimerSlot, &TapGestureDetector::TimerCallback );
}

TapGestureDetector::~TapGestureDetector()
{
}

void TapGestureDetector::SendEvent(const Integration::TouchEvent& event)
{
  if (event.GetPointCount() == 1)
  {
    const TouchPoint& point = event.points[0];
    TouchPoint::State pointState = point.state;

    switch (mState)
    {
      case Clear:
      {
        if (pointState == TouchPoint::Down)
        {
          mTouchPosition.x = point.screen.x;
          mTouchPosition.y = point.screen.y;
          mTouchTime = event.time;
          mTapsRegistered = 0;
          mState = Touched;
          EmitGesture( Gesture::Possible, mTouchTime );
        }
        break;
      }

      case Touched:
      {
        Vector2 distanceDelta(abs(mTouchPosition.x - point.screen.x),
                              abs(mTouchPosition.y - point.screen.y));

        unsigned long timeDelta = abs(event.time - mTouchTime);

        if (distanceDelta.x > MAXIMUM_MOTION_ALLOWED ||
            distanceDelta.y > MAXIMUM_MOTION_ALLOWED ||
            timeDelta > MAXIMUM_TIME_ALLOWED)
        {
          // We may have already registered some taps so try emitting the gesture
          EmitGesture( mTapsRegistered ? Gesture::Started : Gesture::Cancelled, event.time );
          mState = (pointState == TouchPoint::Motion) ? Failed : Clear;
          mTimer.Stop();
        }

        if (mState == Touched && pointState == TouchPoint::Up)
        {
          ++mTapsRegistered;

          if (mTapsRegistered < mMaximumTapsRequired)
          {
            // Only emit gesture after timer expires if asked for multiple taps.
            mState = Registered;
            mTimer.Start();
          }
          else
          {
            EmitGesture(Gesture::Started, event.time);
            mState = Clear;
            mTimer.Stop();
          }
        }
        break;
      }

      case Registered:
      {
        if (pointState == TouchPoint::Down)
        {
          mTimer.Stop();

          Vector2 distanceDelta(abs(mTouchPosition.x - point.screen.x),
                                abs(mTouchPosition.y - point.screen.y));

          // Check if subsequent tap is in a different position, if not then emit the previous tap
          // count gesture (if required),
          if (distanceDelta.x > MAXIMUM_MOTION_ALLOWED ||
              distanceDelta.y > MAXIMUM_MOTION_ALLOWED)
          {
            EmitGesture(Gesture::Started, event.time);
            mTouchPosition.x = point.screen.x;
            mTouchPosition.y = point.screen.y;
          }

          mTouchTime = event.time;
          mState = Touched;
          mTimer.Start();
        }
        break;
      }

      case Failed:
      {
        if (pointState == TouchPoint::Up)
        {
          mState = Clear;
        }
        break;
      }

      default:
        mState = Clear;
        break;
    }
  }
  else
  {
    mState = Failed;

    // We have entered a multi-touch event so emit registered gestures if required.
    EmitGesture(Gesture::Started, event.time);
  }
}

void TapGestureDetector::Update(const Integration::GestureRequest& request)
{
  const Integration::TapGestureRequest& tap = static_cast<const Integration::TapGestureRequest&>(request);

  mMinimumTapsRequired = tap.minTaps;
  mMaximumTapsRequired = tap.maxTaps;
}

bool TapGestureDetector::TimerCallback()
{
  EmitGesture( ( mTapsRegistered >= mMinimumTapsRequired ? Gesture::Started : Gesture::Cancelled ), mTouchTime + MAXIMUM_TIME_ALLOWED);
  mState = Clear;

  // There is no touch event at this time, so ProcessEvents must be called directly
  mCoreEventInterface.ProcessCoreEvents();

  return false;
}

void TapGestureDetector::EmitGesture( Gesture::State state, unsigned int time )
{
  if ( (state == Gesture::Possible) ||
       (state == Gesture::Cancelled) ||
       (mTapsRegistered >= mMinimumTapsRequired && mTapsRegistered <= mMaximumTapsRequired) )
  {
    Integration::TapGestureEvent event( state );
    event.numberOfTaps = mTapsRegistered;
    event.point = mTouchPosition;
    event.time = time;

    mCoreEventInterface.QueueCoreEvent(event);
  }
  mTapsRegistered = 0;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
