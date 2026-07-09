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
#include <dali/internal/input/ubuntu-x11/input-method-context-impl-x.h>

// EXTERNAL INCLUDES
#include <Ecore.h>
#include <dali/devel-api/common/singleton-service.h>
#include <dali/devel-api/object/type-registry.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/events/key-event.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/string-utils.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/input/common/key-impl.h>
#include <dali/internal/input/common/virtual-keyboard-impl.h>
#include <dali/internal/input/linux/dali-ecore-imf.h>
#include <dali/internal/input/ubuntu-x11/dali-ecore-input.h>
#include <dali/internal/system/common/locale-utils.h>
#include <dali/public-api/adaptor-framework/key.h>

using Dali::Integration::ToStdString;

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

// Currently this code is internal to dali/dali/internal/event/text/utf8.h but should be made Public and used from there instead.
size_t Utf8SequenceLength(const unsigned char leadByte)
{
  size_t length = 0;

  if((leadByte & 0x80) == 0) //ASCII character (lead bit zero)
  {
    length = 1;
  }
  else if((leadByte & 0xe0) == 0xc0) //110x xxxx
  {
    length = 2;
  }
  else if((leadByte & 0xf0) == 0xe0) //1110 xxxx
  {
    length = 3;
  }
  else if((leadByte & 0xf8) == 0xf0) //1111 0xxx
  {
    length = 4;
  }
  else if((leadByte & 0xfc) == 0xf8) //1111 10xx
  {
    length = 5;
  }
  else if((leadByte & 0xfe) == 0xfc) //1111 110x
  {
    length = 6;
  }

  return length;
}

Ecore_IMF_Input_Panel_Layout ToEcorePanelLayout(Dali::InputMethod::PanelLayout layout)
{
  switch(layout)
  {
    case Dali::InputMethod::PanelLayout::NORMAL:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL;
    case Dali::InputMethod::PanelLayout::NUMBER:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBER;
    case Dali::InputMethod::PanelLayout::EMAIL:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_EMAIL;
    case Dali::InputMethod::PanelLayout::URL:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_URL;
    case Dali::InputMethod::PanelLayout::PHONENUMBER:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_PHONENUMBER;
    case Dali::InputMethod::PanelLayout::IP:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_IP;
    case Dali::InputMethod::PanelLayout::MONTH:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_MONTH;
    case Dali::InputMethod::PanelLayout::NUMBER_ONLY:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY;
    case Dali::InputMethod::PanelLayout::PASSWORD:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD;
    case Dali::InputMethod::PanelLayout::DATE_TIME:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_DATETIME;
    case Dali::InputMethod::PanelLayout::EMOTICON:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_EMOTICON;
    case Dali::InputMethod::PanelLayout::VOICE:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_VOICE;
    default:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL;
  }
}

Dali::InputMethod::PanelLayout ToDaliPanelLayout(Ecore_IMF_Input_Panel_Layout ecoreLayout)
{
  switch(ecoreLayout)
  {
    case ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBER:
      return Dali::InputMethod::PanelLayout::NUMBER;
    case ECORE_IMF_INPUT_PANEL_LAYOUT_EMAIL:
      return Dali::InputMethod::PanelLayout::EMAIL;
    case ECORE_IMF_INPUT_PANEL_LAYOUT_URL:
      return Dali::InputMethod::PanelLayout::URL;
    case ECORE_IMF_INPUT_PANEL_LAYOUT_PHONENUMBER:
      return Dali::InputMethod::PanelLayout::PHONENUMBER;
    case ECORE_IMF_INPUT_PANEL_LAYOUT_IP:
      return Dali::InputMethod::PanelLayout::IP;
    case ECORE_IMF_INPUT_PANEL_LAYOUT_MONTH:
      return Dali::InputMethod::PanelLayout::MONTH;
    case ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY:
      return Dali::InputMethod::PanelLayout::NUMBER_ONLY;
    case ECORE_IMF_INPUT_PANEL_LAYOUT_HEX:
      return Dali::InputMethod::PanelLayout::NUMBER_ONLY;
    case ECORE_IMF_INPUT_PANEL_LAYOUT_TERMINAL:
      return Dali::InputMethod::PanelLayout::NORMAL;
    case ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD:
      return Dali::InputMethod::PanelLayout::PASSWORD;
    case ECORE_IMF_INPUT_PANEL_LAYOUT_DATETIME:
      return Dali::InputMethod::PanelLayout::DATE_TIME;
    case ECORE_IMF_INPUT_PANEL_LAYOUT_EMOTICON:
      return Dali::InputMethod::PanelLayout::EMOTICON;
    case ECORE_IMF_INPUT_PANEL_LAYOUT_VOICE:
      return Dali::InputMethod::PanelLayout::VOICE;
    default:
      return Dali::InputMethod::PanelLayout::NORMAL;
  }
}

Ecore_IMF_Input_Panel_Return_Key_Type ToEcoreReturnKey(Dali::InputMethod::ReturnKeyType action)
{
  switch(action)
  {
    case Dali::InputMethod::ReturnKeyType::DONE:
      return ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DONE;
    case Dali::InputMethod::ReturnKeyType::GO:
      return ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_GO;
    case Dali::InputMethod::ReturnKeyType::JOIN:
      return ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_JOIN;
    case Dali::InputMethod::ReturnKeyType::LOGIN:
      return ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_LOGIN;
    case Dali::InputMethod::ReturnKeyType::NEXT:
      return ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_NEXT;
    case Dali::InputMethod::ReturnKeyType::SEARCH:
      return ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH;
    case Dali::InputMethod::ReturnKeyType::SEND:
      return ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEND;
    case Dali::InputMethod::ReturnKeyType::SIGNIN:
      return ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SIGNIN;
    default:
      return ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT;
  }
}

