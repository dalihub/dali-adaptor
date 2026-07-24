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
#include <dali/internal/system/windows/trigger-event-win.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <atomic>

// INTERNAL INCLUDES
#include <dali/internal/system/windows/unified-trigger-event-manager-impl-win.h>
#include <dali/internal/window-system/windows/platform-implement-win.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
std::atomic<uint32_t> gUniqueEventId{0u};

uint32_t GetNextEventId()
{
  uint32_t id = gUniqueEventId.fetch_add(1u, std::memory_order_relaxed) + 1u;
  if(DALI_UNLIKELY(id == 0u))
  {
    id = gUniqueEventId.fetch_add(1u, std::memory_order_relaxed) + 1u;
  }
  return id;
}
} // unnamed namespace

TriggerEvent::TriggerEvent(UnifiedTriggerEventManager* manager, CallbackBase* callback)
: mTriggerManager(manager),
  mCallback(callback),
  mSelfCallbackToken(0u),
  mId(GetNextEventId()),
  mThreadId(0u)
{
  if(manager == nullptr)
  {
    mThreadId = static_cast<uint32_t>(WindowsPlatform::GetCurrentThreadId());

    if(mThreadId == 0u)
    {
      DALI_LOG_ERROR("Unable to acquire the TriggerEvent thread id\n");
    }

    mSelfCallbackToken = WindowsPlatform::RegisterWinCallback(MakeCallback(this, &TriggerEvent::Triggered));
  }
}

TriggerEvent::~TriggerEvent()
{
  WindowsPlatform::UnregisterWinCallback(mSelfCallbackToken);
  mSelfCallbackToken = 0u;
  mCallback.reset();
  mThreadId = 0u;
}

void TriggerEvent::Trigger()
{
  if(mThreadId != 0u && mSelfCallbackToken != 0u)
  {
    if(!WindowsPlatform::PostWinCallback(mSelfCallbackToken, mThreadId))
    {
      DALI_LOG_ERROR("Unable to post TriggerEvent[%p] Id(%u)\n", this, mId);
    }
  }
  else if(mTriggerManager)
  {
    GetImplementation(mTriggerManager).Trigger(this);
  }
  else
  {
    DALI_LOG_ERROR("Attempting to write to an invalid file descriptor\n");
  }
}

Dali::UnifiedTriggerEventManager TriggerEvent::GetUnifiedTriggerEventManager() const
{
  return mTriggerManager;
}

void TriggerEvent::Discard()
{
  mTriggerManager.Reset();

  WindowsPlatform::UnregisterWinCallback(mSelfCallbackToken);
  mSelfCallbackToken = 0u;
  mCallback.reset();
}

void TriggerEvent::Triggered()
{
  auto callback = mCallback;
  if(DALI_LIKELY(callback))
  {
    CallbackBase::Execute(*callback);
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
