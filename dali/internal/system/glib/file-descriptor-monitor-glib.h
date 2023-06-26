#ifndef DALI_INTERNAL_ADAPTOR_SYSTEM_GLIB_FILE_DESCRIPTOR_MONITOR_H
#define DALI_INTERNAL_ADAPTOR_SYSTEM_GLIB_FILE_DESCRIPTOR_MONITOR_H

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

// INTERNAL INCLUDES
#include <dali/internal/system/common/file-descriptor-monitor.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * @brief FileDescriptorMonitorGlib class provides an FileDescriptorMonitor Glib implementation.
 */
class FileDescriptorMonitorGlib : public FileDescriptorMonitor
{
public:
  /**
   * @copydoc Dali::Internal::Adaptor::FileDescriptorMonitor()
   */
  FileDescriptorMonitorGlib(int fileDescriptor, CallbackBase* callback, int eventBitmask);

  /**
   * Destructor
   */
  ~FileDescriptorMonitorGlib();

private:
  // Undefined
  FileDescriptorMonitorGlib(const FileDescriptorMonitorGlib& fileDescriptorMonitor) = delete;

  // Undefined
  FileDescriptorMonitorGlib& operator=(const FileDescriptorMonitorGlib& fileDescriptorMonitor) = delete;

private:
  struct Impl;
  Impl* mImpl;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_SYSTEM_GLIB_FILE_DESCRIPTOR_MONITOR_H
