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

// EXTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window-definitions.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/events/wheel-event.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/integration-api/string-utils.h>
#include <dali/internal/window-system/common/window-impl.h>

using Dali::Integration::ToDaliString;
using Dali::Integration::ToStdString;

namespace Dali
{
namespace DevelWindow
{
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

TransitionEffectEventSignalType& TransitionEffectEventSignal(Window window)
{
  return GetImplementation(window).TransitionEffectEventSignal();
}

AuxiliaryMessageSignalType& AuxiliaryMessageSignal(Window window)
{
  return GetImplementation(window).AuxiliaryMessageSignal();
}

AccessibilityHighlightSignalType& AccessibilityHighlightSignal(Window window)
{
  return GetImplementation(window).AccessibilityHighlightSignal();
}

int GetPhysicalOrientation(Window window)
{
  return GetImplementation(window).GetPhysicalOrientation();
}

void SetPositionSizeWithOrientation(Window window, PositionSize positionSize, WindowOrientation orientation)
{
  GetImplementation(window).SetPositionSizeWithOrientation(positionSize, orientation);
}

void RequestResizeToServer(Window window, WindowResizeDirection direction)
{
  GetImplementation(window).RequestResizeToServer(direction);
}

void SetNeedsRotationCompletedAcknowledgement(Window window, bool needAcknowledgement)
{
  GetImplementation(window).SetNeedsRotationCompletedAcknowledgement(needAcknowledgement);
}

void SendRotationCompletedAcknowledgement(Window window)
{
  GetImplementation(window).SendRotationCompletedAcknowledgement();
}

void MaximizeWithRestoreSize(Window window, bool maximize, Dali::Window::WindowSize size)
{
  GetImplementation(window).MaximizeWithRestoreSize(maximize, size);
}

bool PointerConstraintsLock(Window window)
{
  return GetImplementation(window).PointerConstraintsLock();
}

bool PointerConstraintsUnlock(Window window)
{
  return GetImplementation(window).PointerConstraintsUnlock();
}

void LockedPointerRegionSet(Window window, int32_t x, int32_t y, int32_t width, int32_t height)
{
  GetImplementation(window).LockedPointerRegionSet(x, y, width, height);
}

void LockedPointerCursorPositionHintSet(Window window, int32_t x, int32_t y)
{
  GetImplementation(window).LockedPointerCursorPositionHintSet(x, y);
}

bool PointerWarp(Window window, int32_t x, int32_t y)
{
  return GetImplementation(window).PointerWarp(x, y);
}

void CursorVisibleSet(Window window, bool visible)
{
  GetImplementation(window).CursorVisibleSet(visible);
}

bool KeyboardGrab(Window window, Device::Subclass::Type deviceSubclass)
{
  return GetImplementation(window).KeyboardGrab(deviceSubclass);
}

bool KeyboardUnGrab(Window window)
{
  return GetImplementation(window).KeyboardUnGrab();
}

void SetFullScreen(Window window, bool fullscreen)
{
  GetImplementation(window).SetFullScreen(fullscreen);
}

bool GetFullScreen(Window window)
{
  return GetImplementation(window).GetFullScreen();
}

void SetModal(Window window, bool modal)
{
  GetImplementation(window).SetModal(modal);
}

bool IsModal(Window window)
{
  return GetImplementation(window).IsModal();
}

void SetBottom(Window window, bool enable)
{
  GetImplementation(window).SetBottom(enable);
}

bool IsBottom(Window window)
{
  return GetImplementation(window).IsBottom();
}

Any GetNativeBuffer(Window window)
{
  return GetImplementation(window).GetNativeBuffer();
}

bool RelativeMotionGrab(Window window, uint32_t boundary)
{
  return GetImplementation(window).RelativeMotionGrab(boundary);
}

bool RelativeMotionUnGrab(Window window)
{
  return GetImplementation(window).RelativeMotionUnGrab();
}

void SetBlur(Window window, const WindowBlurInfo& blurInfo)
{
  GetImplementation(window).SetBlur(blurInfo);
}

WindowBlurInfo GetBlur(Window window)
{
  return GetImplementation(window).GetBlur();
}

Extents GetInsets(Window window)
{
  return GetImplementation(window).GetInsets();
}

Extents GetInsets(Window window, WindowInsetsPartFlags insetsFlags)
{
  return GetImplementation(window).GetInsets(insetsFlags);
}

void SetForceRendering(Window window, uint32_t frameCount)
{
  return GetImplementation(window).SetForceRendering(frameCount);
}

InterceptKeyEventSignalType& InterceptKeyEventSignal(Window window)
{
  return GetImplementation(window).InterceptKeyEventSignal();
}

MouseRelativeEventSignalType& MouseRelativeEventSignal(Window window)
{
  return GetImplementation(window).MouseRelativeEventSignal();
}

PointerConstraintsSignalType& PointerConstraintsSignal(Window window)
{
  return GetImplementation(window).PointerConstraintsSignal();
}

KeyEventSignalType& KeyEventMonitorSignal(Window window)
{
  return GetImplementation(window).KeyEventMonitorSignal();
}

} // namespace DevelWindow

} // namespace Dali
