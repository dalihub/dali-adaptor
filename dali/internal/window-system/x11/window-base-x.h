#ifndef DALI_INTERNAL_WINDOW_SYSTEM_X11_WINDOW_BASE_X_H
#define DALI_INTERNAL_WINDOW_SYSTEM_X11_WINDOW_BASE_X_H

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
#include <dali/internal/window-system/x11/window-system-x.h>
#include <dali/public-api/object/any.h>

using Dali::Internal::Adaptor::WindowSystem::WindowSystemX;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * WindowBaseX class provides an WindowBase X implementation.
 */
class WindowBaseX : public WindowBase
{
public:
  /**
   * @brief Constructor
   */
  WindowBaseX(PositionSize positionSize, Any surface, bool isTransparent);

  /**
   * @brief Destructor
   */
  virtual ~WindowBaseX();

public:
  /**
   * @brief Called when the window has been moved/resized
   */
  void OnConfigure(WindowSystemBase::EventBase* event);

  /**
   * @brief Called when the window property is changed.
   */
  bool OnWindowPropertyChanged(void* data, WindowSystemBase::Event type, WindowSystemBase::EventBase* event);

  /**
   * @brief Called when the window receives a delete request
   */
  void OnDeleteRequest();

  /**
   * @brief Called when the window gains focus.
   */
  void OnFocusIn(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event);

  /**
   * @brief Called when the window loses focus.
   */
  void OnFocusOut(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event);

  /**
   * @brief Called when the window is damaged.
   */
  void OnWindowDamaged(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event);

  /**
   * @brief Called when a touch down is received.
   */
  void OnMouseButtonDown(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event);

  /**
   * @brief Called when a touch up is received.
   */
  void OnMouseButtonUp(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event);

  /**
   * @brief Called when a touch motion is received.
   */
  void OnMouseButtonMove(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event);

  /**
   * @brief Called when a mouse wheel is received.
   */
  void OnMouseWheel(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event);

  /**
   * Create the dali key event from the X11 event
   */
  Integration::KeyEvent CreateKeyEvent(WindowSystemX::X11KeyEvent* keyEvent, Integration::KeyEvent::State state);

  /**
   * @brief Called when a key down is received.
   */
  void OnKeyDown(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event);

  /**
   * @brief Called when a key up is received.
   */
  void OnKeyUp(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event);

  /**
   * @brief Called when the source window notifies us the content in clipboard is selected.
   */
  void OnSelectionClear(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event);

  /**
   * @brief Called when the source window sends us about the selected content.
   */
  void OnSelectionNotify(void* data, WindowSystemBase::Event, WindowSystemBase::EventBase* event);

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
  void SetType(Dali::Window::Type type) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetType()
   */
  [[nodiscard]] Dali::Window::Type GetType() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetNotificationLevel()
   */
  bool SetNotificationLevel(Dali::Window::NotificationLevel::Type level) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetNotificationLevel()
   */
  [[nodiscard]] Dali::Window::NotificationLevel::Type GetNotificationLevel() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetOpaqueState()
   */
  void SetOpaqueState(bool opaque) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetScreenOffMode()
   */
  bool SetScreenOffMode(Dali::Window::ScreenOffMode::Type screenOffMode) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetScreenOffMode()
   */
  Dali::Window::ScreenOffMode::Type GetScreenOffMode() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetBrightness()
   */
  bool SetBrightness(int brightness) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetBrightness()
   */
  [[nodiscard]] int GetBrightness() const override;

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

  void SetAvailableOrientations(const std::vector<Dali::Window::WindowOrientation>& orientations) override;

  void SetPreferredOrientation(Dali::Window::WindowOrientation orientation) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::WindowRotationCompleted()
   */
  void WindowRotationCompleted(int degree, int width, int height) override;

  /**
   * @copydoc Dali::Internal::Adaptor::WindowBase::SetTransparency()
   */
  void SetTransparency(bool transparent) override;

private:
  /**
   * Second stage initialization
   */
  void Initialize(PositionSize positionSize, Any surface, bool isTransparent);

  void EnableMultipleSelection() const;

  void EnableWindowClose() const;

  void EnableDragAndDrop() const;

  void SetupEvents();

  void DeleteEvents();

  /**
   * @brief Get the surface id if the surface parameter is not empty
   * @param surface Any containing a surface id, or can be empty
   * @return surface id, or zero if surface is empty
   */
  unsigned int GetSurfaceId(Any surface) const;

  /**
   * @brief Create window
   */
  void CreateInternalWindow(PositionSize positionSize, bool isTransparent);

public:
  // Undefined
  WindowBaseX(const WindowBaseX&) = delete;

  // Undefined
  WindowBaseX& operator=(const WindowBaseX& rhs) = delete;

private:
  Dali::Vector<WindowSystemBase::EventHandler*> mEventHandlers;
  ::Window                                      mWindow;         ///< Native window handle
  bool                                          mOwnSurface : 1; ///< Whether we own the surface (responsible for deleting it)
  bool                                          mIsTransparent;  ///< Whether the window is transparent (32 bit or 24 bit)
  bool                                          mRotationAppSet : 1;
  int                                           mWindowRotationAngle;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_LIBUVX_WINDOW_BASE_X_H
