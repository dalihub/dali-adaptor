#ifndef DALI_INTERNAL_ADAPTOR_SYSTEM_WINDOWS_FILE_DESCRIPTOR_MONITOR_H
#define DALI_INTERNAL_ADAPTOR_SYSTEM_WINDOWS_FILE_DESCRIPTOR_MONITOR_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/signals/callback.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/file-descriptor-monitor.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * @brief FileDescriptorMonitorWin class provides an FileDescriptorMonitor Windows implementation.
 */
class FileDescriptorMonitorWin : public FileDescriptorMonitor
{
public:
  /**
   * @copydoc Dali::Internal::Adaptor::FileDescriptorMonitor()
   */
  FileDescriptorMonitorWin(int fileDescriptor, CallbackBase* callback, int eventBitmask);

  /**
   * Destructor
   */
  ~FileDescriptorMonitorWin();

private:
  // Undefined
  FileDescriptorMonitorWin(const FileDescriptorMonitorWin& fileDescriptorMonitor) = delete;

  // Undefined
  FileDescriptorMonitorWin& operator=(const FileDescriptorMonitorWin& fileDescriptorMonitor) = delete;

private:
  struct Impl;
  Impl* mImpl;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_SYSTEM_WINDOWS_FILE_DESCRIPTOR_MONITOR_H
