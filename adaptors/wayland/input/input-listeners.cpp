/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include "input-listeners.h"

// EXTERNAL INCLUDES
#include <cctype>
#include <stdio.h>

// INTERNAL INCLUDES
#include "input-interface.h"
#include "seat.h"


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
/************
 *
 * Pointer events callbacks. See wl_pointer_listener in wayland-client-protocol.h for a description
 *
 ************/
void PointerEnter( void* data,
                   WlPointer* pointer,
                   unsigned int serial,
                   WlSurface* surface,
                   wl_fixed_t surfaceX,
                   wl_fixed_t surfaceY)
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( pointer );

  float x = static_cast<float>( wl_fixed_to_double(surfaceX) );
  float y = static_cast<float>( wl_fixed_to_double(surfaceY) );

  seat->SetPointerPosition( Vector2(x, y) ); // record last pointer position

  input->PointerEnter( seat, serial, surface, x, y );
}

void PointerLeave( void* data,
                   WlPointer* pointer,
                   unsigned int serial,
                   WlSurface* surface )
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( pointer );

  input->PointerLeave( seat, serial, surface );
}

void PointerMotion( void* data,
                    WlPointer* pointer,
                    unsigned int timestamp,
                    wl_fixed_t surfaceX,
                    wl_fixed_t surfaceY )
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( pointer );

  float x = static_cast<float>( wl_fixed_to_double( surfaceX) );
  float y = static_cast<float>( wl_fixed_to_double( surfaceY) );

  seat->SetPointerPosition( Vector2(x, y) ); // record last pointer position

  input->PointerMotion( seat, timestamp, x, y );
}

void PointerButton( void* data,
                    WlPointer* pointer,
                    unsigned int serial,
                    unsigned int timestamp,
                    unsigned int button,
                    unsigned int state )
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( pointer );

  input->PointerButton( seat, serial, timestamp, button, state );

}

void PointerAxis( void* data,
                  WlPointer* pointer,
                  unsigned int timestamp,
                  unsigned int axis,
                  wl_fixed_t value )
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( pointer );

  float length = static_cast<float>( wl_fixed_to_double( value ) );

  input->PointerAxis( seat, timestamp, axis, length);
}

/************
 *
 * Key event callbacks. See wl_keyboard_listener wayland-client-protocol.h for a description
 *
 ************/
void KeyboardKeymap( void* data,
                     WlKeyboard* keyboard,
                     unsigned int format,
                     int fd,
                     unsigned int size )
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( keyboard );

  input->KeyboardKeymap( seat, format, fd, size );

}

void KeyFocusEnter( void* data,
                    WlKeyboard* keyboard,
                    unsigned int serial,
                    WlSurface* surface,
                    WlArray* keys )
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( keyboard );

  input->KeyFocusEnter( seat, serial, surface, keys );
}

void KeyFocusLeave( void* data,
                    WlKeyboard* keyboard,
                    unsigned int serial,
                    WlSurface* surface )
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( keyboard );

  input->KeyFocusLeave( seat, serial, surface );
}

void KeyEvent( void* data,
               WlKeyboard* keyboard,
               unsigned int serial,
               unsigned int timestamp,
               unsigned int keycode,
               unsigned int state)
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( keyboard );

  input->KeyEvent( seat, serial, timestamp, keycode, state );
}

void KeyModifiers( void* data, WlKeyboard* keyboard,
                   unsigned int serial,
                   unsigned int depressed,
                   unsigned int latched,
                   unsigned int locked,
                   unsigned int group)
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( keyboard );

  input->KeyModifiers( seat, serial, depressed, latched, locked, group );


}

void KeyRepeatInfo( void* data,
                    WlKeyboard* keyboard,
                    int32_t rate,
                    int32_t delay)
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( keyboard );

  input->KeyRepeatInfo( seat, rate, delay );

}

/************
 *
 * Touch event callbacks See wl_touch_listener wayland-client-protocol.h for a description
 *
 ************/
void TouchDown( void* data,
                WlTouch* touch,
                unsigned int serial,
                unsigned int timestamp,
                WlSurface* surface,
                int touchId,
                wl_fixed_t surfaceX,
                wl_fixed_t surfaceY)
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( touch );

  float x = static_cast<float>( wl_fixed_to_double( surfaceX) );
  float y = static_cast<float>( wl_fixed_to_double( surfaceY) );


  input->TouchDown( seat, serial, timestamp, surface, touchId, x, y );
}

