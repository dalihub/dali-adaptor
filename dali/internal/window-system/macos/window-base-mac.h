#pragma once

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/common/window-base.h>
#include <memory>

namespace Dali::Internal::Adaptor
{
/**
 * WindowBaseCocoa class provides an WindowBase Win32 implementation.
 */
class WindowBaseCocoa : public WindowBase
{
public:
  struct Impl;

  /**
   * @brief Constructor
   */
  WindowBaseCocoa(PositionSize positionSize, Any surface, bool isTransparent);

  /**
   * @brief Destructor
   */
  ~WindowBaseCocoa();

public:
  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetNativeWindow()
   */
  Any GetNativeWindow() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetNativeWindowId()
   */
  int GetNativeWindowId() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetNativeWindowResourceId()
   */
  std::string GetNativeWindowResourceId() override;

  /**
  * @copydoc Dali::Internal::Adaptor::WindowBase::CreateEglWindow()
   */
  EGLNativeWindowType CreateEglWindow(int width, int height) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::DestroyEglWindow()
   */
  void DestroyEglWindow() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetEglWindowRotation()
   */
  void SetEglWindowRotation(int angle) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetEglWindowBufferTransform()
   */
  void SetEglWindowBufferTransform(int angle) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetEglWindowTransform()
   */
  void SetEglWindowTransform(int angle) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::ResizeEglWindow()
   */
  void ResizeEglWindow(PositionSize positionSize) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::IsEglWindowRotationSupported()
   */
  bool IsEglWindowRotationSupported() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Move()
   */
  void Move(PositionSize positionSize) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Resize()
   */
  void Resize(PositionSize positionSize) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::MoveResize()
   */
  void MoveResize(PositionSize positionSize) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetLayout()
   */
  void SetLayout(unsigned int numCols, unsigned int numRows, unsigned int column, unsigned int row, unsigned int colSpan, unsigned int rowSpan) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetClass()
   */
  void SetClass(const std::string& name, const std::string& className) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Raise()
   */
  void Raise() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Lower()
   */
  void Lower() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Activate()
   */
  void Activate() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Maximize()
   */
  void Maximize(bool maximize) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::IsMaximized()
   */
  bool IsMaximized() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetMaximumSize()
   */
  void SetMaximumSize(Dali::Window::WindowSize size) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Minimize()
   */
  void Minimize(bool minimize) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::IsMinimized()
   */
  bool IsMinimized() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetMimimumSize()
   */
  void SetMimimumSize(Dali::Window::WindowSize size) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetAvailableAnlges()
   */
  void SetAvailableAnlges(const std::vector<int>& angles) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetPreferredAngle()
   */
  void SetPreferredAngle(int angle) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetAcceptFocus()
   */
  void SetAcceptFocus(bool accept) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Show()
   */
  void Show() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::Hide()
   */
  void Hide() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetSupportedAuxiliaryHintCount()
   */
  unsigned int GetSupportedAuxiliaryHintCount() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetSupportedAuxiliaryHint()
   */
  std::string GetSupportedAuxiliaryHint(unsigned int index) const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::AddAuxiliaryHint()
   */
  unsigned int AddAuxiliaryHint(const std::string& hint, const std::string& value) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::RemoveAuxiliaryHint()
   */
  bool RemoveAuxiliaryHint(unsigned int id) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetAuxiliaryHintValue()
   */
  bool SetAuxiliaryHintValue(unsigned int id, const std::string& value) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetAuxiliaryHintValue()
   */
  std::string GetAuxiliaryHintValue(unsigned int id) const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetAuxiliaryHintId()
   */
  unsigned int GetAuxiliaryHintId(const std::string& hint) const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetInputRegion()
   */
  void SetInputRegion(const Rect<int>& inputRegion) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetType()
   */
  void SetType(Dali::WindowType type) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetType()
   */
  Dali::WindowType GetType() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetNotificationLevel()
   */
  WindowOperationResult SetNotificationLevel(WindowNotificationLevel level) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetNotificationLevel()
   */
  WindowNotificationLevel GetNotificationLevel() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetOpaqueState()
   */
  void SetOpaqueState(bool opaque) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetScreenOffMode()
   */
  WindowOperationResult SetScreenOffMode(WindowScreenOffMode screenOffMode) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetScreenOffMode()
   */
  WindowScreenOffMode GetScreenOffMode() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetBrightness()
   */
  WindowOperationResult SetBrightness(int brightness) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetBrightness()
   */
  int GetBrightness() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GrabKey()
   */
  bool GrabKey(Dali::KEY key, KeyGrab::KeyGrabMode grabMode) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::UngrabKey()
   */
  bool UngrabKey(Dali::KEY key) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GrabKeyList()
   */
  bool GrabKeyList(
    const Dali::Vector<Dali::KEY>&            key,
    const Dali::Vector<KeyGrab::KeyGrabMode>& grabMode,
    Dali::Vector<bool>&                       result) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::UngrabKeyList()
   */
  bool UngrabKeyList(
    const Dali::Vector<Dali::KEY>& key,
    Dali::Vector<bool>&            result) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetDpi()
   */
  void GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetWindowRotationAngle()
   */
  int GetWindowRotationAngle() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetScreenRotationAngle()
   */
  int GetScreenRotationAngle() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetWindowRotationAngle()
   */
  void SetWindowRotationAngle(int degree) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::WindowRotationCompleted()
   */
  void WindowRotationCompleted(int degree, int width, int height) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetTransparency()
   */
  void SetTransparency(bool transparent) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetParent()
   */
  void SetParent(WindowBase* parentWinBase, bool belowParent) override;

