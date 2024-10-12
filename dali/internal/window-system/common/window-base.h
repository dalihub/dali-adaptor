#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_BASE_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_BASE_H

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

// EXTERNAL INCLUDES
#include <dali/integration-api/events/key-event-integ.h>
#include <dali/integration-api/events/point.h>
#include <dali/integration-api/events/wheel-event-integ.h>
#include <cstdint>
#include <string>
#include <vector>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/mouse-in-out-event.h>
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/internal/window-system/common/damage-observer.h>
#include <dali/internal/window-system/common/rotation-event.h>
#include <dali/public-api/adaptor-framework/key-grab.h>
#include <dali/public-api/adaptor-framework/style-change.h>
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/public-api/adaptor-framework/window.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * WindowBase interface
 */
class WindowBase
{
public:
  /**
   * @brief Struct used to retrieve accessibility information
   */
  struct AccessibilityInfo
  {
    int gestureValue;
    int startX;
    int startY;
    int endX;
    int endY;
    int state;
    int eventTime;
  };

  // Window
  typedef Signal<void(bool)>                                                           IconifySignalType;
  typedef Signal<void(bool)>                                                           MaximizeSignalType;
  typedef Signal<void(bool)>                                                           FocusSignalType;
  typedef Signal<void()>                                                               OutputSignalType;
  typedef Signal<void()>                                                               DeleteSignalType;
  typedef Signal<void(const DamageArea&)>                                              DamageSignalType;
  typedef Signal<void(const RotationEvent&)>                                           RotationSignalType;
  typedef Signal<void(WindowEffectState, WindowEffectType)>                            TransitionEffectEventSignalType;
  typedef Signal<void()>                                                               KeyboardRepeatSettingsChangedSignalType;
  typedef Signal<void()>                                                               WindowRedrawRequestSignalType;
  typedef Signal<void(Dali::PositionSize&)>                                            UpdatePositionSizeType;
  typedef Signal<void(const std::string&, const std::string&, const Property::Array&)> AuxiliaryMessageSignalType;
  typedef Signal<void(const Dali::DevelWindow::MouseInOutEvent&)>                      MouseInOutEventSignalType;
  typedef Signal<void(const Dali::DevelWindow::MouseRelativeEvent&)>                   MouseRelativeEventSignalType;
  typedef Signal<void(Dali::Int32Pair&)>                                               MoveCompletedSignalType;
  typedef Signal<void(Dali::Uint16Pair&)>                                              ResizeCompletedSignalType;
  typedef Signal<void(WindowInsetsPartType, WindowInsetsPartState, const Extents&)>    InsetsChangedSignalType;
  typedef Signal<void(const Dali::Int32Pair&, bool, bool)>                             PointerConstraintsSignalType;

  // Input events
  typedef Signal<void(Integration::Point&, uint32_t)> TouchEventSignalType;
  typedef Signal<void()>                              MouseFrameEventSignalType;
  typedef Signal<void(Integration::WheelEvent&)>      WheelEventSignalType;
  typedef Signal<void(Integration::KeyEvent&)>        KeyEventSignalType;

  // Clipboard
  typedef Signal<void(void*)> SelectionSignalType;

  // Accessibility
  typedef Signal<void(StyleChange::Type)> StyleSignalType;

  /**
   * @brief Default constructor
   */
  WindowBase();

  /**
   * @brief Destructor
   */
  virtual ~WindowBase();

public:
  /**
   * @brief Get the native window handle
   * @return The native window handle
   */
  virtual Any GetNativeWindow() = 0;

  /**
   * @brief Get the native window id
   * @return The native window id
   */
  virtual int GetNativeWindowId() = 0;

  /**
   * @brief Get the native window resource id assigned by window manager
   * @return The native window resource id
   */
  virtual std::string GetNativeWindowResourceId() = 0;

  /**
   * @brief Create the window
   */
  virtual Any CreateWindow(int width, int height) = 0;

  /**
   * @brief Destroy the window
   */
  virtual void DestroyWindow() = 0;

  /**
   * @brief Set the window rotation
   */
  virtual void SetWindowRotation(int angle) = 0;

  /**
   * @brief Set the window buffer transform
   */
  virtual void SetWindowBufferTransform(int angle) = 0;

