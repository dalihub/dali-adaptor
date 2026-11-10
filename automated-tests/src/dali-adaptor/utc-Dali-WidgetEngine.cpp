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
 */

// EXTERNAL INCLUDES
#include <dlfcn.h>

// INTERNAL INCLUDES
#include <adaptor-environment-variable.h>
#include <dali-test-suite-utils.h>
#include <dali/devel-api/adaptor-framework/widget-engine/widget-engine.h>

using namespace Dali;

namespace
{
constexpr const char* WIDGET_ENGINE_PLUGIN_ENV = "DALI_WIDGET_ENGINE_PLUGIN";

using ResetCountsFunction = void (*)();
using GetCountFunction    = int (*)();
} // namespace

void utc_dali_widget_engine_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_widget_engine_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliWidgetEngineLoadFailureP(void)
{
  EnvironmentVariable::SetTestEnvironmentVariable(WIDGET_ENGINE_PLUGIN_ENV, "/no/such/widget-engine-plugin.so");

  WidgetEngine engine = WidgetEngine::New("test.widget.viewer");
  DALI_TEST_CHECK(!engine);

  END_TEST;
}

int UtcDaliWidgetEngineLifecycleP(void)
{
  void* library = dlopen(TEST_WIDGET_ENGINE_PLUGIN_PATH, RTLD_LAZY);
  DALI_TEST_CHECK(library);

  auto resetCounts               = reinterpret_cast<ResetCountsFunction>(dlsym(library, "ResetWidgetEngineTestCounts"));
  auto getEngineDestroyedCount   = reinterpret_cast<GetCountFunction>(dlsym(library, "GetWidgetEngineDestroyedCount"));
  auto getInstanceDestroyedCount = reinterpret_cast<GetCountFunction>(dlsym(library, "GetWidgetEngineInstanceDestroyedCount"));
  auto getCallbacksResetCount    = reinterpret_cast<GetCountFunction>(dlsym(library, "GetWidgetEngineCallbacksResetCount"));
  DALI_TEST_CHECK(resetCounts && getEngineDestroyedCount && getInstanceDestroyedCount && getCallbacksResetCount);
  resetCounts();

  EnvironmentVariable::SetTestEnvironmentVariable(WIDGET_ENGINE_PLUGIN_ENV, TEST_WIDGET_ENGINE_PLUGIN_PATH);

  WidgetEngine engine = WidgetEngine::New("test.widget.viewer");
  DALI_TEST_CHECK(engine);
  DALI_TEST_CHECK(engine.IsAvailable());
  DALI_TEST_CHECK(WidgetEngine::DownCast(engine));

  WidgetEngineInstance instance = engine.CreateWidget("test.widget", "content", 360, 360, 1.0f);
  DALI_TEST_CHECK(instance);
  DALI_TEST_CHECK(WidgetEngineInstance::DownCast(instance));
  DALI_TEST_CHECK(instance.GetWidgetId() == "test.widget");
  DALI_TEST_CHECK(instance.GetInstanceId() == "test-instance");
  DALI_TEST_CHECK(instance.GetContentInfo() == "content");
  DALI_TEST_CHECK(instance.GetTitle() == "Test widget");

  bool eventReceived = false;
  bool frameReceived = false;
  instance.SetEventCallback([&eventReceived](WidgetEngineInstancePlugin::EventType event)
  {
    eventReceived = event == WidgetEngineInstancePlugin::EventType::ADDED;
  });
  instance.SetFrameUpdatedCallback([&frameReceived](NativeImagePtr image)
  {
    frameReceived = !image;
  });
  DALI_TEST_CHECK(eventReceived);
  DALI_TEST_CHECK(frameReceived);

  DALI_TEST_CHECK(instance.Pause());
  DALI_TEST_CHECK(instance.Resume());

  auto instanceCopy = instance;
  engine.Reset();
  DALI_TEST_CHECK(instance);
  DALI_TEST_EQUALS(getEngineDestroyedCount(), 0, TEST_LOCATION);

  instance.Reset();
  DALI_TEST_EQUALS(getCallbacksResetCount(), 0, TEST_LOCATION);
  DALI_TEST_EQUALS(getInstanceDestroyedCount(), 0, TEST_LOCATION);
  DALI_TEST_EQUALS(getEngineDestroyedCount(), 0, TEST_LOCATION);
  DALI_TEST_CHECK(instanceCopy.Resume());

  instanceCopy.Reset();
  DALI_TEST_EQUALS(getCallbacksResetCount(), 1, TEST_LOCATION);
  DALI_TEST_EQUALS(getInstanceDestroyedCount(), 1, TEST_LOCATION);
  DALI_TEST_EQUALS(getEngineDestroyedCount(), 1, TEST_LOCATION);

  dlclose(library);

  END_TEST;
}

