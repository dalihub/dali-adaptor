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
#include <dali/internal/application-model/widget/appmodel-widget-tizen.h>

// EXTERNAL INCLUDES
#include <bundle.h>
#include <bundle_internal.h>
#include <dlog.h>
#include <glib.h>
#include <system_info.h>
#include <system_settings.h>
#include <tizen.h>
#include <widget_base.h>

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

#define DEBUG_PRINTF(fmt, arg...) LOGD(" " fmt, ##arg)

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
extern "C" DALI_ADAPTOR_API AppModelWidget* Create()
{
  return new AppModelWidget();
}

extern "C" DALI_ADAPTOR_API void Destroy(void* p)
{
  AppModelWidget* appWidget = static_cast<AppModelWidget*>(p);
  delete appWidget;
}

extern "C" DALI_ADAPTOR_API int AppMain(bool isUiThread, void* data, void* pData)
{
  AppModelWidget* appWidget = static_cast<AppModelWidget*>(pData);
  int             ret       = 0;
  if(appWidget != nullptr)
  {
    ret = appWidget->AppMain(data);
  }
  else
  {
    print_log(DLOG_INFO, "DALI", "appWidget is nullptr");
  }
  return ret;
}

extern "C" DALI_ADAPTOR_API void AppExit(AppModelWidget* p)
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

namespace AppCoreWidget
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
} // namespace AppCoreWidget

struct DALI_ADAPTOR_API AppModelWidget::Impl
{
  bool IsWidgetFeatureEnabled()
  {
    static bool feature   = false;
    static bool retrieved = false;
    int         ret;

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

  void AppExit()
  {
    widget_base_exit();
  }

  static int WidgetAppCreate(void* data)
  {
    widget_base_on_create();
    return static_cast<int>(static_cast<FrameworkTizen*>(data)->Create());
  }

  static int WidgetAppTerminate(void* data)
  {
    Framework::Observer* observer = &static_cast<FrameworkTizen*>(data)->GetObserver();
    observer->OnTerminate();

    widget_base_on_terminate();
    return 0;
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

  static void AppFinish(void)
  {
    ecore_shutdown();

    if(Dali::EnvironmentVariable::GetEnvironmentVariable(AUL_LOADER_INIT_ENV))
    {
      Dali::EnvironmentVariable::SetEnvironmentVariable(AUL_LOADER_INIT_ENV, AUL_LOADER_INIT_DEFAULT_VALUE);
      ecore_shutdown();
    }

#ifdef DALI_ELDBUS_AVAILABLE
    // Shutdown ELDBus.
    DALI_LOG_INFO(gDBusLogging, Debug::General, "Shutting down DBus\n");
    eldbus_shutdown();
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

  static void AppLanguageChanged(AppCoreWidget::AppEventInfoPtr event, void* data)
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

  static void AppRegionChanged(AppCoreWidget::AppEventInfoPtr event, void* data)
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

  static void AppBatteryLow(AppCoreWidget::AppEventInfoPtr event, void* data)
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

  static void AppMemoryLow(AppCoreWidget::AppEventInfoPtr event, void* data)
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

  static void AppDeviceOrientationChanged(AppCoreWidget::AppEventInfoPtr event, void* data)
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
    print_log(DLOG_INFO, "DALI", "AppModelWidget AppMain 3");
    if(!IsWidgetFeatureEnabled())
    {
      DALI_LOG_ERROR("widget feature is not supported");
      return TIZEN_ERROR_NOT_SUPPORTED;
    }
    FrameworkTizen* mFramework = static_cast<FrameworkTizen*>(data);

    AppCoreWidget::AppAddEventHandler(&handlers[AppCoreWidget::LOW_BATTERY], AppCoreWidget::LOW_BATTERY, AppBatteryLow, mFramework);
    AppCoreWidget::AppAddEventHandler(&handlers[AppCoreWidget::LOW_MEMORY], AppCoreWidget::LOW_MEMORY, AppMemoryLow, mFramework);
    AppCoreWidget::AppAddEventHandler(&handlers[AppCoreWidget::DEVICE_ORIENTATION_CHANGED], AppCoreWidget::DEVICE_ORIENTATION_CHANGED, AppDeviceOrientationChanged, mFramework);
    AppCoreWidget::AppAddEventHandler(&handlers[AppCoreWidget::LANGUAGE_CHANGED], AppCoreWidget::LANGUAGE_CHANGED, AppLanguageChanged, mFramework);
    AppCoreWidget::AppAddEventHandler(&handlers[AppCoreWidget::REGION_FORMAT_CHANGED], AppCoreWidget::REGION_FORMAT_CHANGED, AppRegionChanged, mFramework);

    widget_base_ops ops = widget_base_get_default_ops();
    print_log(DLOG_INFO, "DALI", "AppModelWidget AppMain 4");
    /* override methods */
    ops.create    = WidgetAppCreate;
    ops.terminate = WidgetAppTerminate;
    ops.init      = AppInit;
    ops.finish    = AppFinish;
    ops.run       = AppRun;
    ops.exit      = AppExit;

    print_log(DLOG_INFO, "DALI", "AppModelWidget AppMain 5");
    int result = widget_base_init(ops, *mFramework->GetArgc(), *mFramework->GetArgv(), mFramework);

    widget_base_fini();
    print_log(DLOG_INFO, "DALI", "AppModelWidget AppMain 6");
    return result;
  }

  Impl(void* data)
  : handlers{nullptr, nullptr, nullptr, nullptr, nullptr}
  {
    mAppModelWidget = static_cast<AppModelWidget*>(data);
  }

  ~Impl()
  {
  }

  AppModelWidget*                   mAppModelWidget;
  AppCoreWidget::AppEventHandlerPtr handlers[5];
}; // Impl

AppModelWidget::AppModelWidget()
{
  mImpl = new Impl(this);
}

AppModelWidget::~AppModelWidget()
{
  delete mImpl;
}

int AppModelWidget::AppMain(void* data)
{
  return mImpl->AppMain(data);
}

void AppModelWidget::AppExit()
{
  mImpl->AppExit();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