  /**
   * @brief Set the window transform
   */
  virtual void SetWindowTransform(int angle) = 0;

  /**
   * @brief Resize the window
   */
  virtual void ResizeWindow(Dali::PositionSize positionSize) = 0;

  /**
   * @brief Returns whether the window support rotation or not
   */
  virtual bool IsWindowRotationSupported() = 0;

  /**
   * @brief Move the window
   */
  virtual void Move(Dali::PositionSize positionSize) = 0;

  /**
   * @brief Resize the window
   */
  virtual void Resize(Dali::PositionSize positionSize) = 0;

  /**
   * @brief Move and resize the window
   */
  virtual void MoveResize(Dali::PositionSize positionSize) = 0;

  /**
   * @copydoc Dali::Window::SetLayout()
   */
  virtual void SetLayout(unsigned int numCols, unsigned int numRows, unsigned int column, unsigned int row, unsigned int colSpan, unsigned int rowSpan) = 0;

  /**
   * @copydoc Dali::Window::SetClass()
   */
  virtual void SetClass(const std::string& name, const std::string& className) = 0;

  /**
   * @copydoc Dali::Window::Raise()
   */
  virtual void Raise() = 0;

  /**
   * @copydoc Dali::Window::Lower()
   */
  virtual void Lower() = 0;

  /**
   * @copydoc Dali::Window::Activate()
   */
  virtual void Activate() = 0;

  /**
   * @copydoc Dali::DevelWindow::Maximize()
   */
  virtual void Maximize(bool maximize) = 0;

  /**
   * @copydoc Dali::DevelWindow::IsMaximized()
   */
  virtual bool IsMaximized() const = 0;

  /**
   * @copydoc Dali::DevelWindow::SetMaximumSize()
   */
  virtual void SetMaximumSize(Dali::Window::WindowSize size) = 0;

  /**
   * @copydoc Dali::DevelWindow::Minimize()
   */
  virtual void Minimize(bool minimize) = 0;

  /**
   * @copydoc Dali::DevelWindow::IsMinimized()
   */
  virtual bool IsMinimized() const = 0;

  /**
   * @copydoc Dali::DevelWindow::SetMimimumSize()
   */
  virtual void SetMimimumSize(Dali::Window::WindowSize size) = 0;

  /**
   * @copydoc Dali::Window::SetAvailableOrientations()
   */
  virtual void SetAvailableAnlges(const std::vector<int>& angles) = 0;

  /**
   * @copydoc Dali::Window::SetPreferredOrientation()
   */
  virtual void SetPreferredAngle(int angle) = 0;

  /**
   * @copydoc Dali::Window::SetAcceptFocus()
   */
  virtual void SetAcceptFocus(bool accept) = 0;

  /**
   * @copydoc Dali::Window::Show()
   */
  virtual void Show() = 0;

  /**
   * @copydoc Dali::Window::Hide()
   */
  virtual void Hide() = 0;

  /**
   * @copydoc Dali::Window::GetSupportedAuxiliaryHintCount()
   */
  virtual unsigned int GetSupportedAuxiliaryHintCount() const = 0;

  /**
   * @copydoc Dali::Window::GetSupportedAuxiliaryHint()
   */
  virtual std::string GetSupportedAuxiliaryHint(unsigned int index) const = 0;

  /**
   * @copydoc Dali::Window::AddAuxiliaryHint()
   */
  virtual unsigned int AddAuxiliaryHint(const std::string& hint, const std::string& value) = 0;

  /**
   * @copydoc Dali::Window::RemoveAuxiliaryHint()
   */
  virtual bool RemoveAuxiliaryHint(unsigned int id) = 0;

  /**
   * @copydoc Dali::Window::SetAuxiliaryHintValue()
   */
  virtual bool SetAuxiliaryHintValue(unsigned int id, const std::string& value) = 0;

  /**
   * @copydoc Dali::Window::GetAuxiliaryHintValue()
   */
  virtual std::string GetAuxiliaryHintValue(unsigned int id) const = 0;

  /**
   * @copydoc Dali::Window::GetAuxiliaryHintId()
   */
  virtual unsigned int GetAuxiliaryHintId(const std::string& hint) const = 0;

