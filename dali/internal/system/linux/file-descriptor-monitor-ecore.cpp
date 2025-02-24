/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/linux/file-descriptor-monitor-ecore.h>

// EXTERNAL INCLUDES
#include <dali/internal/system/linux/dali-ecore.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Using Impl to hide away EFL specific members
 */
struct FileDescriptorMonitorEcore::Impl
{
  // Construction
  Impl(int fileDescriptor, CallbackBase* callback, int eventsToMonitor)
  : mFileDescriptor(fileDescriptor),
    mEventsToMonitor(eventsToMonitor),
    mCallback(callback),
    mHandler(NULL)
  {
  }

  ~Impl()
  {
    delete mCallback;
  }

  // Data
  int               mFileDescriptor;
  int               mEventsToMonitor; ///< what file descriptor events to monitor
  CallbackBase*     mCallback;
  Ecore_Fd_Handler* mHandler;

  // Static Methods

  /**
   * Called when the file descriptor receives an event.
   */
  static Eina_Bool EventDispatch(void* data, Ecore_Fd_Handler* handler)
  {
    Impl* impl = reinterpret_cast<Impl*>(data);

    // if we want read events, check to see if a read event is available
    int type = FileDescriptorMonitor::FD_NO_EVENT;

    if(ecore_main_fd_handler_active_get(handler, ECORE_FD_ERROR))
    {
      DALI_LOG_ERROR("ECORE_FD_ERROR occurred on %d\n", impl->mFileDescriptor);
      CallbackBase::Execute(*impl->mCallback, FileDescriptorMonitor::FD_ERROR, impl->mFileDescriptor);

      return ECORE_CALLBACK_CANCEL;
    }

    if(impl->mEventsToMonitor & ECORE_FD_READ)
    {
      if(ecore_main_fd_handler_active_get(handler, ECORE_FD_READ))
      {
        type = FileDescriptorMonitor::FD_READABLE;
      }
    }
    // check if we want write events
    if(impl->mEventsToMonitor & ECORE_FD_WRITE)
    {
      if(ecore_main_fd_handler_active_get(handler, ECORE_FD_WRITE))
      {
        type |= FileDescriptorMonitor::FD_WRITABLE;
      }
    }

    // if there is an event, execute the callback
    if(type != FileDescriptorMonitor::FD_NO_EVENT)
    {
      CallbackBase::Execute(*impl->mCallback, static_cast<FileDescriptorMonitor::EventType>(type), impl->mFileDescriptor);
    }

    return ECORE_CALLBACK_RENEW;
  }
};

FileDescriptorMonitorEcore::FileDescriptorMonitorEcore(int fileDescriptor, CallbackBase* callback, int eventBitmask)
: FileDescriptorMonitor(fileDescriptor, callback, eventBitmask)
{
  mImpl = new Impl(fileDescriptor, callback, eventBitmask);

  if(fileDescriptor < 0)
  {
    DALI_ASSERT_ALWAYS(0 && "Invalid File descriptor");
    return;
  }

  int events = 0;
  if(eventBitmask & FD_READABLE)
  {
    events = ECORE_FD_READ;
  }
  if(eventBitmask & FD_WRITABLE)
  {
    events |= ECORE_FD_WRITE;
  }
  mImpl->mEventsToMonitor = events;
  mImpl->mHandler         = ecore_main_fd_handler_add(fileDescriptor, static_cast<Ecore_Fd_Handler_Flags>(events), &Impl::EventDispatch, mImpl, NULL, NULL);
}

FileDescriptorMonitorEcore::~FileDescriptorMonitorEcore()
{
  if(mImpl->mHandler)
  {
    try
    {
      ecore_main_fd_handler_del(mImpl->mHandler);
    }
    catch(std::bad_weak_ptr const& ex)
    {
      DALI_LOG_ERROR("FileDescriptorMonitorEcore::~FileDescriptorMonitorEcore() - std::bad_weak_ptr caught: %s\n", ex.what());
    }
  }

  delete mImpl;
  mImpl = NULL;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
