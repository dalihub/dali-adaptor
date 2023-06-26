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
    mInterval(milliSec)
  {
  }

  intptr_t mId;

  uint32_t mInterval;
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
  if(0 > mImpl->mId)
  {
    mImpl->mId = WindowsPlatform::SetTimer(mImpl->mInterval, TimerSourceFunc, this);
  }
}

void TimerWin::Stop()
{
  if(0 <= mImpl->mId)
  {
    WindowsPlatform::KillTimer(mImpl->mId);
    mImpl->mId = -1;
  }
}

void TimerWin::Pause()
{
}

void TimerWin::Resume()
{
}

void TimerWin::SetInterval(uint32_t interval, bool restart)
{
  if(true == restart)
  {
    // stop existing timer
    Stop();
    mImpl->mInterval = interval;
    // start new tick
    Start();
  }
  else
  {
    mImpl->mInterval = interval;
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

  return retVal;
}

bool TimerWin::IsRunning() const
{
  return 0 <= mImpl->mId;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
