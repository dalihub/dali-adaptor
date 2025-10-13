#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_IMPL_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_IMPL_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/public-api/events/touch-event.h>
#include <dali/public-api/object/base-object.h>
#include <dali/public-api/object/property-array.h>
#include <dali/public-api/object/ref-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/integration-api/adaptor-framework/scene-holder-impl.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/window-system/common/event-handler.h>
#include <dali/public-api/adaptor-framework/key-grab.h>
#include <dali/public-api/adaptor-framework/window.h>

namespace Dali
{
class Adaptor;
class Actor;
class RenderSurfaceInterface;

namespace Internal
{
namespace Adaptor
{
class Orientation;
class WindowRenderSurface;
class WindowBase;

class Window;
using WindowPtr             = IntrusivePtr<Window>;
using OrientationPtr        = IntrusivePtr<Orientation>;
using MouseInOutEventPtr    = IntrusivePtr<Dali::DevelWindow::MouseInOutEvent>;
using MouseRelativeEventPtr = IntrusivePtr<Dali::DevelWindow::MouseRelativeEvent>;
using EventHandlerPtr       = IntrusivePtr<EventHandler>;

/**
 * Window provides a surface to render onto with orientation & indicator properties.
 */
class Window : public Dali::Internal::Adaptor::SceneHolder, public EventHandler::Observer, public ConnectionTracker
{
public:
  typedef Dali::Window::FocusChangeSignalType                        FocusChangeSignalType;
  typedef Dali::Window::ResizeSignalType                             ResizeSignalType;
  typedef Dali::DevelWindow::VisibilityChangedSignalType             VisibilityChangedSignalType;
  typedef Dali::DevelWindow::TransitionEffectEventSignalType         TransitionEffectEventSignalType;
  typedef Dali::DevelWindow::KeyboardRepeatSettingsChangedSignalType KeyboardRepeatSettingsChangedSignalType;
  typedef Dali::DevelWindow::AuxiliaryMessageSignalType              AuxiliaryMessageSignalType;
  typedef Dali::DevelWindow::AccessibilityHighlightSignalType        AccessibilityHighlightSignalType;
  typedef Dali::DevelWindow::MovedSignalType                         MovedSignalType;
  typedef Dali::DevelWindow::OrientationChangedSignalType            OrientationChangedSignalType;
  typedef Dali::DevelWindow::MouseInOutEventSignalType               MouseInOutEventSignalType;
  typedef Dali::DevelWindow::MouseRelativeEventSignalType            MouseRelativeEventSignalType;
  typedef Dali::DevelWindow::MoveCompletedSignalType                 MoveCompletedSignalType;
  typedef Dali::DevelWindow::ResizeCompletedSignalType               ResizeCompletedSignalType;
  typedef Dali::DevelWindow::InsetsChangedSignalType                 InsetsChangedSignalType;
  typedef Dali::DevelWindow::PointerConstraintsSignalType            PointerConstraintsSignalType;
  typedef Signal<void()>                                             SignalType;

  /**
   * @brief Create a new Window. This should only be called once by the Application class
   *
   * @param[in] name The window title
   * @param[in] className The window class name
   * @param[in] windowData The window data
   * @return A newly allocated Window
   */
  static Window* New(const std::string& name, const std::string& className, const WindowData& windowData);

  /**
   * @brief Create a new Window. This should only be called once by the Application class
   *
   * @param[in] surface The surface used to render on.
   * @param[in] name The window title
   * @param[in] className The window class name
   * @param[in] windowData The window data
   * @return A newly allocated Window
   */
  static Window* New(Any surface, const std::string& name, const std::string& className, const WindowData& windowData);

  /**
   * @brief Create a new Window. This should only be called once by the Application class
   *
   * @param[in] surface The surface used to render on.
   * @param[in] name The window title
   * @param[in] className The window class name
   * @param[in] windowData The window data
   * @param[in] isUsePreLoader The flag is whether this window is created by preloader process or not.
   * @return A newly allocated Window
   */
  static Window* New(Any surface, const std::string& name, const std::string& className, const WindowData& windowData, const bool isUsePreLoader);

  /**
   * @copydoc Dali::Window::SetClass()
   */
  void SetClass(std::string name, std::string className);

  /**
   * @brief Gets the window class name.
   *
   * @return The class of the window
   */
  std::string GetClassName() const;

  /**
   * @copydoc Dali::Window::Raise()
   */
  void Raise();

  /**
   * @copydoc Dali::Window::Lower()
   */
  void Lower();

  /**
   * @copydoc Dali::Window::Activate()
   */
  void Activate();

