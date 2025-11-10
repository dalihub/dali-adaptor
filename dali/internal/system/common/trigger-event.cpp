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
#include <dali/internal/system/common/trigger-event.h>

// EXTERNAL INCLUDES
#include <sys/eventfd.h>
#include <unistd.h>

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>
#include <dali/internal/system/common/file-descriptor-monitor.h>
#include <dali/internal/system/common/system-error-print.h>
#include <dali/internal/system/common/system-factory.h>
#include <dali/internal/system/common/unified-trigger-event-manager-impl.h>
#include <dali/internal/system/common/unified-trigger-event-manager.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
static uint32_t gUniqueEventId = 0u;
}

TriggerEvent::TriggerEvent(UnifiedTriggerEventManager* manager, CallbackBase* callback, TriggerEventInterface::Options options)
: mTriggerManager(manager),
  mCallback(callback),
  mId(++gUniqueEventId),
  mOptions(options),
  mFileDescriptorMonitor(),
  mFileDescriptor(-1)
{
  // Create accompanying file descriptor.
  if(manager == nullptr)
  {
    mFileDescriptor = eventfd(0, EFD_NONBLOCK);
    if(mFileDescriptor >= 0)
    {
      DALI_LOG_DEBUG_INFO("Create TrigerEvent[%p] Id(%u), eventfd:%d option:%d\n", this, mId, mFileDescriptor, static_cast<int>(options));
      // Now Monitor the created event file descriptor
      mFileDescriptorMonitor = Dali::Internal::Adaptor::GetSystemFactory()->CreateFileDescriptorMonitor(mFileDescriptor, MakeCallback(this, &TriggerEvent::Triggered), FileDescriptorMonitor::FD_READABLE);
    }
    else
    {
      DALI_LOG_ERROR("Unable to create TriggerEvent File descriptor\n");
      DALI_PRINT_SYSTEM_ERROR_LOG();
    }
  }
}

TriggerEvent::~TriggerEvent()
{
  delete mCallback;

  if(mFileDescriptor >= 0)
  {
    DALI_LOG_DEBUG_INFO("Delete TrigerEvent[%p] Id(%u), eventfd:%d\n", this, mId, mFileDescriptor);
    close(mFileDescriptor);
    mFileDescriptor = 0;
  }
}

void TriggerEvent::Trigger()
{
  if(mFileDescriptor >= 0)
  {
    DALI_LOG_DEBUG_INFO("Trigger TrigerEvent[%p] Id(%u), eventfd:%d\n", this, mId, mFileDescriptor);
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
  // Remove reference count of unified trigger event manager
  mTriggerManager.Reset();

  delete mCallback;
  mCallback = nullptr;
}

void TriggerEvent::Triggered(FileDescriptorMonitor::EventType eventBitMask, int fileDescriptor)
{
  if(!(eventBitMask & FileDescriptorMonitor::FD_READABLE))
  {
    DALI_ASSERT_ALWAYS(0 && "Trigger event file descriptor error");
    return;
  }

  // Reading from the file descriptor resets the event counter, we can ignore the count.
  if(mFileDescriptor >= 0)
  {
    DALI_LOG_DEBUG_INFO("Triggered[%p] Id(%u), eventfd:%d mask:%x\n", this, mId, mFileDescriptor, eventBitMask);
    uint64_t receivedData;
    size_t   size;
    size = read(mFileDescriptor, &receivedData, sizeof(uint64_t));
    if(size != sizeof(uint64_t))
    {
      DALI_LOG_ERROR("Unable to read to UpdateEvent File descriptor\n");
      DALI_PRINT_SYSTEM_ERROR_LOG();
    }
  }

  // Save value to prevent duplicate deletion
  TriggerEventInterface::Options options = mOptions;

  // Call the connected callback
  if(DALI_LIKELY(mCallback))
  {
    CallbackBase::Execute(*mCallback);
  }

  // check if we should delete ourselves after the trigger
  if(options == TriggerEventInterface::DELETE_AFTER_TRIGGER)
  {
    Dali::TriggerEventFactory::DestroyTriggerEvent(this);
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
