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
#include <dali/internal/system/linux/ecore/event-loop-ecore.h>

// EXTERNAL INCLUDES
#include <Ecore.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
#if defined(DEBUG_ENABLED)
Integration::Log::Filter* gDBusLogging = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_DBUS");
#endif

const char* AUL_LOADER_INIT_ENV           = "AUL_LOADER_INIT";
const char* AUL_LOADER_INIT_DEFAULT_VALUE = "0";
} // anonymous namespace

void EventLoopEcore::Initialize(int argc, char** argv)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
  ecore_init();
  ecore_app_args_set(argc, (const char**)argv);
#pragma GCC diagnostic pop
}

void EventLoopEcore::Shutdown()
{
  ecore_shutdown();

  if(Dali::EnvironmentVariable::GetEnvironmentVariable(AUL_LOADER_INIT_ENV))
  {
    Dali::EnvironmentVariable::SetEnvironmentVariable(AUL_LOADER_INIT_ENV, AUL_LOADER_INIT_DEFAULT_VALUE);
    ecore_shutdown();
  }
}

void EventLoopEcore::Run()
{
  ecore_main_loop_begin();
}

void EventLoopEcore::Quit()
{
  ecore_main_loop_quit();
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
