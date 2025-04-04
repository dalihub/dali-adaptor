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

#include <dali/internal/input/tizen-wayland/input-method-context-impl-ecore-wl.h>

// EXTERNAL INCLUDES
#include <Ecore_Input.h>
#include <Ecore_Wl2.h>
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/key.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/object/type-registry.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>
#include <dali/internal/input/common/key-impl.h>
#include <dali/internal/system/common/locale-utils.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/public-api/adaptor-framework/input-method.h>

Ecore_IMF_Input_Panel_Layout panelLayoutMap[] =
  {
    ECORE_IMF_INPUT_PANEL_LAYOUT_NORMAL,
    ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBER,
    ECORE_IMF_INPUT_PANEL_LAYOUT_EMAIL,
    ECORE_IMF_INPUT_PANEL_LAYOUT_URL,
    ECORE_IMF_INPUT_PANEL_LAYOUT_PHONENUMBER,
    ECORE_IMF_INPUT_PANEL_LAYOUT_IP,
    ECORE_IMF_INPUT_PANEL_LAYOUT_MONTH,
    ECORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY,
    ECORE_IMF_INPUT_PANEL_LAYOUT_HEX,
    ECORE_IMF_INPUT_PANEL_LAYOUT_TERMINAL,
    ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD,
    ECORE_IMF_INPUT_PANEL_LAYOUT_DATETIME,
    ECORE_IMF_INPUT_PANEL_LAYOUT_EMOTICON,
    ECORE_IMF_INPUT_PANEL_LAYOUT_VOICE};

Ecore_IMF_Autocapital_Type autoCapitalMap[] =
  {
    ECORE_IMF_AUTOCAPITAL_TYPE_NONE,
    ECORE_IMF_AUTOCAPITAL_TYPE_WORD,
    ECORE_IMF_AUTOCAPITAL_TYPE_SENTENCE,
    ECORE_IMF_AUTOCAPITAL_TYPE_ALLCHARACTER,
};

Ecore_IMF_Input_Panel_Return_Key_Type returnKeyTypeMap[] =
  {
    ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT,
    ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DONE,
    ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_GO,
    ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_JOIN,
    ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_LOGIN,
    ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_NEXT,
    ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH,
    ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEND,
    ECORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SIGNIN};

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

const int kUninitializedWindowId = 0;

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

size_t GetNumberOfUtf8Characters(const char* utf8, size_t length)
{
  size_t numberOfCharacters = 0u;

  const uint8_t* begin = reinterpret_cast<const uint8_t*>(utf8);
  const uint8_t* end   = reinterpret_cast<const uint8_t*>(utf8 + length);

  while(begin < end)
  {
    size_t sequenceLength = Utf8SequenceLength(static_cast<unsigned char>(*begin));
    if(sequenceLength == 0u)
    {
      // Invalid case, return zero to avoid infinity loop.
      return 0u;
    }
    begin += sequenceLength;
    numberOfCharacters++;
  }
  return numberOfCharacters;
}

// Static function calls used by ecore 'c' style callback registration
void Commit(void* data, Ecore_IMF_Context* imfContext, void* eventInfo)
{
  if(data)
  {
    InputMethodContextEcoreWl* inputMethodContext = static_cast<InputMethodContextEcoreWl*>(data);
    inputMethodContext->CommitReceived(data, imfContext, eventInfo);
  }
}

void PreEdit(void* data, Ecore_IMF_Context* imfContext, void* eventInfo)
{
  if(data)
  {
    InputMethodContextEcoreWl* inputMethodContext = static_cast<InputMethodContextEcoreWl*>(data);
    inputMethodContext->PreEditChanged(data, imfContext, eventInfo);
  }
}

Eina_Bool ImfRetrieveSurrounding(void* data, Ecore_IMF_Context* imfContext, char** text, int* cursorPosition)
{
  if(data)
  {
    InputMethodContextEcoreWl* inputMethodContext = static_cast<InputMethodContextEcoreWl*>(data);
    return inputMethodContext->RetrieveSurrounding(data, imfContext, text, cursorPosition);
  }
  else
  {
    return false;
  }
}

void InputPanelStateChangeCallback(void* data, Ecore_IMF_Context* context, int value)
{
  if(!data)
  {
    return;
  }
  InputMethodContextEcoreWl* inputMethodContext = static_cast<InputMethodContextEcoreWl*>(data);
  switch(value)
  {
    case ECORE_IMF_INPUT_PANEL_STATE_SHOW:
    {
      inputMethodContext->StatusChangedSignal().Emit(true);
      break;
    }

    case ECORE_IMF_INPUT_PANEL_STATE_HIDE:
    {
      inputMethodContext->StatusChangedSignal().Emit(false);
      break;
    }

    case ECORE_IMF_INPUT_PANEL_STATE_WILL_SHOW:
    default:
    {
      // Do nothing
      break;
    }
  }
}

void InputPanelLanguageChangeCallback(void* data, Ecore_IMF_Context* context, int value)
{
  if(!data)
  {
    return;
  }
  InputMethodContextEcoreWl* inputMethodContext = static_cast<InputMethodContextEcoreWl*>(data);
  // Emit the signal that the language has changed
  inputMethodContext->LanguageChangedSignal().Emit(value);
}

void InputPanelGeometryChangedCallback(void* data, Ecore_IMF_Context* context, int value)
{
  if(!data)
  {
    return;
  }
  InputMethodContextEcoreWl* inputMethodContext = static_cast<InputMethodContextEcoreWl*>(data);
  // Emit signal that the keyboard is resized
  inputMethodContext->ResizedSignal().Emit(value);
}

