/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/android/timer-impl-android.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/adaptor-framework/android/android-framework.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/android/android-framework-impl.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/framework.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
// Copied from x server
static uint32_t GetCurrentMilliSeconds()
{
  struct timeval tv;

  struct timespec  tp;
  static clockid_t clockid;

  if(!clockid)
  {
#ifdef CLOCK_MONOTONIC_COARSE
    if(clock_getres(CLOCK_MONOTONIC_COARSE, &tp) == 0 &&
       (tp.tv_nsec / 1000) <= 1000 && clock_gettime(CLOCK_MONOTONIC_COARSE, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC_COARSE;
    }
    else
#endif
      if(clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
    {
      clockid = CLOCK_MONOTONIC;
    }
    else
    {
      clockid = ~0L;
    }
  }
  if(clockid != ~0L && clock_gettime(clockid, &tp) == 0)
  {
    return (tp.tv_sec * 1000) + (tp.tv_nsec / 1000000L);
  }

  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

} // namespace

struct TimerAndroid::Impl
{
  Impl(uint32_t milliSec)
  : mInterval(milliSec),
    mStartTimestamp(0),
    mPauseTimestamp(0),
    mRunning(false),
    mId(0)
  {
  }

  uint32_t mInterval;
  uint32_t mStartTimestamp;
  uint32_t mPauseTimestamp;
  bool     mRunning;
  uint32_t mId;
};

TimerAndroidPtr TimerAndroid::New(uint32_t milliSec)
{
  TimerAndroidPtr timer(new TimerAndroid(milliSec));
  return timer;
}

TimerAndroid::TimerAndroid(uint32_t milliSec)
: mImpl(new Impl(milliSec))
{
}

TimerAndroid::~TimerAndroid()
{
  Stop();
  delete mImpl;
}

bool TimerCallback(void* data)
{
  TimerAndroid* timer = static_cast<TimerAndroid*>(data);
  if(timer->IsRunning())
  {
    return timer->Tick();
  }

  return false;
}

void TimerAndroid::Start()
{
  // Timer should be used in the event thread
  DALI_ASSERT_DEBUG(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  if(mImpl->mRunning)
  {
    Stop();
  }

  mImpl->mId             = AndroidFramework::GetFramework(Dali::Integration::AndroidFramework::Get()).AddIdle(mImpl->mInterval, this, TimerCallback);
  mImpl->mRunning        = true;
  mImpl->mStartTimestamp = GetCurrentMilliSeconds();
}

void TimerAndroid::Stop()
{
  // Timer should be used in the event thread
  DALI_ASSERT_DEBUG(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  if(mImpl->mId != 0)
  {
    AndroidFramework::GetFramework(Dali::Integration::AndroidFramework::Get()).RemoveIdle(mImpl->mId);
    mImpl->mStartTimestamp = 0;
    mImpl->mPauseTimestamp = 0;
  }

  ResetTimerData();
}

void TimerAndroid::Pause()
{
  // Timer should be used in the event thread
  DALI_ASSERT_DEBUG(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  if(mImpl->mRunning)
  {
    mImpl->mPauseTimestamp = GetCurrentMilliSeconds();
    AndroidFramework::GetFramework(Dali::Integration::AndroidFramework::Get()).RemoveIdle(mImpl->mId);
    mImpl->mId = 0;
  }
}

void TimerAndroid::Resume()
{
  // Timer should be used in the event thread
  DALI_ASSERT_DEBUG(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  if(mImpl->mRunning && mImpl->mId == 0)
  {
    uint32_t newInterval = 0;
    uint32_t runningTime = mImpl->mPauseTimestamp - mImpl->mStartTimestamp;
    if(mImpl->mInterval > runningTime)
    {
      newInterval = mImpl->mInterval - runningTime;
    }

    mImpl->mStartTimestamp = GetCurrentMilliSeconds() - runningTime;
    mImpl->mPauseTimestamp = 0;
    mImpl->mId             = AndroidFramework::GetFramework(Dali::Integration::AndroidFramework::Get()).AddIdle(newInterval, this, TimerCallback);
  }
}

void TimerAndroid::SetInterval(uint32_t interval, bool restart)
{
  // stop existing timer
  Stop();
  mImpl->mInterval = interval;

  if(restart)
  {
    // start new tick
    Start();
  }
}

uint32_t TimerAndroid::GetInterval() const
{
  return mImpl->mInterval;
}

bool TimerAndroid::Tick()
{
  // Guard against destruction during signal emission
  Dali::Timer handle(this);

  bool retVal(false);

  // Override with new signal if used
  if(!mTickSignal.Empty())
  {
    retVal = mTickSignal.Emit();

    // Timer stops if return value is false
    if(retVal == false)
    {
      Stop();
    }
    else
    {
      retVal = true; // continue emission
    }
  }
  else // no callbacks registered
  {
    // periodic timer is started but nobody listens, continue
    retVal = true;
  }

  return retVal;
}

void TimerAndroid::ResetTimerData()
{
  mImpl->mRunning = false;
  mImpl->mId      = 0;
}

bool TimerAndroid::IsRunning() const
{
  return mImpl->mRunning;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
