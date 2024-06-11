/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <dali/internal/adaptor/tizen-wayland/framework-tizen.h>

// EXTERNAL INCLUDES
#include <dlfcn.h>
#include <dlog.h>
#include <system_info.h>
#include <system_settings.h>
#include <tizen.h>

#ifdef UI_THREAD_AVAILABLE
#include <app_core_ui_thread_base.hh>
#endif

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/trace.h>

using namespace tizen_cpp;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
constexpr char const* const kApplicationNamePrefix     = "libdali2-adaptor-application-";
constexpr char const* const kApplicationNamePostfix    = ".so";

std::string MakePluginName(const char* appModelName)
{
  std::stringstream fullName;
  fullName << kApplicationNamePrefix << appModelName << kApplicationNamePostfix;
  return fullName.str();
}

#if defined(DEBUG_ENABLED)
Integration::Log::Filter* gDBusLogging = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_DBUS");
#endif
DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_FRAMEWORK, true);

// Note : tizen appfw don't consider zero-arguments case.
// If framework argc & argv is nullptr, We should add at least one argv.
const int   gTizenDummyArgc    = 1;
const char* gTizenDummyArgv[1] = {"dali-tizen-app"};
} // anonymous namespace

/**
 * Impl to hide EFL data members
 */
struct FrameworkTizen::Impl
{
  // Constructor
  Impl(void* data, Type type, bool isUiThread)
  {
    mFramework = static_cast<FrameworkTizen*>(data);
    mUiThread = isUiThread;
#ifndef APPCORE_WATCH_AVAILABLE
    if(type == WATCH)
    {
      throw Dali::DaliException("", "Watch Application is not supported.");
    }
#endif
    mApplicationType = type;

    std::string pluginName;
    switch(mApplicationType)
    {
      case NORMAL:
      {
        pluginName = MakePluginName("normal");
        break;
      }
      case WIDGET:
      {
        pluginName = MakePluginName("widget");
        break;
      }
      case WATCH:
      {
        pluginName = MakePluginName("watch");
        break;
      }
#ifdef COMPONENT_APPLICATION_SUPPORT
      case COMPONENT:
      {
        pluginName = MakePluginName("component-based");
        break;
      }
#endif
      default:
      {
        DALI_LOG_ERROR("Invalid app type : %d\n", static_cast<int>(mApplicationType));
        pluginName = MakePluginName("normal");
      }
    }

    mHandle = dlopen(pluginName.c_str(), RTLD_LAZY);
    if(mHandle == nullptr)
    {
      print_log(DLOG_INFO, "DALI", "error : %s", dlerror() );
      return;
    }

    createFunctionPtr = reinterpret_cast<CreateFunction>(dlsym(mHandle, "Create"));
    if(createFunctionPtr == nullptr)
    {
      DALI_LOG_ERROR("createFunctionPtr is null\n");
    }
    destroyFunctionPtr = reinterpret_cast<DestroyFunction>(dlsym(mHandle, "Destroy"));
    if(destroyFunctionPtr == nullptr)
    {
      DALI_LOG_ERROR("destroyFunctionPtr is null\n");
    }
    appMainFunctionPtr = reinterpret_cast<AppMainFunction>(dlsym(mHandle, "AppMain"));
    if(appMainFunctionPtr == nullptr)
    {
      DALI_LOG_ERROR("appMainFunctionPtr is null\n");
    }
    appExitFunctionPtr = reinterpret_cast<AppExitFunction>(dlsym(mHandle, "AppExit"));
    if(appExitFunctionPtr == nullptr)
    {
      DALI_LOG_ERROR("appExitFunctionPtr is null\n");
    }
  }

  ~Impl()
  {
    if(mHandle != NULL)
    {
      if(destroyFunctionPtr != NULL)
      {
        destroyFunctionPtr(baseAppPtr);
      }

      dlclose(mHandle);
    }
  }

  int AppMain()
  {
    if(mHandle == nullptr)
    {
      print_log(DLOG_INFO, "DALI", "mHandle is null");
      return TIZEN_ERROR_NOT_SUPPORTED;
    }

    if(createFunctionPtr != nullptr)
    {
      baseAppPtr = createFunctionPtr();
    }

    if(baseAppPtr == nullptr)
    {
      DALI_LOG_ERROR("p is null\n");
      return TIZEN_ERROR_NOT_SUPPORTED;
    }

    int ret = 0;
    if(appMainFunctionPtr != nullptr)
    {
      ret = appMainFunctionPtr(mUiThread, mFramework, baseAppPtr);
    }
    return ret;
  }

  void AppExit()
  {
    if(baseAppPtr == nullptr)
    {
      DALI_LOG_ERROR("baseAppPtr is null\n");
      return;
    }
    appExitFunctionPtr(baseAppPtr);
  }

  void SetLanguage(const std::string& language)
  {
    mLanguage = language;
  }

  void SetRegion(const std::string& region)
  {
    mRegion = region;
  }

