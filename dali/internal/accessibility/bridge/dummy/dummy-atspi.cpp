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

#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/devel-api/atspi-interfaces/component.h>
#include <dali/internal/accessibility/bridge/dummy/dummy-atspi.h>

namespace Dali
{
Accessibility::Accessible::Accessible()
{
}

Accessibility::Accessible::~Accessible() noexcept
{
}

std::vector<Accessibility::Accessible*> Accessibility::Accessible::GetChildren()
{
  return {};
}

Accessibility::Address Accessibility::Accessible::GetAddress() const
{
  return {};
}

std::shared_ptr<Accessibility::Bridge::Data> Accessibility::Accessible::GetBridgeData() const
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

std::string MakeBusNameForWidget(std::string_view widgetInstanceId)
{
  return std::string{widgetInstanceId};
}

void Accessibility::Accessible::EmitStateChanged(Accessibility::State state, int newValue, int reserved)
{
}

void Accessibility::Accessible::Emit(Accessibility::ObjectPropertyChangeEvent event)
{
}

void Accessibility::Accessible::EmitHighlighted(bool set)
{
}

void Accessibility::Accessible::EmitBoundsChanged(Rect<> rect)
{
}

void Accessibility::Accessible::EmitShowing(bool showing)
{
}

void Accessibility::Accessible::EmitFocused(bool set)
{
}

void Accessibility::Accessible::EmitVisible(bool visible)
{
}

void Accessibility::Accessible::EmitTextInserted(unsigned int position, unsigned int length, const std::string& content)
{
}

void Accessibility::Accessible::EmitTextDeleted(unsigned int position, unsigned int length, const std::string& content)
{
}

void Accessibility::Accessible::EmitTextCursorMoved(unsigned int cursorPosition)
{
}

void Accessibility::Accessible::EmitActiveDescendantChanged(Accessibility::Accessible* child)
{
}

void Accessibility::Accessible::EmitMovedOutOfScreen(Accessibility::ScreenRelativeMoveType type)
{
}

void Accessibility::Accessible::EmitSocketAvailable()
{
}

void Accessibility::Accessible::EmitScrollStarted()
{
}

void Accessibility::Accessible::EmitScrollFinished()
{
}

void Accessibility::Accessible::NotifyAccessibilityStateChange(Accessibility::States states, bool isRecursive)
{
}

} // namespace Dali
