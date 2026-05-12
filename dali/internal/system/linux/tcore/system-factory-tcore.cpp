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
 */

// CLASS HEADER
#include <dali/internal/system/linux/tcore/system-factory-tcore.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/framework-factory.h>
#include <dali/internal/system/glib/callback-manager-glib.h>
#include <dali/internal/system/glib/file-descriptor-monitor-glib.h>
#include <dali/internal/system/glib/timer-impl-glib.h>
#include <dali/internal/system/linux/tcore/callback-manager-tcore.h>
#include <dali/internal/system/linux/tcore/event-loop-tcore.h>
#include <dali/internal/system/linux/tcore/file-descriptor-monitor-tcore.h>
#include <dali/internal/system/linux/tcore/timer-impl-tcore.h>
#include <dali/internal/window-system/common/display-utils.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
std::unique_ptr<CallbackManager> SystemFactoryTcore::CreateCallbackManager()
{
  auto frameworkFactory = Dali::Internal::Adaptor::GetFrameworkFactory();
  if(frameworkFactory != nullptr && frameworkFactory->GetFrameworkBackend() == FrameworkBackend::GLIB)
  {
    return Utils::MakeUnique<GlibCallbackManager>();
  }
  return Utils::MakeUnique<TcoreCallbackManager>();
}

std::unique_ptr<FileDescriptorMonitor> SystemFactoryTcore::CreateFileDescriptorMonitor(int fileDescriptor, CallbackBase* callback, int eventBitmask)
{
  auto frameworkFactory = Dali::Internal::Adaptor::GetFrameworkFactory();
  if(frameworkFactory != nullptr && frameworkFactory->GetFrameworkBackend() == FrameworkBackend::GLIB)
  {
    return Utils::MakeUnique<FileDescriptorMonitorGlib>(fileDescriptor, callback, eventBitmask);
  }
  return Utils::MakeUnique<FileDescriptorMonitorTcore>(fileDescriptor, callback, eventBitmask);
}

TimerPtr SystemFactoryTcore::CreateTimer(uint32_t milliSec)
{
  auto frameworkFactory = Dali::Internal::Adaptor::GetFrameworkFactory();
  if(frameworkFactory != nullptr && frameworkFactory->GetFrameworkBackend() == FrameworkBackend::GLIB)
  {
    return TimerGlib::New(milliSec);
  }
  return TimerTcore::New(milliSec);
}

std::unique_ptr<EventLoop> SystemFactoryTcore::CreateEventLoop()
{
  return Utils::MakeUnique<EventLoopTcore>();
}

std::unique_ptr<SystemFactory> GetSystemFactory()
{
  return Utils::MakeUnique<SystemFactoryTcore>();
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
