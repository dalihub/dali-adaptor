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

// CLASS HEADER
#include <dali/internal/window-system/tizen-wayland/window-base-factory-ecore-wl.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/tizen-wayland/window-base-ecore-wl.h>
#include <dali/internal/window-system/common/display-utils.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

std::unique_ptr< Dali::Internal::Adaptor::WindowBase > WindowBaseFactoryEcoreWl::CreateWindowBase( Window* window, WindowRenderSurface* windowRenderSurface )
{
  return Utils::MakeUnique< WindowBaseEcoreWl >( window, windowRenderSurface );
}

// this should be created from somewhere
std::unique_ptr< WindowBaseFactory > GetWindowBaseFactory()
{
  // returns WindowBase factory
  return Utils::MakeUnique< WindowBaseFactoryEcoreWl >();
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
