#ifndef __DALI_WAYLAND_TEXT_INPUT_MANAGER_H__
#define __DALI_WAYLAND_TEXT_INPUT_MANAGER_H__

/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <vector>

// INTERNAL INCLUDES
#include <input/text/text-input-interface.h>
#include <input/seat.h>
#include <virtual-keyboard.h>
#include <wl-types.h>
#include <base/interfaces/window-event-interface.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{


/**
 * Handles wayland text input interface which deals with the input panel (virtual keyboard ).
 *
 */
class TextInputManager : private TextInputInterface
{

public:

  /**
   * @brief Constructor
   */
  TextInputManager();

  /**
   * @brief Destructor
   */
  ~TextInputManager();

  /**
   * @brief Assign the window event interface
   * Used to send key events to DALi
   * @param[in]  eventInterface event interface
   */
  void AssignWindowEventInterface( WindowEventInterface* eventInterface );

  /**
   * @brief Assign the Wayland connection
   * @param[in] display Wayland display
   */
  void AssignDisplay( WlDisplay* display );

  /**
   * @brief Add a seat
   * @param seat seat pointer
   */
  void AddSeat( Seat* seat );

  /**
   * Get the global TextInputManager
   */
  static TextInputManager& Get();

private:  // TextInputInterface

  /**
   * @copydoc TextInputInterface::Enter
   */
  virtual void Enter( Seat* seat, WlSurface* surface );

  /**
   * @copydoc TextInputInterface::Leave
   */
  virtual void Leave( Seat* seat );

  /**
   * @copydoc TextInputInterface::ModifiersMap
   */
  virtual void ModifiersMap( Seat* seat, WlArray *map );

  /**
   * @copydoc TextInputInterface::InputPanelState
   */
  virtual void InputPanelState( Seat* seat, uint32_t state );

  /**
   * @copydoc TextInputInterface::PreeditString
   */
  virtual void PreeditString( Seat* seat, uint32_t serial, const char *text, const char *commit );

  /**
   * @copydoc TextInputInterface::PreeditStyling
   */
  virtual void PreeditStyling( Seat* seat, uint32_t index, uint32_t length, uint32_t style );

  /**
   * @copydoc TextInputInterface::PreeditCursor
   */
  virtual void PreeditCursor( Seat* seat, int32_t index );

  /**
   * @copydoc TextInputInterface::CommitString
   */
  virtual void CommitString( Seat* seat, uint32_t serial, const char *text );

  /**
   * @copydoc TextInputInterface::CursorPosition
   */
  virtual void CursorPosition( Seat* seat, int32_t index, int32_t anchor );

  /**
   * @copydoc TextInputInterface::DeleteSurroundingText
   */
  virtual void DeleteSurroundingText( Seat* seat, int32_t index, uint32_t length );


  /**
   * @copydoc TextInputInterface::Keysym
   */
  virtual void Keysym( Seat* seat,
                       uint32_t serial,
                       uint32_t time,
                       uint32_t sym,
                       uint32_t state,
                       uint32_t modifiers);

  /**
   * @copydoc TextInputInterface::Language
   */
  virtual void Language( Seat* seat, uint32_t serial, const char *language );

  /**
   * @copydoc TextInputInterface::TextDirection
   */
  virtual void TextDirection( Seat* seat, uint32_t serial, uint32_t direction );

  /**
   * @copydoc TextInputInterface::SelectionRegion
   */
  virtual void SelectionRegion( Seat* seat, uint32_t serial, int32_t start, int32_t end );

  /**
   * @copydoc TextInputInterface::PrivateCommand
   */
  virtual void PrivateCommand( Seat* seat, uint32_t serial, const char *command );

  /**
   * @copydoc TextInputInterface::InputPanelGeometry
   */
  virtual void InputPanelGeometry( Seat* seat, uint32_t x, uint32_t y, uint32_t width, uint32_t height );

  /**
   * @copydoc TextInputInterface::GetSeat
   */
  virtual Seat* GetSeat( const WlTextInput* textInput );

public:

  /**
   * @brief show the input panel (virtual keyboard)
   */
  void ShowInputPanel();

  /**
   * @brief hide the input panel
   */
  void HideInputPanel();

  /**
   * @brief see if the input panel is visible
   * @return true if panel is visible
   */
  bool IsInputPanelVisible();

  /**
   * @brief set the return key type
   * @param[in] type return key type
   */
  void SetReturnKeyType( const InputMethod::ActionButton type );

public: // virtual keyboard signals

  /**
   * @copydoc Dali::VirtualKeyboard::StatusChangedSignal
   */
  Dali::VirtualKeyboard::StatusSignalType& StatusChangedSignal();

  /**
   * @copydoc Dali::VirtualKeyboard::ResizedSignal
   */
  Dali::VirtualKeyboard::VoidSignalType& ResizedSignal();

  /**
   * @copydoc Dali::VirtualKeyboard::LanguageChangedSignal
   */
  Dali::VirtualKeyboard::VoidSignalType& LanguageChangedSignal();

public:   ///< Input Panel Signals ( DALi currently doesn't use these, it only uses signals from IMF interface).

   typedef Signal< void ( unsigned int, const std::string, const std::string ) > PreEditStringSignalType;
   typedef Signal< void ( unsigned int, unsigned int length, unsigned int ) > PreEditStylingSignalType;
   typedef Signal< void ( int ) > PreEditCursorSignalType;
   typedef Signal< void ( unsigned int, const std::string ) > CommitStringSignalType;
   typedef Signal< void ( int , int ) > CursorPositionSignalType;
   typedef Signal< void ( int , unsigned int ) > DeleteSurroundingTextSignalType;
   typedef Signal< void ( unsigned int ,int, int ) > SelectionRegionSignalType;

