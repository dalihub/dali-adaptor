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
#include <dali/integration-api/adaptor-framework/input-method-context-integ.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/input-method-context-impl.h>

namespace Dali
{
namespace Integration
{
namespace InputMethodContext
{
Dali::InputMethodContext New()
{
  return New(Dali::Actor());
}

Dali::InputMethodContext New(Dali::Actor actor)
{
  Internal::Adaptor::InputMethodContextPtr inputMethodContext = Internal::Adaptor::InputMethodContext::New(actor);

  if(inputMethodContext)
  {
    inputMethodContext->Initialize();
  }

  return Dali::InputMethodContext(inputMethodContext.Get());
}

void Finalize(Dali::InputMethodContext context)
{
  Internal::Adaptor::InputMethodContext::GetImplementation(context).Finalize();
}

void Activate(Dali::InputMethodContext context)
{
  Internal::Adaptor::InputMethodContext::GetImplementation(context).Activate();
}

void Deactivate(Dali::InputMethodContext context)
{
  Internal::Adaptor::InputMethodContext::GetImplementation(context).Deactivate();
}

void Reset(Dali::InputMethodContext context)
{
  Internal::Adaptor::InputMethodContext::GetImplementation(context).Reset();
}

void NotifyCursorPosition(Dali::InputMethodContext context)
{
  Internal::Adaptor::InputMethodContext::GetImplementation(context).NotifyCursorPosition();
}

void SetCursorPosition(Dali::InputMethodContext context, uint32_t cursorPosition)
{
  Internal::Adaptor::InputMethodContext::GetImplementation(context).SetCursorPosition(cursorPosition);
}

uint32_t GetCursorPosition(Dali::InputMethodContext context)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(context).GetCursorPosition();
}

void SetSurroundingText(Dali::InputMethodContext context, const Dali::String& text)
{
  Internal::Adaptor::InputMethodContext::GetImplementation(context).SetSurroundingText(text);
}

Dali::String GetSurroundingText(Dali::InputMethodContext context)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(context).GetSurroundingText();
}

void NotifyTextInputMultiLine(Dali::InputMethodContext context, bool multiLine)
{
  Internal::Adaptor::InputMethodContext::GetImplementation(context).NotifyTextInputMultiLine(multiLine);
}

TextDirection GetTextDirection(Dali::InputMethodContext context)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(context).GetTextDirection();
}

void SetContentMimeTypes(Dali::InputMethodContext context, const Dali::String& mimeTypes)
{
  Internal::Adaptor::InputMethodContext::GetImplementation(context).SetContentMimeTypes(mimeTypes);
}

void ApplyOptions(Dali::InputMethodContext context, const Dali::Integration::InputMethodOptions& options)
{
  Internal::Adaptor::InputMethodContext::GetImplementation(context).ApplyOptions(options);
}

bool SetInputPanelLanguage(Dali::InputMethodContext context, InputPanelLanguage language)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(context).SetInputPanelLanguage(language);
}

InputPanelLanguage GetInputPanelLanguage(Dali::InputMethodContext context)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(context).GetInputPanelLanguage();
}

bool FilterEventKey(Dali::InputMethodContext context, const Dali::KeyEvent& keyEvent)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(context).FilterEventKey(keyEvent);
}

void GetPreeditStyle(Dali::InputMethodContext context, PreEditAttributeDataContainer& attrs)
{
  Internal::Adaptor::InputMethodContext::GetImplementation(context).GetPreeditStyle(attrs);
}

KeyboardEventSignalType& EventReceivedSignal(Dali::InputMethodContext context)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(context).EventReceivedSignal();
}

KeyboardEventSignalType& KeyboardEventReceivedSignal(Dali::InputMethodContext context)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(context).KeyboardEventReceivedSignal();
}

ContentReceivedSignalType& ContentReceivedSignal(Dali::InputMethodContext context)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(context).ContentReceivedSignal();
}

LegacyStatusChangedSignalType& LegacyStatusChangedSignal(Dali::InputMethodContext context)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(context).LegacyStatusChangedSignal();
}

LegacyKeyboardTypeChangedSignalType& LegacyKeyboardTypeChangedSignal(Dali::InputMethodContext context)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(context).LegacyKeyboardTypeChangedSignal();
}

LegacyKeyboardResizedSignalType& LegacyResizedSignal(Dali::InputMethodContext context)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(context).LegacyResizedSignal();
}

LegacyLanguageChangedSignalType& LegacyLanguageChangedSignal(Dali::InputMethodContext context)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(context).LegacyLanguageChangedSignal();
}

LegacyContentReceivedSignalType& LegacyContentReceivedSignal(Dali::InputMethodContext context)
{
  return Internal::Adaptor::InputMethodContext::GetImplementation(context).LegacyContentReceivedSignal();
}

} // namespace InputMethodContext
} // namespace Integration
} // namespace Dali
