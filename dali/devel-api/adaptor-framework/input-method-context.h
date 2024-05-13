#ifndef DALI_INPUT_METHOD_CONTEXT_H
#define DALI_INPUT_METHOD_CONTEXT_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/events/key-event.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/input-method-options.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/dali-signal.h>

namespace Dali
{
namespace Internal DALI_INTERNAL
{
namespace Adaptor
{
class InputMethodContext;
}
} // namespace DALI_INTERNAL

class Actor;

/**
 * @brief The InputMethodContext class
 *
 * Specifically manages the ecore input method framework which enables the virtual or hardware keyboards.
 */
class DALI_ADAPTOR_API InputMethodContext : public BaseHandle
{
public:
  /**
   * @brief The direction of text.
   */
  enum TextDirection
  {
    LEFT_TO_RIGHT,
    RIGHT_TO_LEFT,
  };

  /**
   * @brief Events that are generated by the InputMethodContext.
   */
  enum EventType
  {
    VOID,               ///< No event
    PRE_EDIT,           ///< Pre-Edit changed
    COMMIT,             ///< Commit recieved
    DELETE_SURROUNDING, ///< Event to delete a range of characters from the string
    GET_SURROUNDING,    ///< Event to query string and cursor position
    PRIVATE_COMMAND,    ///< Private command sent from the input panel
    SELECTION_SET       ///< input method needs to set the selection
  };

  /**
   * @brief Enumeration for state of the input panel.
   */
  enum State
  {
    DEFAULT = 0, ///< Unknown state
    SHOW,        ///< Input panel is shown
    HIDE,        ///< Input panel is hidden
    WILL_SHOW    ///< Input panel in process of being shown
  };

  /**
   * @brief Enumeration for the type of Keyboard.
   */
  enum KeyboardType
  {
    SOFTWARE_KEYBOARD, ///< Software keyboard (Virtual keyboard) is default
    HARDWARE_KEYBOARD  ///< Hardware keyboard
  };

  /**
   * @brief Enumeration for the language mode of the input panel.
   */
  enum class InputPanelLanguage
  {
    AUTOMATIC, ///< IME Language automatically set depending on the system display
    ALPHABET   ///< Latin alphabet(default). It can be changed according to OSD(On Screen Display) language.
  };

  /**
   * @brief Enumeration for defining the types of Ecore_IMF Input Panel align.
   */
  enum class InputPanelAlign
  {
    TOP_LEFT,      ///< The top-left corner
    TOP_CENTER,    ///< The top-center position
    TOP_RIGHT,     ///< The top-right corner
    MIDDLE_LEFT,   ///< The middle-left position
    MIDDLE_CENTER, ///< The middle-center position
    MIDDLE_RIGHT,  ///< The middle-right position
    BOTTOM_LEFT,   ///< The bottom-left corner
    BOTTOM_CENTER, ///< The bottom-center position
    BOTTOM_RIGHT   ///< The bottom-right corner
  };

  /**
   * @brief Enumeration for the preedit style types.
   */
  enum class PreeditStyle
  {
    NONE,                    ///< None style
    UNDERLINE,               ///< Underline substring style
    REVERSE,                 ///< Reverse substring style
    HIGHLIGHT,               ///< Highlight substring style
    CUSTOM_PLATFORM_STYLE_1, ///< Custom style for platform
    CUSTOM_PLATFORM_STYLE_2, ///< Custom style for platform
    CUSTOM_PLATFORM_STYLE_3, ///< Custom style for platform
    CUSTOM_PLATFORM_STYLE_4  ///< Custom style for platform
  };

  /**
   * @brief This structure is for the preedit style types and indices.
   */
  struct PreeditAttributeData
  {
    PreeditAttributeData()
    : preeditType(PreeditStyle::NONE),
      startIndex(0),
      endIndex(0)
    {
    }

    PreeditStyle preeditType; /// The preedit style type
    unsigned int startIndex;  /// The start index of preedit
    unsigned int endIndex;    /// The end index of preedit
  };

  /**
   * @brief This structure is used to pass on data from the InputMethodContext regarding predictive text.
   */
  struct EventData
  {
    /**
     * @brief Default Constructor.
     */
    EventData()
    : predictiveString(),
      eventName(VOID),
      cursorOffset(0),
      numberOfChars(0),
      startIndex(0),
      endIndex(0){};

    /**
     * @brief Constructor
     *
     * @param[in] aEventName The name of the event from the InputMethodContext.
     * @param[in] aPredictiveString The pre-edit or commit string.
     * @param[in] aCursorOffset Start position from the current cursor position to start deleting characters.
     * @param[in] aNumberOfChars The number of characters to delete from the cursorOffset.
     */
    EventData(EventType aEventName, const std::string& aPredictiveString, int aCursorOffset, int aNumberOfChars)
    : predictiveString(aPredictiveString),
      eventName(aEventName),
      cursorOffset(aCursorOffset),
      numberOfChars(aNumberOfChars),
      startIndex(0),
      endIndex(0)
    {
    }