  /**
   * @copydoc  Dali::Internal::Adaptor::WindowBase::CreateFrameRenderedSyncFence()
   */
  int CreateFrameRenderedSyncFence() override;

  /**
   * @copydoc  Dali::Internal::Adaptor::WindowBase::CreateFramePresentedSyncFence()
   */
  int CreateFramePresentedSyncFence() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetPositionSizeWithAngle()
   */
  void SetPositionSizeWithAngle(PositionSize positionSize, int angle) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::InitializeIme()
   */
  void InitializeIme() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::ImeWindowReadyToRender()
   */
  void ImeWindowReadyToRender() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::RequestMoveToServer()
   */
  void RequestMoveToServer() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::RequestResizeToServer()
   */
  void RequestResizeToServer(WindowResizeDirection direction) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::EnableFloatingMode()
   */
  void EnableFloatingMode(bool enable) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::IsFloatingModeEnabled()
   */
  bool IsFloatingModeEnabled() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::IncludeInputRegion()
   */
  void IncludeInputRegion(const Rect<int>& inputRegion) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::ExcludeInputRegion()
   */
  void ExcludeInputRegion(const Rect<int>& inputRegion) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::PointerConstraintsLock()
   */
  bool PointerConstraintsLock() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::PointerConstraintsUnlock()
   */
  bool PointerConstraintsUnlock() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::LockedPointerRegionSet()
   */
  void LockedPointerRegionSet(int32_t x, int32_t y, int32_t width, int32_t height) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::LockedPointerCursorPositionHintSet()
   */
  void LockedPointerCursorPositionHintSet(int32_t x, int32_t y) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::PointerWarp()
   */
  bool PointerWarp(int32_t x, int32_t y) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::CursorVisibleSet()
   */
  void CursorVisibleSet(bool visible) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::KeyboardGrab()
   */
  bool KeyboardGrab(Device::Subclass::Type deviceSubclass) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::KeyboardUnGrab()
   */
  bool KeyboardUnGrab() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetFullScreen()
   */
  void SetFullScreen(bool fullscreen) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetFullScreen()
   */
  bool GetFullScreen() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetFrontBufferRendering()
   */
  void SetFrontBufferRendering(bool enable) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetFrontBufferRendering()
   */
  bool GetFrontBufferRendering() override;

  /**
   * @brief Sets front buffer rendering to the egl window.
   * @param[in] enable True to enable front buffer rendering mode, False to otherwise.
   */
  void SetEglWindowFrontBufferMode(bool enable) override;

private:
  // Undefined
  WindowBaseCocoa(const WindowBaseCocoa&) = delete;

  // Undefined
  WindowBaseCocoa& operator=(const WindowBaseCocoa& rhs) = delete;

private:
  std::unique_ptr<Impl> mImpl;
};

} // namespace Dali::Internal::Adaptor