void InputPanelKeyboardTypeChangedCallback(void* data, Ecore_IMF_Context* context, int value)
{
  if(!data)
  {
    return;
  }

  InputMethodContextEcoreWl* inputMethodContext = static_cast<InputMethodContextEcoreWl*>(data);
  switch(value)
  {
    case ECORE_IMF_INPUT_PANEL_SW_KEYBOARD_MODE:
    {
      // Emit Signal that the keyboard type is changed to Software Keyboard
      inputMethodContext->KeyboardTypeChangedSignal().Emit(Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD);
      break;
    }
    case ECORE_IMF_INPUT_PANEL_HW_KEYBOARD_MODE:
    {
      // Emit Signal that the keyboard type is changed to Hardware Keyboard
      inputMethodContext->KeyboardTypeChangedSignal().Emit(Dali::InputMethodContext::KeyboardType::HARDWARE_KEYBOARD);
      break;
    }
  }
}

/**
 * Called when an IMF delete surrounding event is received.
 * Here we tell the application that it should delete a certain range.
 */
void ImfDeleteSurrounding(void* data, Ecore_IMF_Context* imfContext, void* eventInfo)
{
  if(data)
  {
    InputMethodContextEcoreWl* inputMethodContext = static_cast<InputMethodContextEcoreWl*>(data);
    inputMethodContext->DeleteSurrounding(data, imfContext, eventInfo);
  }
}

/**
 * Called when the input method sends a private command.
 */
void PrivateCommand(void* data, Ecore_IMF_Context* imfContext, void* eventInfo)
{
  if(data)
  {
    InputMethodContextEcoreWl* inputMethodContext = static_cast<InputMethodContextEcoreWl*>(data);
    inputMethodContext->SendPrivateCommand(data, imfContext, eventInfo);
  }
}

/**
 * Called when the input method commits content, such as an image.
 */
void CommitContent(void* data, Ecore_IMF_Context* imfContext, void* eventInfo)
{
  if(data)
  {
    InputMethodContextEcoreWl* inputMethodContext = static_cast<InputMethodContextEcoreWl*>(data);
    inputMethodContext->SendCommitContent(data, imfContext, eventInfo);
  }
}

/**
 * Called when the input method sends a selection set.
 */
void SelectionSet(void* data, Ecore_IMF_Context* imfContext, void* eventInfo)
{
  if(data)
  {
    InputMethodContextEcoreWl* inputMethodContext = static_cast<InputMethodContextEcoreWl*>(data);
    inputMethodContext->SendSelectionSet(data, imfContext, eventInfo);
  }
}

int GetWindowIdFromActor(Dali::Actor actor)
{
  int windowId = kUninitializedWindowId;

  if(actor.GetProperty<bool>(Dali::Actor::Property::CONNECTED_TO_SCENE))
  {
    auto sceneHolder = Dali::Integration::SceneHolder::Get(actor);
    if(DALI_LIKELY(sceneHolder))
    {
      Any nativeWindowHandle = sceneHolder.GetNativeHandle();
      windowId               = ecore_wl2_window_id_get(AnyCast<Ecore_Wl2_Window*>(nativeWindowHandle));
    }
  }

  return windowId;
}

BaseHandle Create()
{
  return Dali::InputMethodContext::New(Dali::Actor());
}

Dali::TypeRegistration type(typeid(Dali::InputMethodContext), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

InputMethodContextPtr InputMethodContextEcoreWl::New(Dali::Actor actor)
{
  InputMethodContextPtr inputMethodContext;

  // Create instance only if the adaptor is available and the valid actor exists
  if(actor && Dali::Adaptor::IsAvailable())
  {
    inputMethodContext = new InputMethodContextEcoreWl(actor);
  }
  return inputMethodContext;
}

void InputMethodContextEcoreWl::Finalize()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContext::Finalize\n");

  DisconnectCallbacks();
  DeleteContext();
}

InputMethodContextEcoreWl::InputMethodContextEcoreWl(Dali::Actor actor)
: mIMFContext(),
  mIMFCursorPosition(0),
  mSurroundingText(),
  mRestoreAfterFocusLost(false),
  mIdleCallbackConnected(false),
  mWindowId(GetWindowIdFromActor(actor))
{
  ecore_imf_init();

  actor.OnSceneSignal().Connect(this, &InputMethodContextEcoreWl::OnStaged);
}

InputMethodContextEcoreWl::~InputMethodContextEcoreWl()
{
  Finalize();
  try
  {
    ecore_imf_shutdown();
  }
  catch(std::bad_weak_ptr const& ex)
  {
    DALI_LOG_ERROR("InputMethodContextEcoreWl::~InputMethodContextEcoreWl() - std::bad_weak_ptr caught: %s\n", ex.what());
  }
}

void InputMethodContextEcoreWl::Initialize()
{
  CreateContext();
  ConnectCallbacks();
  ApplyBackupOperations();
}

void InputMethodContextEcoreWl::CreateContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContext::CreateContext\n");

  if(mWindowId == kUninitializedWindowId)
  {
    return;
  }

  const char* contextId = ecore_imf_context_default_id_get();
  if(contextId)
  {
    mIMFContext = ecore_imf_context_add(contextId);

    if(mIMFContext)
    {
      ecore_imf_context_client_window_set(mIMFContext, reinterpret_cast<void*>(mWindowId));
    }
    else
    {
      DALI_LOG_ERROR("InputMethodContext Unable to get IMFContext (contextId:%p)\n", contextId);
    }
  }
  else
  {
    DALI_LOG_ERROR("InputMethodContext Unable to get IMFContext (contextId:(null))\n");
  }
}

void InputMethodContextEcoreWl::DeleteContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::DeleteContext\n");

  if(mIMFContext)
  {
    ecore_imf_context_del(mIMFContext);
    mIMFContext = NULL;
  }
}

