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

#include <dali/internal/input/tizen-wayland/tcore/input-method-context-impl-tcore-wl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/string-utils.h>
#include <dali/public-api/adaptor-framework/key.h>
#include <dali/public-api/adaptor-framework/input-method.h>
#include <dali/public-api/events/key-event.h>
#include <tizen_core_imf.h>
#include <tizen_core_wl.h>

// INTERNAL INCLUDES
#include <dali/internal/input/common/key-impl.h>
#include <dali/internal/system/common/locale-utils.h>
#include <dali/internal/window-system/common/window-render-surface.h>

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

bool IsTcoreSuccess(tizen_core_imf_error_e error)
{
  return error == TIZEN_CORE_IMF_ERROR_NONE;
}

struct ScopedTcoreEventKey
{
  ScopedTcoreEventKey() = default;

  ~ScopedTcoreEventKey()
  {
    if(handle)
    {
      tizen_core_imf_event_key_destroy(handle);
    }
  }

  ScopedTcoreEventKey(const ScopedTcoreEventKey&)            = delete;
  ScopedTcoreEventKey& operator=(const ScopedTcoreEventKey&) = delete;

  tizen_core_imf_event_key_h handle{nullptr};
};

// Currently this code is internal to dali/dali/internal/event/text/utf8.h but should be made Public and used from there instead.
size_t Utf8SequenceLength(const unsigned char leadByte)
{
  size_t length = 0;

  if((leadByte & 0x80) == 0) // ASCII character (lead bit zero)
  {
    length = 1;
  }
  else if((leadByte & 0xe0) == 0xc0) // 110x xxxx
  {
    length = 2;
  }
  else if((leadByte & 0xf0) == 0xe0) // 1110 xxxx
  {
    length = 3;
  }
  else if((leadByte & 0xf8) == 0xf0) // 1111 0xxx
  {
    length = 4;
  }
  else if((leadByte & 0xfc) == 0xf8) // 1111 10xx
  {
    length = 5;
  }
  else if((leadByte & 0xfe) == 0xfc) // 1111 110x
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

// Static function calls used by Tizen Core IMF 'C' style callback registration
void Commit(tizen_core_imf_context_h ctx, void* eventInfo, void* userData)
{
  if(userData)
  {
    InputMethodContextTcoreWl* inputMethodContext = static_cast<InputMethodContextTcoreWl*>(userData);
    inputMethodContext->CommitReceived(userData, reinterpret_cast<ImfContext*>(ctx), eventInfo);
  }
}

void PreEdit(tizen_core_imf_context_h ctx, void* eventInfo, void* userData)
{
  if(userData)
  {
    InputMethodContextTcoreWl* inputMethodContext = static_cast<InputMethodContextTcoreWl*>(userData);
    inputMethodContext->PreEditChanged(userData, reinterpret_cast<ImfContext*>(ctx), eventInfo);
  }
}

void ImfRetrieveSurrounding(tizen_core_imf_context_h ctx, char** text, int* cursorPosition, void* userData)
{
  if(userData)
  {
    InputMethodContextTcoreWl* inputMethodContext = static_cast<InputMethodContextTcoreWl*>(userData);
    inputMethodContext->RetrieveSurrounding(userData, reinterpret_cast<ImfContext*>(ctx), text, cursorPosition);
  }
}

void InputPanelStateChangeCallback(tizen_core_imf_context_h ctx, int value, void* userData)
{
  if(!userData)
  {
    return;
  }
  InputMethodContextTcoreWl* inputMethodContext = static_cast<InputMethodContextTcoreWl*>(userData);
  switch(value)
  {
    case TIZEN_CORE_IMF_INPUT_PANEL_STATE_SHOW:
    {
      inputMethodContext->EmitStatusChangedSignal(Dali::InputMethodContext::State::SHOW);
      break;
    }
    case TIZEN_CORE_IMF_INPUT_PANEL_STATE_HIDE:
    {
      inputMethodContext->EmitStatusChangedSignal(Dali::InputMethodContext::State::HIDE);
      break;
    }
    case TIZEN_CORE_IMF_INPUT_PANEL_STATE_WILL_SHOW:
    {
      inputMethodContext->EmitStatusChangedSignal(Dali::InputMethodContext::State::WILL_SHOW);
      break;
    }
    default:
    {
      // Do nothing
      break;
    }
  }
}

void InputPanelLanguageChangeCallback(tizen_core_imf_context_h ctx, int value, void* userData)
{
  if(!userData)
  {
    return;
  }
  InputMethodContextTcoreWl* inputMethodContext = static_cast<InputMethodContextTcoreWl*>(userData);
  // Emit the signal that the language has changed
  inputMethodContext->EmitLanguageChangedSignal(value);
}

void InputPanelGeometryChangedCallback(tizen_core_imf_context_h ctx, int value, void* userData)
{
  if(!userData)
  {
    return;
  }
  InputMethodContextTcoreWl* inputMethodContext = static_cast<InputMethodContextTcoreWl*>(userData);
  // Emit signal that the keyboard is resized
  inputMethodContext->EmitKeyboardResizedSignal(value);
}

void InputPanelKeyboardTypeChangedCallback(tizen_core_imf_context_h ctx, int value, void* userData)
{
  if(!userData)
  {
    return;
  }

  InputMethodContextTcoreWl* inputMethodContext = static_cast<InputMethodContextTcoreWl*>(userData);
  switch(value)
  {
    case TIZEN_CORE_IMF_INPUT_PANEL_KEYBOARD_MODE_SW:
    {
      // Emit Signal that the keyboard type is changed to Software Keyboard
      inputMethodContext->EmitKeyboardTypeChangedSignal(Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD);
      break;
    }
    case TIZEN_CORE_IMF_INPUT_PANEL_KEYBOARD_MODE_HW:
    {
      // Emit Signal that the keyboard type is changed to Hardware Keyboard
      inputMethodContext->EmitKeyboardTypeChangedSignal(Dali::InputMethodContext::KeyboardType::HARDWARE_KEYBOARD);
      break;
    }
  }
}

/**
 * Called when an IMF delete surrounding event is received.
 * Here we tell the application that it should delete a certain range.
 */
void ImfDeleteSurrounding(tizen_core_imf_context_h ctx, void* eventInfo, void* userData)
{
  if(userData)
  {
    InputMethodContextTcoreWl* inputMethodContext = static_cast<InputMethodContextTcoreWl*>(userData);
    inputMethodContext->DeleteSurrounding(userData, reinterpret_cast<ImfContext*>(ctx), eventInfo);
  }
}

/**
 * Called when the input method sends a private command.
 */
void PrivateCommand(tizen_core_imf_context_h ctx, void* eventInfo, void* userData)
{
  if(userData)
  {
    InputMethodContextTcoreWl* inputMethodContext = static_cast<InputMethodContextTcoreWl*>(userData);
    inputMethodContext->SendPrivateCommand(userData, reinterpret_cast<ImfContext*>(ctx), eventInfo);
  }
}

/**
 * Called when the input method commits content, such as an image.
 */
void CommitContent(tizen_core_imf_context_h ctx, void* eventInfo, void* userData)
{
  if(userData)
  {
    InputMethodContextTcoreWl* inputMethodContext = static_cast<InputMethodContextTcoreWl*>(userData);
    inputMethodContext->SendCommitContent(userData, reinterpret_cast<ImfContext*>(ctx), eventInfo);
  }
}

/**
 * Called when the input method sends a selection set.
 */
void SelectionSet(tizen_core_imf_context_h ctx, void* eventInfo, void* userData)
{
  if(userData)
  {
    InputMethodContextTcoreWl* inputMethodContext = static_cast<InputMethodContextTcoreWl*>(userData);
    inputMethodContext->SendSelectionSet(userData, reinterpret_cast<ImfContext*>(ctx), eventInfo);
  }
}

/**
 * Called the input method start a composition transaction.
 */
void TransactionStart(tizen_core_imf_context_h ctx, void* eventInfo, void* userData)
{
  if(userData)
  {
    InputMethodContextTcoreWl* inputMethodContext = static_cast<InputMethodContextTcoreWl*>(userData);
    inputMethodContext->TransactionStartReceived(userData, reinterpret_cast<ImfContext*>(ctx), eventInfo);
  }
}

/**
 * Called when the input method ends a composition transaction.
 */
void TransactionEnd(tizen_core_imf_context_h ctx, void* eventInfo, void* userData)
{
  if(userData)
  {
    InputMethodContextTcoreWl* inputMethodContext = static_cast<InputMethodContextTcoreWl*>(userData);
    inputMethodContext->TransactionEndReceived(userData, reinterpret_cast<ImfContext*>(ctx), eventInfo);
  }
}

tizen_core_wl_window_h GetWindowFromActor(Dali::Actor actor)
{
  tizen_core_wl_window_h window = nullptr;

  if(actor.GetProperty<bool>(Dali::Actor::Property::CONNECTED_TO_SCENE))
  {
    auto sceneHolder = Dali::Integration::SceneHolder::Get(actor);
    if(DALI_LIKELY(sceneHolder))
    {
      Any nativeWindowHandle = sceneHolder.GetNativeHandle();
      // Under Tcore_Wl backend, native handle is a tizen_core_wl_window_h.
      window = AnyCast<tizen_core_wl_window_h>(nativeWindowHandle);
    }
  }

  return window;
}

tizen_core_imf_input_panel_layout_e ToTcorePanelLayout(Dali::InputMethod::PanelLayout layout)
{
  switch(layout)
  {
    case Dali::InputMethod::PanelLayout::NORMAL:
      return TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_NORMAL;
    case Dali::InputMethod::PanelLayout::NUMBER:
      return TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_NUMBER;
    case Dali::InputMethod::PanelLayout::EMAIL:
      return TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_EMAIL;
    case Dali::InputMethod::PanelLayout::URL:
      return TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_URL;
    case Dali::InputMethod::PanelLayout::PHONENUMBER:
      return TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_PHONENUMBER;
    case Dali::InputMethod::PanelLayout::IP:
      return TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_IP;
    case Dali::InputMethod::PanelLayout::MONTH:
      return TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_MONTH;
    case Dali::InputMethod::PanelLayout::NUMBER_ONLY:
      return TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY;
    case Dali::InputMethod::PanelLayout::PASSWORD:
      return TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD;
    case Dali::InputMethod::PanelLayout::DATE_TIME:
      return TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_DATETIME;
    case Dali::InputMethod::PanelLayout::EMOTICON:
      return TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_EMOTICON;
    case Dali::InputMethod::PanelLayout::VOICE:
      return TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_VOICE;
    default:
      return TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_NORMAL;
  }
}

Dali::InputMethod::PanelLayout ToDaliPanelLayout(tizen_core_imf_input_panel_layout_e tcoreLayout)
{
  switch(tcoreLayout)
  {
    case TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_NUMBER:
      return Dali::InputMethod::PanelLayout::NUMBER;
    case TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_EMAIL:
      return Dali::InputMethod::PanelLayout::EMAIL;
    case TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_URL:
      return Dali::InputMethod::PanelLayout::URL;
    case TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_PHONENUMBER:
      return Dali::InputMethod::PanelLayout::PHONENUMBER;
    case TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_IP:
      return Dali::InputMethod::PanelLayout::IP;
    case TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_MONTH:
      return Dali::InputMethod::PanelLayout::MONTH;
    case TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_NUMBERONLY:
      return Dali::InputMethod::PanelLayout::NUMBER_ONLY;
    case TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD:
      return Dali::InputMethod::PanelLayout::PASSWORD;
    case TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_DATETIME:
      return Dali::InputMethod::PanelLayout::DATE_TIME;
    case TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_EMOTICON:
      return Dali::InputMethod::PanelLayout::EMOTICON;
    case TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_VOICE:
      return Dali::InputMethod::PanelLayout::VOICE;
    default:
      return Dali::InputMethod::PanelLayout::NORMAL;
  }
}

tizen_core_imf_input_panel_return_key_type_e ToTcoreReturnKey(Dali::InputMethod::ReturnKeyType action)
{
  switch(action)
  {
    case Dali::InputMethod::ReturnKeyType::DONE:
      return TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DONE;
    case Dali::InputMethod::ReturnKeyType::GO:
      return TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_GO;
    case Dali::InputMethod::ReturnKeyType::JOIN:
      return TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_JOIN;
    case Dali::InputMethod::ReturnKeyType::LOGIN:
      return TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_LOGIN;
    case Dali::InputMethod::ReturnKeyType::NEXT:
      return TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_NEXT;
    case Dali::InputMethod::ReturnKeyType::SEARCH:
      return TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH;
    case Dali::InputMethod::ReturnKeyType::SEND:
      return TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEND;
    case Dali::InputMethod::ReturnKeyType::SIGNIN:
      return TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SIGNIN;
    default:
      return TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT;
  }
}

Dali::InputMethod::ReturnKeyType ToDaliReturnKey(tizen_core_imf_input_panel_return_key_type_e tcoreKey)
{
  switch(tcoreKey)
  {
    case TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DONE:
      return Dali::InputMethod::ReturnKeyType::DONE;
    case TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_GO:
      return Dali::InputMethod::ReturnKeyType::GO;
    case TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_JOIN:
      return Dali::InputMethod::ReturnKeyType::JOIN;
    case TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_LOGIN:
      return Dali::InputMethod::ReturnKeyType::LOGIN;
    case TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_NEXT:
      return Dali::InputMethod::ReturnKeyType::NEXT;
    case TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH:
      return Dali::InputMethod::ReturnKeyType::SEARCH;
    case TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SEND:
      return Dali::InputMethod::ReturnKeyType::SEND;
    case TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_SIGNIN:
      return Dali::InputMethod::ReturnKeyType::SIGNIN;
    default:
      return Dali::InputMethod::ReturnKeyType::DEFAULT;
  }
}

tizen_core_imf_autocapital_type_e ToTcoreAutoCapital(Dali::InputMethod::AutoCapitalType autoCapital)
{
  switch(autoCapital)
  {
    case Dali::InputMethod::AutoCapitalType::WORD:
      return TIZEN_CORE_IMF_AUTOCAPITAL_TYPE_WORD;
    case Dali::InputMethod::AutoCapitalType::SENTENCE:
      return TIZEN_CORE_IMF_AUTOCAPITAL_TYPE_SENTENCE;
    case Dali::InputMethod::AutoCapitalType::ALL_CHARACTER:
      return TIZEN_CORE_IMF_AUTOCAPITAL_TYPE_ALLCHARACTER;
    default:
      return TIZEN_CORE_IMF_AUTOCAPITAL_TYPE_NONE;
  }
}

Dali::InputMethod::AutoCapitalType ToDaliAutoCapital(tizen_core_imf_autocapital_type_e tcoreAutoCapital)
{
  switch(tcoreAutoCapital)
  {
    case TIZEN_CORE_IMF_AUTOCAPITAL_TYPE_WORD:
      return Dali::InputMethod::AutoCapitalType::WORD;
    case TIZEN_CORE_IMF_AUTOCAPITAL_TYPE_SENTENCE:
      return Dali::InputMethod::AutoCapitalType::SENTENCE;
    case TIZEN_CORE_IMF_AUTOCAPITAL_TYPE_ALLCHARACTER:
      return Dali::InputMethod::AutoCapitalType::ALL_CHARACTER;
    default:
      return Dali::InputMethod::AutoCapitalType::NONE;
  }
}

int GetOptionRawVariation(const Dali::Integration::InputMethodOptions& options)
{
  Property::Map settings;
  const_cast<Dali::Integration::InputMethodOptions&>(options).RetrieveProperty(settings);

  if(Property::Value* value = settings.Find("VARIATION", Property::INTEGER))
  {
    return value->Get<int>();
  }

  return 0;
}

int ToTcoreLayoutVariation(Dali::InputMethod::PanelLayout layout, Dali::InputMethod::PanelLayoutVariation variation)
{
  switch(layout)
  {
    case Dali::InputMethod::PanelLayout::NUMBER_ONLY:
    {
      switch(variation)
      {
        case Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_WITH_SIGNED:
          return TIZEN_CORE_IMF_LAYOUT_NUMBERONLY_VARIATION_SIGNED;
        case Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_WITH_DECIMAL:
          return TIZEN_CORE_IMF_LAYOUT_NUMBERONLY_VARIATION_DECIMAL;
        case Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_WITH_SIGNED_AND_DECIMAL:
          return TIZEN_CORE_IMF_LAYOUT_NUMBERONLY_VARIATION_SIGNED_AND_DECIMAL;
        default:
          return TIZEN_CORE_IMF_LAYOUT_NUMBERONLY_VARIATION_NORMAL;
      }
    }
    case Dali::InputMethod::PanelLayout::PASSWORD:
    {
      switch(variation)
      {
        case Dali::InputMethod::PanelLayoutVariation::PASSWORD_WITH_NUMBER_ONLY:
          return TIZEN_CORE_IMF_LAYOUT_PASSWORD_VARIATION_NUMBERONLY;
        default:
          return TIZEN_CORE_IMF_LAYOUT_PASSWORD_VARIATION_NORMAL;
      }
    }
    case Dali::InputMethod::PanelLayout::NORMAL:
    {
      switch(variation)
      {
        case Dali::InputMethod::PanelLayoutVariation::NORMAL_WITH_FILENAME:
          return TIZEN_CORE_IMF_LAYOUT_NORMAL_VARIATION_FILENAME;
        case Dali::InputMethod::PanelLayoutVariation::NORMAL_WITH_PERSON_NAME:
          return TIZEN_CORE_IMF_LAYOUT_NORMAL_VARIATION_PERSON_NAME;
        default:
          return TIZEN_CORE_IMF_LAYOUT_NORMAL_VARIATION_NORMAL;
      }
    }
    default:
      return TIZEN_CORE_IMF_LAYOUT_NORMAL_VARIATION_NORMAL;
  }
}

bool SetTcoreInputPanelLayoutVariation(tizen_core_imf_context_h context, Dali::InputMethod::PanelLayout layout, Dali::InputMethod::PanelLayoutVariation variation)
{
  return IsTcoreSuccess(tizen_core_imf_context_set_input_panel_layout_variation(context, ToTcoreLayoutVariation(layout, variation)));
}

bool SetTcoreInputHintFlag(tizen_core_imf_context_h context, tizen_core_imf_input_hints_e flag, bool enabled)
{
  tizen_core_imf_input_hints_e hints = TIZEN_CORE_IMF_INPUT_HINTS_NONE;
  if(!IsTcoreSuccess(tizen_core_imf_context_get_input_hint(context, &hints)))
  {
    return false;
  }

  const auto updatedHints = enabled ? static_cast<tizen_core_imf_input_hints_e>(hints | flag)
                                    : static_cast<tizen_core_imf_input_hints_e>(hints & ~flag);
  return IsTcoreSuccess(tizen_core_imf_context_set_input_hint(context, updatedHints));
}

Dali::InputMethod::PanelLayoutVariation ToPanelLayoutVariation(Dali::InputMethod::PanelLayout layout, int rawVariation)
{
  switch(layout)
  {
    case Dali::InputMethod::PanelLayout::NUMBER_ONLY:
    {
      switch(rawVariation)
      {
        case TIZEN_CORE_IMF_LAYOUT_NUMBERONLY_VARIATION_SIGNED:
          return Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_WITH_SIGNED;
        case TIZEN_CORE_IMF_LAYOUT_NUMBERONLY_VARIATION_DECIMAL:
          return Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_WITH_DECIMAL;
        case TIZEN_CORE_IMF_LAYOUT_NUMBERONLY_VARIATION_SIGNED_AND_DECIMAL:
          return Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_WITH_SIGNED_AND_DECIMAL;
        default:
          return Dali::InputMethod::PanelLayoutVariation::NUMBER_ONLY_NORMAL;
      }
    }
    case Dali::InputMethod::PanelLayout::PASSWORD:
    {
      switch(rawVariation)
      {
        case TIZEN_CORE_IMF_LAYOUT_PASSWORD_VARIATION_NUMBERONLY:
          return Dali::InputMethod::PanelLayoutVariation::PASSWORD_WITH_NUMBER_ONLY;
        default:
          return Dali::InputMethod::PanelLayoutVariation::PASSWORD_NORMAL;
      }
    }
    case Dali::InputMethod::PanelLayout::NORMAL:
    {
      switch(rawVariation)
      {
        case TIZEN_CORE_IMF_LAYOUT_NORMAL_VARIATION_FILENAME:
          return Dali::InputMethod::PanelLayoutVariation::NORMAL_WITH_FILENAME;
        case TIZEN_CORE_IMF_LAYOUT_NORMAL_VARIATION_PERSON_NAME:
          return Dali::InputMethod::PanelLayoutVariation::NORMAL_WITH_PERSON_NAME;
        default:
          return Dali::InputMethod::PanelLayoutVariation::NORMAL_NORMAL;
      }
    }
    default:
    {
      return Dali::InputMethod::PanelLayoutVariation::NORMAL_NORMAL;
    }
  }
}

} // unnamed namespace