    /**
     * @brief Constructor
     *
     * @param[in] aEventName The name of the event from the InputMethodContext.
     * @param[in] aStartIndex The start index of selection.
     * @param[in] aEndIndex The end index of selection.
     */
    EventData(EventType aEventName, int aStartIndex, int aEndIndex)
    : predictiveString(),
      eventName(aEventName),
      cursorOffset(0),
      numberOfChars(0),
      startIndex(aStartIndex),
      endIndex(aEndIndex)
    {
    }

    // Data
    std::string predictiveString; ///< The pre-edit or commit string.
    EventType   eventName;        ///< The name of the event from the InputMethodContext.
    int         cursorOffset;     ///< Start position from the current cursor position to start deleting characters.
    int         numberOfChars;    ///< number of characters to delete from the cursorOffset.
    int         startIndex;       ///< The start index of selection.
    int         endIndex;         ///< The end index of selection.
  };

  /**
   * @brief Data required by InputMethodContext from the callback
   */
  struct CallbackData
  {
    /**
     * @brief Constructor
     */
    CallbackData()
    : currentText(),
      cursorPosition(0),
      update(false),
      preeditResetRequired(false)
    {
    }

    /**
     * @brief Constructor
     * @param[in] aUpdate True if cursor position needs to be updated
     * @param[in] aCursorPosition new position of cursor
     * @param[in] aCurrentText current text string
     * @param[in] aPreeditResetRequired flag if preedit reset is required.
     */
    CallbackData(bool aUpdate, int aCursorPosition, const std::string& aCurrentText, bool aPreeditResetRequired)
    : currentText(aCurrentText),
      cursorPosition(aCursorPosition),
      update(aUpdate),
      preeditResetRequired(aPreeditResetRequired)
    {
    }

    std::string currentText;              ///< current text string
    int         cursorPosition;           ///< new position of cursor
    bool        update : 1;               ///< if cursor position needs to be updated
    bool        preeditResetRequired : 1; ///< flag if preedit reset is required.
  };

  typedef Signal<void(InputMethodContext&)>                                        ActivatedSignalType;     ///< Keyboard actived signal
  typedef Signal<CallbackData(InputMethodContext&, const EventData&)>              KeyboardEventSignalType; ///< keyboard events
  typedef Signal<void()>                                                           VoidSignalType;
  typedef Signal<void(bool)>                                                       StatusSignalType;
  typedef Signal<void(KeyboardType)>                                               KeyboardTypeSignalType;    ///< keyboard type
  typedef Signal<void(int)>                                                        KeyboardResizedSignalType; ///< Keyboard resized signal
  typedef Signal<void(int)>                                                        LanguageChangedSignalType; ///< Language changed signal
  typedef Signal<void(const std::string&, const std::string&, const std::string&)> ContentReceivedSignalType; ///< Content received signal

  using PreEditAttributeDataContainer = Vector<Dali::InputMethodContext::PreeditAttributeData>;

public:
  /**
   * @brief Constructor.
   */
  InputMethodContext();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~InputMethodContext();

  /**
   * @brief Create a new instance of an InputMethodContext.
   */
  static InputMethodContext New();

  /**
   * @brief Create a new instance of an InputMethodContext.
   *
   * @param[in] actor The actor that uses the new InputMethodContext instance.
   */
  static InputMethodContext New(Actor actor);

  /**
   * @brief Copy constructor.
   *
   * @param[in] inputMethodContext InputMethodContext to copy. The copied inputMethodContext will point at the same implementation.
   */
  InputMethodContext(const InputMethodContext& inputMethodContext);

  /**
   * @brief Assignment operator.
   *
   * @param[in] inputMethodContext The InputMethodContext to assign from.
   * @return The updated InputMethodContext.
   */
  InputMethodContext& operator=(const InputMethodContext& inputMethodContext);

  /**
   * @brief Downcast a handle to InputMethodContext handle.
   *
   * If handle points to an InputMethodContext the downcast produces valid
   * handle. If not the returned handle is left uninitialized.
   *
   * @param[in] handle Handle to an object.
   * @return Handle to an InputMethodContext or an uninitialized handle.
   */
  static InputMethodContext DownCast(BaseHandle handle);

public:
  /**
   * @brief Finalize the InputMethodContext.
   *
   * It means that the context will be deleted.
   */
  void Finalize();

  /**
   * @brief Activate the InputMethodContext.
   *
   * It means that the text editing is started at somewhere.
   * If the H/W keyboard isn't connected then it will show the virtual keyboard.
   */
  void Activate();

