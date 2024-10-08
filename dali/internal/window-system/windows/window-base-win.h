#ifndef DALI_INTERNAL_WINDOWSYSTEM_WINDOW_BASE_WIN_H
#define DALI_INTERNAL_WINDOWSYSTEM_WINDOW_BASE_WIN_H

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

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/window-base.h>
#include <dali/internal/window-system/windows/event-system-win.h>
#include <dali/internal/window-system/windows/platform-implement-win.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class Window;
class WindowRenderSurface;
class WindowRenderSurfaceWin;

/**
 * WindowBaseWin class provides an WindowBase Win32 implementation.
 */
class WindowBaseWin : public WindowBase
{
public:
  /**
   * @brief Constructor
   */
  WindowBaseWin(PositionSize positionSize, Any surface, bool isTransparent);

  /**
   * @brief Destructor
   */
  virtual ~WindowBaseWin();

public:
  /**
   * Called when the window receives a delete request
   */
  void OnDeleteRequest();

  /**
   * @brief Called when the window gains focus.
   */
  void OnFocusIn(int type, TWinEventInfo* event);

  /**
   * @brief Called when the window loses focus.
   */
  void OnFocusOut(int type, TWinEventInfo* event);

  /**
   * @brief Called when the window is damaged.
   */
  void OnWindowDamaged(int type, TWinEventInfo* event);

  /**
   * @brief Called when a touch down is received.
   */
  void OnMouseButtonDown(int type, TWinEventInfo* event);

  /**
   * @brief Called when a touch up is received.
   */
  void OnMouseButtonUp(int type, TWinEventInfo* event);

  /**
   * @brief Called when a touch motion is received.
   */
  void OnMouseButtonMove(int type, TWinEventInfo* event);

  /**
   * @brief Called when a mouse wheel is received.
   */
  void OnMouseWheel(int type, TWinEventInfo* event);

  /**
   * @brief Called when a key down is received.
   */
  void OnKeyDown(int type, TWinEventInfo* event);

  /**
   * @brief Called when a key up is received.
   */
  void OnKeyUp(int type, TWinEventInfo* event);

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
   * @copydoc Dali::Internal::Adaptor::WindowBase::CreateWindow()
   */
  Dali::Any CreateWindow(int width, int height) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::DestroyWindow()
   */
  void DestroyWindow() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetWindowRotation()
   */
  void SetWindowRotation(int angle) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetWindowBufferTransform()
   */
  void SetWindowBufferTransform(int angle) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetWindowTransform()
   */
  void SetWindowTransform(int angle) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::ResizeWindow()
   */
  void ResizeWindow(PositionSize positionSize) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::IsWindowRotationSupported()
   */
  bool IsWindowRotationSupported() override;

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
  Dali::WindowOperationResult SetNotificationLevel(Dali::WindowNotificationLevel level) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetNotificationLevel()
   */
  Dali::WindowNotificationLevel GetNotificationLevel() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetOpaqueState()
   */
  void SetOpaqueState(bool opaque) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetScreenOffMode()
   */
  Dali::WindowOperationResult SetScreenOffMode(WindowScreenOffMode screenOffMode) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetScreenOffMode()
   */
  WindowScreenOffMode GetScreenOffMode() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetBrightness()
   */
  Dali::WindowOperationResult SetBrightness(int brightness) override;

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
  bool GrabKeyList(const Dali::Vector<Dali::KEY>& key, const Dali::Vector<KeyGrab::KeyGrabMode>& grabMode, Dali::Vector<bool>& result) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::UngrabKeyList()
   */
  bool UngrabKeyList(const Dali::Vector<Dali::KEY>& key, Dali::Vector<bool>& result) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetDpi()
   */
  void GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical) override;

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
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetWindowRotationAngle()
   */
  int GetWindowRotationAngle() const override;

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
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetWindowFrontBufferMode()
   */
  void SetWindowFrontBufferMode(bool enable) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetModal()
   */
  void SetModal(bool modal) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::IsModal()
   */
  bool IsModal() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetAlwaysOnTop()
   */
  void SetAlwaysOnTop(bool alwaysOnTop) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::IsAlwaysOnTop()
   */
  bool IsAlwaysOnTop() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetBottom()
   */
  void SetBottom(bool enable) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::IsBottom()
   */
  bool IsBottom() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetNativeBuffer()
   */
  Any GetNativeBuffer() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::RelativeMotionGrab()
   */
  bool RelativeMotionGrab(uint32_t boundary) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::RelativeMotionUnGrab()
   */
  bool RelativeMotionUnGrab() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetBackgroundBlur()
   */
  void SetBackgroundBlur(int blurRadius, int cornerRadius) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetBackgroundBlur()
   */
  int GetBackgroundBlur() override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetBehindBlur()
   */
  void SetBehindBlur(int blurRadius) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetBehindBlur()
   */
  int GetBehindBlur() override;
private:
  /**
   * Second stage initialization
   */
  void Initialize(PositionSize positionSize, Any surface, bool isTransparent);

  /**
   * @brief Get the surface id if the surface parameter is not empty
   * @param surface Any containing a surface id, or can be empty
   * @return surface id, or zero if surface is empty
   */
  uintptr_t GetSurfaceId(Any surface) const;

  /**
   * @brief Create window
   */
  void CreateWinWindow(PositionSize positionSize, bool isTransparent);

  /**
   * @brief Sets up an already created window.
   */
  void SetWinWindow(uintptr_t surfaceId);

private:
  // Undefined
  WindowBaseWin(const WindowBaseWin&) = delete;

  // Undefined
  WindowBaseWin& operator=(const WindowBaseWin& rhs) = delete;

private:
  void EventEntry(TWinEventInfo* event);

private:
  WinWindowHandle mWin32Window;       ///< Native window handle
  bool            mOwnSurface : 1;    ///< Whether we own the surface (responsible for deleting it)
  bool            mIsTransparent : 1; ///< Whether the window is transparent (32 bit or 24 bit)
  bool            mRotationAppSet : 1;

  WindowsPlatform::WindowImpl mWindowImpl;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_WINDOW_BASE_WIN_H
