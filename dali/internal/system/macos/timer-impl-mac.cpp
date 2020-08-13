/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include "extern-definitions.h"

namespace Dali::Internal::Adaptor
{


/**
 * Struct to hide away macOS implementation details
 */
struct Timer::Impl
{
  Impl(Timer *parent, unsigned int milliSec)
    : mTimer(CreateTimer(parent, milliSec)) {}

  ~Impl() { Stop(); }

  static void TimerProc(CFRunLoopTimerRef timer, void *info);

  void Start();
  void Stop();
  void Reset(Timer *parent, unsigned int milliSec);

  unsigned int GetInterval() const noexcept
  {
    return CFRunLoopTimerGetInterval(mTimer.get()) * 1000.0;
  }

  bool IsRunning() const noexcept
  {
    return CFRunLoopTimerIsValid(mTimer.get());
  }

private:
  CFRef<CFRunLoopTimerRef> CreateTimer(Timer *parent, unsigned int milliSec);

  CFRef<CFRunLoopTimerRef> mTimer;
};

void Timer::Impl::TimerProc(CFRunLoopTimerRef timer, void *info)
{
  auto *pTimer = static_cast<Timer*>(info);
  pTimer->Tick();
}

void Timer::Impl::Start()
{
  if (!IsRunning())
  {
    auto runLoop = CFRunLoopGetMain();
    CFRunLoopAddTimer(runLoop, mTimer.get(), kCFRunLoopDefaultMode);
  }
}

void Timer::Impl::Stop()
{
  if (IsRunning())
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
      &context
    ));
  }
}

void Timer::Impl::Reset(Timer *parent, unsigned int milliSec)
{
  Stop();
  mTimer = CreateTimer(parent, milliSec);
  Start();
}

CFRef<CFRunLoopTimerRef>
Timer::Impl::CreateTimer(Timer *parent, unsigned int milliSec)
{
  const auto interval = static_cast<CFAbsoluteTime>(milliSec) / 1000;
  const auto fireDate = CFAbsoluteTimeGetCurrent() + interval;
  CFRunLoopTimerContext context =
  {
    .version = 0,
    .info = parent,
    .retain = nullptr,
    .release = nullptr,
  };

  return MakeRef(CFRunLoopTimerCreate(
    kCFAllocatorDefault,
    fireDate, interval,
    0,
    0,
    TimerProc,
    &context
  ));
}

TimerPtr Timer::New( unsigned int milliSec )
{
  return new Timer( milliSec );
}

Timer::Timer( unsigned int milliSec )
: mImpl(new Impl(this, milliSec))
{
}

Timer::~Timer()
{
  // stop timers
  Stop();

  delete mImpl;
  mImpl = NULL;
}

void Timer::Start()
{
  mImpl->Start();
}

void Timer::Stop()
{
  mImpl->Stop();
}

void Timer::Pause()
{

}

void Timer::Resume()
{

}

void Timer::SetInterval( unsigned int interval, bool restart )
{
  mImpl->Reset(this, interval);
}

unsigned int Timer::GetInterval() const
{
  return mImpl->GetInterval();
}

bool Timer::Tick()
{
  // Guard against destruction during signal emission
  Dali::Timer handle( this );

  bool retVal( false );

  // Override with new signal if used
  if( !mTickSignal.Empty() )
  {
    retVal = mTickSignal.Emit();

    // Timer stops if return value is false
    if (retVal == false)
    {
      Stop();
    }
    else
    {
      retVal = true;   // continue emission
    }
  }
  else // no callbacks registered
  {
    // periodic timer is started but nobody listens, continue
    retVal = true;
  }

  return retVal;
}

Dali::Timer::TimerSignalType& Timer::TickSignal()
{
  return mTickSignal;
}

bool Timer::IsRunning() const
{
  return mImpl->IsRunning();
}

} // namespace Dali::Internal::Adaptor

