/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/internal/addons/common/addon-manager-factory.h>
#include <dali/internal/addons/linux/addon-manager-impl-linux.h>

// EXTERNAL INCLUDES
#include <dlfcn.h>

namespace Dali
{
namespace Internal
{
Integration::AddOnManager* AddOnManagerFactory::CreateAddOnManager()
{
  return new Dali::Adaptor::AddOnManager(new AddOnManagerLinux(RTLD_DEEPBIND | RTLD_LAZY));
}

} // namespace Internal
} // namespace Dali
