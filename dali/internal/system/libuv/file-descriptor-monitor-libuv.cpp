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
#include <dali/internal/system/libuv/file-descriptor-monitor-libuv.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
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
void FreeHandleCallback(uv_handle_t* handle)
{
  delete handle;
}

} // namespace

/**
 * Using Impl to hide away UV specific members
 */
struct FileDescriptorMonitorLibuv::Impl
{
public:
  // Constructor
  Impl(int fileDescriptor, CallbackBase* callback, uv_poll_event eventsToMonitor)
  : mFileDescriptor(fileDescriptor),
    mEventsToMonitor(eventsToMonitor),
    mCallback(callback),
    pollHandle(NULL)
  {
    // heap allocate a handle as it will be alive after the FileDescriptorMonitor::Impl object is deleted.
    pollHandle = new uv_poll_t;

    uv_poll_init(GetUVMainLoop(), pollHandle, fileDescriptor);

    pollHandle->data = this;

    uv_poll_start(pollHandle, mEventsToMonitor, PollCallback);
  }

  ~Impl()
  {
    uv_poll_stop(pollHandle);

    // the handle will still be alive for a short period after calling uv_close
    // set the data to NULL to avoid a dangling pointer
    pollHandle->data = NULL;

    uv_close(reinterpret_cast<uv_handle_t*>(pollHandle), FreeHandleCallback);

    delete mCallback;
  }

  static void PollCallback(uv_poll_t* handle, int status, int events)
  {
    if(handle->data)
    {
      FileDescriptorMonitorLibuv::Impl* impl = static_cast<FileDescriptorMonitorLibuv::Impl*>(handle->data);

      if(status < 0)
      {
        DALI_LOG_ERROR("LibUV FD_ERROR occurred on %d: %s\n", impl->mFileDescriptor, strerror(-status));
        CallbackBase::Execute(*impl->mCallback, FileDescriptorMonitor::FD_ERROR);
        return;
      }
      // filter the events that have occured based on what we are monitoring

      int eventType = FileDescriptorMonitor::FD_NO_EVENT;

      if((impl->mEventsToMonitor & UV_READABLE) && (events & UV_READABLE))
      {
        eventType = FileDescriptorMonitor::FD_READABLE;
      }
      if((impl->mEventsToMonitor & UV_WRITABLE) && (events & UV_WRITABLE))
      {
        eventType |= FileDescriptorMonitor::FD_WRITABLE;
      }

      // if there is an event, execute the callback
      if(eventType != FileDescriptorMonitor::FD_NO_EVENT)
      {
        CallbackBase::Execute(*impl->mCallback, static_cast<FileDescriptorMonitor::EventType>(eventType));
      }
    }
  }
  // Data
  int           mFileDescriptor;
  uv_poll_event mEventsToMonitor;
  CallbackBase* mCallback;
  uv_poll_t*    pollHandle;
};

FileDescriptorMonitorLibuv::FileDescriptorMonitorLibuv(int fileDescriptor, CallbackBase* callback, int eventBitmask)
: FileDescriptorMonitor(fileDescriptor, callback, eventBitmask)
{
  if(fileDescriptor < 1)
  {
    DALI_ASSERT_ALWAYS(0 && "Invalid File descriptor");
    return;
  }
  int events = 0;
  if(eventBitmask & FD_READABLE)
  {
    events = UV_READABLE;
  }
  if(eventBitmask & FD_WRITABLE)
  {
    events |= UV_WRITABLE;
  }

  DALI_ASSERT_ALWAYS(events && "Invalid FileDescriptorMonitor event type ");

  // waiting for a write event on a file descriptor
  mImpl = new Impl(fileDescriptor, callback, static_cast<uv_poll_event>(events));
}

FileDescriptorMonitorLibuv::~FileDescriptorMonitorLibuv()
{
  delete mImpl;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
