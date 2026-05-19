/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/linux/tcore/timer-impl-tcore.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/trace.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <tizen_core.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>

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

bool TimerSourceFunc(void* data)
{
  TimerTcore* timer = static_cast<TimerTcore*>(data);

  if(DALI_LIKELY(Dali::Adaptor::IsAvailable()))
  {
    bool keepRunning = timer->Tick();

    return keepRunning;
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
    return false;
  }
}

tizen_core_h GetTizenCore()
{
  tizen_core_h core = NULL;
  if(tizen_core_find("main", &core) != TIZEN_CORE_ERROR_NONE)
  {
    tizen_core_find_from_this_thread(&core);
  }
  return core;
}
} // unnamed namespace

/**
 * Struct to hide away tizen-core implementation details
 */
struct TimerTcore::Impl
{
  Impl(uint32_t milliSec)
  : mSource(NULL),
    mInterval(milliSec),
    mPaused(false)
  {
  }

  tizen_core_source_h mSource;
  uint32_t           mInterval;
  bool               mPaused;  ///< true when paused (source removed, will re-add on resume)
};

TimerTcorePtr TimerTcore::New(uint32_t milliSec)
{
  TimerTcorePtr timer(new TimerTcore(milliSec));
  return timer;
}

TimerTcore::TimerTcore(uint32_t milliSec)
: mImpl(new Impl(milliSec))
{
}

TimerTcore::~TimerTcore()
{
  try
  {
    ResetTimerData();
  }
  catch(std::bad_weak_ptr const& ex)
  {
    DALI_LOG_ERROR("TimerTcore::~TimerTcore() - std::bad_weak_ptr caught: %s\n", ex.what());
  }
  delete mImpl;
}

void TimerTcore::Start()
{
  // Timer should be used in the event thread
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  if(mImpl->mSource != NULL)
  {
    Stop();
  }
  mImpl->mPaused = false;

  if(DALI_LIKELY(Dali::Adaptor::IsAvailable()))
  {
    tizen_core_h core = GetTizenCore();
    if(core)
    {
      int ret = tizen_core_add_timer(core, mImpl->mInterval, TimerSourceFunc, this, &mImpl->mSource);

      if(ret == TIZEN_CORE_ERROR_NONE)
      {
#ifdef TRACE_ENABLED
        if(gTraceFilter && gTraceFilter->IsTraceEnabled())
        {
          DALI_LOG_DEBUG_INFO("Start tizen core timer : %p with interval : %u ms\n", mImpl->mSource, mImpl->mInterval);
        }
#endif
      }
      else
      {
        DALI_LOG_ERROR("Failed to add tizen core timer: %d\n", ret);
        mImpl->mSource = NULL;
      }
    }
  }
}

void TimerTcore::Stop()
{
  // Timer should be used in the event thread
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  ResetTimerData();
}

void TimerTcore::Pause()
{
  // Timer should be used in the event thread
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  if(mImpl->mSource != NULL && !mImpl->mPaused)
  {
#ifdef TRACE_ENABLED
    if(gTraceFilter && gTraceFilter->IsTraceEnabled())
    {
      DALI_LOG_DEBUG_INFO("Pause tizen core timer : %p with interval : %u ms\n", mImpl->mSource, mImpl->mInterval);
    }
#endif
    if(DALI_LIKELY(Dali::Adaptor::IsAvailable()))
    {
      tizen_core_h core = GetTizenCore();
      if(core)
      {
        tizen_core_remove_source(core, mImpl->mSource);
      }
      mImpl->mSource = NULL;
      mImpl->mPaused = true;
    }
  }
}

void TimerTcore::Resume()
{
  // Timer should be used in the event thread
  DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

  if(mImpl->mPaused)
  {
#ifdef TRACE_ENABLED
    if(gTraceFilter && gTraceFilter->IsTraceEnabled())
    {
      DALI_LOG_DEBUG_INFO("Resume tizen core timer with interval : %u ms\n", mImpl->mInterval);
    }
#endif
    mImpl->mPaused = false;
    Start();
  }
}

void TimerTcore::SetInterval(uint32_t interval, bool restart)
{
  // stop existing timer
  Stop();
  mImpl->mInterval = interval;
  mImpl->mPaused  = false;

  if(restart)
  {
    // start new tick
    Start();
  }
}

uint32_t TimerTcore::GetInterval() const
{
  return mImpl->mInterval;
}

bool TimerTcore::Tick()
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
        oss << "[tcoreSource:" << mImpl->mSource << ", interval:" << mImpl->mInterval << "]";
      });
      retVal = mTickSignal.Emit();
      DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_TIMER_TICK", [&](std::ostringstream& oss)
      {
        oss << "[return:" << retVal << "]";
      });

      // Timer stops if return value is false
      if(retVal == false)
      {
        // Don't call Stop() - tizen_core will remove source when we return false.
        // Just clear mSource so IsRunning() and destructor behave correctly.
        mImpl->mSource = NULL;
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
      DALI_LOG_DEBUG_INFO("Invalidated tizen core timer : %p with interval : %u ms\n", mImpl->mSource, mImpl->mInterval);
    }
#endif
  }

  return retVal;
}

void TimerTcore::ResetTimerData()
{
  if(mImpl->mSource != NULL)
  {
    DALI_ASSERT_ALWAYS(Dali::Stage::IsCoreThread() && "Core is not installed. Might call this API from worker thread?");

    if(DALI_LIKELY(Dali::Adaptor::IsAvailable()))
    {
      tizen_core_h core = GetTizenCore();
      if(core)
      {
#ifdef TRACE_ENABLED
        if(gTraceFilter && gTraceFilter->IsTraceEnabled())
        {
          DALI_LOG_DEBUG_INFO("Stop tizen core timer : %p with interval : %u ms\n", mImpl->mSource, mImpl->mInterval);
        }
#endif
        tizen_core_remove_source(core, mImpl->mSource);
      }
    }
    mImpl->mSource = NULL;
  }
  mImpl->mPaused = false;
}

bool TimerTcore::IsRunning() const
{
  return mImpl->mSource != NULL;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
