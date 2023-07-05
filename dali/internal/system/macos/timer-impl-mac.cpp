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
#include <dali/internal/system/macos/timer-impl-mac.h>

// EXTERNAL INCLUDES
#include "extern-definitions.h"

namespace Dali::Internal::Adaptor
{
/**
 * Struct to hide away macOS implementation details
 */
struct TimerMac::Impl
{
  Impl(TimerMac* parent, uint32_t milliSec)
  : mTimer(CreateTimer(parent, milliSec))
  {
  }

  ~Impl()
  {
    Stop();
  }

  static void TimerProc(CFRunLoopTimerRef timer, void* info);

  void Start();
  void Stop();
  void Reset(TimerMac* parent, uint32_t milliSec);

  uint32_t GetInterval() const noexcept
  {
    return CFRunLoopTimerGetInterval(mTimer.get()) * 1000.0;
  }

  bool IsRunning() const noexcept
  {
    return CFRunLoopTimerIsValid(mTimer.get());
  }

private:
  CFRef<CFRunLoopTimerRef> CreateTimer(TimerMac* parent, uint32_t milliSec);

  CFRef<CFRunLoopTimerRef> mTimer;
};

void TimerMac::Impl::TimerProc(CFRunLoopTimerRef timer, void* info)
{
  auto* pTimer = static_cast<TimerMac*>(info);
  pTimer->Tick();
}

void TimerMac::Impl::Start()
{
  if(!IsRunning())
  {
    auto runLoop = CFRunLoopGetMain();
    CFRunLoopAddTimer(runLoop, mTimer.get(), kCFRunLoopDefaultMode);
  }
}

void TimerMac::Impl::Stop()
{
  if(IsRunning())
  {
    CFRunLoopTimerContext context;
    CFRunLoopTimerGetContext(mTimer.get(), &context);
    const auto interval = CFRunLoopTimerGetInterval(mTimer.get());
    CFRunLoopTimerInvalidate(mTimer.get());

    // After we invalidate the timer, we can't reuse it, so we create
    // a new timer for case the user calls Start again
    const auto fireDate = CFAbsoluteTimeGetCurrent() + interval;
    mTimer.reset(CFRunLoopTimerCreate(
      kCFAllocatorDefault,
      fireDate,
      interval,
      0,
      0,
      TimerProc,
      &context));
  }
}

void TimerMac::Impl::Reset(TimerMac* parent, uint32_t milliSec)
{
  Stop();
  mTimer = CreateTimer(parent, milliSec);
  Start();
}

CFRef<CFRunLoopTimerRef>
TimerMac::Impl::CreateTimer(TimerMac* parent, uint32_t milliSec)
{
  const auto            interval = static_cast<CFAbsoluteTime>(milliSec) / 1000;
  const auto            fireDate = CFAbsoluteTimeGetCurrent() + interval;
  CFRunLoopTimerContext context =
    {
      .version = 0,
      .info    = parent,
      .retain  = nullptr,
      .release = nullptr,
    };

  return MakeRef(CFRunLoopTimerCreate(
    kCFAllocatorDefault,
    fireDate,
    interval,
    0,
    0,
    TimerProc,
    &context));
}

TimerMacPtr TimerMac::New(uint32_t milliSec)
{
  return new TimerMac(milliSec);
}

TimerMac::TimerMac(uint32_t milliSec)
: mImpl(new Impl(this, milliSec))
{
}

TimerMac::~TimerMac()
{
  // stop timers
  Stop();

  delete mImpl;
  mImpl = NULL;
}

void TimerMac::Start()
{
  mImpl->Start();
}

void TimerMac::Stop()
{
  mImpl->Stop();
}

void TimerMac::Pause()
{
}

void TimerMac::Resume()
{
}

void TimerMac::SetInterval(uint32_t interval, bool restart)
{
  mImpl->Reset(this, interval);
}

uint32_t TimerMac::GetInterval() const
{
  return mImpl->GetInterval();
}

bool TimerMac::Tick()
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

bool TimerMac::IsRunning() const
{
  return mImpl->IsRunning();
}

} // namespace Dali::Internal::Adaptor