  /**
   * @copydoc Dali::DevelWindow::Maximize()
   */
  void Maximize(bool maximize);

  /**
   * @copydoc Dali::DevelWindow::IsMaximized()
   */
  bool IsMaximized() const;

  /**
   * @copydoc Dali::DevelWindow::SetMaximumSize()
   */
  void SetMaximumSize(Dali::Window::WindowSize size);

  /**
   * @copydoc Dali::DevelWindow::Minimize()
   */
  void Minimize(bool minimize);

  /**
   * @copydoc Dali::DevelWindow::IsMinimized()
   */
  bool IsMinimized() const;

  /**
   * @copydoc Dali::DevelWindow::SetMimimumSize()
   */
  void SetMimimumSize(Dali::Window::WindowSize size);

  /**
   * @copydoc Dali::Window::GetLayerCount()
   */
  uint32_t GetLayerCount() const;

  /**
   * @copydoc Dali::Window::GetLayer()
   */
  Dali::Layer GetLayer(uint32_t depth) const;

  /**
   * @copydoc Dali::Window::KeepRendering()
   */
  void KeepRendering(float durationSeconds);

  /**
   * @copydoc Dali::Window::SetPartialUpdateEnabled()
   */
  void SetPartialUpdateEnabled(bool enabled);

  /**
   * @copydoc Dali::Window::IsPartialUpdateEnabled()
   */
  bool IsPartialUpdateEnabled() const;

  /**
   * @brief Get window resource ID assigned by window manager
   *
   * @return The resource ID of the window
   */
  std::string GetNativeResourceId() const;

  /**
   * @copydoc Dali::Window::AddAvailableOrientation()
   */
  void AddAvailableOrientation(WindowOrientation orientation);

  /**
   * @copydoc Dali::Window::RemoveAvailableOrientation()
   */
  void RemoveAvailableOrientation(WindowOrientation orientation);

  /**
   * @copydoc Dali::Window::SetPreferredOrientation()
   */
  void SetPreferredOrientation(WindowOrientation orientation);

  /**
   * @copydoc Dali::Window::GetPreferredOrientation()
   */
  WindowOrientation GetPreferredOrientation();

  /**
   * @copydoc Dali::Window::SetAcceptFocus()
   */
  void SetAcceptFocus(bool accept);

  /**
   * @copydoc Dali::Window::IsFocusAcceptable()
   */
  bool IsFocusAcceptable() const;

  /**
   * @copydoc Dali::Window::Show()
   */
  void Show();

  /**
   * @copydoc Dali::Window::Hide()
   */
  void Hide();

  /**
   * @copydoc Dali::Window::GetSupportedAuxiliaryHintCount()
   */
  unsigned int GetSupportedAuxiliaryHintCount() const;

  /**
   * @copydoc Dali::Window::GetSupportedAuxiliaryHint()
   */
  std::string GetSupportedAuxiliaryHint(unsigned int index) const;

  /**
   * @copydoc Dali::Window::AddAuxiliaryHint()
   */
  unsigned int AddAuxiliaryHint(const std::string& hint, const std::string& value);

  /**
   * @copydoc Dali::Window::RemoveAuxiliaryHint()
   */
  bool RemoveAuxiliaryHint(unsigned int id);

  /**
   * @copydoc Dali::Window::SetAuxiliaryHintValue()
   */
  bool SetAuxiliaryHintValue(unsigned int id, const std::string& value);

  /**
   * @copydoc Dali::Window::GetAuxiliaryHintValue()
   */
  std::string GetAuxiliaryHintValue(unsigned int id) const;

  /**
   * @copydoc Dali::Window::GetAuxiliaryHintId()
   */
  unsigned int GetAuxiliaryHintId(const std::string& hint) const;

  /**
   * @copydoc Dali::Window::SetInputRegion()
   */
  void SetInputRegion(const Rect<int>& inputRegion);

  /**
   * @copydoc Dali::Window::SetType()
   */
  void SetType(WindowType type);

  /**
   * @copydoc Dali::Window::GetType() const
   */
  WindowType GetType() const;

  /**
   * @copydoc Dali::Window::SetNotificationLevel()
   */
  WindowOperationResult SetNotificationLevel(WindowNotificationLevel level);

  /**
   * @copydoc Dali::Window::GetNotificationLevel()
   */
  WindowNotificationLevel GetNotificationLevel() const;

  /**
   * @copydoc Dali::Window::SetOpaqueState()
   */
  void SetOpaqueState(bool opaque);

  /**
   * @copydoc Dali::Window::IsOpaqueState()
   */
  bool IsOpaqueState() const;