Dali::InputMethod::ReturnKeyType ToDaliReturnKey(Ecore_IMF_Input_Panel_Return_Key_Type ecoreKey)
{
  switch(ecoreKey)
  {
    case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DONE:
      return Dali::InputMethod::ReturnKeyType::DONE;
    case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_GO:
      return Dali::InputMethod::ReturnKeyType::GO;
    case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_JOIN:
      return Dali::InputMethod::ReturnKeyType::JOIN;
    case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_LOGIN:
      return Dali::InputMethod::ReturnKeyType::LOGIN;
    case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_NEXT:
      return Dali::InputMethod::ReturnKeyType::NEXT;
    case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH:
      return Dali::InputMethod::ReturnKeyType::SEARCH;
    case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEND:
      return Dali::InputMethod::ReturnKeyType::SEND;
    case ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SIGNIN:
      return Dali::InputMethod::ReturnKeyType::SIGNIN;
    default:
      return Dali::InputMethod::ReturnKeyType::DEFAULT;
  }
}

Ecore_IMF_Autocapital_Type ToEcoreAutoCapital(Dali::InputMethod::AutoCapitalType autoCapital)
{
  switch(autoCapital)
  {
    case Dali::InputMethod::AutoCapitalType::WORD:
      return ECORE_IMF_AUTOCAPITAL_TYPE_WORD;
    case Dali::InputMethod::AutoCapitalType::SENTENCE:
      return ECORE_IMF_AUTOCAPITAL_TYPE_SENTENCE;
    case Dali::InputMethod::AutoCapitalType::ALL_CHARACTER:
      return ECORE_IMF_AUTOCAPITAL_TYPE_ALLCHARACTER;
    default:
      return ECORE_IMF_AUTOCAPITAL_TYPE_NONE;
  }
}

Dali::InputMethod::AutoCapitalType ToDaliAutoCapital(Ecore_IMF_Autocapital_Type ecoreAutoCapital)
{
  switch(ecoreAutoCapital)
  {
    case ECORE_IMF_AUTOCAPITAL_TYPE_WORD:
      return Dali::InputMethod::AutoCapitalType::WORD;
    case ECORE_IMF_AUTOCAPITAL_TYPE_SENTENCE:
      return Dali::InputMethod::AutoCapitalType::SENTENCE;
    case ECORE_IMF_AUTOCAPITAL_TYPE_ALLCHARACTER:
      return Dali::InputMethod::AutoCapitalType::ALL_CHARACTER;
    default:
      return Dali::InputMethod::AutoCapitalType::NONE;
  }
}

int ToPlatformLayoutVariation(Dali::InputMethod::PanelLayoutVariation variation)
{
  switch(variation)
  {
    case Dali::InputMethod::PanelLayoutVariation::NORMAL_WITH_FILENAME:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_FILENAME;
    case Dali::InputMethod::PanelLayoutVariation::NORMAL_WITH_PERSON_NAME:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_PERSON_NAME;
    case Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_NORMAL:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_NORMAL;
    case Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_WITH_SIGNED:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_SIGNED;
    case Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_WITH_DECIMAL:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_DECIMAL;
    case Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_WITH_SIGNED_AND_DECIMAL:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_SIGNED_AND_DECIMAL;
    case Dali::InputMethod::PanelLayoutVariation::PASSWORD_NORMAL:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD_VARIATION_NORMAL;
    case Dali::InputMethod::PanelLayoutVariation::PASSWORD_WITH_NUMBER_ONLY:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD_VARIATION_NUMBERONLY;
    default:
      return ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_NORMAL;
  }
}

Dali::InputMethod::PanelLayoutVariation ToPanelLayoutVariation(Dali::InputMethod::PanelLayout layout, int rawVariation)
{
  switch(layout)
  {
    case Dali::InputMethod::PanelLayout::NUMBER_ONLY:
    {
      switch(rawVariation)
      {
        case ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_SIGNED:
          return Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_WITH_SIGNED;
        case ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_DECIMAL:
          return Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_WITH_DECIMAL;
        case ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY_VARIATION_SIGNED_AND_DECIMAL:
          return Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_WITH_SIGNED_AND_DECIMAL;
        default:
          return Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_NORMAL;
      }
    }
    case Dali::InputMethod::PanelLayout::PASSWORD:
    {
      switch(rawVariation)
      {
        case ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD_VARIATION_NUMBERONLY:
          return Dali::InputMethod::PanelLayoutVariation::PASSWORD_WITH_NUMBER_ONLY;
        default:
          return Dali::InputMethod::PanelLayoutVariation::PASSWORD_NORMAL;
      }
    }
    default:
    {
      switch(rawVariation)
      {
        case ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_FILENAME:
          return Dali::InputMethod::PanelLayoutVariation::NORMAL_WITH_FILENAME;
        case ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_PERSON_NAME:
          return Dali::InputMethod::PanelLayoutVariation::NORMAL_WITH_PERSON_NAME;
        default:
          return Dali::InputMethod::PanelLayoutVariation::NORMAL_NORMAL;
      }
    }
  }
}

void SetEcoreInputPanelLayout(Ecore_IMF_Context* imfContext, Dali::InputMethod::PanelLayout layout)
{
  const auto ecoreLayout = ToEcorePanelLayout(layout);
  ecore_imf_context_input_panel_layout_set(imfContext, ecoreLayout);

  Ecore_IMF_Input_Hints currentHint = ecore_imf_context_input_hint_get(imfContext);
  if(ecoreLayout == ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD)
  {
    ecore_imf_context_input_hint_set(imfContext, static_cast<Ecore_IMF_Input_Hints>(currentHint | ECORE_IMF_INPUT_HINT_SENSITIVE_DATA));
  }
  else
  {
    ecore_imf_context_input_hint_set(imfContext, static_cast<Ecore_IMF_Input_Hints>(currentHint & ~ECORE_IMF_INPUT_HINT_SENSITIVE_DATA));
  }
}

