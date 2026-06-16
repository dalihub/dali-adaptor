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
#include <dali/public-api/adaptor-framework/input-method-context.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/input-method-context-impl.h>

namespace Dali
{
InputMethodContext::InputMethodContext() = default;

InputMethodContext::~InputMethodContext() = default;

InputMethodContext::InputMethodContext(const InputMethodContext& inputMethodContext)
: BaseHandle(inputMethodContext)
{
}

InputMethodContext& InputMethodContext::operator=(const InputMethodContext& inputMethodContext)
{
  if(*this != inputMethodContext)
  {
    BaseHandle::operator=(inputMethodContext);
  }
  return *this;
}

InputMethodContext InputMethodContext::DownCast(BaseHandle handle)
{
  return InputMethodContext(dynamic_cast<Internal::Adaptor::InputMethodContext*>(handle.GetObjectPtr()));
}

bool InputMethodContext::IsRestoreAfterFocusLostEnabled() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).RestoreAfterFocusLost();
}

bool InputMethodContext::SetRestoreAfterFocusLostEnabled(bool enabled)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetRestoreAfterFocusLost(enabled);
}

BoundsInteger InputMethodContext::GetInputPanelArea()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetInputPanelArea();
}

bool InputMethodContext::SetInputPanelUserData(const Dali::String& data)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetInputPanelUserData(data);
}

Dali::String InputMethodContext::GetInputPanelUserData() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetInputPanelUserData();
}

Dali::InputMethodContext::State InputMethodContext::GetInputPanelState() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(const_cast<InputMethodContext&>(*this)).GetInputPanelState();
}

bool InputMethodContext::SetReturnKeyEnabled(bool enabled)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetReturnKeyState(enabled);
}

bool InputMethodContext::IsReturnKeyEnabled() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).IsReturnKeyEnabled();
}

bool InputMethodContext::SetInputPanelAutoShowEnabled(bool enabled)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).AutoEnableInputPanel(enabled);
}

bool InputMethodContext::ShowInputPanel()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).ShowInputPanel();
}

bool InputMethodContext::HideInputPanel()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).HideInputPanel();
}

Dali::InputMethodContext::KeyboardType InputMethodContext::GetKeyboardType() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(const_cast<InputMethodContext&>(*this)).GetKeyboardType();
}

bool InputMethodContext::SetInputPanelLanguageLocale(const Dali::String& locale)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetInputPanelLanguageLocale(locale);
}

Dali::String InputMethodContext::GetInputPanelLanguageLocale() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetInputPanelLanguageLocale();
}

bool InputMethodContext::SetTextPredictionEnabled(bool enabled)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).AllowTextPrediction(enabled);
}

bool InputMethodContext::IsTextPredictionEnabled() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).IsTextPredictionAllowed();
}

bool InputMethodContext::SetFullScreenModeEnabled(bool enabled)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetFullScreenMode(enabled);
}

bool InputMethodContext::IsFullScreenModeEnabled() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).IsFullScreenMode();
}

bool InputMethodContext::SetInputPanelPosition(uint32_t x, uint32_t y)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetInputPanelPosition(x, y);
}

bool InputMethodContext::SetInputPanelPositionAlign(int32_t x, int32_t y, Dali::InputMethodContext::InputPanelAlign align)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetInputPanelPositionAlign(x, y, align);
}

bool InputMethodContext::SetInputPanelLayout(Dali::InputMethod::PanelLayout layout)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetInputPanelLayout(layout);
}

Dali::InputMethod::PanelLayout InputMethodContext::GetInputPanelLayout() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetInputPanelLayout();
}

bool InputMethodContext::SetInputPanelReturnKeyType(Dali::InputMethod::ReturnKeyType action)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetInputPanelReturnKeyType(action);
}

Dali::InputMethod::ReturnKeyType InputMethodContext::GetInputPanelReturnKeyType() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetInputPanelReturnKeyType();
}

bool InputMethodContext::SetInputPanelAutoCapitalType(Dali::InputMethod::AutoCapitalType autoCapital)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetInputPanelAutoCapitalType(autoCapital);
}

Dali::InputMethod::AutoCapitalType InputMethodContext::GetInputPanelAutoCapitalType() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetInputPanelAutoCapitalType();
}

bool InputMethodContext::SetInputPanelLayoutVariation(Dali::InputMethod::PanelLayoutVariation variation)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetInputPanelLayoutVariation(variation);
}

Dali::InputMethod::PanelLayoutVariation InputMethodContext::GetInputPanelLayoutVariation() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetInputPanelLayoutVariation();
}

// Signals
InputMethodContext::ActivatedSignalType& InputMethodContext::ActivatedSignal()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).ActivatedSignal();
}

InputMethodContext::StatusChangedSignalType& InputMethodContext::StatusChangedSignal()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).StatusChangedSignal();
}

InputMethodContext::KeyboardResizedSignalType& InputMethodContext::ResizedSignal()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).ResizedSignal();
}

InputMethodContext::LanguageChangedSignalType& InputMethodContext::LanguageChangedSignal()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).LanguageChangedSignal();
}

InputMethodContext::KeyboardTypeChangedSignalType& InputMethodContext::KeyboardTypeChangedSignal()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).KeyboardTypeChangedSignal();
}

InputMethodContext::PrivateCommandReceivedSignalType& InputMethodContext::PrivateCommandReceivedSignal()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).PrivateCommandReceivedSignal();
}

InputMethodContext::InputMethodContext(Internal::Adaptor::InputMethodContext* impl)
: BaseHandle(impl)
{
}
} // namespace Dali