   /**
    * @brief Notify when composing new text
    * Description from wayland-extension/protocol/text.xml:
    * Notify when a new composing text (pre-edit) should be set around the
    * current cursor position. Any previously set composing text should
    * be removed.
    *
    * The commit text can be used to replace the preedit text on reset (for example on unfocus).
    * The text input should also handle all preedit_style and preedit_cursor
    * events occurring directly before preedit_string.
    *
    * @code
    *   void YourCallbackName( unsigned int serial, std::string text, std::string commit );
    * @endcode
    */
   PreEditStringSignalType& PreEditStringSignal();

   /**
    * @brief Pre-edit styling
    *
    * Description from wayland-extension/protocol/text.xml:
    * Set styling information on composing text. The style is applied for
    * length bytes from index relative to the beginning of the composing
    * text (as byte offset). Multiple styles can be applied to a composing
    * text by sending multiple preedit_styling events.
    *
    *
    * @code
    *   void YourCallbackName( unsigned int index, unsigned int length, unsigned int style );
    * @endcode
    */
   PreEditStylingSignalType& PreEditStylingSignal();

   /**
    * @brief Notify pre-edit cursor position
    * Description from wayland-extension/protocol/text.xml:
    * Set the cursor position inside the composing text (as byte
    * offset) relative to the start of the composing text. When index is a
    * negative number no cursor is shown.
    *
    * @code
    *   void YourCallbackName( int index );
    * @endcode
    */
   PreEditCursorSignalType& PreeditCursorSignal();

   /**
    * @brief Commit string text
    * From wayland-extension/protocol/text.xml:
    * Notify when text should be inserted into the editor widget. The text to
    * commit could be either just a single character after a key press or the
    * result of some composing (pre-edit). It could be also an empty text
    * when some text should be removed (see delete_surrounding_text) or when
    * the input cursor should be moved (see cursor_position).
    *
    * Any previously set composing text should be removed.
    *
    * @code
    *   void YourCallbackName(  const std::string text );
    * @endcode
    */
   CommitStringSignalType& CommitStringSignal();

   /**
    * @brief Cursor position signal
    * From wayland-extension/protocol/text.xml:
    * Notify when the cursor or anchor position should be modified.
    * This event should be handled as part of a following commit_string  event.
    *  @code
    *   void YourCallbackName( int index, int anchor );
    * @endcode
    */
   CursorPositionSignalType& CursorPositionSignal();

   /**
    * @brief delete surrounding text
    * From wayland-extension/protocol/text.xml:
    * Notify when the text around the current cursor position should be deleted.
    * @code
    * Index is relative to the current cursor (in bytes).
    * Length is the length of deleted text (in bytes).
    *
    *  void YourCallbackName( int index, unsigned int length );
    * @endcode
    * This event should be handled as part of a following commit_string event.
    */
   DeleteSurroundingTextSignalType& DeleteSurroundingTextSignal();

   /**
    * @brief text selection region
    * Notify when the input panels ask to select the characters
    * from the start cursor position to the end cursor position.
    * * @code
    * Index is relative to the current cursor (in bytes).
    * Length is the length of deleted text (in bytes).
    *
    * void YourCallbackName( unsigned int serial, int start, int end );
    * @endcode
    */
   SelectionRegionSignalType& SelectionRegionSignal();

public:

  /**
   * POD to store text input data for each seat.
   */
  struct SeatInfo
  {
    SeatInfo()
    : mTextDirection( Dali::VirtualKeyboard::LeftToRight ),
      mSeat( NULL ),
      mInputPanelDimensions( 0,0,0,0 ),
      mReturnKeyType( InputMethod::ACTION_UNSPECIFIED ),
      mInputPanelVisible( false ),
      mFocused( false )
    {

    }
    Dali::VirtualKeyboard::TextDirection mTextDirection;
    Seat* mSeat;
    Dali::Rect<int> mInputPanelDimensions;    ///< size of the input panel
    std::string mLanguage;                    ///< input panel language
    InputMethod::ActionButton mReturnKeyType; ///< return key
    bool mInputPanelVisible:1;                ///< panel status
    bool mFocused:1;
  };

  /**
   * @brief get the last active seat
   * @return the last active seat
   */
  SeatInfo& GetLastActiveSeat();

private:

  WlDisplay* mDisplay;        ///< Wayland display, handles all the data sent from and to the compositor
  Seat* mLastActiveSeat;      ///< Seat that was last used
  WindowEventInterface* mWindowEventInterface;

  std::vector< SeatInfo > mSeats;         ///< keeps track of text input information for each seat
  Dali::VirtualKeyboard::StatusSignalType mKeyboardStatusSignal;
  Dali::VirtualKeyboard::VoidSignalType   mKeyboardResizeSignal;
  Dali::VirtualKeyboard::VoidSignalType   mKeyboardLanguageChangedSignal;

  // Input Panel (Virtual Keyboard) signals
  PreEditStringSignalType                 mPreEditStringSignal;
  PreEditStylingSignalType                mPreEditStylingSignal;
  PreEditCursorSignalType                 mPreEditCursorSignal;
  CommitStringSignalType                  mCommitStringSignal;
  CursorPositionSignalType                mCursorPositionSignal;
  DeleteSurroundingTextSignalType         mDeleteSurroundingTextSignal;
  SelectionRegionSignalType               mSelectionRegionSignal;

};


} // Internal
} // Adaptor
} // Dali

#endif
