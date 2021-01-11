/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/macos/display-connection-factory-mac.h>
#include <dali/internal/window-system/macos/display-connection-impl-mac.h>

namespace Dali::Internal::Adaptor
{

std::unique_ptr<DisplayConnection>
DisplayConnectionFactoryCocoa::CreateDisplayConnection()
{
  return Utils::MakeUnique<DisplayConnectionCocoa>();
}

std::unique_ptr<DisplayConnectionFactory>
GetDisplayConnectionFactory()
{
  return Utils::MakeUnique<DisplayConnectionFactoryCocoa>();
}

}
