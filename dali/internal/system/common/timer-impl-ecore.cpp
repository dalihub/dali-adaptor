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

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/public-api/dali-adaptor-common.h>

// Ecore is littered with C style cast
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <Ecore.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

// LOCAL STUFF
namespace
{
Eina_Bool TimerSourceFunc (void *data)
{
  Timer* timer = static_cast<Timer*>(data);

  bool keepRunning = timer->Tick();

  return keepRunning ? EINA_TRUE : EINA_FALSE;
}
} // unnamed namespace

/**
 * Struct to hide away Ecore implementation details
 */
struct Timer::Impl
{
  Impl( unsigned int milliSec )
  : mId(NULL),
    mInterval(milliSec)
  {
  }

  Ecore_Timer * mId;
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
  ResetTimerData();
  delete mImpl;
}

void Timer::Start()
{
  // Timer should be used in the event thread
  DALI_ASSERT_DEBUG( Adaptor::IsAvailable() );

  if(mImpl->mId != NULL)
  {
    Stop();
  }
  mImpl->mId = ecore_timer_add( (double)mImpl->mInterval/1000.0f, (Ecore_Task_Cb)TimerSourceFunc, this );
}

void Timer::Stop()
{
  // Timer should be used in the event thread
  DALI_ASSERT_DEBUG( Adaptor::IsAvailable() );

  ResetTimerData();
}

void Timer::Pause()
{
  // Timer should be used in the event thread
  DALI_ASSERT_DEBUG( Adaptor::IsAvailable() );

  if( mImpl->mId != NULL )
  {
    ecore_timer_freeze( mImpl->mId );
  }
}

void Timer::Resume()
{
  // Timer should be used in the event thread
  DALI_ASSERT_DEBUG( Adaptor::IsAvailable() );

  if( mImpl->mId != NULL )
  {
    ecore_timer_thaw( mImpl->mId );
  }
}

void Timer::SetInterval( unsigned int interval, bool restart )
{
  // stop existing timer
  Stop();
  mImpl->mInterval = interval;

  if( restart )
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

void Timer::ResetTimerData()
{
  if (mImpl->mId != NULL)
  {
    ecore_timer_del(mImpl->mId);
    mImpl->mId = NULL;
  }
}

bool Timer::IsRunning() const
{
  return mImpl->mId != NULL;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
