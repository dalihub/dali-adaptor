/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <system_settings.h>
#include <Elementary.h>

// INTERNAL INCLUDES
#include <system-settings.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

int GetLongPressTime( int defaultTime )
{
  int delay( 0 );

  // read system setting
  if( SYSTEM_SETTINGS_ERROR_NONE != system_settings_get_value_int(SYSTEM_SETTINGS_KEY_TAP_AND_HOLD_DELAY, &delay ) )
  {
    // on error, return default
    delay = defaultTime;
  }

  return delay;
}

int GetElmAccessActionOver()
{
  return ELM_ACCESS_ACTION_OVER;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
