#ifndef __DALI_INTERNAL_ADAPTOR_TEXT_INPUT_INTERFACE_H__
#define __DALI_INTERNAL_ADAPTOR_TEXT_INPUT_INTERFACE_H__

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
#include <wl-types.h>


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class Seat;

/**
 * @brief interface used to wrap events that come form the Tizen specific Text input interface (wl_text_input_listener).
 *
 * wayland-extension/protocol/text.xml has been used for the callback descriptions
 *
 * The events are for from a specific seat & surface combination.
 *
 * Once the text input interface is enabled, all key presses come from it
 * except for specifc hardware keys ( volume up / down ) which still get routed
 * to the wl_keyboard interface.
 *
 *
 */
class TextInputInterface
{

public:

  /**
   * @brief Notify the text input has received focus. Typically in response to an activate request.
   *
   * @param[in] seat the seat that produced the event
   * @param[in] surface wayland surface
   */
  virtual void Enter( Seat* seat, WlSurface* surface ) = 0;

  /**
   * @brief Notify the text input when it lost focus.
   * Either in response to a deactivate request or when the assigned surface lost focus or was destroyed.
   * @param[in] seat the seat that produced the event
   */
  virtual void Leave( Seat* seat ) = 0;

  /**
   * @brief key modifiers names.
   * The position in the array is the index of the modifier as used in the modifiers
   * bitmask in the keysym event
   * @param[in] seat the seat that produced the event
   * @param[in] map modifier map
   */
  virtual void ModifiersMap( Seat* seat, WlArray *map ) = 0;

  /**
   * @brief called when the input panel state changes
   * @param[in] seat the seat that produced the event
   * @param[in] state 0 == panel hidden, 1 == panel visible
   */
  virtual void InputPanelState( Seat* seat, uint32_t state ) = 0;

  /**
   * @brief pre-edit string
   * Notify when a new composing text (pre-edit) should be set around the
   * current cursor position. Any previously set composing text should
   * be removed.
   * The commit text can be used to replace the preedit text on reset
   * (for example on unfocus).
   *
   * The text input should also handle all preedit_style and preedit_cursor
   * events occurring directly before preedit_string.
   * @param[in] seat the seat that produced the event
   * @param[in] serial of the latest known text input state
   * @param [in] text text
   * @param [in] commit  commit text
   *
   */
  virtual void PreeditString( Seat* seat, uint32_t serial, const char *text, const char *commit ) = 0;

  /**
   * @brief Set styling information on composing text.
   * The style is applied for length bytes from index relative to the beginning of the composing
   * text (as byte offset). Multiple styles can be applied to a composing
   * text by sending multiple preedit_styling events.
   * this event occurs with a preedit_string event.
   *
   * @param[in] seat the seat that produced the event
   * @param[in] index start of the text style
   * @param[in] length of the text style
   * @param[in] style text style
   */
  virtual void PreeditStyling( Seat* seat, uint32_t index, uint32_t length, uint32_t style ) = 0;

  /**
   * @brief Set the cursor position inside the composing text (as byte offset)
   * relative to the start of the composing text.
   * When index is a negative number no cursor is shown.
   *
   * this event occurs with a preedit_string event.
   * @param[in] seat the seat that produced the event
   * @param[in] index start of the text style
   */
  virtual void PreeditCursor( Seat* seat, int32_t index ) = 0;

  /**
   * @brief Notify when text should be inserted into a DALi text control.
   * The text to commit could be either just a single character after a key press or the
   * result of some composing (pre-edit). It could be also an empty text
   * when some text should be removed (see delete_surrounding_text) or when
   * the input cursor should be moved (see cursor_position).
   * Any previously set composing text should be removed.
   *
   * @param[in] seat the seat that produced the event
   * @param[in] serial of the latest known text input state
   * @param[in] text the text to commit
   */
  virtual void CommitString( Seat* seat, uint32_t serial, const char *text ) = 0;

  /**
   * @brief  Notify when the cursor or anchor position should be modified.
   * This event should be handled as part of a following commit_string event.
   *
   * @param[in] seat the seat that produced the event
   * @param[in] index relative to the current cursor (in bytes).
   * @param[in] anchor cursor anchor
   */
  virtual void CursorPosition( Seat* seat, int32_t index, int32_t anchor) = 0;