// Static function calls used by ecore 'c' style callback registration
void Commit(void* data, Ecore_IMF_Context* imfContext, void* eventInfo)
{
  if(data)
  {
    InputMethodContextX* inputMethodContext = static_cast<InputMethodContextX*>(data);
    inputMethodContext->CommitReceived(data, imfContext, eventInfo);
  }
}

void PreEdit(void* data, Ecore_IMF_Context* imfContext, void* eventInfo)
{
  if(data)
  {
    InputMethodContextX* inputMethodContext = static_cast<InputMethodContextX*>(data);
    inputMethodContext->PreEditChanged(data, imfContext, eventInfo);
  }
}

Eina_Bool ImfRetrieveSurrounding(void* data, Ecore_IMF_Context* imfContext, char** text, int* cursorPosition)
{
  if(data)
  {
    InputMethodContextX* inputMethodContext = static_cast<InputMethodContextX*>(data);
    return inputMethodContext->RetrieveSurrounding(data, imfContext, text, cursorPosition);
  }
  else
  {
    return false;
  }
}

/**
 * Called when an InputMethodContext delete surrounding event is received.
 * Here we tell the application that it should delete a certain range.
 */
void ImfDeleteSurrounding(void* data, Ecore_IMF_Context* imfContext, void* eventInfo)
{
  if(data)
  {
    InputMethodContextX* inputMethodContext = static_cast<InputMethodContextX*>(data);
    inputMethodContext->DeleteSurrounding(data, imfContext, eventInfo);
  }
}

} // unnamed namespace

InputMethodContextPtr InputMethodContextX::New(Dali::Actor actor)
{
  InputMethodContextPtr manager;

  if(actor && Dali::Adaptor::IsAvailable())
  {
    manager = new InputMethodContextX(actor);
  }

  return manager;
}

void InputMethodContextX::Finalize()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::Finalize\n");
  DisconnectCallbacks();
  DeleteContext();
}

InputMethodContextX::InputMethodContextX(Dali::Actor actor)
: mSurroundingText(),
  mIMFContext(),
  mEcoreXwin(0),
  mIMFCursorPosition(0),
  mRestoreAfterFocusLost(false),
  mIdleCallbackConnected(false)
{
  ecore_imf_init();

  actor.SceneConnectedSignal().Connect(this, &InputMethodContextX::OnStaged);
}

InputMethodContextX::~InputMethodContextX()
{
  Finalize();
  ecore_imf_shutdown();
}

void InputMethodContextX::Initialize()
{
  CreateContext();
  ConnectCallbacks();
  ApplyBackupOperations();
}

void InputMethodContextX::CreateContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::CreateContext\n");

  if(!mEcoreXwin)
  {
    return;
  }

  const char* contextId = ecore_imf_context_default_id_get();
  if(contextId)
  {
    mIMFContext = ecore_imf_context_add(contextId);

    if(mIMFContext)
    {
      ecore_imf_context_client_window_set(mIMFContext, reinterpret_cast<void*>(mEcoreXwin));
    }
    else
    {
      DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContext Unable to get IMFContext\n");
    }
  }
  else
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContext Unable to get IMFContext\n");
  }
}

void InputMethodContextX::DeleteContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::DeleteContext\n");

  if(mIMFContext)
  {
    ecore_imf_context_del(mIMFContext);
    mIMFContext = NULL;
  }
}

// Callbacks for predicitive text support.
void InputMethodContextX::ConnectCallbacks()
{
  if(mIMFContext)
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::ConnectCallbacks\n");

    ecore_imf_context_event_callback_add(mIMFContext, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, PreEdit, this);
    ecore_imf_context_event_callback_add(mIMFContext, ECORE_IMF_CALLBACK_COMMIT, Commit, this);
    ecore_imf_context_event_callback_add(mIMFContext, ECORE_IMF_CALLBACK_DELETE_SURROUNDING, ImfDeleteSurrounding, this);

    ecore_imf_context_retrieve_surrounding_callback_set(mIMFContext, ImfRetrieveSurrounding, this);
  }
}

void InputMethodContextX::DisconnectCallbacks()
{
  if(mIMFContext)
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::DisconnectCallbacks\n");

    ecore_imf_context_event_callback_del(mIMFContext, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, PreEdit);
    ecore_imf_context_event_callback_del(mIMFContext, ECORE_IMF_CALLBACK_COMMIT, Commit);
    ecore_imf_context_event_callback_del(mIMFContext, ECORE_IMF_CALLBACK_DELETE_SURROUNDING, ImfDeleteSurrounding);

    // We do not need to unset the retrieve surrounding callback.
  }
}

void InputMethodContextX::Activate()
{
  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;

  if(mIMFContext)
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::Activate\n");

    ecore_imf_context_focus_in(mIMFContext);

    // emit keyboard activated signal
    Dali::InputMethodContext handle(this);
    mActivatedSignal.Emit(handle);
  }
}

void InputMethodContextX::Deactivate()
{
  if(mIMFContext)
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::Deactivate\n");

    Reset();
    ecore_imf_context_focus_out(mIMFContext);
  }

  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;
}

void InputMethodContextX::Reset()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::Reset\n");

  if(mIMFContext)
  {
    ecore_imf_context_reset(mIMFContext);
  }
}

ImfContext* InputMethodContextX::GetContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::GetContext\n");

  return mIMFContext;
}

bool InputMethodContextX::RestoreAfterFocusLost() const
{
  return mRestoreAfterFocusLost;
}

bool InputMethodContextX::SetRestoreAfterFocusLost(bool toggle)
{
  mRestoreAfterFocusLost = toggle;
  return true;
}

