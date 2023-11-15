/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/libuv/timer-impl-libuv.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// EXTERNAL INCLUDES
#include <uv.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
extern uv_loop_t* GetUVMainLoop();

namespace
{
void TimerSourceFunc(uv_timer_t* handle)
{
  TimerLibuv* timer = static_cast<TimerLibuv*>(handle->data);

  bool keepRunning = timer->Tick();
  if(!keepRunning)
  {
    timer->Stop();
  }
}
void FreeHandleCallback(uv_handle_t* handle)
{
  delete handle;
}

} // unnamed namespace

/**
 * Struct to hide away libuv implementation details
 */
struct TimerLibuv::Impl
{
  Impl(uint32_t milliSec)
  : mTimerHandle(NULL),
    mInterval(milliSec),
    mRunning(false)
  {
  }

  ~Impl()
  {
    // the handle will still be alive for a short period after calling uv_close
    // set the data to NULL to avoid a dangling pointer
    if(mTimerHandle)
    {
      mTimerHandle->data = NULL;
      uv_close(reinterpret_cast<uv_handle_t*>(mTimerHandle), FreeHandleCallback);
    }
  }

  bool Running()
  {
    return mRunning;
  }

  void Start(void* internalTimerPtr)
  {
    Stop(); // make sure we stop first if its currently running

    if(!mTimerHandle)
    {
      // heap allocate the handle as its lifetime will be longer than TimerImpl
      mTimerHandle = new uv_timer_t;

      // initialize the handle
      uv_timer_init(GetUVMainLoop(), mTimerHandle);
    }

    mRunning = true;

    mTimerHandle->data = internalTimerPtr;

    uv_timer_start(mTimerHandle, TimerSourceFunc, mInterval, mInterval);
  }

  void Stop()
  {
    if(mRunning)
    {
      mTimerHandle->data = NULL;
      uv_timer_stop(mTimerHandle);
      mRunning = false;
    }
  }

  void Pause()
  {
    uv_timer_stop(mTimerHandle);
  }

  void Resume()
  {
    uv_timer_start(mTimerHandle, TimerSourceFunc, mInterval, mInterval);
  }

  uv_timer_t* mTimerHandle;
  uint32_t    mInterval;
  bool        mRunning;
};

TimerLibuvPtr TimerLibuv::New(uint32_t milliSec)
{
  DALI_LOG_ERROR(" new timer\n");
  TimerLibuvPtr timer(new TimerLibuv(milliSec));
  return timer;
}

TimerLibuv::TimerLibuv(uint32_t milliSec)
: mImpl(new Impl(milliSec))
{
}

TimerLibuv::~TimerLibuv()
{
  // stop timers
  Stop();

  delete mImpl;
}

void TimerLibuv::Start()
{
  mImpl->Start(this);
}

void TimerLibuv::Stop()
{
  mImpl->Stop();
}

void TimerLibuv::Pause()
{
  mImpl->Pause();
}

void TimerLibuv::Resume()
{
  mImpl->Resume();
}

void TimerLibuv::SetInterval(uint32_t interval, bool restart)
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

uint32_t TimerLibuv::GetInterval() const
{
  return mImpl->mInterval;
}

bool TimerLibuv::Tick()
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

bool TimerLibuv::IsRunning() const
{
  return mImpl->mRunning;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