// Callbacks for predicitive text support.
void InputMethodContextEcoreWl::ConnectCallbacks()
{
  if(mIMFContext)
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::ConnectCallbacks\n");

    ecore_imf_context_event_callback_add(mIMFContext, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, PreEdit, this);
    ecore_imf_context_event_callback_add(mIMFContext, ECORE_IMF_CALLBACK_COMMIT, Commit, this);
    ecore_imf_context_event_callback_add(mIMFContext, ECORE_IMF_CALLBACK_DELETE_SURROUNDING, ImfDeleteSurrounding, this);
    ecore_imf_context_event_callback_add(mIMFContext, ECORE_IMF_CALLBACK_PRIVATE_COMMAND_SEND, PrivateCommand, this);
    ecore_imf_context_event_callback_add(mIMFContext, ECORE_IMF_CALLBACK_COMMIT_CONTENT, CommitContent, this);
    ecore_imf_context_event_callback_add(mIMFContext, ECORE_IMF_CALLBACK_SELECTION_SET, SelectionSet, this);

    ecore_imf_context_input_panel_event_callback_add(mIMFContext, ECORE_IMF_INPUT_PANEL_STATE_EVENT, InputPanelStateChangeCallback, this);
    ecore_imf_context_input_panel_event_callback_add(mIMFContext, ECORE_IMF_INPUT_PANEL_LANGUAGE_EVENT, InputPanelLanguageChangeCallback, this);
    ecore_imf_context_input_panel_event_callback_add(mIMFContext, ECORE_IMF_INPUT_PANEL_GEOMETRY_EVENT, InputPanelGeometryChangedCallback, this);
    ecore_imf_context_input_panel_event_callback_add(mIMFContext, ECORE_IMF_INPUT_PANEL_KEYBOARD_MODE_EVENT, InputPanelKeyboardTypeChangedCallback, this);

    ecore_imf_context_retrieve_surrounding_callback_set(mIMFContext, ImfRetrieveSurrounding, this);
  }
}

void InputMethodContextEcoreWl::DisconnectCallbacks()
{
  if(mIMFContext)
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::DisconnectCallbacks\n");

    ecore_imf_context_event_callback_del(mIMFContext, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, PreEdit);
    ecore_imf_context_event_callback_del(mIMFContext, ECORE_IMF_CALLBACK_COMMIT, Commit);
    ecore_imf_context_event_callback_del(mIMFContext, ECORE_IMF_CALLBACK_DELETE_SURROUNDING, ImfDeleteSurrounding);
    ecore_imf_context_event_callback_del(mIMFContext, ECORE_IMF_CALLBACK_PRIVATE_COMMAND_SEND, PrivateCommand);
    ecore_imf_context_event_callback_del(mIMFContext, ECORE_IMF_CALLBACK_COMMIT_CONTENT, CommitContent);
    ecore_imf_context_event_callback_del(mIMFContext, ECORE_IMF_CALLBACK_SELECTION_SET, SelectionSet);

    ecore_imf_context_input_panel_event_callback_del(mIMFContext, ECORE_IMF_INPUT_PANEL_STATE_EVENT, InputPanelStateChangeCallback);
    ecore_imf_context_input_panel_event_callback_del(mIMFContext, ECORE_IMF_INPUT_PANEL_LANGUAGE_EVENT, InputPanelLanguageChangeCallback);
    ecore_imf_context_input_panel_event_callback_del(mIMFContext, ECORE_IMF_INPUT_PANEL_GEOMETRY_EVENT, InputPanelGeometryChangedCallback);
    ecore_imf_context_input_panel_event_callback_del(mIMFContext, ECORE_IMF_INPUT_PANEL_KEYBOARD_MODE_EVENT, InputPanelKeyboardTypeChangedCallback);

    // We do not need to unset the retrieve surrounding callback.
  }
}

void InputMethodContextEcoreWl::Activate()
{
  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;

  if(mIMFContext)
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::Activate\n");

    ecore_imf_context_focus_in(mIMFContext);

    // emit keyboard activated signal
    Dali::InputMethodContext handle(this);
    mActivatedSignal.Emit(handle);
  }
}

void InputMethodContextEcoreWl::Deactivate()
{
  if(mIMFContext)
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::Deactivate\n");

    Reset();
    ecore_imf_context_focus_out(mIMFContext);
  }

  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;
}

void InputMethodContextEcoreWl::Reset()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::Reset\n");

  if(mIMFContext)
  {
    ecore_imf_context_reset(mIMFContext);
  }
}

ImfContext* InputMethodContextEcoreWl::GetContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetContext\n");

  return mIMFContext;
}

bool InputMethodContextEcoreWl::RestoreAfterFocusLost() const
{
  return mRestoreAfterFocusLost;
}

void InputMethodContextEcoreWl::SetRestoreAfterFocusLost(bool toggle)
{
  mRestoreAfterFocusLost = toggle;
}

/**
 * Called when an InputMethodContext Pre-Edit changed event is received.
 * We are still predicting what the user is typing.  The latest string is what the InputMethodContext module thinks
 * the user wants to type.
 */
void InputMethodContextEcoreWl::PreEditChanged(void*, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::PreEditChanged\n");
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
      Dali::InputMethodContext::PreeditAttributeData data;
      data.startIndex = 0;
      data.endIndex   = 0;

      uint32_t visualCharacterIndex = 0;
      size_t   byteIndex            = 0;

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
          data.preeditType = Dali::InputMethodContext::PreeditStyle::NONE;
          break;
        }
        case ECORE_IMF_PREEDIT_TYPE_SUB1:
        {
          data.preeditType = Dali::InputMethodContext::PreeditStyle::UNDERLINE;
          break;
        }
        case ECORE_IMF_PREEDIT_TYPE_SUB2:
        {
          data.preeditType = Dali::InputMethodContext::PreeditStyle::REVERSE;
          break;
        }
        case ECORE_IMF_PREEDIT_TYPE_SUB3:
        {
          data.preeditType = Dali::InputMethodContext::PreeditStyle::HIGHLIGHT;
          break;
        }
        case ECORE_IMF_PREEDIT_TYPE_SUB4:
        {
          data.preeditType = Dali::InputMethodContext::PreeditStyle::CUSTOM_PLATFORM_STYLE_1;
          break;
        }
        case ECORE_IMF_PREEDIT_TYPE_SUB5:
        {
          data.preeditType = Dali::InputMethodContext::PreeditStyle::CUSTOM_PLATFORM_STYLE_2;
          break;
        }
        case ECORE_IMF_PREEDIT_TYPE_SUB6:
        {
          data.preeditType = Dali::InputMethodContext::PreeditStyle::CUSTOM_PLATFORM_STYLE_3;
          break;
        }
        case ECORE_IMF_PREEDIT_TYPE_SUB7:
        {
          data.preeditType = Dali::InputMethodContext::PreeditStyle::CUSTOM_PLATFORM_STYLE_4;
          break;
        }
        default:
        {
          data.preeditType = Dali::InputMethodContext::PreeditStyle::NONE;
          break;
        }
      }
      mPreeditAttrs.PushBack(data);
    }
  }

  if(Dali::Adaptor::IsAvailable())
  {
    Dali::InputMethodContext            handle(this);
    Dali::InputMethodContext::EventData eventData(Dali::InputMethodContext::PRE_EDIT, preEditString, cursorPosition, 0);
    mEventSignal.Emit(handle, eventData);
    Dali::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, eventData);

    if(callbackData.update)
    {
      SetCursorPosition(callbackData.cursorPosition);
      SetSurroundingText(callbackData.currentText);

      NotifyCursorPosition();
    }

    if(callbackData.preeditResetRequired)
    {
      Reset();
    }
  }
  free(preEditString);
}

