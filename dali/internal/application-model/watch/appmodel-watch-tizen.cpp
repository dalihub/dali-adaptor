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
#include <dali/internal/application-model/watch/appmodel-watch-tizen.h>

// CONDITIONAL INCLUDES
#ifdef APPCORE_WATCH_AVAILABLE
#include <appcore-watch/watch_app.h>
#endif

// EXTERNAL INCLUDES
#include <app_common.h>
#include <app_control_internal.h>
#include <appcore_base.h>
#include <bundle.h>
#include <bundle_internal.h>
#include <dlog.h>
#include <tizen.h>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/internal/adaptor/common/framework.h>
#include <dali/internal/adaptor/tizen-wayland/framework-tizen.h>
#include <dali/public-api/watch/watch-time.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
extern "C" DALI_ADAPTOR_API AppModelWatch* Create()
{
  return new AppModelWatch;
}

extern "C" DALI_ADAPTOR_API void Destroy(void* p)
{
  AppModelWatch* appWatch = static_cast<AppModelWatch*>(p);
  delete appWatch;
}

extern "C" DALI_ADAPTOR_API int AppMain(bool isUiThread, void* data, void* pData)
{
  AppModelWatch* appWatch = static_cast<AppModelWatch*>(pData);
  int            ret      = 0;
  if(appWatch != nullptr)
  {
    ret = appWatch->AppMain(data);
  }
  else
  {
    print_log(DLOG_INFO, "DALI", "appWatch is nullptr");
  }
  return ret;
}

extern "C" DALI_ADAPTOR_API void AppExit(AppModelWatch* p)
{
  p->AppExit();
}

namespace AppCoreWatch
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
    [LOW_MEMORY]                 = APPCORE_BASE_EVENT_LOW_MEMORY,
    [LOW_BATTERY]                = APPCORE_BASE_EVENT_LOW_BATTERY,
    [LANGUAGE_CHANGED]           = APPCORE_BASE_EVENT_LANG_CHANGE,
    [DEVICE_ORIENTATION_CHANGED] = APPCORE_BASE_EVENT_DEVICE_ORIENTATION_CHANGED,
    [REGION_FORMAT_CHANGED]      = APPCORE_BASE_EVENT_REGION_CHANGE,
    [SUSPENDED_STATE_CHANGED]    = APPCORE_BASE_EVENT_SUSPENDED_STATE_CHANGE,
};

struct AppEventInfo
{
  AppEventType type;
  void*        value;
};

typedef struct AppEventInfo* AppEventInfoPtr;

typedef void (*AppEventCallback)(AppEventInfoPtr eventInfo, void* userData);

struct AppEventHandler
{
  AppEventType     type;
  AppEventCallback cb;
  void*            data;
  void*            raw;
};

typedef struct AppEventHandler* AppEventHandlerPtr;

int EventCallback(void* event, void* data)
{
  AppEventHandlerPtr handler = static_cast<AppEventHandlerPtr>(data);

  struct AppEventInfo appEvent;

  appEvent.type  = handler->type;
  appEvent.value = event;

  if(handler->cb)
    handler->cb(&appEvent, handler->data);

  return 0;
}

int AppAddEventHandler(AppEventHandlerPtr* eventHandler, AppEventType eventType, AppEventCallback callback, void* userData)
{
  AppEventHandlerPtr handler;

  handler = static_cast<AppEventHandlerPtr>(calloc(1, sizeof(struct AppEventHandler)));
  if(DALI_UNLIKELY(!handler))
  {
    DALI_LOG_ERROR("failed to create handler. calloc size : %zu\n", sizeof(struct AppEventHandler));
    return TIZEN_ERROR_UNKNOWN;
  }
  handler->type = eventType;
  handler->cb   = callback;
  handler->data = userData;
  handler->raw  = appcore_base_add_event(static_cast<appcore_base_event>(AppEventConverter[static_cast<int>(eventType)]), EventCallback, handler);

  *eventHandler = handler;

  return TIZEN_ERROR_NONE;
}
} // namespace AppCoreWatch
struct DALI_ADAPTOR_API AppModelWatch::Impl
{
#ifdef APPCORE_WATCH_AVAILABLE
  static bool WatchAppCreate(int width, int height, void* data)
  {
    return static_cast<FrameworkTizen*>(data)->Create();
  }

