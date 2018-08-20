/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

// Ecore is littered with C style cast
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

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
void TimerSourceFunc (void *data)
{
  Timer* timer = static_cast<Timer*>(data);

  bool keepRunning = timer->Tick();
}
}

/**
 * Struct to hide away Ecore implementation details
 */
struct Timer::Impl
{
  Impl( unsigned int milliSec ) :
    mId(-1),
    mInterval(milliSec)
  {
  }

  int mId;

  unsigned int mInterval;
};

TimerPtr Timer::New( unsigned int milliSec )
{
  TimerPtr timer( new Timer( milliSec ) );
  return timer;
}

Timer::Timer( unsigned int milliSec )
: mImpl(new Impl(milliSec))
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
  if( 0 > mImpl->mId )
  {
    mImpl->mId = WindowsPlatformImplement::SetTimer( mImpl->mInterval, TimerSourceFunc, this );
  }
}

void Timer::Stop()
{
  if( 0 <= mImpl->mId )
  {
    WindowsPlatformImplement::KillTimer( mImpl->mId );
    mImpl->mId = -1;
  }
}

void Timer::SetInterval( unsigned int interval )
{
  // stop existing timer
  Stop();
  mImpl->mInterval = interval;
  // start new tick
  Start();
}

unsigned int Timer::GetInterval() const
{
  return mImpl->mInterval;
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
  return 0 <= mImpl->mId;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