InputMethodContextPtr InputMethodContextTcoreWl::New(Dali::Actor actor)
{
  InputMethodContextPtr inputMethodContext;

  // Create instance only if the adaptor is available and the valid actor exists
  if(actor && Dali::Adaptor::IsAvailable())
  {
    inputMethodContext = new InputMethodContextTcoreWl(actor);
  }
  return inputMethodContext;
}

void InputMethodContextTcoreWl::Finalize()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContext::Finalize\n");

  DisconnectCallbacks();
  DeleteContext();
}

InputMethodContextTcoreWl::InputMethodContextTcoreWl(Dali::Actor actor)
: mSurroundingText(),
  mIMFContext(),
  mWindow(GetWindowFromActor(actor)),
  mIMFCursorPosition(0),
  mRestoreAfterFocusLost(false),
  mIdleCallbackConnected(false),
  mTxCapturing(false)
{
  // Initialize Tizen Core IMF (plugin loader etc.)
  tizen_core_imf_init();

  actor.OnSceneSignal().Connect(this, &InputMethodContextTcoreWl::OnStaged);
}

InputMethodContextTcoreWl::~InputMethodContextTcoreWl()
{
  Finalize();
  tizen_core_imf_shutdown();
}

void InputMethodContextTcoreWl::Initialize()
{
  CreateContext();
  ConnectCallbacks();
  ApplyBackupOperations();
}

