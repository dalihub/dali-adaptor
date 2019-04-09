/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/adaptor/common/framework.h>

// EXTERNAL INCLUDES
#include <appcore_ui_base.h>
#include <app_control_internal.h>
#include <app_common.h>
#include <bundle.h>
#include <dali/internal/system/linux/dali-ecore.h>

#include <system_info.h>
#include <system_settings.h>
#include <bundle_internal.h>
#include <widget_base.h>
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
#include <dali/internal/system/common/callback-manager.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
#if defined(DEBUG_ENABLED)
Integration::Log::Filter* gDBusLogging = Integration::Log::Filter::New( Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_DBUS" );
#endif

bool IsWidgetFeatureEnabled()
{
  static bool feature = false;
  static bool retrieved = false;
  int ret;

  if(retrieved == true)
  {
    return feature;
  }

  ret = system_info_get_platform_bool("http://tizen.org/feature/shell.appwidget", &feature);
  if(ret != SYSTEM_INFO_ERROR_NONE)
  {
    DALI_LOG_ERROR("failed to get system info");
    return false;
  }

  retrieved = true;
  return feature;
}

} // anonymous namespace

namespace AppCore
{

typedef enum
{
  LOW_MEMORY,                 //< The low memory event
  LOW_BATTERY,                //< The low battery event
  LANGUAGE_CHANGED,           //< The system language changed event
  DEVICE_ORIENTATION_CHANGED, //< The device orientation changed event
  REGION_FORMAT_CHANGED,      //< The region format changed event
  SUSPENDED_STATE_CHANGED,    //< The suspended state changed event of the application
  UPDATE_REQUESTED,           //< The update requested event. This event can occur when an app needs to be updated. It is dependent on target devices.
} AppEventType;

static int AppEventConverter[APPCORE_BASE_EVENT_MAX] =
{
  [LOW_MEMORY] = APPCORE_BASE_EVENT_LOW_MEMORY,
  [LOW_BATTERY] = APPCORE_BASE_EVENT_LOW_BATTERY,
  [LANGUAGE_CHANGED] = APPCORE_BASE_EVENT_LANG_CHANGE,
  [DEVICE_ORIENTATION_CHANGED] = APPCORE_BASE_EVENT_DEVICE_ORIENTATION_CHANGED,
  [REGION_FORMAT_CHANGED] = APPCORE_BASE_EVENT_REGION_CHANGE,
  [SUSPENDED_STATE_CHANGED] = APPCORE_BASE_EVENT_SUSPENDED_STATE_CHANGE,
};

struct AppEventInfo
{
  AppEventType type;
  void *value;
};

typedef struct AppEventInfo *AppEventInfoPtr;

typedef void (*AppEventCallback)(AppEventInfoPtr eventInfo, void *userData);

struct AppEventHandler
{
  AppEventType type;
  AppEventCallback cb;
  void *data;
  void *raw;
};

typedef struct AppEventHandler *AppEventHandlerPtr;

int EventCallback(void *event, void *data)
{
  AppEventHandlerPtr handler = static_cast<AppEventHandlerPtr>(data);

  struct AppEventInfo appEvent;

  appEvent.type = handler->type;
  appEvent.value = event;

  if (handler->cb)
    handler->cb(&appEvent, handler->data);

  return 0;
}

int AppAddEventHandler(AppEventHandlerPtr *eventHandler, AppEventType eventType, AppEventCallback callback, void *userData)
{
  AppEventHandlerPtr handler;

  handler = static_cast<AppEventHandlerPtr>( calloc(1, sizeof(struct AppEventHandler)) );
  if (!handler)
  {
    DALI_LOG_ERROR( "failed to create handler" );
    return TIZEN_ERROR_UNKNOWN;
  }
  else
  {
    handler->type = eventType;
    handler->cb = callback;
    handler->data = userData;
    handler->raw = appcore_base_add_event( static_cast<appcore_base_event>(AppEventConverter[static_cast<int>(eventType)]), EventCallback, handler);

    *eventHandler = handler;

    return TIZEN_ERROR_NONE;
  }
}

} // namespace Appcore

/**
 * Impl to hide EFL data members
 */
