#ifndef __DALI_WAYLAND_SEAT_H__
#define __DALI_WAYLAND_SEAT_H__

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

// EXTERNAL INCLUDES
#include <string>
#include <xkbcommon/xkbcommon.h>
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/events/key-event.h>

// INTERNAL INCLUDES
#include <wl-types.h>


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class InputInterface;

/**
 *
 * A single seat is a group of keyboards, pointers and touch devices.
 *
 * For example you can have a car with 4 seats each with their own touch screen that
 * is run by a central computer with a multi-display output.
 *
 * Or you may have two people / two seats in an office sharing the same screen at the same time.
 *
 *
 * There is a single wl_pointer / wl_touch and wl_keyboard interface per seat.
 * But each interface can have multiple devices. E.g. wl_pointer interface may have 2
 * mouse pointers attached.
 *
 * Input devices can be hot plugged. However wl_pointer / wl_touch and wl_keyboard interface
 * will only be destroyed when the interface has no devices associated with it.
 * E.g. if a seat has a single mouse, when you unplug the mouse the interface is deleted.
 * If a seat has two mice, only when you unplug both mice is the interface deleted.
 *
 */
class Seat
{

public:

  /**
   * @brief Seat interface status for a device class ( keyboard, touch, pointer )
   */
  enum InterfaceStatus
  {
    INTERFACE_AVAILABLE,      ///< Occurs when at least 1 device for this device class is plugged in
    INTERFACE_NOT_AVAILABLE,  ///< Occurs when all devices for a device class are unplugged (e.g. all mice )
  };

  /**
   * @brief constructor
   * @param[in] inputInterface input interface
   * @param[in] seatInterface Wayland seat interface
   */
  Seat( InputInterface* inputInterface, WlSeat* seatInterface );

  /**
   * @brief non virtual destructor, not intended as base class
   */
  ~Seat();

  /**
   * @brief set Tizen Wayland Text Input interface
   * @param[in] textInputManager interface
   */
  void SetTextInputInterface( WlTextInput* textInput );

  /**
   * @brief set the surface
   * @param[in] surface Wayland surface currently associated with this seat ( for input panel / IMF )
   */
  void SetSurfaceInterface( WlSurface* surface );

  /**
   * @brief Set the pointer interface
   * @param[in] pointer Wayland pointer interface
   * @param[in] status of the interface
   */
  void SetPointerInterface( InterfaceStatus status );

  /**
   * @brief Set the touch interface
   * @param[in] pointer Wayland pointer interface
   * @param[in] status of the interface
   */
  void SetTouchInterface( InterfaceStatus status );

  /**
   * @brief Set the keyboard interface
   * @param[in] pointer Wayland pointer interface
   * @param[in] status of the interface
   */
  void SetKeyboardInterface( InterfaceStatus status );

  /**
   * @brief Get the pointer interface
   * @return Wayland pointer interface
   */
  WlPointer* GetPointerInterface();

  /**
   * @brief Get the touch interface
   * @return Wayland touch interface
   */
  WlTouch* GetTouchInterface();

  /**
   * @brief Get the keyboard interface
   * @return Wayland keyboard interface
   */
  WlKeyboard* GetKeyboardInterface();

  /**
   * @brief Get the keyboard interface
   * @return Wayland keyboard interface
   */
  WlSeat* GetSeatInterface();

  /**
   * @brief Get the text input interface
   * @return Wayland text input interface
   */
  WlTextInput* GetTextInputInterface();

  /**
   * @brief Get the surface
   * @return Wayland surface
   */
  WlSurface* GetSurface();

  /**
   * @brief calls wl_pointer_destroy on the pointer interface
   */
  void DestroyPointerInterface();

  /**
   * @brief calls wl_touch_destroy on the touch interface
   */
  void DestroyTouchInterface();

  /**
   * @brief calls wl_touch_keyboard on the keyboard interface
   */
  void DestroyKeyboardInterface();

  /**
   * @brief place holder store the seat name
   * Currently we don't store the name as DALi core isn't seat name aware
   * Need to think about adding Seat name to touch / key events.
   * @param[in] name seat name
   */
  void SetName( const char* name );

  /**
   * @brief get the seat name
   * E.g. may return "front-passenger-seat"
   * @return seat name
   */
  const std::string& GetName() const;