void InputMethodContextEcoreWl::CommitReceived(void*, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::CommitReceived\n");

  if(Dali::Adaptor::IsAvailable())
  {
    const std::string keyString(static_cast<char*>(eventInfo));

    Dali::InputMethodContext            handle(this);
    Dali::InputMethodContext::EventData eventData(Dali::InputMethodContext::COMMIT, keyString, 0, 0);
    mEventSignal.Emit(handle, eventData);
    Dali::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, eventData);

    if(callbackData.update)
    {
      SetCursorPosition(callbackData.cursorPosition);
      SetSurroundingText(callbackData.currentText);

      NotifyCursorPosition();
    }
  }
}

/**
 * Called when an InputMethodContext retrieve surround event is received.
 * Here the InputMethodContext module wishes to know the string we are working with and where within the string the cursor is
 * We need to signal the application to tell us this information.
 */
bool InputMethodContextEcoreWl::RetrieveSurrounding(void* data, ImfContext* imfContext, char** text, int* cursorPosition)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::RetrieveSurrounding\n");

  Dali::InputMethodContext::EventData imfData(Dali::InputMethodContext::GET_SURROUNDING, std::string(), 0, 0);
  Dali::InputMethodContext            handle(this);
  mEventSignal.Emit(handle, imfData);
  Dali::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, imfData);

  if(callbackData.update)
  {
    if(cursorPosition)
    {
      mIMFCursorPosition = static_cast<int>(callbackData.cursorPosition);
      *cursorPosition    = mIMFCursorPosition;
    }

    if(text)
    {
      const char* plainText = callbackData.currentText.c_str();

      if(plainText)
      {
        // If the current input panel is password mode, dali should replace the plain text with '*' (Asterisk) character.
        if(ecore_imf_context_input_hint_get(mIMFContext) & ECORE_IMF_INPUT_HINT_SENSITIVE_DATA)
        {
          size_t textLength = callbackData.currentText.length();
          size_t utf8Length = GetNumberOfUtf8Characters(plainText, textLength);
          if(textLength > 0u && utf8Length == 0u)
          {
            DALI_LOG_ERROR("Invalid utf8 characters, utf8 len:%zu, text len:%zu, text:%s\n", utf8Length, textLength, plainText);
            return EINA_FALSE;
          }

          std::string asterisks(utf8Length, '*');
          *text = strdup(asterisks.c_str());
        }
        else
        {
          // The memory allocated by strdup() can be freed by ecore_imf_context_surrounding_get() internally.
          *text = strdup(plainText);
        }

        return EINA_TRUE;
      }
    }
  }

  return EINA_FALSE;
}

/**
 * Called when an InputMethodContext delete surrounding event is received.
 * Here we tell the application that it should delete a certain range.
 */
void InputMethodContextEcoreWl::DeleteSurrounding(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::DeleteSurrounding\n");

  if(Dali::Adaptor::IsAvailable())
  {
    Ecore_IMF_Event_Delete_Surrounding* deleteSurroundingEvent = static_cast<Ecore_IMF_Event_Delete_Surrounding*>(eventInfo);

    Dali::InputMethodContext::EventData imfData(Dali::InputMethodContext::DELETE_SURROUNDING, std::string(), deleteSurroundingEvent->offset, deleteSurroundingEvent->n_chars);
    Dali::InputMethodContext            handle(this);
    mEventSignal.Emit(handle, imfData);
    mKeyboardEventSignal.Emit(handle, imfData);
  }
}

/**
 * Called when the input method sends a private command.
 */
void InputMethodContextEcoreWl::SendPrivateCommand(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::SendPrivateCommand\n");

  if(Dali::Adaptor::IsAvailable())
  {
    const char* privateCommandSendEvent = static_cast<const char*>(eventInfo);

    Dali::InputMethodContext::EventData imfData(Dali::InputMethodContext::PRIVATE_COMMAND, privateCommandSendEvent, 0, 0);
    Dali::InputMethodContext            handle(this);
    mEventSignal.Emit(handle, imfData);
    mKeyboardEventSignal.Emit(handle, imfData);
  }
}

/**
 * Called when the input method commits content, such as an image.
 */
void InputMethodContextEcoreWl::SendCommitContent(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::SendCommitContent\n");

  if(Dali::Adaptor::IsAvailable())
  {
    Ecore_IMF_Event_Commit_Content* commitContent = static_cast<Ecore_IMF_Event_Commit_Content*>(eventInfo);
    if(commitContent)
    {
      DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::SendCommitContent commit content : %s, description : %s, mime type : %s\n", commitContent->content_uri, commitContent->description, commitContent->mime_types);
      mContentReceivedSignal.Emit(commitContent->content_uri, commitContent->description, commitContent->mime_types);
    }
  }
}

/**
 * Called when the input method selection set.
 */