  /**
   * @copydoc Dali::Window::SetInputRegion()
   */
  virtual void SetInputRegion(const Rect<int>& inputRegion) = 0;

  /**
   * @copydoc Dali::Window::SetType()
   */
  virtual void SetType(Dali::WindowType type) = 0;

  /**
   * @copydoc Dali::Window::GetType()
   */
  virtual Dali::WindowType GetType() const = 0;

  /**
   * @copydoc Dali::Window::SetNotificationLevel()
   */
  virtual Dali::WindowOperationResult SetNotificationLevel(Dali::WindowNotificationLevel level) = 0;

  /**
   * @copydoc Dali::Window::GetNotificationLevel()
   */
  virtual Dali::WindowNotificationLevel GetNotificationLevel() const = 0;

  /**
   * @copydoc Dali::Window::SetOpaqueState()
   */
  virtual void SetOpaqueState(bool opaque) = 0;

  /**
   * @copydoc Dali::Window::SetScreenOffMode()
   */
  virtual Dali::WindowOperationResult SetScreenOffMode(WindowScreenOffMode screenOffMode) = 0;

  /**
   * @copydoc Dali::Window::GetScreenOffMode()
   */
  virtual WindowScreenOffMode GetScreenOffMode() const = 0;

  /**
   * @copydoc Dali::Window::SetBrightness()
   */
  virtual Dali::WindowOperationResult SetBrightness(int brightness) = 0;

  /**
   * @copydoc Dali::Window::GetBrightness()
   */
  virtual int GetBrightness() const = 0;

  /**
   * @copydoc Dali::KeyGrab::GrabKey()
   */
  virtual bool GrabKey(Dali::KEY key, KeyGrab::KeyGrabMode grabMode) = 0;

  /**
   * @copydoc Dali::KeyGrab::UngrabKey()
   */
  virtual bool UngrabKey(Dali::KEY key) = 0;

  /**
   * @copydoc Dali::KeyGrab::GrabKeyList()
   */
  virtual bool GrabKeyList(const Dali::Vector<Dali::KEY>& key, const Dali::Vector<KeyGrab::KeyGrabMode>& grabMode, Dali::Vector<bool>& result) = 0;

  /**
   * @copydoc Dali::KeyGrab::UngrabKeyList()
   */
  virtual bool UngrabKeyList(const Dali::Vector<Dali::KEY>& key, Dali::Vector<bool>& result) = 0;

