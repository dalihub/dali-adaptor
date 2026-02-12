/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include <dali-test-suite-utils.h>
#include <dali/dali.h>
#include <dali/devel-api/adaptor-framework/application-devel.h>
#include <stdlib.h>

#include <adaptor-environment-variable.h> ///< for Dali::SetTestEnvironmentVariable

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
  MyTestApp(Application& app)
  : initCalled(false),
    application(app)
  {
    application.InitSignal().Connect(this, &MyTestApp::Create);
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
  bool         initCalled;
  Application& application;
};

void ApplicationSignalCallback(Application& app)
{
}

void ApplicationControlSignalCallback(Application&, void*)
{
}

} // unnamed namespace

void LowBatterySignalCallback(Dali::DeviceStatus::Battery::Status status)
{
}

void LowMemorySignalCallback(Dali::DeviceStatus::Memory::Status status)
{
}

void OnDeviceOrientationChangedSignalCallback(DeviceStatus::Orientation::Status status)
{
}

int UtcDaliApplicationNew01(void)
{
  Application application = Application::New();

  MyTestApp testApp(application);

  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationNew02(void)
{
  int         argc(1);
  const char* argList[1] = {"program"};
  char**      argv       = const_cast<char**>(argList);

  Application application = Application::New(&argc, &argv);

  MyTestApp testApp(application);

  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationNew03(void)
{
  int         argc(1);
  const char* argList[1] = {"program"};
  char**      argv       = const_cast<char**>(argList);

  Application application = Application::New(&argc, &argv, "stylesheet");

  MyTestApp testApp(application);

  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationNew04(void)
{
  int         argc(1);
  const char* argList[1] = {"program"};
  char**      argv       = const_cast<char**>(argList);

  Application application = Application::New(&argc, &argv, "stylesheet", Application::TRANSPARENT);

  MyTestApp testApp(application);

  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationNew06P(void)
{
  int         argc(1);
  const char* argList[1] = {"program"};
  char**      argv       = const_cast<char**>(argList);

  Application application = Application::New(&argc, &argv, "stylesheet", Application::WindowOpacity::OPAQUE, PositionSize(), true);

  MyTestApp testApp(application);

  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationNew07P(void)
{
  int         argc(1);
  const char* argList[1] = {"program"};
  char**      argv       = const_cast<char**>(argList);

  Application application = Application::New(&argc, &argv, "stylesheet", Application::WindowOpacity::OPAQUE, PositionSize());

  MyTestApp testApp(application);

  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationNew08P(void)
{
  int         argc(1);
  const char* argList[1] = {"program"};
  char**      argv       = const_cast<char**>(argList);
  WindowData  windowData;

  Application application = Application::New(&argc, &argv, "stylesheet", false, windowData);

  MyTestApp testApp(application);

  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationCopyAndAssignment(void)
{
  Application application = Application::New();
  Application copy(application);
  DALI_TEST_CHECK(copy == application);

  Application assigned;
  DALI_TEST_CHECK(!assigned);
  assigned = application;
  DALI_TEST_CHECK(copy == assigned);

  END_TEST;
}

int UtcDaliApplicationMoveConstructor(void)
{
  Application application = Application::New();
  DALI_TEST_CHECK(application);
  DALI_TEST_EQUALS(1, application.GetBaseObject().ReferenceCount(), TEST_LOCATION);

  Application moved = std::move(application);
  DALI_TEST_CHECK(moved);
  DALI_TEST_EQUALS(1, moved.GetBaseObject().ReferenceCount(), TEST_LOCATION);
  DALI_TEST_CHECK(!application);

  END_TEST;
}

int UtcDaliApplicationMoveAssignment(void)
{
  Application application = Application::New();
  DALI_TEST_CHECK(application);
  DALI_TEST_EQUALS(1, application.GetBaseObject().ReferenceCount(), TEST_LOCATION);

  Application moved;
  moved = std::move(application);
  DALI_TEST_CHECK(moved);
  DALI_TEST_EQUALS(1, moved.GetBaseObject().ReferenceCount(), TEST_LOCATION);
  DALI_TEST_CHECK(!application);

  END_TEST;
}

int UtcDaliApplicationMainLoop01N(void)
{
  Application application;

  try
  {
    application.MainLoop();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationLowerN(void)
{
  Application application;

  try
  {
    application.Lower();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationQuitN(void)
{
  Application application;

  try
  {
    application.Quit();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationAddIdleN(void)
{
  Application application;

  try
  {
    CallbackBase* callback = NULL;
    application.AddIdle(callback);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationGetWindowN(void)
{
  Application application;

  try
  {
    (void)application.GetWindow();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationInitSignalP(void)
{
  Application application = Application::New();
  application.InitSignal().Connect(&ApplicationSignalCallback);
  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationInitSignalN(void)
{
  Application application;

  try
  {
    application.InitSignal().Connect(&ApplicationSignalCallback);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationTerminateSignalP(void)
{
  Application application = Application::New();
  application.TerminateSignal().Connect(&ApplicationSignalCallback);
  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationTerminateSignalN(void)
{
  Application application;

  try
  {
    application.TerminateSignal().Connect(&ApplicationSignalCallback);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationPauseSignalP(void)
{
  Application application = Application::New();
  application.PauseSignal().Connect(&ApplicationSignalCallback);
  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationPauseSignalN(void)
{
  Application application;

  try
  {
    application.PauseSignal().Connect(&ApplicationSignalCallback);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationResumeSignalP(void)
{
  Application application = Application::New();
  application.ResumeSignal().Connect(&ApplicationSignalCallback);
  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationResumeSignalN(void)
{
  Application application;

  try
  {
    application.ResumeSignal().Connect(&ApplicationSignalCallback);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationResetSignalP(void)
{
  Application application = Application::New();
  application.ResetSignal().Connect(&ApplicationSignalCallback);
  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationResetSignalN(void)
{
  Application application;

  try
  {
    application.ResetSignal().Connect(&ApplicationSignalCallback);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationlControlSignalP(void)
{
  Application application = Application::New();
  application.AppControlSignal().Connect(&ApplicationControlSignalCallback);
  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationlControlSignalN(void)
{
  Application application;

  try
  {
    application.AppControlSignal().Connect(&ApplicationControlSignalCallback);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationLanguageChangedSignalP(void)
{
  Application application = Application::New();
  application.LanguageChangedSignal().Connect(&ApplicationSignalCallback);
  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationLanguageChangedSignalN(void)
{
  Application application;

  try
  {
    application.LanguageChangedSignal().Connect(&ApplicationSignalCallback);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationRegionChangedSignalP(void)
{
  Application application = Application::New();
  application.RegionChangedSignal().Connect(&ApplicationSignalCallback);
  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationRegionChangedSignalN(void)
{
  Application application;

  try
  {
    application.RegionChangedSignal().Connect(&ApplicationSignalCallback);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationLowBatterySignalP(void)
{
  Application application = Application::New();
  application.LowBatterySignal().Connect(&LowBatterySignalCallback);
  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationLowBatterySignalN(void)
{
  Application application;

  try
  {
    application.LowBatterySignal().Connect(&LowBatterySignalCallback);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationLowMemorySignalP(void)
{
  Application application = Application::New();
  application.LowMemorySignal().Connect(&LowMemorySignalCallback);
  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationLowMemorySignalN(void)
{
  Application application;

  try
  {
    application.LowMemorySignal().Connect(&LowMemorySignalCallback);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationOrientationChangedSignalP(void)
{
  Application application = Application::New();
  application.DeviceOrientationChangedSignal().Connect(&OnDeviceOrientationChangedSignalCallback);
  DALI_TEST_CHECK(application);

  END_TEST;
}

int UtcDaliApplicationOrientationChangedSignalN(void)
{
  Application application;

  try
  {
    application.DeviceOrientationChangedSignal().Connect(&OnDeviceOrientationChangedSignalCallback);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationGetResourcePathP(void)
{
  Application application = Application::New();
  std::string result("**invalid path**"); // Calling GetResourcePath should replace this with a system dependent path or "".
  result = application.GetResourcePath();
  DALI_TEST_CHECK(result != "**invalid path**");

  END_TEST;
}

int UtcDaliApplicationGetRegionP(void)
{
  Application application = Application::New();
  std::string result;
  result = application.GetRegion();
  DALI_TEST_CHECK(result == "NOT_SUPPORTED"); // Not supported in UBUNTU

  END_TEST;
}

int UtcDaliApplicationGetLanguageP(void)
{
  Application application = Application::New();
  std::string result;
  result = application.GetLanguage();
  DALI_TEST_CHECK(result == "NOT_SUPPORTED"); // Not supported in UBUNTU

  END_TEST;
}

int UtcDaliApplicationGetObjectRegistryN(void)
{
  Application application = Application::New();
  DALI_TEST_CHECK(!application.GetObjectRegistry());
  END_TEST;
}

int UtcDaliApplicationAddIdleWithReturnValueN(void)
{
  Application application;

  try
  {
    DevelApplication::AddIdleWithReturnValue(application, nullptr);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationGetDataPathP(void)
{
  Dali::EnvironmentVariable::SetTestEnvironmentVariable("DALI_APPLICATION_DATA_DIR", "MyDataPath");
  std::string dataPath = DevelApplication::GetDataPath();
  DALI_TEST_EQUALS(dataPath, "MyDataPath", TEST_LOCATION);
  END_TEST;
}

int UtcDaliApplicationDownCastN(void)
{
  Application application = DevelApplication::DownCast(nullptr);
  DALI_TEST_CHECK(!application);
  END_TEST;
}

int UtcDaliApplicationTaskInitSignalN(void)
{
  Application application;

  try
  {
    application.TaskInitSignal();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationTaskTerminateSignalN(void)
{
  Application application;

  try
  {
    application.TaskTerminateSignal();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationTaskAppControlSignalN(void)
{
  Application application;

  try
  {
    application.TaskAppControlSignal();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationTaskLanguageChangedSignalN(void)
{
  Application application;

  try
  {
    application.TaskLanguageChangedSignal();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationTaskRegionChangedSignalN(void)
{
  Application application;

  try
  {
    application.TaskRegionChangedSignal();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationTaskLowBatterySignalN(void)
{
  Application application;

  try
  {
    application.TaskLowBatterySignal();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationTaskLowMemorySignalN(void)
{
  Application application;

  try
  {
    application.TaskLowMemorySignal();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliApplicationTaskDeviceOrientationChangedSignalN(void)
{
  Application application;

  try
  {
    application.TaskDeviceOrientationChangedSignal();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}
