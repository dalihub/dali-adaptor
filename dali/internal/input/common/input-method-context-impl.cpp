/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

#include <dali/devel-api/common/singleton-service.h>
#include <dali/devel-api/object/type-registry.h>
#include <dali/integration-api/adaptor-framework/input-method-context-integ.h>
#include <dali/internal/input/common/input-method-context-factory.h>
#include <dali/internal/input/common/input-method-context-impl.h>
#include <dali/internal/system/common/locale-utils.h>

namespace
{

Dali::BaseHandle Create()
{
  return Dali::Integration::InputMethodContext::New(Dali::Actor());
}

Dali::TypeRegistration type(typeid(Dali::InputMethodContext), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
InputMethodContextPtr InputMethodContext::New(Dali::Actor actor)
{
  return Dali::Internal::Adaptor::InputMethodContextFactory::CreateInputMethodContext(actor);
}

Dali::String InputMethodContext::GetSurroundingText() const
{
  return Dali::String();
}

InputMethodContext::InputMethodContext()
: mBackupOperations(Operation::MAX_COUNT)
{
}

void InputMethodContext::ApplyBackupOperations()
{
  // Items in mBackupOperations will be changed while the iteration
  OperationList copiedList = mBackupOperations;

  for(auto& operation : copiedList)
  {
    if(operation)
    {
      operation();
    }
  }
}

// Signal accessors - public / integration

InputMethodContext::ActivatedSignalType& InputMethodContext::ActivatedSignal()
{
  return mActivatedSignal;
}

InputMethodContext::KeyboardEventSignalType& InputMethodContext::EventReceivedSignal()
{
  return mEventSignal;
}

InputMethodContext::KeyboardEventSignalType& InputMethodContext::KeyboardEventReceivedSignal()
{
  return mKeyboardEventSignal;
}

InputMethodContext::StatusChangedSignalType& InputMethodContext::StatusChangedSignal()
{
  return mKeyboardStatusSignal;
}

InputMethodContext::KeyboardResizedSignalType& InputMethodContext::ResizedSignal()
{
  return mKeyboardResizeSignal;
}

InputMethodContext::LanguageChangedSignalType& InputMethodContext::LanguageChangedSignal()
{
  return mKeyboardLanguageChangedSignal;
}

InputMethodContext::KeyboardTypeChangedSignalType& InputMethodContext::KeyboardTypeChangedSignal()
{
  return mKeyboardTypeChangedSignal;
}

InputMethodContext::ContentReceivedSignalType& InputMethodContext::ContentReceivedSignal()
{
  return mContentReceivedSignal;
}

InputMethodContext::PrivateCommandReceivedSignalType& InputMethodContext::PrivateCommandReceivedSignal()
{
  return mPrivateCommandReceivedSignal;
}

// Signal accessors - legacy

InputMethodContext::LegacyStatusChangedSignalType& InputMethodContext::LegacyStatusChangedSignal()
{
  return mLegacyKeyboardStatusSignal;
}

InputMethodContext::LegacyKeyboardResizedSignalType& InputMethodContext::LegacyResizedSignal()
{
  return mLegacyKeyboardResizeSignal;
}

InputMethodContext::LegacyLanguageChangedSignalType& InputMethodContext::LegacyLanguageChangedSignal()
{
  return mLegacyKeyboardLanguageChangedSignal;
}

InputMethodContext::LegacyKeyboardTypeChangedSignalType& InputMethodContext::LegacyKeyboardTypeChangedSignal()
{
  return mLegacyKeyboardTypeChangedSignal;
}

InputMethodContext::LegacyContentReceivedSignalType& InputMethodContext::LegacyContentReceivedSignal()
{
  return mLegacyContentReceivedSignal;
}

// Emit helpers

void InputMethodContext::EmitStatusChangedSignal(Dali::InputMethodContext::State state)
{
  Dali::InputMethodContext handle(this);

  mKeyboardStatusSignal.Emit(handle, state);

  if(state == Dali::InputMethodContext::State::SHOW)
  {
    mLegacyKeyboardStatusSignal.Emit(true);
  }
  else if(state == Dali::InputMethodContext::State::HIDE)
  {
    mLegacyKeyboardStatusSignal.Emit(false);
  }
  // Legacy bool status keeps the previous behavior: emit only SHOW/HIDE.
}

void InputMethodContext::EmitKeyboardResizedSignal(int value)
{
  Dali::InputMethodContext handle(this);

  mKeyboardResizeSignal.Emit(handle);
  mLegacyKeyboardResizeSignal.Emit(value);
}

void InputMethodContext::EmitLanguageChangedSignal(int value)
{
  Dali::InputMethodContext handle(this);

  mKeyboardLanguageChangedSignal.Emit(handle);
  mLegacyKeyboardLanguageChangedSignal.Emit(value);
}

void InputMethodContext::EmitKeyboardTypeChangedSignal(Dali::InputMethodContext::KeyboardType keyboardType)
{
  Dali::InputMethodContext handle(this);

  mKeyboardTypeChangedSignal.Emit(handle, keyboardType);
  mLegacyKeyboardTypeChangedSignal.Emit(keyboardType);
}

void InputMethodContext::EmitContentReceivedSignal(const Dali::String& content, const Dali::String& description, const Dali::String& mimeTypes)
{
  Dali::InputMethodContext handle(this);
  Dali::Integration::InputMethodContext::ContentReceivedData data(content, description, mimeTypes);

  mContentReceivedSignal.Emit(handle, data);
  mLegacyContentReceivedSignal.Emit(content.CStr(), description.CStr(), mimeTypes.CStr());
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
