#ifndef DALI_KEY_DEVEL_H
#define DALI_KEY_DEVEL_H

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/key.h>

namespace Dali
{

namespace DevelKey
{

enum Key
{
  DALI_KEY_INVALID = Dali::DALI_KEY_INVALID,
  DALI_KEY_ESCAPE = Dali::DALI_KEY_ESCAPE,
  DALI_KEY_BACKSPACE = Dali::DALI_KEY_BACKSPACE,
  DALI_KEY_SHIFT_LEFT = Dali::DALI_KEY_SHIFT_LEFT,
  DALI_KEY_SHIFT_RIGHT = Dali::DALI_KEY_SHIFT_RIGHT,
  DALI_KEY_CURSOR_UP = Dali::DALI_KEY_CURSOR_UP,
  DALI_KEY_CURSOR_LEFT = Dali::DALI_KEY_CURSOR_LEFT,
  DALI_KEY_CURSOR_RIGHT = Dali::DALI_KEY_CURSOR_RIGHT,
  DALI_KEY_CURSOR_DOWN = Dali::DALI_KEY_CURSOR_DOWN,
  DALI_KEY_BACK = Dali::DALI_KEY_BACK,
  DALI_KEY_CAMERA = Dali::DALI_KEY_CAMERA,
  DALI_KEY_CONFIG = Dali::DALI_KEY_CONFIG,
  DALI_KEY_POWER = Dali::DALI_KEY_POWER,
  DALI_KEY_PAUSE = Dali::DALI_KEY_PAUSE,
  DALI_KEY_CANCEL = Dali::DALI_KEY_CANCEL,
  DALI_KEY_PLAY_CD = Dali::DALI_KEY_PLAY_CD,
  DALI_KEY_STOP_CD = Dali::DALI_KEY_STOP_CD,
  DALI_KEY_PAUSE_CD = Dali::DALI_KEY_PAUSE_CD,
  DALI_KEY_NEXT_SONG = Dali::DALI_KEY_NEXT_SONG,
  DALI_KEY_PREVIOUS_SONG = Dali::DALI_KEY_PREVIOUS_SONG,
  DALI_KEY_REWIND = Dali::DALI_KEY_REWIND,
  DALI_KEY_FASTFORWARD = Dali::DALI_KEY_FASTFORWARD,
  DALI_KEY_MEDIA = Dali::DALI_KEY_MEDIA,
  DALI_KEY_PLAY_PAUSE = Dali::DALI_KEY_PLAY_PAUSE,
  DALI_KEY_MUTE = Dali::DALI_KEY_MUTE,
  DALI_KEY_MENU = Dali::DALI_KEY_MENU,
  DALI_KEY_HOME = Dali::DALI_KEY_HOME,
  DALI_KEY_HOMEPAGE = Dali::DALI_KEY_HOMEPAGE,
  DALI_KEY_WEBPAGE = Dali::DALI_KEY_WEBPAGE,
  DALI_KEY_MAIL = Dali::DALI_KEY_MAIL,
  DALI_KEY_SCREENSAVER = Dali::DALI_KEY_SCREENSAVER,
  DALI_KEY_BRIGHTNESS_UP = Dali::DALI_KEY_BRIGHTNESS_UP,
  DALI_KEY_BRIGHTNESS_DOWN = Dali::DALI_KEY_BRIGHTNESS_DOWN,
  DALI_KEY_SOFT_KBD = Dali::DALI_KEY_SOFT_KBD,
  DALI_KEY_QUICK_PANEL = Dali::DALI_KEY_QUICK_PANEL,
  DALI_KEY_TASK_SWITCH = Dali::DALI_KEY_TASK_SWITCH,
  DALI_KEY_APPS = Dali::DALI_KEY_APPS,
  DALI_KEY_SEARCH = Dali::DALI_KEY_SEARCH,
  DALI_KEY_VOICE = Dali::DALI_KEY_VOICE,
  DALI_KEY_LANGUAGE = Dali::DALI_KEY_LANGUAGE,
  DALI_KEY_VOLUME_UP = Dali::DALI_KEY_VOLUME_UP,
  DALI_KEY_VOLUME_DOWN = Dali::DALI_KEY_VOLUME_DOWN,

  /**
   * @brief Delete key.
   */
  DALI_KEY_DELETE = 119,

  /**
   * @brief Control Left key.
   */
  DALI_KEY_CONTROL_LEFT = 37,

  /**
   * @brief Control Right key.
   */
  DALI_KEY_CONTROL_RIGHT = 105,

  /**
   * @brief Control Return key.
   */
  DALI_KEY_RETURN = 36

};

/**
 * @brief Get the key code from a key name.
 * @param[in] keyName The key name
 * @return The key code. -1 if the daliKey does not exist in the supported key lookup table.
 */
DALI_ADAPTOR_API int GetDaliKeyCode( const char* keyName );

} // namespace DevelKey

} // namespace Dali

#endif // DALI_KEY_DEVEL_H
