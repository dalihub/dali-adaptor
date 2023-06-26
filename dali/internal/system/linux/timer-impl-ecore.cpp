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
#include <dali/internal/system/linux/timer-impl-ecore.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/public-api/dali-adaptor-common.h>

#include <dali/internal/system/linux/dali-ecore.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
// LOCAL STUFF
namespace
{
Eina_Bool TimerSourceFunc(void* data)
{
  TimerEcore* timer = static_cast<TimerEcore*>(data);

  bool keepRunning = timer->Tick();

  return keepRunning ? EINA_TRUE : EINA_FALSE;
}
} // unnamed namespace

/**
 * Struct to hide away Ecore implementation details
 */
struct TimerEcore::Impl
{
  Impl(uint32_t milliSec)
  : mId(NULL),
    mInterval(milliSec)
  {
  }

  Ecore_Timer* mId;
  uint32_t     mInterval;
};

TimerEcorePtr TimerEcore::New(uint32_t milliSec)
{
  TimerEcorePtr timer(new TimerEcore(milliSec));
  return timer;
}

TimerEcore::TimerEcore(uint32_t milliSec)
: mImpl(new Impl(milliSec))
{
}

TimerEcore::~TimerEcore()
{
  ResetTimerData();
  delete mImpl;
}

void TimerEcore::Start()
{
  // Timer should be used in the event thread
  DALI_ASSERT_ALWAYS(Adaptor::IsAvailable());

  if(mImpl->mId != NULL)
  {
    Stop();
  }
  double interval = static_cast<double>(mImpl->mInterval) / 1000.0f;
  mImpl->mId      = ecore_timer_add(interval, reinterpret_cast<Ecore_Task_Cb>(TimerSourceFunc), this);
}

void TimerEcore::Stop()
{
  // Timer should be used in the event thread
  DALI_ASSERT_ALWAYS(Adaptor::IsAvailable());

  ResetTimerData();
}

void TimerEcore::Pause()
{
  // Timer should be used in the event thread
  DALI_ASSERT_ALWAYS(Adaptor::IsAvailable());

  if(mImpl->mId != NULL)
  {
    ecore_timer_freeze(mImpl->mId);
  }
}

void TimerEcore::Resume()
{
  // Timer should be used in the event thread
  DALI_ASSERT_ALWAYS(Adaptor::IsAvailable());

  if(mImpl->mId != NULL)
  {
    ecore_timer_thaw(mImpl->mId);
  }
}

void TimerEcore::SetInterval(uint32_t interval, bool restart)
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

uint32_t TimerEcore::GetInterval() const
{
  return mImpl->mInterval;
}

bool TimerEcore::Tick()
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

void TimerEcore::ResetTimerData()
{
  if(mImpl->mId != NULL)
  {
    ecore_timer_del(mImpl->mId);
    mImpl->mId = NULL;
  }
}

bool TimerEcore::IsRunning() const
{
  return mImpl->mId != NULL;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
