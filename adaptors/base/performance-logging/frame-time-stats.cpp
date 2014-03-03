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

// STRUCT HEADER
#include "frame-time-stats.h"

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
const float EPSILON = 0.9f; // rolling average = (average * epsilon) + (current * epsilon)
const float ONE_OVER_MICROSECONDS_TO_SECONDS = 1.f / 1000000.f; ///< microseconds per second
}

FrameTimeStats::FrameTimeStats()
: mTotal( 0.f)
{
  Reset();
}

FrameTimeStats::~FrameTimeStats()
{
}

void FrameTimeStats::StartTime( const FrameTimeStamp& timeStamp )
{
  // check to make sure we don't get 2 start times in a row
  if( mTimeState != WAITING_FOR_START_TIME )
  {
    Reset();
  }

  mStart = timeStamp;
  mTimeState = WAITING_FOR_END_TIME;
}

void FrameTimeStats::EndTime( const FrameTimeStamp& timeStamp )
{
  if( mTimeState != WAITING_FOR_END_TIME )
  {
    Reset();
    return;
  }

  mTimeState = WAITING_FOR_START_TIME;
  mRunCount++;

  // frame time in seconds
  unsigned int elapsedTime = FrameTimeStamp::MicrosecondDiff( mStart, timeStamp);

  // if the min and max times haven't been set, do that now.
  if( !mMinMaxTimeSet )
  {
    mMin = elapsedTime;
    mMax = elapsedTime;
    mAvg = elapsedTime;
    mMinMaxTimeSet = true;
  }
  else
  {
    if (elapsedTime < mMin)
    {
      mMin= elapsedTime;
    }
    else if (elapsedTime > mMax)
    {
      mMax = elapsedTime;
    }
  }

  mTotal += elapsedTime;

  // calculate a rolling average
  mAvg = (elapsedTime * (1.0f - EPSILON)) + (mAvg * EPSILON);

}

void FrameTimeStats::Reset()
{
  mTimeState = WAITING_FOR_START_TIME;
  mMinMaxTimeSet = false;
  mMin = 0.f;
  mMax = 0.f;
  mAvg = 0.f;
  mRunCount = 0;
}

float FrameTimeStats::GetRollingAverageTime() const
{
  return mAvg * ONE_OVER_MICROSECONDS_TO_SECONDS;
}

float FrameTimeStats::GetMaxTime() const
{
  return mMax * ONE_OVER_MICROSECONDS_TO_SECONDS;
}

float FrameTimeStats::GetMinTime() const
{
  return mMin * ONE_OVER_MICROSECONDS_TO_SECONDS;
}

float FrameTimeStats::GetTotalTime() const
{
  return mTotal * ONE_OVER_MICROSECONDS_TO_SECONDS;
}

unsigned int FrameTimeStats::GetRunCount() const
{
  return mRunCount;
}


} // namespace Adaptor

} // namespace Internal

} // namespace Dali
