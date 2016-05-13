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
// CLASS HEADER
#include "text-input-listeners.h"

// INTERNAL INCLUDES
#include <input/text/text-input-interface.h>
#include <input/seat.h>


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{

void Enter( void* data, WlTextInput* textInput, WlSurface* surface )
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->Enter( seat, surface );
}

void Leave( void* data, WlTextInput* textInput )
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->Leave( seat );
}

void ModifiersMap( void* data, WlTextInput* textInput, struct wl_array* map )
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->ModifiersMap( seat, map );
}

void InputPanelState( void* data, WlTextInput* textInput, uint32_t state )
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->InputPanelState( seat, state );
}

void PreeditString( void* data, WlTextInput* textInput, uint32_t serial, const char* text, const char* commit )
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->PreeditString( seat, serial, text, commit );
}

void PreeditStyling( void* data, WlTextInput* textInput, uint32_t index, uint32_t length, uint32_t style)
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->PreeditStyling( seat, index, length, style );
}

void PreeditCursor(void* data,
           WlTextInput* textInput,
           int32_t index)
{

  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->PreeditCursor( seat, index );
}

void CommitString(void* data,
          WlTextInput* textInput,
          uint32_t serial,
          const char* text)
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->CommitString( seat, serial, text );
}

void CursorPosition(void* data,
      WlTextInput* textInput,
      int32_t index,
      int32_t anchor)
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->CursorPosition( seat, index, anchor );
}

void DeleteSurroundingText(void* data,
          WlTextInput* textInput,
          int32_t index,
          uint32_t length)
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->DeleteSurroundingText( seat, index, length );
}

void Keysym(void* data,
         WlTextInput* textInput,
         uint32_t serial,
         uint32_t time,
         uint32_t sym,
         uint32_t state,
         uint32_t modifiers)
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->Keysym( seat, serial, time, sym, state, modifiers );
}

void Language(void* data,
     WlTextInput* textInput,
     uint32_t serial,
     const char* language)
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->Language( seat, serial, language );
}

void TextDirection(void* data,
           WlTextInput* textInput,
           uint32_t serial,
           uint32_t direction)
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->TextDirection( seat, serial, direction );
}

void SelectionRegion(void* data,
       WlTextInput* textInput,
       uint32_t serial,
       int32_t start,
       int32_t end)
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->SelectionRegion( seat, serial, start, end );
}

void PrivateCommand(void* data,
      WlTextInput* textInput,
      uint32_t serial,
      const char* command)
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->PrivateCommand( seat, serial, command );
}

void InputPanelGeometry(void* data,
           WlTextInput* textInput,
           uint32_t x,
           uint32_t y,
           uint32_t width,
           uint32_t height)
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->InputPanelGeometry( seat, x, y, width, height );
}

void InputPanelData( void* data,
          WlTextInput* textInput,
          uint32_t serial,
          const char* input_panel_data,
          uint32_t input_panel_data_length)
{
  TextInputInterface* input = static_cast< TextInputInterface* >( data );
  Seat* seat = input->GetSeat( textInput );

  input->InputPanelData( seat, serial, input_panel_data, input_panel_data_length);
}


/**
 * If when running DALi on target a message like
 * listener function for opcode 16 of wl_text_input is NULL,
 * then it means the interface has been updated, and they've added an extra function
 * to the listener
 */
const WlTextInputListener TextInputListener =
{
  Enter,
  Leave,
  ModifiersMap,
  InputPanelState,
  PreeditString,
  PreeditStyling,
  PreeditCursor,
  CommitString,
  CursorPosition,
  DeleteSurroundingText,
  Keysym,
  Language,
  TextDirection,
  SelectionRegion,
  PrivateCommand,
  InputPanelGeometry,
  InputPanelData
};

} // unnamed namespace


namespace Wayland
{

const WlTextInputListener* GetTextInputListener()
{
  return &TextInputListener;
}

} // Wayland
} // Adaptor
} // Internal
} // Dali
