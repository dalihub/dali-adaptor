#ifndef DALI_INTERNAL_FILE_DESCRIPTOR_MONITOR_H
#define DALI_INTERNAL_FILE_DESCRIPTOR_MONITOR_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <dali/public-api/signals/callback.h>

namespace Dali
{

namespace Integration
{
class Core;
}

namespace Internal
{

namespace Adaptor
{

/**
 * @brief Monitors the given file descriptor and whenever anything is written to it, the provided
 * callback is called
 */
class FileDescriptorMonitor
{
public:

  /**
   * @brief Bitmask of file descriptor event types
   */
  enum EventType
  {
    FD_NO_EVENT = 0x0,
    FD_READABLE = 0x1, // For example when monitoring a socket, data is available to read from the socket receive buffer
    FD_WRITABLE = 0x2, // For example when monitoring a socket space is available in socket send buffer
    FD_ERROR    = 0x4,
  };

  /**
   * @brief Constructor.
   *
   * The callback will be passed a EventType bitmask to signal what type of events occured on the file
   * descriptor.
   * Example:
   *
   * MyClass::MyClass()
   * {
   *    mFileDescriptorMonitor = new FileDescriptorMonitor( myFd, MakeCallback( this, &MyClass::FdCallback ), FileDescriptorMonitor::FD_READABLE );
   * }
   *
   * void MyClass::FdCallback( EventType event )
   * {
   *    if( event & FileDescriptorMonitor::FD_ERROR)
   *    {
   *      LOG_ERROR("...)
   *    }
   *    if( event & FileDescriptorMonitor::FD_READABLE )
   *    {
   *      // read from FD
   *    }
   *
   * }
   *
   * @param[in] fileDescriptor  The file descriptor to monitor
   * @param[in] callback Called when anything is written to the file descriptor
   * @param[in] eventBitmask Bitmask of what to monitor on the file descriptor ( readable / writable ).
   * @note The ownership of callback is taken by this class.
   * @note Under Linux it is possible the file descriptor monitor will signal a fd is
   * readable or writable even when it isn’t. The developer should check for handle EAGAIN or equivalent
   * when reading from or write to the fd.
   */
  FileDescriptorMonitor( int fileDescriptor, CallbackBase* callback, int eventBitmask );

  /**
   * Destructor
   */
  ~FileDescriptorMonitor();

private:

  // Undefined
  FileDescriptorMonitor( const FileDescriptorMonitor& fileDescriptorMonitor );

  // Undefined
  FileDescriptorMonitor& operator=( const FileDescriptorMonitor& fileDescriptorMonitor );

private:
  struct Impl;
  Impl* mImpl;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_FILE_DESCRIPTOR_MONITOR_H