  /**
   * @brief Get DPI
   * @param[out] dpiHorizontal set to the horizontal dpi
   * @param[out] dpiVertical set to the vertical dpi
   */
  virtual void GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical) = 0;

  /**
   * @brief Return the angle of the window's rotation.
   * @return The window orientation
   */
  virtual int GetWindowRotationAngle() const = 0;

  /**
   * @brief Get the angle of screen rotation for the window
   * @return The screen orientation
   */
  virtual int GetScreenRotationAngle() = 0;

  /**
   * @brief Set the screen rotation angle of the window
   */
  virtual void SetWindowRotationAngle(int degree) = 0;

  /**
   * @brief Inform the window rotation is completed
   */
  virtual void WindowRotationCompleted(int degree, int width, int height) = 0;

  /**
   * @brief starts the window is moved by display server
   */
  virtual void RequestMoveToServer() = 0;

  /**
   * @brief starts the window is resized by display server
   *
   * @param[in] direction It is direction of the started edge/side.
   */
  virtual void RequestResizeToServer(WindowResizeDirection direction) = 0;

  /**
   * @brief Enables the floating mode of window.
   *
   * The floating mode is to support making partial size window easliy.
   * It is useful to make popup style window
   * and this window is always upper than the other normal window.
   *
   * A special display server(as a Tizen display server) supports this mode.
   *
   * @param[in] enable Enable floating mode or not.
   */
  virtual void EnableFloatingMode(bool enable) = 0;

  /**
   * @brief Gets whether floating mode is enabled or not.
   */
  virtual bool IsFloatingModeEnabled() const = 0;

  /**
   * @copydoc Dali::Window::SetTransparency()
   */
  virtual void SetTransparency(bool transparent) = 0;

  /**
   * @copydoc Dali::Window::SetParent()
   */
  virtual void SetParent(WindowBase* parentWinBase, bool belowParent) = 0;

  /**
   * @brief Create a sync fence that can tell the frame is rendered by the graphics driver.
   * @return The file descriptor that tells when it is rendered.
   */
  virtual int CreateFrameRenderedSyncFence() = 0;

  /**
   * @brief Create a sync fence that can tell the frame is presented by the display server.
   * @return The file descriptor that tells when it is presented.
   */
  virtual int CreateFramePresentedSyncFence() = 0;

  /**
   * @copydoc Dali::Window::SetPositionSizeWithAngle()
   */
  virtual void SetPositionSizeWithAngle(PositionSize positionSize, int angle) = 0;

  /**
   * @brief Initialize for Ime window.
   * It should be called when the window is only used for Ime keyboard window.
   */
  virtual void InitializeIme() = 0;

  /**
   * @brief Send the signal to display server for Ime Window is ready to render.
   * It is used for compositing by display server.
   */
  virtual void ImeWindowReadyToRender() = 0;

  /**
   * @brief Includes input region.
   * @param[in] inputRegion The added region to accept input events.
   */
  virtual void IncludeInputRegion(const Rect<int>& inputRegion) = 0;

  /**
   * @brief Excludes input region.
   * @param[in] inputRegion The subtracted region to except input events.
   */
  virtual void ExcludeInputRegion(const Rect<int>& inputRegion) = 0;

  /**
   * @brief Sets the pointer constraints lock.
   * @return Returns true if PointerConstraintsLock succeeds.
   */
  virtual bool PointerConstraintsLock() = 0;

  /**
   * @brief Sets the pointer constraints unlock.
   * @return Returns true if PointerConstraintsUnlock succeeds.
   */
  virtual bool PointerConstraintsUnlock() = 0;

  /**
   * @brief Sets the locked pointer region
   *
   * @param[in] x The x position.
   * @param[in] y The y position.
   * @param[in] width The width.
   * @param[in] height The height
   */
  virtual void LockedPointerRegionSet(int32_t x, int32_t y, int32_t width, int32_t height) = 0;

  /**
   * @brief Sets the locked pointer cursor position hintset
   *
   * @param[in] x The x position.
   * @param[in] y The y position.
   */
  virtual void LockedPointerCursorPositionHintSet(int32_t x, int32_t y) = 0;

  /**
   * @brief Sets the pointer warp. The pointer moves to the set coordinates.
   *
   * @param[in] x The x position.
   * @param[in] y The y position.
   * @return Returns true if PointerWarp succeeds.
   */
  virtual bool PointerWarp(int32_t x, int32_t y) = 0;

  /**
   * @brief Sets visibility on/off of cursor
   *
   * @param[in] visible The visibility of cursor
   */
  virtual void CursorVisibleSet(bool visible) = 0;

  /**
   * @brief Requests grab key events according to the requested device subtype
   *
   * @param[in] deviceSubclass The deviceSubclass type.
   * @return Returns true if KeyboardGrab succeeds.
   */
  virtual bool KeyboardGrab(Device::Subclass::Type deviceSubclass) = 0;

  /**
   * @brief Requests ungrab key events
   *
   * @param[in] window The window instance.
   * @return Returns true if KeyboardUnGrab succeeds.
   */
  virtual bool KeyboardUnGrab() = 0;

  /**
   * @brief Sets full screen sized window.
   * If full screen size is set for the window,
   * window will be resized with full screen size.
   * In addition, the full screen sized window's z-order is the highest.
   *
   * @param[in] fullscreen true to set fullscreen, false to unset.
   */
  virtual void SetFullScreen(bool fullscreen) = 0;

  /**
   * @brief Gets whether the full screen sized window or not
   *
   * @return Returns true if the full screen sized window is.
   */
  virtual bool GetFullScreen() = 0;

  /**
   * @brief Enables or disables front buffer rendering.
   * @param[in] enable true to enable front buffer rendering, false to disable.
   */
  virtual void SetFrontBufferRendering(bool enable) = 0;

  /**
   * @brief Enables or disables front buffer rendering.
   * @return Returns whether front buffer rendering has been enabled or not.
   */
  virtual bool GetFrontBufferRendering() = 0;

  /**
   * @brief Sets front buffer rendering to the window.
   * @param[in] enable True to enable front buffer rendering mode, False to otherwise.
   */
  virtual void SetWindowFrontBufferMode(bool enable) = 0;

  /**
   * @brief Enables or disables the modal of window.
   *
   * @param[in] modal true to enable the modal of window, false to disable.
   */
  virtual void SetModal(bool modal) = 0;

  /**
   * @brief Returns whether the window has a modal or not.
   *
   * @return True if the window is modal, false otherwise.
   */
  virtual bool IsModal() = 0;

  /**
   * @brief Enables or disables the window always is on top.
   *
   * @param[in] alwaysOnTop true to enable the window always is on top, false to disable.
   */
  virtual void SetAlwaysOnTop(bool alwaysOnTop) = 0;

  /**
   * @brief Returns whether the window always is on top.
   *
   * @return True if the window always is on top, false otherwise.
   */
  virtual bool IsAlwaysOnTop() = 0;

  /**
   * @brief Get native buffer of window.
   * @return The native window buffer handle
   */
  virtual Any GetNativeBuffer() const = 0;

  /**
   * @brief Requests relative motion grab
   * @param[in] boundary The Pointer edge boundary for grab.
   * @return True if the request was successful, false otherwise.
   */
  virtual bool RelativeMotionGrab(uint32_t boundary) = 0;

  /**
   * @brief Requests relative motion ungrab
   *
   * @return True if the request was successful, false otherwise.
   */
  virtual bool RelativeMotionUnGrab() = 0;

  /**
   * @brief Set the window's background blur.
   * If this value is 0, the background blur is disabled.
   *
   * @param[in] blurRadius The radius of the blur effect.
   * @param[in] cornerRadius The radius of the corner radius.
   */
  virtual void SetBackgroundBlur(int blurRadius, int cornerRadius) = 0;

  /**
   * @brief Returns current background blur.
   * If this value is 0, background blur is disabled.
   *
   * @return current background blur.
   */
  virtual int GetBackgroundBlur() = 0;

  /**
   * @brief Set the window's behind blur.
   * If this value is 0, the behind blur is disabled.
   *
   * @param[in] blurRadius The radius of the blur effect.
   */
  virtual void SetBehindBlur(int blurRadius) = 0;

  /**
   * @brief Returns current behind blur.
   * If this value is 0, behind blur is disabled.
   *
   * @return current behind blur.
   */
  virtual int GetBehindBlur() = 0;

  // Signals

  /**
   * @brief This signal is emitted when the window becomes iconified or deiconified.
   */
  IconifySignalType& IconifyChangedSignal();

  /**
   * @brief This signal is emitted when the window becomes maximized or unmaximized.
   */
  MaximizeSignalType& MaximizeChangedSignal();

  /**
   * @brief This signal is emitted when the window focus is changed.
   */
  FocusSignalType& FocusChangedSignal();

  /**
   * @brief This signal is emitted when the output is transformed.
   */
  OutputSignalType& OutputTransformedSignal();

  /**
   * @brief This signal is emitted when the window receives a delete request.
   */
  DeleteSignalType& DeleteRequestSignal();

  /**
   * @brief This signal is emitted when the window is damaged.
   */
  DamageSignalType& WindowDamagedSignal();

  /**
   * @brief This signal is emitted when a rotation event is recevied.
   */
  RotationSignalType& RotationSignal();

  /**
   * @brief This signal is emitted when a touch event is received.
   */
  TouchEventSignalType& TouchEventSignal();

  /**
   * @brief This signal is emitted when a mouse frame event is received.
   */
  MouseFrameEventSignalType& MouseFrameEventSignal();

  /**
   * @brief This signal is emitted when a mouse wheel is received.
   */
  WheelEventSignalType& WheelEventSignal();

  /**
   * @brief This signal is emitted when a key event is received.
   */
  KeyEventSignalType& KeyEventSignal();

  /**
   * @brief This signal is emitted when the source window notifies us the content in clipboard is selected.
   */
  SelectionSignalType& SelectionDataSendSignal();

  /**
   * @brief This signal is emitted when the source window sends us about the selected content.
   */
  SelectionSignalType& SelectionDataReceivedSignal();

  /**
   * @brief This signal is emitted when the style is changed.
   */
  StyleSignalType& StyleChangedSignal();

  /**
   * @brief This signal is emitted when window's transition animation is started or ended.
   */
  TransitionEffectEventSignalType& TransitionEffectEventSignal();

  /**
   * @brief This signal is emitted when the keyboard repeat is changed.
   */
  KeyboardRepeatSettingsChangedSignalType& KeyboardRepeatSettingsChangedSignal();

  /**
   * @brief This signal is emitted when the window redraw is requested.
   */
  WindowRedrawRequestSignalType& WindowRedrawRequestSignal();

  /**
   * @brief This signal is emitted when the window's geometry data is changed by display server or client.
   * It is based on configure noification event.
   */
  UpdatePositionSizeType& UpdatePositionSizeSignal();

  /**
   * @brief This signal is emitted when the window is received the auxiliary message from display server.
   */
  AuxiliaryMessageSignalType& AuxiliaryMessageSignal();

  /**
   * @brief This signal is emitted when a mouse in or out event is recevied.
   */
  MouseInOutEventSignalType& MouseInOutEventSignal();

  /**
   * @brief This signal is emitted when a mouse relative event is recevied.
   */
  MouseRelativeEventSignalType& MouseRelativeEventSignal();

  /**
   * @brief This signal is emitted when window has been moved by then display server.
   * To be moved the window by display server, RequestMoveToServer() should be called.
   * After the moving job is finished, this function will be called.
   */
  MoveCompletedSignalType& MoveCompletedSignal();

  /**
   * @brief This signal is emitted when window has been resized by then display server.
   * To be resized the window by display server, RequestResizeToServer() should be called.
   * After the resizing job is finished, this function will be called.
   */
  ResizeCompletedSignalType& ResizeCompletedSignal();

  /**
   * @brief This signal is emitted when window insets are changed by appearing or disappearing indicator, virtual keyboard, or clipboard.
   */
  InsetsChangedSignalType& InsetsChangedSignal();

  /**
   * @brief This signal is emitted when window pointer is locked/unlocked
   */
  PointerConstraintsSignalType& PointerConstraintsSignal();

