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
#include <dali/devel-api/adaptor-framework/input-method-context.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/input-method-context-impl.h>

#include <dali/internal/adaptor/common/adaptor-impl.h>

namespace Dali
{

InputMethodContext::InputMethodContext() = default;

InputMethodContext::~InputMethodContext() = default;

InputMethodContext InputMethodContext::New()
{
  Internal::Adaptor::InputMethodContextPtr inputMethodContext = Internal::Adaptor::InputMethodContext::New();

  if( inputMethodContext )
  {
    inputMethodContext->Initialize();
  }

  return InputMethodContext( inputMethodContext.Get() );
}

InputMethodContext::InputMethodContext( const InputMethodContext& inputMethodContext )
: BaseHandle( inputMethodContext )
{
}

InputMethodContext& InputMethodContext::operator=( const InputMethodContext& inputMethodContext )
{
 if( *this != inputMethodContext )
 {
   BaseHandle::operator=( inputMethodContext );
 }
 return *this;
}

InputMethodContext InputMethodContext::DownCast( BaseHandle handle )
{
  return InputMethodContext( dynamic_cast< Internal::Adaptor::InputMethodContext* >( handle.GetObjectPtr() ) );
}

void InputMethodContext::Finalize()
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).Finalize();
}

void InputMethodContext::Activate()
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).Activate();
}

void InputMethodContext::Deactivate()
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).Deactivate();
}

bool InputMethodContext::RestoreAfterFocusLost() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).RestoreAfterFocusLost();
}

void InputMethodContext::SetRestoreAfterFocusLost( bool toggle )
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetRestoreAfterFocusLost( toggle );
}

void InputMethodContext::Reset()
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).Reset();
}

void InputMethodContext::NotifyCursorPosition()
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).NotifyCursorPosition();
}

void InputMethodContext::SetCursorPosition( unsigned int SetCursorPosition )
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetCursorPosition( SetCursorPosition );
}

unsigned int InputMethodContext::GetCursorPosition() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetCursorPosition();
}

void InputMethodContext::SetSurroundingText( const std::string& text )
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetSurroundingText( text );
}

const std::string& InputMethodContext::GetSurroundingText() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetSurroundingText();
}

void InputMethodContext::NotifyTextInputMultiLine( bool multiLine )
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).NotifyTextInputMultiLine( multiLine );
}

InputMethodContext::TextDirection InputMethodContext::GetTextDirection()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetTextDirection();
}

Rect<int> InputMethodContext::GetInputMethodArea()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetInputMethodArea();
}

void InputMethodContext::ApplyOptions( const InputMethodOptions& options )
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).ApplyOptions( options );
}

void InputMethodContext::SetInputPanelData( const std::string& data )
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetInputPanelData( data );
}

void InputMethodContext::GetInputPanelData( std::string& data )
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetInputPanelData( data );
}

Dali::InputMethodContext::State InputMethodContext::GetInputPanelState()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetInputPanelState();
}

void InputMethodContext::SetReturnKeyState( bool visible )
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).SetReturnKeyState( visible );
}

void InputMethodContext::AutoEnableInputPanel( bool enabled )
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).AutoEnableInputPanel( enabled );
}

void InputMethodContext::ShowInputPanel()
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).ShowInputPanel();
}

void InputMethodContext::HideInputPanel()
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).HideInputPanel();
}

Dali::InputMethodContext::KeyboardType InputMethodContext::GetKeyboardType()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetKeyboardType();
}

std::string InputMethodContext::GetInputPanelLocale()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).GetInputPanelLocale();
}

bool InputMethodContext::FilterEventKey( const Dali::KeyEvent& keyEvent )
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).FilterEventKey( keyEvent );
}

void InputMethodContext::AllowTextPrediction( bool prediction )
{
  Internal::Adaptor::InputMethodContext::GetImplementation(*this).AllowTextPrediction( prediction );
}

bool InputMethodContext::IsTextPredictionAllowed() const
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).IsTextPredictionAllowed();
}

InputMethodContext::ActivatedSignalType& InputMethodContext::ActivatedSignal()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).ActivatedSignal();
}

InputMethodContext::KeyboardEventSignalType& InputMethodContext::EventReceivedSignal()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).EventReceivedSignal();
}

InputMethodContext::StatusSignalType& InputMethodContext::StatusChangedSignal()
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

InputMethodContext::KeyboardTypeSignalType& InputMethodContext::KeyboardTypeChangedSignal()
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(*this).KeyboardTypeChangedSignal();
}

InputMethodContext::InputMethodContext(Internal::Adaptor::InputMethodContext *impl)
  : BaseHandle(impl)
{
}
} // namespace Dali
