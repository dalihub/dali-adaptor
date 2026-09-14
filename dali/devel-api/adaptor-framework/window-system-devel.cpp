/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/window-system-devel.h>
#include <dali/internal/window-system/common/window-system-impl.h>

namespace DALI_NAMESPACE
{
namespace DevelWindowSystem
{
void GetDpi(uint32_t& horizontalDpi, uint32_t& verticalDpi)
{
  Dali::Internal::Adaptor::WindowSystem::GetDpi(horizontalDpi, verticalDpi);
}

void SetGeometryHittestEnabled(bool enabled)
{
  Dali::Internal::Adaptor::WindowSystem::SetGeometryHittestEnabled(enabled);
}

bool IsGeometryHittestEnabled()
{
  return Dali::Internal::Adaptor::WindowSystem::IsGeometryHittestEnabled();
}

std::vector<Dali::ScreenInformation> GetAvailableScreens()
{
  return Internal::Adaptor::WindowSystem::GetAvailableScreens();
}

} // namespace DevelWindowSystem

} //namespace DALI_NAMESPACE