  /**
   * @copydoc Dali::Window::SetScreenOffMode()
   */
  WindowOperationResult SetScreenOffMode(WindowScreenOffMode screenOffMode);

  /**
   * @copydoc Dali::Window::GetScreenOffMode()
   */
  WindowScreenOffMode GetScreenOffMode() const;

  /**
   * @copydoc Dali::Window::SetBrightness()
   */
  WindowOperationResult SetBrightness(int brightness);

  /**
   * @copydoc Dali::Window::GetBrightness()
   */
  int GetBrightness() const;

  /**
   * @copydoc Dali::Window::SetSize()
   */
  void SetSize(Dali::Window::WindowSize size);

  /**
   * @copydoc Dali::Window::GetSize()
   */
  Dali::Window::WindowSize GetSize() const;

  /**
   * @copydoc Dali::Window::SetPosition()
   */
  void SetPosition(Dali::Window::WindowPosition position);

  /**
   * @copydoc Dali::Window::GetPosition()
   */
  Dali::Window::WindowPosition GetPosition() const;

  /**
   * @copydoc Dali::DevelWindow::SetPositionSize()
   */
  void SetPositionSize(PositionSize positionSize);

  /**
   * @copydoc Dali::DevelWindow::GetPositionSize()
   */
  PositionSize GetPositionSize() const;

  /**
   * @copydoc Dali::Window::SetLayout()
   */
  void SetLayout(unsigned int numCols, unsigned int numRows, unsigned int column, unsigned int row, unsigned int colSpan, unsigned int rowSpan);

  /**
   * @copydoc Dali::Window::GetRootLayer()
   */
  Dali::Layer GetRootLayer() const;

  /**
   * @copydoc Dali::Window::SetBackgroundColor
   */
  void SetBackgroundColor(const Dali::Vector4& color);

  /**
   * @copydoc Dali::Window::GetBackgroundColor
   */
  Vector4 GetBackgroundColor() const;

  /**
   * @copydoc Dali::Window::SetTransparency()
   */
  void SetTransparency(bool transparent);

  /**
   * @copydoc Dali::KeyGrab::GrabKey()
   */
  bool GrabKey(Dali::KEY key, KeyGrab::KeyGrabMode grabMode);

  /**
   * @copydoc Dali::KeyGrab::UngrabKey()
   */
  bool UngrabKey(Dali::KEY key);

  /**
   * @copydoc Dali::KeyGrab::GrabKeyList()
   */
  bool GrabKeyList(const Dali::Vector<Dali::KEY>& key, const Dali::Vector<KeyGrab::KeyGrabMode>& grabMode, Dali::Vector<bool>& result);

  /**
   * @copydoc Dali::KeyGrab::UngrabKeyList()
   */
  bool UngrabKeyList(const Dali::Vector<Dali::KEY>& key, Dali::Vector<bool>& result);

  /**
   * @copydoc Dali::DevelWindow::Get()
   */
  static Dali::Window Get(Dali::Actor actor);

  /**
   * @copydoc Dali::DevelWindow::SetParent(Window window, Window parent)
   */
  void SetParent(Dali::Window& parent);

  /**
   * @copydoc Dali::DevelWindow::SetParent(Window window, Window parent, bool belowParent)
   */
  void SetParent(Dali::Window& parent, bool belowParent);

  /**
   * @copydoc Dali::DevelWindow::Unparent()
   */
  void Unparent();

  /**
   * @copydoc Dali::DevelWindow::GetParent()
   */
  Dali::Window GetParent();

  /**
   * @copydoc Dali::DevelWindow::GetCurrentOrientation()
   */
  WindowOrientation GetCurrentOrientation() const;

  /**
   * @copydoc Dali::DevelWindow::GetPhysicalOrientation()
   */
  int GetPhysicalOrientation() const;

  /**
   * @copydoc Dali::DevelWindow::SetAvailableOrientations()
   */
  void SetAvailableOrientations(const Dali::Vector<WindowOrientation>& orientations);

  /**
   * @copydoc Dali::DevelWindow::SetPositionSizeWithOrientation()
   */
  void SetPositionSizeWithOrientation(PositionSize positionSize, WindowOrientation orientation);

  /**
   * @brief Emit the accessibility highlight signal.
   *
   * The highlight indicates that it is an object to interact with the user regardless of focus.
   * After setting the highlight on the object, you can do things that the object can do, such as
   * giving or losing focus.
   *
   * @param[in] highlight If window needs to grab or clear highlight.
   */
  void EmitAccessibilityHighlightSignal(bool highlight);