void InputMethodContextTcoreWl::CreateContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContext::CreateContext\n");

  if(mWindow == nullptr)
  {
    return;
  }

  if(mIMFContext)
  {
    tizen_core_imf_context_destroy(mIMFContext);
    mIMFContext = nullptr;
  }

  if(!IsTcoreSuccess(tizen_core_imf_context_create(&mIMFContext)) || !mIMFContext)
  {
    DALI_LOG_ERROR("InputMethodContext Unable to create Tizen Core IMF context\n");
    return;
  }

  // Associate native window handle with IMF context
  if(!IsTcoreSuccess(tizen_core_imf_context_set_client_window(mIMFContext, static_cast<void*>(mWindow))))
  {
    DALI_LOG_ERROR("InputMethodContext Unable to set Tizen Core IMF client window\n");
  }
}

void InputMethodContextTcoreWl::DeleteContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::DeleteContext\n");

  if(mIMFContext)
  {
    tizen_core_imf_context_destroy(mIMFContext);
    mIMFContext = nullptr;
  }
}

// Callbacks for predicitive text support.
void InputMethodContextTcoreWl::ConnectCallbacks()
{
  if(mIMFContext)
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::ConnectCallbacks\n");

    tizen_core_imf_context_add_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_PREEDIT_CHANGED, PreEdit, this);
    tizen_core_imf_context_add_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_COMMIT, Commit, this);
    tizen_core_imf_context_add_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_DELETE_SURROUNDING, ImfDeleteSurrounding, this);
    tizen_core_imf_context_add_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_PRIVATE_COMMAND_SEND, PrivateCommand, this);
    tizen_core_imf_context_add_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_COMMIT_CONTENT, CommitContent, this);
    tizen_core_imf_context_add_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_SELECTION_SET, SelectionSet, this);
    tizen_core_imf_context_add_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_TRANSACTION_START, TransactionStart, this);
    tizen_core_imf_context_add_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_TRANSACTION_END, TransactionEnd, this);

    tizen_core_imf_context_add_input_panel_event_callback(mIMFContext, TIZEN_CORE_IMF_INPUT_PANEL_EVENT_STATE, InputPanelStateChangeCallback, this);
    tizen_core_imf_context_add_input_panel_event_callback(mIMFContext, TIZEN_CORE_IMF_INPUT_PANEL_EVENT_LANGUAGE, InputPanelLanguageChangeCallback, this);
    tizen_core_imf_context_add_input_panel_event_callback(mIMFContext, TIZEN_CORE_IMF_INPUT_PANEL_EVENT_GEOMETRY, InputPanelGeometryChangedCallback, this);
    tizen_core_imf_context_add_input_panel_event_callback(mIMFContext, TIZEN_CORE_IMF_INPUT_PANEL_EVENT_KEYBOARD_MODE, InputPanelKeyboardTypeChangedCallback, this);

    tizen_core_imf_context_set_retrieve_surrounding_callback(mIMFContext, ImfRetrieveSurrounding, this);
  }
}

void InputMethodContextTcoreWl::DisconnectCallbacks()
{
  if(mIMFContext)
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::DisconnectCallbacks\n");

    tizen_core_imf_context_del_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_PREEDIT_CHANGED, PreEdit);
    tizen_core_imf_context_del_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_COMMIT, Commit);
    tizen_core_imf_context_del_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_DELETE_SURROUNDING, ImfDeleteSurrounding);
    tizen_core_imf_context_del_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_PRIVATE_COMMAND_SEND, PrivateCommand);
    tizen_core_imf_context_del_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_COMMIT_CONTENT, CommitContent);
    tizen_core_imf_context_del_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_SELECTION_SET, SelectionSet);
    tizen_core_imf_context_del_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_TRANSACTION_START, TransactionStart);
    tizen_core_imf_context_del_event_callback(mIMFContext, TIZEN_CORE_IMF_CALLBACK_TRANSACTION_END, TransactionEnd);

    tizen_core_imf_context_del_input_panel_event_callback(mIMFContext, TIZEN_CORE_IMF_INPUT_PANEL_EVENT_STATE, InputPanelStateChangeCallback);
    tizen_core_imf_context_del_input_panel_event_callback(mIMFContext, TIZEN_CORE_IMF_INPUT_PANEL_EVENT_LANGUAGE, InputPanelLanguageChangeCallback);
    tizen_core_imf_context_del_input_panel_event_callback(mIMFContext, TIZEN_CORE_IMF_INPUT_PANEL_EVENT_GEOMETRY, InputPanelGeometryChangedCallback);
    tizen_core_imf_context_del_input_panel_event_callback(mIMFContext, TIZEN_CORE_IMF_INPUT_PANEL_EVENT_KEYBOARD_MODE, InputPanelKeyboardTypeChangedCallback);
  }
}

void InputMethodContextTcoreWl::Activate()
{
  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;

  if(mIMFContext)
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::Activate\n");

    if(IsTcoreSuccess(tizen_core_imf_context_focus_in(mIMFContext)))
    {
      // emit keyboard activated signal
      Dali::InputMethodContext handle(this);
      mActivatedSignal.Emit(handle);
    }
  }
}

void InputMethodContextTcoreWl::Deactivate()
{
  if(mIMFContext)
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::Deactivate\n");

    Reset();
    tizen_core_imf_context_focus_out(mIMFContext);
  }

  // Reset mIdleCallbackConnected
  mIdleCallbackConnected = false;
}

void InputMethodContextTcoreWl::Reset()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::Reset\n");

  if(mIMFContext)
  {
    tizen_core_imf_context_reset(mIMFContext);
  }
}

ImfContext* InputMethodContextTcoreWl::GetContext()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::GetContext\n");

  return mIMFContext;
}

bool InputMethodContextTcoreWl::RestoreAfterFocusLost() const
{
  return mRestoreAfterFocusLost;
}

bool InputMethodContextTcoreWl::SetRestoreAfterFocusLost(bool toggle)
{
  mRestoreAfterFocusLost = toggle;
  return true;
}

/**
 * Called when an InputMethodContext Pre-Edit changed event is received.
 * We are still predicting what the user is typing.  The latest string is what the InputMethodContext module thinks
 * the user wants to type.
 */