  static void WatchAppTimeTick(watch_time_h time, void* data)
  {
    Framework::Observer* observer = &static_cast<FrameworkTizen*>(data)->GetObserver();
    WatchTime            curTime(time);

    observer->OnTimeTick(curTime);
  }

  static void WatchAppAmbientTick(watch_time_h time, void* data)
  {
    Framework::Observer* observer = &static_cast<FrameworkTizen*>(data)->GetObserver();
    WatchTime            curTime(time);

    observer->OnAmbientTick(curTime);
  }

  static void WatchAppAmbientChanged(bool ambient, void* data)
  {
    Framework::Observer* observer = &static_cast<FrameworkTizen*>(data)->GetObserver();

    observer->OnAmbientChanged(ambient);
  }

  static void WatchAppControl(app_control_h app_control, void* data)
  {
    FrameworkTizen*      framework  = static_cast<FrameworkTizen*>(data);
    Framework::Observer* observer   = &framework->GetObserver();
    bundle*              bundleData = NULL;

    app_control_to_bundle(app_control, &bundleData);
    ProcessBundle(framework, bundleData);

    observer->OnReset();
    observer->OnAppControl(app_control);
  }

  static void WatchAppTerminate(void* data)
  {
    Framework::Observer* observer = &static_cast<FrameworkTizen*>(data)->GetObserver();

    observer->OnTerminate();
  }

  static void WatchAppPause(void* data)
  {
    Framework::Observer* observer = &static_cast<FrameworkTizen*>(data)->GetObserver();

    observer->OnPause();
  }

  static void WatchAppResume(void* data)
  {
    Framework::Observer* observer = &static_cast<FrameworkTizen*>(data)->GetObserver();

    observer->OnResume();
  }

  static void ProcessBundle(FrameworkTizen* framework, bundle* bundleData)
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
#endif

  static void AppLanguageChanged(AppCoreWatch::AppEventInfoPtr event, void* data)
  {
    FrameworkTizen*      framework = static_cast<FrameworkTizen*>(data);
    Framework::Observer* observer  = &framework->GetObserver();

    if(event && event->value)
    {
      framework->SetLanguage(std::string(static_cast<const char*>(event->value)));
      observer->OnLanguageChanged();
    }
    else
    {
      DALI_LOG_ERROR("NULL pointer in Language changed event\n");
    }
  }

  static void AppRegionChanged(AppCoreWatch::AppEventInfoPtr event, void* data)
  {
    FrameworkTizen*      framework = static_cast<FrameworkTizen*>(data);
    Framework::Observer* observer  = &framework->GetObserver();

    if(event && event->value)
    {
      framework->SetRegion(std::string(static_cast<const char*>(event->value)));
      observer->OnRegionChanged();
    }
    else
    {
      DALI_LOG_ERROR("NULL pointer in Region changed event\n");
    }
  }