  /**
   * @brief Sets the render notification trigger to call when render thread is completed a frame
   *
   * @param[in] renderNotification to use
   */
  void SetRenderNotification(TriggerEventInterface* renderNotification);

public: // Dali::Internal::Adaptor::SceneHolder
  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::GetNativeHandle
   */
  Dali::Any GetNativeHandle() const override;

  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::IsVisible
   */
  bool IsVisible() const override;

  /**
   * @copydoc Dali::DevelWindow::GetNativeId()
   */
  int32_t GetNativeId() const;

  /**
   * @copydoc Dali::DevelWindow::RequestMoveToServer()
   */
  void RequestMoveToServer();

  /**
   * @copydoc Dali::DevelWindow::RequestResizeToServer()
   */
  void RequestResizeToServer(WindowResizeDirection direction);

  /**
   * @copydoc Dali::DevelWindow::EnableFloatingMode()
   */
  void EnableFloatingMode(bool enable);

  /**
   * @copydoc Dali::DevelWindow::IsFloatingModeEnabled()
   */
  bool IsFloatingModeEnabled();

  /**
   * @copydoc Dali::DevelWindow::IncludeInputRegion()
   */
  void IncludeInputRegion(const Rect<int>& inputRegion);

  /**
   * @copydoc Dali::DevelWindow::ExcludeInputRegion()
   */
  void ExcludeInputRegion(const Rect<int>& inputRegion);

  /**
   * @copydoc Dali::DevelWindow::SetNeedsRotationCompletedAcknowledgement()
   */
  void SetNeedsRotationCompletedAcknowledgement(bool needAcknowledgement);

  /**
   * @copydoc Dali::DevelWindow::SendRotationCompletedAcknowledgement()
   */
  void SendRotationCompletedAcknowledgement();

  /**
   * @copydoc Dali::DevelWindow::IsWindowRotating()
   */
  bool IsWindowRotating() const;

  /**
   * @copydoc Dali::DevelWindow::GetLastKeyEvent()
   */
  const Dali::KeyEvent& GetLastKeyEvent() const;

  /**
   * @copydoc Dali::DevelWindow::PointerConstraintsLock()
   */
  bool PointerConstraintsLock();

  /**
   * @copydoc Dali::DevelWindow::PointerConstraintsUnlock()
   */
  bool PointerConstraintsUnlock();

  /**
   * @copydoc Dali::DevelWindow::LockedPointerRegionSet()
   */
  void LockedPointerRegionSet(int32_t x, int32_t y, int32_t width, int32_t height);

  /**
   * @copydoc Dali::DevelWindow::LockedPointerCursorPositionHintSet()
   */
  void LockedPointerCursorPositionHintSet(int32_t x, int32_t y);

  /**
   * @copydoc Dali::DevelWindow::PointerWarp()
   */
  bool PointerWarp(int32_t x, int32_t y);

  /**
   * @copydoc Dali::DevelWindow::CursorVisibleSet()
   */
  void CursorVisibleSet(bool visible);

  /**
   * @copydoc Dali::DevelWindow::KeyboardGrab()
   */
  bool KeyboardGrab(Device::Subclass::Type deviceSubclass);

  /**
   * @copydoc Dali::DevelWindow::KeyboardUnGrab()
   */
  bool KeyboardUnGrab();

  /**
   * @copydoc Dali::DevelWindow::SetFullScreen()
   */
  void SetFullScreen(bool fullscreen);

  /**
   * @copydoc Dali::DevelWindow::GetFullScreen()
   */
  bool GetFullScreen();

  /**
   * @copydoc Dali::DevelWindow::SetFrontBufferRendering()
   */
  void SetFrontBufferRendering(bool enable);

  /**
   * @copydoc Dali::DevelWindow::GetFrontBufferRendering()
   */
  bool GetFrontBufferRendering();

  /**
   * @copydoc Dali::DevelWindow::SetModal()
   */
  void SetModal(bool modal);

  /**
   * @copydoc Dali::DevelWindow::IsModal()
   */
  bool IsModal();

  /**
   * @copydoc Dali::DevelWindow::SetAlwaysOnTop()
   */
  void SetAlwaysOnTop(bool alwaysOnTop);

  /**
   * @copydoc Dali::DevelWindow::IsAlwaysOnTop()
   */
  bool IsAlwaysOnTop();

  /**
   * @copydoc Dali::DevelWindow::SetBottom()
   */
  void SetBottom(bool enable);

  /**
   * @copydoc Dali::DevelWindow::IsBottom()
   */
  bool IsBottom();