/**
 * Called when an InputMethodContext Pre-Edit changed event is received.
 * We are still predicting what the user is typing.  The latest string is what the InputMethodContext module thinks
 * the user wants to type.
 */
void InputMethodContextX::PreEditChanged(void*, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::PreEditChanged\n");
  auto context = static_cast<Ecore_IMF_Context*>(imfContext);

  char*      preEditString(NULL);
  int        cursorPosition(0);
  Eina_List* attrs = NULL;
  Eina_List* l     = NULL;

  Ecore_IMF_Preedit_Attr* attr;

  mPreeditAttrs.Clear();

  // Retrieves attributes as well as the string the cursor position offset from start of pre-edit string.
  // the attributes (attrs) is used in languages that use the soft arrows keys to insert characters into a current pre-edit string.
  ecore_imf_context_preedit_string_with_attributes_get(context, &preEditString, &attrs, &cursorPosition);

  if(attrs)
  {
    // iterate through the list of attributes getting the type, start and end position.
    for(l = attrs, (attr = static_cast<Ecore_IMF_Preedit_Attr*>(eina_list_data_get(l))); l; l = eina_list_next(l), (attr = static_cast<Ecore_IMF_Preedit_Attr*>(eina_list_data_get(l))))
    {
      Dali::Integration::InputMethodContext::PreeditAttributeData data;
      data.startIndex = 0;
      data.endIndex   = 0;

      size_t visualCharacterIndex = 0;
      size_t byteIndex            = 0;

      // iterate through null terminated string checking each character's position against the given byte position ( attr->end_index ).
      char leadByte = preEditString[byteIndex];

      while(leadByte != '\0')
      {
        leadByte = preEditString[byteIndex]; // Update the character to get the number of its byte

        // attr->end_index is provided as a byte position not character and we need to know the character position.
        const size_t currentSequenceLength = Utf8SequenceLength(leadByte); // returns number of bytes used to represent character.
        if(byteIndex <= attr->start_index)
        {
          data.startIndex = visualCharacterIndex;
        }
        if(byteIndex >= attr->end_index)
        {
          data.endIndex = visualCharacterIndex;
          break;
          // end loop as found cursor position that matches byte position
        }
        else
        {
          byteIndex += currentSequenceLength; // jump to next character
          visualCharacterIndex++;             // increment character count so we know our position for when we get a match
        }
      }

      switch(attr->preedit_type)
      {
        case ECORE_IMF_PREEDIT_TYPE_NONE:
        {
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::NONE;
          break;
        }
        case ECORE_IMF_PREEDIT_TYPE_SUB1:
        {
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::UNDERLINE;
          break;
        }
        case ECORE_IMF_PREEDIT_TYPE_SUB2:
        {
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::REVERSE;
          break;
        }
        case ECORE_IMF_PREEDIT_TYPE_SUB3:
        {
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::HIGHLIGHT;
          break;
        }
        case ECORE_IMF_PREEDIT_TYPE_SUB4:
        {
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::CUSTOM_PLATFORM_STYLE_1;
          break;
        }
        case ECORE_IMF_PREEDIT_TYPE_SUB5:
        {
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::CUSTOM_PLATFORM_STYLE_2;
          break;
        }
        case ECORE_IMF_PREEDIT_TYPE_SUB6:
        {
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::CUSTOM_PLATFORM_STYLE_3;
          break;
        }
        case ECORE_IMF_PREEDIT_TYPE_SUB7:
        {
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::CUSTOM_PLATFORM_STYLE_4;
          break;
        }
        default:
        {
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::NONE;
          break;
        }
      }
      mPreeditAttrs.PushBack(data);
    }
  }

  if(Dali::Adaptor::IsAvailable())
  {
    Dali::InputMethodContext                         handle(this);
    Dali::Integration::InputMethodContext::EventData eventData(Dali::Integration::InputMethodContext::PRE_EDIT, Dali::String(preEditString ? preEditString : ""), cursorPosition, 0);
    mEventSignal.Emit(handle, eventData);
    Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, eventData);

    if(callbackData.update)
    {
      mIMFCursorPosition = static_cast<int>(callbackData.cursorPosition);

      NotifyCursorPosition();
    }

    if(callbackData.preeditResetRequired)
    {
      Reset();
    }
  }
  free(preEditString);
}

void InputMethodContextX::CommitReceived(void*, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::CommitReceived\n");

  if(Dali::Adaptor::IsAvailable())
  {
    const std::string keyString(static_cast<char*>(eventInfo));

    Dali::InputMethodContext                         handle(this);
    Dali::Integration::InputMethodContext::EventData eventData(Dali::Integration::InputMethodContext::COMMIT, Dali::String(keyString.c_str()), 0, 0);
    mEventSignal.Emit(handle, eventData);
    Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, eventData);

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
bool InputMethodContextX::RetrieveSurrounding(void* data, ImfContext* imfContext, char** text, int* cursorPosition)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::RetrieveSurrounding\n");

  Dali::Integration::InputMethodContext::EventData imfData(Dali::Integration::InputMethodContext::GET_SURROUNDING, Dali::String(), 0, 0);
  Dali::InputMethodContext                         handle(this);
  mEventSignal.Emit(handle, imfData);
  Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, imfData);

  if(callbackData.update)
  {
    if(text)
    {
      *text = strdup(callbackData.currentText.CStr());
    }

    if(cursorPosition)
    {
      mIMFCursorPosition = static_cast<int>(callbackData.cursorPosition);
      *cursorPosition    = mIMFCursorPosition;
    }
  }

  return EINA_TRUE;
}

/**
 * Called when an InputMethodContext delete surrounding event is received.
 * Here we tell the application that it should delete a certain range.
 */