  std::string GetLanguage()
  {
    if(mLanguage.empty())
    {
      char* language = nullptr;
      system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &language);

      if(language != nullptr)
      {
        mLanguage = std::string(language);
        free(language);
      }
    }
    return mLanguage;
  }

  std::string GetRegion()
  {
    if(mRegion.empty())
    {
      char* region = nullptr;
      system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_COUNTRY, &region);

      if(region != nullptr)
      {
        mRegion = std::string(region);
        free(region);
      }
    }
    return mRegion;
  }

  using CreateFunction          = void* (*)();
  using DestroyFunction         = void (*)(void*);

  using AppMainFunction         = int (*)(bool, void*, void*);
  using AppExitFunction         = void (*)(void*);

  void*                         mHandle{nullptr};
  CreateFunction                createFunctionPtr;
  DestroyFunction               destroyFunctionPtr;
  AppMainFunction               appMainFunctionPtr;
  AppExitFunction               appExitFunctionPtr;
  void*                         baseAppPtr = nullptr;
  bool                          mUiThread;

  // Data
  Type                          mApplicationType;
  std::string                   mLanguage{};
  std::string                   mRegion{};

  FrameworkTizen*               mFramework;

private:
  // Undefined
  Impl(const Impl& impl);

  // Undefined
  Impl& operator=(const Impl& impl);
};

FrameworkTizen::FrameworkTizen(Framework::Observer& observer, Framework::TaskObserver& taskObserver, int* argc, char*** argv, Type type, bool useUiThread)
: Framework(observer, taskObserver, argc, argv, type, useUiThread),
  mInitialised(false),
  mPaused(false),
  mBundleName(""),
  mBundleId(""),
  mImpl(NULL)
{
  if(mArgc == nullptr || mArgv == nullptr)
  {
    mArgc = const_cast<int*>(&gTizenDummyArgc);
    mArgv = const_cast<char***>(reinterpret_cast<const char***>(&gTizenDummyArgv));
  }

  bool featureFlag = true;
  system_info_get_platform_bool("tizen.org/feature/opengles.version.2_0", &featureFlag);

  if(featureFlag == false)
  {
    set_last_result(TIZEN_ERROR_NOT_SUPPORTED);
  }

  mImpl = new Impl(this, type, useUiThread);
}

FrameworkTizen::~FrameworkTizen()
{
  if(mRunning)
  {
    Quit();
  }

  delete mImpl;
}

bool FrameworkTizen::Create()
{
  mInitialised = true;
  mObserver.OnInit();
  return true;
}

void FrameworkTizen::Run()
{
  mRunning = true;
  int ret;

  DALI_TRACE_BEGIN(gTraceFilter, "DALI_APPMAIN");
  ret = mImpl->AppMain();
  DALI_TRACE_END(gTraceFilter, "DALI_APPMAIN");
  if(ret != TIZEN_ERROR_NONE)
  {
    DALI_LOG_ERROR("Framework::Run(), ui_app_main() is failed. err = %d\n", ret);
  }
  mRunning = false;
}

void FrameworkTizen::Quit()
{
  mImpl->AppExit();
}

std::string FrameworkTizen::GetBundleName() const
{
  return mBundleName;
}

void FrameworkTizen::SetBundleName(const std::string& name)
{
  mBundleName = name;
}

std::string FrameworkTizen::GetBundleId() const
{
  return mBundleId;
}

void FrameworkTizen::SetBundleId(const std::string& id)
{
  mBundleId = id;
}

void FrameworkTizen::SetLanguage(const std::string& language)
{
  mImpl->SetLanguage(language);
}

void FrameworkTizen::SetRegion(const std::string& region)
{
  mImpl->SetRegion(region);
}

std::string FrameworkTizen::GetLanguage() const
{
  return mImpl->GetLanguage();
}

std::string FrameworkTizen::GetRegion() const
{
  return mImpl->GetRegion();
}

/**
 * Impl for Pre-Initailized using UI Thread.
 */
struct UIThreadLoader::Impl
{
  // Constructor
  Impl(void* data)
  {
#ifdef UI_THREAD_AVAILABLE
    mUIThreadLoader      = static_cast<UIThreadLoader*>(data);
    mAppCoreUiThreadBase = new AppCoreUiThreadBase();
#endif
  }

  // Destructor
  ~Impl()
  {
#ifdef UI_THREAD_AVAILABLE
    if(mAppCoreUiThreadBase)
    {
      mAppCoreUiThreadBase->Exit();
      delete mAppCoreUiThreadBase;
    }
#endif
  }

  /**
   * Runs to work create window in UI thread when application is pre-initialized.
   */
  void Run(Runner runner)
  {
#ifdef UI_THREAD_AVAILABLE
    mAppCoreUiThreadBase->Post(runner);
    mAppCoreUiThreadBase->Run(*(mUIThreadLoader->mArgc), *(mUIThreadLoader->mArgv));
#endif
  }

private:
  // Undefined
  Impl(const Impl& impl);
  Impl& operator=(const Impl& impl);

#ifdef UI_THREAD_AVAILABLE
  // Data
  AppCoreUiThreadBase* mAppCoreUiThreadBase;
  UIThreadLoader*      mUIThreadLoader;
#endif
};

/**
 * UI Thread loader to support Pre-Initailized using UI Thread.
 */
UIThreadLoader::UIThreadLoader(int* argc, char*** argv)
: mArgc(argc),
  mArgv(argv),
  mImpl(nullptr)
{
  if(mArgc == nullptr || mArgv == nullptr)
  {
    mArgc = const_cast<int*>(&gTizenDummyArgc);
    mArgv = const_cast<char***>(reinterpret_cast<const char***>(&gTizenDummyArgv));
  }

  mImpl = new Impl(this);
}

UIThreadLoader::~UIThreadLoader()
{
  if(mImpl)
  {
    delete mImpl;
  }
}

/**
 * Runs to work create window in UI thread when application is pre-initialized.
 */
void UIThreadLoader::Run(Runner runner)
{
  mImpl->Run(runner);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
