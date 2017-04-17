#ifndef __DALI_WAYLAND_INPUT_H__
#define __DALI_WAYLAND_INPUT_H__

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
#include <dali/public-api/common/dali-vector.h>

// INTERNAL INCLUDES
#include <base/interfaces/window-event-interface.h>
#include <wl-types.h>
#include <input/input-interface.h>
#include <input/seat.h>
#include <input/text/text-input-manager.h>



namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * @brief Listens to Wayland input events.
 * Translates the wayland events into DALi events and forwards them to a WindowEventInterface
 */
class InputManager : public InputInterface
{

public:


  /**
   * @brief Constructor
   */
  InputManager();

  /**
   * @brief Destructor
   */
  virtual ~InputManager();

  /**
   * @brief Assign the window event interface
   * @param[in]  eventInterface event interface
   */
  void AssignWindowEventInterface( WindowEventInterface* eventInterface);

  /**
   * @brief Assign the Wayland connection
   * @param[in] display Wayland display
   */
  void AssignDisplay( WlDisplay* display );

  /**
   * @brief Assign the Wayland surface
   * @param[in] surface wayland surface
   */
  void AssignSurface( WlSurface* surface);

  /**
   * @brief listen to events on this seat
   * @param[in] seatInterface output interface
   */
  void AddSeatListener( Dali::WlSeat* seatInterface );

  /**
   * @brief add text input manager interface
   * @param[in] textInputManager text input manager
   */
  void AddTextInputManager( Dali::WlTextInputManager* textInputManager );


protected: //InputInterface, pointer events

  /**
   * @copydoc InputInterface::PointerEnter
   */
  virtual void PointerEnter( Seat* seat, unsigned int serial, WlSurface* surface, float x, float y );

  /**
   * @copydoc InputInterface::PointerLeave
   */
  virtual void PointerLeave( Seat* seat, unsigned int serial, WlSurface* surface );

  /**
   * @copydoc InputInterface::PointerMotion
   */
  virtual void PointerMotion( Seat* seat, unsigned int timestamp, float x, float y );

  /**
   * @copydoc InputInterface::PointerButton
   */
  virtual void PointerButton( Seat* seat, unsigned int serial, unsigned int timestamp, unsigned int button, unsigned int state );

  /**
   * @copydoc InputInterface::PointerAxis
   */
  virtual void PointerAxis( Seat* seat, unsigned int timestamp, unsigned int axis, float value );


protected: //InputInterface, keyboard events

  /**
   * @copydoc InputInterface::KeyboardKeymap
   */
  virtual void KeyboardKeymap( Seat* seat, unsigned int format, int fd, unsigned int size );

  /**
   * @copydoc InputInterface::KeyFocusEnter
   */
  virtual void KeyFocusEnter( Seat* seat, unsigned int serial, WlSurface* surface, WlArray* keys );

  /**
   * @copydoc InputInterface::KeyFocusLeave
   */
  virtual void KeyFocusLeave( Seat* seat, unsigned int serial, WlSurface* surface );

  /**
   * @copydoc InputInterface::KeyEvent
   */
  virtual void KeyEvent( Seat* seat, unsigned int serial, unsigned int timestamp, unsigned int keycode, unsigned int state );

  /**
   * @copydoc InputInterface::KeyModifiers
   */
  virtual void KeyModifiers( Seat* seat,
                            unsigned int serial,
                            unsigned int depressed,
                            unsigned int latched,
                            unsigned int locked,
                            unsigned int group );

  /**
   * @copydoc InputInterface::KeyRepeatInfo
   */
  virtual void KeyRepeatInfo( Seat* seat, int32_t rate, int32_t delay);

protected: //InputInterface, touch events

  /**
   * @copydoc InputInterface::TouchDown
   */
  virtual void TouchDown( Seat* seat, unsigned int serial, unsigned int timestamp, WlSurface* surface, int touchId, float x, float y);

  /**
   * @copydoc InputInterface::TouchUp
   */
  virtual void TouchUp( Seat* seat, unsigned int serial, unsigned int timestamp, int touchId );

  /**
   * @copydoc InputInterface::TouchMotion
   */
  virtual void TouchMotion( Seat* seat, unsigned int timestamp, int touchId, float x, float y );

  /**
   * @copydoc InputInterface::TouchFrame
   */
  virtual void TouchFrame( Seat* seat );

  /**
   * @copydoc InputInterface::TouchCancel
   */
  virtual void TouchCancel( Seat* seat );


protected: //InputInterface  Helper functions

  /**
   * @copydoc InputInterface::GetSeat( const WlKeyboard* keyboard )
   */
  virtual Seat* GetSeat( const WlKeyboard* keyboard );

  /**
   * @copydoc InputInterface::GetSeat( const WlPointer* pointer )
   */
  virtual Seat* GetSeat( const WlPointer* pointer );

  /**
   * @copydoc InputInterface::GetSeat( const WlTouch* touch )
   */
  virtual Seat* GetSeat( const WlTouch* touch );

  /**
   * @copydoc InputInterface::GetSeat( const WlSeat* seat )
   */
  virtual Seat* GetSeat( const WlSeat* seat );

public:

  /**
   * @brief Add a new seat ( collection of input devices)
   * @param[in] wlRegistry
   * @param[in] seatName seat name / id
   * @param[in] version interface version
   */
  void AddSeat( Seat* seat );


private:

 Dali::Vector< Seat* > mSeats;
 TextInputManager mTextInputManger;
 WlDisplay* mDisplay;
 WindowEventInterface* mWindowEventInterface;

};

} // Internal
} // Adaptor
} // Dali

#endif  //__DALI_WAYLAND_INPUT_H__