void InputMethodContextX::DeleteSurrounding(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::DeleteSurrounding\n");

  if(Dali::Adaptor::IsAvailable())
  {
    Ecore_IMF_Event_Delete_Surrounding* deleteSurroundingEvent = static_cast<Ecore_IMF_Event_Delete_Surrounding*>(eventInfo);

    Dali::Integration::InputMethodContext::EventData imfData(Dali::Integration::InputMethodContext::DELETE_SURROUNDING, Dali::String(), deleteSurroundingEvent->offset, deleteSurroundingEvent->n_chars);
    Dali::InputMethodContext                         handle(this);
    mEventSignal.Emit(handle, imfData);
    Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, imfData);

    if(callbackData.update)
    {
      mIMFCursorPosition = static_cast<int>(callbackData.cursorPosition);

      NotifyCursorPosition();
    }
  }
}

void InputMethodContextX::NotifyCursorPosition()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::NotifyCursorPosition\n");

  if(mIMFContext)
  {
    ecore_imf_context_cursor_position_set(mIMFContext, mIMFCursorPosition);
  }
}

void InputMethodContextX::SetCursorPosition(unsigned int cursorPosition)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::SetCursorPosition\n");

  mIMFCursorPosition = static_cast<int>(cursorPosition);
}

unsigned int InputMethodContextX::GetCursorPosition() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::GetCursorPosition\n");

  return static_cast<unsigned int>(mIMFCursorPosition);
}

void InputMethodContextX::SetSurroundingText(const Dali::String& text)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::SetSurroundingText\n");

  mSurroundingText = text;
}

Dali::String InputMethodContextX::GetSurroundingText() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::GetSurroundingText\n");

  return mSurroundingText;
}

void InputMethodContextX::NotifyTextInputMultiLine(bool multiLine)
{
}

Dali::Integration::InputMethodContext::TextDirection InputMethodContextX::GetTextDirection()
{
  Dali::Integration::InputMethodContext::TextDirection direction(Dali::Integration::InputMethodContext::LEFT_TO_RIGHT);

  if(mIMFContext)
  {
    char* locale(NULL);
    ecore_imf_context_input_panel_language_locale_get(mIMFContext, &locale);

    if(locale)
    {
      direction = static_cast<Dali::Integration::InputMethodContext::TextDirection>(Locale::GetDirection(std::string(locale)));
      free(locale);
    }
  }

  return direction;
}

BoundsInteger InputMethodContextX::GetInputPanelArea()
{
  int xPos, yPos, width, height;

  width = height = xPos = yPos = 0;

  if(mIMFContext)
  {
    ecore_imf_context_input_panel_geometry_get(mIMFContext, &xPos, &yPos, &width, &height);
  }
  else
  {
    DALI_LOG_ERROR("VKB Unable to get InputMethodContext Context so GetSize unavailable\n");
  }

  return BoundsInteger(xPos, yPos, width, height);
}

void InputMethodContextX::ApplyOptions(const Dali::Integration::InputMethodOptions& options)
{
  using namespace Dali::Integration::InputMethod::Category;

  int index;

  if(mIMFContext == NULL)
  {
    DALI_LOG_ERROR("VKB Unable to excute ApplyOptions with Null ImfContext\n");
    return;
  }

  if(mOptions.CompareAndSet(PANEL_LAYOUT, options, index))
  {
    SetEcoreInputPanelLayout(mIMFContext, mOptions.GetPanelLayout());
  }
  if(mOptions.CompareAndSet(BUTTON_ACTION, options, index))
  {
    ecore_imf_context_input_panel_return_key_type_set(mIMFContext, ToEcoreReturnKey(static_cast<Dali::InputMethod::ReturnKeyType>(index)));
  }
  if(mOptions.CompareAndSet(AUTO_CAPITALIZE, options, index))
  {
    ecore_imf_context_autocapital_type_set(mIMFContext, ToEcoreAutoCapital(static_cast<Dali::InputMethod::AutoCapitalType>(index)));
  }
  if(mOptions.CompareAndSet(VARIATION, options, index))
  {
    ecore_imf_context_input_panel_layout_variation_set(mIMFContext, ToPlatformLayoutVariation(mOptions.GetPanelLayoutVariation()));
  }
}

bool InputMethodContextX::SetInputPanelUserData(const Dali::String& data)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::SetInputPanelUserData\n");

  if(mIMFContext)
  {
    int length = static_cast<int>(data.Size());
    ecore_imf_context_input_panel_imdata_set(mIMFContext, data.CStr(), length);
  }

  return true;
}

Dali::String InputMethodContextX::GetInputPanelUserData() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::GetInputPanelUserData\n");

  if(mIMFContext)
  {
    int                length = 4096; // The max length is 4096 bytes
    Dali::Vector<char> buffer;
    buffer.Resize(length);
    ecore_imf_context_input_panel_imdata_get(mIMFContext, &buffer[0], &length);
    return Dali::Integration::ToDaliString(std::string(&buffer[0], length));
  }

  return Dali::String();
}

Dali::InputMethodContext::State InputMethodContextX::GetInputPanelState()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::GetInputPanelState\n");

  if(mIMFContext)
  {
    int value;
    value = ecore_imf_context_input_panel_state_get(mIMFContext);

    switch(value)
    {
      case ECORE_IMF_INPUT_PANEL_STATE_SHOW:
      {
        return Dali::InputMethodContext::State::SHOW;
        break;
      }

      case ECORE_IMF_INPUT_PANEL_STATE_HIDE:
      {
        return Dali::InputMethodContext::State::HIDE;
        break;
      }

      case ECORE_IMF_INPUT_PANEL_STATE_WILL_SHOW:
      {
        return Dali::InputMethodContext::State::WILL_SHOW;
        break;
      }

      default:
      {
        return Dali::InputMethodContext::State::HIDE;
      }
    }
  }
  return Dali::InputMethodContext::State::HIDE;
}

