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
const char* AUL_LOADER_INIT_ENV           = "AUL_LOADER_INIT";
const char* AUL_LOADER_INIT_DEFAULT_VALUE = "0";
} // anonymous namespace

void EventLoopTcore::Initialize(int argc, char** argv)
{
  (void)argc;
  (void)argv;

  tizen_core_init();

  auto* task = static_cast<tizen_core_task_h*>(&mTask);
  int   ret  = tizen_core_task_create("main", false, task);
  if(ret != TIZEN_CORE_ERROR_NONE)
  {
    DALI_LOG_ERROR("Failed to create tizen core task: %d\n", ret);
    tizen_core_shutdown();
    mTask = nullptr;
    return;
  }
}

void EventLoopTcore::Shutdown()
{
  auto* task = static_cast<tizen_core_task_h>(mTask);
  if(task)
  {
    tizen_core_task_destroy(task);
    mTask = nullptr;
  }

  tizen_core_shutdown();

  if(Dali::EnvironmentVariable::GetEnvironmentVariable(AUL_LOADER_INIT_ENV))
  {
    Dali::EnvironmentVariable::SetEnvironmentVariable(AUL_LOADER_INIT_ENV, AUL_LOADER_INIT_DEFAULT_VALUE);
    tizen_core_shutdown();
  }
}

void EventLoopTcore::Run()
{
  auto* task = static_cast<tizen_core_task_h>(mTask);
  if(task)
  {
    tizen_core_task_run(task);
  }
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
