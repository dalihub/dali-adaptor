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
#include <dali/internal/system/linux/tcore/file-descriptor-monitor-tcore.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <tizen_core.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
tizen_core_h GetTizenCore()
{
  tizen_core_h core = nullptr;
  if(tizen_core_find("main", &core) != TIZEN_CORE_ERROR_NONE)
  {
    tizen_core_find_from_this_thread(&core);
  }
  return core;
}
} // unnamed namespace

struct FileDescriptorMonitorTcore::Impl
{
  Impl(int fileDescriptor, CallbackBase* callback, int eventsToMonitor)
  : mFileDescriptor(fileDescriptor),
    mEventsToMonitor(eventsToMonitor),
    mCallback(callback),
    mSource(nullptr),
    mPollFd(nullptr)
  {
  }

  ~Impl()
  {
    delete mCallback;
  }

  static bool SourcePrepare(tizen_core_source_h /*source*/, int* timeout, void* /*userData*/)
  {
    if(timeout)
    {
      *timeout = -1;
    }
    return false;
  }

  static bool SourceCheck(tizen_core_source_h /*source*/, void* userData)
  {
    Impl* impl = static_cast<Impl*>(userData);
    if(!impl || !impl->mPollFd)
    {
      DALI_LOG_ERROR("FileDescriptorMonitorTcore::SourceCheck: invalid impl or mPollFd\n");
      return false;
    }

    if(!DALI_LIKELY(Dali::Adaptor::IsAvailable()))
    {
      DALI_LOG_ERROR("FD source check comes after adaptor invalidated. Ignore.\n");
      return false;
    }

    uint16_t returnedEvents = 0;
    const int ret = tizen_core_poll_fd_get_returned_events(impl->mPollFd, &returnedEvents);
    if(ret != TIZEN_CORE_ERROR_NONE)
    {
      DALI_LOG_ERROR("FileDescriptorMonitorTcore::SourceCheck: tizen_core_poll_fd_get_returned_events failed: %d\n", ret);
      return false;
    }

    if(returnedEvents & (TIZEN_CORE_POLL_EVENT_ERR | TIZEN_CORE_POLL_EVENT_HUP | TIZEN_CORE_POLL_EVENT_NVAL))
    {
      return true;
    }

    if((returnedEvents & TIZEN_CORE_POLL_EVENT_IN) || (returnedEvents & TIZEN_CORE_POLL_EVENT_OUT))
    {
      return true;
    }

    return false;
  }

  static bool SourceDispatch(tizen_core_source_h /*source*/, void* userData)
  {
    Impl* impl = static_cast<Impl*>(userData);
    if(!impl || !impl->mPollFd || !impl->mCallback)
    {
      DALI_LOG_ERROR("FileDescriptorMonitorTcore::SourceDispatch: invalid impl or mPollFd or mCallback\n");
      return false;
    }

    if(!DALI_LIKELY(Dali::Adaptor::IsAvailable()))
    {
      DALI_LOG_ERROR("FD dispatch signal comes after adaptor invalidated. Ignore dispatch callback.\n");
      return false;
    }

    uint16_t returnedEvents = 0;
    const int ret = tizen_core_poll_fd_get_returned_events(impl->mPollFd, &returnedEvents);
    if(ret != TIZEN_CORE_ERROR_NONE)
    {
      DALI_LOG_ERROR("FileDescriptorMonitorTcore::SourceDispatch: tizen_core_poll_fd_get_returned_events failed: %d\n", ret);
      return false;
    }

    int type = FileDescriptorMonitor::FD_NO_EVENT;

    if(returnedEvents & (TIZEN_CORE_POLL_EVENT_ERR | TIZEN_CORE_POLL_EVENT_HUP | TIZEN_CORE_POLL_EVENT_NVAL))
    {
      type = FileDescriptorMonitor::FD_ERROR;
    }
    else
    {
      if(impl->mEventsToMonitor & TIZEN_CORE_POLL_EVENT_IN)
      {
        if(returnedEvents & TIZEN_CORE_POLL_EVENT_IN)
        {
          type |= FileDescriptorMonitor::FD_READABLE;
        }
      }

      if(impl->mEventsToMonitor & TIZEN_CORE_POLL_EVENT_OUT)
      {
        if(returnedEvents & TIZEN_CORE_POLL_EVENT_OUT)
        {
          type |= FileDescriptorMonitor::FD_WRITABLE;
        }
      }
    }

    if(type != FileDescriptorMonitor::FD_NO_EVENT)
    {
      CallbackBase::Execute(*impl->mCallback, static_cast<FileDescriptorMonitor::EventType>(type), impl->mFileDescriptor);
    }

    return type != FileDescriptorMonitor::FD_ERROR;
  }

