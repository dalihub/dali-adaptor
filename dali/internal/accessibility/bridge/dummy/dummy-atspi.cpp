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
#include <dali/devel-api/atspi-interfaces/action.h>
#include <dali/devel-api/atspi-interfaces/value.h>
#include <dali/devel-api/atspi-interfaces/hyperlink.h>
#include <dali/devel-api/atspi-interfaces/editable-text.h>
#include <dali/devel-api/atspi-interfaces/hypertext.h>
#include <dali/devel-api/atspi-interfaces/text.h>
#include <dali/devel-api/atspi-interfaces/selection.h>
#include <dali/devel-api/adaptor-framework/proxy-accessible.h>
#include <dali/integration-api/adaptor-framework/accessibility/accessibility-bridge.h>
#include <dali/integration-api/adaptor-framework/accessibility/accessibility-integ.h>
#include <dali/internal/accessibility/bridge/collection-impl.h>
#include <dali/internal/accessibility/bridge/dummy/dummy-atspi.h>

namespace Dali::Accessibility
{

// ============================================================================
// ATSPI interface class destructors (DALI_ADAPTOR_API forces non-inline)
// ============================================================================
Action::~Action() {}
Value::~Value() {}
Hyperlink::~Hyperlink() {}
EditableText::~EditableText() {}
Hypertext::~Hypertext() {}
Text::~Text() {}
Selection::~Selection() {}

// ============================================================================
// CollectionImpl
// ============================================================================

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

// ============================================================================
// Accessible class - static and instance methods
// ============================================================================

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

// ============================================================================
// ProxyAccessible class - constructor and setters (moved out-of-line for DLL export)
// ============================================================================

ProxyAccessible::ProxyAccessible()
: mAddress{},
  mParent{nullptr}
{
}

void ProxyAccessible::SetAddress(Dali::Devel::Accessibility::Address address)
{
  mAddress = std::move(address);
}

void ProxyAccessible::SetParent(Accessible* parent)
{
  mParent = parent;
}

} // namespace Dali::Accessibility

// ============================================================================
// Bridge stubs
// ============================================================================

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