  static void AppBatteryLow(AppCoreWatch::AppEventInfoPtr event, void* data)
  {
    Framework::Observer*                observer = &static_cast<FrameworkTizen*>(data)->GetObserver();
    int                                 status   = *static_cast<int*>(event->value);
    Dali::DeviceStatus::Battery::Status result   = Dali::DeviceStatus::Battery::Status::NORMAL;

    // convert to dali battery status
    switch(status)
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
      default:
        break;
    }
    observer->OnBatteryLow(result);
  }

  static void AppMemoryLow(AppCoreWatch::AppEventInfoPtr event, void* data)
  {
    Framework::Observer*               observer = &static_cast<FrameworkTizen*>(data)->GetObserver();
    int                                status   = *static_cast<int*>(event->value);
    Dali::DeviceStatus::Memory::Status result   = Dali::DeviceStatus::Memory::Status::NORMAL;

    // convert to dali memmory status
    switch(status)
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
      default:
        break;
    }
    observer->OnMemoryLow(result);
  }

  static void AppDeviceOrientationChanged(AppCoreWatch::AppEventInfoPtr event, void* data)
  {
    Framework::Observer*                    observer = &static_cast<FrameworkTizen*>(data)->GetObserver();
    int                                     status   = *static_cast<int*>(event->value);
    Dali::DeviceStatus::Orientation::Status result   = Dali::DeviceStatus::Orientation::Status::ORIENTATION_0;

    switch(status)
    {
      case APP_DEVICE_ORIENTATION_0:
      {
        result = Dali::DeviceStatus::Orientation::Status::ORIENTATION_0;
        break;
      }
      case APP_DEVICE_ORIENTATION_90:
      {
        result = Dali::DeviceStatus::Orientation::Status::ORIENTATION_90;
        break;
      }
      case APP_DEVICE_ORIENTATION_180:
      {
        result = Dali::DeviceStatus::Orientation::Status::ORIENTATION_180;
        break;
      }
      case APP_DEVICE_ORIENTATION_270:
      {
        result = Dali::DeviceStatus::Orientation::Status::ORIENTATION_270;
        break;
      }

      default:
        break;
    }
    observer->OnDeviceOrientationChanged(result);
  }

  int AppMain(void* data)
  {
    int ret = TIZEN_ERROR_NOT_SUPPORTED;

#ifdef APPCORE_WATCH_AVAILABLE
    FrameworkTizen* mFramework     = static_cast<FrameworkTizen*>(data);
    mWatchCallback.create          = WatchAppCreate;
    mWatchCallback.app_control     = WatchAppControl;
    mWatchCallback.terminate       = WatchAppTerminate;
    mWatchCallback.pause           = WatchAppPause;
    mWatchCallback.resume          = WatchAppResume;
    mWatchCallback.time_tick       = WatchAppTimeTick;
    mWatchCallback.ambient_tick    = WatchAppAmbientTick;
    mWatchCallback.ambient_changed = WatchAppAmbientChanged;

    AppCoreWatch::AppAddEventHandler(&handlers[AppCoreWatch::LOW_BATTERY], AppCoreWatch::LOW_BATTERY, AppBatteryLow, mFramework);
    AppCoreWatch::AppAddEventHandler(&handlers[AppCoreWatch::LOW_MEMORY], AppCoreWatch::LOW_MEMORY, AppMemoryLow, mFramework);
    AppCoreWatch::AppAddEventHandler(&handlers[AppCoreWatch::LANGUAGE_CHANGED], AppCoreWatch::LANGUAGE_CHANGED, AppLanguageChanged, mFramework);
    AppCoreWatch::AppAddEventHandler(&handlers[AppCoreWatch::REGION_FORMAT_CHANGED], AppCoreWatch::REGION_FORMAT_CHANGED, AppRegionChanged, mFramework);

    ret = watch_app_main(*mFramework->GetArgc(), *mFramework->GetArgv(), &mWatchCallback, mFramework);
#else
    DALI_LOG_ERROR("watch feature is not supported");
#endif
    return ret;
  }

  void AppExit()
  {
#ifdef APPCORE_WATCH_AVAILABLE
    watch_app_exit();
#endif
  }

  Impl(void* data)
  : handlers
  {
    nullptr, nullptr, nullptr, nullptr, nullptr
  }
#ifdef APPCORE_WATCH_AVAILABLE
  ,
    mWatchCallback()
#endif
  {
    mAppModelWatch = static_cast<AppModelWatch*>(data);
  }

  ~Impl()
  {
  }

  AppModelWatch*                   mAppModelWatch;
  AppCoreWatch::AppEventHandlerPtr handlers[5];
#ifdef APPCORE_WATCH_AVAILABLE
  watch_app_lifecycle_callback_s mWatchCallback;
#endif
};

AppModelWatch::AppModelWatch()
{
  mImpl = new Impl(this);
}

AppModelWatch::~AppModelWatch()
{
  delete mImpl;
}

int AppModelWatch::AppMain(void* data)
{
  return mImpl->AppMain(data);
}

void AppModelWatch::AppExit()
{
  mImpl->AppExit();
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
