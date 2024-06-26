#ifndef DALI_ADAPTOR_DUMMY_ATSPI_H
#define DALI_ADAPTOR_DUMMY_ATSPI_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

  void RegisterDefaultLabel(std::shared_ptr<Accessibility::Accessible> object) override
  {
  }

  void UnregisterDefaultLabel(std::shared_ptr<Accessibility::Accessible> object) override
  {
  }

  Dali::Accessibility::Accessible* GetDefaultLabel(Dali::Accessibility::Accessible* root) override
  {
    return nullptr;
  }

  void SetApplicationName(std::string name) override
  {
  }

  void SetToolkitName(std::string_view toolkitName) override
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

  void WindowCreated(Window window) override
  {
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

  void WindowMinimized(Window window) override
  {
  }

  void WindowRestored(Window window, WindowRestoreType detail) override
  {
  }

  void WindowMaximized(Window window) override
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

  void EmitSocketAvailable(Accessibility::Accessible* obj) override
  {
  }

  void EmitScrollStarted(Accessibility::Accessible* obj) override
  {
  }

  void EmitScrollFinished(Accessibility::Accessible* obj) override
  {
  }

  void EmitStateChanged(std::shared_ptr<Accessibility::Accessible> obj, Accessibility::State state, int newValue, int reserved) override
  {
  }

  void Emit(Accessibility::Accessible* obj, Accessibility::WindowEvent event, unsigned int detail) override
  {
  }

  void Emit(std::shared_ptr<Accessibility::Accessible> obj, Accessibility::ObjectPropertyChangeEvent event) override
  {
  }

  void EmitBoundsChanged(std::shared_ptr<Accessibility::Accessible> obj, Rect<> rect) override
  {
  }

  void EmitPostRender(std::shared_ptr<Accessibility::Accessible> obj) override
  {
  }

  bool EmitKeyEvent(Dali::KeyEvent keyEvent, std::function<void(Dali::KeyEvent, bool)> callback) override
  {
    return false;
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

  Address EmbedSocket(const Address& plug, const Address& socket) override
  {
    return {};
  }

  void EmbedAtkSocket(const Address& plug, const Address& socket) override
  {
  }

  void UnembedSocket(const Address& plug, const Address& socket) override
  {
  }

  void SetSocketOffset(ProxyAccessible* socket, std::int32_t x, std::int32_t y) override
  {
  }

  void SetExtentsOffset(std::int32_t x, std::int32_t y) override
  {
  }

  void SetPreferredBusName(std::string_view preferredBusName) override
  {
  }
};

} // namespace Dali::Accessibility

#endif // DALI_ADAPTOR_DUMMY_ATSPI_H
