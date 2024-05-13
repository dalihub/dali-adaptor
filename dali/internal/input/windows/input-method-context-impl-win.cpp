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
#include <dali/internal/input/windows/input-method-context-impl-win.h>

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

InputMethodContextPtr InputMethodContextWin::New(Dali::Actor actor)
{
  InputMethodContextPtr manager;

  if(actor && Adaptor::IsAvailable())
  {
    manager = new InputMethodContextWin(actor);
  }

  return manager;
}

void InputMethodContextWin::Finalize()
{
}

InputMethodContextWin::InputMethodContextWin(Dali::Actor actor)
: mWin32Window(0),
  mIMFCursorPosition(0),
  mSurroundingText(),
  mRestoreAfterFocusLost(false),
  mIdleCallbackConnected(false)
{
  actor.OnSceneSignal().Connect(this, &InputMethodContextWin::OnStaged);
}

InputMethodContextWin::~InputMethodContextWin()
{
  Finalize();
}

void InputMethodContextWin::Initialize()
{
  CreateContext(mWin32Window);
  ConnectCallbacks();
}

void InputMethodContextWin::CreateContext(WinWindowHandle winHandle)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::CreateContext\n");
}

void InputMethodContextWin::DeleteContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::DeleteContext\n");
}

// Callbacks for predicitive text support.
void InputMethodContextWin::ConnectCallbacks()
{
}

void InputMethodContextWin::DisconnectCallbacks()
{
}

void InputMethodContextWin::Activate()
{
  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;
}

void InputMethodContextWin::Deactivate()
{
  mIdleCallbackConnected = false;
}

void InputMethodContextWin::Reset()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::Reset\n");
}

ImfContext* InputMethodContextWin::GetContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetContext\n");

  return NULL;
}

bool InputMethodContextWin::RestoreAfterFocusLost() const
{
  return mRestoreAfterFocusLost;
}

void InputMethodContextWin::SetRestoreAfterFocusLost(bool toggle)
{
  mRestoreAfterFocusLost = toggle;
}

/**
 * Called when an InputMethodContext Pre-Edit changed event is received.
 * We are still predicting what the user is typing.  The latest string is what the InputMethodContext module thinks
 * the user wants to type.
 */
void InputMethodContextWin::PreEditChanged(void*, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::PreEditChanged\n");
}

void InputMethodContextWin::CommitReceived(void*, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::CommitReceived\n");

  if(Dali::Adaptor::IsAvailable())
  {
    const std::string keyString(static_cast<char*>(eventInfo));

    Dali::InputMethodContext               handle(this);
    Dali::InputMethodContext::EventData    eventData(Dali::InputMethodContext::COMMIT, keyString, 0, 0);
    Dali::InputMethodContext::CallbackData callbackData = mEventSignal.Emit(handle, eventData);

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
bool InputMethodContextWin::RetrieveSurrounding(void* data, ImfContext* imfContext, char** text, int* cursorPosition)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::RetrieveSurrounding\n");

  Dali::InputMethodContext::EventData    imfData(Dali::InputMethodContext::GET_SURROUNDING, std::string(), 0, 0);
  Dali::InputMethodContext               handle(this);
  Dali::InputMethodContext::CallbackData callbackData = mEventSignal.Emit(handle, imfData);

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
void InputMethodContextWin::DeleteSurrounding(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::DeleteSurrounding\n");
}

void InputMethodContextWin::NotifyCursorPosition()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::NotifyCursorPosition\n");
}

void InputMethodContextWin::SetCursorPosition(unsigned int cursorPosition)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetCursorPosition\n");

  mIMFCursorPosition = static_cast<int>(cursorPosition);
}

unsigned int InputMethodContextWin::GetCursorPosition() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetCursorPosition\n");

  return static_cast<unsigned int>(mIMFCursorPosition);
}

void InputMethodContextWin::SetSurroundingText(const std::string& text)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetSurroundingText\n");

  mSurroundingText = text;
}

const std::string& InputMethodContextWin::GetSurroundingText() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetSurroundingText\n");

  return mSurroundingText;
}

void InputMethodContextWin::NotifyTextInputMultiLine(bool multiLine)
{
}

Dali::InputMethodContext::TextDirection InputMethodContextWin::GetTextDirection()
{
  Dali::InputMethodContext::TextDirection direction(Dali::InputMethodContext::LEFT_TO_RIGHT);

  return direction;
}

Rect<int> InputMethodContextWin::GetInputMethodArea()
{
  int xPos, yPos, width, height;

  width = height = xPos = yPos = 0;

  return Rect<int>(xPos, yPos, width, height);
}

void InputMethodContextWin::ApplyOptions(const InputMethodOptions& options)
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

void InputMethodContextWin::SetInputPanelData(const std::string& data)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetInputPanelData\n");
}

void InputMethodContextWin::GetInputPanelData(std::string& data)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetInputPanelData\n");
}

Dali::InputMethodContext::State InputMethodContextWin::GetInputPanelState()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetInputPanelState\n");
  return Dali::InputMethodContext::DEFAULT;
}

void InputMethodContextWin::SetReturnKeyState(bool visible)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetReturnKeyState\n");
}

void InputMethodContextWin::AutoEnableInputPanel(bool enabled)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::AutoEnableInputPanel\n");
}

void InputMethodContextWin::ShowInputPanel()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::ShowInputPanel\n");
}

void InputMethodContextWin::HideInputPanel()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::HideInputPanel\n");
}

Dali::InputMethodContext::KeyboardType InputMethodContextWin::GetKeyboardType()
{
  return Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD;
}

std::string InputMethodContextWin::GetInputPanelLocale()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetInputPanelLocale\n");

  std::string locale = "";
  return locale;
}

void InputMethodContextWin::SetContentMIMETypes(const std::string& mimeTypes)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetContentMIMETypes\n");
}

bool InputMethodContextWin::FilterEventKey(const Dali::KeyEvent& keyEvent)
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

void InputMethodContextWin::SetInputPanelLanguage(Dali::InputMethodContext::InputPanelLanguage language)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetInputPanelLanguage\n");
}

Dali::InputMethodContext::InputPanelLanguage InputMethodContextWin::GetInputPanelLanguage() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetInputPanelLanguage\n");
  return Dali::InputMethodContext::InputPanelLanguage::AUTOMATIC;
}

void InputMethodContextWin::SetInputPanelPosition(unsigned int x, unsigned int y)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetInputPanelPosition\n");
}

bool InputMethodContextWin::SetInputPanelPositionAlign(int x, int y, Dali::InputMethodContext::InputPanelAlign align)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::SetInputPanelPositionAlign\n");
  return false;
}

void InputMethodContextWin::GetPreeditStyle(Dali::InputMethodContext::PreEditAttributeDataContainer& attrs) const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextWin::GetPreeditStyle\n");
  attrs = mPreeditAttrs;
}

bool InputMethodContextWin::ProcessEventKeyDown(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);
  return eventHandled;
}

bool InputMethodContextWin::ProcessEventKeyUp(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);
  return eventHandled;
}

void InputMethodContextWin::OnStaged(Dali::Actor actor)
{
  WinWindowHandle winWindow(AnyCast<WinWindowHandle>(Dali::Integration::SceneHolder::Get(actor).GetNativeHandle()));

  if(mWin32Window != winWindow)
  {
    mWin32Window = winWindow;

    // Reset
    Finalize();
    Initialize();
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
