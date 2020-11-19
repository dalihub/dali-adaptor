/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/devel-api/adaptor-framework/accessibility-impl.h>

namespace Dali {
namespace {

struct DummyBridge : Dali::Accessibility::Bridge
{
  const std::string& GetBusName() const override
  {
    static const std::string name = "";
    return name;
  }

  void AddTopLevelWindow(Accessibility::Accessible *) override
  {
  }

  void RemoveTopLevelWindow(Accessibility::Accessible *) override
  {
  }

  void AddPopup(Accessibility::Accessible *) override
  {
  }

  void RemovePopup(Accessibility::Accessible *) override
  {
  }

  void SetApplicationName(std::string) override
  {
  }

  Accessibility::Accessible *GetApplication() const override
  {
    return nullptr;
  }

  Accessibility::Accessible *FindByPath(const std::string& s) const override
  {
    return nullptr;
  }

  void ApplicationShown() override
  {
  }

  void ApplicationHidden() override
  {
  }

  void Initialize() override
  {
  }

  void Terminate() override
  {
  }

  ForceUpResult ForceUp() override
  {
    return ForceUpResult::JUST_STARTED;
  }

  void ForceDown() override
  {
  }

  void EmitCaretMoved(Accessibility::Accessible *obj, unsigned int cursorPosition) override
  {
  }

  void EmitActiveDescendantChanged(Accessibility::Accessible *obj, Accessibility::Accessible *child) override
  {
  }

  void EmitTextChanged(Accessibility::Accessible *obj, Accessibility::TextChangedState state, unsigned int position, unsigned int length, const std::string &content) override
  {
  }

  void EmitStateChanged(Accessibility::Accessible *obj, Accessibility::State state, int val1, int val2) override
  {
  }

  void Emit(Accessibility::Accessible *obj, Accessibility::WindowEvent we, unsigned int detail1) override
  {
  }

  void Emit(Accessibility::Accessible *obj, Accessibility::ObjectPropertyChangeEvent event) override
  {
  }

  void EmitBoundsChanged(Accessibility::Accessible *obj, Rect<> rect) override
  {
  }

  Accessibility::Consumed Emit(Accessibility::KeyEventType type, unsigned int keyCode, const std::string& keyName, unsigned int timeStamp, bool isText) override
  {
    return Accessibility::Consumed::YES;
  }

  void Say( const std::string& text, bool discardable, std::function<void(std::string)> callback ) override
  {
  }

  void Pause() override
  {
  }

  void Resume() override
  {
  }

  bool GetScreenReaderEnabled() override
  {
    return false;
  }

  bool GetIsEnabled() override
  {
    return false;
  }
};

DummyBridge dummyBridge;

} // namespace

Accessibility::Accessible::Accessible()
{
}

Accessibility::Accessible::~Accessible()
{
}

std::vector<Accessibility::Accessible *> Accessibility::Accessible::GetChildren()
{
  return {};
}

Accessibility::Accessible *Accessibility::Accessible::GetDefaultLabel()
{
  return nullptr;
}

Accessibility::Address Accessibility::Accessible::GetAddress()
{
  return {};
}

std::shared_ptr< Accessibility::Bridge::Data > Accessibility::Accessible::GetBridgeData()
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

Accessibility::Accessible *Accessibility::Component::GetAccessibleAtPoint(Accessibility::Point p, Accessibility::CoordType ctype)
{
  return nullptr;
}

Accessibility::Bridge *Accessibility::Bridge::GetCurrentBridge()
{
  return &dummyBridge;
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

void Accessibility::Accessible::EmitTextInserted(unsigned int position, unsigned int length, const std::string &content)
{
}

void Accessibility::Accessible::EmitTextDeleted(unsigned int position, unsigned int length, const std::string &content)
{
}

void Accessibility::Accessible::EmitTextCaretMoved(unsigned int cursorPosition)
{
}

void Accessibility::Accessible::EmitActiveDescendantChanged(Accessibility::Accessible* obj, Accessibility::Accessible *child)
{
}

void Accessibility::Accessible::FindWordSeparationsUtf8(const utf8_t *s, size_t length, const char *language, char *breaks)
{
}

void Accessibility::Accessible::FindLineSeparationsUtf8(const utf8_t *s, size_t length, const char *language, char *breaks)
{
}

void Accessibility::Accessible::NotifyAccessibilityStateChange(Accessibility::States states, bool doRecursive)
{
}

} // namespace Dali