void InputMethodContextEcoreWl::SendSelectionSet(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::SendCommitContent\n");

  if(Dali::Adaptor::IsAvailable())
  {
    Ecore_IMF_Event_Selection* selection = static_cast<Ecore_IMF_Event_Selection*>(eventInfo);
    if(selection)
    {
      DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::SendSelectionSet selection start index : %d, end index : %d\n", selection->start, selection->end);
      Dali::InputMethodContext::EventData imfData(Dali::InputMethodContext::SELECTION_SET, selection->start, selection->end);
      Dali::InputMethodContext            handle(this);
      mEventSignal.Emit(handle, imfData);
      mKeyboardEventSignal.Emit(handle, imfData);
    }
  }
}

void InputMethodContextEcoreWl::NotifyCursorPosition()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::NotifyCursorPosition\n");

  if(mIMFContext)
  {
    ecore_imf_context_cursor_position_set(mIMFContext, mIMFCursorPosition);
  }
}

void InputMethodContextEcoreWl::SetCursorPosition(unsigned int cursorPosition)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::SetCursorPosition\n");

  mIMFCursorPosition = static_cast<int>(cursorPosition);
}

unsigned int InputMethodContextEcoreWl::GetCursorPosition() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetCursorPosition\n");

  return static_cast<unsigned int>(mIMFCursorPosition);
}

void InputMethodContextEcoreWl::SetSurroundingText(const std::string& text)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::SetSurroundingText\n");

  mSurroundingText = text;
}

const std::string& InputMethodContextEcoreWl::GetSurroundingText() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetSurroundingText\n");

  return mSurroundingText;
}

void InputMethodContextEcoreWl::NotifyTextInputMultiLine(bool multiLine)
{
  if(mIMFContext)
  {
    Ecore_IMF_Input_Hints currentHint = ecore_imf_context_input_hint_get(mIMFContext);
    ecore_imf_context_input_hint_set(mIMFContext,
                                     static_cast<Ecore_IMF_Input_Hints>(multiLine ? (currentHint | ECORE_IMF_INPUT_HINT_MULTILINE) : (currentHint & ~ECORE_IMF_INPUT_HINT_MULTILINE)));
  }

  mBackupOperations[Operation::NOTIFY_TEXT_INPUT_MULTILINE] = std::bind(&InputMethodContextEcoreWl::NotifyTextInputMultiLine, this, multiLine);
}

Dali::InputMethodContext::TextDirection InputMethodContextEcoreWl::GetTextDirection()
{
  Dali::InputMethodContext::TextDirection direction(Dali::InputMethodContext::LEFT_TO_RIGHT);

  if(mIMFContext)
  {
    char* locale(NULL);
    ecore_imf_context_input_panel_language_locale_get(mIMFContext, &locale);

    if(locale)
    {
      direction = static_cast<Dali::InputMethodContext::TextDirection>(Locale::GetDirection(std::string(locale)));
      free(locale);
    }
  }

  return direction;
}

Rect<int> InputMethodContextEcoreWl::GetInputMethodArea()
{
  int xPos, yPos, width, height;

  width = height = xPos = yPos = 0;

  if(mIMFContext)
  {
    ecore_imf_context_input_panel_geometry_get(mIMFContext, &xPos, &yPos, &width, &height);
  }
  else
  {
    DALI_LOG_ERROR("VKB Unable to get IMFContext so GetSize unavailable\n");
    // return 0 as real size unknown.
  }

  return Rect<int>(xPos, yPos, width, height);
}

void InputMethodContextEcoreWl::ApplyOptions(const InputMethodOptions& options)
{
  using namespace Dali::InputMethod::Category;

  int index;

  if(mIMFContext == NULL)
  {
    DALI_LOG_ERROR("VKB Unable to excute ApplyOptions with Null ImfContext\n");
    return;
  }

  if(mOptions.CompareAndSet(PANEL_LAYOUT, options, index))
  {
    ecore_imf_context_input_panel_layout_set(mIMFContext, panelLayoutMap[index]);

    // Sets the input hint which allows input methods to fine-tune their behavior.
    if(panelLayoutMap[index] == ECORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD)
    {
      ecore_imf_context_input_hint_set(mIMFContext, static_cast<Ecore_IMF_Input_Hints>(ecore_imf_context_input_hint_get(mIMFContext) | ECORE_IMF_INPUT_HINT_SENSITIVE_DATA));
    }
    else
    {
      ecore_imf_context_input_hint_set(mIMFContext, static_cast<Ecore_IMF_Input_Hints>(ecore_imf_context_input_hint_get(mIMFContext) & ~ECORE_IMF_INPUT_HINT_SENSITIVE_DATA));
    }
  }
  if(mOptions.CompareAndSet(BUTTON_ACTION, options, index))
  {
    ecore_imf_context_input_panel_return_key_type_set(mIMFContext, returnKeyTypeMap[index]);
  }
  if(mOptions.CompareAndSet(AUTO_CAPITALIZE, options, index))
  {
    ecore_imf_context_autocapital_type_set(mIMFContext, autoCapitalMap[index]);
  }
  if(mOptions.CompareAndSet(VARIATION, options, index))
  {
    ecore_imf_context_input_panel_layout_variation_set(mIMFContext, index);
  }
}

void InputMethodContextEcoreWl::SetInputPanelData(const std::string& data)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::SetInputPanelData\n");

  if(mIMFContext)
  {
    int length = data.length();
    ecore_imf_context_input_panel_imdata_set(mIMFContext, data.c_str(), length);
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_DATA] = std::bind(&InputMethodContextEcoreWl::SetInputPanelData, this, data);
}

void InputMethodContextEcoreWl::GetInputPanelData(std::string& data)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetInputPanelData\n");

  if(mIMFContext)
  {
    int                length = 4096; // The max length is 4096 bytes
    Dali::Vector<char> buffer;
    buffer.Resize(length);
    ecore_imf_context_input_panel_imdata_get(mIMFContext, &buffer[0], &length);
    data = std::string(buffer.Begin(), buffer.End());
  }
}

