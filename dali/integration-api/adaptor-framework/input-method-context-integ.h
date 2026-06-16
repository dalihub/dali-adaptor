#ifndef DALI_INTEGRATION_INPUT_METHOD_CONTEXT_H
#define DALI_INTEGRATION_INPUT_METHOD_CONTEXT_H

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

// EXTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/input-method-options.h>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/adaptor-framework/input-method-context.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/signals/dali-signal.h>
#include <cstdint>

namespace Dali
{
namespace Integration
{
/**
 * @brief Integration-only InputMethodContext APIs.
 *
 * This namespace provides low-level input method integration APIs used by DALi
 * text controls, toolkit, and language bindings to integrate with the platform
 * input method framework.
 *
 * These APIs are not intended for application-level use. Applications should use
 * Dali::InputMethodContext from the public API.
 *
 * EventData, CallbackData, and EventReceivedSignal() represent the protocol-like
 * event path between the platform input method backend and DALi text controls.
 * They are kept in integration API to preserve existing toolkit and NUI binding
 * behavior without exposing them through the public InputMethodContext API.
 *
 * Applications that only need input panel private commands should use
 * Dali::InputMethodContext::PrivateCommandReceivedSignal().
 */
namespace InputMethodContext
{
/**
 * @brief Text direction used by the input method integration layer.
 */
enum TextDirection
{
  LEFT_TO_RIGHT,
  RIGHT_TO_LEFT,
};

/**
 * @brief Input panel language mode used by legacy integration consumers.
 */
enum class InputPanelLanguage
{
  AUTOMATIC,
  ALPHABET
};

/**
 * @brief Input method event type used by integration event signals.
 */
enum EventType
{
  VOID,
  PRE_EDIT,
  COMMIT,
  DELETE_SURROUNDING,
  GET_SURROUNDING,
  PRIVATE_COMMAND,
  SELECTION_SET
};

/**
 * @brief Preedit style type used by integration preedit attributes.
 */
enum class PreeditStyle
{
  NONE,
  UNDERLINE,
  REVERSE,
  HIGHLIGHT,
  CUSTOM_PLATFORM_STYLE_1,
  CUSTOM_PLATFORM_STYLE_2,
  CUSTOM_PLATFORM_STYLE_3,
  CUSTOM_PLATFORM_STYLE_4
};

/**
 * @brief Preedit style attribute range used by the input method integration layer.
 */
struct DALI_ADAPTOR_API PreeditAttributeData
{
  PreeditAttributeData()
  : preeditType(PreeditStyle::NONE),
    startIndex(0u),
    endIndex(0u)
  {
  }

  PreeditStyle preeditType;
  uint32_t     startIndex;
  uint32_t     endIndex;
};

/**
 * @brief Event data passed through the integration input method event signal.
 */
struct DALI_ADAPTOR_API EventData
{
  EventData()
  : predictiveString(),
    eventName(VOID),
    cursorOffset(0),
    numberOfChars(0),
    startIndex(0),
    endIndex(0)
  {
  }

  EventData(EventType aEventName, const Dali::String& aPredictiveString, int32_t aCursorOffset, int32_t aNumberOfChars)
  : predictiveString(aPredictiveString),
    eventName(aEventName),
    cursorOffset(aCursorOffset),
    numberOfChars(aNumberOfChars),
    startIndex(0),
    endIndex(0)
  {
  }

  EventData(EventType aEventName, int32_t aStartIndex, int32_t aEndIndex)
  : predictiveString(),
    eventName(aEventName),
    cursorOffset(0),
    numberOfChars(0),
    startIndex(aStartIndex),
    endIndex(aEndIndex)
  {
  }

  Dali::String predictiveString;
  EventType    eventName;
  int32_t      cursorOffset;
  int32_t      numberOfChars;
  int32_t      startIndex;
  int32_t      endIndex;
};

/**
 * @brief Callback result returned from integration input method event handlers.
 */
struct DALI_ADAPTOR_API CallbackData
{
  CallbackData()
  : currentText(),
    cursorPosition(0),
    update(false),
    preeditResetRequired(false)
  {
  }

  CallbackData(bool aUpdate, int32_t aCursorPosition, const Dali::String& aCurrentText, bool aPreeditResetRequired)
  : currentText(aCurrentText),
    cursorPosition(aCursorPosition),
    update(aUpdate),
    preeditResetRequired(aPreeditResetRequired)
  {
  }

  Dali::String currentText;
  int32_t      cursorPosition;
  bool         update : 1;
  bool         preeditResetRequired : 1;
};

/**
 * @brief Data received from the platform IME content commit event.
 *
 * This integration-only payload carries backend/platform boundary information
 * for IME content commits. It is intentionally kept out of the public
 * InputMethodContext API so the platform payload can evolve without fixing a
 * public application ABI.
 *
 * The current platform event maps the content URI, description, and MIME types
 * from the input method backend.
 *
 * @see ContentReceivedSignal
 */
struct DALI_ADAPTOR_API ContentReceivedData
{
  /**
   * @brief Default constructor.
   */
  ContentReceivedData()
  : contentUri(),
    description(),
    mimeTypes()
  {
  }