void TouchUp( void* data,
              WlTouch* touch,
              unsigned int serial,
              unsigned int timestamp,
              int touchId )
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( touch );

  input->TouchUp( seat, serial, timestamp, touchId );
}

void TouchMotion( void* data,
                  WlTouch* touch,
                  unsigned int timestamp,
                  int touchId,
                  wl_fixed_t surfaceX,
                  wl_fixed_t surfaceY)
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( touch );

  float x = static_cast<float>( wl_fixed_to_double( surfaceX) );
  float y = static_cast<float>( wl_fixed_to_double( surfaceY) );

  input->TouchMotion( seat, timestamp, touchId, x, y );
}

void TouchFrame( void* data, WlTouch* touch )
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( touch );

  input->TouchFrame( seat );
}

void TouchCancel( void* data, WlTouch* touch )
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( touch );

  input->TouchCancel( seat );
}

/**
 * pointer listener interface ( wl_pointer_listener )
 */
const WlPointerListener PointerListener =
{
   PointerEnter,
   PointerLeave,
   PointerMotion,
   PointerButton,
   PointerAxis,
};

/**
 * Keyboard listener interface ( wl_keyboard_listener )
 */
const WlKeyboardListener KeyboardListener =
{
   KeyboardKeymap,
   KeyFocusEnter,
   KeyFocusLeave,
   KeyEvent,
   KeyModifiers,
   KeyRepeatInfo
};

/**
 * Touch listener interface ( wl_touch_listener )
 */
const WlTouchListener TouchListener =
{
   TouchDown,
   TouchUp,
   TouchMotion,
   TouchFrame,
   TouchCancel
};

/**
 * @brief emitted whenever a seat gains or loses the pointer, keyboard or touch capabilities.
 * @param[in] data user data
 * @param[in] seatInterface seat interface
 * @param[in] caps enum containing the complete set of capabilities this seat has.
 */
void SeatHandleCapabilities( void* data, WlSeat* seatInterface, unsigned int caps)
{

  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( seatInterface );


  // go through either adding or removing pointer/keyboard/touch interfaces
  // most devices are hot plug so this function may be called many times
  // a single keyboard/ pointer / touch interface handles multiple devices
  // e.g. if you plug in 2 mouses, you will still only get a single pointer_interface

  if( caps & WL_SEAT_CAPABILITY_POINTER )
  {
    // at least one pointer available
    seat->SetPointerInterface( Seat::INTERFACE_AVAILABLE );
  }
  else
  {
    // all pointer devices removed, or never connected
    seat->SetPointerInterface( Seat::INTERFACE_NOT_AVAILABLE );
  }

  if( caps & WL_SEAT_CAPABILITY_KEYBOARD )
  {
    // at least one keyboard available
    seat->SetKeyboardInterface( Seat::INTERFACE_AVAILABLE );
  }
  else
  {
    // all keyboard devices removed, or never connected
    seat->SetKeyboardInterface( Seat::INTERFACE_NOT_AVAILABLE );
  }

  if( caps & WL_SEAT_CAPABILITY_TOUCH )
  {
    // new touch device found
    seat->SetTouchInterface( Seat::INTERFACE_AVAILABLE );
  }
  else
  {
    // all touch devices removed, or never connected
    seat->SetTouchInterface( Seat::INTERFACE_NOT_AVAILABLE );
  }

}

void SeatName(void* data, WlSeat* seatInterface, const char* name)
{
  InputInterface* input = static_cast< InputInterface* >( data );
  Seat* seat = input->GetSeat( seatInterface );
  seat->SetName( name );
}

const WlSeatListener SeatListener =
{
  SeatHandleCapabilities, //emitted whenever a seat gains or loses the pointer, keyboard or touch capabilities.
  SeatName, // used to help identify seat in multi-seat configurations
};

} // unnamed namespace


namespace Wayland
{

const WlSeatListener* GetSeatListener()
{
  return &SeatListener;
}

const WlPointerListener* GetPointerListener()
{
  return &PointerListener;
}

const  WlTouchListener* GetTouchListener()
{
  return &TouchListener;
}

const WlKeyboardListener* GetKeyboardListener()
{
  return &KeyboardListener;
}

}
}
}
}
