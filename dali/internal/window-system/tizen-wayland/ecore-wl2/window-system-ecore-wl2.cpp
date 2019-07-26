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

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-system.h>
#include <dali/devel-api/adaptor-framework/keyboard.h>

// EXTERNAL_HEADERS
#include <Ecore_Wl2.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace WindowSystem
{

void Initialize()
{
  ecore_wl2_init();
}

void Shutdown()
{
  ecore_wl2_shutdown();
}

void GetScreenSize( int& width, int& height )
{
  Ecore_Wl2_Display* display = ecore_wl2_display_connect( NULL );
  if( display )
  {
    ecore_wl2_display_screen_size_get( display, &width, &height );
    DALI_ASSERT_ALWAYS((width>0) && "screen width is 0");
    DALI_ASSERT_ALWAYS((height>0) && "screen height is 0");
  }
}

bool SetKeyboardRepeatInfo( float rate, float delay )
{
  Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get( ecore_wl2_connected_display_get( NULL ) );
  return ecore_wl2_input_keyboard_repeat_set( input, static_cast<double>( rate ), static_cast<double>( delay ) );
}

bool GetKeyboardRepeatInfo( float& rate, float& delay )
{
  Ecore_Wl2_Input* input = ecore_wl2_input_default_input_get( ecore_wl2_connected_display_get( NULL ) );
  double rateVal, delayVal;
  bool ret = ecore_wl2_input_keyboard_repeat_get( input, &rateVal, &delayVal );
  rate = static_cast<float>( rateVal );
  delay = static_cast<float>( delayVal );

  return ret;
}

} // namespace WindowSystem

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