  /**
   * @brief Creates content received data.
   *
   * @param[in] contentUri The URI of the received content.
   * @param[in] description The description of the received content.
   * @param[in] mimeTypes The MIME types of the received content.
   */
  ContentReceivedData(const Dali::String& contentUri,
                      const Dali::String& description,
                      const Dali::String& mimeTypes)
  : contentUri(contentUri),
    description(description),
    mimeTypes(mimeTypes)
  {
  }

  Dali::String contentUri;  ///< The URI of the received content.
  Dali::String description; ///< The description of the received content.
  Dali::String mimeTypes;   ///< The MIME types of the received content.
};

/**
 * @brief Signal type used for low-level input method events.
 */
using KeyboardEventSignalType = Signal<CallbackData(Dali::InputMethodContext, const EventData&)>;

/**
 * @brief Signal type used to deliver IME content commit data across the platform integration boundary.
 */
using ContentReceivedSignalType = Signal<void(Dali::InputMethodContext, const ContentReceivedData&)>;

/**
 * @brief Legacy bool status signal type used by language bindings.
 */
using LegacyStatusChangedSignalType = Signal<void(bool)>;

/**
 * @brief Legacy keyboard type changed signal type used by language bindings.
 */
using LegacyKeyboardTypeChangedSignalType = Signal<void(Dali::InputMethodContext::KeyboardType)>;

/**
 * @brief Legacy keyboard resized signal type used by language bindings.
 */
using LegacyKeyboardResizedSignalType = Signal<void(int)>;

/**
 * @brief Legacy language changed signal type used by language bindings.
 */
using LegacyLanguageChangedSignalType = Signal<void(int)>;

/**
 * @brief Legacy content received signal type used by language bindings.
 */
using LegacyContentReceivedSignalType = Signal<void(const char*, const char*, const char*)>;

/**
 * @brief Container type for preedit style attributes.
 */
using PreEditAttributeDataContainer = Vector<PreeditAttributeData>;

/**
 * @brief Creates an input method context for integration use.
 * @return A handle to the newly created InputMethodContext.
 */
DALI_ADAPTOR_API Dali::InputMethodContext New();

/**
 * @brief Creates an input method context associated with the given actor.
 * @param[in] actor The actor associated with the input method context.
 * @return A handle to the newly created InputMethodContext.
 */
DALI_ADAPTOR_API Dali::InputMethodContext New(Dali::Actor actor);

/**
 * @brief Finalizes the input method context.
 * @param[in] context The input method context.
 */
DALI_ADAPTOR_API void Finalize(Dali::InputMethodContext context);

/**
 * @brief Activates the input method context.
 * @param[in] context The input method context.
 */
DALI_ADAPTOR_API void Activate(Dali::InputMethodContext context);

/**
 * @brief Deactivates the input method context.
 * @param[in] context The input method context.
 */
DALI_ADAPTOR_API void Deactivate(Dali::InputMethodContext context);

/**
 * @brief Resets the preedit state or input method module state.
 * @param[in] context The input method context.
 */
DALI_ADAPTOR_API void Reset(Dali::InputMethodContext context);

/**
 * @brief Notifies the input method context that the cursor position has changed.
 * @param[in] context The input method context.
 */
DALI_ADAPTOR_API void NotifyCursorPosition(Dali::InputMethodContext context);

/**
 * @brief Sets the cursor position stored in the input method context.
 * @param[in] context The input method context.
 * @param[in] cursorPosition The cursor position.
 */
DALI_ADAPTOR_API void SetCursorPosition(Dali::InputMethodContext context, uint32_t cursorPosition);

/**
 * @brief Gets the cursor position stored in the input method context.
 * @param[in] context The input method context.
 * @return The cursor position.
 */
DALI_ADAPTOR_API uint32_t GetCursorPosition(Dali::InputMethodContext context);

/**
 * @brief Sets the surrounding text used by the input method context.
 * @param[in] context The input method context.
 * @param[in] text The surrounding text.
 */
DALI_ADAPTOR_API void SetSurroundingText(Dali::InputMethodContext context, const Dali::String& text);

/**
 * @brief Gets the surrounding text stored in the input method context.
 * @param[in] context The input method context.
 * @return The surrounding text.
 */
DALI_ADAPTOR_API Dali::String GetSurroundingText(Dali::InputMethodContext context);

/**
 * @brief Notifies whether the associated text input supports multiple lines.
 * @param[in] context The input method context.
 * @param[in] multiLine True if the associated text input is multiline.
 */
DALI_ADAPTOR_API void NotifyTextInputMultiLine(Dali::InputMethodContext context, bool multiLine);

/**
 * @brief Gets the text direction of the current input method language.
 * @param[in] context The input method context.
 * @return The text direction.
 */
DALI_ADAPTOR_API TextDirection GetTextDirection(Dali::InputMethodContext context);

/**
 * @brief Sets the allowed MIME types to deliver from the platform input method backend.
 *
 * This integration API is used at the DALi/platform backend boundary to
 * configure which IME content commit MIME types should be accepted for the
 * current text input.
 *
 * @param[in] context The input method context.
 * @param[in] mimeTypes The allowed MIME types.
 */
DALI_ADAPTOR_API void SetContentMimeTypes(Dali::InputMethodContext context, const Dali::String& mimeTypes);

/**
 * @brief Applies input method options to the input method context.
 *
 * This API is kept for compatibility with the existing toolkit
 * INPUT_METHOD_SETTINGS path. New code that needs to control input panel
 * options directly should use Dali::InputMethodContext input panel APIs such as
 * SetInputPanelLayout(), SetInputPanelReturnKeyType(),
 * SetInputPanelAutoCapitalType(), and SetInputPanelLayoutVariation().
 *
 * @param[in] context The input method context.
 * @param[in] options The input method options.
 */
DALI_ADAPTOR_API void ApplyOptions(Dali::InputMethodContext context, const Dali::Integration::InputMethodOptions& options);

/**
 * @brief Sets the language mode of the input panel for legacy integration consumers.
 * @param[in] context The input method context.
 * @param[in] language The language mode.
 * @return True on success, false otherwise.
 */
DALI_ADAPTOR_API bool SetInputPanelLanguage(Dali::InputMethodContext context, InputPanelLanguage language);

/**
 * @brief Gets the language mode of the input panel for legacy integration consumers.
 * @param[in] context The input method context.
 * @return The language mode.
 */
DALI_ADAPTOR_API InputPanelLanguage GetInputPanelLanguage(Dali::InputMethodContext context);

/**
 * @brief Filters a key event through the input method context.
 * @param[in] context The input method context.
 * @param[in] keyEvent The key event to filter.
 * @return True if the key event was handled by the input method context.
 */
DALI_ADAPTOR_API bool FilterEventKey(Dali::InputMethodContext context, const Dali::KeyEvent& keyEvent);

/**
 * @brief Gets preedit style attributes from the input method context.
 * @param[in] context The input method context.
 * @param[out] attrs The preedit style attributes.
 */
DALI_ADAPTOR_API void GetPreeditStyle(Dali::InputMethodContext context, PreEditAttributeDataContainer& attrs);

/**
 * @brief Gets the legacy input method event signal.
 * @param[in] context The input method context.
 * @return The legacy input method event signal.
 */
DALI_ADAPTOR_API KeyboardEventSignalType& EventReceivedSignal(Dali::InputMethodContext context);

/**
 * @brief Gets the DALi-internal input method event signal.
 * @param[in] context The input method context.
 * @return The DALi-internal input method event signal.
 */
DALI_ADAPTOR_API KeyboardEventSignalType& KeyboardEventReceivedSignal(Dali::InputMethodContext context);

/**
 * @brief Gets the integration content received signal.
 *
 * The signal delivers structured IME content commit information from the
 * platform input method backend to DALi integration consumers.
 *
 * @param[in] context The input method context.
 * @return The integration content received signal.
 */
DALI_ADAPTOR_API ContentReceivedSignalType& ContentReceivedSignal(Dali::InputMethodContext context);

/**
 * @brief Gets the legacy bool status signal used by language bindings.
 * @param[in] context The input method context.
 * @return The legacy status signal.
 */
DALI_ADAPTOR_API LegacyStatusChangedSignalType& LegacyStatusChangedSignal(Dali::InputMethodContext context);

/**
 * @brief Gets the legacy keyboard type changed signal used by language bindings.
 * @param[in] context The input method context.
 * @return The legacy keyboard type changed signal.
 */
DALI_ADAPTOR_API LegacyKeyboardTypeChangedSignalType& LegacyKeyboardTypeChangedSignal(Dali::InputMethodContext context);

/**
 * @brief Gets the legacy keyboard resized signal used by language bindings.
 * @param[in] context The input method context.
 * @return The legacy keyboard resized signal.
 */
DALI_ADAPTOR_API LegacyKeyboardResizedSignalType& LegacyResizedSignal(Dali::InputMethodContext context);

/**
 * @brief Gets the legacy language changed signal used by language bindings.
 * @param[in] context The input method context.
 * @return The legacy language changed signal.
 */
DALI_ADAPTOR_API LegacyLanguageChangedSignalType& LegacyLanguageChangedSignal(Dali::InputMethodContext context);

/**
 * @brief Gets the legacy content received signal used by language bindings.
 * @param[in] context The input method context.
 * @return The legacy content received signal.
 */
DALI_ADAPTOR_API LegacyContentReceivedSignalType& LegacyContentReceivedSignal(Dali::InputMethodContext context);

} // namespace InputMethodContext
} // namespace Integration
} // namespace Dali

#endif // DALI_INTEGRATION_INPUT_METHOD_CONTEXT_H
