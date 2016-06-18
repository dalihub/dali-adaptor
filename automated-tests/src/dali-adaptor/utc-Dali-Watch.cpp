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

#include <dali/dali.h>
#include <dali-test-suite-utils.h>

using namespace Dali;

void utc_dali_watchapplication_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_watchapplication_cleanup(void)
{
  test_return_value = TET_PASS;
}

namespace
{

struct MyTestApp : public ConnectionTracker
{
  MyTestApp( WatchApplication& app)
  : initCalled( false ),
    mApplication( app )
  {
    mApplication.InitSignal().Connect( this, &MyTestApp::Create );
  }

  void Create(Application& app)
  {
    initCalled = true;
  }

  void Quit()
  {
    mApplication.Quit();
  }

  // Data
  bool initCalled;
  WatchApplication&  mApplication;
};

void WatchTimeSignalCallback( Application& app, const WatchTime& time)
{
}

void WatchChangedSignalCallback( Application& app, bool ambient)
{
}

} // unnamed namespace

int UtcDaliWatchApplicationNew01(void)
{
  WatchApplication application = WatchApplication::New();

  MyTestApp testApp( application );

  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliWatchApplicationNew02(void)
{
  int argc( 1 );
  const char* argList[1] = { "program" };
  char** argv = const_cast<char**>(argList);

  WatchApplication application = WatchApplication::New( &argc, &argv );

  MyTestApp testApp( application );

  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliWatchApplicationNew03(void)
{
  int argc( 1 );
  const char* argList[1] = { "program" };
  char** argv = const_cast<char**>(argList);

  WatchApplication application = WatchApplication::New( &argc, &argv, "stylesheet" );

  MyTestApp testApp( application );

  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliWatchApplicationCopyAndAssignment(void)
{
  WatchApplication application = WatchApplication::New();
  WatchApplication copy( application );
  DALI_TEST_CHECK( copy == application );

  WatchApplication assigned;
  DALI_TEST_CHECK( !assigned );
  assigned = application;
  DALI_TEST_CHECK( copy == assigned );

  END_TEST;
}

int UtcDaliWatchApplicationTimeTickSignalP(void)
{
  WatchApplication application = WatchApplication::New();
  application.TimeTickSignal().Connect( &WatchTimeSignalCallback );
  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliWatchApplicationTimeTickSignalN(void)
{
  WatchApplication application;

  try
  {
    application.TimeTickSignal().Connect( &WatchTimeSignalCallback );
    DALI_TEST_CHECK( false ); // Should not get here
  }
  catch( ... )
  {
    DALI_TEST_CHECK( true );
  }

  END_TEST;
}

int UtcDaliWatchApplicationAmbientTickSignalP(void)
{
  WatchApplication application = WatchApplication::New();
  application.AmbientTickSignal().Connect( &WatchTimeSignalCallback );
  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliWatchApplicationAmbientTickSignalN(void)
{
  WatchApplication application;

  try
  {
    application.AmbientTickSignal().Connect( &WatchTimeSignalCallback );
    DALI_TEST_CHECK( false ); // Should not get here
  }
  catch( ... )
  {
    DALI_TEST_CHECK( true );
  }

  END_TEST;
}

int UtcDaliWatchApplicationAmbientChangedSignalP(void)
{
  WatchApplication application = WatchApplication::New();
  application.AmbientChangedSignal().Connect( &WatchChangedSignalCallback );
  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliWatchApplicationAmbientChangedSignalN(void)
{
  WatchApplication application;

  try
  {
    application.AmbientChangedSignal().Connect( &WatchChangedSignalCallback );
    DALI_TEST_CHECK( false ); // Should not get here
  }
  catch( ... )
  {
    DALI_TEST_CHECK( true );
  }

  END_TEST;
}

