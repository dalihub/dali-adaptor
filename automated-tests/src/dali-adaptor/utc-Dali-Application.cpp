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

void utc_dali_application_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_application_cleanup(void)
{
  test_return_value = TET_PASS;
}

namespace
{

struct MyTestApp : public ConnectionTracker
{
  MyTestApp( Application& app)
  : initCalled( false ),
    application( app )
  {
    application.InitSignal().Connect( this, &MyTestApp::Create );
  }

  void Create(Application& app)
  {
    initCalled = true;
  }

  void Quit()
  {
    application.Quit();
  }

  // Data
  bool initCalled;
  Application& application;
};

void ApplicationSignalCallback( Application& app )
{
}

void ApplicationControlSignalCallback(Application&, void *)
{
}

} // unnamed namespace

int UtcDaliApplicationNew(void)
{
  Application application = Application::New();

  MyTestApp testApp( application );

  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliApplicationCopyAndAssignment(void)
{
  Application application = Application::New();
  Application copy( application );
  DALI_TEST_CHECK( copy == application );

  Application assigned;
  DALI_TEST_CHECK( !assigned );
  assigned = application;
  DALI_TEST_CHECK( copy == assigned );

  END_TEST;
}

int UtcDaliApplicationInitSignal(void)
{
  Application application = Application::New();
  application.InitSignal().Connect( &ApplicationSignalCallback );
  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliApplicationTerminateSignal(void)
{
  Application application = Application::New();
  application.TerminateSignal().Connect( &ApplicationSignalCallback );
  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliApplicationPauseSignal(void)
{
  Application application = Application::New();
  application.PauseSignal().Connect( &ApplicationSignalCallback );
  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliApplicationResumeSignal(void)
{
  Application application = Application::New();
  application.ResumeSignal().Connect( &ApplicationSignalCallback );
  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliApplicationResetSignal(void)
{
  Application application = Application::New();
  application.ResetSignal().Connect( &ApplicationSignalCallback );
  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliApplicationResizeSignal(void)
{
  Application application = Application::New();
  application.ResizeSignal().Connect( &ApplicationSignalCallback );
  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliApplicationlControlSignal(void)
{
  Application application = Application::New();
  application.AppControlSignal().Connect( &ApplicationControlSignalCallback );
  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliApplicationLanguageChangedSignal(void)
{
  Application application = Application::New();
  application.LanguageChangedSignal().Connect( &ApplicationSignalCallback );
  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliApplicationRegionChangedSignal(void)
{
  Application application = Application::New();
  application.RegionChangedSignal().Connect( &ApplicationSignalCallback );
  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliApplicationBatteryLowSignal(void)
{
  Application application = Application::New();
  application.BatteryLowSignal().Connect( &ApplicationSignalCallback );
  DALI_TEST_CHECK( application );

  END_TEST;
}

int UtcDaliApplicationMemoryLowSignal(void)
{
  Application application = Application::New();
  application.MemoryLowSignal().Connect( &ApplicationSignalCallback );
  DALI_TEST_CHECK( application );

  END_TEST;
}
