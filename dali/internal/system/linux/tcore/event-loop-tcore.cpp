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

// CLASS HEADER
#include <dali/internal/system/linux/tcore/event-loop-tcore.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/debug.h>
#include <tizen_core.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const char* AUL_LOADER_INIT_ENV = "AUL_LOADER_INIT";
} // anonymous namespace

void EventLoopTcore::Initialize(int argc, char** argv)
{
  (void)argc;
  (void)argv;

  DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Initialize() - Entering\n");

  DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Initialize() - Calling tizen_core_init()\n");
  tizen_core_init();
  DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Initialize() - tizen_core_init() completed\n");

  auto* task = static_cast<tizen_core_task_h*>(&mTask);
  DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Initialize() - Creating tizen_core_task...\n");
  int   ret  = tizen_core_task_create("main", false, task);
  if(ret != TIZEN_CORE_ERROR_NONE)
  {
    DALI_LOG_ERROR("[EVENT_LOOP] Failed to create tizen core task: %d\n", ret);
    tizen_core_shutdown();
    mTask = nullptr;
    return;
  }
  DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Initialize() - tizen_core_task created successfully\n");

  DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Initialize() - Exiting successfully\n");
}

void EventLoopTcore::Shutdown()
{
  DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Shutdown() - Entering\n");

  auto* task = static_cast<tizen_core_task_h>(mTask);
  if(task)
  {
    DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Shutdown() - Destroying tizen_core_task...\n");
    tizen_core_task_destroy(task);
    mTask = nullptr;
    DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Shutdown() - tizen_core_task destroyed\n");
  }
  else
  {
    DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Shutdown() - Task is null, skipping task destroy\n");
  }

  // Check the AUL_LOADER_INIT environment variable.
  const char* loaderInit = Dali::EnvironmentVariable::GetEnvironmentVariable(AUL_LOADER_INIT_ENV);
  if(loaderInit)
  {
    DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Shutdown() - AUL_LOADER_INIT is set to '%s'\n", loaderInit);

    // Handle the launchpad-loader environment.
    // WidgetBase::Dispose() may unset AUL_LOADER_INIT, so only log it here.
    if(strcmp(loaderInit, "1") == 0)
    {
      DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Shutdown() - Detected launchpad-loader environment, calling tizen_core_shutdown\n");
    }
  }
  else
  {
    DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Shutdown() - AUL_LOADER_INIT is not set (normal environment)\n");
  }

  DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Shutdown() - Calling tizen_core_shutdown()...\n");
  tizen_core_shutdown();
  DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Shutdown() - tizen_core_shutdown() completed\n");

  DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Shutdown() - Exiting\n");
}

void EventLoopTcore::Run()
{
  DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Run() - Entering\n");
  auto* task = static_cast<tizen_core_task_h>(mTask);
  if(task)
  {
    DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Run() - Starting tizen_core_task_run()\n");
    tizen_core_task_run(task);
    DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Run() - tizen_core_task_run() returned\n");
  }
  else
  {
    DALI_LOG_ERROR("[EVENT_LOOP] EventLoopTcore::Run() - Task is null, cannot run\n");
  }
  DALI_LOG_RELEASE_INFO("[EVENT_LOOP] EventLoopTcore::Run() - Exiting\n");
}

void EventLoopTcore::Quit()
{
  auto* task = static_cast<tizen_core_task_h>(mTask);
  if(task)
  {
    tizen_core_task_quit(task);
  }
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
