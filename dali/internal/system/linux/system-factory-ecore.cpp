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
#include <dali/internal/system/linux/system-factory-ecore.h>

// INTERNAL HEADERS
#include <dali/internal/adaptor/common/framework-factory.h>
#include <dali/internal/system/glib/callback-manager-glib.h>
#include <dali/internal/system/glib/file-descriptor-monitor-glib.h>
#include <dali/internal/system/glib/timer-impl-glib.h>
#include <dali/internal/system/linux/callback-manager-ecore.h>
#include <dali/internal/system/linux/file-descriptor-monitor-ecore.h>
#include <dali/internal/system/linux/timer-impl-ecore.h>
#include <dali/internal/window-system/common/display-utils.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
std::unique_ptr<CallbackManager> SystemFactoryEcore::CreateCallbackManager()
{
  auto backend = Dali::Internal::Adaptor::GetFrameworkFactory()->GetFrameworkBackend();
  if(backend == FrameworkBackend::GLIB)
  {
    return Utils::MakeUnique<GlibCallbackManager>();
  }
  else
  {
    return Utils::MakeUnique<EcoreCallbackManager>();
  }
}

std::unique_ptr<FileDescriptorMonitor> SystemFactoryEcore::CreateFileDescriptorMonitor(int fileDescriptor, CallbackBase* callback, int eventBitmask)
{
  auto backend = Dali::Internal::Adaptor::GetFrameworkFactory()->GetFrameworkBackend();
  if(backend == FrameworkBackend::GLIB)
  {
    return Utils::MakeUnique<FileDescriptorMonitorGlib>(fileDescriptor, callback, eventBitmask);
  }
  else
  {
    return Utils::MakeUnique<FileDescriptorMonitorEcore>(fileDescriptor, callback, eventBitmask);
  }
}

TimerPtr SystemFactoryEcore::CreateTimer(uint32_t milliSec)
{
  auto backend = Dali::Internal::Adaptor::GetFrameworkFactory()->GetFrameworkBackend();
  if(backend == FrameworkBackend::GLIB)
  {
    return TimerGlib::New(milliSec);
  }
  else
  {
    return TimerEcore::New(milliSec);
  }
}

std::unique_ptr<SystemFactory> GetSystemFactory()
{
  return Utils::MakeUnique<SystemFactoryEcore>();
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
