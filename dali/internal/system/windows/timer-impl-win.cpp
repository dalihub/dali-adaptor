/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/windows/timer-impl-win.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/object/type-registry.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/windows/platform-implement-win.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
// LOCAL STUFF
namespace
{
Dali::TypeRegistration TIMER_WIN_TYPE(typeid(Dali::Internal::Adaptor::TimerWin), typeid(Dali::Timer), nullptr);

bool TimerSourceFunc(void* data)
{
  TimerWin* timer = static_cast<TimerWin*>(data);
  return timer->Tick();
}
} // namespace

/**
 * Struct to hide away Windows implementation details
 */
struct TimerWin::Impl
{
  Impl(uint32_t milliSec)
  : mId(-1),
    mInterval(milliSec),
    mStartTimestamp(0u),
    mPauseTimestamp(0u),
    mRunning(false),
    mPaused(false),
    mRestartAfterExpiry(false)
  {
  }

  intptr_t mId;

  uint32_t mInterval;
  uint32_t mStartTimestamp;
  uint32_t mPauseTimestamp;
  bool     mRunning;
  bool     mPaused;
  bool     mRestartAfterExpiry;
};

TimerWinPtr TimerWin::New(uint32_t milliSec)
{
  TimerWinPtr timer(new TimerWin(milliSec));
  return timer;
}

TimerWin::TimerWin(uint32_t milliSec)
: mImpl(new Impl(milliSec))
{
}

TimerWin::~TimerWin()
{
  // stop timers
  Stop();

  delete mImpl;
  mImpl = NULL;
}

void TimerWin::Start()
{
  if(mImpl->mRunning)
  {
    Stop();
  }

  mImpl->mId = WindowsPlatform::SetTimer(mImpl->mInterval, TimerSourceFunc, this);
  if(mImpl->mId >= 0)
  {
    mImpl->mRunning        = true;
    mImpl->mPaused         = false;
    mImpl->mStartTimestamp = WindowsPlatform::GetCurrentMilliSeconds();
  }
}

void TimerWin::Stop()
{
  if(0 <= mImpl->mId)
  {
    WindowsPlatform::KillTimer(mImpl->mId);
  }
  ResetTimerData();
}

void TimerWin::Pause()
{
  if(mImpl->mRunning && !mImpl->mPaused)
  {
    mImpl->mPauseTimestamp = WindowsPlatform::GetCurrentMilliSeconds();
    if(mImpl->mId >= 0)
    {
      WindowsPlatform::KillTimer(mImpl->mId);
      mImpl->mId = -1;
    }
    mImpl->mPaused = true;
  }
}

void TimerWin::Resume()
{
  if(mImpl->mRunning && mImpl->mPaused)
  {
    const uint32_t elapsed   = mImpl->mPauseTimestamp - mImpl->mStartTimestamp;
    const uint32_t remaining = elapsed < mImpl->mInterval ? mImpl->mInterval - elapsed : 1u;

    mImpl->mId = WindowsPlatform::SetTimer(remaining, TimerSourceFunc, this);
    if(mImpl->mId >= 0)
    {
      mImpl->mStartTimestamp     = WindowsPlatform::GetCurrentMilliSeconds() - elapsed;
      mImpl->mPauseTimestamp     = 0u;
      mImpl->mPaused             = false;
      mImpl->mRestartAfterExpiry = remaining < mImpl->mInterval;
    }
    else
    {
      ResetTimerData();
    }
  }
}

void TimerWin::SetInterval(uint32_t interval, bool restart)
{
  Stop();
  mImpl->mInterval = interval;
  if(restart)
  {
    Start();
  }
}

uint32_t TimerWin::GetInterval() const
{
  return mImpl->mInterval;
}

bool TimerWin::Tick()
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
    if(mImpl->mPaused)
    {
      // The resumed remainder has expired and the callback paused this timer.
      // The next Resume() starts a fresh full interval instead of scheduling
      // another near-immediate remainder.
      mImpl->mRestartAfterExpiry = false;
      mImpl->mStartTimestamp     = mImpl->mPauseTimestamp;
      return retVal;
    }

    // Resume first waits only for the remainder of the interrupted period.
    // Subsequent ticks must use the full configured interval again.
    mImpl->mRestartAfterExpiry = false;
    if(mImpl->mId >= 0)
    {
      WindowsPlatform::KillTimer(mImpl->mId);
      mImpl->mId = -1;
    }

    if(retVal)
    {
      mImpl->mId = WindowsPlatform::SetTimer(mImpl->mInterval, TimerSourceFunc, this);
      if(mImpl->mId < 0)
      {
        ResetTimerData();
        retVal = false;
      }
    }
  }

  if(retVal && mImpl->mRunning && !mImpl->mPaused)
  {
    mImpl->mStartTimestamp = WindowsPlatform::GetCurrentMilliSeconds();
  }
  return retVal;
}

bool TimerWin::IsRunning() const
{
  return mImpl->mRunning;
}

void TimerWin::ResetTimerData()
{
  mImpl->mId                 = -1;
  mImpl->mStartTimestamp     = 0u;
  mImpl->mPauseTimestamp     = 0u;
  mImpl->mRunning            = false;
  mImpl->mPaused             = false;
  mImpl->mRestartAfterExpiry = false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
