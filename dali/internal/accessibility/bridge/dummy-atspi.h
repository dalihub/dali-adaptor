#ifndef DALI_ADAPTOR_DUMMY_ATSPI_H
#define DALI_ADAPTOR_DUMMY_ATSPI_H

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

#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>

namespace Dali::Accessibility
{
struct DummyBridge : Dali::Accessibility::Bridge
{
  static std::shared_ptr<DummyBridge> GetInstance()
  {
    static auto instance = std::make_shared<DummyBridge>();

    return instance;
  }

  const std::string& GetBusName() const override
  {
    static const std::string name = "";
    return name;
  }

  void AddTopLevelWindow(Accessibility::Accessible* object) override
  {
  }

  void RemoveTopLevelWindow(Accessibility::Accessible* object) override
  {
  }

  void RegisterDefaultLabel(Accessibility::Accessible* object) override
  {
  }

  void UnregisterDefaultLabel(Accessibility::Accessible* object) override
  {
  }

  Dali::Accessibility::Accessible* GetDefaultLabel() const override
  {
    return nullptr;
  }

  void SetApplicationName(std::string name) override
  {
  }

  Accessibility::Accessible* GetApplication() const override
  {
    return nullptr;
  }

  Accessibility::Accessible* FindByPath(const std::string& path) const override
  {
    return nullptr;
  }

  void WindowShown(Window window) override
  {
  }

  void WindowHidden(Window window) override
  {
  }

  void WindowFocused(Window window) override
  {
  }

  void WindowUnfocused(Window window) override
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

  void EmitCursorMoved(Accessibility::Accessible* obj, unsigned int cursorPosition) override
  {
  }

  void EmitActiveDescendantChanged(Accessibility::Accessible* obj, Accessibility::Accessible* child) override
  {
  }

  void EmitTextChanged(Accessibility::Accessible* obj, Accessibility::TextChangedState state, unsigned int position, unsigned int length, const std::string& content) override
  {
  }

  void EmitMovedOutOfScreen(Accessibility::Accessible* obj, ScreenRelativeMoveType type) override
  {
  }

  void EmitStateChanged(Accessibility::Accessible* obj, Accessibility::State state, int newValue, int reserved) override
  {
  }

  void Emit(Accessibility::Accessible* obj, Accessibility::WindowEvent event, unsigned int detail) override
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

  void StopReading(bool alsoNonDiscardable) override
  {
  }

  void SuppressScreenReader(bool suppress) override
  {
  }

  bool GetScreenReaderEnabled() override
  {
    return false;
  }

  bool IsEnabled() override
  {
    return false;
  }
};

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_DUMMY_ATSPI_H
