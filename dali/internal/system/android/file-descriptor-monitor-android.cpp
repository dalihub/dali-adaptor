/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/common/file-descriptor-monitor.h>

// EXTERNAL INCLUDES
#include <looper.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Using Impl to hide away Android specific members
 */
struct FileDescriptorMonitor::Impl
{
  // Construction
  Impl( int fileDescriptor, CallbackBase* callback, int eventBitmask )
  : mCallback( callback ),
    mFileDescriptor( fileDescriptor ),
    mEventsToMonitor( eventBitmask )
  {
  }

  ~Impl()
  {
    delete mCallback;
  }

  // Data
  CallbackBase* mCallback;
  int mFileDescriptor;
  int mEventsToMonitor;

  // Static Methods

  /**
   * Called when the file descriptor receives an event.
   */
  static int EventDispatch( int fd, int events, void* data )
  {
    Impl* impl = reinterpret_cast<Impl*>(data);

    // if we want read events, check to see if a read event is available
    int type = FileDescriptorMonitor::FD_NO_EVENT;

    if( impl->mEventsToMonitor & ALOOPER_EVENT_INPUT )
    {

      type = FileDescriptorMonitor::FD_READABLE;

    }
    // check if we want write events
    if( impl->mEventsToMonitor & ALOOPER_EVENT_OUTPUT )
    {

      type |= FileDescriptorMonitor::FD_WRITABLE;

    }

    // if there is an event, execute the callback
    if( type != FileDescriptorMonitor::FD_NO_EVENT )
    {
      CallbackBase::Execute( *impl->mCallback, static_cast< FileDescriptorMonitor::EventType >( type ) );
    }

    return 1; // Continue receiving callbacks
  }
};

FileDescriptorMonitor::FileDescriptorMonitor( int fileDescriptor, CallbackBase* callback, int eventBitmask )
{
  mImpl = new Impl(fileDescriptor, callback, eventBitmask);

  if (fileDescriptor >= 0)
  {
    int events = 0;
    if( eventBitmask & FD_READABLE)
    {
      events = ALOOPER_EVENT_INPUT;
    }
    if( eventBitmask & FD_WRITABLE)
    {
      events |= ALOOPER_EVENT_OUTPUT;
    }

    mImpl->mEventsToMonitor = events;

    ALooper* looper = ALooper_forThread();
    if( looper )
    {
      ALooper_addFd( looper, fileDescriptor, ALOOPER_POLL_CALLBACK, events, &Impl::EventDispatch, mImpl );
    }
  }
}

FileDescriptorMonitor::~FileDescriptorMonitor()
{
  if( mImpl->mFileDescriptor )
  {
    ALooper* looper = ALooper_forThread();
    if( looper )
    {
      ALooper_removeFd( looper, mImpl->mFileDescriptor );
    }
  }

  delete mImpl;
  mImpl = NULL;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

