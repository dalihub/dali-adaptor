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
#include <dali/internal/system/common/trigger-event.h>

// EXTERNAL INCLUDES
#include <unistd.h>

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/file-descriptor-monitor.h>
#include <dali/internal/window-system/windows/platform-implement-win.h>

#define MESSAGE_TYPE_OFFSET     10000

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
TriggerEvent* triggerEventArray[10000] = { 0 };

int SetTriggerEvent(TriggerEvent *event)
{
    for (size_t i = 0; i < 10000; i++)
    {
        if (NULL == triggerEventArray[i])
        {
            triggerEventArray[i] = event;
            return i;
        }
    }

    return -1;
}

TriggerEvent::TriggerEvent( CallbackBase* callback, TriggerEventInterface::Options options )
: mCallback( callback ),
  mFileDescriptor( -1 ),
  mOptions( options )
{
  // Create accompanying file descriptor.
  mFileDescriptor = SetTriggerEvent( this );
  WindowsPlatformImplement::AddListener( mFileDescriptor + MESSAGE_TYPE_OFFSET, mCallback );

  if (mFileDescriptor >= 0)
  {
    // Now Monitor the created event file descriptor
  }
  else
  {
    DALI_LOG_ERROR("Unable to create TriggerEvent File descriptor\n");
  }
}

TriggerEvent::~TriggerEvent()
{
  delete mCallback;

  if (mFileDescriptor >= 0)
  {
    mFileDescriptor = 0;
  }
}

void TriggerEvent::Trigger()
{
  if (mFileDescriptor >= 0)
  {
    // Increment event counter by 1.
    // Writing to the file descriptor triggers the Dispatch() method in the other thread
    // (if in multi-threaded environment).

    WindowsPlatformImplement::PostWinMessage( mFileDescriptor + MESSAGE_TYPE_OFFSET, 0, 0 );
  }
  else
  {
    DALI_LOG_WARNING("Attempting to write to an invalid file descriptor\n");
  }
}

void TriggerEvent::Triggered( FileDescriptorMonitor::EventType eventBitMask )
{
  if( !( eventBitMask & FileDescriptorMonitor::FD_READABLE ) )
  {
    DALI_ASSERT_ALWAYS( 0 && "Trigger event file descriptor error");
    return;
  }

  // Reading from the file descriptor resets the event counter, we can ignore the count.
  uint64_t receivedData;
  size_t size;
  size = read(mFileDescriptor, &receivedData, sizeof(uint64_t));
  if (size != sizeof(uint64_t))
  {
    DALI_LOG_WARNING("Unable to read to UpdateEvent File descriptor\n");
  }

  // Call the connected callback
  CallbackBase::Execute( *mCallback );

  //check if we should delete ourselves after the trigger
  if( mOptions == TriggerEventInterface::DELETE_AFTER_TRIGGER )
  {
    delete this;
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