void InputMethodContextTcoreWl::PreEditChanged(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::PreEditChanged\n");
  auto context = reinterpret_cast<tizen_core_imf_context_h>(imfContext);

  char*                             preEditString = nullptr;
  int                               cursorPosition(0);
  tizen_core_imf_preedit_attr_h*    attrs       = nullptr;
  int                               attrsCount  = 0;

  mPreeditAttrs.Clear();

  // Retrieves attributes as well as the string and cursor position from Tizen Core IMF.
  const bool preeditStringRetrieved = IsTcoreSuccess(tizen_core_imf_context_get_preedit_string(context, &preEditString, &attrs, &attrsCount, &cursorPosition));

  if(preeditStringRetrieved && preEditString && attrs && attrsCount > 0)
  {
    for(int i = 0; i < attrsCount; ++i)
    {
      tizen_core_imf_preedit_attr_h attr = attrs[i];
      tizen_core_imf_preedit_type_e preedit_type = TIZEN_CORE_IMF_PREEDIT_TYPE_NONE;
      unsigned int start_index = 0u;
      unsigned int end_index   = 0u;
      if(!IsTcoreSuccess(tizen_core_imf_preedit_attr_get_preedit_type(attr, &preedit_type)) ||
         !IsTcoreSuccess(tizen_core_imf_preedit_attr_get_start_index(attr, &start_index)) ||
         !IsTcoreSuccess(tizen_core_imf_preedit_attr_get_end_index(attr, &end_index)))
      {
        continue;
      }

      Dali::Integration::InputMethodContext::PreeditAttributeData data;
      data.startIndex = 0;
      data.endIndex   = 0;

      uint32_t visualCharacterIndex = 0;
      size_t   byteIndex            = 0;

      // iterate through null terminated string checking each character's position against the given byte position ( attr.end_index ).
      char leadByte = preEditString[byteIndex];

      while(leadByte != '\0')
      {
        leadByte = preEditString[byteIndex]; // Update the character to get the number of its byte

        // attr.end_index is provided as a byte position not character and we need to know the character position.
        const size_t currentSequenceLength = Utf8SequenceLength(leadByte); // returns number of bytes used to represent character.
        if(byteIndex <= start_index)
        {
          data.startIndex = visualCharacterIndex;
        }
        if(byteIndex >= end_index)
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

      switch(preedit_type)
      {
        case TIZEN_CORE_IMF_PREEDIT_TYPE_NONE:
        {
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::NONE;
          break;
        }
        case TIZEN_CORE_IMF_PREEDIT_TYPE_UNDERLINE:
        {
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::UNDERLINE;
          break;
        }
        case TIZEN_CORE_IMF_PREEDIT_TYPE_REVERSE:
        {
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::REVERSE;
          break;
        }
        case TIZEN_CORE_IMF_PREEDIT_TYPE_HIGHLIGHT:
        {
          data.preeditType = Dali::Integration::InputMethodContext::PreeditStyle::HIGHLIGHT;
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

  if(mTxCapturing)
  {
    mTxQueue.push(TxEvent::PreEdit(data, imfContext, preEditString ? preEditString : std::string(), cursorPosition));
  }
  else
  {
    if(Dali::Adaptor::IsAvailable())
    {
      Dali::InputMethodContext            handle(this);
      Dali::Integration::InputMethodContext::EventData eventData(Dali::Integration::InputMethodContext::PRE_EDIT, Dali::String(preEditString ? preEditString : ""), cursorPosition, 0);
      mEventSignal.Emit(handle, eventData);
      Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, eventData);

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
  }

  if(attrs)
  {
    tizen_core_imf_preedit_attrs_destroy(attrs, attrsCount);
  }
  if(preEditString)
  {
    free(preEditString);
  }
}

void InputMethodContextTcoreWl::CommitReceived(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::CommitReceived\n");

  std::string keyString;

  if(eventInfo)
  {
    keyString = static_cast<const char*>(eventInfo);
  }

  if(mTxCapturing)
  {
    mTxQueue.push(TxEvent::Commit(data, imfContext, keyString));
  }
  else
  {
    if(Dali::Adaptor::IsAvailable())
    {
      Dali::InputMethodContext            handle(this);
      Dali::Integration::InputMethodContext::EventData eventData(Dali::Integration::InputMethodContext::COMMIT, Dali::String(keyString.c_str()), 0, 0);
      mEventSignal.Emit(handle, eventData);
      Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, eventData);

      if(callbackData.update)
      {
        SetCursorPosition(callbackData.cursorPosition);
        SetSurroundingText(callbackData.currentText);

        NotifyCursorPosition();
      }
    }
  }
}

/**
 * Called when an InputMethodContext retrieve surround event is received.
 * Here the InputMethodContext module wishes to know the string we are working with and where within the string the cursor is
 * We need to signal the application to tell us this information.
 */
bool InputMethodContextTcoreWl::RetrieveSurrounding(void* data, ImfContext* imfContext, char** text, int* cursorPosition)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::RetrieveSurrounding\n");

  if(text)
  {
    *text = nullptr;
  }

  if(!mIMFContext || (!text && !cursorPosition))
  {
    return false;
  }

  Dali::Integration::InputMethodContext::EventData imfData(Dali::Integration::InputMethodContext::GET_SURROUNDING, Dali::String(), 0, 0);
  Dali::InputMethodContext                         handle(this);
  mEventSignal.Emit(handle, imfData);
  Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, imfData);

  if(callbackData.update)
  {
    if(cursorPosition)
    {
      mIMFCursorPosition = static_cast<int>(callbackData.cursorPosition);
      *cursorPosition    = mIMFCursorPosition;
    }

    if(text)
    {
      const char* plainText = callbackData.currentText.CStr();

      if(plainText)
      {
        // If the current input panel is password mode, dali should replace the plain text with '*' (Asterisk) character.
        tizen_core_imf_input_hints_e hints = TIZEN_CORE_IMF_INPUT_HINTS_NONE;
        if(!IsTcoreSuccess(tizen_core_imf_context_get_input_hint(mIMFContext, &hints)))
        {
          return false;
        }
        if(hints & TIZEN_CORE_IMF_INPUT_HINTS_SENSITIVE_DATA)
        {
          size_t textLength = callbackData.currentText.Size();
          size_t utf8Length = GetNumberOfUtf8Characters(plainText, textLength);
          if(textLength > 0u && utf8Length == 0u)
          {
            DALI_LOG_ERROR("Invalid utf8 characters, utf8 len:%zu, text len:%zu, text:%s\n", utf8Length, textLength, plainText);
            return false;
          }

          std::string asterisks(utf8Length, '*');
          *text = strdup(asterisks.c_str());
        }
        else
        {
          // The memory allocated by strdup() can be freed by tizen_core_imf_context_get_surrounding() internally.
          *text = strdup(plainText);
        }

        return *text != nullptr;
      }
    }
  }

  return false;
}

/**
 * Called when an InputMethodContext delete surrounding event is received.
 * Here we tell the application that it should delete a certain range.
 */
void InputMethodContextTcoreWl::DeleteSurrounding(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::DeleteSurrounding\n");

  const char* deleteSurroundingEvent = static_cast<const char*>(eventInfo);

  if(deleteSurroundingEvent)
  {
    if(mTxCapturing)
    {
      mTxQueue.push(TxEvent::DeleteSurrounding(data, imfContext, 0, 0));
    }
    else
    {
      if(Dali::Adaptor::IsAvailable())
      {
        Dali::Integration::InputMethodContext::EventData imfData(Dali::Integration::InputMethodContext::DELETE_SURROUNDING, Dali::String(), 0, 0);
        Dali::InputMethodContext                         handle(this);
        mEventSignal.Emit(handle, imfData);
        mKeyboardEventSignal.Emit(handle, imfData);
      }
    }
  }
}

/**
 * Called when the input method sends a private command.
 */
void InputMethodContextTcoreWl::SendPrivateCommand(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SendPrivateCommand\n");

  const char* privateCommandSendEvent = static_cast<const char*>(eventInfo);

  if(privateCommandSendEvent)
  {
    if(mTxCapturing)
    {
      mTxQueue.push(TxEvent::PrivateCommand(data, imfContext, privateCommandSendEvent));
    }
    else
    {
      if(Dali::Adaptor::IsAvailable())
      {
        Dali::Integration::InputMethodContext::EventData imfData(Dali::Integration::InputMethodContext::PRIVATE_COMMAND, Dali::String(privateCommandSendEvent), 0, 0);
        Dali::InputMethodContext                         handle(this);
        mEventSignal.Emit(handle, imfData);
        mPrivateCommandReceivedSignal.Emit(handle, imfData.predictiveString);
        mKeyboardEventSignal.Emit(handle, imfData);
      }
    }
  }
}

/**
 * Called when the input method commits content, such as an image.
 */
void InputMethodContextTcoreWl::SendCommitContent(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SendCommitContent\n");

  const char* commitContent = static_cast<const char*>(eventInfo);

  if(commitContent)
  {
    if(mTxCapturing)
    {
      mTxQueue.push(TxEvent::CommitContent(data, imfContext, commitContent, "", ""));
    }
    else
    {
      if(Dali::Adaptor::IsAvailable())
      {
        DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SendCommitContent commit content : %s\n", commitContent);
        EmitContentReceivedSignal(commitContent, "", "");
      }
    }
  }
}

/**
 * Called when the input method selection set.
 */
void InputMethodContextTcoreWl::SendSelectionSet(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SendCommitContent\n");

  const char* selection = static_cast<const char*>(eventInfo);

  if(selection)
  {
    if(mTxCapturing)
    {
      mTxQueue.push(TxEvent::SelectionSet(data, imfContext, 0, 0));
    }
    else
    {
      if(Dali::Adaptor::IsAvailable())
      {
        DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SendSelectionSet selection : %s\n", selection);
        Dali::Integration::InputMethodContext::EventData imfData(Dali::Integration::InputMethodContext::SELECTION_SET, selection, 0, 0);
        Dali::InputMethodContext                         handle(this);
        mEventSignal.Emit(handle, imfData);
        mKeyboardEventSignal.Emit(handle, imfData);
      }
    }
  }
}

void InputMethodContextTcoreWl::TransactionStartReceived(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::TransactionStartReceived\n");

  mTxCapturing = true;
}

void InputMethodContextTcoreWl::TransactionEndReceived(void* data, ImfContext* imfContext, void* eventInfo)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::TransactionEndReceived\n");

  mTxCapturing = false;

  while(!mTxQueue.empty())
  {
    if(DALI_UNLIKELY(mTxCapturing))
    {
      DALI_LOG_ERROR("Transaction started during end event queueing!\n");
      break;
    }

    TxEvent currentEvent = mTxQueue.front();

    switch(currentEvent.type)
    {
      case TxEventType::COMMIT:
      {
        if(Dali::Adaptor::IsAvailable())
        {
          const String keyString = currentEvent.eventValue.GetElementAt(0).Get<String>();

          Dali::InputMethodContext            handle(this);
          Dali::Integration::InputMethodContext::EventData eventData(Dali::Integration::InputMethodContext::COMMIT, keyString, 0, 0);
          mEventSignal.Emit(handle, eventData);
          Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, eventData);

          if(callbackData.update)
          {
            SetCursorPosition(callbackData.cursorPosition);
            SetSurroundingText(callbackData.currentText);

            NotifyCursorPosition();
          }
        }
        break;
      }
      case TxEventType::PREEDIT:
      {
        if(Dali::Adaptor::IsAvailable())
        {
          Dali::InputMethodContext            handle(this);
          Dali::String                        preEditString  = currentEvent.eventValue.GetElementAt(0).Get<Dali::String>();
          int                                 cursorPosition = currentEvent.eventValue.GetElementAt(1).Get<int>();
          Dali::Integration::InputMethodContext::EventData eventData(Dali::Integration::InputMethodContext::PRE_EDIT, preEditString, cursorPosition, 0);
          mEventSignal.Emit(handle, eventData);
          Dali::Integration::InputMethodContext::CallbackData callbackData = mKeyboardEventSignal.Emit(handle, eventData);

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
        break;
      }
      case TxEventType::DELETE_SURROUNDING:
      {
        if(Dali::Adaptor::IsAvailable())
        {
          int                                 offset  = currentEvent.eventValue.GetElementAt(0).Get<int>();
          int                                 n_chars = currentEvent.eventValue.GetElementAt(1).Get<int>();
          Dali::Integration::InputMethodContext::EventData imfData(Dali::Integration::InputMethodContext::DELETE_SURROUNDING, Dali::String(), offset, n_chars);
          Dali::InputMethodContext            handle(this);
          mEventSignal.Emit(handle, imfData);
          mKeyboardEventSignal.Emit(handle, imfData);
        }
        break;
      }
      case TxEventType::PRIVATE_COMMAND:
      {
        if(Dali::Adaptor::IsAvailable())
        {
          Dali::String privateCommandSendEvent = currentEvent.eventValue.GetElementAt(0).Get<Dali::String>();

          Dali::Integration::InputMethodContext::EventData imfData(Dali::Integration::InputMethodContext::PRIVATE_COMMAND, privateCommandSendEvent, 0, 0);
          Dali::InputMethodContext            handle(this);
          mEventSignal.Emit(handle, imfData);
          mPrivateCommandReceivedSignal.Emit(handle, imfData.predictiveString);
          mKeyboardEventSignal.Emit(handle, imfData);
        }
        break;
      }
      case TxEventType::COMMIT_CONTENT:
      {
        if(Dali::Adaptor::IsAvailable())
        {
          Dali::String contentUri  = currentEvent.eventValue.GetElementAt(0).Get<Dali::String>();
          Dali::String description = currentEvent.eventValue.GetElementAt(1).Get<Dali::String>();
          Dali::String mimeTypes   = currentEvent.eventValue.GetElementAt(2).Get<Dali::String>();
          EmitContentReceivedSignal(contentUri, description, mimeTypes);
        }
        break;
      }
      case TxEventType::SELECTION_SET:
      {
        if(Dali::Adaptor::IsAvailable())
        {
          int                                 start = currentEvent.eventValue.GetElementAt(0).Get<int>();
          int                                 end   = currentEvent.eventValue.GetElementAt(1).Get<int>();
          Dali::Integration::InputMethodContext::EventData imfData(Dali::Integration::InputMethodContext::SELECTION_SET, start, end);
          Dali::InputMethodContext            handle(this);
          mEventSignal.Emit(handle, imfData);
          mKeyboardEventSignal.Emit(handle, imfData);
        }
        break;
      }
      default:
      {
        // Do nothing
        break;
      }
    }

    mTxQueue.pop();
  }
}

void InputMethodContextTcoreWl::NotifyCursorPosition()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::NotifyCursorPosition\n");

  if(mIMFContext)
  {
    tizen_core_imf_context_set_cursor_position(mIMFContext, mIMFCursorPosition);
  }
}

