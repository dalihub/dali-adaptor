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

// CLASS HEADER
#include <key-impl.h>

// EXTERNAL INCLUDES
#include <utilX.h>
#include <map>
#include <string.h>
#include <iostream>

#include <dali/integration-api/debug.h>


namespace Dali
{

const KEY DALI_KEY_INVALID          = -1;
const KEY DALI_KEY_ESCAPE           = 9;
const KEY DALI_KEY_BACKSPACE        = 22;
const KEY DALI_KEY_CURSOR_UP        = 111;
const KEY DALI_KEY_CURSOR_LEFT      = 113;
const KEY DALI_KEY_CURSOR_RIGHT     = 114;
const KEY DALI_KEY_CURSOR_DOWN      = 116;
const KEY DALI_KEY_BACK             = 166;
const KEY DALI_KEY_CAMERA           = 167;
const KEY DALI_KEY_CONFIG           = 168;
const KEY DALI_KEY_POWER            = 169;
const KEY DALI_KEY_PAUSE            = 170;
const KEY DALI_KEY_CANCEL           = 171;
const KEY DALI_KEY_PLAY_CD          = 172;
const KEY DALI_KEY_STOP_CD          = 173;
const KEY DALI_KEY_PAUSE_CD         = 174;
const KEY DALI_KEY_NEXT_SONG        = 175;
const KEY DALI_KEY_PREVIOUS_SONG    = 176;
const KEY DALI_KEY_REWIND           = 177;
const KEY DALI_KEY_FASTFORWARD      = 178;
const KEY DALI_KEY_MEDIA            = 179;
const KEY DALI_KEY_PLAY_PAUSE       = 180;
const KEY DALI_KEY_MUTE             = 181;
const KEY DALI_KEY_SEND             = 182;
const KEY DALI_KEY_SELECT           = 183;
const KEY DALI_KEY_END              = DALI_KEY_BACK;
const KEY DALI_KEY_MENU             = DALI_KEY_SEND;
const KEY DALI_KEY_HOME             = DALI_KEY_SELECT;
const KEY DALI_KEY_HOMEPAGE         = 187;
const KEY DALI_KEY_WEBPAGE          = 188;
const KEY DALI_KEY_MAIL             = 189;
const KEY DALI_KEY_SCREENSAVER      = 190;
const KEY DALI_KEY_BRIGHTNESS_UP    = 191;
const KEY DALI_KEY_BRIGHTNESS_DOWN  = 192;
const KEY DALI_KEY_SOFT_KBD         = 193;
const KEY DALI_KEY_QUICK_PANEL      = 194;
const KEY DALI_KEY_TASK_SWITCH      = 195;
const KEY DALI_KEY_APPS             = 196;
const KEY DALI_KEY_SEARCH           = 197;
const KEY DALI_KEY_VOICE            = 198;
const KEY DALI_KEY_LANGUAGE         = 199;
const KEY DALI_KEY_VOLUME_UP        = 200;
const KEY DALI_KEY_VOLUME_DOWN      = 201;

namespace Internal
{

namespace Adaptor
{

namespace KeyLookup
{

namespace
{

struct KeyLookup
{
  const char* keyName;      ///< X string representation
  const int   daliKeyCode;  ///< Dali Enum Representation
  const bool  deviceButton; ///< Whether the key is from a button on the device
};

// matches a DALI_KEY enum, to a X key name
KeyLookup KeyLookupTable[]=
{
  // more than one key name can be assigned to a single dali-key code
  // e.g. Menu and KEY_MENU("FS86KeyMenu") are both assigned to  DALI_KEY_MENU

  { "Escape",               DALI_KEY_ESCAPE,          false },  // item not defined in utilX
  { "Menu",                 DALI_KEY_MENU,            false },  // item not defined in utilX
//  { KEY_CAMERA,             DALI_KEY_CAMERA,          false },
//  { KEY_CONFIG,             DALI_KEY_CONFIG,          false },
  { KEY_POWER,              DALI_KEY_POWER,           true  },
  { KEY_PAUSE,              DALI_KEY_PAUSE,           false },
  { KEY_CANCEL,             DALI_KEY_CANCEL,          false },
//  { KEY_PLAYCD,             DALI_KEY_PLAY_CD,         false },
//  { KEY_STOPCD,             DALI_KEY_STOP_CD,         false },
//  { KEY_PAUSECD,            DALI_KEY_PAUSE_CD,        false },
//  { KEY_NEXTSONG,           DALI_KEY_NEXT_SONG,       false },
//  { KEY_PREVIOUSSONG,       DALI_KEY_PREVIOUS_SONG,   false },
//  { KEY_REWIND,             DALI_KEY_REWIND,          false },
//  { KEY_FASTFORWARD,        DALI_KEY_FASTFORWARD,     false },
//  { KEY_MEDIA,              DALI_KEY_MEDIA,           false },
//  { KEY_PLAYPAUSE,          DALI_KEY_PLAY_PAUSE,      false },
  { KEY_MUTE,               DALI_KEY_MUTE,            false },
//  { KEY_SEND,               DALI_KEY_SEND,            true  },
//  { KEY_SELECT,             DALI_KEY_SELECT,          true  },
//  { KEY_END,                DALI_KEY_END,             true  },
  { KEY_MENU,               DALI_KEY_MENU,            true  },
  { KEY_HOME,               DALI_KEY_HOME,            true  },
  { KEY_BACK,               DALI_KEY_BACK,            true  },
//  { KEY_HOMEPAGE,           DALI_KEY_HOMEPAGE,        false },
//  { KEY_WEBPAGE,            DALI_KEY_WEBPAGE,         false },
//  { KEY_MAIL,               DALI_KEY_MAIL,            false },
//  { KEY_SCREENSAVER,        DALI_KEY_SCREENSAVER,     false },
//  { KEY_BRIGHTNESSUP,       DALI_KEY_BRIGHTNESS_UP,   false },
//  { KEY_BRIGHTNESSDOWN,     DALI_KEY_BRIGHTNESS_DOWN, false },
//  { KEY_SOFTKBD,            DALI_KEY_SOFT_KBD,        false },
//  { KEY_QUICKPANEL,         DALI_KEY_QUICK_PANEL,     false },
//  { KEY_TASKSWITCH,         DALI_KEY_TASK_SWITCH,     false },
//  { KEY_APPS,               DALI_KEY_APPS,            false },
  { KEY_SEARCH,             DALI_KEY_SEARCH,          false },
//  { KEY_VOICE,              DALI_KEY_VOICE,           false },
//  { KEY_LANGUAGE,           DALI_KEY_LANGUAGE,        false },
  { KEY_VOLUMEUP,           DALI_KEY_VOLUME_UP,       true  },
  { KEY_VOLUMEDOWN,         DALI_KEY_VOLUME_DOWN,     true  },
};

const std::size_t KEY_LOOKUP_COUNT = (sizeof( KeyLookupTable))/ (sizeof(KeyLookup));

class KeyMap
{
  public:

  KeyMap():
  mLookup( cmpString )
  {
    // create the lookup
    for( size_t i = 0; i < KEY_LOOKUP_COUNT ; ++i )
    {
      const KeyLookup&  keyLookup( KeyLookupTable[i] );
      mLookup[ keyLookup.keyName  ] = DaliKeyType( keyLookup.daliKeyCode, keyLookup.deviceButton );
    }
  }

  int GetDaliKeyEnum( const char* keyName ) const
  {
    Lookup::const_iterator i = mLookup.find( keyName );
    if( i == mLookup.end() )
    {
      return -1;
    }
    else
    {
      return (*i).second.first;
    }
  }

  bool IsDeviceButton( const char* keyName ) const
  {
    Lookup::const_iterator i = mLookup.find( keyName );
    if ( i != mLookup.end() )
    {
      return (*i).second.second;
    }
    return false;
  }

  private:

  /**
   * compare function, to compare string by pointer
   */
  static bool cmpString( const char* a, const char* b)
  {
    return strcmp(a, b) < 0;
  }

  typedef std::pair< int, bool > DaliKeyType;
  typedef std::map<const char* /* key name */, DaliKeyType /* key code */, bool(*) ( char const* a, char const* b) > Lookup;
  Lookup mLookup;

};
const KeyMap globalKeyLookup;

} // un-named name space

bool IsKey( const Dali::KeyEvent& keyEvent, Dali::KEY daliKey)
{
  int key = globalKeyLookup.GetDaliKeyEnum( keyEvent.keyPressedName.c_str() );
  return daliKey == key;
}

bool IsDeviceButton( const char* keyName )
{
  return globalKeyLookup.IsDeviceButton( keyName );
}

} // namespace KeyLookup

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
