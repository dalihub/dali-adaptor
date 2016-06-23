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
#include "seat.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <cctype>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <cstring>

// INTERNAL INCLUDES
#include "input-listeners.h"


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
const unsigned int DEFAULT_KEY_REPEAT_RATE = 25;
const unsigned int DEFAULT_KEY_REPEAT_DELAY = 400;
}

Seat::Seat( InputInterface* inputInterface,  WlSeat* seatInterface )
:mPointer( NULL ),
 mKeyboard( NULL ),
 mTouch( NULL ),
 mWaylandSeat( seatInterface ),
 mTextInput( NULL ),
 mSurface( NULL ),
 mInputInterface( inputInterface ),
 mPointerPosition( 0, 0),
 mDepressedKeyboardModifiers(0),
 mKeyRepeatRate( DEFAULT_KEY_REPEAT_RATE ),
 mKeyRepeatDelay( DEFAULT_KEY_REPEAT_DELAY )
{
}

Seat::~Seat()
{
  DestroyPointerInterface();
  DestroyTouchInterface();
  DestroyKeyboardInterface();
}

void Seat::SetTextInputInterface( WlTextInput* textInput )
{
  mTextInput = textInput;
}

void Seat::SetSurfaceInterface( WlSurface* surface )
{
  mSurface = surface;
}

void Seat::SetPointerInterface( InterfaceStatus status )
{
  if( status == INTERFACE_AVAILABLE )
  {
    if( ! mPointer )
    {
      WlPointer* pointerInterface = wl_seat_get_pointer( mWaylandSeat );

      // store the interface and add an event listener
      wl_pointer_add_listener( pointerInterface, Wayland::GetPointerListener(), mInputInterface );

      mPointer = pointerInterface;
    }
  }
  else
  {
    DestroyPointerInterface();
  }
}

void Seat::SetTouchInterface( InterfaceStatus status )
{
  if( status == INTERFACE_AVAILABLE )
  {
    // check if it's configured already
    if( ! mTouch )
    {
      WlTouch* touchInterface = wl_seat_get_touch( mWaylandSeat );

      wl_touch_add_listener( touchInterface, Wayland::GetTouchListener(), mInputInterface );

      // store the interface and add an event listener
      mTouch = touchInterface;
    }
  }
  else
  {
    DestroyTouchInterface();
  }

}

void Seat::SetKeyboardInterface( InterfaceStatus status )
{
  if( status == INTERFACE_AVAILABLE )
  {
    // check if it's configured already
    if( ! mKeyboard )
    {
      WlKeyboard* keyboardInterface = wl_seat_get_keyboard( mWaylandSeat );

      wl_keyboard_add_listener( keyboardInterface, Wayland::GetKeyboardListener(), mInputInterface );

      // store the interface and add an event listener
      mKeyboard = keyboardInterface;
    }
  }
  else
  {
    DestroyKeyboardInterface();
  }
}

WlPointer* Seat::GetPointerInterface()
{
  return mPointer;
}

WlTouch* Seat::GetTouchInterface()
{
  return mTouch;
}

WlKeyboard* Seat::GetKeyboardInterface()
{
  return mKeyboard;
}

WlSeat* Seat::GetSeatInterface()
{
  return mWaylandSeat;
}

WlTextInput* Seat::GetTextInputInterface()
{
  return mTextInput;
}

WlSurface* Seat::GetSurface()
{
  return mSurface;
}

void Seat::DestroyPointerInterface()
{
  if( mPointer )
  {
    wl_pointer_destroy( mPointer );
    mPointer = NULL;
  }
}

void Seat::DestroyTouchInterface()
{
  if( mTouch )
  {
    wl_touch_destroy( mTouch );
    mTouch = NULL;
  }
}

void Seat::DestroyKeyboardInterface()
{
  if( mKeyboard )
  {
    wl_keyboard_destroy( mKeyboard );
    mKeyboard = NULL;
  }
}

void Seat::SetName( const char* name )
{
  mName = std::string( mName );
}
const std::string& Seat::GetName() const
{
  return mName;
}

const Dali::Vector2& Seat::GetLastPointerPosition() const
{
  return mPointerPosition;
}

void Seat::SetPointerPosition( Dali::Vector2 position)
{
  mPointerPosition = position;
}

