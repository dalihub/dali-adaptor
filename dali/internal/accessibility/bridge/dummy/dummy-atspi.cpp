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

#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/devel-api/atspi-interfaces/component.h>
#include <dali/internal/accessibility/bridge/dummy/dummy-atspi.h>

namespace Dali
{
std::vector<Accessibility::Accessible*> Accessibility::Accessible::GetChildren()
{
  return {};
}

std::string Accessibility::Accessible::DumpTree(Accessibility::Accessible::DumpDetailLevel detailLevel)
{
  return {};
}

bool Accessibility::Accessible::IsHidden() const
{
  return false;
}

bool Accessibility::Accessible::IsProxy() const
{
  return false;
}

bool Accessibility::Component::IsScrollable() const
{
  return false;
}

void Accessibility::Accessible::SetListenPostRender(bool enabled)
{
}

bool Accessibility::Component::IsAccessibleContainingPoint(Point point, CoordinateType type) const
{
  return false;
}

Accessibility::Accessible* Accessibility::Component::GetAccessibleAtPoint(Accessibility::Point p, Accessibility::CoordinateType ctype)
{
  return nullptr;
}

std::shared_ptr<Accessibility::Bridge> Accessibility::Bridge::GetCurrentBridge()
{
  return Accessibility::DummyBridge::GetInstance();
}

void Accessibility::Bridge::DisableAutoInit()
{
}

void Accessibility::Bridge::EnableAutoInit()
{
}

std::string MakeBusNameForWidget(std::string_view widgetInstanceId, int widgetProcessId)
{
  return std::string{widgetInstanceId};
}

} // namespace Dali
