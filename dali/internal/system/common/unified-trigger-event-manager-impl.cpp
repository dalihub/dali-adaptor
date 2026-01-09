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
#include <dali/internal/system/common/unified-trigger-event-manager-impl.h>

// EXTERNAL INCLUDES
#include <sys/eventfd.h>
#include <unistd.h>

#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/trace.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/file-descriptor-monitor.h>
#include <dali/internal/system/common/system-error-print.h>
#include <dali/internal/system/common/system-factory.h>
#include <dali/internal/system/common/trigger-event.h>

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
      // If so, downcast the handle
      unifiedTriggerEventManager = Dali::UnifiedTriggerEventManager(dynamic_cast<UnifiedTriggerEventManager*>(handle.GetObjectPtr()));
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
: mFileDescriptorMonitor(),
  mValidEventsId(),
  mFileDescriptor(-1),
  mTriggerMutex{},
  mTriggeredEvents(),
  mFileDescriptorWritten(false)
{
  mFileDescriptor = eventfd(0, EFD_NONBLOCK);
  if(mFileDescriptor >= 0)
  {
    DALI_LOG_DEBUG_INFO("Create unified eventfd:%d\n", mFileDescriptor);
    // Now Monitor the created event file descriptor
    mFileDescriptorMonitor = Dali::Internal::Adaptor::GetSystemFactory()->CreateFileDescriptorMonitor(mFileDescriptor, MakeCallback(this, &UnifiedTriggerEventManager::Triggered), FileDescriptorMonitor::FD_READABLE);
  }
  else
  {
    DALI_LOG_ERROR("Unable to create UnifiedTriggerEventManager File descriptor\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
  }
}

UnifiedTriggerEventManager::~UnifiedTriggerEventManager()
{
  if(mFileDescriptor >= 0)
  {
    DALI_LOG_DEBUG_INFO("Close unified eventfd:%d\n", mFileDescriptor);
    mFileDescriptorMonitor.reset();
    close(mFileDescriptor);
    mFileDescriptor = -1;
  }

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

TriggerEvent* UnifiedTriggerEventManager::GenerateTriggerEvent(CallbackBase* callback, TriggerEventInterface::Options options)
{
  auto* triggerEvent = new Internal::Adaptor::TriggerEvent(this, callback, options);
  mValidEventsId.insert(triggerEvent->GetId());
  DALI_LOG_DEBUG_INFO("Generated Trigger[%p] Id(%u) options:%d\n", triggerEvent, triggerEvent->GetId(), static_cast<int>(options));
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

void UnifiedTriggerEventManager::Triggered(FileDescriptorMonitor::EventType eventBitMask, int fileDescriptor)
{
  DALI_LOG_DEBUG_INFO("Triggered fd:%d, mask:%x\n", mFileDescriptor, eventBitMask);
  if(!(eventBitMask & FileDescriptorMonitor::FD_READABLE))
  {
    DALI_ASSERT_ALWAYS(0 && "Trigger event file descriptor error");
    return;
  }

  // Reading from the file descriptor resets the event counter, we can ignore the count.
  uint64_t receivedData;
  size_t   size;
  size = read(mFileDescriptor, &receivedData, sizeof(uint64_t));
  if(size != sizeof(uint64_t))
  {
    DALI_LOG_ERROR("Unable to read to UpdateEvent File descriptor\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
  }

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
      triggerEvent->Triggered(eventBitMask, fileDescriptor);
    }
    else if(DALI_UNLIKELY(triggerEvent))
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
  DALI_LOG_DEBUG_INFO("Trigger[%p] Id(%u) fd:%d\n", triggerEvent, DALI_LIKELY(triggerEvent) ? triggerEvent->GetId() : 0, mFileDescriptor);

  if(DALI_LIKELY(mFileDescriptor >= 0))
  {
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
      // Increment event counter by 1.
      // Writing to the file descriptor triggers the Dispatch() method in the other thread
      // (if in multi-threaded environment).
      uint64_t data = 1;
      int      size = write(mFileDescriptor, &data, sizeof(uint64_t));

      if(size != sizeof(uint64_t))
      {
        DALI_LOG_ERROR("Unable to write to UpdateEvent File descriptor\n");
        DALI_PRINT_SYSTEM_ERROR_LOG();
      }
    }
  }
}
} // namespace Dali::Internal::Adaptor