bool InputMethodContextX::SetReturnKeyState(bool visible)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::SetReturnKeyState\n");

  if(mIMFContext)
  {
    ecore_imf_context_input_panel_return_key_disabled_set(mIMFContext, !visible);
  }

  return true;
}

bool InputMethodContextX::IsReturnKeyEnabled() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::IsReturnKeyEnabled\n");

  if(mIMFContext)
  {
    return !ecore_imf_context_input_panel_return_key_disabled_get(mIMFContext);
  }

  return true;
}

bool InputMethodContextX::AutoEnableInputPanel(bool enabled)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::AutoEnableInputPanel\n");

  if(mIMFContext)
  {
    ecore_imf_context_input_panel_enabled_set(mIMFContext, enabled);
  }

  return true;
}

bool InputMethodContextX::ShowInputPanel()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::ShowInputPanel\n");

  if(mIMFContext)
  {
    ecore_imf_context_input_panel_show(mIMFContext);
    return true;
  }

  return false;
}

bool InputMethodContextX::HideInputPanel()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::HideInputPanel\n");

  if(mIMFContext)
  {
    ecore_imf_context_input_panel_hide(mIMFContext);
    return true;
  }

  return false;
}

Dali::InputMethodContext::KeyboardType InputMethodContextX::GetKeyboardType()
{
  return Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD;
}

bool InputMethodContextX::SetInputPanelLanguageLocale(const Dali::String& locale)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::SetInputPanelLanguageLocale\n");
  return false;
}

Dali::String InputMethodContextX::GetInputPanelLanguageLocale() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::GetInputPanelLanguageLocale\n");

  Dali::String locale;

  if(mIMFContext)
  {
    char* value = NULL;
    ecore_imf_context_input_panel_language_locale_get(mIMFContext, &value);

    if(value)
    {
      locale = Dali::String(value);

      // The locale string retrieved must be freed with free().
      free(value);
    }
  }
  return locale;
}

void InputMethodContextX::SetContentMimeTypes(const Dali::String& mimeTypes)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::SetContentMimeTypes\n");
  // ecore_imf_context_mime_type_accept_set() is supported from ecore-imf 1.20.0 version.
}

bool InputMethodContextX::FilterEventKey(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);

  // If a device key then skip ecore_imf_context_filter_event.
  if(!KeyLookup::IsDeviceButton(keyEvent.GetKeyName().CStr()))
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

bool InputMethodContextX::AllowTextPrediction(bool prediction)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::AllowTextPrediction\n");

  if(mIMFContext)
  {
    ecore_imf_context_prediction_allow_set(mIMFContext, prediction);
  }

  return true;
}

bool InputMethodContextX::IsTextPredictionAllowed() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::IsTextPredictionAllowed\n");
  bool prediction = false;
  if(mIMFContext)
  {
    prediction = ecore_imf_context_prediction_allow_get(mIMFContext);
  }
  return prediction;
}

bool InputMethodContextX::SetFullScreenMode(bool prediction)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::SetFullScreenMode\n");
  return true;
}

bool InputMethodContextX::IsFullScreenMode() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextGeneric::IsFullScreenMode\n");

  return false;
}

bool InputMethodContextX::SetInputPanelLanguage(Dali::Integration::InputMethodContext::InputPanelLanguage language)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::SetInputPanelLanguage\n");
  if(mIMFContext)
  {
    switch(language)
    {
      case Dali::Integration::InputMethodContext::InputPanelLanguage::AUTOMATIC:
      {
        ecore_imf_context_input_panel_language_set(mIMFContext, ECORE_IMF_INPUT_PANEL_LANG_AUTOMATIC);
        break;
      }
      case Dali::Integration::InputMethodContext::InputPanelLanguage::ALPHABET:
      {
        ecore_imf_context_input_panel_language_set(mIMFContext, ECORE_IMF_INPUT_PANEL_LANG_ALPHABET);
        break;
      }
    }
  }

  return true;
}

Dali::Integration::InputMethodContext::InputPanelLanguage InputMethodContextX::GetInputPanelLanguage() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::GetInputPanelLanguage\n");
  if(mIMFContext)
  {
    int value;
    value = ecore_imf_context_input_panel_language_get(mIMFContext);

    switch(value)
    {
      case ECORE_IMF_INPUT_PANEL_LANG_AUTOMATIC:
      {
        return Dali::Integration::InputMethodContext::InputPanelLanguage::AUTOMATIC;
        break;
      }
      case ECORE_IMF_INPUT_PANEL_LANG_ALPHABET:
      {
        return Dali::Integration::InputMethodContext::InputPanelLanguage::ALPHABET;
        break;
      }
    }
  }
  return Dali::Integration::InputMethodContext::InputPanelLanguage::AUTOMATIC;
}

bool InputMethodContextX::SetInputPanelPosition(unsigned int x, unsigned int y)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::SetInputPanelPosition\n");

  // ecore_imf_context_input_panel_position_set() is supported from ecore-imf 1.21.0 version.
  return true;
}

bool InputMethodContextX::SetInputPanelPositionAlign(int x, int y, Dali::InputMethodContext::InputPanelAlign align)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::SetInputPanelPositionAlign\n");
  return false;
}

bool InputMethodContextX::SetInputPanelLayout(Dali::InputMethod::PanelLayout layout)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::SetInputPanelLayout\n");

  if(mIMFContext)
  {
    SetEcoreInputPanelLayout(mIMFContext, layout);
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_LAYOUT] = std::bind(&InputMethodContextX::SetInputPanelLayout, this, layout);
  return true;
}

Dali::InputMethod::PanelLayout InputMethodContextX::GetInputPanelLayout() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::GetInputPanelLayout\n");

  if(mIMFContext)
  {
    return ToDaliPanelLayout(ecore_imf_context_input_panel_layout_get(mIMFContext));
  }

  return Dali::InputMethod::PanelLayout::NORMAL;
}

