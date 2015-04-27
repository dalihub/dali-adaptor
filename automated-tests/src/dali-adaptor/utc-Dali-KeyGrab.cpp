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

// Copied from key-impl.h
struct KeyLookup
{
  const char* keyName;      ///< X string representation
  const KEY   daliKeyCode;  ///< Dali Enum Representation
  const bool  deviceButton; ///< Whether the key is from a button on the device
};

KeyLookup TestKeyLookupTable[]=
{
  { "XF86Camera",            DALI_KEY_CAMERA,          false },
  { "XF86Camera_Full",       DALI_KEY_CONFIG,          false },
  { "XF86PowerOff",          DALI_KEY_POWER,           true  },
  { "Cancel",                DALI_KEY_CANCEL,          false },
  { "XF86AudioStop",         DALI_KEY_STOP_CD,         false },
  { "XF86AudioPause",        DALI_KEY_PAUSE_CD,        false },
  { "XF86AudioNext",         DALI_KEY_NEXT_SONG,       false },
  { "XF86AudioPrev",         DALI_KEY_PREVIOUS_SONG,   false },
  { "XF86AudioRewind",       DALI_KEY_REWIND,          false },
  { "XF86AudioForward",      DALI_KEY_FASTFORWARD,     false },
  { "XF86AudioMedia",        DALI_KEY_MEDIA,           false },
  { "XF86AudioPlayPause",    DALI_KEY_PLAY_PAUSE,      false },
  { "XF86AudioMute",         DALI_KEY_MUTE,            false },
  { "XF86HomePage",          DALI_KEY_HOMEPAGE,        false },
  { "XF86WWW",               DALI_KEY_WEBPAGE,         false },
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
};

const std::size_t KEY_LOOKUP_COUNT = (sizeof( TestKeyLookupTable))/ (sizeof(KeyLookup));

enum TEST_TYPE
{
  GRAB_KEY_P,
  UNGRAB_KEY_P,
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
    mStartTimer = Timer::New( 100 );
    mStartTimer.TickSignal().Connect( this, &MyTestApp::StartTick );
    mStartTimer.Start();

    mTimer = Timer::New( 500 );
    mTimer.TickSignal().Connect( this, &MyTestApp::Tick );
    mTimer.Start();
  }

  bool StartTick()
  {
    mStartTimer.Stop();
    ExcuteTest();
    return true;
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
      case GRAB_KEY_P:
        TestGrabKeyP();
        break;
      case UNGRAB_KEY_P:
        TestUngrabKeyP();
        break;
      case UNGRAB_KEY_N:
        TestUngrabKeyN();
        break;
      case GRAB_KEY_TOPMOST_P:
        TestGrabKeyTopmostP();
        break;
      case UNGRAB_KEY_TOPMOST_P:
        TestUngrabKeyTopmostP();
        break;
    }
  }

  void TestGrabKeyP()
  {
    for ( std::size_t i = 0; i < KEY_LOOKUP_COUNT; ++i )
    {
      DALI_TEST_CHECK( KeyGrab::GrabKey( mApplication.GetWindow(), TestKeyLookupTable[i].daliKeyCode, KeyGrab::TOPMOST ) );
      DALI_TEST_CHECK( KeyGrab::GrabKey( mApplication.GetWindow(), TestKeyLookupTable[i].daliKeyCode, KeyGrab::SHARED ) );
    }
  }

  void TestUngrabKeyP()
  {
    for ( std::size_t i = 0; i < KEY_LOOKUP_COUNT; ++i )
    {
      DALI_TEST_CHECK( KeyGrab::GrabKey( mApplication.GetWindow(), TestKeyLookupTable[i].daliKeyCode, KeyGrab::TOPMOST ) );
      DALI_TEST_CHECK( KeyGrab::UngrabKey( mApplication.GetWindow(), TestKeyLookupTable[i].daliKeyCode ) );
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
  Timer mTimer, mStartTimer;
};

int UtcDaliKeyGrabGrabKeyP(void)
{
  Application application = Application::New( &gArgc, &gArgv );
  MyTestApp testApp( application, GRAB_KEY_P );
  application.MainLoop();
  END_TEST;
}

int UtcDaliKeyGrabUngrabKeyP(void)
{
  Application application = Application::New( &gArgc, &gArgv );
  MyTestApp testApp( application, UNGRAB_KEY_P );
  application.MainLoop();
  END_TEST;
}

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

