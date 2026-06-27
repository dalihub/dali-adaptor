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

#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/integration-api/adaptor-framework/accessibility/accessibility-bridge.h>
#include <dali/integration-api/adaptor-framework/accessibility/accessibility-integ.h>
#include <dali/internal/accessibility/bridge/collection-impl.h>
#include <dali/internal/accessibility/bridge/dummy/dummy-atspi.h>

namespace Dali::Accessibility
{

CollectionImpl::CollectionImpl(Dali::WeakPtr<Accessible> accessible)
{
}

std::vector<Accessible*> CollectionImpl::GetMatches(MatchRule rule, uint32_t sortBy, size_t maxCount)
{
  return {};
}

std::vector<Accessible*> CollectionImpl::GetMatchesInMatches(MatchRule firstRule, MatchRule secondRule, uint32_t sortBy, int32_t firstCount, int32_t secondCount)
{
  return {};
}

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

void Accessibility::Accessible::SetListenPostRender(bool enabled)
{
}

bool Accessibility::Accessible::IsAccessibleContainingPoint(Dali::Devel::Accessibility::Point point, Dali::Devel::Accessibility::CoordinateType type) const
{
  return false;
}

Accessibility::Accessible* Accessibility::Accessible::GetAccessibleAtPoint(Dali::Devel::Accessibility::Point p, Dali::Devel::Accessibility::CoordinateType ctype)
{
  return nullptr;
}

bool Accessibility::Component::IsAccessibleContainingPoint(Dali::Devel::Accessibility::Point point, Dali::Devel::Accessibility::CoordinateType type) const
{
  return false;
}

Accessibility::Accessible* Accessibility::Component::GetAccessibleAtPoint(Dali::Devel::Accessibility::Point point, Dali::Devel::Accessibility::CoordinateType type)
{
  return nullptr;
}

} //namespace Dali::Accessibility

Dali::SharedPtr<Dali::Integration::Accessibility::Bridge> Dali::Integration::Accessibility::Bridge::GetCurrentBridge()
{
  return Dali::Accessibility::DummyBridge::GetInstance();
}

void Dali::Integration::Accessibility::Bridge::DisableAutoInit()
{
}

void Dali::Integration::Accessibility::Bridge::EnableAutoInit()
{
}

std::string Dali::Integration::Accessibility::Bridge::MakeBusNameForWidget(std::string_view widgetInstanceId, int widgetProcessId)
{
  return std::string{widgetInstanceId};
}
