/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <keyboard.h>

// EXTERNAL INCLUDES
#include <Ecore_Wayland.h>


namespace Dali
{

namespace Keyboard
{


bool SetKeyboardRepeatInfo( double rate, double delay )
{
  return ecore_wl_keyboard_repeat_info_set( rate, delay );
}

bool GetKeyboardRepeatInfo( double& rate, double& delay )
{
  return ecore_wl_keyboard_repeat_info_get( &rate, &delay );
}


} // namespace Keyboard

} // namespace Dali

