#ifndef __DALI_INTERNAL_ADAPTOR_INPUT_INTERFACE_H__
#define __DALI_INTERNAL_ADAPTOR_INPUT_INTERFACE_H__

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
#include <wl-types.h>


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class Seat;

/**
 * @brief interface used by the Wayland input listeners to signal an event.
 *
 * The events that come from keyboard, pointer and touch devices are from a specific seat.
 * A single seat can have multiple keyboards, pointers and touch devices connected to it.
 *
 * For example you can have a car with 4 seats each with their own touch screen that
 * are run by a central device with multi-display output.
 *
 * Some input events have serial numbers and time stamps.
 *
 * Serial numbers description from Wayland documentation:
 * "To avoid race conditions, input events that are likely to trigger further requests
 * (such as button presses, key events, pointer motions) carry serial numbers, and requests such as
 * wl_surface.set_popup require that the serial number of the triggering event is specified."
 *
 *  Time stamps description from Wayland documentation:
 * "Input events also carry timestamps with millisecond granularity.
 *  Their base is undefined, so they can't be compared against system time
 *  (as obtained with clock_gettime or gettimeofday). They can be compared
 *  with each other though, and for instance be used to identify sequences
 *  of button presses as double or triple clicks."
 */
class InputInterface
{

public: // Pointer events. See wl_pointer_listener in wayland-client-protocol.h for more information.

  /**
   * @brief Called when a seat's pointer has entered a surface
   *
   * When focus enters a surface, the pointer image is
   * undefined and a client should respond to this event by setting
   * an appropriate pointer image with the set_cursor request.
   *
   * @param[in] seat the seat that produced the event
   * @param[in] serial  serial number
   * @param[in] surface surface
   * @param[in] x x coordinate in surface-relative coordinates ( not screen )
   * @param[in] y y coordinate in surface-relative coordinates ( not screen )
   */
  virtual void PointerEnter( Seat* seat, unsigned int serial, WlSurface* surface, float x, float y) = 0;

  /**
   * @brief Called when a seat's pointer leaves a surface
   * @param[in] seat the seat that produced the event
   * @param[in] serial serial number
   * @param[in] surface surface
   *
   * The leave notification is sent before the enter notification for
   * the new focus.
   */
  virtual void PointerLeave( Seat* seat, unsigned int serial, WlSurface* surface ) = 0;

  /**
   * @brief pointer motion event
   *
   * @param[in] seat the seat that produced the event
   * @param[in] timestamp timestamp with millisecond granularity
   * @param[in] x x coordinate in surface-relative coordinates ( not screen )
   * @param[in] y y coordinate in surface-relative coordinates ( not screen )
   *
   */
  virtual void PointerMotion( Seat* seat, unsigned int timestamp, float x, float y ) = 0;

  /**
   * @brief pointer click and release events
   *
   * @param[in] seat the seat that produced the event
   * @param[in] serial serial number
   * @param[in] timestamp timestamp with millisecond granularity
   * @param[in] button pointer button pressed
   * @param[in] state button state ( 1 = down, 0 = up)
   *
   * The location of the click is given by the last motion or enter
   * event.
   */
  virtual void PointerButton( Seat* seat, unsigned int serial, unsigned int timestamp, unsigned int button, unsigned int state ) = 0;

  /**
   * @brief Pointer scroll and other axis notifications.
   *
   * @param[in] seat the seat that produced the event
   * @param[in] timestamp timestamp with millisecond granularity
   * @param[in] axis pointer axis
   * @param[in] value length of a vector along the specified axis in a coordinate space identical to those of motion event
  */
  virtual void PointerAxis( Seat* seat, unsigned int timestamp, unsigned int axis, float value ) = 0;

public: // Key  events. See wl_keyboard_listener in wayland-client-protocol.h for more information.

  /**
   * @brief keyboard mapping
   * Provides a file descriptor which can be memory-mapped to a keyboard mapping description
   *
   * @param[in] seat the seat that produced the event
   * @param[in] format see wl_keyboard_keymap_format
   * @param[in] fd  file descriptor
   * @param[in] size size of the memory mapped region in bytes
  */
  virtual void KeyboardKeymap( Seat* seat, unsigned int format, int fd, unsigned int size ) = 0;

  /**
   * @brief The seat's keyboard focus has entered a surface
   *
   * @param[in] seat the seat that produced the event
   * @param[in] serial serial number
   * @param[in] surface surface
   * @param[in] keys the currently pressed keys
   */
  virtual void KeyFocusEnter( Seat* seat, unsigned int serial, WlSurface* surface, WlArray* keys ) = 0;

  /**
   * @brief The seat's keyboard focus has left a surface
   *
   * The leave notification is sent before the enter notification for the new focus.
   * @param[in] seat the seat that produced the event
   * @param[in] serial serial number
   * @param[in] surface surface
   */
  virtual void KeyFocusLeave( Seat* seat, unsigned int serial, WlSurface* surface ) = 0;

