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
 */

// CLASS HEADER
#include <dali/internal/system/macos/system-factory-mac.h>

// INTERNAL HEADERS
#include <dali/internal/system/macos/callback-manager-mac.h>
#include <dali/internal/system/macos/file-descriptor-monitor-macos.h>
#include <dali/internal/system/macos/timer-impl-mac.h>
#include <dali/internal/window-system/common/display-utils.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
std::unique_ptr<CallbackManager> SystemFactoryMac::CreateCallbackManager()
{
  return Utils::MakeUnique<CocoaCallbackManager>();
}

std::unique_ptr<FileDescriptorMonitor> SystemFactoryMac::CreateFileDescriptorMonitor(int fileDescriptor, CallbackBase* callback, int eventBitmask)
{
  return Utils::MakeUnique<FileDescriptorMonitorMac>(fileDescriptor, callback, eventBitmask);
}

TimerPtr SystemFactoryMac::CreateTimer(uint32_t milliSec)
{
  return TimerMac::New(milliSec);
}

std::unique_ptr<SystemFactory> GetSystemFactory()
{
  return Utils::MakeUnique<SystemFactoryMac>();
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
