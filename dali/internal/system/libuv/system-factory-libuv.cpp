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
#include <dali/internal/system/libuv/system-factory-libuv.h>

// INTERNAL HEADERS
#include <dali/internal/system/libuv/callback-manager-libuv.h>
#include <dali/internal/system/libuv/file-descriptor-monitor-libuv.h>
#include <dali/internal/system/libuv/timer-impl-libuv.h>
#include <dali/internal/window-system/common/display-utils.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
std::unique_ptr<CallbackManager> SystemFactoryLibuv::CreateCallbackManager()
{
  return Utils::MakeUnique<UvCallbackManager>();
}

std::unique_ptr<FileDescriptorMonitor> SystemFactoryLibuv::CreateFileDescriptorMonitor(int fileDescriptor, CallbackBase* callback, int eventBitmask)
{
  return Utils::MakeUnique<FileDescriptorMonitorLibuv>(fileDescriptor, callback, eventBitmask);
}

TimerPtr SystemFactoryLibuv::CreateTimer(uint32_t milliSec)
{
  return TimerLibuv::New(milliSec);
}

std::unique_ptr<SystemFactory> GetSystemFactory()
{
  return Utils::MakeUnique<SystemFactoryLibuv>();
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
