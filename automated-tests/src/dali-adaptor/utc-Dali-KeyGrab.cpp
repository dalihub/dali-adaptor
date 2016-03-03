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
#include <map>
#include <string.h>
#include <iostream>

// CLASS HEADER
#include <stdlib.h>
#include <iostream>
#include <dali.h>
#include <dali-test-suite-utils.h>
#include <dali/public-api/adaptor-framework/key-grab.h>

extern int gArgc;
extern char ** gArgv;

using namespace Dali;

void utc_dali_adaptor_keygrab_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_adaptor_keygrab_cleanup(void)
{
  test_return_value = TET_PASS;
}

// Copied from key-impl.cpp
struct KeyLookup
{
  const char* keyName;      ///< X string representation
  const KEY   daliKeyCode;  ///< Dali Enum Representation
  const bool  deviceButton; ///< Whether the key is from a button on the device
};

// Taken from key-impl.cpp
KeyLookup TestKeyLookupTable[]=
{
  { "Escape",                DALI_KEY_ESCAPE,          false },  // item not defined in utilX
  { "Menu",                  DALI_KEY_MENU,            false },  // item not defined in utilX

  // Now the key names are used as literal string not defined symbols,
  // since these definition in utilX.h is deprecated and we're guided not to use them
  { "XF86Camera",            DALI_KEY_CAMERA,          false },
  { "XF86Camera_Full",       DALI_KEY_CONFIG,          false },
  { "XF86PowerOff",          DALI_KEY_POWER,           true  },
  { "Cancel",                DALI_KEY_CANCEL,          false },
  { "XF86AudioPlay",         DALI_KEY_PLAY_CD,         false },
  { "XF86AudioStop",         DALI_KEY_STOP_CD,         false },
  { "XF86AudioPause",        DALI_KEY_PAUSE_CD,        false },
  { "XF86AudioNext",         DALI_KEY_NEXT_SONG,       false },
  { "XF86AudioPrev",         DALI_KEY_PREVIOUS_SONG,   false },
  { "XF86AudioRewind",       DALI_KEY_REWIND,          false },
  { "XF86AudioForward",      DALI_KEY_FASTFORWARD,     false },
  { "XF86AudioMedia",        DALI_KEY_MEDIA,           false },
  { "XF86AudioPlayPause",    DALI_KEY_PLAY_PAUSE,      false },
  { "XF86AudioMute",         DALI_KEY_MUTE,            false },
  { "XF86Menu",              DALI_KEY_MENU,            true  },
  { "XF86Send",              DALI_KEY_MENU,            true  },
  { "XF86HomePage",          DALI_KEY_HOMEPAGE,        false },
  { "XF86WWW",               DALI_KEY_WEBPAGE,         false },
  { "XF86Mail",              DALI_KEY_MAIL,            false },
  { "XF86ScreenSaver",       DALI_KEY_SCREENSAVER,     false },
  { "XF86MonBrightnessUp",   DALI_KEY_BRIGHTNESS_UP,   false },
  { "XF86MonBrightnessDown", DALI_KEY_BRIGHTNESS_DOWN, false },
  { "XF86SoftKBD",           DALI_KEY_SOFT_KBD,        false },
  { "XF86QuickPanel",        DALI_KEY_QUICK_PANEL,     false },
  { "XF86TaskPane",          DALI_KEY_TASK_SWITCH,     false },
  { "XF86Apps",              DALI_KEY_APPS,            false },
  { "XF86Search",            DALI_KEY_SEARCH,          false },
  { "XF86Voice",             DALI_KEY_VOICE,           false },
  { "Hangul",                DALI_KEY_LANGUAGE,        false },
  { "XF86AudioRaiseVolume",  DALI_KEY_VOLUME_UP,       true  },
  { "XF86AudioLowerVolume",  DALI_KEY_VOLUME_DOWN,     true  },

  { "BackSpace",             DALI_KEY_BACKSPACE,       false },
  { "Left",                  DALI_KEY_CURSOR_LEFT,     false },
  { "Right",                 DALI_KEY_CURSOR_RIGHT,    false }
};

const std::size_t KEY_LOOKUP_COUNT = (sizeof( TestKeyLookupTable))/ (sizeof(KeyLookup));

enum TEST_TYPE
{
  GRAB_KEY_TOPMOST_P,
  UNGRAB_KEY_TOPMOST_P
};

struct MyTestApp : public ConnectionTracker
{
  MyTestApp( Application& app, int type )
  : mApplication( app ),
    mTestType( type )
  {
    mApplication.InitSignal().Connect( this, &MyTestApp::OnInit );
  }

  void OnInit(Application& app)
  {
    mTimer = Timer::New( 500 );
    mTimer.TickSignal().Connect( this, &MyTestApp::Tick );
    mTimer.Start();

    ExcuteTest();
  }

  bool Tick()
  {
    mTimer.Stop();
    mApplication.Quit();
    return true;
  }

  void ExcuteTest()
  {
    switch (mTestType)
    {
      case GRAB_KEY_TOPMOST_P:
        TestGrabKeyTopmostP();
        break;
      case UNGRAB_KEY_TOPMOST_P:
        TestUngrabKeyTopmostP();
        break;
    }
  }

  void TestGrabKeyTopmostP()
  {
    for ( std::size_t i = 0; i < KEY_LOOKUP_COUNT; ++i )
    {
      DALI_TEST_CHECK( KeyGrab::GrabKeyTopmost( mApplication.GetWindow(), TestKeyLookupTable[i].daliKeyCode ) );
    }
  }

  void TestUngrabKeyTopmostP()
  {
    for ( std::size_t i = 0; i < KEY_LOOKUP_COUNT; ++i )
    {
      DALI_TEST_CHECK( KeyGrab::GrabKeyTopmost( mApplication.GetWindow(), TestKeyLookupTable[i].daliKeyCode ) );
      DALI_TEST_CHECK( KeyGrab::UngrabKeyTopmost( mApplication.GetWindow(), TestKeyLookupTable[i].daliKeyCode ) );
    }
  }

  // Data
  Application& mApplication;
  int mTestType;
  Timer mTimer;
};

int UtcDaliKeyGrabGrabKeyTopmostP(void)
{
  Application application = Application::New( &gArgc, &gArgv );
  MyTestApp testApp( application, GRAB_KEY_TOPMOST_P );
  application.MainLoop();
  END_TEST;
}

int UtcDaliKeyGrabUngrabKeyTopmostP(void)
{
  Application application = Application::New( &gArgc, &gArgv );
  MyTestApp testApp( application, UNGRAB_KEY_TOPMOST_P );
  application.MainLoop();
  END_TEST;
}
