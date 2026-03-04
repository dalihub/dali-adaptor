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
#include <dali/internal/legacy/common/tizen-platform-abstraction.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/signals/callback.h>
#include <algorithm>
#include <fstream>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/system-error-print.h>

namespace Dali
{
namespace TizenPlatform
{
struct TizenPlatformAbstraction::TimerCallback : ConnectionTracker
{
  Dali::Timer                   mTimer;
  TizenPlatformAbstraction*     mOwner;
  std::unique_ptr<CallbackBase> mCallback;
  const uint32_t                mIdNumber;

  static uint32_t sNextTimerId;

  TimerCallback(TizenPlatformAbstraction* owner, CallbackBase* callback, uint32_t ms)
  : mTimer(Dali::Timer::New(ms)),
    mOwner(owner),
    mCallback(std::unique_ptr<CallbackBase>(callback)),
    mIdNumber(++sNextTimerId)
  {
    mTimer.TickSignal().Connect(this, &TimerCallback::Tick);
    mTimer.Start();
  }
  ~TimerCallback()
  {
  }

  bool Tick()
  {
    mOwner->RunTimerFunction(*this);
    return false;
  }
};

uint32_t TizenPlatformAbstraction::TimerCallback::sNextTimerId = 0;

TizenPlatformAbstraction::TizenPlatformAbstraction()
: mTimerPairsWaiting(),
  mTimerPairsSpent(),
  mCleanupIdleCallback(nullptr)
{
}

TizenPlatformAbstraction::~TizenPlatformAbstraction()
{
  if(mCleanupIdleCallback && Dali::Adaptor::IsAvailable())
  {
    Dali::Adaptor::Get().RemoveIdle(mCleanupIdleCallback);
    mCleanupIdleCallback = nullptr;
  }
}

uint32_t TizenPlatformAbstraction::StartTimer(uint32_t milliseconds, CallbackBase* callback)
{
  TimerCallback* timerCallbackPtr = new TimerCallback(this, callback, milliseconds);

  // Stick it in the list
  mTimerPairsWaiting.push_back(std::unique_ptr<TimerCallback>(timerCallbackPtr));

  return timerCallbackPtr->mIdNumber;
}

void TizenPlatformAbstraction::CancelTimer(uint32_t timerId)
{
  std::vector<std::unique_ptr<TimerCallback> >::iterator timerIter = std::find_if(mTimerPairsWaiting.begin(), mTimerPairsWaiting.end(), [&timerId](std::unique_ptr<TimerCallback>& timerCallbackPtr)
  {
    if(timerCallbackPtr->mIdNumber == timerId)
    {
      timerCallbackPtr->mTimer.Stop();
      return true;
    }
    else
    {
      return false;
    }
  });

  if(timerIter == std::end(mTimerPairsWaiting))
  {
    DALI_LOG_DEBUG_INFO("TimerId %u Cancelled duplicated.\n", timerId);
    return;
  }

  // Move the canceled item to the spent list.
  std::move(timerIter, timerIter + 1, std::back_inserter(mTimerPairsSpent));

  mTimerPairsWaiting.erase(timerIter, timerIter + 1);

  RequestCleanupTimers();
}

void TizenPlatformAbstraction::RunTimerFunction(TimerCallback& timerPtr)
{
  CallbackBase::Execute(*timerPtr.mCallback);

  std::vector<std::unique_ptr<TimerCallback> >::iterator timerIter = std::find_if(mTimerPairsWaiting.begin(), mTimerPairsWaiting.end(), [&](std::unique_ptr<TimerCallback>& p)
  { return p.get() == &timerPtr; });

  if(timerIter == std::end(mTimerPairsWaiting))
  {
    // It might be cancel during callback execute.
    DALI_LOG_DEBUG_INFO("Timer might be cancelled during execute.\n");
    return;
  }

  // Move the executed item to the spent list.
  std::move(timerIter, timerIter + 1, std::back_inserter(mTimerPairsSpent));

  mTimerPairsWaiting.erase(timerIter, timerIter + 1);

  RequestCleanupTimers();
}

void TizenPlatformAbstraction::CleanupTimers()
{
  // Idler callback called. Remove it.
  mCleanupIdleCallback = nullptr;

  mTimerPairsSpent.clear();
}

void TizenPlatformAbstraction::RequestCleanupTimers()
{
  if(!mCleanupIdleCallback)
  {
    mCleanupIdleCallback = MakeCallback(this, &TizenPlatformAbstraction::CleanupTimers);
    if(DALI_UNLIKELY(!Dali::Adaptor::IsAvailable() || !Dali::Adaptor::Get().AddIdle(mCleanupIdleCallback, false)))
    {
      DALI_LOG_ERROR("Fail to add idle callback for timer function. Call it synchronously.\n");
      CleanupTimers();
    }
  }
}

TizenPlatformAbstraction* CreatePlatformAbstraction()
{
  return new TizenPlatformAbstraction();
}

bool SaveFile(const std::string& filename, const unsigned char* buffer, unsigned int numBytes)
{
  DALI_ASSERT_DEBUG(0 != filename.length());

  bool result = false;

  std::filebuf buf;
  buf.open(filename.c_str(), std::ios::out | std::ios_base::trunc | std::ios::binary);
  if(buf.is_open())
  {
    std::ostream stream(&buf);

    // determine size of buffer
    int length = static_cast<int>(numBytes);

    // write contents of buffer to the file
    stream.write(reinterpret_cast<const char*>(buffer), length);

    if(!stream.bad())
    {
      result = true;
    }
    else
    {
      DALI_LOG_ERROR("std::ostream.write failed! [numBytes:%d]\n", numBytes);
    }
  }
  else
  {
    DALI_LOG_ERROR("std::filebuf.open failed!\n");
  }

  if(!result)
  {
    DALI_LOG_ERROR("Can't write to %s. buffer pointer : %p, length : %u\n", filename.c_str(), buffer, numBytes);
    DALI_PRINT_SYSTEM_ERROR_LOG();
  }

  return result;
}

} // namespace TizenPlatform

} // namespace Dali