  static void SourceFinalize(tizen_core_source_h /*source*/, void* userData)
  {
    Impl* impl = static_cast<Impl*>(userData);
    if(!impl)
    {
      DALI_LOG_ERROR("FileDescriptorMonitorTcore::SourceFinalize: invalid impl\n");
      return;
    }

    if(!DALI_LIKELY(Dali::Adaptor::IsAvailable()))
    {
      DALI_LOG_ERROR("FD source finalize comes after adaptor invalidated. Ignore.\n");
      return;
    }

    if(impl->mPollFd)
    {
      tizen_core_poll_fd_destroy(impl->mPollFd);
      impl->mPollFd = nullptr;
    }
  }

  int                  mFileDescriptor;
  uint16_t             mEventsToMonitor;
  CallbackBase*        mCallback;
  tizen_core_source_h  mSource;
  tizen_core_poll_fd_h mPollFd;
};

FileDescriptorMonitorTcore::FileDescriptorMonitorTcore(int fileDescriptor, CallbackBase* callback, int eventBitmask)
: FileDescriptorMonitor(fileDescriptor, callback, eventBitmask),
  mImpl(nullptr)
{
  mImpl = new Impl(fileDescriptor, callback, 0u);

  if(fileDescriptor < 0)
  {
    DALI_ASSERT_ALWAYS(0 && "Invalid File descriptor");
    return;
  }

  if(!DALI_LIKELY(Dali::Adaptor::IsAvailable()))
  {
    DALI_LOG_DEBUG_INFO("FileDescriptorMonitorTcore constructed after adaptor invalidated. Ignore\n");
    return;
  }

  uint16_t events = 0u;
  if(eventBitmask & FD_READABLE)
  {
    events |= TIZEN_CORE_POLL_EVENT_IN;
  }
  if(eventBitmask & FD_WRITABLE)
  {
    events |= TIZEN_CORE_POLL_EVENT_OUT;
  }

  mImpl->mEventsToMonitor = events;

  int ret = tizen_core_poll_fd_create(&mImpl->mPollFd);
  if(ret != TIZEN_CORE_ERROR_NONE || !mImpl->mPollFd)
  {
    DALI_LOG_ERROR("Failed to create tizen core poll fd: %d\n", ret);
    return;
  }

  ret = tizen_core_poll_fd_set_fd(mImpl->mPollFd, fileDescriptor);
  if(ret != TIZEN_CORE_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed to set poll fd: %d\n", ret);
    return;
  }

  ret = tizen_core_poll_fd_set_events(mImpl->mPollFd, events);
  if(ret != TIZEN_CORE_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed to set poll events: %d\n", ret);
    return;
  }

  ret = tizen_core_source_create(&mImpl->mSource);
  if(ret != TIZEN_CORE_ERROR_NONE || !mImpl->mSource)
  {
    DALI_LOG_ERROR("Failed to create tizen core source: %d\n", ret);
    return;
  }

  tizen_core_source_set_prepare_cb(mImpl->mSource, &Impl::SourcePrepare, mImpl);
  tizen_core_source_set_check_cb(mImpl->mSource, &Impl::SourceCheck, mImpl);
  tizen_core_source_set_dispatch_cb(mImpl->mSource, &Impl::SourceDispatch, mImpl);
  tizen_core_source_set_finalize_cb(mImpl->mSource, &Impl::SourceFinalize, mImpl);

  ret = tizen_core_source_add_poll(mImpl->mSource, mImpl->mPollFd);
  if(ret != TIZEN_CORE_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed to add poll fd to tizen core source: %d\n", ret);
    return;
  }

  tizen_core_h core = GetTizenCore();
  if(!core)
  {
    DALI_LOG_ERROR("Failed to get tizen core for fd monitor\n");
    return;
  }

  ret = tizen_core_add_source(core, mImpl->mSource);
  if(ret != TIZEN_CORE_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed to add source to tizen core: %d\n", ret);
  }
}

FileDescriptorMonitorTcore::~FileDescriptorMonitorTcore()
{
  if(DALI_LIKELY(Dali::Adaptor::IsAvailable()) && mImpl && mImpl->mSource)
  {
    tizen_core_h core = GetTizenCore();
    if(core)
    {
      tizen_core_remove_source(core, mImpl->mSource);
    }
    tizen_core_source_destroy(mImpl->mSource);
    mImpl->mSource = nullptr;
  }

  delete mImpl;
  mImpl = nullptr;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
