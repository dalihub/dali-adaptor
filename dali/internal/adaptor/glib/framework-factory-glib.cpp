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
#include <dali/internal/adaptor/glib/framework-factory-glib.h>

// INTERNAL HEADERS
#include <dali/internal/adaptor/glib/framework-glib.h>
#include <dali/internal/window-system/common/display-utils.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
FrameworkFactory* gFrameworkFactory = nullptr;

FrameworkFactoryGlib::FrameworkFactoryGlib()
{
  gFrameworkFactory = this;
}

FrameworkFactoryGlib::~FrameworkFactoryGlib()
{
  gFrameworkFactory = nullptr;
}

std::unique_ptr<Framework> FrameworkFactoryGlib::CreateFramework(FrameworkBackend backend, Framework::Observer& observer, Framework::TaskObserver& taskObserver, int* argc, char*** argv, Framework::Type type, bool useUiThread)
{
  return Utils::MakeUnique<FrameworkGlib>(observer, taskObserver, argc, argv, type, useUiThread);
}

FrameworkFactory* CreateFrameworkFactory()
{
  return (new FrameworkFactoryGlib());
}

FrameworkFactory* GetFrameworkFactory()
{
  return gFrameworkFactory;
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
