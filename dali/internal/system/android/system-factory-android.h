#ifndef DALI_INTERNAL_ADAPTOR_SYSTEM_ANDROID_SYSTEM_FACTORY_H
#define DALI_INTERNAL_ADAPTOR_SYSTEM_ANDROID_SYSTEM_FACTORY_H

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
#include <dali/internal/system/common/system-factory.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class SystemFactoryAndroid : public SystemFactory
{
public:
  std::unique_ptr<CallbackManager>       CreateCallbackManager() override;
  std::unique_ptr<FileDescriptorMonitor> CreateFileDescriptorMonitor(int fileDescriptor, CallbackBase* callback, int eventBitmask) override;
  TimerPtr                               CreateTimer(uint32_t milliSec) override;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_SYSTEM_GLIB_SYSTEM_FACTORY_H