  /**
   * @brief get the last known pointer position
   * @return pointer position
   */
  const Dali::Vector2& GetLastPointerPosition() const;

  /**
   * @brief set the pointer position
   * So we need to cache mouse x/y position, for pointer down events which
   * don't have x,y position attached
   * @param[in] position pointer position
   */
  void SetPointerPosition( Dali::Vector2 position);

  /**
   * @brief keyboard mapping
   * Provides a file descriptor which can be memory-mapped to a keyboard mapping description
   * @param[in] seat the seat that produced the event
   * @param[in] format see wl_keyboard_keymap_format
   * @param[in] fd  file descriptor
   * @param[in] size size of the memory mapped region in bytes
  */
  void KeyboardKeymap( unsigned int format, int fd, unsigned int size );

  /**
   * @brief get current depressed keyboard modifiers (not latched)
   * @return keyboard modifiers
   */
  unsigned int GetDepressedKeyboardModifiers() const ;

  /**
   * @brief get current depressed keyboard modifiers (not latched)
   * @param[in] modifiers depressed keyboard modifiers
   */
  void SetDepressedKeyboardModifiers( unsigned int modifiers);

  /**
   * @brief set key repeat rate and delay
   * @param[in] rate repeat rate in milliseconds
   * @param[in] delay delay in milliseconds
   */
  void SetKeyRepeatInfo( unsigned int rate, unsigned int delay );


  /**
   * @brief Key has been pressed or released.
   * Used for key events from Tizen Wayland wl_text_input interface.
   *
   * @param[in] seat the seat that produced the event
   * @param[in] timestamp timestamp
   * @param[in] symbol key symbol
   * @param[in] state key state
   * @param[in] modifiers keyboard modifiers
   * @return Key event
   */

  Dali::KeyEvent GetDALiKeyEventFromSymbol( unsigned int serial,
                                        unsigned int timestamp,
                                        unsigned int symbol,
                                        unsigned int state,
                                        unsigned int modifiers );

  /**
   * @brief Key has been pressed or released. Used
   *
   * @param[in] seat the seat that produced the event
   * @param[in] serial serial number
   * @param[in] timestamp timestamp
   * @param[in] keycode raw hardware key code
   * @param[in] state
   * @return Key event
   */
  KeyEvent GetDALiKeyEvent( unsigned int serial, unsigned int timestamp, unsigned int keycode, unsigned int state  );


private:  // data specific to a single seat

  /**
   * Keyboard data
   */
  struct XkbData
  {
    XkbData()
    :mContext( NULL ),
     mKeymap( NULL ),
     mState( NULL ),
     mControlMask( 0 ),
     mAltMask( 0 ),
     mShiftMask( 0 ),
     mControlDown( 0 ),
     mShiftDown( 0 ),
     mAltDown( 0 )
    {

    }
    struct xkb_context *mContext;
    struct xkb_keymap* mKeymap;
    struct xkb_state* mState;
    xkb_mod_mask_t mControlMask;
    xkb_mod_mask_t mAltMask;
    xkb_mod_mask_t mShiftMask;
    bool mControlDown:1;
    bool mShiftDown:1;
    bool mAltDown:1;
  };

  XkbData mXkbData;          ///< Keyboard data. Believe this can vary per seat
  std::string mName;         ///< Seat name
  WlPointer* mPointer;       ///< Wayland Pointer interface ( for multiple pointers )
  WlKeyboard* mKeyboard;     ///< Wayland Keyboard interface ( for multiple keyboards )
  WlTouch* mTouch;           ///< Wayland Touch interface ( for multiple touch devices )
  WlSeat* mWaylandSeat;      ///< Wayland Seat interface
  WlTextInput* mTextInput;   ///< Wayland Tizen Text input interface (Virtual Keyboard / IMF)
  WlSurface* mSurface;       ///< Surface currently used by this seat
  InputInterface* mInputInterface;  ///< DALi Wayland Input interface
  Vector2 mPointerPosition; ///< Current pointer X,Y position
  unsigned int mDepressedKeyboardModifiers; ///< keyboard modifiers
  unsigned int mKeyRepeatRate;
  unsigned int mKeyRepeatDelay;


};

} // Internal
} // Adaptor
} // Dali

#endif  //__DALI_WAYLAND_SEAT_H__