bool InputMethodContextX::SetInputPanelReturnKeyType(Dali::InputMethod::ReturnKeyType action)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::SetInputPanelReturnKeyType\n");

  if(mIMFContext)
  {
    ecore_imf_context_input_panel_return_key_type_set(mIMFContext, ToEcoreReturnKey(action));
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_RETURN_KEY] = std::bind(&InputMethodContextX::SetInputPanelReturnKeyType, this, action);
  return true;
}

Dali::InputMethod::ReturnKeyType InputMethodContextX::GetInputPanelReturnKeyType() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::GetInputPanelReturnKeyType\n");

  if(mIMFContext)
  {
    return ToDaliReturnKey(ecore_imf_context_input_panel_return_key_type_get(mIMFContext));
  }

  return Dali::InputMethod::ReturnKeyType::DEFAULT;
}

bool InputMethodContextX::SetInputPanelAutoCapitalType(Dali::InputMethod::AutoCapitalType autoCapital)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::SetInputPanelAutoCapitalType\n");

  if(mIMFContext)
  {
    ecore_imf_context_autocapital_type_set(mIMFContext, ToEcoreAutoCapital(autoCapital));
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_AUTO_CAPITAL] = std::bind(&InputMethodContextX::SetInputPanelAutoCapitalType, this, autoCapital);
  return true;
}

Dali::InputMethod::AutoCapitalType InputMethodContextX::GetInputPanelAutoCapitalType() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::GetInputPanelAutoCapitalType\n");

  if(mIMFContext)
  {
    return ToDaliAutoCapital(ecore_imf_context_autocapital_type_get(mIMFContext));
  }

  return Dali::InputMethod::AutoCapitalType::SENTENCE;
}

bool InputMethodContextX::SetInputPanelLayoutVariation(Dali::InputMethod::PanelLayoutVariation variation)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::SetInputPanelLayoutVariation\n");

  if(mIMFContext)
  {
    ecore_imf_context_input_panel_layout_variation_set(mIMFContext, ToPlatformLayoutVariation(variation));
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_LAYOUT_VARIATION] = std::bind(&InputMethodContextX::SetInputPanelLayoutVariation, this, variation);
  return true;
}

Dali::InputMethod::PanelLayoutVariation InputMethodContextX::GetInputPanelLayoutVariation() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::GetInputPanelLayoutVariation\n");

  if(mIMFContext)
  {
    const auto layout       = ToDaliPanelLayout(ecore_imf_context_input_panel_layout_get(mIMFContext));
    const int  rawVariation = ecore_imf_context_input_panel_layout_variation_get(mIMFContext);
    return ToPanelLayoutVariation(layout, rawVariation);
  }

  return Dali::InputMethod::PanelLayoutVariation::NORMAL_NORMAL;
}

void InputMethodContextX::GetPreeditStyle(Dali::Integration::InputMethodContext::PreEditAttributeDataContainer& attrs) const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextX::GetPreeditStyle\n");
  attrs = mPreeditAttrs;
}

bool InputMethodContextX::ProcessEventKeyDown(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);
  if(mIMFContext)
  {
    Integration::KeyEvent integKeyEvent(keyEvent.GetKeyName(), keyEvent.GetLogicalKey(), keyEvent.GetKeyString(), keyEvent.GetKeyCode(), keyEvent.GetKeyModifier(), keyEvent.GetTime(), static_cast<Integration::KeyEvent::State>(keyEvent.GetState()), keyEvent.GetCompose(), keyEvent.GetDeviceName(), keyEvent.GetDeviceClass(), keyEvent.GetDeviceSubclass());

    std::string key     = ToStdString(integKeyEvent.logicalKey);
    std::string compose = ToStdString(integKeyEvent.compose);

    // We're consuming key down event so we have to pass to InputMethodContext so that it can parse it as well.
    Ecore_IMF_Event_Key_Down ecoreKeyDownEvent;
    ecoreKeyDownEvent.keyname   = integKeyEvent.keyName.CStr();
    ecoreKeyDownEvent.key       = key.c_str();
    ecoreKeyDownEvent.string    = integKeyEvent.keyString.CStr();
    ecoreKeyDownEvent.compose   = compose.c_str();
    ecoreKeyDownEvent.timestamp = integKeyEvent.time;
    ecoreKeyDownEvent.modifiers = EcoreInputModifierToEcoreIMFModifier(integKeyEvent.keyModifier);
    ecoreKeyDownEvent.locks     = EcoreInputModifierToEcoreIMFLock(integKeyEvent.keyModifier);

#if defined(ECORE_VERSION_MAJOR) && (ECORE_VERSION_MAJOR >= 1) && defined(ECORE_VERSION_MINOR)
#if (ECORE_VERSION_MINOR >= 14)
    ecoreKeyDownEvent.dev_name     = "";
    ecoreKeyDownEvent.dev_class    = ECORE_IMF_DEVICE_CLASS_KEYBOARD;
    ecoreKeyDownEvent.dev_subclass = ECORE_IMF_DEVICE_SUBCLASS_NONE;
#endif // Since ecore_imf 1.14 version
#if (ECORE_VERSION_MINOR >= 22)
    ecoreKeyDownEvent.keycode = integKeyEvent.keyCode;
#endif // Since ecore_imf 1.22 version
#endif // Since ecore_imf Version 1

    // If the device is IME and the focused key is the direction keys, then we should send a key event to move a key cursor.
    if((integKeyEvent.deviceName == "ime") && ((!strncmp(integKeyEvent.keyName.CStr(), "Left", 4)) ||
                                               (!strncmp(integKeyEvent.keyName.CStr(), "Right", 5)) ||
                                               (!strncmp(integKeyEvent.keyName.CStr(), "Up", 2)) ||
                                               (!strncmp(integKeyEvent.keyName.CStr(), "Down", 4))))
    {
      eventHandled = 0;
    }
    else
    {
      eventHandled = ecore_imf_context_filter_event(mIMFContext,
                                                    ECORE_IMF_EVENT_KEY_DOWN,
                                                    reinterpret_cast<Ecore_IMF_Event*>(&ecoreKeyDownEvent));
    }

    // If the event has not been handled by InputMethodContext then check if we should reset our IMFcontext
    if(!eventHandled)
    {
      if(!strcmp(integKeyEvent.keyName.CStr(), "Escape") ||
         !strcmp(integKeyEvent.keyName.CStr(), "Return") ||
         !strcmp(integKeyEvent.keyName.CStr(), "KP_Enter"))
      {
        ecore_imf_context_reset(mIMFContext);
      }
    }
  }
  return eventHandled;
}