void InputMethodContextTcoreWl::SetCursorPosition(unsigned int cursorPosition)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SetCursorPosition\n");

  mIMFCursorPosition = static_cast<int>(cursorPosition);
}

unsigned int InputMethodContextTcoreWl::GetCursorPosition() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::GetCursorPosition\n");

  return static_cast<unsigned int>(mIMFCursorPosition);
}

void InputMethodContextTcoreWl::SetSurroundingText(const Dali::String& text)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SetSurroundingText\n");

  mSurroundingText = text;
}

Dali::String InputMethodContextTcoreWl::GetSurroundingText() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::GetSurroundingText\n");

  return mSurroundingText;
}

void InputMethodContextTcoreWl::NotifyTextInputMultiLine(bool multiLine)
{
  if(mIMFContext)
  {
    SetTcoreInputHintFlag(mIMFContext, TIZEN_CORE_IMF_INPUT_HINTS_MULTILINE, multiLine);
  }

  mBackupOperations[Operation::NOTIFY_TEXT_INPUT_MULTILINE] = std::bind(&InputMethodContextTcoreWl::NotifyTextInputMultiLine, this, multiLine);
}

Dali::Integration::InputMethodContext::TextDirection InputMethodContextTcoreWl::GetTextDirection()
{
  Dali::Integration::InputMethodContext::TextDirection direction(Dali::Integration::InputMethodContext::LEFT_TO_RIGHT);

  if(mIMFContext)
  {
    char* locale = nullptr;
    tizen_core_imf_context_get_input_panel_language_locale(mIMFContext, &locale);

    if(locale)
    {
      direction = static_cast<Dali::Integration::InputMethodContext::TextDirection>(Locale::GetDirection(std::string(locale)));
      free(locale);
    }
  }

  return direction;
}

BoundsInteger InputMethodContextTcoreWl::GetInputPanelArea()
{
  int xPos, yPos, width, height;

  width = height = xPos = yPos = 0;

  if(mIMFContext)
  {
    tizen_core_imf_context_get_input_panel_geometry(mIMFContext, &xPos, &yPos, &width, &height);
  }
  else
  {
    DALI_LOG_ERROR("VKB Unable to get IMFContext so GetSize unavailable\n");
    // return 0 as real size unknown.
  }

  return BoundsInteger(xPos, yPos, width, height);
}

void InputMethodContextTcoreWl::ApplyOptions(const Dali::Integration::InputMethodOptions& options)
{
  using namespace Dali::Integration::InputMethod::Category;

  int index;
  const auto requestedLayout       = options.GetPanelLayout();
  const int  requestedRawVariation = GetOptionRawVariation(options);
  const auto requestedVariation    = ToPanelLayoutVariation(requestedLayout, requestedRawVariation);

  if(mIMFContext == NULL)
  {
    DALI_LOG_ERROR("VKB Unable to excute ApplyOptions with Null ImfContext\n");
    return;
  }

  if(mOptions.CompareAndSet(PANEL_LAYOUT, options, index))
  {
    const auto layout      = mOptions.GetPanelLayout();
    const auto tcoreLayout = ToTcorePanelLayout(layout);
    if(IsTcoreSuccess(tizen_core_imf_context_set_input_panel_layout(mIMFContext, tcoreLayout)))
    {
      // Sets the input hint which allows input methods to fine-tune their behavior.
      SetTcoreInputHintFlag(mIMFContext, TIZEN_CORE_IMF_INPUT_HINTS_SENSITIVE_DATA, tcoreLayout == TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD);
      SetTcoreInputPanelLayoutVariation(mIMFContext, layout, ToPanelLayoutVariation(layout, requestedRawVariation));
    }
  }
  if(mOptions.CompareAndSet(BUTTON_ACTION, options, index))
  {
    tizen_core_imf_context_set_input_panel_return_key_type(mIMFContext, ToTcoreReturnKey(static_cast<Dali::InputMethod::ReturnKeyType>(index)));
  }
  if(mOptions.CompareAndSet(AUTO_CAPITALIZE, options, index))
  {
    tizen_core_imf_context_set_autocapital_type(mIMFContext, ToTcoreAutoCapital(static_cast<Dali::InputMethod::AutoCapitalType>(index)));
  }
  if(mOptions.CompareAndSet(VARIATION, options, index))
  {
    SetTcoreInputPanelLayoutVariation(mIMFContext, requestedLayout, requestedVariation);
  }
}