protected:
  // Undefined
  WindowBase(const WindowBase&) = delete;

  // Undefined
  WindowBase& operator=(const WindowBase& rhs) = delete;

protected:
  IconifySignalType                       mIconifyChangedSignal;
  MaximizeSignalType                      mMaximizeChangedSignal;
  FocusSignalType                         mFocusChangedSignal;
  OutputSignalType                        mOutputTransformedSignal;
  DeleteSignalType                        mDeleteRequestSignal;
  DamageSignalType                        mWindowDamagedSignal;
  RotationSignalType                      mRotationSignal;
  TouchEventSignalType                    mTouchEventSignal;
  MouseFrameEventSignalType               mMouseFrameEventSignal;
  WheelEventSignalType                    mWheelEventSignal;
  KeyEventSignalType                      mKeyEventSignal;
  SelectionSignalType                     mSelectionDataSendSignal;
  SelectionSignalType                     mSelectionDataReceivedSignal;
  StyleSignalType                         mStyleChangedSignal;
  TransitionEffectEventSignalType         mTransitionEffectEventSignal;
  KeyboardRepeatSettingsChangedSignalType mKeyboardRepeatSettingsChangedSignal;
  WindowRedrawRequestSignalType           mWindowRedrawRequestSignal;
  UpdatePositionSizeType                  mUpdatePositionSizeSignal;
  AuxiliaryMessageSignalType              mAuxiliaryMessageSignal;
  MouseInOutEventSignalType               mMouseInOutEventSignal;
  MouseRelativeEventSignalType            mMouseRelativeEventSignal;
  MoveCompletedSignalType                 mMoveCompletedSignal;
  ResizeCompletedSignalType               mResizeCompletedSignal;
  InsetsChangedSignalType                 mInsetsChangedSignal;
  PointerConstraintsSignalType            mPointerConstraintsSignal;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_BASE_H