  /**
   * @brief Deactivate the InputMethodContext.
   *
   * It means that the text editing is finished at somewhere.
   */
  void Deactivate();

  /**
   * @brief Get the restoration status, which controls if the keyboard is restored after the focus lost then regained.
   *
   * If true then keyboard will be restored (activated) after focus is regained.
   * @return restoration status.
   */
  bool RestoreAfterFocusLost() const;

  /**
   * @brief Set status whether the InputMethodContext has to restore the keyboard after losing focus.
   *
   * @param[in] toggle True means that keyboard should be restored after focus lost and regained.
   */
  void SetRestoreAfterFocusLost(bool toggle);

  /**
   * @brief Send message reset the pred-edit state / InputMethodContext module.
   *
   * Used to interupt pre-edit state maybe due to a touch input.
   */
  void Reset();

  /**
   * @brief Notifies InputMethodContext that the cursor position has changed, required for features like auto-capitalisation.
   */
  void NotifyCursorPosition();

  /**
   * @brief Sets cursor position stored in VirtualKeyboard, this is required by the InputMethodContext.
   *
   * @param[in] cursorPosition position of cursor
   */
  void SetCursorPosition(unsigned int cursorPosition);

  /**
   * @brief Gets cursor position stored in VirtualKeyboard, this is required by the InputMethodContext.
   *
   * @return current position of cursor
   */
  unsigned int GetCursorPosition() const;

  /**
   * @brief Method to store the string required by the InputMethodContext, this is used to provide predictive word suggestions.
   *
   * @param[in] text The text string surrounding the current cursor point.
   */
  void SetSurroundingText(const std::string& text);

  /**
   * @brief Gets current text string set within the InputMethodContext manager, this is used to offer predictive suggestions.
   *
   * @return current position of cursor
   */
  const std::string& GetSurroundingText() const;

  /**
 * @brief Notifies InputMethodContext that text input is set to multi line or not
 *
 * @param[in] multiLine True if multiline text input is used
 */
  void NotifyTextInputMultiLine(bool multiLine);

  /**
   * @brief Returns text direction of the keyboard's current input language.
   * @return The direction of the text.
   */
  TextDirection GetTextDirection();

  /**
   * @brief Provides size and position of keyboard.
   *
   * Position is relative to whether keyboard is visible or not.
   * If keyboard is not visible then position will be off the screen.
   * If keyboard is not being shown when this method is called the keyboard is partially setup (IMFContext) to get
   * the values then taken down.  So ideally GetInputMethodArea() should be called after Show().
   * @return rect which is keyboard panel x, y, width, height
   */
  Dali::Rect<int> GetInputMethodArea();

  /**
   * @brief Set one or more of the Input Method options
   * @param[in] options The options to be applied
   */
  void ApplyOptions(const InputMethodOptions& options);

  /**
   * @brief Sets up the input-panel specific data.
   * @param[in] data The specific data to be set to the input panel
   */
  void SetInputPanelData(const std::string& data);

  /**
   * @brief Gets the specific data of the current active input panel.
   *
   * Input Panel Data is not always the data which is set by SetInputPanelData().
   * Data can be changed internally in the input panel.
   * It is just used to get a specific data from the input panel to an application.
   * @param[in] data The specific data to be got from the input panel
   */
  void GetInputPanelData(std::string& data);

  /**
   * @brief Gets the state of the current active input panel.
   * @return The state of the input panel.
   */
  State GetInputPanelState();

  /**
   * @brief Sets the return key on the input panel to be visible or invisible.
   *
   * The default is true.
   * @param[in] visible True if the return key is visible(enabled), false otherwise.
   */
  void SetReturnKeyState(bool visible);

  /**
   * @brief Enable to show the input panel automatically when focused.
   * @param[in] enabled If true, the input panel will be shown when focused
   */
  void AutoEnableInputPanel(bool enabled);

  /**
   * @brief Shows the input panel.
   */
  void ShowInputPanel();

  /**
   * @brief Hides the input panel.
   */
  void HideInputPanel();

  /**
   * @brief Gets the keyboard type.
   *
   * The default keyboard type is SOFTWARE_KEYBOARD.
   * @return The keyboard type
   */
  KeyboardType GetKeyboardType();

  /**
   * @brief Gets the current language locale of the input panel.
   *
   * ex) en_US, en_GB, en_PH, fr_FR, ...
   * @return The current language locale of the input panel
   */
  std::string GetInputPanelLocale();

  /**
   * @brief Sets the allowed MIME Types to deliver to the input panel.
   *
   * ex) std::string mimeTypes = "text/plain,image/png,image/gif,application/pdf";
   *
   * You can receive a media content URI and its MIME type from ContentReceivedSignal(). @see ContentReceivedSignal
   * @param[in] mimeTypes The allowed MIME types
   */
  void SetContentMIMETypes(const std::string& mimeTypes);