struct Framework::Impl
{
// Constructor
  Impl(void* data, Type type )
  : mAbortCallBack( NULL ),
    mCallbackManager( NULL )
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

    char* region = nullptr;
    char* language = nullptr;
    system_settings_get_value_string( SYSTEM_SETTINGS_KEY_LOCALE_COUNTRY, &region );
    system_settings_get_value_string( SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &language );

    if (region != nullptr)
    {
      mRegion = std::string( region );
    }

    if ( language != nullptr)
    {
      mLanguage = std::string( language );
    }
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
    else if(mApplicationType == WIDGET)
    {
      ret = AppWidgetMain();
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
    else if(mApplicationType == WIDGET)
    {
      AppWidgetExit();
    }
    else
    {
      AppWatchExit();
    }
  }

  void SetLanguage( const std::string& language )
  {
    mLanguage = language;
  }

  void SetRegion( const std::string& region )
  {
    mRegion = region;
  }

  std::string GetLanguage() const
  {
    return mLanguage;
  }

  std::string GetRegion() const
  {
    return mRegion;
  }

  // Data
  Type mApplicationType;
  CallbackBase* mAbortCallBack;
  CallbackManager *mCallbackManager;
  std::string mLanguage;
  std::string mRegion;

  Framework* mFramework;
  AppCore::AppEventHandlerPtr handlers[5];
#ifdef APPCORE_WATCH_AVAILABLE
  watch_app_lifecycle_callback_s mWatchCallback;
  app_event_handler_h watchHandlers[5];
#endif

  static int AppCreate(void *data)
  {
    appcore_ui_base_on_create();
    return static_cast<int>( static_cast<Framework*>(data)->Create() );
  }

  static int AppTerminate(void *data)
  {
    appcore_ui_base_on_terminate();
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnTerminate();

    return 0;
  }

  static int AppPause(void *data)
  {
    appcore_ui_base_on_pause();
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnPause();

    return 0;
  }