void Seat::KeyboardKeymap( unsigned int format, int fd, unsigned int size )
{

  if( !mXkbData.mContext )
  {
    mXkbData.mContext = xkb_context_new( XKB_CONTEXT_NO_FLAGS );
  }

  // current formats defined in wayland-client-protocol.h
  // WL_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP =0, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1=1

  if( format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 )
  {
    DALI_LOG_ERROR("expected WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1");
    close(fd);
    return;
  }

  // memory map the shared region between us and XKB
  char* map =  static_cast<char*> (mmap( NULL, size, PROT_READ, MAP_SHARED, fd, 0));
  if( map == MAP_FAILED)
  {
    DALI_LOG_ERROR("mmap xkb failed");
    close(fd);
    return;
  }

  if( !mXkbData.mContext )
  {
    DALI_LOG_ERROR("xkb_context_new failed");
    close(fd);
    return;
  }
  mXkbData.mKeymap = xkb_map_new_from_string(mXkbData.mContext, map, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);

  munmap(map, size);
  close(fd);

  if (! mXkbData.mKeymap )
  {
    DALI_LOG_ERROR(" xkb_map_new_from_string failed");
    return;
  }

  mXkbData.mState =  xkb_state_new( mXkbData.mKeymap );

  if( ! mXkbData.mState )
  {
    xkb_map_unref(mXkbData.mKeymap );
    mXkbData.mKeymap = NULL;
    return;
  }

  // store the bit which each mod will set when calling xkb_state_serialize_mods
  mXkbData.mControlMask = 1 << xkb_map_mod_get_index( mXkbData.mKeymap, XKB_MOD_NAME_CTRL);
  mXkbData.mAltMask =     1 << xkb_map_mod_get_index( mXkbData.mKeymap, XKB_MOD_NAME_ALT);
  mXkbData.mShiftMask =   1 << xkb_map_mod_get_index( mXkbData.mKeymap, XKB_MOD_NAME_SHIFT);

}

Dali::KeyEvent Seat::GetDALiKeyEventFromSymbol( unsigned int serial,
                                      unsigned int timestamp,
                                      unsigned int symbol,
                                      unsigned int state,
                                      unsigned int modifiers )
{
 char key[256] = { 0 };
 char keyName[256] = { 0 };

 // get its name
 xkb_keysym_get_name( symbol, key, sizeof(key));

 // copy the keyname
 memcpy(keyName, key, sizeof(keyName));

 if (keyName[0] == '\0')
 {
   snprintf(keyName, sizeof(keyName), "Keycode-%u", symbol);
 }

 Dali::KeyEvent keyEvent;

 keyEvent.keyCode = symbol; // we don't get the keycode so just the symbol
 if( state == 1)
 {
   keyEvent.state = KeyEvent::Down;
 }
 else
 {
   keyEvent.state = KeyEvent::Up;
 }

 keyEvent.keyPressed = keyName;
 keyEvent.keyPressedName = keyName;
 keyEvent.time = timestamp;
 keyEvent.keyModifier =  modifiers;

 return keyEvent;

}


Dali::KeyEvent Seat::GetDALiKeyEvent( unsigned int serial, unsigned int timestamp, unsigned int keycode, unsigned int state  )
{
 unsigned int code( 0 );
 unsigned int symbolCount( 0 );
 const xkb_keysym_t* symbols( NULL );
 xkb_keysym_t symbol = XKB_KEY_NoSymbol;
 char key[256] = { 0 };
 char keyName[256] = { 0 };

 // X11 historically has a min keycode of 8 instead of 1, XKB follow this
 code = keycode + 8;

 //get the key symbols
 symbolCount = xkb_key_get_syms( mXkbData.mState, code, &symbols);

 if( symbolCount == 1)
 {
   symbol = symbols[0];
 }

 // get its name
 xkb_keysym_get_name( symbol, key, sizeof(key));

 // copy the keyname
 memcpy(keyName, key, sizeof(keyName));

 if (keyName[0] == '\0')
 {
   snprintf(keyName, sizeof(keyName), "Keycode-%u", code);
 }

 // todo support key repeat settings

 Dali::KeyEvent keyEvent;

 keyEvent.keyCode = code;
 if( state == 1)
 {
   keyEvent.state = KeyEvent::Down;
 }
 else
 {
   keyEvent.state = KeyEvent::Up;
 }
 keyEvent.keyPressed = keyName;
 keyEvent.keyPressedName = keyName;
 keyEvent.time = timestamp;
 keyEvent.keyModifier =  mDepressedKeyboardModifiers;

 return keyEvent;

}
unsigned int Seat::GetDepressedKeyboardModifiers() const
{
  return mDepressedKeyboardModifiers;
}

void Seat::SetDepressedKeyboardModifiers( unsigned int modifiers)
{
  mDepressedKeyboardModifiers = modifiers;
}

void Seat::SetKeyRepeatInfo( unsigned int rate, unsigned int delay )
{
  mKeyRepeatRate = rate;
  mKeyRepeatDelay = delay;
}

} // Internal
} // Adaptor
} // Dali