  /**
   * @copydoc Dali::DevelWindow::GetNativeBuffer()
   */
  Dali::Any GetNativeBuffer() const;

  /**
   * @copydoc Dali::DevelWindow::RelativeMotionGrab()
   */
  bool RelativeMotionGrab(uint32_t boundary);

  /**
   * @copydoc Dali::DevelWindow::RelativeMotionUnGrab()
   */
  bool RelativeMotionUnGrab();

  /**
   * @copydoc Dali::DevelWindow::SetBlur()
   */
  void SetBlur(const WindowBlurInfo& blurInfo);

  /**
   * @copydoc Dali::DevelWindow::GetBlur()
   */
  WindowBlurInfo GetBlur() const;

  /**
   * @copydoc Dali::DevelWindow::GetInsets()
   */
  Extents GetInsets();

  /**
   * @copydoc Dali::DevelWindow::GetInsets(WindowInsetsPartFlags)
   */
  Extents GetInsets(WindowInsetsPartFlags insetsFlags);

public: // It is only for window-impl. Need not public for the others.
  /**
   * @brief Gets current window rotation angle.
   *
   * Gets current window rotation's angle is related to window rotation
   * It is just supported in window-impl
   *
   * @return A current window rotation's angle.
   */
  int GetCurrentWindowRotationAngle() const;

  /**
   * @brief Gets current screen rotation's angle.
   *
   * Gets current screen rotation's angle is related to screen rotation
   * It is just supported in window-impl
   *
   * @return A current screen rotation's angle.
   */
  int GetCurrentScreenRotationAngle() const;

  /**
   * @brief Sets user geometry flag when window's geometry is changed.
   *
   * Window is created with screen size or not.
   * If window is created with screen size or the geometry is changed by user,
   * client should inform to server setting user.geometry flag
   */
  void SetUserGeometryPolicy();

  /**
   * @brief Sets the screen for the window. It means the window is moved to the screen.
   *
   * This method sets the window to the specified screen.
   * The window will be moved to the specified screen.
   *
   * @param[in] screenName The name of the screen to set.
   */
  void SetScreen(const std::string& screenName);

  /**
   * @brief Gets the screen that the window is included.
   *
   * This method returns the name of the screen where the window is currently displayed.
   *
   * @return The screen name.
   */
  std::string GetScreen() const;

  /**
   * @brief Initialize for Ime window and surface.
   * It should be called when the window is only used for Ime keyboard window.
   */
  void InitializeImeInfo();

private:
  /**
   * @brief Enumeration for orietation mode.
   *
   * The Orientation Mode is related to screen size.
   * If screen width is longer than height, the Orientation Mode will have LANDSCAPE.
   * Otherwise screen width is shorter than height or same, the Orientation Mode will have PORTRAIT.
   */
  enum class OrientationMode
  {
    PORTRAIT = 0,
    LANDSCAPE
  };

  /**
   * Private constructor.
   * @sa Window::New()
   */
  Window();

  /**
   * Destructor
   */
  ~Window() override;

  /**
   * @brief Second stage initialization
   *
   * @param[in] surface The surface used to render on.
   * @param[in] positionSize The window's position and size in initailized time.
   * @param[in] name The window title
   * @param[in] className The window class name
   * @param[in] type window's type. Refer the WindowType in window-enumerations.h.
   * @param[in] screenName screen name to set current screen name if this string is not empty.
   * @param[in] isPreLoader The flag is whether this window is created by preloader process or not.
   */
  void Initialize(Any surface, const PositionSize& positionSize, const std::string& name, const std::string& className, WindowType type, const std::string& screenName, const bool isPreLoader);

  /**
   * @brief Called when the window becomes iconified or deiconified.
   *
   * @param[in] iconified true If the window is iconified, othewise false.
   */
  void OnIconifyChanged(bool iconified);

  /**
   * @brief Called when the window becomes maximized or unmaximized.
   *
   * @param[in] maximized true If the window is maximized, othewise false.
   */
  void OnMaximizeChanged(bool maximized);

  /**
   * @brief Called when the window focus is changed.
   *
   * @param[in] focusIn true If the window is focused, othewise false.
   */
  void OnFocusChanged(bool focusIn);

  /**
   * @brief Called when the output is transformed.
   *
   * @param screenRotationAngle The new screen rotation angle.
   */
  void OnOutputTransformed(int screenRotationAngle);

  /**
   * @brief Called when the window receives a delete request.
   */
  void OnDeleteRequest();