bool InputMethodContextTcoreWl::SetInputPanelUserData(const Dali::String& data)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SetInputPanelUserData\n");

  bool result = true;

  if(mIMFContext)
  {
    int length = static_cast<int>(data.Size());
    result     = IsTcoreSuccess(tizen_core_imf_context_set_input_panel_data(mIMFContext, reinterpret_cast<const unsigned char*>(data.CStr()), length));
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_USER_DATA] = std::bind(&InputMethodContextTcoreWl::SetInputPanelUserData, this, data);

  return result;
}

Dali::String InputMethodContextTcoreWl::GetInputPanelUserData() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::GetInputPanelUserData\n");

  if(mIMFContext)
  {
    int            length = 4096; // The max length is 4096 bytes
    unsigned char* raw = nullptr;
    const bool     dataRetrieved = IsTcoreSuccess(tizen_core_imf_context_get_input_panel_data(mIMFContext, &raw, &length));
    if(raw && length > 0)
    {
      Dali::String data = Dali::Integration::ToDaliString(std::string(reinterpret_cast<char*>(raw), length));
      free(raw);
      return data;
    }
    if(raw)
    {
      free(raw);
    }
    if(!dataRetrieved)
    {
      DALI_LOG_ERROR("InputMethodContextTcoreWl::GetInputPanelUserData failed\n");
    }
  }

  return Dali::String();
}

Dali::InputMethodContext::State InputMethodContextTcoreWl::GetInputPanelState()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::GetInputPanelState\n");

  if(mIMFContext)
  {
    tizen_core_imf_input_panel_state_e state = TIZEN_CORE_IMF_INPUT_PANEL_STATE_HIDE;
    tizen_core_imf_context_get_input_panel_state(mIMFContext, &state);

    switch(state)
    {
      case TIZEN_CORE_IMF_INPUT_PANEL_STATE_SHOW:
      {
        return Dali::InputMethodContext::State::SHOW;
        break;
      }
      case TIZEN_CORE_IMF_INPUT_PANEL_STATE_HIDE:
      {
        return Dali::InputMethodContext::State::HIDE;
        break;
      }
      case TIZEN_CORE_IMF_INPUT_PANEL_STATE_WILL_SHOW:
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

bool InputMethodContextTcoreWl::SetReturnKeyState(bool visible)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SetReturnKeyState\n");

  bool result = true;

  if(mIMFContext)
  {
    result = IsTcoreSuccess(tizen_core_imf_context_set_input_panel_return_key_disabled(mIMFContext, !visible));
  }

  mBackupOperations[Operation::SET_RETURN_KEY_STATE] = std::bind(&InputMethodContextTcoreWl::SetReturnKeyState, this, visible);

  return result;
}

bool InputMethodContextTcoreWl::IsReturnKeyEnabled() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::IsReturnKeyEnabled\n");

  if(mIMFContext)
  {
    bool disabled = false;
    if(IsTcoreSuccess(tizen_core_imf_context_get_input_panel_return_key_disabled(mIMFContext, &disabled)))
    {
      return !disabled;
    }
  }

  return true;
}

bool InputMethodContextTcoreWl::AutoEnableInputPanel(bool enabled)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::AutoEnableInputPanel\n");

  bool result = true;

  if(mIMFContext)
  {
    result = IsTcoreSuccess(tizen_core_imf_context_set_input_panel_enabled(mIMFContext, enabled));
  }

  mBackupOperations[Operation::AUTO_ENABLE_INPUT_PANEL] = std::bind(&InputMethodContextTcoreWl::AutoEnableInputPanel, this, enabled);

  return result;
}

bool InputMethodContextTcoreWl::ShowInputPanel()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::ShowInputPanel\n");

  if(mIMFContext)
  {
    return IsTcoreSuccess(tizen_core_imf_context_input_panel_show(mIMFContext));
  }

  return false;
}

bool InputMethodContextTcoreWl::HideInputPanel()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::HideInputPanel\n");

  if(mIMFContext)
  {
    return IsTcoreSuccess(tizen_core_imf_context_input_panel_hide(mIMFContext));
  }

  return false;
}

Dali::InputMethodContext::KeyboardType InputMethodContextTcoreWl::GetKeyboardType()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::GetKeyboardType\n");

  if(mIMFContext)
  {
    tizen_core_imf_input_panel_keyboard_mode_e mode = TIZEN_CORE_IMF_INPUT_PANEL_KEYBOARD_MODE_SW;
    tizen_core_imf_context_get_keyboard_mode(mIMFContext, &mode);

    switch(mode)
    {
      case TIZEN_CORE_IMF_INPUT_PANEL_KEYBOARD_MODE_SW:
      {
        return Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD;
        break;
      }
      case TIZEN_CORE_IMF_INPUT_PANEL_KEYBOARD_MODE_HW:
      {
        return Dali::InputMethodContext::KeyboardType::HARDWARE_KEYBOARD;
        break;
      }
    }
  }

  return Dali::InputMethodContext::KeyboardType::SOFTWARE_KEYBOARD;
}

bool InputMethodContextTcoreWl::SetInputPanelLanguageLocale(const Dali::String& locale)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SetInputPanelLanguageLocale\n");

  bool result = true;

  if(mIMFContext)
  {
    result = IsTcoreSuccess(tizen_core_imf_context_set_input_panel_language_locale(mIMFContext, locale.CStr()));
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_LANGUAGE_LOCALE] = std::bind(&InputMethodContextTcoreWl::SetInputPanelLanguageLocale, this, locale);

  return result;
}

Dali::String InputMethodContextTcoreWl::GetInputPanelLanguageLocale() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::GetInputPanelLanguageLocale\n");

  Dali::String locale;

  if(mIMFContext)
  {
    char* value = nullptr;
    tizen_core_imf_context_get_input_panel_language_locale(mIMFContext, &value);

    if(value)
    {
      locale = Dali::String(value);

      // The locale string retrieved must be freed with free().
      free(value);
    }
  }
  return locale;
}

void InputMethodContextTcoreWl::SetContentMimeTypes(const Dali::String& mimeTypes)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SetContentMimeTypes\n");

  if(mIMFContext)
  {
    tizen_core_imf_context_set_mime_type_accept(mIMFContext, mimeTypes.CStr());
  }

  mBackupOperations[Operation::SET_CONTENT_MIME_TYPES] = std::bind(&InputMethodContextTcoreWl::SetContentMimeTypes, this, mimeTypes);
}

bool InputMethodContextTcoreWl::FilterEventKey(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);

  // If a device key then skip IMF context filter event.
  if(!KeyLookup::IsDeviceButton(keyEvent.GetKeyName().CStr()))
  {
    // check whether it's key down or key up event
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

bool InputMethodContextTcoreWl::AllowTextPrediction(bool prediction)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::AllowTextPrediction\n");

  bool result = true;

  if(mIMFContext)
  {
    result = SetTcoreInputHintFlag(mIMFContext, TIZEN_CORE_IMF_INPUT_HINTS_AUTO_COMPLETE, prediction);
  }

  mBackupOperations[Operation::ALLOW_TEXT_PREDICTION] = std::bind(&InputMethodContextTcoreWl::AllowTextPrediction, this, prediction);

  return result;
}

bool InputMethodContextTcoreWl::IsTextPredictionAllowed() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::IsTextPredictionAllowed\n");
  bool prediction = false;
  if(mIMFContext)
  {
    tizen_core_imf_input_hints_e hints = TIZEN_CORE_IMF_INPUT_HINTS_NONE;
    if(IsTcoreSuccess(tizen_core_imf_context_get_input_hint(mIMFContext, &hints)))
    {
      prediction = (hints & TIZEN_CORE_IMF_INPUT_HINTS_AUTO_COMPLETE);
    }
  }
  return prediction;
}

bool InputMethodContextTcoreWl::SetFullScreenMode(bool fullScreen)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SetFullScreenMode\n");

#ifdef OVER_TIZEN_VERSION_10
  bool result = true;

  if(mIMFContext)
  {
    result = SetTcoreInputHintFlag(mIMFContext, TIZEN_CORE_IMF_INPUT_HINTS_FULLSCREEN_MODE, fullScreen);
  }

  mBackupOperations[Operation::FULLSCREEN_MODE] = std::bind(&InputMethodContextTcoreWl::SetFullScreenMode, this, fullScreen);
  return result;
#else
  DALI_LOG_ERROR("SetFullScreenMode NOT SUPPORT THIS TIZEN VERSION!\n");
  return false;
#endif
}

bool InputMethodContextTcoreWl::IsFullScreenMode() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::IsFullScreenMode\n");
  bool fullScreen = false;

#ifdef OVER_TIZEN_VERSION_10
  if(mIMFContext)
  {
    tizen_core_imf_input_hints_e hints = TIZEN_CORE_IMF_INPUT_HINTS_NONE;
    if(IsTcoreSuccess(tizen_core_imf_context_get_input_hint(mIMFContext, &hints)))
    {
      fullScreen = (hints & TIZEN_CORE_IMF_INPUT_HINTS_FULLSCREEN_MODE);
    }
  }
#else
  DALI_LOG_ERROR("IsFullScreenMode NOT SUPPORT THIS TIZEN VERSION!\n");
#endif
  return fullScreen;
}

