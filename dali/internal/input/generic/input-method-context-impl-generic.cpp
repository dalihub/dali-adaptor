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

// CLASS HEADER
#include <dali/internal/input/generic/input-method-context-impl-generic.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_INPUT_METHOD_CONTEXT");
#endif
} // namespace

InputMethodContextPtr InputMethodContextGeneric::New(Dali::Actor actor)
{
  InputMethodContextPtr manager;

  if(actor && Dali::Adaptor::IsAvailable())
  {
    manager = new InputMethodContextGeneric(actor);
  }

  return manager;
}

void InputMethodContextGeneric::Finalize()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::Finalize\n");
  DisconnectCallbacks();
}

InputMethodContextGeneric::InputMethodContextGeneric(Dali::Actor actor)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::InputMethodContextGeneric\n");
  Initialize();
}

InputMethodContextGeneric::~InputMethodContextGeneric()
{
  Finalize();
}

void InputMethodContextGeneric::Initialize()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::Initialize\n");
}

void InputMethodContextGeneric::ConnectCallbacks()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::ConnectCallbacks\n");
}

void InputMethodContextGeneric::DisconnectCallbacks()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::DisconnectCallbacks\n");
}

void InputMethodContextGeneric::Activate()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::Activate\n");
}

void InputMethodContextGeneric::Deactivate()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::Deactivate\n");
}

void InputMethodContextGeneric::Reset()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::Reset\n");
}

ImfContext* InputMethodContextGeneric::GetContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::GetContext\n");

  return nullptr;
}

bool InputMethodContextGeneric::RestoreAfterFocusLost() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::RestoreAfterFocusLost\n");

  return false;
}

void InputMethodContextGeneric::SetRestoreAfterFocusLost(bool toggle)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::SetRestoreAfterFocusLost\n");
}

void InputMethodContextGeneric::PreEditChanged(void*, ImfContext* imfContext, void* event_info)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::PreEditChanged\n");
}

void InputMethodContextGeneric::CommitReceived(void*, ImfContext* imfContext, void* event_info)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::CommitReceived\n");
}

bool InputMethodContextGeneric::RetrieveSurrounding(void* data, ImfContext* imfContext, char** text, int* cursorPosition)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::RetrieveSurrounding\n");
  return false;
}

void InputMethodContextGeneric::DeleteSurrounding(void* data, ImfContext* imfContext, void* event_info)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::DeleteSurrounding\n");
}

void InputMethodContextGeneric::SendPrivateCommand(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::SendPrivateCommand\n");
}

void InputMethodContextGeneric::SendCommitContent(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::SendCommitContent\n");
}

void InputMethodContextGeneric::SendSelectionSet(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::SendSelectionSet\n");
}

void InputMethodContextGeneric::NotifyCursorPosition()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::NotifyCursorPosition\n");
}

void InputMethodContextGeneric::SetCursorPosition(unsigned int cursorPosition)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::SetCursorPosition\n");
}

unsigned int InputMethodContextGeneric::GetCursorPosition() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::GetCursorPosition\n");

  return 0;
}

void InputMethodContextGeneric::SetSurroundingText(const std::string& text)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::SetSurroundingText\n");
}

const std::string& InputMethodContextGeneric::GetSurroundingText() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::GetSurroundingText\n");

  static const std::string empty("");
  return empty;
}

void InputMethodContextGeneric::NotifyTextInputMultiLine(bool multiLine)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::NotifyTextInputMultiLine\n");
}

Dali::InputMethodContext::TextDirection InputMethodContextGeneric::GetTextDirection()
{
  Dali::InputMethodContext::TextDirection direction(Dali::InputMethodContext::LEFT_TO_RIGHT);
  return direction;
}

Rect<int> InputMethodContextGeneric::GetInputMethodArea()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::GetInputMethodArea\n");

  int xPos, yPos, width, height;

  width = height = xPos = yPos = 0;
  return Rect<int>(xPos, yPos, width, height);
}

void InputMethodContextGeneric::ApplyOptions(const InputMethodOptions& options)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::ApplyOptions\n");
}

void InputMethodContextGeneric::SetInputPanelData(const std::string& data)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::SetInputPanelData\n");
}

void InputMethodContextGeneric::GetInputPanelData(std::string& data)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::GetInputPanelData\n");

  data.assign("");
}

Dali::InputMethodContext::State InputMethodContextGeneric::GetInputPanelState()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::GetInputPanelState\n");
  return Dali::InputMethodContext::DEFAULT;
}

void InputMethodContextGeneric::SetReturnKeyState(bool visible)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::SetReturnKeyState\n");
}

void InputMethodContextGeneric::AutoEnableInputPanel(bool enabled)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::AutoEnableInputPanel\n");
}

void InputMethodContextGeneric::ShowInputPanel()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::ShowInputPanel\n");
}

void InputMethodContextGeneric::HideInputPanel()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::HideInputPanel\n");
}

Dali::InputMethodContext::KeyboardType InputMethodContextGeneric::GetKeyboardType()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::GetKeyboardType\n");

  return Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD;
}

std::string InputMethodContextGeneric::GetInputPanelLocale()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::GetInputPanelLocale\n");

  std::string locale = "";
  return locale;
}

void InputMethodContextGeneric::SetContentMIMETypes(const std::string& mimeTypes)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::SetContentMIMETypes\n");
}

bool InputMethodContextGeneric::FilterEventKey(const Dali::KeyEvent& keyEvent)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::FilterEventKey\n");

  return false;
}

void InputMethodContextGeneric::AllowTextPrediction(bool prediction)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::AllowTextPrediction\n");
}

bool InputMethodContextGeneric::IsTextPredictionAllowed() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::IsTextPredictionAllowed\n");

  return false;
}

void InputMethodContextGeneric::SetFullScreenMode(bool prediction)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::SetFullScreenMode\n");
}

bool InputMethodContextGeneric::IsFullScreenMode() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::IsFullScreenMode\n");

  return false;
}

void InputMethodContextGeneric::SetInputPanelLanguage(Dali::InputMethodContext::InputPanelLanguage language)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::SetInputPanelLanguage\n");
}

Dali::InputMethodContext::InputPanelLanguage InputMethodContextGeneric::GetInputPanelLanguage() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::GetInputPanelLanguage\n");

  return Dali::InputMethodContext::InputPanelLanguage::AUTOMATIC;
}

void InputMethodContextGeneric::SetInputPanelPosition(unsigned int x, unsigned int y)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::SetInputPanelPosition\n");
}

bool InputMethodContextGeneric::SetInputPanelPositionAlign(int x, int y, Dali::InputMethodContext::InputPanelAlign align)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::SetInputPanelPositionAlign\n");
  return false;
}

void InputMethodContextGeneric::GetPreeditStyle(Dali::InputMethodContext::PreEditAttributeDataContainer& attrs) const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::GetPreeditStyle\n");
  // Do Nothing
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
