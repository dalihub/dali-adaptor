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

// CLASS HEADER
#include <dali/internal/input/macos/input-method-context-impl-mac.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/input/common/key-impl.h>
#include <dali/internal/input/common/virtual-keyboard-impl.h>
#include <dali/internal/system/common/locale-utils.h>
#include <dali/public-api/adaptor-framework/key.h>

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

InputMethodContextPtr InputMethodContextCocoa::New(Dali::Actor actor)
{
  InputMethodContextPtr manager;

  if(actor && Adaptor::IsAvailable())
  {
    manager = new InputMethodContextCocoa(actor);
  }

  return manager;
}

void InputMethodContextCocoa::Finalize()
{
}

InputMethodContextCocoa::InputMethodContextCocoa(Dali::Actor actor)
: mIMFCursorPosition(0),
  mSurroundingText(),
  mRestoreAfterFocusLost(false),
  mIdleCallbackConnected(false)
{
  actor.OnSceneSignal().Connect(this, &InputMethodContextCocoa::OnStaged);
}

InputMethodContextCocoa::~InputMethodContextCocoa()
{
  Finalize();
}

void InputMethodContextCocoa::Initialize()
{
  ConnectCallbacks();
}

// Callbacks for predicitive text support.
void InputMethodContextCocoa::ConnectCallbacks()
{
}

void InputMethodContextCocoa::DisconnectCallbacks()
{
}

void InputMethodContextCocoa::Activate()
{
  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;
}

void InputMethodContextCocoa::Deactivate()
{
  mIdleCallbackConnected = false;
}

void InputMethodContextCocoa::Reset()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::Reset\n");
}

ImfContext* InputMethodContextCocoa::GetContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::GetContext\n");

  return NULL;
}

bool InputMethodContextCocoa::RestoreAfterFocusLost() const
{
  return mRestoreAfterFocusLost;
}

void InputMethodContextCocoa::SetRestoreAfterFocusLost(bool toggle)
{
  mRestoreAfterFocusLost = toggle;
}

/**
 * Called when an InputMethodContext Pre-Edit changed event is received.
 * We are still predicting what the user is typing.  The latest string is what the InputMethodContext module thinks
 * the user wants to type.
 */
void InputMethodContextCocoa::PreEditChanged(void*, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::PreEditChanged\n");
}

void InputMethodContextCocoa::CommitReceived(void*, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::CommitReceived\n");

  if(Dali::Adaptor::IsAvailable())
  {
    const std::string keyString(static_cast<char*>(eventInfo));

    Dali::InputMethodContext            handle(this);
    Dali::InputMethodContext::EventData eventData(Dali::InputMethodContext::COMMIT, keyString, 0, 0);
    mEventSignal.Emit(handle, eventData);
    Dali::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, eventData);

    if(callbackData.update)
    {
      mIMFCursorPosition = static_cast<int>(callbackData.cursorPosition);

      NotifyCursorPosition();
    }
  }
}

/**
 * Called when an InputMethodContext retrieve surround event is received.
 * Here the InputMethodContext module wishes to know the string we are working with and where within the string the cursor is
 * We need to signal the application to tell us this information.
 */
bool InputMethodContextCocoa::RetrieveSurrounding(void* data, ImfContext* imfContext, char** text, int* cursorPosition)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::RetrieveSurrounding\n");

  Dali::InputMethodContext::EventData imfData(Dali::InputMethodContext::GET_SURROUNDING, std::string(), 0, 0);
  Dali::InputMethodContext            handle(this);
  mEventSignal.Emit(handle, imfData);
  Dali::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, imfData);

  if(callbackData.update)
  {
    if(text)
    {
      *text = strdup(callbackData.currentText.c_str());
    }

    if(cursorPosition)
    {
      mIMFCursorPosition = static_cast<int>(callbackData.cursorPosition);
      *cursorPosition    = mIMFCursorPosition;
    }
  }

  return true;
}

/**
 * Called when an InputMethodContext delete surrounding event is received.
 * Here we tell the application that it should delete a certain range.
 */
void InputMethodContextCocoa::DeleteSurrounding(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::DeleteSurrounding\n");
}

void InputMethodContextCocoa::NotifyCursorPosition()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::NotifyCursorPosition\n");
}

void InputMethodContextCocoa::SetCursorPosition(unsigned int cursorPosition)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::SetCursorPosition\n");

  mIMFCursorPosition = static_cast<int>(cursorPosition);
}

unsigned int InputMethodContextCocoa::GetCursorPosition() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::GetCursorPosition\n");

  return static_cast<unsigned int>(mIMFCursorPosition);
}