bool InputMethodContextTcoreWl::SetInputPanelLanguage(Dali::Integration::InputMethodContext::InputPanelLanguage language)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SetInputPanelLanguage\n");
  // Tizen Core IMF does not expose a simple language enum; keep value only at Dali level.

  mBackupOperations[Operation::SET_INPUT_PANEL_LANGUAGE] = std::bind(&InputMethodContextTcoreWl::SetInputPanelLanguage, this, language);
  return true;
}

Dali::Integration::InputMethodContext::InputPanelLanguage InputMethodContextTcoreWl::GetInputPanelLanguage() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::GetInputPanelLanguage\n");
  return Dali::Integration::InputMethodContext::InputPanelLanguage::AUTOMATIC;
}

bool InputMethodContextTcoreWl::SetInputPanelPosition(unsigned int x, unsigned int y)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SetInputPanelPosition\n");

  bool result = true;

  if(mIMFContext)
  {
    result = IsTcoreSuccess(tizen_core_imf_context_set_input_panel_position(mIMFContext, static_cast<int>(x), static_cast<int>(y)));
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_POSITION] = std::bind(&InputMethodContextTcoreWl::SetInputPanelPosition, this, x, y);

  return result;
}

bool InputMethodContextTcoreWl::SetInputPanelPositionAlign(int x, int y, Dali::InputMethodContext::InputPanelAlign align)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SetInputPanelPositionAlign\n");
  bool result = false;

  if(mIMFContext)
  {
    tizen_core_imf_input_panel_align_e inputPanelAlign = TIZEN_CORE_IMF_INPUT_PANEL_ALIGN_TOP_LEFT;
    switch(align)
    {
      case Dali::InputMethodContext::InputPanelAlign::TOP_LEFT:
      {
        inputPanelAlign = TIZEN_CORE_IMF_INPUT_PANEL_ALIGN_TOP_LEFT;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::TOP_CENTER:
      {
        inputPanelAlign = TIZEN_CORE_IMF_INPUT_PANEL_ALIGN_TOP_CENTER;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::TOP_RIGHT:
      {
        inputPanelAlign = TIZEN_CORE_IMF_INPUT_PANEL_ALIGN_TOP_RIGHT;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::MIDDLE_LEFT:
      {
        inputPanelAlign = TIZEN_CORE_IMF_INPUT_PANEL_ALIGN_MIDDLE_LEFT;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::MIDDLE_CENTER:
      {
        inputPanelAlign = TIZEN_CORE_IMF_INPUT_PANEL_ALIGN_MIDDLE_CENTER;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::MIDDLE_RIGHT:
      {
        inputPanelAlign = TIZEN_CORE_IMF_INPUT_PANEL_ALIGN_MIDDLE_RIGHT;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::BOTTOM_LEFT:
      {
        inputPanelAlign = TIZEN_CORE_IMF_INPUT_PANEL_ALIGN_BOTTOM_LEFT;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::BOTTOM_CENTER:
      {
        inputPanelAlign = TIZEN_CORE_IMF_INPUT_PANEL_ALIGN_BOTTOM_CENTER;
        break;
      }
      case Dali::InputMethodContext::InputPanelAlign::BOTTOM_RIGHT:
      {
        inputPanelAlign = TIZEN_CORE_IMF_INPUT_PANEL_ALIGN_BOTTOM_RIGHT;
        break;
      }
    }

    result = IsTcoreSuccess(tizen_core_imf_context_set_input_panel_position_align(mIMFContext, x, y, inputPanelAlign));
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_POSITION_ALIGN] = std::bind(&InputMethodContextTcoreWl::SetInputPanelPositionAlign, this, x, y, align);

  return result;
}

void InputMethodContextTcoreWl::GetPreeditStyle(Dali::Integration::InputMethodContext::PreEditAttributeDataContainer& attrs) const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::GetPreeditStyle\n");
  attrs = mPreeditAttrs;
}

bool InputMethodContextTcoreWl::SetInputPanelLayout(Dali::InputMethod::PanelLayout layout)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SetInputPanelLayout\n");

  bool result = true;

  if(mIMFContext)
  {
    const auto tcoreLayout = ToTcorePanelLayout(layout);
    if(IsTcoreSuccess(tizen_core_imf_context_set_input_panel_layout(mIMFContext, tcoreLayout)))
    {
      result = SetTcoreInputHintFlag(mIMFContext, TIZEN_CORE_IMF_INPUT_HINTS_SENSITIVE_DATA, tcoreLayout == TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_PASSWORD);
      result = SetTcoreInputPanelLayoutVariation(mIMFContext, layout, ToPanelLayoutVariation(layout, 0)) && result;
    }
    else
    {
      result = false;
    }
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_LAYOUT] = std::bind(&InputMethodContextTcoreWl::SetInputPanelLayout, this, layout);

  return result;
}

Dali::InputMethod::PanelLayout InputMethodContextTcoreWl::GetInputPanelLayout() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::GetInputPanelLayout\n");

  if(mIMFContext)
  {
    tizen_core_imf_input_panel_layout_e layout = TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_NORMAL;
    if(IsTcoreSuccess(tizen_core_imf_context_get_input_panel_layout(mIMFContext, &layout)))
    {
      return ToDaliPanelLayout(layout);
    }
  }

  return Dali::InputMethod::PanelLayout::NORMAL;
}

bool InputMethodContextTcoreWl::SetInputPanelReturnKeyType(Dali::InputMethod::ReturnKeyType action)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SetInputPanelReturnKeyType\n");

  bool result = true;

  if(mIMFContext)
  {
    result = IsTcoreSuccess(tizen_core_imf_context_set_input_panel_return_key_type(mIMFContext, ToTcoreReturnKey(action)));
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_RETURN_KEY] = std::bind(&InputMethodContextTcoreWl::SetInputPanelReturnKeyType, this, action);

  return result;
}

Dali::InputMethod::ReturnKeyType InputMethodContextTcoreWl::GetInputPanelReturnKeyType() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::GetInputPanelReturnKeyType\n");

  if(mIMFContext)
  {
    tizen_core_imf_input_panel_return_key_type_e returnKey = TIZEN_CORE_IMF_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT;
    if(IsTcoreSuccess(tizen_core_imf_context_get_input_panel_return_key_type(mIMFContext, &returnKey)))
    {
      return ToDaliReturnKey(returnKey);
    }
  }

  return Dali::InputMethod::ReturnKeyType::DEFAULT;
}

bool InputMethodContextTcoreWl::SetInputPanelAutoCapitalType(Dali::InputMethod::AutoCapitalType autoCapital)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SetInputPanelAutoCapitalType\n");

  bool result = true;

  if(mIMFContext)
  {
    result = IsTcoreSuccess(tizen_core_imf_context_set_autocapital_type(mIMFContext, ToTcoreAutoCapital(autoCapital)));
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_AUTO_CAPITAL] = std::bind(&InputMethodContextTcoreWl::SetInputPanelAutoCapitalType, this, autoCapital);

  return result;
}

Dali::InputMethod::AutoCapitalType InputMethodContextTcoreWl::GetInputPanelAutoCapitalType() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::GetInputPanelAutoCapitalType\n");

  if(mIMFContext)
  {
    tizen_core_imf_autocapital_type_e autoCapital = TIZEN_CORE_IMF_AUTOCAPITAL_TYPE_SENTENCE;
    if(IsTcoreSuccess(tizen_core_imf_context_get_autocapital_type(mIMFContext, &autoCapital)))
    {
      return ToDaliAutoCapital(autoCapital);
    }
  }

  return Dali::InputMethod::AutoCapitalType::SENTENCE;
}

bool InputMethodContextTcoreWl::SetInputPanelLayoutVariation(Dali::InputMethod::PanelLayoutVariation variation)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::SetInputPanelLayoutVariation\n");

  bool result = true;

  if(mIMFContext)
  {
    tizen_core_imf_input_panel_layout_e tcoreLayout = TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_NORMAL;
    if(IsTcoreSuccess(tizen_core_imf_context_get_input_panel_layout(mIMFContext, &tcoreLayout)))
    {
      result = SetTcoreInputPanelLayoutVariation(mIMFContext, ToDaliPanelLayout(tcoreLayout), variation);
    }
    else
    {
      result = SetTcoreInputPanelLayoutVariation(mIMFContext, Dali::InputMethod::PanelLayout::NORMAL, variation);
    }
  }

  mBackupOperations[Operation::SET_INPUT_PANEL_LAYOUT_VARIATION] = std::bind(&InputMethodContextTcoreWl::SetInputPanelLayoutVariation, this, variation);

  return result;
}

Dali::InputMethod::PanelLayoutVariation InputMethodContextTcoreWl::GetInputPanelLayoutVariation() const
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "InputMethodContextTcoreWl::GetInputPanelLayoutVariation\n");

  if(mIMFContext)
  {
    tizen_core_imf_input_panel_layout_e tcoreLayout  = TIZEN_CORE_IMF_INPUT_PANEL_LAYOUT_NORMAL;
    int                                 rawVariation = TIZEN_CORE_IMF_LAYOUT_NORMAL_VARIATION_NORMAL;
    if(!IsTcoreSuccess(tizen_core_imf_context_get_input_panel_layout(mIMFContext, &tcoreLayout)) ||
       !IsTcoreSuccess(tizen_core_imf_context_get_input_panel_layout_variation(mIMFContext, &rawVariation)))
    {
      return Dali::InputMethod::PanelLayoutVariation::NORMAL_NORMAL;
    }
    return ToPanelLayoutVariation(ToDaliPanelLayout(tcoreLayout), rawVariation);
  }

  return Dali::InputMethod::PanelLayoutVariation::NORMAL_NORMAL;
}

