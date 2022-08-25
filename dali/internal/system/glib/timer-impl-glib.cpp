/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/common/timer-impl.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/internal/system/common/time-service.h>

// EXTERNAL INCLUDES
#include <glib.h>
#include <sys/time.h>

namespace Dali::Internal::Adaptor
{
extern GMainContext* GetMainLoopContext();

namespace
{
gboolean TimerSourceFunc(gpointer userData)
{
  Timer* timer = static_cast<Timer*>(userData);

  bool keepRunning = timer->Tick();
  return keepRunning ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
}

} // unnamed namespace

struct Timer::Impl
{
  Impl(unsigned int milliSec)
  : mInterval(milliSec)
  {
  }

  GSource*     mTimerHandle{nullptr};
  unsigned int mInterval{0};
  uint32_t     mStartTimestamp{0};
  uint32_t     mPauseTimestamp{0};
  bool         mRunning{false};
  bool         mRestartAfterExpiry{false}; // Restart at full interval after pause/resume/expiry
};

TimerPtr Timer::New(unsigned int milliSec)
{
  TimerPtr timer(new Timer(milliSec));
  return timer;
}

Timer::Timer(unsigned int milliSec)
: mImpl(new Impl(milliSec))
{
}

Timer::~Timer()
{
  Stop();
  delete mImpl;
}

void Timer::Start()
{
  if(mImpl->mRunning && mImpl->mTimerHandle)
  {
    Stop();
  }

  mImpl->mTimerHandle = g_timeout_source_new(mImpl->mInterval);
  g_source_set_callback(mImpl->mTimerHandle, TimerSourceFunc, this, nullptr); // user data is this object, no need for destroy notify.
  g_source_attach(mImpl->mTimerHandle, GetMainLoopContext());

  mImpl->mRunning        = true;
  mImpl->mStartTimestamp = TimeService::GetMilliSeconds();
}

void Timer::Stop()
{
  if(mImpl->mTimerHandle != nullptr)
  {
    g_source_destroy(mImpl->mTimerHandle);
    g_source_unref(mImpl->mTimerHandle);
    mImpl->mTimerHandle = nullptr;

    mImpl->mStartTimestamp = 0;
    mImpl->mPauseTimestamp = 0;
  }

  ResetTimerData();
}

void Timer::Pause()
{
  if(mImpl->mRunning)
  {
    g_source_destroy(mImpl->mTimerHandle);
    g_source_unref(mImpl->mTimerHandle);
    mImpl->mTimerHandle    = nullptr;
    mImpl->mPauseTimestamp = TimeService::GetMilliSeconds();
  }
}

void Timer::Resume()
{
  if(mImpl->mRunning && mImpl->mTimerHandle == nullptr)
  {
    uint32_t newInterval = 0;
    uint32_t runningTime = mImpl->mPauseTimestamp - mImpl->mStartTimestamp;
    if(mImpl->mInterval > runningTime)
    {
      newInterval = mImpl->mInterval - runningTime;
    }

    mImpl->mStartTimestamp = TimeService::GetMilliSeconds() - runningTime;
    mImpl->mPauseTimestamp = 0;

    mImpl->mTimerHandle = g_timeout_source_new(newInterval);
    g_source_set_callback(mImpl->mTimerHandle, TimerSourceFunc, this, nullptr);
    g_source_attach(mImpl->mTimerHandle, GetMainLoopContext());
    // After next expiry, stop and restart with correct interval
    mImpl->mRestartAfterExpiry = true;
  }
}

void Timer::SetInterval(unsigned int interval, bool restart)
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

unsigned int Timer::GetInterval() const
{
  return mImpl->mInterval;
}

bool Timer::Tick()
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

  if(mImpl->mRestartAfterExpiry && mImpl->mRunning)
  {
    mImpl->mRestartAfterExpiry = false;
    // Timer was resumed.
    retVal = false;
    ResetTimerData();
    Start();
  }

  return retVal;
}

Dali::Timer::TimerSignalType& Timer::TickSignal()
{
  return mTickSignal;
}

void Timer::ResetTimerData()
{
  mImpl->mRunning = false;
  if(mImpl->mTimerHandle)
  {
    g_source_unref(mImpl->mTimerHandle);
  }
  mImpl->mTimerHandle = nullptr;
}

bool Timer::IsRunning() const
{
  return mImpl->mRunning;
}

} // namespace Dali::Internal::Adaptor
