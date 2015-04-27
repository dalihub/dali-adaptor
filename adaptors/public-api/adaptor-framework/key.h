#ifndef __DALI_KEY_H__
#define __DALI_KEY_H__

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

#include <dali/public-api/events/key-event.h>

namespace Dali DALI_IMPORT_API
{

/**
 * @brief Mapping of keyboard and mouse button event keycodes to platform specific codes.
 *
 * For tizen the X Server Keycode is used as reference, unless it's over ridden
 * in utilX.h in which case the values are based on utilX.h
 */

typedef int KEY;

extern const KEY DALI_KEY_INVALID;
extern const KEY DALI_KEY_ESCAPE;
extern const KEY DALI_KEY_BACK;
extern const KEY DALI_KEY_CAMERA;
extern const KEY DALI_KEY_CONFIG;
extern const KEY DALI_KEY_POWER;
extern const KEY DALI_KEY_PAUSE;
extern const KEY DALI_KEY_CANCEL;
extern const KEY DALI_KEY_PLAY_CD;
extern const KEY DALI_KEY_STOP_CD;
extern const KEY DALI_KEY_PAUSE_CD;
extern const KEY DALI_KEY_NEXT_SONG;
extern const KEY DALI_KEY_PREVIOUS_SONG;
extern const KEY DALI_KEY_REWIND;
extern const KEY DALI_KEY_FASTFORWARD;
extern const KEY DALI_KEY_MEDIA;
extern const KEY DALI_KEY_PLAY_PAUSE;
extern const KEY DALI_KEY_MUTE;
extern const KEY DALI_KEY_SEND;
extern const KEY DALI_KEY_SELECT;
extern const KEY DALI_KEY_END;
extern const KEY DALI_KEY_MENU;
extern const KEY DALI_KEY_HOME;
extern const KEY DALI_KEY_HOMEPAGE;
extern const KEY DALI_KEY_WEBPAGE;
extern const KEY DALI_KEY_MAIL;
extern const KEY DALI_KEY_SCREENSAVER;
extern const KEY DALI_KEY_BRIGHTNESS_UP;
extern const KEY DALI_KEY_BRIGHTNESS_DOWN;
extern const KEY DALI_KEY_SOFT_KBD;
extern const KEY DALI_KEY_QUICK_PANEL;
extern const KEY DALI_KEY_TASK_SWITCH;
extern const KEY DALI_KEY_APPS;
extern const KEY DALI_KEY_SEARCH;
extern const KEY DALI_KEY_VOICE;
extern const KEY DALI_KEY_LANGUAGE;
extern const KEY DALI_KEY_VOLUME_UP;
extern const KEY DALI_KEY_VOLUME_DOWN;
extern const KEY DALI_KEY_BACKSPACE;
extern const KEY DALI_KEY_CURSOR_LEFT;
extern const KEY DALI_KEY_CURSOR_RIGHT;
extern const KEY DALI_KEY_CURSOR_UP;
extern const KEY DALI_KEY_CURSOR_DOWN;

/**
 * @brief Check if a key event is for a specific DALI KEY.
 *
 * @param keyEvent reference to a keyEvent structure
 * @param daliKey dali key enum
 * @return true if the key is matched, false if not
 */
bool IsKey( const KeyEvent& keyEvent, KEY daliKey);

} // namespace Dali

#endif // __DALI_KEY_H__