bool InputMethodContextTcoreWl::ProcessEventKeyDown(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);
  if(mIMFContext)
  {
    Integration::KeyEvent integKeyEvent(keyEvent.GetKeyName(), keyEvent.GetLogicalKey(), keyEvent.GetKeyString(), keyEvent.GetKeyCode(), keyEvent.GetKeyModifier(), keyEvent.GetTime(), static_cast<Integration::KeyEvent::State>(keyEvent.GetState()), keyEvent.GetCompose(), keyEvent.GetDeviceName(), keyEvent.GetDeviceClass(), keyEvent.GetDeviceSubclass());
    String                key        = integKeyEvent.logicalKey;
    String                compose    = integKeyEvent.compose;
    String                deviceName = integKeyEvent.deviceName;

    // We're consuming key down event so we have to pass to InputMethodContext so that it can parse it as well.
    ScopedTcoreEventKey keyDownEvent;
    if(!IsTcoreSuccess(tizen_core_imf_event_key_create(&keyDownEvent.handle)) || !keyDownEvent.handle)
    {
      return false;
    }

    // Build event metadata on a best-effort basis. The filter result below is
    // the only source of truth for whether the IME consumed this event.
    tizen_core_imf_event_key_set_keyname(keyDownEvent.handle, integKeyEvent.keyName.CStr());
    tizen_core_imf_event_key_set_key(keyDownEvent.handle, key.CStr());
    tizen_core_imf_event_key_set_string(keyDownEvent.handle, integKeyEvent.keyString.CStr());
    tizen_core_imf_event_key_set_compose(keyDownEvent.handle, compose.CStr());
    tizen_core_imf_event_key_set_timestamp(keyDownEvent.handle, static_cast<unsigned int>(integKeyEvent.time));
    tizen_core_imf_event_key_set_modifiers(keyDownEvent.handle, TcoreInputModifierToImfModifier(integKeyEvent.keyModifier));
    tizen_core_imf_event_key_set_locks(keyDownEvent.handle, TcoreInputModifierToImfLock(integKeyEvent.keyModifier));
    tizen_core_imf_event_key_set_device_name(keyDownEvent.handle, deviceName.CStr());
    tizen_core_imf_event_key_set_device_class(keyDownEvent.handle, TIZEN_CORE_IMF_DEVICE_CLASS_KEYBOARD);
    tizen_core_imf_event_key_set_device_subclass(keyDownEvent.handle, TIZEN_CORE_IMF_DEVICE_SUBCLASS_NONE);
    tizen_core_imf_event_key_set_keycode(keyDownEvent.handle, static_cast<unsigned int>(integKeyEvent.keyCode));

    // If the device is IME and the focused key is the direction keys, then we should send a key event to move a key cursor.
    if((integKeyEvent.deviceName == "ime") && ((!strncmp(integKeyEvent.keyName.CStr(), "Left", 4)) ||
                                               (!strncmp(integKeyEvent.keyName.CStr(), "Right", 5)) ||
                                               (!strncmp(integKeyEvent.keyName.CStr(), "Up", 2)) ||
                                               (!strncmp(integKeyEvent.keyName.CStr(), "Down", 4))))
    {
      eventHandled = false;
    }
    else
    {
      bool filtered = false;
      eventHandled = IsTcoreSuccess(tizen_core_imf_context_filter_event(mIMFContext,
                                                                        TIZEN_CORE_IMF_EVENT_TYPE_KEY_DOWN,
                                                                        keyDownEvent.handle,
                                                                        &filtered)) && filtered;
    }

    // If the event has not been handled by InputMethodContext then check if we should reset our input method context
    if(!eventHandled)
    {
      if(!strcmp(integKeyEvent.keyName.CStr(), "Escape") ||
         !strcmp(integKeyEvent.keyName.CStr(), "Return") ||
         !strcmp(integKeyEvent.keyName.CStr(), "KP_Enter"))
      {
        tizen_core_imf_context_reset(mIMFContext);
      }
    }
  }
  return eventHandled;
}

bool InputMethodContextTcoreWl::ProcessEventKeyUp(const Dali::KeyEvent& keyEvent)
{
  bool eventHandled(false);
  if(mIMFContext)
  {
    Integration::KeyEvent integKeyEvent(keyEvent.GetKeyName(), keyEvent.GetLogicalKey(), keyEvent.GetKeyString(), keyEvent.GetKeyCode(), keyEvent.GetKeyModifier(), keyEvent.GetTime(), static_cast<Integration::KeyEvent::State>(keyEvent.GetState()), keyEvent.GetCompose(), keyEvent.GetDeviceName(), keyEvent.GetDeviceClass(), keyEvent.GetDeviceSubclass());
    String                key        = integKeyEvent.logicalKey;
    String                compose    = integKeyEvent.compose;
    String                deviceName = integKeyEvent.deviceName;

    // We're consuming key up event so we have to pass to InputMethodContext so that it can parse it as well.
    ScopedTcoreEventKey keyUpEvent;
    if(!IsTcoreSuccess(tizen_core_imf_event_key_create(&keyUpEvent.handle)) || !keyUpEvent.handle)
    {
      return false;
    }

    // Build event metadata on a best-effort basis. The filter result below is
    // the only source of truth for whether the IME consumed this event.
    tizen_core_imf_event_key_set_keyname(keyUpEvent.handle, integKeyEvent.keyName.CStr());
    tizen_core_imf_event_key_set_key(keyUpEvent.handle, key.CStr());
    tizen_core_imf_event_key_set_string(keyUpEvent.handle, integKeyEvent.keyString.CStr());
    tizen_core_imf_event_key_set_compose(keyUpEvent.handle, compose.CStr());
    tizen_core_imf_event_key_set_timestamp(keyUpEvent.handle, static_cast<unsigned int>(integKeyEvent.time));
    tizen_core_imf_event_key_set_modifiers(keyUpEvent.handle, TcoreInputModifierToImfModifier(integKeyEvent.keyModifier));
    tizen_core_imf_event_key_set_locks(keyUpEvent.handle, TcoreInputModifierToImfLock(integKeyEvent.keyModifier));
    tizen_core_imf_event_key_set_device_name(keyUpEvent.handle, deviceName.CStr());
    tizen_core_imf_event_key_set_device_class(keyUpEvent.handle, TIZEN_CORE_IMF_DEVICE_CLASS_KEYBOARD);
    tizen_core_imf_event_key_set_device_subclass(keyUpEvent.handle, TIZEN_CORE_IMF_DEVICE_SUBCLASS_NONE);
    tizen_core_imf_event_key_set_keycode(keyUpEvent.handle, static_cast<unsigned int>(integKeyEvent.keyCode));

    bool filtered = false;
    eventHandled = IsTcoreSuccess(tizen_core_imf_context_filter_event(mIMFContext,
                                                                      TIZEN_CORE_IMF_EVENT_TYPE_KEY_UP,
                                                                      keyUpEvent.handle,
                                                                      &filtered)) && filtered;
  }
  return eventHandled;
}

tizen_core_imf_keyboard_modifiers_e InputMethodContextTcoreWl::TcoreInputModifierToImfModifier(unsigned int modifierMask)
{
  unsigned int modifier(TIZEN_CORE_IMF_KEYBOARD_MODIFIERS_NONE); // If no other matches returns NONE.

  // modifierMask follows tizen_core_wl_modifier_e bit layout (see tizen_core_wl_modifier_e).
  if(modifierMask & TIZEN_CORE_WL_MODIFIER_SHIFT)
  {
    modifier |= TIZEN_CORE_IMF_KEYBOARD_MODIFIERS_SHIFT;
  }

  if(modifierMask & TIZEN_CORE_WL_MODIFIER_ALT)
  {
    modifier |= TIZEN_CORE_IMF_KEYBOARD_MODIFIERS_ALT;
  }

  if(modifierMask & TIZEN_CORE_WL_MODIFIER_CTRL)
  {
    modifier |= TIZEN_CORE_IMF_KEYBOARD_MODIFIERS_CTRL;
  }

  if(modifierMask & TIZEN_CORE_WL_MODIFIER_WIN)
  {
    modifier |= TIZEN_CORE_IMF_KEYBOARD_MODIFIERS_WIN;
  }

  if(modifierMask & TIZEN_CORE_WL_MODIFIER_ALTGR)
  {
    modifier |= TIZEN_CORE_IMF_KEYBOARD_MODIFIERS_ALTGR;
  }

  return static_cast<tizen_core_imf_keyboard_modifiers_e>(modifier);
}

tizen_core_imf_keyboard_locks_e InputMethodContextTcoreWl::TcoreInputModifierToImfLock(unsigned int modifierMask)
{
  unsigned int lock(TIZEN_CORE_IMF_KEYBOARD_LOCKS_NONE); // If no other matches, returns NONE.

  if(modifierMask & TIZEN_CORE_WL_LOCK_NUM)
  {
    lock |= TIZEN_CORE_IMF_KEYBOARD_LOCKS_NUM;
  }

  if(modifierMask & TIZEN_CORE_WL_LOCK_CAPS)
  {
    lock |= TIZEN_CORE_IMF_KEYBOARD_LOCKS_CAPS;
  }

  if(modifierMask & TIZEN_CORE_WL_LOCK_SCROLL)
  {
    lock |= TIZEN_CORE_IMF_KEYBOARD_LOCKS_SCROLL;
  }

  return static_cast<tizen_core_imf_keyboard_locks_e>(lock);
}

void InputMethodContextTcoreWl::OnStaged(Dali::Actor actor)
{
  tizen_core_wl_window_h window = GetWindowFromActor(actor);

  if(mWindow != window)
  {
    mWindow = window;

    // Reset
    Finalize();
    Initialize();
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