  /**
   * @brief  Notify when the text around the current cursor position should be deleted.
   * Index is relative to the current cursor (in bytes).
   * Length is the length of deleted text (in bytes).
   * This event should be handled as part of a following commit_string event.
   * @param[in] seat the seat that produced the event
   * @param[in] index relative to the current cursor (in bytes).
   * @param[in] length length of the text to be deleted
   */
  virtual void DeleteSurroundingText( Seat* seat, int32_t index,  uint32_t length ) = 0;


  /**
   * @brief Notify when a key event was sent
   * Key events should not be used for normal text input operations,
   * which should be done with commit_string, delete_surrounding_text, etc.
   * The key event follows the wl_keyboard key event convention.
   * Sym is a XKB keysym, state a wl_keyboard key_state.
   * Modifiers are a mask for effective modifiers
   * (where the modifier indices are set by the modifiers_map event)
   * @param[in] seat the seat that produced the event
   * @param[in] serial of the latest known text input state
   * @param[in] time time stamp
   * @param[in] sym symbol
   * @param[in] state state
   * @param[in] modifiers modifiers
   */
  virtual void Keysym( Seat* seat,
                       uint32_t serial,
                       uint32_t time,
                       uint32_t sym,
                       uint32_t state,
                       uint32_t modifiers) = 0;

  /**
   * @brief Set the language of the input text.
   * @param[in] seat the seat that produced the event
   * @param[in] serial of the latest known text input state
   * @param[in] language The "language" argument is a RFC-3066 format language tag.
   */
  virtual void Language( Seat* seat, uint32_t serial, const char *language ) = 0;

  /**
   * @brief Set the text direction of input text.
   *
   * It is mainly needed for showing input cursor on correct side of the
   * editor when there is no input yet done and making sure neutral
   * direction text is laid out properly.
    *
   * @param[in] seat the seat that produced the event
   * @param[in] serial of the latest known text input state
   * @param[in] direction ( see text_direction enum in wayland-extension/protocol/text.xml )
   */
  virtual void TextDirection( Seat* seat, uint32_t serial, uint32_t direction ) = 0;

  /**
   * @brief Notify when the input panels ask to select the characters
   * from the start cursor position to the end cursor position.
   *
   * @param[in] seat the seat that produced the event
   * @param[in] serial of the latest known text input state
   * @param[in] start start index
   * @param[in] end end index
   */
  virtual void SelectionRegion( Seat* seat, uint32_t serial, int32_t start, int32_t end) = 0;

  /**
   * @brief Notify when the input panels ask to send private command
   * @param[in] seat the seat that produced the event
   * @param[in] serial of the latest known text input state
   * @param[in] command private command string
   */
  virtual void PrivateCommand( Seat* seat, uint32_t serial, const char *command) = 0;

  /**
   * @brief Notify when the geometry of the input panel changed.
   * @param[in] seat the seat that produced the event
   * @param[in] x position
   * @param[in] y position
   * @param[in] width panel width
   * @param[in] height panel height
   */
  virtual void InputPanelGeometry( Seat* seat,
                                   uint32_t x,
                                   uint32_t y,
                                   uint32_t width,
                                   uint32_t height) = 0;

  /**
   * @brief Notify when the input panels ask to send input panel data
   * Not sure what this is for exactly
   * @param[in] seat the seat that produced the event
   * @param[in] serial of the latest known text input state
   * @param[in] data input panel data
   * @param[in] dataLength data length
   *
   */
  virtual void InputPanelData( Seat* seat,
                               uint32_t serial,
                               const char* data,
                               uint32_t dataLength ) = 0;

public: // Helper functions used to find the seat associated with the keyboard/pointer/touch device

  /**
   * @brief get the seat that contains the text input interface
   * @param[in] textInput wayland textinput interface
   * @return the seat the keyboard belongs to
   */
  virtual Seat* GetSeat( const WlTextInput* textInput) = 0;

protected:

  /**
   * @brief Constructor
   */
  TextInputInterface()
  {
  }

  /**
   * @brief destructor
   */
  virtual ~TextInputInterface()
  {
  }

  // Undefined copy constructor.
  TextInputInterface( const TextInputInterface& );

  // Undefined assignment operator.
  TextInputInterface& operator=( const TextInputInterface& );
};



} // Internal
} // Adaptor
} // Dali

#endif  //__DALI_INTERNAL_ADAPTOR_TEXT_INPUT_INTERFACE_H__
