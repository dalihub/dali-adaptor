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
#include <dali/internal/application-model/component-based/appmodel-component-based-tizen.h>

// EXTERNAL INCLUDES
#include <dlog.h>

// CONDITIONAL INCLUDES
#ifdef COMPONENT_APPLICATION_SUPPORT
#include <component_based_app_base.h>
#endif

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/adaptor/common/framework.h>
#include <dali/internal/adaptor/tizen/framework-tizen.h>
#include <dali/internal/system/common/event-loop.h>
#include <dali/internal/system/common/system-factory.h>

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

struct DALI_ADAPTOR_API AppModelComponentBased::Impl
{
  int AppMain(void* data)
  {
    int ret = TIZEN_ERROR_NOT_SUPPORTED;
#ifdef COMPONENT_APPLICATION_SUPPORT
    mFramework = static_cast<FrameworkTizen*>(data);

    /*Crate component_based_app_base_lifecycle_callback*/
    component_based_app_base_lifecycle_callback_s callback;
    callback.init      = AppInit;
    callback.run       = AppRun;
    callback.exit      = AppExit;
    callback.create    = ComponentAppCreate;
    callback.terminate = ComponentAppTerminate;
    callback.fini      = ComponentAppFinish;

    ret = component_based_app_base_main(*mFramework->GetArgc(), *mFramework->GetArgv(), &callback, mAppModelComponentBased);
#else
    DALI_LOG_ERROR("component application feature is not supported");
#endif
    return ret;
  }

  static void AppInit(int argc, char** argv, void* data)
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppInit() emitted", __MODULE__, __func__, __LINE__);
    auto* impl       = static_cast<AppModelComponentBased*>(data)->mImpl;
    impl->mEventLoop = Dali::Internal::Adaptor::GetSystemFactory()->CreateEventLoop();
    if(impl->mEventLoop)
    {
      impl->mEventLoop->Initialize(argc, argv);
    }
    else
    {
      print_log(DLOG_INFO, "DALI", "%s: %s(%d) > Failed to create EventLoop", __MODULE__, __func__, __LINE__);
    }
  }

  static void AppRun(void* data)
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppRun() emitted", __MODULE__, __func__, __LINE__);
    auto* impl = static_cast<AppModelComponentBased*>(data)->mImpl;
    if(impl->mEventLoop)
    {
      impl->mEventLoop->Run();
    }
    else
    {
      print_log(DLOG_INFO, "DALI", "%s: %s(%d) > EventLoop is not created", __MODULE__, __func__, __LINE__);
    }
  }

  static void AppExit(void* data)
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppExit() emitted", __MODULE__, __func__, __LINE__);
    auto* impl = static_cast<AppModelComponentBased*>(data)->mImpl;
    if(impl->mEventLoop)
    {
      impl->mEventLoop->Quit();
    }
    else
    {
      print_log(DLOG_INFO, "DALI", "%s: %s(%d) > EventLoop is not created", __MODULE__, __func__, __LINE__);
    }
  }

  static void* ComponentAppCreate(void* data)
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > ComponentAppCreate() emitted", __MODULE__, __func__, __LINE__);
    auto*                impl      = static_cast<AppModelComponentBased*>(data)->mImpl;
    FrameworkTizen*      framework = impl->mFramework;
    Framework::Observer* observer  = &framework->GetObserver();
    observer->OnInit();

    return Dali::AnyCast<void*>(observer->OnCreate());
  }

  static void ComponentAppTerminate(void* data)
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > ComponentAppTerminate() emitted", __MODULE__, __func__, __LINE__);
    auto*                impl     = static_cast<AppModelComponentBased*>(data)->mImpl;
    Framework::Observer* observer = &impl->mFramework->GetObserver();
    observer->OnTerminate();
  }

  static void ComponentAppFinish(void* data)
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > ComponentAppFinish() emitted", __MODULE__, __func__, __LINE__);
    auto* impl = static_cast<AppModelComponentBased*>(data)->mImpl;
    if(impl->mEventLoop)
    {
      impl->mEventLoop->Shutdown();
      impl->mEventLoop.reset();
    }
    else
    {
      print_log(DLOG_INFO, "DALI", "%s: %s(%d) > EventLoop is not created", __MODULE__, __func__, __LINE__);
    }
  }

  void AppExit()
  {
#ifdef COMPONENT_APPLICATION_SUPPORT
    component_based_app_base_exit();
#endif
  }

  Impl(void* data)
  : mEventLoop(),
    mAppModelComponentBased(static_cast<AppModelComponentBased*>(data)),
    mFramework(nullptr)
  {
  }

  ~Impl()
  {
  }

  std::unique_ptr<EventLoop> mEventLoop;
  AppModelComponentBased*    mAppModelComponentBased;
  FrameworkTizen*            mFramework;
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
