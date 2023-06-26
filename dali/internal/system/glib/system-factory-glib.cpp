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
#include <dali/internal/system/glib/system-factory-glib.h>

// INTERNAL HEADERS
#include <dali/internal/system/glib/callback-manager-glib.h>
#include <dali/internal/system/glib/file-descriptor-monitor-glib.h>
#include <dali/internal/system/glib/timer-impl-glib.h>
#include <dali/internal/window-system/common/display-utils.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
std::unique_ptr<CallbackManager> SystemFactoryGlib::CreateCallbackManager()
{
  return Utils::MakeUnique<GlibCallbackManager>();
}

std::unique_ptr<FileDescriptorMonitor> SystemFactoryGlib::CreateFileDescriptorMonitor(int fileDescriptor, CallbackBase* callback, int eventBitmask)
{
  return Utils::MakeUnique<FileDescriptorMonitorGlib>(fileDescriptor, callback, eventBitmask);
}

TimerPtr SystemFactoryGlib::CreateTimer(uint32_t milliSec)
{
  return TimerGlib::New(milliSec);
}

std::unique_ptr<SystemFactory> GetSystemFactory()
{
  return Utils::MakeUnique<SystemFactoryGlib>();
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
