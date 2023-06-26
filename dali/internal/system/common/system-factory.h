#ifndef DALI_INTERNAL_ADAPTOR_SYSTEM_COMMON_SYSTEM_FACTORY_H
#define DALI_INTERNAL_ADAPTOR_SYSTEM_COMMON_SYSTEM_FACTORY_H

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

// EXTERNAL INCLUDES
#include <memory>

// INTERNAL INCLUDES
#include <dali/internal/system/common/timer-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class CallbackManager;
class FileDescriptorMonitor;

class SystemFactory
{
public:
  SystemFactory()          = default;
  virtual ~SystemFactory() = default;

  virtual std::unique_ptr<CallbackManager>       CreateCallbackManager()                                                                   = 0;
  virtual std::unique_ptr<FileDescriptorMonitor> CreateFileDescriptorMonitor(int fileDescriptor, CallbackBase* callback, int eventBitmask) = 0;
  virtual TimerPtr                               CreateTimer(uint32_t milliSec)                                                            = 0;
};

extern std::unique_ptr<SystemFactory> GetSystemFactory();

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_SYSTEM_COMMON_SYSTEM_FACTORY_H
