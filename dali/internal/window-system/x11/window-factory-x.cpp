/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/x11/window-factory-x.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/common/display-utils.h>
#include <dali/internal/window-system/x11/window-base-x.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
std::unique_ptr<WindowBase> WindowFactoryX::CreateWindowBase(Dali::PositionSize positionSize, Any surface, bool isTransparent)
{
  return Utils::MakeUnique<WindowBaseX>(positionSize, surface, isTransparent);
}

// this should be created from Window impl
std::unique_ptr<WindowFactory> GetWindowFactory()
{
  // returns Window factory
  return Utils::MakeUnique<WindowFactoryX>();
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