Dali::InputMethodContext::State InputMethodContextEcoreWl::GetInputPanelState()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetInputPanelState\n");

  if(mIMFContext)
  {
    int value;
    value = ecore_imf_context_input_panel_state_get(mIMFContext);

    switch(value)
    {
      case ECORE_IMF_INPUT_PANEL_STATE_SHOW:
      {
        return Dali::InputMethodContext::SHOW;
        break;
      }

      case ECORE_IMF_INPUT_PANEL_STATE_HIDE:
      {
        return Dali::InputMethodContext::HIDE;
        break;
      }

      case ECORE_IMF_INPUT_PANEL_STATE_WILL_SHOW:
      {
        return Dali::InputMethodContext::WILL_SHOW;
        break;
      }

      default:
      {
        return Dali::InputMethodContext::DEFAULT;
      }
    }
  }
  return Dali::InputMethodContext::DEFAULT;
}

void InputMethodContextEcoreWl::SetReturnKeyState(bool visible)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::SetReturnKeyState\n");

  if(mIMFContext)
  {
    ecore_imf_context_input_panel_return_key_disabled_set(mIMFContext, !visible);
  }

  mBackupOperations[Operation::SET_RETURN_KEY_STATE] = std::bind(&InputMethodContextEcoreWl::SetReturnKeyState, this, visible);
}

void InputMethodContextEcoreWl::AutoEnableInputPanel(bool enabled)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::AutoEnableInputPanel\n");

  if(mIMFContext)
  {
    ecore_imf_context_input_panel_enabled_set(mIMFContext, enabled);
  }

  mBackupOperations[Operation::AUTO_ENABLE_INPUT_PANEL] = std::bind(&InputMethodContextEcoreWl::AutoEnableInputPanel, this, enabled);
}

void InputMethodContextEcoreWl::ShowInputPanel()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::ShowInputPanel\n");

  if(mIMFContext)
  {
    ecore_imf_context_input_panel_show(mIMFContext);
  }
}

void InputMethodContextEcoreWl::HideInputPanel()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::HideInputPanel\n");

  if(mIMFContext)
  {
    ecore_imf_context_input_panel_hide(mIMFContext);
  }
}

Dali::InputMethodContext::KeyboardType InputMethodContextEcoreWl::GetKeyboardType()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetKeyboardType\n");

  if(mIMFContext)
  {
    int value;
    value = ecore_imf_context_keyboard_mode_get(mIMFContext);

    switch(value)
    {
      case ECORE_IMF_INPUT_PANEL_SW_KEYBOARD_MODE:
      {
        return Dali::InputMethodContext::SOFTWARE_KEYBOARD;
        break;
      }
      case ECORE_IMF_INPUT_PANEL_HW_KEYBOARD_MODE:
      {
        return Dali::InputMethodContext::HARDWARE_KEYBOARD;
        break;
      }
    }
  }

  return Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD;
}

std::string InputMethodContextEcoreWl::GetInputPanelLocale()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetInputPanelLocale\n");

  std::string locale = "";

  if(mIMFContext)
  {
    char* value = NULL;
    ecore_imf_context_input_panel_language_locale_get(mIMFContext, &value);

    if(value)
    {
      std::string valueCopy(value);
      locale = valueCopy;

      // The locale string retrieved must be freed with free().
      free(value);
    }
  }
  return locale;
}

void InputMethodContextEcoreWl::SetContentMIMETypes(const std::string& mimeTypes)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::SetContentMIMETypes\n");

  if(mIMFContext)
  {
    ecore_imf_context_mime_type_accept_set(mIMFContext, mimeTypes.c_str());
  }

  mBackupOperations[Operation::SET_CONTENT_MIME_TYPES] = std::bind(&InputMethodContextEcoreWl::SetContentMIMETypes, this, mimeTypes);
}

bool InputMethodContextEcoreWl::FilterEventKey(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);

  // If a device key then skip ecore_imf_context_filter_event.
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

void InputMethodContextEcoreWl::AllowTextPrediction(bool prediction)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::AllowTextPrediction\n");

  if(mIMFContext)
  {
    ecore_imf_context_prediction_allow_set(mIMFContext, prediction);
  }

  mBackupOperations[Operation::ALLOW_TEXT_PREDICTION] = std::bind(&InputMethodContextEcoreWl::AllowTextPrediction, this, prediction);
}

bool InputMethodContextEcoreWl::IsTextPredictionAllowed() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::IsTextPredictionAllowed\n");
  bool prediction = false;
  if(mIMFContext)
  {
    prediction = ecore_imf_context_prediction_allow_get(mIMFContext);
  }
  return prediction;
}

void InputMethodContextEcoreWl::SetFullScreenMode(bool fullScreen)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::SetFullScreenMode\n");

#ifdef OVER_TIZEN_VERSION_10
  if(mIMFContext)
  {
    Ecore_IMF_Input_Hints currentHint = ecore_imf_context_input_hint_get(mIMFContext);
    ecore_imf_context_input_hint_set(mIMFContext,
                                     static_cast<Ecore_IMF_Input_Hints>(fullScreen ? (currentHint | ECORE_IMF_INPUT_HINT_FULLSCREEN_MODE) : (currentHint & ~ECORE_IMF_INPUT_HINT_FULLSCREEN_MODE)));
  }

  mBackupOperations[Operation::FULLSCREEN_MODE] = std::bind(&InputMethodContextEcoreWl::SetFullScreenMode, this, fullScreen);
#else
  DALI_LOG_ERROR("SetFullScreenMode NOT SUPPORT THIS TIZEN VERSION!\n");
#endif
}

bool InputMethodContextEcoreWl::IsFullScreenMode() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::IsFullScreenMode\n");
  bool fullScreen = false;

#ifdef OVER_TIZEN_VERSION_10
  if(mIMFContext)
  {
    fullScreen = ecore_imf_context_input_hint_get(mIMFContext) & ECORE_IMF_INPUT_HINT_FULLSCREEN_MODE;
  }