  /**
   * @brief Process event key down or up, whether filter a key to isf.
   *
   * @param[in] keyEvent The event key to be handled.
   * @return Whether the event key is handled.
   */
  bool FilterEventKey(const Dali::KeyEvent& keyEvent);

  /**
   * @brief Sets whether the IM context should allow to use the text prediction.
   *
   * @param[in] prediction Whether to allow text prediction or not.
   */
  void AllowTextPrediction(bool prediction);

  /**
   * @brief Gets whether the IM context allow to use the text prediction.
   *
   * @return Whether the IM allow text prediction or not.
   */
  bool IsTextPredictionAllowed() const;

  /**
   * @brief Sets the language of the input panel.
   *
   * This method can be used when you want to show the English keyboard.
   * @param[in] language The language to be set to the input panel
   */
  void SetInputPanelLanguage(InputPanelLanguage language);

  /**
   * @brief Gets the language of the input panel.
   *
   * @return The language of the input panel
   */
  InputPanelLanguage GetInputPanelLanguage() const;

  /**
   * @brief Sets the x,y coordinates of the input panel.
   *
   * @param[in] x The top-left x coordinate of the input panel
   * @param[in] y The top-left y coordinate of the input panel
   */
  void SetInputPanelPosition(unsigned int x, unsigned int y);

  /**
   * @brief Sets the alignment and its x, y coordinates of the input panel.
   *
   * Regardless of the rotation degree, the x, y values of the top-left corner on the screen are based on 0, 0.
   * When the IME size is changed, its size will change according to the set alignment.
   *
   * @param[in] x The x coordinate of the InputPanelAlign value.
   * @param[in] y The y coordinate of the InputPanelAlign value.
   * @param[in] align one of the InputPanelAlign values specifying the desired alignment.
   * @return true on success, false otherwise.
   * @remarks This API can be used to set the alignment of a floating IME.
   */
  bool SetInputPanelPositionAlign(int x, int y, InputPanelAlign align);

  /**
   * @brief Gets the preedit attributes data.
   *
   * @param[out] attrs The preedit attributes data.
   */
  void GetPreeditStyle(PreEditAttributeDataContainer& attrs) const;

public:
  // Signals

  /**
   * @brief This is emitted when the virtual keyboard is connected to or the hardware keyboard is activated.
   *
   * @return The InputMethodContext Activated signal.
   */
  ActivatedSignalType& ActivatedSignal();

  /**
   * @brief This is emitted when the InputMethodContext manager receives an event from the InputMethodContext.
   *
   * @return The Event signal containing the event data.
   */
  KeyboardEventSignalType& EventReceivedSignal();

  /**
   * @brief Connect to this signal to be notified when the virtual keyboard is shown or hidden.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(bool keyboardShown);
   * @endcode
   * If the parameter keyboardShown is true, then the keyboard has just shown, if it is false, then it
   * has just been hidden.
   * @return The signal to connect to.
   */
  StatusSignalType& StatusChangedSignal();

  /**
   * @brief Connect to this signal to be notified when the virtual keyboard is resized.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName( int resolvedResize );
   * @endcode
   * The parameter sends the resolved resize defined by the InputMethodContext.
   *
   * User can get changed size by using GetInputMethodArea() in the callback
   * @return The signal to connect to.
   */
  KeyboardResizedSignalType& ResizedSignal();

  /**
   * @brief Connect to this signal to be notified when the virtual keyboard's language is changed.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName( int resolvedLanguage );
   * @endcode
   * The parameter sends the resolved language defined by the InputMethodContext.
   *
   * User can get the text direction of the language by calling GetTextDirection() in the callback.
   * @return The signal to connect to.
   */
  LanguageChangedSignalType& LanguageChangedSignal();

  /**
   * @brief Connect to this signal to be notified when the keyboard type is changed.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName( KeyboardType keyboard );
   * @endcode
   *
   * @return The signal to connect to.
   */
  KeyboardTypeSignalType& KeyboardTypeChangedSignal();

  /**
   * @brief Connect to this signal to be notified when the content, such as images, of input method is received.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName( const std::string& contentUri, const std::string& description, const std::string& contentMIMEType );
   * @endcode
   *
   * @return The signal to connect to.
   */
  ContentReceivedSignalType& ContentReceivedSignal();

public:
  /**
   * @brief This constructor is used by InputMethodContext::New().
   *
   * @param[in] inputMethodContext A pointer to the InputMethodContext.
   */
  explicit DALI_INTERNAL InputMethodContext(Internal::Adaptor::InputMethodContext* inputMethodContext);
};

} // namespace Dali

#endif // DALI_INPUT_METHOD_CONTEXT_H
