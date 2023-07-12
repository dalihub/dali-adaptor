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
#include <dali/internal/adaptor/ubuntu/framework-factory-ubuntu.h>

// INTERNAL HEADERS
#include <dali/internal/adaptor/ubuntu/framework-ubuntu.h>
#include <dali/internal/window-system/common/display-utils.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
FrameworkFactory* gFrameworkFactory = nullptr;

FrameworkFactoryUbuntu::FrameworkFactoryUbuntu()
{
  gFrameworkFactory = this;
}

FrameworkFactoryUbuntu::~FrameworkFactoryUbuntu()
{
  gFrameworkFactory = nullptr;
}

std::unique_ptr<Framework> FrameworkFactoryUbuntu::CreateFramework(FrameworkBackend backend, Framework::Observer& observer, Framework::TaskObserver& taskObserver, int* argc, char*** argv, Framework::Type type, bool useUiThread)
{
  return Utils::MakeUnique<FrameworkUbuntu>(observer, taskObserver, argc, argv, type, useUiThread);
}

FrameworkFactory* CreateFrameworkFactory()
{
  return (new FrameworkFactoryUbuntu());
}

FrameworkFactory* GetFrameworkFactory()
{
  return gFrameworkFactory;
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