#else
  DALI_LOG_ERROR("IsFullScreenMode NOT SUPPORT THIS TIZEN VERSION!\n");
#endif
  return fullScreen;
}

void InputMethodContextEcoreWl::SetInputPanelLanguage(Dali::InputMethodContext::InputPanelLanguage language)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::SetInputPanelLanguage\n");
  if(mIMFContext)
  {
    switch(language)
    {
      case Dali::InputMethodContext::InputPanelLanguage::AUTOMATIC:
      {
        ecore_imf_context_input_panel_language_set(mIMFContext, ECORE_IMF_INPUT_PANEL_LANG_AUTOMATIC);
        break;
      }
      case Dali::InputMethodContext::InputPanelLanguage::ALPHABET:
      {
        ecore_imf_context_input_panel_language_set(mIMFContext, ECORE_IMF_INPUT_PANEL_LANG_ALPHABET);
        break;
      }
    }
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_LANGUAGE] = std::bind(&InputMethodContextEcoreWl::SetInputPanelLanguage, this, language);
}

Dali::InputMethodContext::InputPanelLanguage InputMethodContextEcoreWl::GetInputPanelLanguage() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetInputPanelLanguage\n");
  if(mIMFContext)
  {
    int value;
    value = ecore_imf_context_input_panel_language_get(mIMFContext);

    switch(value)
    {
      case ECORE_IMF_INPUT_PANEL_LANG_AUTOMATIC:
      {
        return Dali::InputMethodContext::InputPanelLanguage::AUTOMATIC;
        break;
      }
      case ECORE_IMF_INPUT_PANEL_LANG_ALPHABET:
      {
        return Dali::InputMethodContext::InputPanelLanguage::ALPHABET;
        break;
      }
    }
  }
  return Dali::InputMethodContext::InputPanelLanguage::AUTOMATIC;
}

void InputMethodContextEcoreWl::SetInputPanelPosition(unsigned int x, unsigned int y)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::SetInputPanelPosition\n");

  if(mIMFContext)
  {
    ecore_imf_context_input_panel_position_set(mIMFContext, x, y);
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_POSITION] = std::bind(&InputMethodContextEcoreWl::SetInputPanelPosition, this, x, y);
}

