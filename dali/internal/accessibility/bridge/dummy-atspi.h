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

namespace Dali::Accessibility
{

struct DummyBridge : Dali::Accessibility::Bridge
{
  static DummyBridge *GetInstance()
  {
    static DummyBridge instance;

    return &instance;
  }

  const std::string& GetBusName() const override
  {
    static const std::string name = "";
    return name;
  }

  void AddTopLevelWindow(Accessibility::Accessible*) override
  {
  }

  void RemoveTopLevelWindow(Accessibility::Accessible*) override
  {
  }

  void AddPopup(Accessibility::Accessible*) override
  {
  }

  void RemovePopup(Accessibility::Accessible*) override
  {
  }

  void SetApplicationName(std::string) override
  {
  }

  Accessibility::Accessible* GetApplication() const override
  {
    return nullptr;
  }

  Accessibility::Accessible* FindByPath(const std::string& s) const override
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

  void EmitCaretMoved(Accessibility::Accessible* obj, unsigned int cursorPosition) override
  {
  }

  void EmitActiveDescendantChanged(Accessibility::Accessible* obj, Accessibility::Accessible* child) override
  {
  }

  void EmitTextChanged(Accessibility::Accessible* obj, Accessibility::TextChangedState state, unsigned int position, unsigned int length, const std::string& content) override
  {
  }

  void EmitStateChanged(Accessibility::Accessible* obj, Accessibility::State state, int val1, int val2) override
  {
  }

  void Emit(Accessibility::Accessible* obj, Accessibility::WindowEvent we, unsigned int detail1) override
  {
  }

  void Emit(Accessibility::Accessible* obj, Accessibility::ObjectPropertyChangeEvent event) override
  {
  }

  void EmitBoundsChanged(Accessibility::Accessible* obj, Rect<> rect) override
  {
  }

  Accessibility::Consumed Emit(Accessibility::KeyEventType type, unsigned int keyCode, const std::string& keyName, unsigned int timeStamp, bool isText) override
  {
    return Accessibility::Consumed::YES;
  }

  void Say(const std::string& text, bool discardable, std::function<void(std::string)> callback) override
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

} // namespace Dali::Accessibility
