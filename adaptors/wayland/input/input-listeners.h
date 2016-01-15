#ifndef __DALI_WAYLAND_INPUT_LISTENERS_H__
#define __DALI_WAYLAND_INPUT_LISTENERS_H__

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

namespace Wayland
{
/**
 * Functions for getting wayland listener objects as defined in wayland-client-protocol.h
 * The listen objects contain C callbacks, which currently expect the user data-field
 * to be a DALi InputInterface pointer.
 *
 * E.g. if setting the keyboard listener
 *
 * wl_keyboard_add_listener( keyboard, &Wayland::GetKeyboardListener(), &InputInterface );
 *
 */


/**
 * @brief Listens for keyboard / touch / pointer interfaces being enabled /disabled on a seat.
 * @return seat listener
 */
const WlSeatListener* GetSeatListener();

/**
 * @brief returns a pointer to the callbacks used to listen to pointer events
 * @return pointer listener
 */
const WlPointerListener* GetPointerListener();

/**
 * @brief returns a pointer to the callbacks used to listen to touch events
 * @return touch listener
 */
const  WlTouchListener* GetTouchListener();

/**
 * @brief returns a pointer to the callbacks used to listen to keyboard events
 * @return keyboard listener
 */
const WlKeyboardListener* GetKeyboardListener();

} // Wayland
} // Internal
} // Adaptor
} // Dali

#endif  //__DALI_WAYLAND_INPUT_LISTENERS_H__
