#ifndef DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_WINDOW_BASE_ECORE_WL_H
#define DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_WINDOW_BASE_ECORE_WL_H

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

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/window-base.h>

// EXTERNAL HEADERS
#include <Ecore.h>
#include <Ecore_Wayland.h>
#include <tizen-extension-client-protocol.h>
#include <wayland-egl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class WindowRenderSurface;
class WindowRenderSurfaceEcoreWl;

/**
 * WindowBaseEcoreWl class provides an WindowBase Ecore-Wayland implementation.
 */
class WindowBaseEcoreWl : public WindowBase
{
public:
  /**
   * @brief Constructor
   */
  WindowBaseEcoreWl(PositionSize positionSize, Any surface, bool isTransparent);

  /**
   * @brief Destructor
   */
  virtual ~WindowBaseEcoreWl();

public:
  /**
   * @brief Called when the window iconify state is changed.
   */
  Eina_Bool OnIconifyStateChanged(void* data, int type, void* event);

  /**
   * @brief Called when the window gains focus.
   */
  Eina_Bool OnFocusIn(void* data, int type, void* event);

  /**
   * @brief Called when the window loses focus.
   */
  Eina_Bool OnFocusOut(void* data, int type, void* event);

  /**
   * @brief Called when the output is transformed.
   */
  Eina_Bool OnOutputTransform(void* data, int type, void* event);

  /**
   * @brief Called when the output transform should be ignored.
   */
  Eina_Bool OnIgnoreOutputTransform(void* data, int type, void* event);

  /**
   * @brief Called when a rotation event is recevied.
   */
  void OnRotation(void* data, int type, void* event);

  /**
   * @brief Called when a touch down is received.
   */
  void OnMouseButtonDown(void* data, int type, void* event);

  /**
   * @brief Called when a touch up is received.
   */
  void OnMouseButtonUp(void* data, int type, void* event);

  /**
   * @brief Called when a touch motion is received.
   */
  void OnMouseButtonMove(void* data, int type, void* event);

  /**
   * @brief Called when a touch is canceled.
   */
  void OnMouseButtonCancel(void* data, int type, void* event);

  /**
   * @brief Called when a mouse wheel is received.
   */
  void OnMouseWheel(void* data, int type, void* event);

  /**
   * @brief Called when a detent rotation event is recevied.
   */
  void OnDetentRotation(void* data, int type, void* event);

  /**
   * @brief Called when a key down is received.
   */
  void OnKeyDown(void* data, int type, void* event);

  /**
   * @brief Called when a key up is received.
   */
  void OnKeyUp(void* data, int type, void* event);

  /**
   * @brief Called when the source window notifies us the content in clipboard is selected.
   */
  void OnDataSend(void* data, int type, void* event);

  /**
   * @brief Called when the source window sends us about the selected content.
   */
  void OnDataReceive(void* data, int type, void* event);

  /**
   * @brief Called when a font name is changed.
   */
  void OnFontNameChanged();

  /**
   * @brief Called when a font size is changed.
   */
  void OnFontSizeChanged();

  /**
   * @brief RegistryGlobalCallback
   */
  void RegistryGlobalCallback(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version);

  /**
   * @brief RegistryGlobalCallbackRemove
   */
  void RegistryGlobalCallbackRemove(void* data, struct wl_registry* registry, uint32_t id);

  /**
   * @brief TizenPolicyNotificationChangeDone
   */
  void TizenPolicyNotificationChangeDone(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, int32_t level, uint32_t state);

  /**
   * @brief TizenPolicyScreenModeChangeDone
   */
  void TizenPolicyScreenModeChangeDone(void* data, struct tizen_policy* tizenPolicy, struct wl_surface* surface, uint32_t mode, uint32_t state);

  /**
   * @brief DisplayPolicyBrightnessChangeDone
   */
  void DisplayPolicyBrightnessChangeDone(void* data, struct tizen_display_policy* displayPolicy, struct wl_surface* surface, int32_t brightness, uint32_t state);

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
   * @copydoc Dali::Internal::Adaptor::WindowBase::GetOrientation()
   */
  int GetOrientation() const override;

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

private:
  /**
   * Second stage initialization
   */
  void Initialize(PositionSize positionSize, Any surface, bool isTransparent);

  /**
   * @brief Create window
   */
  void CreateWindow(PositionSize positionSize);

protected:
  // Undefined
  WindowBaseEcoreWl(const WindowBaseEcoreWl&) = delete;

  // Undefined
  WindowBaseEcoreWl& operator=(const WindowBaseEcoreWl& rhs) = delete;

private:
  typedef std::vector<std::pair<std::string, std::string> > AuxiliaryHints;

  Dali::Vector<Ecore_Event_Handler*> mEcoreEventHandler;

  Ecore_Wl_Window*      mEcoreWindow;
  wl_surface*           mWlSurface;
  wl_egl_window*        mEglWindow;
  wl_display*           mDisplay;
  wl_event_queue*       mEventQueue;
  tizen_policy*         mTizenPolicy;
  tizen_display_policy* mTizenDisplayPolicy;

  std::vector<std::string> mSupportedAuxiliaryHints;
  AuxiliaryHints           mAuxiliaryHints;

  int      mNotificationLevel;
  uint32_t mNotificationChangeState;
  bool     mNotificationLevelChangeDone;

  int      mScreenOffMode;
  uint32_t mScreenOffModeChangeState;
  bool     mScreenOffModeChangeDone;

  int      mBrightness;
  uint32_t mBrightnessChangeState;
  bool     mBrightnessChangeDone;

  bool mOwnSurface;

  int mWindowRotationAngle;
  int mScreenRotationAngle;
  int mSupportedPreProtation;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_TIZENWAYLAND_WINDOW_BASE_ECORE_WL_H