  /**
   * @brief Called when the window receives a Transition effect-start/end event.
   *
   * This event is emitted by display server.
   *
   * @param[in] state current window transition effect state, refer the WindowEffectState in window-enumerations.h.
   * @param[in] type current window transition effect type, refer the WindowEffectType in window-enumerations.h.
   */
  void OnTransitionEffectEvent(WindowEffectState state, WindowEffectType type);

  /**
   * @brief Called when window receives a keyboard repeat event.
   */
  void OnKeyboardRepeatSettingsChanged();

  /**
   * @brief Called when the window redraw is requested.
   */
  void OnWindowRedrawRequest();

  /**
   * @brief Called when the window's geometry data is changed by display server or client.
   * It is based on configure noification event.
   *
   * @param[in] positionSize the updated window's position and size.
   */
  void OnUpdatePositionSize(Dali::PositionSize& positionSize);

  /**
   * @brief Called when display server sent the auxiliary message.
   *
   * @param[in] key the auxiliary message's key.
   * @param[in] value the auxiliary message's value.
   * @param[in] options the auxiliary message's options. This is the list of string.
   */
  void OnAuxiliaryMessage(const std::string& key, const std::string& value, const Property::Array& options);

  /**
   * @brief Called when Accessibility is enabled.
   *
   * This method is to register the window to accessibility bridge.
   */
  void OnAccessibilityEnabled();

  /**
   * @brief Called when Accessibility is disabled.
   *
   * This method is to remove the window from accessibility bridge.
   */
  void OnAccessibilityDisabled();

  /**
   * @brief Called in Accessibility mode on every KeyEvent
   *
   * @param[in] keyEvent The key event
   * @return Always true, meaning that the event is consumed
   */
  bool OnAccessibilityInterceptKeyEvent(const Dali::KeyEvent& keyEvent);

  /**
   * @brief Called when the window rotation is finished.
   *
   * This signal is emmit when window rotation is finisehd and WindowRotationCompleted() is called.
   */
  void OnRotationFinished();

  /**
   * @brief Called when the mouse in or out event is received.
   *
   * @param[in] mouseInOutEvent the mouse event
   */
  void OnMouseInOutEvent(const Dali::DevelWindow::MouseInOutEvent& mouseInOutEvent);

  /**
   * @brief Called when the mouse relative event is received.
   *
   * @param[in] mouseRelativeEvent the mouse event
   */
  void OnMouseRelativeEvent(const Dali::DevelWindow::MouseRelativeEvent& mouseRelativeEvent);

  /**
   * @brief Called when the pointer is locked/unlocked
   *
   * @param[in] position The x, y coordinate relative to window where event happened
   * @param[in] locked The status whether pointer is locked/unlocked
   * @param[in] confined The status whether pointer is confined/unconfined
   */
  void OnPointerConstraints(const Dali::Int32Pair& position, bool locked, bool confined);

  /**
   * @brief Called when the window is moved by display server.
   *
   * @param[in] position the moved window's position.
   */
  void OnMoveCompleted(Dali::Window::WindowPosition& position);

  /**
   * @brief Called when the window is resized by display server.
   *
   * @param[in] positionSize the resized window's size.
   */
  void OnResizeCompleted(Dali::Window::WindowSize& size);

  /**
   * @brief Set available orientation to window base.
   */
  void SetAvailableAnlges(const std::vector<int>& angles);

  /**
   * @brief Convert from window orientation to angle using OrientationMode.
   */
  int ConvertToAngle(WindowOrientation orientation);

  /**
   * @brief Convert from angle to window orientation using OrientationMode.
   */
  WindowOrientation ConvertToOrientation(int angle) const;

  /**
   * @brief Check available window orientation for Available orientation.
   */
  bool IsOrientationAvailable(WindowOrientation orientation) const;

  /**
   * @brief Called when window insets are changed by appearing or disappearing indicator, virtual keyboard, or clipboard.
   *
   * @param[in] partType the type of the part that occurs the window insets change.
   * @param[in] partState the state of the part that occurs the window insets change.
   * @param[in] insets the extents value of window insets.
   */
  void OnInsetsChanged(WindowInsetsPartType partType, WindowInsetsPartState partState, const Extents& insets);

  /**
   * @brief Window's position and size are updated.
   * It is called when window's geometry is changed by display server or client.
   *
   * By server, the related events are window configure notification, window move completed and window resize completed event.
   * When window configure notification event is emitted, window resize and move signals are emitted to application.
   *
   * When window move or resize completed event is emitted, window move or resize signal is emitted to application, too.
   *
   * By client, application calls SetPositionSize function.
   * It means application requests to change window's geometry to display server.
   * Window resize and move signals are emitted to application.
   *
   * @param[in] positionSize the updated window's position and size.
   * @param[in] requestChangeGeometry the flag of whether requests to change window geometry to display server or just updating local geometry information.
   */
  void UpdatePositionSize(Dali::PositionSize& positionSize, bool requestChangeGeometry);

