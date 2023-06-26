/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/glib/file-descriptor-monitor-glib.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <glib-unix.h>
#include <glib.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
extern GMainContext* GetMainLoopContext();

/**
 * Using Impl to hide away framework specific members
 */
struct FileDescriptorMonitorGlib::Impl
{
public:
  // Constructor
  Impl(int fileDescriptor, CallbackBase* callback, GIOCondition eventsToMonitor)
  : mFileDescriptor(fileDescriptor),
    mEventsToMonitor(eventsToMonitor),
    mCallback(callback),
    mPollSource(nullptr)
  {
    mPollSource = g_unix_fd_source_new(fileDescriptor, eventsToMonitor);
    g_source_set_callback(mPollSource, reinterpret_cast<GSourceFunc>(PollCallback), this, nullptr);
    g_source_attach(mPollSource, GetMainLoopContext());
  }

  ~Impl()
  {
    g_source_destroy(mPollSource);
    g_source_unref(mPollSource);
    mPollSource = nullptr;

    delete mCallback;
  }

  static gboolean PollCallback(int fileDescriptor, GIOCondition events, gpointer userData)
  {
    if(userData)
    {
      FileDescriptorMonitorGlib::Impl* impl = static_cast<FileDescriptorMonitorGlib::Impl*>(userData);

      // filter the events that have occured based on what we are monitoring
      int eventType = FileDescriptorMonitor::FD_NO_EVENT;

      if((impl->mEventsToMonitor & G_IO_IN) && (events & G_IO_IN))
      {
        eventType = FileDescriptorMonitor::FD_READABLE;
      }
      if((impl->mEventsToMonitor & G_IO_OUT) && (events & G_IO_OUT))
      {
        eventType |= FileDescriptorMonitor::FD_WRITABLE;
      }

      // if there is an event, execute the callback
      if(eventType != FileDescriptorMonitor::FD_NO_EVENT)
      {
        CallbackBase::Execute(*impl->mCallback, static_cast<FileDescriptorMonitor::EventType>(eventType));
      }
    }
    return G_SOURCE_CONTINUE;
  }
  // Data
  int           mFileDescriptor;
  GIOCondition  mEventsToMonitor;
  CallbackBase* mCallback;
  GSource*      mPollSource;
};

FileDescriptorMonitorGlib::FileDescriptorMonitorGlib(int fileDescriptor, CallbackBase* callback, int eventBitmask)
: FileDescriptorMonitor(fileDescriptor, callback, eventBitmask)
{
  if(fileDescriptor < 1)
  {
    DALI_ASSERT_ALWAYS(0 && "Invalid File descriptor");
    return;
  }
  GIOCondition events = static_cast<GIOCondition>(0);
  if(eventBitmask & FD_READABLE)
  {
    events = G_IO_IN;
  }
  if(eventBitmask & FD_WRITABLE)
  {
    events = static_cast<GIOCondition>(static_cast<int>(events) | G_IO_OUT);
  }

  DALI_ASSERT_ALWAYS(events && "Invalid FileDescriptorMonitor event type ");

  // waiting for a write event on a file descriptor
  mImpl = new Impl(fileDescriptor, callback, static_cast<GIOCondition>(events));
}

FileDescriptorMonitorGlib::~FileDescriptorMonitorGlib()
{
  delete mImpl;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