bool InputMethodContextX::ProcessEventKeyUp(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);
  if(mIMFContext)
  {
    Integration::KeyEvent integKeyEvent(keyEvent.GetKeyName(), keyEvent.GetLogicalKey(), keyEvent.GetKeyString(), keyEvent.GetKeyCode(), keyEvent.GetKeyModifier(), keyEvent.GetTime(), static_cast<Integration::KeyEvent::State>(keyEvent.GetState()), keyEvent.GetCompose(), keyEvent.GetDeviceName(), keyEvent.GetDeviceClass(), keyEvent.GetDeviceSubclass());

    String key     = integKeyEvent.logicalKey;
    String compose = integKeyEvent.compose;

    // We're consuming key up event so we have to pass to InputMethodContext so that it can parse it as well.
    Ecore_IMF_Event_Key_Up ecoreKeyUpEvent;
    ecoreKeyUpEvent.keyname   = integKeyEvent.keyName.CStr();
    ecoreKeyUpEvent.key       = key.CStr();
    ecoreKeyUpEvent.string    = integKeyEvent.keyString.CStr();
    ecoreKeyUpEvent.compose   = compose.CStr();
    ecoreKeyUpEvent.timestamp = integKeyEvent.time;
    ecoreKeyUpEvent.modifiers = EcoreInputModifierToEcoreIMFModifier(integKeyEvent.keyModifier);
    ecoreKeyUpEvent.locks     = EcoreInputModifierToEcoreIMFLock(integKeyEvent.keyModifier);
#if defined(ECORE_VERSION_MAJOR) && (ECORE_VERSION_MAJOR >= 1) && defined(ECORE_VERSION_MINOR)
#if (ECORE_VERSION_MINOR >= 14)
    ecoreKeyUpEvent.dev_name = "";
#endif // Since ecore_imf 1.14 version
#if (ECORE_VERSION_MINOR >= 22)
    ecoreKeyUpEvent.keycode = integKeyEvent.keyCode;
#endif // Since ecore_imf 1.22 version
#endif // Since ecore_imf Version 1

    eventHandled = ecore_imf_context_filter_event(mIMFContext,
                                                  ECORE_IMF_EVENT_KEY_UP,
                                                  reinterpret_cast<Ecore_IMF_Event*>(&ecoreKeyUpEvent));
  }
  return eventHandled;
}

Ecore_IMF_Keyboard_Modifiers InputMethodContextX::EcoreInputModifierToEcoreIMFModifier(unsigned int ecoreModifier)
{
  unsigned int modifier(ECORE_IMF_KEYBOARD_MODIFIER_NONE); // If no other matches returns NONE.

  if(ecoreModifier & ECORE_EVENT_MODIFIER_SHIFT) // enums from ecore_input/Ecore_Input.h
  {
    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_SHIFT; // enums from ecore_imf/ecore_imf.h
  }

  if(ecoreModifier & ECORE_EVENT_MODIFIER_ALT)
  {
    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_ALT;
  }

  if(ecoreModifier & ECORE_EVENT_MODIFIER_CTRL)
  {
    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_CTRL;
  }

  if(ecoreModifier & ECORE_EVENT_MODIFIER_WIN)
  {
    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_WIN;
  }

  if(ecoreModifier & ECORE_EVENT_MODIFIER_ALTGR)
  {
    modifier |= ECORE_IMF_KEYBOARD_MODIFIER_ALTGR;
  }

  return static_cast<Ecore_IMF_Keyboard_Modifiers>(modifier);
}

Ecore_IMF_Keyboard_Locks InputMethodContextX::EcoreInputModifierToEcoreIMFLock(unsigned int modifier)
{
  unsigned int lock(ECORE_IMF_KEYBOARD_LOCK_NONE); // If no other matches, returns NONE.

  if(modifier & ECORE_EVENT_LOCK_NUM)
  {
    lock |= ECORE_IMF_KEYBOARD_LOCK_NUM; // Num lock is active.
  }

  if(modifier & ECORE_EVENT_LOCK_CAPS)
  {
    lock |= ECORE_IMF_KEYBOARD_LOCK_CAPS; // Caps lock is active.
  }

  if(modifier & ECORE_EVENT_LOCK_SCROLL)
  {
    lock |= ECORE_IMF_KEYBOARD_LOCK_SCROLL; // Scroll lock is active.
  }

  return static_cast<Ecore_IMF_Keyboard_Locks>(lock);
}

void InputMethodContextX::OnStaged(Dali::Actor actor)
{
  Ecore_X_Window ecoreXwin(AnyCast<Ecore_X_Window>(Dali::Integration::SceneHolder::Get(actor).GetNativeHandle()));

  if(mEcoreXwin != ecoreXwin)
  {
    mEcoreXwin = ecoreXwin;

    // Reset
    Finalize();
    Initialize();
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
