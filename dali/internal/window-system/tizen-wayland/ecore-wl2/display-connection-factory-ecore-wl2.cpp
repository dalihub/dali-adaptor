/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <dali/internal/window-system/tizen-wayland/ecore-wl2/display-connection-factory-ecore-wl2.h>
#include <dali/internal/window-system/tizen-wayland/ecore-wl2/display-connection-impl-ecore-wl2.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{



std::unique_ptr<Dali::Internal::Adaptor::DisplayConnection> DisplayConnectionFactoryEcoreWl2::CreateDisplayConnection()
{
  return Utils::MakeUnique<DisplayConnectionEcoreWl2>();
}

// this should be created from somewhere
std::unique_ptr<DisplayConnectionFactory> GetDisplayConnectionFactory()
{
  // returns X display factory
  return Utils::MakeUnique<DisplayConnectionFactoryEcoreWl2>();
}

void DisplayConnectionFactoryGetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  DisplayConnectionEcoreWl2::GetDpi( dpiHorizontal, dpiVertical );
}

void DisplayConnectionFactoryGetDpi(Any nativeWindow, unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  DisplayConnectionEcoreWl2::GetDpi( nativeWindow, dpiHorizontal, dpiVertical );
}

}
}
}