  /**
   * @brief Reset input region when window insets are changed.
   *
   * @param[in] inputRegion the input region
   */
  void ResetInput(const Rect<int>& inputRegion);

private: // Dali::Internal::Adaptor::SceneHolder
  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::OnAdaptorSet
   */
  void OnAdaptorSet(Dali::Adaptor& adaptor) override;

  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::OnSurfaceSet
   */
  void OnSurfaceSet(Dali::Integration::RenderSurfaceInterface* surface) override;

  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::OnPause
   */
  void OnPause() override;

  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::OnResume
   */
  void OnResume() override;

  /**
   * @copydoc Dali::Internal::Adaptor::SceneHolder::RecalculatePosition
   */
  Vector2 RecalculatePosition(const Vector2& position) override;

private: // Dali::Internal::Adaptor::EventHandler::Observer
  /**
   * @copydoc Dali::Internal::Adaptor::EventHandler::Observer::OnTouchPoint
   */
  void OnTouchPoint(Dali::Integration::Point& point, int timeStamp) override;

  /**
   * @copydoc Dali::Internal::Adaptor::EventHandler::Observer::OnMouseFrameEvent
   */
  void OnMouseFrameEvent() override;

  /**
   * @copydoc Dali::Internal::Adaptor::EventHandler::Observer::OnWheelEvent
   */
  void OnWheelEvent(Dali::Integration::WheelEvent& wheelEvent) override;

  /**
   * @copydoc Dali::Internal::Adaptor::EventHandler::Observer::OnKeyEvent
   */
  void OnKeyEvent(Dali::Integration::KeyEvent& keyEvent) override;

  /**
   * @copydoc Dali::Internal::Adaptor::EventHandler::Observer::OnRotation
   */
  void OnRotation(const RotationEvent& rotation) override;

public: // Signals
  /**
   * @copydoc Dali::Window::FocusChangeSignal()
   */
  FocusChangeSignalType& FocusChangeSignal()
  {
    return mFocusChangeSignal;
  }

  /**
   * @copydoc Dali::Window::ResizedSignal()
   */
  ResizeSignalType& ResizeSignal()
  {
    return mResizeSignal;
  }

  /**
   * @brief This signal is emitted when the window is requesting to be deleted
   *
   * It is internal signal in Dali-adaptor.
   */
  SignalType& DeleteRequestSignal()
  {
    return mDeleteRequestSignal;
  }

  /**
   * @copydoc Dali::DevelWindow::VisibilityChangedSignal()
   */
  VisibilityChangedSignalType& VisibilityChangedSignal()
  {
    return mVisibilityChangedSignal;
  }

  /**
   * @copydoc Dali::Window::SignalEventProcessingFinished()
   */
  Dali::DevelWindow::EventProcessingFinishedSignalType& EventProcessingFinishedSignal()
  {
    return mScene.EventProcessingFinishedSignal();
  }

  /**
   * @copydoc Dali::DevelWindow::TransitionEffectEventSignal()
   */
  TransitionEffectEventSignalType& TransitionEffectEventSignal()
  {
    return mTransitionEffectEventSignal;
  }

  /**
   * @copydoc Dali::DevelWindow::KeyboardRepeatSettingsChangedSignal()
   */
  KeyboardRepeatSettingsChangedSignalType& KeyboardRepeatSettingsChangedSignal()
  {
    return mKeyboardRepeatSettingsChangedSignal;
  }

  /**
   * @copydoc Dali::DevelWindow::AuxiliaryMessageSignal()
   */
  AuxiliaryMessageSignalType& AuxiliaryMessageSignal()
  {
    return mAuxiliaryMessageSignal;
  }

  /**
   * @copydoc Dali::DevelWindow::AccessibilityHighlightSignal()
   */
  AccessibilityHighlightSignalType& AccessibilityHighlightSignal()
  {
    return mAccessibilityHighlightSignal;
  }

  /**
   * @copydoc Dali::DevelWindow::MovedSignal()
   */
  MovedSignalType& MovedSignal()
  {
    return mMovedSignal;
  }

  /**
   * @copydoc Dali::DevelWindow::OrientationChangedSignal()
   */
  OrientationChangedSignalType& OrientationChangedSignal()
  {
    return mOrientationChangedSignal;
  }

  /**
   * @copydoc Dali::DevelWindow::MouseInOutEventSignal()
   */
  MouseInOutEventSignalType& MouseInOutEventSignal()
  {
    return mMouseInOutEventSignal;
  }

