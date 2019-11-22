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
#include <dali/internal/system/windows/trigger-event.h>

// EXTERNAL INCLUDES
#include <unistd.h>

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/file-descriptor-monitor.h>
#include <dali/internal/window-system/windows/platform-implement-win.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

TriggerEvent::TriggerEvent( CallbackBase* callback, TriggerEventInterface::Options options )
: mCallback( callback ),
  mThreadID( -1 ),
  mOptions( options )
{
  // Create accompanying file descriptor.
  mThreadID = WindowsPlatformImplementation::GetCurrentThreadId();

  if ( mThreadID < 0)
  {
    DALI_LOG_ERROR("Unable to create TriggerEvent File descriptor\n");
  }

  mSelfCallback = MakeCallback( this, &TriggerEvent::Triggered );
}

TriggerEvent::~TriggerEvent()
{
  delete mCallback;
  delete mSelfCallback;

  if ( mThreadID >= 0)
  {
    mThreadID = 0;
  }
}

void TriggerEvent::Trigger()
{
  if ( mThreadID >= 0)
  {
    // Increment event counter by 1.
    // Writing to the file descriptor triggers the Dispatch() method in the other thread
    // (if in multi-threaded environment).
    WindowsPlatformImplementation::PostWinThreadMessage( WIN_CALLBACK_EVENT, reinterpret_cast<uint64_t>( mSelfCallback ), 0, mThreadID );
  }
  else
  {
    DALI_LOG_WARNING("Attempting to write to an invalid file descriptor\n");
  }
}

void TriggerEvent::Triggered()
{
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