bool InputMethodContextEcoreWl::SetInputPanelPositionAlign(int x, int y, Dali::InputMethodContext::InputPanelAlign align)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::SetInputPanelPositionAlign\n");
  bool result = false;

  if(mIMFContext)
  {
    Ecore_IMF_Input_Panel_Align inputPanelAlign = ECORE_IMF_INPUT_PANEL_ALIGN_TOP_LEFT;
    switch(align)
    {
      case Dali::InputMethodContext::InputPanelAlign::TOP_LEFT:
      {
        inputPanelAlign = ECORE_IMF_INPUT_PANEL_ALIGN_TOP_LEFT;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::TOP_CENTER:
      {
        inputPanelAlign = ECORE_IMF_INPUT_PANEL_ALIGN_TOP_CENTER;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::TOP_RIGHT:
      {
        inputPanelAlign = ECORE_IMF_INPUT_PANEL_ALIGN_TOP_RIGHT;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::MIDDLE_LEFT:
      {
        inputPanelAlign = ECORE_IMF_INPUT_PANEL_ALIGN_MIDDLE_LEFT;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::MIDDLE_CENTER:
      {
        inputPanelAlign = ECORE_IMF_INPUT_PANEL_ALIGN_MIDDLE_CENTER;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::MIDDLE_RIGHT:
      {
        inputPanelAlign = ECORE_IMF_INPUT_PANEL_ALIGN_MIDDLE_RIGHT;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::BOTTOM_LEFT:
      {
        inputPanelAlign = ECORE_IMF_INPUT_PANEL_ALIGN_BOTTOM_LEFT;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::BOTTOM_CENTER:
      {
        inputPanelAlign = ECORE_IMF_INPUT_PANEL_ALIGN_BOTTOM_CENTER;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::BOTTOM_RIGHT:
      {
        inputPanelAlign = ECORE_IMF_INPUT_PANEL_ALIGN_BOTTOM_RIGHT;
        break;
      }
    }

    result = ecore_imf_context_input_panel_position_align_set(mIMFContext, x, y, inputPanelAlign);
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_POSITION_ALIGN] = std::bind(&InputMethodContextEcoreWl::SetInputPanelPositionAlign, this, x, y, align);

  return result;
}

void InputMethodContextEcoreWl::GetPreeditStyle(Dali::InputMethodContext::PreEditAttributeDataContainer& attrs) const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextEcoreWl::GetPreeditStyle\n");
  attrs = mPreeditAttrs;
}

bool InputMethodContextEcoreWl::ProcessEventKeyDown(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);
  if(mIMFContext)
  {
    Integration::KeyEvent integKeyEvent(keyEvent.GetKeyName(), keyEvent.GetLogicalKey(), keyEvent.GetKeyString(), keyEvent.GetKeyCode(), keyEvent.GetKeyModifier(), keyEvent.GetTime(), static_cast<Integration::KeyEvent::State>(keyEvent.GetState()), keyEvent.GetCompose(), keyEvent.GetDeviceName(), keyEvent.GetDeviceClass(), keyEvent.GetDeviceSubclass());
    std::string           key = integKeyEvent.logicalKey;

    std::string compose    = integKeyEvent.compose;
    std::string deviceName = integKeyEvent.deviceName;

    // We're consuming key down event so we have to pass to InputMethodContext so that it can parse it as well.
    Ecore_IMF_Event_Key_Down ecoreKeyDownEvent;
    ecoreKeyDownEvent.keyname      = integKeyEvent.keyName.c_str();
    ecoreKeyDownEvent.key          = key.c_str();
    ecoreKeyDownEvent.string       = integKeyEvent.keyString.c_str();
    ecoreKeyDownEvent.compose      = compose.c_str();
    ecoreKeyDownEvent.timestamp    = static_cast<uint32_t>(integKeyEvent.time);
    ecoreKeyDownEvent.modifiers    = EcoreInputModifierToEcoreIMFModifier(integKeyEvent.keyModifier);
    ecoreKeyDownEvent.locks        = EcoreInputModifierToEcoreIMFLock(integKeyEvent.keyModifier);
    ecoreKeyDownEvent.dev_name     = deviceName.c_str();
    ecoreKeyDownEvent.dev_class    = static_cast<Ecore_IMF_Device_Class>(integKeyEvent.deviceClass);       //ECORE_IMF_DEVICE_CLASS_KEYBOARD;
    ecoreKeyDownEvent.dev_subclass = static_cast<Ecore_IMF_Device_Subclass>(integKeyEvent.deviceSubclass); //ECORE_IMF_DEVICE_SUBCLASS_NONE;
#if defined(ECORE_VERSION_MAJOR) && (ECORE_VERSION_MAJOR >= 1) && defined(ECORE_VERSION_MINOR) && (ECORE_VERSION_MINOR >= 22)
    ecoreKeyDownEvent.keycode = integKeyEvent.keyCode; // Ecore_IMF_Event structure has added 'keycode' variable since ecore_imf 1.22 version.
#endif                                                 // Since ecore_imf 1.22 version

    // If the device is IME and the focused key is the direction keys, then we should send a key event to move a key cursor.
    if((integKeyEvent.deviceName == "ime") && ((!strncmp(integKeyEvent.keyName.c_str(), "Left", 4)) ||
                                               (!strncmp(integKeyEvent.keyName.c_str(), "Right", 5)) ||
                                               (!strncmp(integKeyEvent.keyName.c_str(), "Up", 2)) ||
                                               (!strncmp(integKeyEvent.keyName.c_str(), "Down", 4))))
    {
      eventHandled = 0;
    }
    else
    {
      eventHandled = ecore_imf_context_filter_event(mIMFContext,
                                                    ECORE_IMF_EVENT_KEY_DOWN,
                                                    reinterpret_cast<Ecore_IMF_Event*>(&ecoreKeyDownEvent));
    }

    // If the event has not been handled by InputMethodContext then check if we should reset our input method context
    if(!eventHandled)
    {
      if(!strcmp(integKeyEvent.keyName.c_str(), "Escape") ||
         !strcmp(integKeyEvent.keyName.c_str(), "Return") ||
         !strcmp(integKeyEvent.keyName.c_str(), "KP_Enter"))
      {
        ecore_imf_context_reset(mIMFContext);
      }
    }
  }
  return eventHandled;
}

bool InputMethodContextEcoreWl::ProcessEventKeyUp(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);
  if(mIMFContext)
  {
    Integration::KeyEvent integKeyEvent(keyEvent.GetKeyName(), keyEvent.GetLogicalKey(), keyEvent.GetKeyString(), keyEvent.GetKeyCode(), keyEvent.GetKeyModifier(), keyEvent.GetTime(), static_cast<Integration::KeyEvent::State>(keyEvent.GetState()), keyEvent.GetCompose(), keyEvent.GetDeviceName(), keyEvent.GetDeviceClass(), keyEvent.GetDeviceSubclass());
    std::string           key = integKeyEvent.logicalKey;

    std::string compose    = integKeyEvent.compose;
    std::string deviceName = integKeyEvent.deviceName;

    // We're consuming key up event so we have to pass to InputMethodContext so that it can parse it as well.
    Ecore_IMF_Event_Key_Up ecoreKeyUpEvent;
    ecoreKeyUpEvent.keyname      = integKeyEvent.keyName.c_str();
    ecoreKeyUpEvent.key          = key.c_str();
    ecoreKeyUpEvent.string       = integKeyEvent.keyString.c_str();
    ecoreKeyUpEvent.compose      = compose.c_str();
    ecoreKeyUpEvent.timestamp    = static_cast<uint32_t>(integKeyEvent.time);
    ecoreKeyUpEvent.modifiers    = EcoreInputModifierToEcoreIMFModifier(integKeyEvent.keyModifier);
    ecoreKeyUpEvent.locks        = EcoreInputModifierToEcoreIMFLock(integKeyEvent.keyModifier);
    ecoreKeyUpEvent.dev_name     = deviceName.c_str();
    ecoreKeyUpEvent.dev_class    = static_cast<Ecore_IMF_Device_Class>(integKeyEvent.deviceClass);       //ECORE_IMF_DEVICE_CLASS_KEYBOARD;
    ecoreKeyUpEvent.dev_subclass = static_cast<Ecore_IMF_Device_Subclass>(integKeyEvent.deviceSubclass); //ECORE_IMF_DEVICE_SUBCLASS_NONE;
#if defined(ECORE_VERSION_MAJOR) && (ECORE_VERSION_MAJOR >= 1) && defined(ECORE_VERSION_MINOR) && (ECORE_VERSION_MINOR >= 22)
    ecoreKeyUpEvent.keycode = integKeyEvent.keyCode; // Ecore_IMF_Event structure has added 'keycode' variable since ecore_imf 1.22 version.
#endif                                               // Since ecore_imf 1.22 version

    eventHandled = ecore_imf_context_filter_event(mIMFContext,
                                                  ECORE_IMF_EVENT_KEY_UP,
                                                  reinterpret_cast<Ecore_IMF_Event*>(&ecoreKeyUpEvent));
  }
  return eventHandled;
}

Ecore_IMF_Keyboard_Modifiers InputMethodContextEcoreWl::EcoreInputModifierToEcoreIMFModifier(unsigned int ecoreModifier)
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

Ecore_IMF_Keyboard_Locks InputMethodContextEcoreWl::EcoreInputModifierToEcoreIMFLock(unsigned int modifier)
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

void InputMethodContextEcoreWl::OnStaged(Dali::Actor actor)
{
  int windowId = GetWindowIdFromActor(actor);

  if(mWindowId != windowId)
  {
    mWindowId = windowId;

    // Reset
    Finalize();
    Initialize();
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
