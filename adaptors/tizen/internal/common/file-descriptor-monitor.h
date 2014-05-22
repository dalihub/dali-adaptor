#ifndef __DALI_INTERNAL_FILE_DESCRIPTOR_MONITOR_H__
#define __DALI_INTERNAL_FILE_DESCRIPTOR_MONITOR_H__

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

// EXTERNAL INCLUDES
#include <boost/function.hpp>

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
 * Monitors the given file descriptor and whenever anything is written to it, it calls
 * the given boost function.
 */
class FileDescriptorMonitor
{
public:

  /**
   * Constructor
   * @param[in]  fileDescriptor  The file descriptor to monitor
   * @param[in]  functor         The function to call when anything is written to the file descriptor
   */
  FileDescriptorMonitor(int fileDescriptor, boost::function<void()> functor);

  /**
   * Destructor
   */
  ~FileDescriptorMonitor();

private:
  struct Impl;
  Impl* mImpl;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_FILE_DESCRIPTOR_MONITOR_H__
