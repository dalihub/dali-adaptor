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
#include <dali/internal/window-system/common/window-base.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/time-service.h>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
WindowBase::WindowBase()
: mIconifyChangedSignal(),
  mMaximizeChangedSignal(),
  mFocusChangedSignal(),
  mOutputTransformedSignal(),
  mDeleteRequestSignal(),
  mWindowDamagedSignal(),
  mRotationSignal(),
  mTouchEventSignal(),
  mMouseFrameEventSignal(),
  mWheelEventSignal(),
  mKeyEventSignal(),
  mSelectionDataSendSignal(),
  mSelectionDataReceivedSignal(),
  mStyleChangedSignal(),
  mTransitionEffectEventSignal(),
  mUpdatePositionSizeSignal(),
  mAuxiliaryMessageSignal(),
  mMouseInOutEventSignal(),
  mMouseRelativeEventSignal(),
  mMoveCompletedSignal(),
  mResizeCompletedSignal(),
  mInsetsChangedSignal(),
  mPointerConstraintsSignal()
{
}

WindowBase::~WindowBase()
{
}

bool WindowBase::SetMouseInputEnabled(bool enabled)
{
  if(!mMouseInputState.SetEnabled(enabled, MouseInputState::UNKNOWN_BUTTONS))
  {
    return true;
  }

  Integration::Point interruptedPoint;
  const bool         hasPoint = !enabled && mMouseInputState.TakeInterruptedPoint(interruptedPoint);
  OnMouseInputEnabledChanged(enabled);
  if(hasPoint)
  {
    // This terminal event bypasses the native input filter.
    mTouchEventSignal.Emit(interruptedPoint, TimeService::GetMilliSeconds());
    mMouseFrameEventSignal.Emit();
  }
  return true;
}

bool WindowBase::IsMouseInputEnabled() const
{
  return mMouseInputState.IsEnabled();
}

void WindowBase::OnMouseInputEnabledChanged(bool /* enabled */)
{
}

void WindowBase::EmitTouchEvent(Integration::Point& point, uint32_t timeStamp, uint32_t pressedButtons)
{
  if(mMouseInputState.OnPoint(point, pressedButtons))
  {
    mTouchEventSignal.Emit(point, timeStamp);
  }
}

void WindowBase::EmitWheelEvent(Integration::WheelEvent& wheelEvent)
{
  if(IsMouseInputEnabled())
  {
    mWheelEventSignal.Emit(wheelEvent);
  }
}

void WindowBase::EmitMouseInOutEvent(const Dali::MouseInOutEvent& event)
{
  if(IsMouseInputEnabled())
  {
    mMouseInOutEventSignal.Emit(event);
  }
}

void WindowBase::EmitMouseRelativeEvent(const Dali::DevelWindow::MouseRelativeEvent& event)
{
  if(IsMouseInputEnabled())
  {
    mMouseRelativeEventSignal.Emit(event);
  }
}

void WindowBase::SetSuppressedMouseButtons(uint32_t pressedButtons)
{
  mMouseInputState.SetSuppressedButtons(pressedButtons);
}

int32_t WindowBase::AccumulateMouseWheel(int32_t delta, bool horizontal, int32_t deltaPerStep)
{
  return mMouseInputState.OnWheel(delta, horizontal, deltaPerStep);
}

WindowBase::IconifySignalType& WindowBase::IconifyChangedSignal()
{
  return mIconifyChangedSignal;
}

WindowBase::MaximizeSignalType& WindowBase::MaximizeChangedSignal()
{
  return mMaximizeChangedSignal;
}

WindowBase::FocusSignalType& WindowBase::FocusChangedSignal()
{
  return mFocusChangedSignal;
}

WindowBase::OutputSignalType& WindowBase::OutputTransformedSignal()
{
  return mOutputTransformedSignal;
}

WindowBase::DeleteSignalType& WindowBase::DeleteRequestSignal()
{
  return mDeleteRequestSignal;
}

WindowBase::DamageSignalType& WindowBase::WindowDamagedSignal()
{
  return mWindowDamagedSignal;
}

WindowBase::RotationSignalType& WindowBase::RotationSignal()
{
  return mRotationSignal;
}

WindowBase::TouchEventSignalType& WindowBase::TouchEventSignal()
{
  return mTouchEventSignal;
}

WindowBase::MouseFrameEventSignalType& WindowBase::MouseFrameEventSignal()
{
  return mMouseFrameEventSignal;
}

WindowBase::WheelEventSignalType& WindowBase::WheelEventSignal()
{
  return mWheelEventSignal;
}

WindowBase::KeyEventSignalType& WindowBase::KeyEventSignal()
{
  return mKeyEventSignal;
}

WindowBase::SelectionSignalType& WindowBase::SelectionDataSendSignal()
{
  return mSelectionDataSendSignal;
}

WindowBase::SelectionSignalType& WindowBase::SelectionDataReceivedSignal()
{
  return mSelectionDataReceivedSignal;
}

WindowBase::StyleSignalType& WindowBase::StyleChangedSignal()
{
  return mStyleChangedSignal;
}

WindowBase::TransitionEffectEventSignalType& WindowBase::TransitionEffectEventSignal()
{
  return mTransitionEffectEventSignal;
}

WindowBase::WindowRedrawRequestSignalType& WindowBase::WindowRedrawRequestSignal()
{
  return mWindowRedrawRequestSignal;
}

WindowBase::UpdatePositionSizeType& WindowBase::UpdatePositionSizeSignal()
{
  return mUpdatePositionSizeSignal;
}

WindowBase::AuxiliaryMessageSignalType& WindowBase::AuxiliaryMessageSignal()
{
  return mAuxiliaryMessageSignal;
}

WindowBase::MouseInOutEventSignalType& WindowBase::MouseInOutEventSignal()
{
  return mMouseInOutEventSignal;
}

WindowBase::MouseRelativeEventSignalType& WindowBase::MouseRelativeEventSignal()
{
  return mMouseRelativeEventSignal;
}

WindowBase::MoveCompletedSignalType& WindowBase::MoveCompletedSignal()
{
  return mMoveCompletedSignal;
}

WindowBase::ResizeCompletedSignalType& WindowBase::ResizeCompletedSignal()
{
  return mResizeCompletedSignal;
}

WindowBase::InsetsChangedSignalType& WindowBase::InsetsChangedSignal()
{
  return mInsetsChangedSignal;
}

WindowBase::PointerConstraintsSignalType& WindowBase::PointerConstraintsSignal()
{
  return mPointerConstraintsSignal;
}

} // namespace Adaptor

} // namespace Internal

} //namespace DALI_NAMESPACE
