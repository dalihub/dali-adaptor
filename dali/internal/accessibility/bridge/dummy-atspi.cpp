/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

#include <dali/devel-api/adaptor-framework/accessibility-impl.h>
#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/internal/accessibility/bridge/dummy-atspi.h>

namespace Dali
{

Accessibility::Accessible::Accessible()
{
}

Accessibility::Accessible::~Accessible()
{
}

std::vector<Accessibility::Accessible*> Accessibility::Accessible::GetChildren()
{
  return {};
}

Accessibility::Accessible* Accessibility::Accessible::GetDefaultLabel()
{
  return nullptr;
}

Accessibility::Address Accessibility::Accessible::GetAddress()
{
  return {};
}

std::shared_ptr<Accessibility::Bridge::Data> Accessibility::Accessible::GetBridgeData()
{
  return {};
}

bool Accessibility::Accessible::IsProxy()
{
  return false;
}

bool Accessibility::Component::IsScrollable()
{
  return false;
}

bool Accessibility::Component::Contains(Point p, CoordType ctype)
{
  return false;
}

Accessibility::Accessible* Accessibility::Component::GetAccessibleAtPoint(Accessibility::Point p, Accessibility::CoordType ctype)
{
  return nullptr;
}

Accessibility::Bridge* Accessibility::Bridge::GetCurrentBridge()
{
  return Accessibility::DummyBridge::GetInstance();
}

void Accessibility::Bridge::DisableAutoInit()
{
}

void Accessibility::Bridge::EnableAutoInit(Accessible*, const std::string&)
{
}

void Accessibility::Accessible::EmitStateChanged(Accessibility::State state, int newValue1, int newValue2)
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

void Accessibility::Accessible::EmitTextCaretMoved(unsigned int cursorPosition)
{
}

void Accessibility::Accessible::EmitActiveDescendantChanged(Accessibility::Accessible* obj, Accessibility::Accessible* child)
{
}

void Accessibility::Accessible::FindWordSeparationsUtf8(const utf8_t* s, size_t length, const char* language, char* breaks)
{
}

void Accessibility::Accessible::FindLineSeparationsUtf8(const utf8_t* s, size_t length, const char* language, char* breaks)
{
}

void Accessibility::Accessible::NotifyAccessibilityStateChange(Accessibility::States states, bool doRecursive)
{
}

} // namespace Dali
