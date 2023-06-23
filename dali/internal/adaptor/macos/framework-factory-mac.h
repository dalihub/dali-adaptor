#ifndef DALI_INTERNAL_ADAPTOR_MAC_FRAMEWORK_FACTORY_H
#define DALI_INTERNAL_ADAPTOR_MAC_FRAMEWORK_FACTORY_H

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

#include <dali/internal/adaptor/common/framework-factory.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class FrameworkFactoryMac : public FrameworkFactory
{
public:
  std::unique_ptr<Framework> CreateFramework(Framework::Observer& observer, Framework::TaskObserver& taskObserver, int* argc, char*** argv, Framework::Type type, bool useUiThread) override;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_MAC_FRAMEWORK_FACTORY_H
