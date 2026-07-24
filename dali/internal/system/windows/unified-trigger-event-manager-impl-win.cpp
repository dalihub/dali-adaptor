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
#include <dali/internal/system/windows/unified-trigger-event-manager-impl-win.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/trace.h>

// INTERNAL INCLUDES
#include <dali/internal/system/windows/trigger-event-win.h>
#include <dali/internal/window-system/windows/platform-implement-win.h>

namespace
{
DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_PERFORMANCE_MARKER, false);
} //namespace

namespace Dali::Internal::Adaptor
{
Dali::UnifiedTriggerEventManager UnifiedTriggerEventManager::Get()
{
  Dali::UnifiedTriggerEventManager unifiedTriggerEventManager;

  Dali::SingletonService service(SingletonService::Get());
  if(service)
  {
    // Check whether the singleton is already created
    Dali::BaseHandle handle = service.GetSingleton(typeid(Dali::UnifiedTriggerEventManager));
    if(handle)
    {
      unifiedTriggerEventManager = Dali::UnifiedTriggerEventManager(static_cast<UnifiedTriggerEventManager*>(handle.GetObjectPtr()));
    }
    else
    {
      unifiedTriggerEventManager = Dali::UnifiedTriggerEventManager(new UnifiedTriggerEventManager());
      service.Register(typeid(unifiedTriggerEventManager), unifiedTriggerEventManager);
    }
  }

  return unifiedTriggerEventManager;
}

UnifiedTriggerEventManager::UnifiedTriggerEventManager()
: mSelfCallbackToken(0u),
  mValidEventsId(),
  mDiscardedEvents(),
  mThreadId(0u),
  mTriggerMutex{},
  mTriggeredEvents(),
  mFileDescriptorWritten(false)
{
  mThreadId = static_cast<uint32_t>(WindowsPlatform::GetCurrentThreadId());
  if(mThreadId == 0u)
  {
    DALI_LOG_ERROR("Unable to acquire the UnifiedTriggerEventManager thread id\n");
  }

  mSelfCallbackToken = WindowsPlatform::RegisterWinCallback(MakeCallback(this, &UnifiedTriggerEventManager::Triggered));
}

UnifiedTriggerEventManager::~UnifiedTriggerEventManager()
{
  WindowsPlatform::UnregisterWinCallback(mSelfCallbackToken);
  mSelfCallbackToken = 0u;

  mThreadId = 0u;

  {
    Dali::Mutex::ScopedLock mutexLock(mTriggerMutex);
    mTriggeredEvents.clear();
    mFileDescriptorWritten = false;
  }
  mValidEventsId.clear();

  for(auto* discardedEvent : mDiscardedEvents)
  {
    delete discardedEvent;
  }
  mDiscardedEvents.clear();
}

TriggerEvent* UnifiedTriggerEventManager::GenerateTriggerEvent(CallbackBase* callback)
{
  auto* triggerEvent = new Internal::Adaptor::TriggerEvent(this, callback);
  mValidEventsId.insert(triggerEvent->GetId());
  DALI_LOG_DEBUG_INFO("Generated Trigger[%p] Id(%u)\n", triggerEvent, triggerEvent->GetId());
  return triggerEvent;
}

void UnifiedTriggerEventManager::DiscardTriggerEvent(TriggerEvent* triggerEvent)
{
  if(DALI_LIKELY(triggerEvent))
  {
    DALI_LOG_DEBUG_INFO("Discard Trigger[%p] Id(%u)\n", triggerEvent, triggerEvent->GetId());
    mValidEventsId.erase(triggerEvent->GetId());
    mDiscardedEvents.insert(triggerEvent);

    // To make ensure to call Triggered callback
    Trigger(nullptr);

    // Discard the trigger event
    triggerEvent->Discard();
  }
}

void UnifiedTriggerEventManager::Triggered()
{
  // DevNote : Gattering discarded events before triggered events get.
  decltype(mDiscardedEvents) discardedEvents;
  discardedEvents.swap(mDiscardedEvents);

  decltype(mTriggeredEvents) triggeredEvents;
  {
    Dali::Mutex::ScopedLock mutexLock(mTriggerMutex);
    // DevNote : Get triggered event after read fd.
    mTriggeredEvents.swap(triggeredEvents);
    mFileDescriptorWritten = false;
  }

  // Keep reference of self during trigger events.
  Dali::UnifiedTriggerEventManager handle(this);

  DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_UNIFIED_TRIGGER", [&](std::ostringstream& oss)
  { oss << "[" << triggeredEvents.size() << ", " << mValidEventsId.size() << ", " << discardedEvents.size() << "]"; });
  for(auto* triggerEvent : triggeredEvents)
  {
    // Check validation of trigger event.
    if(DALI_LIKELY(triggerEvent && mValidEventsId.find(triggerEvent->GetId()) != mValidEventsId.end()))
    {
      DALI_LOG_DEBUG_INFO("Triggered[%p] Id(%u)\n", triggerEvent, triggerEvent->GetId());
      triggerEvent->Triggered();
    }
    else
    {
      DALI_LOG_DEBUG_INFO("Ignore invalid trigger[%p]\n", triggerEvent);
    }
  }

  DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_UNIFIED_TRIGGER", [&](std::ostringstream& oss)
  { oss << "[" << mValidEventsId.size() << ", " << mDiscardedEvents.size() << "]"; });

  for(auto* discardedEvent : discardedEvents)
  {
    delete discardedEvent;
  }

  if(DALI_UNLIKELY(!mDiscardedEvents.empty()))
  {
    // To make ensure to call Triggered callback
    Trigger(nullptr);
  }
}

// From worker thread.
void UnifiedTriggerEventManager::Trigger(TriggerEvent* triggerEvent)
{
  DALI_LOG_DEBUG_INFO("Trigger[%p] Id(%u) token:%llu\n", triggerEvent, DALI_LIKELY(triggerEvent) ? triggerEvent->GetId() : 0, static_cast<unsigned long long>(mSelfCallbackToken));

  bool writeRequired = true;
  {
    Dali::Mutex::ScopedLock mutexLock(mTriggerMutex);
    // DevNote : Insert triggered event before write fd.
    mTriggeredEvents.insert(triggerEvent);
    if(mFileDescriptorWritten)
    {
      writeRequired = false;
    }
    else
    {
      mFileDescriptorWritten = writeRequired = true;
    }
  }

  if(writeRequired)
  {
    if(mThreadId == 0u || mSelfCallbackToken == 0u || !WindowsPlatform::PostWinCallback(mSelfCallbackToken, mThreadId))
    {
      Dali::Mutex::ScopedLock mutexLock(mTriggerMutex);
      mFileDescriptorWritten = false;
      DALI_LOG_ERROR("Unable to post a unified TriggerEvent to thread %u\n", mThreadId);
    }
  }
}
} // namespace Dali::Internal::Adaptor