  /**
   * @copydoc Dali::DevelWindow::MouseRelativeEventSignal()
   */
  MouseRelativeEventSignalType& MouseRelativeEventSignal()
  {
    return mMouseRelativeEventSignal;
  }

  /**
   * @copydoc Dali::DevelWindow::PointerConstraintsSignal()
   */
  PointerConstraintsSignalType& PointerConstraintsSignal()
  {
    return mPointerConstraintsSignal;
  }

  /**
   * @copydoc Dali::DevelWindow::MoveCompletedSignal()
   */
  MoveCompletedSignalType& MoveCompletedSignal()
  {
    return mMoveCompletedSignal;
  }

  /**
   * @copydoc Dali::DevelWindow::ResizeCompletedSignal()
   */
  ResizeCompletedSignalType& ResizeCompletedSignal()
  {
    return mResizeCompletedSignal;
  }

  /**
   * @copydoc Dali::DevelWindow::InsetsChangedSignal()
   */
  InsetsChangedSignalType& InsetsChangedSignal()
  {
    return mInsetsChangedSignal;
  }

private:
  WindowRenderSurface* mWindowSurface; ///< The window rendering surface
  WindowBase*          mWindowBase;
  std::string          mName;
  std::string          mClassName;
  Dali::Window         mParentWindow;

  OrientationPtr   mOrientation;
  std::vector<int> mAvailableAngles;
  int              mPreferredAngle;

  int mRotationAngle;  ///< The angle of the rotation
  int mWindowWidth;    ///< The width of the window
  int mWindowHeight;   ///< The height of the window
  int mNativeWindowId; ///< The Native Window Id

  EventHandlerPtr mEventHandler;    ///< The window events handler
  OrientationMode mOrientationMode; ///< The physical screen mode is portrait or landscape

  // Signals
  SignalType                              mDeleteRequestSignal;
  FocusChangeSignalType                   mFocusChangeSignal;
  ResizeSignalType                        mResizeSignal;
  VisibilityChangedSignalType             mVisibilityChangedSignal;
  TransitionEffectEventSignalType         mTransitionEffectEventSignal;
  KeyboardRepeatSettingsChangedSignalType mKeyboardRepeatSettingsChangedSignal;
  AuxiliaryMessageSignalType              mAuxiliaryMessageSignal;
  AccessibilityHighlightSignalType        mAccessibilityHighlightSignal;
  MovedSignalType                         mMovedSignal;
  OrientationChangedSignalType            mOrientationChangedSignal;
  MouseInOutEventSignalType               mMouseInOutEventSignal;
  MouseRelativeEventSignalType            mMouseRelativeEventSignal;
  MoveCompletedSignalType                 mMoveCompletedSignal;
  ResizeCompletedSignalType               mResizeCompletedSignal;
  InsetsChangedSignalType                 mInsetsChangedSignal;
  PointerConstraintsSignalType            mPointerConstraintsSignal;

  Dali::KeyEvent mLastKeyEvent;

  Vector4              mBackgroundColor;
  Dali::WindowBlurInfo mBlurInfo;

  bool mIsTransparent : 1;
  bool mIsFocusAcceptable : 1;
  bool mIconified : 1;
  bool mMaximized : 1;
  bool mOpaqueState : 1;
  bool mWindowRotationAcknowledgement : 1;
  bool mFocused : 1;
  bool mIsWindowRotating : 1;            ///< The window rotating flag.
  bool mIsEnabledUserGeometry : 1;       ///< The user geometry enable flag.
  bool mIsEmittedWindowCreatedEvent : 1; ///< The Window Created Event emit flag for accessibility.
  bool mIsFrontBufferRendering : 1;      ///< The Front Buffer Rendering state.
  bool mIsUsePreLoader : 1;              ///< The flag is whether is created by preloader process or not.
};

} // namespace Adaptor
} // namespace Internal

// Helpers for public-api forwarding methods

inline Internal::Adaptor::Window& GetImplementation(Dali::Window& window)
{
  DALI_ASSERT_ALWAYS(window && "Window handle is empty");
  BaseObject& object = window.GetBaseObject();
  return static_cast<Internal::Adaptor::Window&>(object);
}

inline const Internal::Adaptor::Window& GetImplementation(const Dali::Window& window)
{
  DALI_ASSERT_ALWAYS(window && "Window handle is empty");
  const BaseObject& object = window.GetBaseObject();
  return static_cast<const Internal::Adaptor::Window&>(object);
}

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_WINDOW_IMPL_H
