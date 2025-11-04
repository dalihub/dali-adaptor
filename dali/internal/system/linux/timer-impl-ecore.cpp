/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/trace.h>

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
DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_TIMER_PERFORMANCE_MARKER, false);

Eina_Bool TimerSourceFunc(void* data)
{
  TimerEcore* timer = static_cast<TimerEcore*>(data);

  if(DALI_LIKELY(Dali::Adaptor::IsAvailable()))
  {
    bool keepRunning = timer->Tick();

    return keepRunning ? EINA_TRUE : EINA_FALSE;
  }
  else
  {
    DALI_LOG_DEBUG_INFO("Tick signal comes after adaptor invalidated. Ignore tick callback.\n");
#ifdef TRACE_ENABLED
    if(gTraceFilter && gTraceFilter->IsTraceEnabled())
    {
      DALI_LOG_DEBUG_INFO("Invalidated timer : %p\n", timer);
    }
#endif
    return EINA_FALSE;
  }
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
  try
  {
    ResetTimerData();
  }
  catch(std::bad_weak_ptr const& ex)
  {
    DALI_LOG_ERROR("TimerEcore::~TimerEcore() - std::bad_weak_ptr caught: %s\n", ex.what());
  }
  delete mImpl;
}

void TimerEcore::Start()
{
  // Timer should be used in the event thread
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  if(mImpl->mId != NULL)
  {
    Stop();
  }
  if(DALI_LIKELY(Dali::Adaptor::IsAvailable()))
  {
    double interval = static_cast<double>(mImpl->mInterval) / 1000.0f;
    mImpl->mId      = ecore_timer_add(interval, reinterpret_cast<Ecore_Task_Cb>(TimerSourceFunc), this);

#ifdef TRACE_ENABLED
    if(gTraceFilter && gTraceFilter->IsTraceEnabled())
    {
      DALI_LOG_DEBUG_INFO("Start ecore timer : %p with interval : %u ms\n", mImpl->mId, mImpl->mInterval);
    }
#endif
  }
}

void TimerEcore::Stop()
{
  // Timer should be used in the event thread
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  ResetTimerData();
}

void TimerEcore::Pause()
{
  // Timer should be used in the event thread
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  if(mImpl->mId != NULL)
  {
#ifdef TRACE_ENABLED
    if(gTraceFilter && gTraceFilter->IsTraceEnabled())
    {
      DALI_LOG_DEBUG_INFO("Freeze ecore timer : %p with interval : %u ms\n", mImpl->mId, mImpl->mInterval);
    }
#endif
    if(DALI_LIKELY(Dali::Adaptor::IsAvailable()))
    {
      ecore_timer_freeze(mImpl->mId);
    }
  }
}

void TimerEcore::Resume()
{
  // Timer should be used in the event thread
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  if(mImpl->mId != NULL)
  {
#ifdef TRACE_ENABLED
    if(gTraceFilter && gTraceFilter->IsTraceEnabled())
    {
      DALI_LOG_DEBUG_INFO("Thaw ecore timer : %p with interval : %u ms\n", mImpl->mId, mImpl->mInterval);
    }
#endif
    if(DALI_LIKELY(Dali::Adaptor::IsAvailable()))
    {
      ecore_timer_thaw(mImpl->mId);
    }
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

  if(DALI_LIKELY(Dali::Adaptor::IsAvailable()))
  {
    // Override with new signal if used
    if(!mTickSignal.Empty())
    {
      DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_TIMER_TICK", [&](std::ostringstream& oss)
      {
        oss << "[ecoreId:" << mImpl->mId << ", interval:" << mImpl->mInterval << "]";
      });
      retVal = mTickSignal.Emit();
      DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_TIMER_TICK", [&](std::ostringstream& oss)
      {
        oss << "[return:" << retVal << "]";
      });

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
  }
  else
  {
    DALI_LOG_DEBUG_INFO("Tick signal comes after adaptor invalidated. Ignore tick callback.\n");
#ifdef TRACE_ENABLED
    if(gTraceFilter && gTraceFilter->IsTraceEnabled())
    {
      DALI_LOG_DEBUG_INFO("Invalidated ecore timer : %p with interval : %u ms\n", mImpl->mId, mImpl->mInterval);
    }
#endif
  }

  return retVal;
}

void TimerEcore::ResetTimerData()
{
  if(mImpl->mId != NULL)
  {
    if(DALI_LIKELY(Dali::Adaptor::IsAvailable()))
    {
#ifdef TRACE_ENABLED
      if(gTraceFilter && gTraceFilter->IsTraceEnabled())
      {
        DALI_LOG_DEBUG_INFO("Stop ecore timer : %p with interval : %u ms\n", mImpl->mId, mImpl->mInterval);
      }
#endif
      ecore_timer_del(mImpl->mId);
    }
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
