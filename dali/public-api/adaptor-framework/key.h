#ifndef DALI_KEY_H
#define DALI_KEY_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/events/key-event.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

/**
 * @brief Enumeration for mapping keyboard and mouse button event keycodes to platform specific codes.
 * @SINCE_1_0.0
 */

enum KEY
{
  DALI_KEY_INVALID          = -1,       ///< Invalid key value @SINCE_1_0.0
  DALI_KEY_ESCAPE           = 9,        ///< Escape key @SINCE_1_0.0
  DALI_KEY_BACKSPACE        = 22,       ///< Backspace key @SINCE_1_0.0
  DALI_KEY_SHIFT_LEFT       = 50,       ///< Shift Left key @SINCE_1_0.0
  DALI_KEY_SHIFT_RIGHT      = 62,       ///< Shift Right key @SINCE_1_0.0
  DALI_KEY_CURSOR_UP        = 111,      ///< Cursor up key @SINCE_1_0.0
  DALI_KEY_CURSOR_LEFT      = 113,      ///< Cursor left key @SINCE_1_0.0
  DALI_KEY_CURSOR_RIGHT     = 114,      ///< Cursor right key @SINCE_1_0.0
  DALI_KEY_CURSOR_DOWN      = 116,      ///< Cursor down key @SINCE_1_0.0
  DALI_KEY_BACK             = 166,      ///< Back key @SINCE_1_0.0
  DALI_KEY_CAMERA           = 167,      ///< Camera key @SINCE_1_0.0
  DALI_KEY_CONFIG           = 168,      ///< Config key @SINCE_1_0.0
  DALI_KEY_POWER            = 169,      ///< Power key @SINCE_1_0.0
  DALI_KEY_PAUSE            = 170,      ///< Pause key @SINCE_1_0.0
  DALI_KEY_CANCEL           = 171,      ///< Cancel key @SINCE_1_0.0
  DALI_KEY_PLAY_CD          = 172,      ///< Play CD key @SINCE_1_0.0
  DALI_KEY_STOP_CD          = 173,      ///< Stop CD key @SINCE_1_0.0
  DALI_KEY_PAUSE_CD         = 174,      ///< Pause CD key @SINCE_1_0.0
  DALI_KEY_NEXT_SONG        = 175,      ///< Next song key @SINCE_1_0.0
  DALI_KEY_PREVIOUS_SONG    = 176,      ///< Previous song key @SINCE_1_0.0
  DALI_KEY_REWIND           = 177,      ///< Rewind key @SINCE_1_0.0
  DALI_KEY_FASTFORWARD      = 178,      ///< Fastforward key @SINCE_1_0.0
  DALI_KEY_MEDIA            = 179,      ///< Media key @SINCE_1_0.0
  DALI_KEY_PLAY_PAUSE       = 180,      ///< Play pause key @SINCE_1_0.0
  DALI_KEY_MUTE             = 181,      ///< Mute key @SINCE_1_0.0
  DALI_KEY_MENU             = 182,      ///< Menu key @SINCE_1_0.0
  DALI_KEY_HOME             = 183,      ///< Home key @SINCE_1_0.0
  DALI_KEY_HOMEPAGE         = 187,      ///< Homepage key @SINCE_1_0.0
  DALI_KEY_WEBPAGE          = 188,      ///< Webpage key @SINCE_1_0.0
  DALI_KEY_MAIL             = 189,      ///< Mail key @SINCE_1_0.0
  DALI_KEY_SCREENSAVER      = 190,      ///< Screensaver key @SINCE_1_0.0
  DALI_KEY_BRIGHTNESS_UP    = 191,      ///< Brightness up key @SINCE_1_0.0
  DALI_KEY_BRIGHTNESS_DOWN  = 192,      ///< Brightness down key @SINCE_1_0.0
  DALI_KEY_SOFT_KBD         = 193,      ///< Soft KBD key @SINCE_1_0.0
  DALI_KEY_QUICK_PANEL      = 194,      ///< Quick panel key @SINCE_1_0.0
  DALI_KEY_TASK_SWITCH      = 195,      ///< Task switch key @SINCE_1_0.0
  DALI_KEY_APPS             = 196,      ///< Apps key @SINCE_1_0.0
  DALI_KEY_SEARCH           = 197,      ///< Search key @SINCE_1_0.0
  DALI_KEY_VOICE            = 198,      ///< Voice key @SINCE_1_0.0
  DALI_KEY_LANGUAGE         = 199,      ///< Language key @SINCE_1_0.0
  DALI_KEY_VOLUME_UP        = 200,      ///< Volume up key @SINCE_1_0.0
  DALI_KEY_VOLUME_DOWN      = 201       ///< Volume down key @SINCE_1_0.0
};

/**
 * @brief Checks if a key event is for a specific DALI KEY.
 *
 * @SINCE_1_0.0
 * @param keyEvent reference to a keyEvent structure
 * @param daliKey Dali key enum
 * @return @c true if the key is matched, @c false if not
 */
DALI_ADAPTOR_API bool IsKey( const Dali::KeyEvent& keyEvent, Dali::KEY daliKey);

/**
 * @}
 */
} // namespace Dali

#endif // DALI_KEY_H
