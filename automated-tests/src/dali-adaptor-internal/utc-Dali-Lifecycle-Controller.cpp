/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali-test-suite-utils.h>
#include <dali.h>
#include <dali/devel-api/adaptor-framework/lifecycle-controller.h>
#include <stdlib.h>
#include <iostream>

#include <dali/internal/adaptor/common/lifecycle-controller-impl.h>

using namespace Dali;

void utc_dali_lifecycle_controller_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_lifecycle_controller_cleanup(void)
{
  test_return_value = TET_PASS;
}

namespace
{
bool g_OnPreInitCalled         = false;
bool g_OnInitCalled            = false;
bool g_OnTerminateCalled       = false;
bool g_OnPauseCalled           = false;
bool g_OnResumeCalled          = false;
bool g_OnResetCalled           = false;
bool g_OnLanguageChangedCalled = false;

void OnPreInit()
{
  g_OnPreInitCalled = true;
}

void OnInit()
{
  g_OnInitCalled = true;
}

void OnTerminate()
{
  g_OnTerminateCalled = true;
}

void OnPause()
{
  g_OnPauseCalled = true;
}

void OnResume()
{
  g_OnResumeCalled = true;
}

void OnReset()
{
  g_OnResetCalled = true;
}

void OnLanguageChanged()
{
  g_OnLanguageChangedCalled = true;
}

} // anonymous namespace

int UtcDaliLifecycleControllerGet(void)
{
  // Attempt to retrieve LifecycleController before creating application
  LifecycleController lifecycleController;
  lifecycleController = LifecycleController::Get();
  DALI_TEST_CHECK(!lifecycleController);

  TestApplication app;
  Application     application = Application::New();

  lifecycleController = LifecycleController::Get();
  DALI_TEST_CHECK(lifecycleController);

  END_TEST;
}

int UtcDaliLifecycleControllerSignalPreInit(void)
{
  TestApplication app;
  Application     application = Application::New();

  DALI_TEST_CHECK(!g_OnPreInitCalled);

  LifecycleController lifecycleController = LifecycleController::Get();

  lifecycleController.PreInitSignal().Connect(&OnPreInit);

  GetImplementation(lifecycleController).OnPreInit(application);

  DALI_TEST_CHECK(g_OnPreInitCalled);

  END_TEST;
}

int UtcDaliLifecycleControllerSignalInit(void)
{
  TestApplication app;
  Application     application = Application::New();

  DALI_TEST_CHECK(!g_OnInitCalled);

  LifecycleController lifecycleController = LifecycleController::Get();

  lifecycleController.InitSignal().Connect(&OnInit);

  GetImplementation(lifecycleController).OnInit(application);

  DALI_TEST_CHECK(g_OnInitCalled);

  END_TEST;
}

int UtcDaliLifecycleControllerSignalTerminate(void)
{
  TestApplication app;
  Application     application = Application::New();

  DALI_TEST_CHECK(!g_OnTerminateCalled);

  LifecycleController lifecycleController = LifecycleController::Get();

  lifecycleController.TerminateSignal().Connect(&OnTerminate);

  GetImplementation(lifecycleController).OnTerminate(application);

  DALI_TEST_CHECK(g_OnTerminateCalled);

  END_TEST;
}

int UtcDaliLifecycleControllerSignalPause(void)
{
  TestApplication app;
  Application     application = Application::New();

  DALI_TEST_CHECK(!g_OnPauseCalled);

  LifecycleController lifecycleController = LifecycleController::Get();

  lifecycleController.PauseSignal().Connect(&OnPause);

  GetImplementation(lifecycleController).OnPause(application);

  DALI_TEST_CHECK(g_OnPauseCalled);

  END_TEST;
}

int UtcDaliLifecycleControllerSignalResume(void)
{
  TestApplication app;
  Application     application = Application::New();

  DALI_TEST_CHECK(!g_OnResumeCalled);

  LifecycleController lifecycleController = LifecycleController::Get();

  lifecycleController.ResumeSignal().Connect(&OnResume);

  GetImplementation(lifecycleController).OnResume(application);

  DALI_TEST_CHECK(g_OnResumeCalled);

  END_TEST;
}

int UtcDaliLifecycleControllerSignalReset(void)
{
  TestApplication app;
  Application     application = Application::New();

  DALI_TEST_CHECK(!g_OnResetCalled);

  LifecycleController lifecycleController = LifecycleController::Get();

  lifecycleController.ResetSignal().Connect(&OnReset);

  GetImplementation(lifecycleController).OnReset(application);

  DALI_TEST_CHECK(g_OnResetCalled);

  END_TEST;
}

int UtcDaliLifecycleControllerSignalLanguageChanged(void)
{
  TestApplication app;
  Application     application = Application::New();

  DALI_TEST_CHECK(!g_OnLanguageChangedCalled);

  LifecycleController lifecycleController = LifecycleController::Get();

  lifecycleController.LanguageChangedSignal().Connect(&OnLanguageChanged);

  GetImplementation(lifecycleController).OnLanguageChanged(application);

  DALI_TEST_CHECK(g_OnLanguageChangedCalled);

  END_TEST;
}
