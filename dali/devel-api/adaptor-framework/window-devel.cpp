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

// EXTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/events/wheel-event.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/internal/window-system/common/window-impl.h>

namespace Dali
{
namespace DevelWindow
{
Window New(Any surface, PositionSize windowPosition, const std::string& name, bool isTransparent)
{
  return DevelWindow::New(surface, windowPosition, name, "", isTransparent);
}

Window New(Any surface, PositionSize windowPosition, const std::string& name, const std::string& className, bool isTransparent)
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
    Internal::Adaptor::Window* window = Internal::Adaptor::Window::New(surface, windowPosition, name, className, WindowType::NORMAL, isTransparent);

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

void SetPositionSize(Window window, PositionSize positionSize)
{
  GetImplementation(window).SetPositionSize(positionSize);
}

Window Get(Actor actor)
{
  return Internal::Adaptor::Window::Get(actor);
}

EventProcessingFinishedSignalType& EventProcessingFinishedSignal(Window window)
{
  return GetImplementation(window).EventProcessingFinishedSignal();
}

WheelEventSignalType& WheelEventSignal(Window window)
{
  return GetImplementation(window).WheelEventSignal();
}

VisibilityChangedSignalType& VisibilityChangedSignal(Window window)
{
  return GetImplementation(window).VisibilityChangedSignal();
}

TransitionEffectEventSignalType& TransitionEffectEventSignal(Window window)
{
  return GetImplementation(window).TransitionEffectEventSignal();
}

KeyboardRepeatSettingsChangedSignalType& KeyboardRepeatSettingsChangedSignal(Window window)
{
  return GetImplementation(window).KeyboardRepeatSettingsChangedSignal();
}

AuxiliaryMessageSignalType& AuxiliaryMessageSignal(Window window)
{
  return GetImplementation(window).AuxiliaryMessageSignal();
}

AccessibilityHighlightSignalType& AccessibilityHighlightSignal(Window window)
{
  return GetImplementation(window).AccessibilityHighlightSignal();
}

MovedSignalType& MovedSignal(Window window)
{
  return GetImplementation(window).MovedSignal();
}

OrientationChangedSignalType& OrientationChangedSignal(Window window)
{
  return GetImplementation(window).OrientationChangedSignal();
}

MoveCompletedSignalType& MoveCompletedSignal(Window window)
{
  return GetImplementation(window).MoveCompletedSignal();
}

ResizeCompletedSignalType& ResizeCompletedSignal(Window window)
{
  return GetImplementation(window).ResizeCompletedSignal();
}

void SetParent(Window window, Window parent)
{
  GetImplementation(window).SetParent(parent);
}

void SetParent(Window window, Window parent, bool belowParent)
{
  GetImplementation(window).SetParent(parent, belowParent);
}

void Unparent(Window window)
{
  GetImplementation(window).Unparent();
}

Window GetParent(Window window)
{
  return GetImplementation(window).GetParent();
}

WindowOrientation GetCurrentOrientation(Window window)
{
  return GetImplementation(window).GetCurrentOrientation();
}

int GetPhysicalOrientation(Window window)
{
  return GetImplementation(window).GetPhysicalOrientation();
}

void SetAvailableOrientations(Window window, const Dali::Vector<WindowOrientation>& orientations)
{
  GetImplementation(window).SetAvailableOrientations(orientations);
}

int32_t GetNativeId(Window window)
{
  return GetImplementation(window).GetNativeId();
}

void AddFrameRenderedCallback(Window window, std::unique_ptr<CallbackBase> callback, int32_t frameId)
{
  GetImplementation(window).AddFrameRenderedCallback(std::move(callback), frameId);
}

void AddFramePresentedCallback(Window window, std::unique_ptr<CallbackBase> callback, int32_t frameId)
{
  GetImplementation(window).AddFramePresentedCallback(std::move(callback), frameId);
}

void SetPositionSizeWithOrientation(Window window, PositionSize positionSize, WindowOrientation orientation)
{
  GetImplementation(window).SetPositionSizeWithOrientation(positionSize, orientation);
}

void RequestMoveToServer(Window window)
{
  GetImplementation(window).RequestMoveToServer();
}

void RequestResizeToServer(Window window, WindowResizeDirection direction)
{
  GetImplementation(window).RequestResizeToServer(direction);
}

void EnableFloatingMode(Window window, bool enable)
{
  GetImplementation(window).EnableFloatingMode(enable);
}

void IncludeInputRegion(Window window, const Rect<int>& inputRegion)
{
  GetImplementation(window).IncludeInputRegion(inputRegion);
}

void ExcludeInputRegion(Window window, const Rect<int>& inputRegion)
{
  GetImplementation(window).ExcludeInputRegion(inputRegion);
}

void SetNeedsRotationCompletedAcknowledgement(Window window, bool needAcknowledgement)
{
  GetImplementation(window).SetNeedsRotationCompletedAcknowledgement(needAcknowledgement);
}

void SendRotationCompletedAcknowledgement(Window window)
{
  GetImplementation(window).SendRotationCompletedAcknowledgement();
}

void FeedTouchPoint(Window window, const Dali::TouchPoint& point, int32_t timeStamp)
{
  Integration::Point convertedPoint(point);
  GetImplementation(window).FeedTouchPoint(convertedPoint, timeStamp);
}

void FeedWheelEvent(Window window, const Dali::WheelEvent& wheelEvent)
{
  Integration::WheelEvent convertedEvent(static_cast<Integration::WheelEvent::Type>(wheelEvent.GetType()), wheelEvent.GetDirection(), wheelEvent.GetModifiers(), wheelEvent.GetPoint(), wheelEvent.GetDelta(), wheelEvent.GetTime());
  GetImplementation(window).FeedWheelEvent(convertedEvent);
}

void FeedKeyEvent(Window window, const Dali::KeyEvent& keyEvent)
{
  Integration::KeyEvent convertedEvent(keyEvent.GetKeyName(), keyEvent.GetLogicalKey(), keyEvent.GetKeyString(), keyEvent.GetKeyCode(), keyEvent.GetKeyModifier(), keyEvent.GetTime(), static_cast<Integration::KeyEvent::State>(keyEvent.GetState()), keyEvent.GetCompose(), keyEvent.GetDeviceName(), keyEvent.GetDeviceClass(), keyEvent.GetDeviceSubclass());
  GetImplementation(window).FeedKeyEvent(convertedEvent);
}

void Maximize(Window window, bool maximize)
{
  GetImplementation(window).Maximize(maximize);
}

bool IsMaximized(Window window)
{
  return GetImplementation(window).IsMaximized();
}

void SetMaximumSize(Window window, Dali::Window::WindowSize size)
{
  GetImplementation(window).SetMaximumSize(size);
}

void Minimize(Window window, bool miniimize)
{
  GetImplementation(window).Minimize(miniimize);
}

bool IsMinimized(Window window)
{
  return GetImplementation(window).IsMinimized();
}

void SetMimimumSize(Window window, Dali::Window::WindowSize size)
{
  GetImplementation(window).SetMimimumSize(size);
}

bool IsWindowRotating(Window window)
{
  return GetImplementation(window).IsWindowRotating();
}

const KeyEvent& GetLastKeyEvent(Window window)
{
  return GetImplementation(window).GetLastKeyEvent();
}

const TouchEvent& GetLastTouchEvent(Window window)
{
  return GetImplementation(window).GetLastTouchEvent();
}

InterceptKeyEventSignalType& InterceptKeyEventSignal(Window window)
{
  return GetImplementation(window).InterceptKeyEventSignal();
}

MouseInOutEventSignalType& MouseInOutEventSignal(Window window)
{
  return GetImplementation(window).MouseInOutEventSignal();
}

DeviceInfoEventSignalType& DeviceInfoEventSignal(Window window)
{
  return GetImplementation(window).DeviceInfoEventSignal();
}

} // namespace DevelWindow

} // namespace Dali