  /**
   * @brief Key has been pressed or released
   *
   * @param[in] seat the seat that produced the event
   * @param[in] serial serial number
   * @param[in] timestamp timestamp
   * @param[in] keycode raw hardware key code
   * @param[in] state
   */
  virtual void KeyEvent( Seat* seat, unsigned int serial, unsigned int timestamp, unsigned int keycode, unsigned int state) = 0;

  /**
   * @brief Key modifier state has changed
   *
   * @param[in] seat the seat that produced the event
   * @param[in] serial serial number
   * @param[in] depressed modifiers depressed
   * @param[in] latched modifiers latched
   * @param[in] locked modifiers locked
   * @param[in[ group group
   */
  virtual void KeyModifiers( Seat* seat,
                            unsigned int serial,
                            unsigned int depressed,
                            unsigned int latched,
                            unsigned int locked,
                            unsigned int group) = 0;


  /**
   * @brief Key repeat rate and delay.
   *
   * Informs DALi of the keyboard's repeat rate and delay.
   *
   * Event sent when wl_keyboard object created, before any key events
   * If rate == zero, repeating is disabled
   *
   * @param[in] seat the seat that produced the event
   * @param[in] rate rate of repeating keys in characters per second
   * @param[in] delay delay in milliseconds since key down until repeating starts
   */
  virtual void KeyRepeatInfo( Seat* seat, int32_t rate, int32_t delay) = 0;


public: // touch  events. See wl_touch_listener in wayland-client-protocol.h for more information.

  /**
   * @brief Touch down event on a surface.
   *
   * Future events from this touch point will have the same id which is valid
   * until a touch up event.
   * @param[in] seat the seat that produced the event
   * @param[in] serial serial number
   * @param[in] timestamp timestamp
   * @param[in] surface surface
   * @param[in] touchId unique touch id valid until touch is released
   * @param[in] x coordinate in surface-relative coordinates
   * @param[in[ y coordinate in surface-relative coordinates
   */
  virtual void TouchDown( Seat* seat, unsigned int serial, unsigned int timestamp, WlSurface* surface, int touchId, float x, float y) = 0;

  /**
   * @brief Touch up event on a surface.
   *
   * @param[in] seat the seat that produced the event
   * @param[in] serial serial number
   * @param[in] timestamp timestamp with millisecond granularity
   * @param[in] touchId unique touch id valid until touch is released ( can be re-used for future events)
   */
  virtual void TouchUp( Seat* seat, unsigned int serial, unsigned int timestamp, int touchId ) = 0;

  /**
   * @brief Touch Motion event on a surface.
   * Events relating to the same touch point can be identified by the touchId
   *
   * @param[in] seat the seat that produced the event
   * @param[in] timestamp timestamp with millisecond granularity
   * @param[in] touchId unique touch id valid until touch is released ( then can be re-used)
   * @param[in] x coordinate in surface-relative coordinates
   * @param[in[ y coordinate in surface-relative coordinates
   */
  virtual void TouchMotion( Seat* seat, unsigned int timestamp, int touchId, float x, float y ) = 0;

  /**
   * @brief  end of touch frame event
   * Unsure what this means.
   * @param[in] seat the seat that produced the event
   */
  virtual void TouchFrame( Seat* seat ) = 0;

  /**
   * @brief  Cancel current touch session
   * If Wayland Compositor decides current touch stream is a global
   * gesture we have to cancel any touch / gesture detection.
   *
   * Touch cancellation applies to all touch points currently active on
   * our surface. TouchPoint::Interrupted will be sent to dali-core
   *
   * @param[in] seat the seat that produced the event
   */
  virtual void TouchCancel( Seat* seat ) = 0;

public: // Helper functions used to find the seat associated with the keyboard/pointer/touch device

  /**
   * @brief get the seat that contains the keyboard interface
   * @param[in] keyboard wayland keyboard interface
   * @return the seat the keyboard belongs to
   */
  virtual Seat* GetSeat( const WlKeyboard* keyboard ) = 0;

  /**
   * @brief get the seat that contains the pointer interface
   * @param[in] pointer wayland pointer interface
   * @return the seat the pointer belongs to
   */
  virtual Seat* GetSeat( const WlPointer* pointer ) = 0;

  /**
   * @brief get the seat that contains the touch interface
   * @param[in] touch wayland touch interface
   * @return the seat the touch device belongs to
   */
  virtual Seat* GetSeat( const WlTouch* touch ) = 0;

  /**
   * @brief get the seat that contains the wayland seat interface
   * @param[in] seat wayland seat interface
   * @return the seat the touch device belongs to
   */
  virtual Seat* GetSeat( const WlSeat* seat ) = 0;

protected:

  /**
   * @brief Constructor
   */
  InputInterface()
  {
  }

  /**
   * @brief destructor
   */
  virtual ~InputInterface()
  {
  }

  // Undefined copy constructor.
  InputInterface( const InputInterface& );

  // Undefined assignment operator.
  InputInterface& operator=( const InputInterface& );
};



} // Internal
} // Adaptor
} // Dali

#endif  //__DALI_INTERNAL_ADAPTOR_INPUT_INTERFACE_H__
