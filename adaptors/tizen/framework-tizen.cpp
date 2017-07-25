/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include "framework.h"

// EXTERNAL INCLUDES
#include <app.h>
#include <bundle.h>
#include <Ecore.h>

#include <system_info.h>
#include <app_control_internal.h>
#include <bundle_internal.h>

// CONDITIONAL INCLUDES
#ifdef APPCORE_WATCH_AVAILABLE
#include <appcore-watch/watch_app.h>
#endif
#ifdef DALI_ELDBUS_AVAILABLE
#include <Eldbus.h>
#endif // DALI_ELDBUS_AVAILABLE

#if defined( TIZEN_PLATFORM_CONFIG_SUPPORTED ) && TIZEN_PLATFORM_CONFIG_SUPPORTED
#include <tzplatform_config.h>
#endif // TIZEN_PLATFORM_CONFIG_SUPPORTED

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <callback-manager.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

#if defined(DEBUG_ENABLED)
namespace
{
Integration::Log::Filter* gDBusLogging = Integration::Log::Filter::New( Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_DBUS" );
} // anonymous namespace
#endif

/**
 * Impl to hide EFL data members
 */
struct Framework::Impl
{
// Constructor
  Impl(void* data, Type type )
  : mAbortCallBack( NULL ),
    mCallbackManager( NULL ),
    mEventCallback()
#ifdef APPCORE_WATCH_AVAILABLE
    , mWatchCallback()
#endif
  {
    mFramework = static_cast<Framework*>(data);

#ifndef APPCORE_WATCH_AVAILABLE
    if ( type == WATCH )
    {
      throw Dali::DaliException( "", "Watch Application is not supported." );
    }
#endif
    mApplicationType = type;
    mCallbackManager = CallbackManager::New();
  }

  ~Impl()
  {
    delete mAbortCallBack;

    // we're quiting the main loop so
    // mCallbackManager->RemoveAllCallBacks() does not need to be called
    // to delete our abort handler
    delete mCallbackManager;
  }

  int AppMain()
  {
    int ret;

    if (mApplicationType == NORMAL)
    {
      ret = AppNormalMain();
    }
    else
    {
      ret = AppWatchMain();
    }
    return ret;
  }

  void AppExit()
  {
    if (mApplicationType == NORMAL)
    {
      AppNormalExit();
    }
    else
    {
      AppWatchExit();
    }
  }


  // Data
  Type mApplicationType;
  CallbackBase* mAbortCallBack;
  CallbackManager *mCallbackManager;

  Framework* mFramework;
  app_event_handler_h handlers[5];
  ui_app_lifecycle_callback_s mEventCallback;
#ifdef APPCORE_WATCH_AVAILABLE
  watch_app_lifecycle_callback_s mWatchCallback;
#endif

  static bool AppCreate(void *data)
  {
    return static_cast<Framework*>(data)->Create();
  }

  static void AppTerminate(void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnTerminate();
  }

  static void AppPause(void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnPause();
  }

  static void AppResume(void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnResume();
  }

  static void ProcessBundle(Framework* framework, bundle *bundleData)
  {
    if(bundleData == NULL)
    {
      return;
    }

    // get bundle name
    char* bundleName = const_cast<char*>(bundle_get_val(bundleData, "name"));
    if(bundleName != NULL)
    {
      framework->SetBundleName(bundleName);
    }

    // get bundle id
    char* bundleId = const_cast<char*>(bundle_get_val(bundleData, "id"));
    if(bundleId != NULL)
    {
      framework->SetBundleId(bundleId);
    }
  }

  /**
   * Called by AppCore when the application is launched from another module (e.g. homescreen).
   * @param[in] b the bundle data which the launcher module sent
   */
  static void AppControl(app_control_h app_control, void *data)
  {
    Framework* framework = static_cast<Framework*>(data);
    Observer *observer = &framework->mObserver;
    bundle *bundleData = NULL;

    app_control_to_bundle(app_control, &bundleData);
    ProcessBundle(framework, bundleData);

    observer->OnReset();
    observer->OnAppControl(app_control);
  }

  int AppNormalMain()
  {
    int ret;

    mEventCallback.create = AppCreate;
    mEventCallback.terminate = AppTerminate;
    mEventCallback.pause = AppPause;
    mEventCallback.resume = AppResume;
    mEventCallback.app_control = AppControl;

    ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, AppBatteryLow, mFramework);
    ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, AppMemoryLow, mFramework);
    ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, AppDeviceRotated, mFramework);
    ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, AppLanguageChanged, mFramework);
    ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, AppRegionChanged, mFramework);

    ret = ui_app_main(*mFramework->mArgc, *mFramework->mArgv, &mEventCallback, mFramework);

    return ret;
  }

  void AppNormalExit()
  {
      ui_app_exit();
  }

#ifdef APPCORE_WATCH_AVAILABLE
  static bool AppCreateWatch(int width, int height, void *data)
  {
    return static_cast<Framework*>(data)->Create();
  }

  static void AppTimeTick(watch_time_h time, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;
    WatchTime curTime(time);

    observer->OnTimeTick(curTime);
  }

  static void AppAmbientTick(watch_time_h time, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;
    WatchTime curTime(time);

    observer->OnAmbientTick(curTime);
  }

  static void AppAmbientChanged(bool ambient, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnAmbientChanged(ambient);
  }