  static int AppResume(void *data)
  {
    appcore_ui_base_on_resume();
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnResume();

    return 0;
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

    // get bundle? id
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
  static int AppControl(bundle* bundleData, void *data)
  {
    app_control_h appControl = NULL;

    appcore_ui_base_on_control(bundleData);

    if (bundleData)
    {
      if (app_control_create_event(bundleData, &appControl) != TIZEN_ERROR_NONE)
      {
        DALI_LOG_ERROR("Failed to create an app_control handle");
      }
    }
    else
    {
      if (app_control_create(&appControl) != TIZEN_ERROR_NONE)
      {
        DALI_LOG_ERROR("Failed to create an app_control handle");
      }
    }

    Framework* framework = static_cast<Framework*>(data);
    Observer *observer = &framework->mObserver;

    ProcessBundle(framework, bundleData);

    observer->OnReset();
    observer->OnAppControl(appControl);

    app_control_destroy(appControl);

    return 0;
  }

  static void AppInit(int argc, char **argv, void *data)
  {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

    ecore_init();
    ecore_app_args_set( argc, (const char **)argv );

#pragma GCC diagnostic pop
  }

  static void AppFinish(void)
  {
    ecore_shutdown();

    if(getenv("AUL_LOADER_INIT"))
    {
      unsetenv("AUL_LOADER_INIT");
      ecore_shutdown();
    }
  }

  static void AppRun(void *data)
  {
    ecore_main_loop_begin();
  }

  static void AppExit(void *data)
  {
    ecore_main_loop_quit();
  }

  static void AppLanguageChanged(AppCore::AppEventInfoPtr event, void *data)
  {
    Framework* framework = static_cast<Framework*>(data);
    Observer *observer = &framework->mObserver;

    if( event && event->value )
    {
      framework->SetLanguage( std::string( static_cast<const char *>(event->value) ) );
      observer->OnLanguageChanged();
    }
    else
    {
      DALI_LOG_ERROR( "NULL pointer in Language changed event\n" );
    }
  }

  static void AppDeviceRotated(AppCore::AppEventInfoPtr event_info, void *data)
  {
  }

  static void AppRegionChanged(AppCore::AppEventInfoPtr event, void *data)
  {
    Framework* framework = static_cast<Framework*>(data);
    Observer *observer = &framework->mObserver;

    if( event && event->value )
    {
      framework->SetRegion( std::string( static_cast<const char *>(event->value) ) );
      observer->OnRegionChanged();
    }
    else
    {
      DALI_LOG_ERROR( "NULL pointer in Region changed event\n" );
    }
  }

  static void AppBatteryLow(AppCore::AppEventInfoPtr event, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;
    int status = *static_cast<int *>(event->value);
    Dali::DeviceStatus::Battery::Status result = Dali::DeviceStatus::Battery::NORMAL;

    // convert to dali battery status
    switch( status )
    {
      case 1:
      {
        result = Dali::DeviceStatus::Battery::POWER_OFF;
        break;
      }
      case 2:
      {
        result = Dali::DeviceStatus::Battery::CRITICALLY_LOW;
        break;
      }
      default :
        break;
    }
    observer->OnBatteryLow(result);
  }

  static void AppMemoryLow(AppCore::AppEventInfoPtr event, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;
    int status = *static_cast<int *>(event->value);
    Dali::DeviceStatus::Memory::Status result = Dali::DeviceStatus::Memory::NORMAL;

    // convert to dali memmory status
    switch( status )
    {
      case 1:
      {
        result = Dali::DeviceStatus::Memory::NORMAL;
        break;
      }
      case 2:
      {
        result = Dali::DeviceStatus::Memory::LOW;
        break;
      }
      case 4:
      {
        result = Dali::DeviceStatus::Memory::CRITICALLY_LOW;
        break;
      }
      default :
        break;
    }
    observer->OnMemoryLow(result);
  }


  int AppNormalMain()
  {
    int ret;

    AppCore::AppAddEventHandler(&handlers[AppCore::LOW_BATTERY], AppCore::LOW_BATTERY, AppBatteryLow, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::LOW_MEMORY], AppCore::LOW_MEMORY, AppMemoryLow, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::DEVICE_ORIENTATION_CHANGED], AppCore::DEVICE_ORIENTATION_CHANGED, AppDeviceRotated, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::LANGUAGE_CHANGED], AppCore::LANGUAGE_CHANGED, AppLanguageChanged, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::REGION_FORMAT_CHANGED], AppCore::REGION_FORMAT_CHANGED, AppRegionChanged, mFramework);

    appcore_ui_base_ops ops = appcore_ui_base_get_default_ops();

    /* override methods */
    ops.base.create = AppCreate;
    ops.base.control = AppControl;
    ops.base.terminate = AppTerminate;
    ops.pause = AppPause;
    ops.resume = AppResume;
    ops.base.init = AppInit;
    ops.base.finish = AppFinish;
    ops.base.run = AppRun;
    ops.base.exit = AppExit;

    ret = appcore_ui_base_init(ops, *mFramework->mArgc, *mFramework->mArgv, mFramework, APPCORE_UI_BASE_HINT_WINDOW_GROUP_CONTROL |
                                                                                        APPCORE_UI_BASE_HINT_WINDOW_STACK_CONTROL |
                                                                                        APPCORE_UI_BASE_HINT_BG_LAUNCH_CONTROL |
                                                                                        APPCORE_UI_BASE_HINT_HW_ACC_CONTROL |
                                                                                        APPCORE_UI_BASE_HINT_WINDOW_AUTO_CONTROL );

    if (ret != TIZEN_ERROR_NONE)
      return ret;

    appcore_ui_base_fini();

    return TIZEN_ERROR_NONE;
  }

  void AppNormalExit()
  {
    appcore_ui_base_exit();
  }

  void AppWidgetExit()
  {
    widget_base_exit();
  }

  static int WidgetAppCreate( void *data )
  {
    widget_base_on_create();
    return static_cast<int>( static_cast<Framework*>(data)->Create() );
  }

  static int WidgetAppTerminate( void *data )
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;
    observer->OnTerminate();

    widget_base_on_terminate();
    return 0;
  }

  int AppWidgetMain()
  {
    if( !IsWidgetFeatureEnabled() )
    {
      DALI_LOG_ERROR("widget feature is not supported");
      return 0;
    }

    AppCore::AppAddEventHandler(&handlers[AppCore::LOW_BATTERY], AppCore::LOW_BATTERY, AppBatteryLow, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::LOW_MEMORY], AppCore::LOW_MEMORY, AppMemoryLow, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::DEVICE_ORIENTATION_CHANGED], AppCore::DEVICE_ORIENTATION_CHANGED, AppDeviceRotated, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::LANGUAGE_CHANGED], AppCore::LANGUAGE_CHANGED, AppLanguageChanged, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::REGION_FORMAT_CHANGED], AppCore::REGION_FORMAT_CHANGED, AppRegionChanged, mFramework);

    widget_base_ops ops = widget_base_get_default_ops();

    /* override methods */
    ops.create = WidgetAppCreate;
    ops.terminate = WidgetAppTerminate;
    ops.init = AppInit;
    ops.finish = AppFinish;
    ops.run = AppRun;
    ops.exit = AppExit;

    int result = widget_base_init(ops, *mFramework->mArgc, *mFramework->mArgv, mFramework);

    widget_base_fini();

    return result;
  }