void InputMethodContextCocoa::SetSurroundingText(const std::string& text)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::SetSurroundingText\n");

  mSurroundingText = text;
}

const std::string& InputMethodContextCocoa::GetSurroundingText() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::GetSurroundingText\n");

  return mSurroundingText;
}

void InputMethodContextCocoa::NotifyTextInputMultiLine(bool multiLine)
{
}

Dali::InputMethodContext::TextDirection InputMethodContextCocoa::GetTextDirection()
{
  Dali::InputMethodContext::TextDirection direction(Dali::InputMethodContext::LEFT_TO_RIGHT);

  return direction;
}

Rect<int> InputMethodContextCocoa::GetInputMethodArea()
{
  int xPos, yPos, width, height;

  width = height = xPos = yPos = 0;

  return Rect<int>(xPos, yPos, width, height);
}

void InputMethodContextCocoa::ApplyOptions(const InputMethodOptions& options)
{
  using namespace Dali::InputMethod::Category;

  int index;

  if(mOptions.CompareAndSet(PANEL_LAYOUT, options, index))
  {
  }
  if(mOptions.CompareAndSet(BUTTON_ACTION, options, index))
  {
  }
  if(mOptions.CompareAndSet(AUTO_CAPITALIZE, options, index))
  {
  }
  if(mOptions.CompareAndSet(VARIATION, options, index))
  {
  }
}

void InputMethodContextCocoa::SetInputPanelData(const std::string& data)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::SetInputPanelData\n");
}

void InputMethodContextCocoa::GetInputPanelData(std::string& data)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::GetInputPanelData\n");
}

Dali::InputMethodContext::State InputMethodContextCocoa::GetInputPanelState()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::GetInputPanelState\n");
  return Dali::InputMethodContext::DEFAULT;
}

void InputMethodContextCocoa::SetReturnKeyState(bool visible)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::SetReturnKeyState\n");
}

void InputMethodContextCocoa::AutoEnableInputPanel(bool enabled)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::AutoEnableInputPanel\n");
}

void InputMethodContextCocoa::ShowInputPanel()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::ShowInputPanel\n");
}

void InputMethodContextCocoa::HideInputPanel()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::HideInputPanel\n");
}

Dali::InputMethodContext::KeyboardType InputMethodContextCocoa::GetKeyboardType()
{
  return Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD;
}

std::string InputMethodContextCocoa::GetInputPanelLocale()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::GetInputPanelLocale\n");

  std::string locale = "";
  return locale;
}

void InputMethodContextCocoa::SetContentMIMETypes(const std::string& mimeTypes)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::SetContentMIMETypes\n");
}

bool InputMethodContextCocoa::FilterEventKey(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);

  if(!KeyLookup::IsDeviceButton(keyEvent.GetKeyName().c_str()))
  {
    //check whether it's key down or key up event
    if(keyEvent.GetState() == Dali::KeyEvent::DOWN)
    {
      eventHandled = ProcessEventKeyDown(keyEvent);
    }
    else if(keyEvent.GetState() == Dali::KeyEvent::UP)
    {
      eventHandled = ProcessEventKeyUp(keyEvent);
    }
  }

  return eventHandled;
}

void InputMethodContextCocoa::SetInputPanelLanguage(Dali::InputMethodContext::InputPanelLanguage language)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::SetInputPanelLanguage\n");
}

Dali::InputMethodContext::InputPanelLanguage InputMethodContextCocoa::GetInputPanelLanguage() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::GetInputPanelLanguage\n");
  return Dali::InputMethodContext::InputPanelLanguage::AUTOMATIC;
}

void InputMethodContextCocoa::SetInputPanelPosition(unsigned int x, unsigned int y)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::SetInputPanelPosition\n");
}

bool InputMethodContextCocoa::SetInputPanelPositionAlign(int x, int y, Dali::InputMethodContext::InputPanelAlign align)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::SetInputPanelPositionAlign\n");
  return false;
}

void InputMethodContextCocoa::GetPreeditStyle(Dali::InputMethodContext::PreEditAttributeDataContainer& attrs) const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextCocoa::GetPreeditStyle\n");
  attrs = mPreeditAttrs;
}

bool InputMethodContextCocoa::ProcessEventKeyDown(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);
  return eventHandled;
}

bool InputMethodContextCocoa::ProcessEventKeyUp(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);
  return eventHandled;
}

void InputMethodContextCocoa::OnStaged(Dali::Actor actor)
{
  // Reset
  Finalize();
  Initialize();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