#endif

  int AppWatchMain()
  {
    int ret = true;

#ifdef APPCORE_WATCH_AVAILABLE
    mWatchCallback.create = AppCreateWatch;
    mWatchCallback.app_control = AppControl;
    mWatchCallback.terminate = AppTerminate;
    mWatchCallback.pause = AppPause;
    mWatchCallback.resume = AppResume;
    mWatchCallback.time_tick = AppTimeTick;
    mWatchCallback.ambient_tick = AppAmbientTick;
    mWatchCallback.ambient_changed = AppAmbientChanged;

    watch_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, AppBatteryLow, mFramework);
    watch_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, AppMemoryLow, mFramework);
    watch_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, AppLanguageChanged, mFramework);
    watch_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, AppRegionChanged, mFramework);

    ret = watch_app_main(*mFramework->mArgc, *mFramework->mArgv, &mWatchCallback, mFramework);
#endif
    return ret;
  }

  void AppWatchExit()
  {
#ifdef APPCORE_WATCH_AVAILABLE
    watch_app_exit();
#endif
  }

  static void AppLanguageChanged(app_event_info_h event, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnLanguageChanged();
  }

  static void AppDeviceRotated(app_event_info_h event_info, void *data)
  {
  }

  static void AppRegionChanged(app_event_info_h event, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnRegionChanged();
  }

  static void AppBatteryLow(app_event_info_h event, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnBatteryLow();
  }

  static void AppMemoryLow(app_event_info_h event, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnMemoryLow();
  }

private:
  // Undefined
  Impl( const Impl& impl );

  // Undefined
  Impl& operator=( const Impl& impl );
};

Framework::Framework( Framework::Observer& observer, int *argc, char ***argv, Type type )
: mObserver(observer),
  mInitialised(false),
  mRunning(false),
  mArgc(argc),
  mArgv(argv),
  mBundleName(""),
  mBundleId(""),
  mAbortHandler( MakeCallback( this, &Framework::AbortCallback ) ),
  mImpl(NULL)
{
  bool featureFlag = true;
  system_info_get_platform_bool( "tizen.org/feature/opengles.version.2_0", &featureFlag );

  if( featureFlag == false )
  {
    set_last_result( TIZEN_ERROR_NOT_SUPPORTED );
  }
#ifdef DALI_ELDBUS_AVAILABLE
  // Initialize ElDBus.
  DALI_LOG_INFO( gDBusLogging, Debug::General, "Starting DBus Initialization\n" );
  eldbus_init();
#endif
  InitThreads();

  mImpl = new Impl(this, type);
}

Framework::~Framework()
{
  if (mRunning)
  {
    Quit();
  }

#ifdef DALI_ELDBUS_AVAILABLE
  // Shutdown ELDBus.
  DALI_LOG_INFO( gDBusLogging, Debug::General, "Shutting down DBus\n" );
  eldbus_shutdown();
#endif

  delete mImpl;
}

bool Framework::Create()
{
  mInitialised = true;
  mObserver.OnInit();
  return true;
}

void Framework::Run()
{
  mRunning = true;
  int ret;

  ret = mImpl->AppMain();
  if (ret != APP_ERROR_NONE)
  {
    DALI_LOG_ERROR("Framework::Run(), ui_app_main() is failed. err = %d\n", ret);
  }
  mRunning = false;
}

void Framework::Quit()
{
  mImpl->AppExit();
}

bool Framework::IsMainLoopRunning()
{
  return mRunning;
}

void Framework::AddAbortCallback( CallbackBase* callback )
{
  mImpl->mAbortCallBack = callback;
}

std::string Framework::GetBundleName() const
{
  return mBundleName;
}

void Framework::SetBundleName(const std::string& name)
{
  mBundleName = name;
}

std::string Framework::GetBundleId() const
{
  return mBundleId;
}

std::string Framework::GetResourcePath()
{
  std::string resourcePath = "";
#if defined( TIZEN_PLATFORM_CONFIG_SUPPORTED ) && TIZEN_PLATFORM_CONFIG_SUPPORTED
  resourcePath = app_get_resource_path();
#else // For backwards compatibility with older Tizen versions

  // "DALI_APPLICATION_PACKAGE" is used to get the already configured Application package path.
  const char* environmentVariable = "DALI_APPLICATION_PACKAGE";
  char* value = getenv( environmentVariable );
  if ( value != NULL )
  {
    resourcePath = value;
  }
#endif //TIZEN_PLATFORM_CONFIG_SUPPORTED

  return resourcePath;
}

void Framework::SetBundleId(const std::string& id)
{
  mBundleId = id;
}

void Framework::AbortCallback( )
{
  // if an abort call back has been installed run it.
  if (mImpl->mAbortCallBack)
  {
    CallbackBase::Execute( *mImpl->mAbortCallBack );
  }
  else
  {
    Quit();
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
