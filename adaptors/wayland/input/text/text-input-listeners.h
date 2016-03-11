#ifndef __DALI_WAYLAND_TEXT_INPUT_LISTENERS_H__
#define __DALI_WAYLAND_TEXT_INPUT_LISTENERS_H__

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
 * Function for getting Tizen Wayland TextInput listener object
 * The listen object contains C callbacks, which currently expect the user data-field
 * to be a DALi TextInputInterface pointer.
 */

/**
 * @brief Listeners for input panel ( virtual keyboard ) events.
 * @return text input listener
 */
const WlTextInputListener* GetTextInputListener();


} // Wayland
} // Internal
} // Adaptor
} // Dali

#endif  //__DALI_WAYLAND_INPUT_LISTENERS_H__