#ifdef APPCORE_WATCH_AVAILABLE
  static bool WatchAppCreate(int width, int height, void *data)
  {
    return static_cast<Framework*>(data)->Create();
  }

  static void WatchAppTimeTick(watch_time_h time, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;
    WatchTime curTime(time);

    observer->OnTimeTick(curTime);
  }

  static void WatchAppAmbientTick(watch_time_h time, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;
    WatchTime curTime(time);

    observer->OnAmbientTick(curTime);
  }

  static void WatchAppAmbientChanged(bool ambient, void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnAmbientChanged(ambient);
  }

  static void WatchAppControl(app_control_h app_control, void *data)
  {
    Framework* framework = static_cast<Framework*>(data);
    Observer *observer = &framework->mObserver;
    bundle *bundleData = NULL;

    app_control_to_bundle(app_control, &bundleData);
    ProcessBundle(framework, bundleData);

    observer->OnReset();
    observer->OnAppControl(app_control);
  }

  static void WatchAppTerminate(void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnTerminate();
  }

  static void WatchAppPause(void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnPause();
  }

  static void WatchAppResume(void *data)
  {
    Observer *observer = &static_cast<Framework*>(data)->mObserver;

    observer->OnResume();
  }

#endif

  int AppWatchMain()
  {
    int ret = true;

#ifdef APPCORE_WATCH_AVAILABLE
    mWatchCallback.create = WatchAppCreate;
    mWatchCallback.app_control = WatchAppControl;
    mWatchCallback.terminate = WatchAppTerminate;
    mWatchCallback.pause = WatchAppPause;
    mWatchCallback.resume = WatchAppResume;
    mWatchCallback.time_tick = WatchAppTimeTick;
    mWatchCallback.ambient_tick = WatchAppAmbientTick;
    mWatchCallback.ambient_changed = WatchAppAmbientChanged;

    AppCore::AppAddEventHandler(&handlers[AppCore::LOW_BATTERY], AppCore::LOW_BATTERY, AppBatteryLow, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::LOW_MEMORY], AppCore::LOW_MEMORY, AppMemoryLow, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::LANGUAGE_CHANGED], AppCore::LANGUAGE_CHANGED, AppLanguageChanged, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::REGION_FORMAT_CHANGED], AppCore::REGION_FORMAT_CHANGED, AppRegionChanged, mFramework);

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
  char* app_rsc_path = app_get_resource_path();
  if (app_rsc_path)
  {
    resourcePath = app_rsc_path;
    free(app_rsc_path);
  }
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

std::string Framework::GetDataPath()
{
  std::string result;
  char* dataPath = app_get_data_path();
  if( dataPath )
  {
    result = dataPath;
    free(dataPath);
  }
  return result;
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

void Framework::InitThreads()
{
}

void Framework::SetLanguage( const std::string& language )
{
  mImpl->SetLanguage( language );
}

void Framework::SetRegion( const std::string& region )
{
  mImpl->SetRegion( region );
}

std::string Framework::GetLanguage() const
{
  return mImpl->GetLanguage();
}

std::string Framework::GetRegion() const
{
  return mImpl->GetRegion();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
