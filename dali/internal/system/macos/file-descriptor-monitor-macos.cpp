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
#include <dali/internal/system/macos/file-descriptor-monitor-macos.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace Dali::Internal::Adaptor
{
FileDescriptorMonitorMac::FileDescriptorMonitorMac(int fileDescriptor, CallbackBase* callback, int eventBitmask)
: FileDescriptorMonitor(fileDescriptor, callback, eventBitmask)
{
  DALI_LOG_WARNING("Implementation missing for macOS");
}

FileDescriptorMonitorMac::~FileDescriptorMonitorMac()
{
}

} // namespace Dali::Internal::Adaptor
