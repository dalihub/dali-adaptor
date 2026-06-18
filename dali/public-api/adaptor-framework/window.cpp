/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <dali/public-api/adaptor-framework/window.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/string-utils.h>
#include <dali/public-api/render-tasks/render-task-list.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/public-api/actors/actor.h>

using Dali::Integration::ToDaliString;
using Dali::Integration::ToStdString;

namespace Dali
{
Window Window::New(PositionSize posSize, const Dali::String& name, bool isTransparent)
{
  WindowData windowData;
  windowData.SetPositionSize(posSize);
  windowData.SetTransparency(isTransparent);
  windowData.SetWindowType(WindowType::NORMAL);
  return Dali::Window::New(name, String(""), windowData);
}

Window Window::New(const Dali::String& name, const Dali::String& className, const WindowData& windowData)
{
  Window newWindow;

  const bool isAdaptorAvailable = Dali::Adaptor::IsAvailable();
  bool       isNewWindowAllowed = true;

  if(isAdaptorAvailable)
  {
    Dali::Adaptor& adaptor = Internal::Adaptor::Adaptor::Get();
    isNewWindowAllowed     = Internal::Adaptor::Adaptor::GetImplementation(adaptor).IsMultipleWindowSupported();
  }

  if(isNewWindowAllowed)
  {
    Any                        surface;
    Internal::Adaptor::Window* window = Internal::Adaptor::Window::New(surface, ToStdString(name), ToStdString(className), windowData, false);

    Integration::SceneHolder sceneHolder = Integration::SceneHolder(window);

    if(isAdaptorAvailable)
    {
      Dali::Adaptor& adaptor = Internal::Adaptor::Adaptor::Get();
      Internal::Adaptor::Adaptor::GetImplementation(adaptor).AddWindow(sceneHolder);
    }
    newWindow = Window(window);
  }
  else
  {
    DALI_LOG_ERROR("This device can't support multiple windows.\n");
  }

  return newWindow;
}

Window::Window()
{
}

Window::~Window()
{
}

Window::Window(const Window& copy) = default;

Window& Window::operator=(const Window& rhs) = default;

Window::Window(Window&& rhs) noexcept = default;

Window& Window::operator=(Window&& rhs) noexcept = default;

Window Window::DownCast(BaseHandle handle)
{
  return Window(dynamic_cast<Dali::Internal::Adaptor::Window*>(handle.GetObjectPtr()));
}

void Window::Add(Dali::Actor actor)
{
  GetImplementation(*this).Add(actor);
}

void Window::Remove(Dali::Actor actor)
{
  GetImplementation(*this).Remove(actor);
}

void Window::SetBackgroundColor(const Vector4& color)
{
  GetImplementation(*this).SetBackgroundColor(color);
}

Vector4 Window::GetBackgroundColor() const
{
  return GetImplementation(*this).GetBackgroundColor();
}

Layer Window::GetRootLayer() const
{
  return GetImplementation(*this).GetRootLayer();
}

Layer Window::GetOverlayLayer()
{
  return GetImplementation(*this).GetOverlayLayer();
}

uint32_t Window::GetLayerCount() const
{
  return GetImplementation(*this).GetLayerCount();
}

Layer Window::GetLayer(uint32_t depth) const
{
  return GetImplementation(*this).GetLayer(depth);
}

Uint16Pair Window::GetDpi() const
{
  return GetImplementation(*this).GetDpi();
}

void Window::SetClass(Dali::String name, Dali::String klass)
{
  GetImplementation(*this).SetClass(ToStdString(name), ToStdString(klass));
}

void Window::Raise()
{
  GetImplementation(*this).Raise();
}

void Window::Lower()
{
  GetImplementation(*this).Lower();
}

void Window::Activate()
{
  GetImplementation(*this).Activate();
}

void Window::AddAvailableOrientation(WindowOrientation orientation)
{
  GetImplementation(*this).AddAvailableOrientation(orientation);
}

void Window::RemoveAvailableOrientation(WindowOrientation orientation)
{
  GetImplementation(*this).RemoveAvailableOrientation(orientation);
}

void Window::SetPreferredOrientation(WindowOrientation orientation)
{
  GetImplementation(*this).SetPreferredOrientation(orientation);
}

WindowOrientation Window::GetPreferredOrientation()
{
  return GetImplementation(*this).GetPreferredOrientation();
}

void Window::SetAvailableOrientations(const Dali::Vector<WindowOrientation>& orientations)
{
  GetImplementation(*this).SetAvailableOrientations(orientations);
}

WindowOrientation Window::GetCurrentOrientation()
{
  return GetImplementation(*this).GetCurrentOrientation();
}

bool Window::IsOrientationChanging() const
{
  return GetImplementation(*this).IsOrientationChanging();
}

Any Window::GetNativeHandle() const
{
  return GetImplementation(*this).GetNativeHandle();
}

int32_t Window::GetNativeId() const
{
  return GetImplementation(*this).GetNativeId();
}

Window::FocusChangeSignalType& Window::FocusChangeSignal()
{
  return GetImplementation(*this).FocusChangeSignal();
}

void Window::SetAcceptFocus(bool accept)
{
  GetImplementation(*this).SetAcceptFocus(accept);
}

bool Window::IsFocusAcceptable() const
{
  return GetImplementation(*this).IsFocusAcceptable();
}

void Window::Show()
{
  GetImplementation(*this).Show();
}

void Window::Hide()
{
  GetImplementation(*this).Hide();
}

bool Window::IsVisible() const
{
  return GetImplementation(*this).IsVisible();
}

unsigned int Window::GetSupportedAuxiliaryHintCount() const
{
  return GetImplementation(*this).GetSupportedAuxiliaryHintCount();
}

Dali::String Window::GetSupportedAuxiliaryHint(unsigned int index) const
{
  return ToDaliString(GetImplementation(*this).GetSupportedAuxiliaryHint(index));
}

unsigned int Window::AddAuxiliaryHint(const Dali::String& hint, const Dali::String& value)
{
  return GetImplementation(*this).AddAuxiliaryHint(ToStdString(hint), ToStdString(value));
}

bool Window::RemoveAuxiliaryHint(unsigned int id)
{
  return GetImplementation(*this).RemoveAuxiliaryHint(id);
}

bool Window::SetAuxiliaryHintValue(unsigned int id, const Dali::String& value)
{
  return GetImplementation(*this).SetAuxiliaryHintValue(id, ToStdString(value));
}

Dali::String Window::GetAuxiliaryHintValue(unsigned int id) const
{
  return ToDaliString(GetImplementation(*this).GetAuxiliaryHintValue(id));
}

unsigned int Window::GetAuxiliaryHintId(const Dali::String& hint) const
{
  return GetImplementation(*this).GetAuxiliaryHintId(ToStdString(hint));
}

void Window::SetInputRegion(const BoundsInteger& inputRegion)
{
  return GetImplementation(*this).SetInputRegion(inputRegion);
}

void Window::IncludeInputRegion(const BoundsInteger& inputRegion)
{
  GetImplementation(*this).IncludeInputRegion(inputRegion);
}

void Window::ExcludeInputRegion(const BoundsInteger& inputRegion)
{
  GetImplementation(*this).ExcludeInputRegion(inputRegion);
}

void Window::SetType(WindowType type)
{
  GetImplementation(*this).SetType(type);
}

WindowType Window::GetType() const
{
  return GetImplementation(*this).GetType();
}

void Window::EnableFloatingMode(bool enable)
{
  GetImplementation(*this).EnableFloatingMode(enable);
}

bool Window::IsFloatingModeEnabled() const
{
  return GetImplementation(*this).IsFloatingModeEnabled();
}

WindowOperationResult Window::SetNotificationLevel(WindowNotificationLevel level)
{
  return GetImplementation(*this).SetNotificationLevel(level);
}

WindowNotificationLevel Window::GetNotificationLevel() const
{
  return GetImplementation(*this).GetNotificationLevel();
}

void Window::SetAlwaysOnTop(bool alwaysOnTop)
{
  GetImplementation(*this).SetAlwaysOnTop(alwaysOnTop);
}

bool Window::IsAlwaysOnTop() const
{
  return GetImplementation(*this).IsAlwaysOnTop();
}

void Window::SetOpaqueState(bool opaque)
{
  GetImplementation(*this).SetOpaqueState(opaque);
}

bool Window::IsOpaqueState() const
{
  return GetImplementation(*this).IsOpaqueState();
}

WindowOperationResult Window::SetScreenOffMode(WindowScreenOffMode screenMode)
{
  return GetImplementation(*this).SetScreenOffMode(screenMode);
}

WindowScreenOffMode Window::GetScreenOffMode() const
{
  return GetImplementation(*this).GetScreenOffMode();
}

void Window::SetScreen(const Dali::String& screenName)
{
  GetImplementation(*this).SetScreen(ToStdString(screenName));
}

Dali::String Window::GetScreen() const
{
  return ToDaliString(GetImplementation(*this).GetScreen());
}

void Window::FeedTouchEvent(const Dali::TouchEvent& touchEvent)
{
  Integration::Point convertedPoint;
  convertedPoint.SetDeviceId(touchEvent.GetDeviceId(0));
  convertedPoint.SetState(touchEvent.GetState(0));
  convertedPoint.SetScreenPosition(touchEvent.GetScreenPosition(0));
  convertedPoint.SetRadius(touchEvent.GetRadius(0), touchEvent.GetEllipseRadius(0));
  convertedPoint.SetPressure(touchEvent.GetPressure(0));
  convertedPoint.SetAngle(touchEvent.GetAngle(0));
  convertedPoint.SetDeviceClass(touchEvent.GetDeviceClass(0));
  convertedPoint.SetDeviceSubclass(touchEvent.GetDeviceSubclass(0));
  convertedPoint.SetMouseButton(touchEvent.GetMouseButton(0));
  Dali::String deviceName = touchEvent.GetDeviceName(0);
  convertedPoint.SetDeviceName(deviceName);
  GetImplementation(*this).FeedTouchPoint(convertedPoint, touchEvent.GetTime());
}

void Window::FeedWheelEvent(Dali::WheelEvent wheelEvent)
{
  Integration::WheelEvent convertedEvent(static_cast<Integration::WheelEvent::Type>(wheelEvent.GetType()), wheelEvent.GetDirection(), wheelEvent.GetModifiers(), wheelEvent.GetPoint(), wheelEvent.GetDelta(), wheelEvent.GetTime());
  GetImplementation(*this).FeedWheelEvent(convertedEvent);
}

void Window::FeedKeyEvent(const Dali::KeyEvent& keyEvent)
{
  Integration::KeyEvent convertedEvent(keyEvent.GetKeyName(),
                                       keyEvent.GetLogicalKey(),
                                       keyEvent.GetKeyString(),
                                       keyEvent.GetKeyCode(),
                                       keyEvent.GetKeyModifier(),
                                       keyEvent.GetTime(),
                                       static_cast<Integration::KeyEvent::State>(keyEvent.GetState()),
                                       keyEvent.GetCompose(),
                                       keyEvent.GetDeviceName(),
                                       keyEvent.GetDeviceClass(),
                                       keyEvent.GetDeviceSubclass());
  convertedEvent.receiveTime = keyEvent.GetReceiveTime();
  GetImplementation(*this).FeedKeyEvent(convertedEvent);
}

void Window::FeedHoverEvent(const Dali::HoverEvent& hoverEvent)
{
  Integration::Point convertedPoint;
  convertedPoint.SetDeviceId(hoverEvent.GetDeviceId(0));
  convertedPoint.SetState(hoverEvent.GetState(0));
  convertedPoint.SetScreenPosition(hoverEvent.GetScreenPosition(0));
  convertedPoint.SetDeviceClass(hoverEvent.GetDeviceClass(0));
  convertedPoint.SetDeviceSubclass(hoverEvent.GetDeviceSubclass(0));
  GetImplementation(*this).FeedHoverEvent(convertedPoint);
}

WindowOperationResult Window::SetBrightness(int brightness)
{
  return GetImplementation(*this).SetBrightness(brightness);
}

int Window::GetBrightness() const
{
  return GetImplementation(*this).GetBrightness();
}

Window::ResizeSignalType& Window::ResizeSignal()
{
  return GetImplementation(*this).ResizeSignal();
}

void Window::SetSize(Window::WindowSize size)
{
  GetImplementation(*this).SetSize(size);
}

Window::WindowSize Window::GetSize() const
{
  return GetImplementation(*this).GetSize();
}

void Window::SetPosition(Dali::Window::WindowPosition position)
{
  GetImplementation(*this).SetPosition(position);
}

Dali::Window::WindowPosition Window::GetPosition() const
{
  return GetImplementation(*this).GetPosition();
}

void Window::RequestMoveToServer()
{
  GetImplementation(*this).RequestMoveToServer();
}

void Window::SetLayout(unsigned int numCols, unsigned int numRows, unsigned int column, unsigned int row, unsigned int colSpan, unsigned int rowSpan)
{
  return GetImplementation(*this).SetLayout(numCols, numRows, column, row, colSpan, rowSpan);
}

void Window::SetTransparency(bool transparent)
{
  GetImplementation(*this).SetTransparency(transparent);
}

Dali::RenderTaskList Window::GetRenderTaskList()
{
  return GetImplementation(*this).GetRenderTaskList();
}

void Window::KeepRendering(float durationSeconds)
{
  GetImplementation(*this).KeepRendering(durationSeconds);
}

void Window::SetDepthBufferEnabled(bool enabled)
{
  GetImplementation(*this).SetDepthBufferEnabled(enabled);
}

bool Window::IsDepthBufferEnabled() const
{
  return GetImplementation(*this).IsDepthBufferEnabled();
}

void Window::SetStencilBufferEnabled(bool enabled)
{
  GetImplementation(*this).SetStencilBufferEnabled(enabled);
}

bool Window::IsStencilBufferEnabled() const
{
  return GetImplementation(*this).IsStencilBufferEnabled();
}

void Window::SetMultiSampledAntiAliasingEnabled(bool enabled)
{
  GetImplementation(*this).SetMultiSampledAntiAliasingEnabled(enabled);
}

bool Window::IsMultiSampledAntiAliasingEnabled() const
{
  return GetImplementation(*this).IsMultiSampledAntiAliasingEnabled();
}

void Window::SetPartialUpdateEnabled(bool enabled)
{
  GetImplementation(*this).SetPartialUpdateEnabled(enabled);
}

bool Window::IsPartialUpdateEnabled() const
{
  return GetImplementation(*this).IsPartialUpdateEnabled();
}

void Window::Maximize(bool maximize)
{
  GetImplementation(*this).Maximize(maximize);
}

bool Window::IsMaximized() const
{
  return GetImplementation(*this).IsMaximized();
}

void Window::SetMaximumSize(WindowSize size)
{
  GetImplementation(*this).SetMaximumSize(size);
}

void Window::Minimize(bool minimize)
{
  GetImplementation(*this).Minimize(minimize);
}

bool Window::IsMinimized() const
{
  return GetImplementation(*this).IsMinimized();
}

void Window::SetMinimumSize(WindowSize size)
{
  GetImplementation(*this).SetMinimumSize(size);
}

void Window::SetParent(Window parent, bool belowParent)
{
  GetImplementation(*this).SetParent(parent, belowParent);
}

void Window::Unparent()
{
  GetImplementation(*this).Unparent();
}

Window Window::GetParent()
{
  return GetImplementation(*this).GetParent();
}

void Window::AddFrameRenderedCallback(CallbackBase* callback, int32_t frameId)
{
  GetImplementation(*this).AddFrameRenderedCallback(std::unique_ptr<CallbackBase>(callback), frameId);
}

void Window::AddFramePresentedCallback(CallbackBase* callback, int32_t frameId)
{
  GetImplementation(*this).AddFramePresentedCallback(std::unique_ptr<CallbackBase>(callback), frameId);
}

Window::KeyEventSignalType& Window::KeyEventSignal()
{
  return GetImplementation(*this).KeyEventSignal();
}

Window::TouchEventSignalType& Window::TouchedSignal()
{
  return GetImplementation(*this).TouchedSignal();
}

const KeyEvent& Window::GetLastKeyEvent() const
{
  return GetImplementation(*this).GetLastKeyEvent();
}

const TouchEvent& Window::GetLastTouchEvent() const
{
  return GetImplementation(*this).GetLastTouchEvent();
}

const HoverEvent& Window::GetLastHoverEvent() const
{
  return GetImplementation(*this).GetLastHoverEvent();
}

GestureState Window::GetLastPanGestureState() const
{
  return GetImplementation(*this).GetLastPanGestureState();
}

void Window::SetFrontBufferRenderingEnabled(bool enable)
{
  GetImplementation(*this).SetFrontBufferRenderingEnabled(enable);
}

bool Window::IsFrontBufferRenderingEnabled() const
{
  return GetImplementation(*this).IsFrontBufferRenderingEnabled();
}

Window::Window(Internal::Adaptor::Window* window)
: BaseHandle(window)
{
}

} // namespace Dali
