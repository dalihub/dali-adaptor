/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

WindowBase::WindowBase()
: mIconifyChangedSignal(),
  mFocusChangedSignal(),
  mOutputTransformedSignal(),
  mDeleteRequestSignal(),
  mWindowDamagedSignal(),
  mRotationSignal(),
  mTouchEventSignal(),
  mWheelEventSignal(),
  mKeyEventSignal(),
  mSelectionDataSendSignal(),
  mSelectionDataReceivedSignal(),
  mStyleChangedSignal(),
  mAccessibilitySignal(),
  mTransitionEffectEventSignal()
{
}

WindowBase::~WindowBase()
{
}

WindowBase::IconifySignalType& WindowBase::IconifyChangedSignal()
{
  return mIconifyChangedSignal;
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

WindowBase::AccessibilitySignalType& WindowBase::AccessibilitySignal()
{
  return mAccessibilitySignal;
}

WindowBase::TransitionEffectEventSignalType& WindowBase::TransitionEffectEventSignal()
{
  return mTransitionEffectEventSignal;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