int UtcDaliWidgetEngineDeferredReleaseWithoutMainLoopP(void)
{
  void* library = dlopen(TEST_WIDGET_ENGINE_PLUGIN_PATH, RTLD_LAZY);
  DALI_TEST_CHECK(library);
  auto resetCounts       = reinterpret_cast<ResetCountsFunction>(dlsym(library, "ResetWidgetEngineTestCounts"));
  auto completeRelease   = reinterpret_cast<ResetCountsFunction>(dlsym(library, "CompleteWidgetEngineTestRelease"));
  auto engineDestroyed   = reinterpret_cast<GetCountFunction>(dlsym(library, "GetWidgetEngineDestroyedCount"));
  auto instanceDestroyed = reinterpret_cast<GetCountFunction>(dlsym(library, "GetWidgetEngineInstanceDestroyedCount"));
  DALI_TEST_CHECK(resetCounts && completeRelease && engineDestroyed && instanceDestroyed);
  resetCounts();
  EnvironmentVariable::SetTestEnvironmentVariable(WIDGET_ENGINE_PLUGIN_ENV, TEST_WIDGET_ENGINE_PLUGIN_PATH);

  auto engine = WidgetEngine::New("test.widget.viewer.deferred");
  DALI_TEST_CHECK(engine);
  auto instance = engine.CreateWidget("test.widget", "", 360, 360, 1.0f);
  DALI_TEST_CHECK(instance);
  engine.Reset();
  instance.Reset();
  DALI_TEST_EQUALS(instanceDestroyed(), 1, TEST_LOCATION);
  DALI_TEST_EQUALS(engineDestroyed(), 0, TEST_LOCATION);

  // A completion must not unload the code that is still dispatching it.
  completeRelease();
  completeRelease();
  DALI_TEST_EQUALS(engineDestroyed(), 0, TEST_LOCATION);

  // Without an idle loop, the next safe host entry reclaims the retained entry.
  auto next = WidgetEngine::New("test.widget.viewer");
  DALI_TEST_CHECK(next);
  DALI_TEST_EQUALS(engineDestroyed(), 1, TEST_LOCATION);
  next.Reset();
  DALI_TEST_EQUALS(engineDestroyed(), 2, TEST_LOCATION);
  dlclose(library);
  END_TEST;
}

int UtcDaliWidgetEngineCompletionPreservesLiveHandleP(void)
{
  void* library = dlopen(TEST_WIDGET_ENGINE_PLUGIN_PATH, RTLD_LAZY);
  DALI_TEST_CHECK(library);
  auto resetCounts     = reinterpret_cast<ResetCountsFunction>(dlsym(library, "ResetWidgetEngineTestCounts"));
  auto completeRelease = reinterpret_cast<ResetCountsFunction>(dlsym(library, "CompleteWidgetEngineTestRelease"));
  auto engineDestroyed = reinterpret_cast<GetCountFunction>(dlsym(library, "GetWidgetEngineDestroyedCount"));
  DALI_TEST_CHECK(resetCounts && completeRelease && engineDestroyed);
  resetCounts();
  EnvironmentVariable::SetTestEnvironmentVariable(WIDGET_ENGINE_PLUGIN_ENV, TEST_WIDGET_ENGINE_PLUGIN_PATH);

  auto engine = WidgetEngine::New("test.widget.viewer.deferred");
  DALI_TEST_CHECK(engine);
  completeRelease();
  auto next = WidgetEngine::New("test.widget.viewer");
  DALI_TEST_CHECK(next);
  DALI_TEST_EQUALS(engineDestroyed(), 0, TEST_LOCATION);
  engine.Reset();
  DALI_TEST_EQUALS(engineDestroyed(), 1, TEST_LOCATION);
  next.Reset();
  DALI_TEST_EQUALS(engineDestroyed(), 2, TEST_LOCATION);
  dlclose(library);
  END_TEST;
}

int UtcDaliWidgetEngineLibraryOutlivesInstancesP(void)
{
  EnvironmentVariable::SetTestEnvironmentVariable(WIDGET_ENGINE_PLUGIN_ENV, TEST_WIDGET_ENGINE_PLUGIN_PATH);
  auto engine = WidgetEngine::New("test.widget.viewer");
  DALI_TEST_CHECK(engine);

  void* library = dlopen(TEST_WIDGET_ENGINE_PLUGIN_PATH, RTLD_LAZY | RTLD_NOLOAD);
  DALI_TEST_CHECK(library);
  auto resetCounts = reinterpret_cast<ResetCountsFunction>(dlsym(library, "ResetWidgetEngineTestCounts"));
  DALI_TEST_CHECK(resetCounts);
  resetCounts();
  // Drop the test's reference too: only the engine's shared loader may pin it.
  dlclose(library);
  engine.Reset();

  library = dlopen(TEST_WIDGET_ENGINE_PLUGIN_PATH, RTLD_LAZY | RTLD_NOLOAD);
  DALI_TEST_CHECK(library);
  auto engineDestroyed = reinterpret_cast<GetCountFunction>(dlsym(library, "GetWidgetEngineDestroyedCount"));
  DALI_TEST_CHECK(engineDestroyed);
  DALI_TEST_EQUALS(engineDestroyed(), 1, TEST_LOCATION);

  auto next = WidgetEngine::New("test.widget.viewer");
  DALI_TEST_CHECK(next);
  next.Reset();
  DALI_TEST_EQUALS(engineDestroyed(), 2, TEST_LOCATION);
  dlclose(library);
  END_TEST;
}

int UtcDaliWidgetEngineReusesLoadedLibraryP(void)
{
  EnvironmentVariable::SetTestEnvironmentVariable(WIDGET_ENGINE_PLUGIN_ENV, TEST_WIDGET_ENGINE_PLUGIN_PATH);
  auto first = WidgetEngine::New("test.widget.viewer");
  DALI_TEST_CHECK(first);

  // Once loaded, the same library is used for every engine in this process.
  EnvironmentVariable::SetTestEnvironmentVariable(WIDGET_ENGINE_PLUGIN_ENV, "/no/such/widget-engine-plugin.so");
  auto next = WidgetEngine::New("test.widget.viewer");
  DALI_TEST_CHECK(next);
  DALI_TEST_CHECK(first.IsAvailable());

  EnvironmentVariable::SetTestEnvironmentVariable(WIDGET_ENGINE_PLUGIN_ENV, TEST_WIDGET_ENGINE_PLUGIN_PATH);
  first.Reset();
  DALI_TEST_CHECK(next.IsAvailable());
  auto invalid = WidgetEngine::New("invalid.application.id");
  DALI_TEST_CHECK(!invalid);
  DALI_TEST_CHECK(next.IsAvailable());
  next.Reset();
  END_TEST;
}
