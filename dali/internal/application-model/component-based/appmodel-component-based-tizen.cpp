/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/application-model/component-based/appmodel-component-based-tizen.h>

// EXTERNAL INCLUDES
#include <dlog.h>

// CONDITIONAL INCLUDES
#ifdef COMPONENT_APPLICATION_SUPPORT
#include <component_based_app_base.h>
#endif
#ifdef DALI_ELDBUS_AVAILABLE
#include <Eldbus.h>
#endif // DALI_ELDBUS_AVAILABLE

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/trace.h>
#include <dali/internal/adaptor/common/framework.h>
#include <dali/internal/adaptor/tizen-wayland/framework-tizen.h>
#include <dali/internal/system/linux/dali-ecore.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
extern "C" DALI_ADAPTOR_API AppModelComponentBased* Create()
{
  return new AppModelComponentBased;
}

extern "C" DALI_ADAPTOR_API void Destroy(void* p)
{
  AppModelComponentBased* appComponent = static_cast<AppModelComponentBased*>(p);
  delete appComponent;
}

extern "C" DALI_ADAPTOR_API int AppMain(bool isUiThread, void* data, void* pData)
{
  AppModelComponentBased* appComponent = static_cast<AppModelComponentBased*>(pData);

  int ret = 0;
  if(appComponent != nullptr)
  {
    ret = appComponent->AppMain(data);
  }
  else
  {
    print_log(DLOG_INFO, "DALI", "appComponent is nullptr");
  }
  return ret;
}

extern "C" DALI_ADAPTOR_API void AppExit(AppModelComponentBased* p)
{
  p->AppExit();
}

namespace
{
#if defined(DEBUG_ENABLED)
Integration::Log::Filter* gDBusLogging = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_DBUS");
#endif
DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_FRAMEWORK, true);

const char* AUL_LOADER_INIT_ENV           = "AUL_LOADER_INIT";
const char* AUL_LOADER_INIT_DEFAULT_VALUE = "0";
} // anonymous namespace

struct DALI_ADAPTOR_API AppModelComponentBased::Impl
{
  int AppMain(void* data)
  {
    int ret = TIZEN_ERROR_NOT_SUPPORTED;
#ifdef COMPONENT_APPLICATION_SUPPORT
    FrameworkTizen* mFramework = static_cast<FrameworkTizen*>(data);

    /*Crate component_based_app_base_lifecycle_callback*/
    component_based_app_base_lifecycle_callback_s callback;
    callback.init      = AppInit;
    callback.run       = AppRun;
    callback.exit      = AppExit;
    callback.create    = ComponentAppCreate;
    callback.terminate = ComponentAppTerminate;
    callback.fini      = ComponentAppFinish;

    ret = component_based_app_base_main(*mFramework->GetArgc(), *mFramework->GetArgv(), &callback, mFramework);
#else
    DALI_LOG_ERROR("component application feature is not supported");
#endif
    return ret;
  }

  static void AppInit(int argc, char** argv, void* data)
  {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    ecore_init();
    ecore_app_args_set(argc, (const char**)argv);
#pragma GCC diagnostic pop

#ifdef DALI_ELDBUS_AVAILABLE
    // Initialize ElDBus.
    DALI_LOG_INFO(gDBusLogging, Debug::General, "Starting DBus Initialization\n");
    eldbus_init();
#endif
  }

  static void AppRun(void* data)
  {
    ecore_main_loop_begin();
  }

  static void AppExit(void* data)
  {
    ecore_main_loop_quit();
  }

  static void* ComponentAppCreate(void* data)
  {
    FrameworkTizen*      framework = static_cast<FrameworkTizen*>(data);
    Framework::Observer* observer  = &framework->GetObserver();
    observer->OnInit();

    return Dali::AnyCast<void*>(observer->OnCreate());
  }

  static void ComponentAppTerminate(void* data)
  {
    Framework::Observer* observer = &static_cast<FrameworkTizen*>(data)->GetObserver();
    observer->OnTerminate();
  }

  static void ComponentAppFinish(void* data)
  {
    ecore_shutdown();

    if(Dali::EnvironmentVariable::GetEnvironmentVariable(AUL_LOADER_INIT_ENV))
    {
      Dali::EnvironmentVariable::SetEnvironmentVariable(AUL_LOADER_INIT_ENV, AUL_LOADER_INIT_DEFAULT_VALUE);
      ecore_shutdown();
    }
  }

  void AppExit()
  {
#ifdef COMPONENT_APPLICATION_SUPPORT
    component_based_app_base_exit();
#endif
  }

  Impl(void* data)
  {
    mAppModelComponentBased = static_cast<AppModelComponentBased*>(data);
  }

  ~Impl()
  {
  }

  AppModelComponentBased* mAppModelComponentBased;
};

AppModelComponentBased::AppModelComponentBased()
{
  mImpl = new Impl(this);
}

AppModelComponentBased::~AppModelComponentBased()
{
  delete mImpl;
}

int AppModelComponentBased::AppMain(void* data)
{
  return mImpl->AppMain(data);
}

void AppModelComponentBased::AppExit()
{
  mImpl->AppExit();
}
} // namespace Adaptor
} // namespace Internal
} // namespace Dali