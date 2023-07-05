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
#include <app_common.h>
#include <app_control_internal.h>
#include <bundle.h>
#include <bundle_internal.h>
#include <dlog.h>
#include <glib.h>
#include <system_info.h>
#include <system_settings.h>
#include <widget_base.h>
#include <app_core_ui_base.hh>
#include <app_event_internal.hh>
#include <app_core_ui_thread_base.hh>

// CONDITIONAL INCLUDES
#ifdef APPCORE_WATCH_AVAILABLE
#include <appcore-watch/watch_app.h>
#endif
#ifdef DALI_ELDBUS_AVAILABLE
#include <Eldbus.h>
#endif // DALI_ELDBUS_AVAILABLE

#ifdef COMPONENT_APPLICATION_SUPPORT
#include <component_based_app_base.h>
#endif

#include <dali/integration-api/debug.h>
#include <dali/integration-api/trace.h>

// INTERNAL INCLUDES
#include <dali/internal/system/linux/dali-ecore.h>

using namespace tizen_cpp;

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
DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_FRAMEWORK, true);

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

// Note : tizen appfw don't consider zero-arguments case.
// If framework argc & argv is nullptr, We should add at least one argv.
const int   gTizenDummyArgc    = 1;
const char* gTizenDummyArgv[1] = {"dali-tizen-app"};

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
  if(!handler)
  {
    DALI_LOG_ERROR("failed to create handler");
    return TIZEN_ERROR_UNKNOWN;
  }
  else
  {
    handler->type = eventType;
    handler->cb   = callback;
    handler->data = userData;
    handler->raw  = appcore_base_add_event(static_cast<appcore_base_event>(AppEventConverter[static_cast<int>(eventType)]), EventCallback, handler);

    *eventHandler = handler;

    return TIZEN_ERROR_NONE;
  }
}

DeviceStatus::Memory::Status GetMemoryStatus(app_event_low_memory_status_e memoryStatus)
{
  switch(memoryStatus)
  {
    case APP_EVENT_LOW_MEMORY_SOFT_WARNING: // 0x02
    {
      return Dali::DeviceStatus::Memory::Status::LOW;
    }
    case APP_EVENT_LOW_MEMORY_HARD_WARNING: // 0x04
    {
      return Dali::DeviceStatus::Memory::Status::CRITICALLY_LOW;
    }
    default: // APP_EVENT_LOW_MEMORY_NORMAL 0x01
    {
      return Dali::DeviceStatus::Memory::Status::NORMAL;
    }
  }
}

DeviceStatus::Battery::Status GetBatteryStatus(app_event_low_battery_status_e batteryStatus)
{
  switch(batteryStatus)
  {
    case APP_EVENT_LOW_BATTERY_POWER_OFF: // 1
    {
      return Dali::DeviceStatus::Battery::Status::POWER_OFF;
    }
    case APP_EVENT_LOW_BATTERY_CRITICAL_LOW: // 2
    {
      return Dali::DeviceStatus::Battery::Status::CRITICALLY_LOW;
    }
    default:
    {
      return Dali::DeviceStatus::Battery::Status::NORMAL;
    }
  }
}

DeviceStatus::Orientation::Status GetOrientationStatus(app_device_orientation_e orientationStatus)
{
  switch(orientationStatus)
  {
    case APP_DEVICE_ORIENTATION_0:
    {
      return Dali::DeviceStatus::Orientation::Status::ORIENTATION_0;
    }
    case APP_DEVICE_ORIENTATION_90:
    {
      return Dali::DeviceStatus::Orientation::Status::ORIENTATION_90;
    }
    case APP_DEVICE_ORIENTATION_180:
    {
      return Dali::DeviceStatus::Orientation::Status::ORIENTATION_180;
    }
    case APP_DEVICE_ORIENTATION_270:
    {
      return Dali::DeviceStatus::Orientation::Status::ORIENTATION_270;
    }
    default:
    {
      return Dali::DeviceStatus::Orientation::Status::ORIENTATION_0;
    }
  }
}

} // namespace AppCore

/**
 * Impl to hide EFL data members
 */
struct FrameworkTizen::Impl
{
  class UiAppContext : public AppCoreUiBase
  {
  public:
    class Task : public AppCoreTaskBase
    {
    public:
      explicit Task(FrameworkTizen* framework)
      : mFramework(framework),
        mNewBatteryStatus(Dali::DeviceStatus::Battery::Status::NORMAL),
        mNewMemoryStatus(Dali::DeviceStatus::Memory::NORMAL),
        mNewDeviceOrientationStatus(Dali::DeviceStatus::Orientation::ORIENTATION_0)
      {
      }

      virtual ~Task()
      {
      }

      int OnCreate() override
      {
        // On the main thread, the log functions are not set. So print_log() is used directly.
        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > OnCreate() emitted", __MODULE__, __func__, __LINE__);
        mFramework->mTaskObserver.OnTaskInit();
        return AppCoreTaskBase::OnCreate();
      }

      int OnTerminate() override
      {
        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > OnTerminate() emitted", __MODULE__, __func__, __LINE__);
        mFramework->mTaskObserver.OnTaskTerminate();
        return AppCoreTaskBase::OnTerminate();
      }

      int OnControl(tizen_base::Bundle b) override
      {
        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > OnControl() emitted", __MODULE__, __func__, __LINE__);
        AppCoreTaskBase::OnControl(b);

        app_control_h appControl = nullptr;

        auto* bundleData = b.GetHandle();
        if(bundleData)
        {
          if(app_control_create_event(bundleData, &appControl) != TIZEN_ERROR_NONE)
          {
            print_log(DLOG_ERROR, "DALI", "%s: %s(%d) > Failed to create an app_control handle with Bundle", __MODULE__, __func__, __LINE__);
          }
        }
        else
        {
          if(app_control_create(&appControl) != TIZEN_ERROR_NONE)
          {
            print_log(DLOG_ERROR, "DALI", "%s: %s(%d) > Failed to create an app_control handle", __MODULE__, __func__, __LINE__);
          }
        }
        mFramework->mTaskObserver.OnTaskAppControl(appControl);

        app_control_destroy(appControl);
        return 0;
      }

      void OnUiEvent(AppCoreTaskBase::UiState state) override
      {
        // This event is emitted when the UI thread is paused or resumed.
        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > OnUiEvent() emitted", __MODULE__, __func__, __LINE__);

        // Note: This isn't implemented.
        AppCoreTaskBase::OnUiEvent(state);
      }

      void OnLowMemory(AppCoreTaskBase::LowMemoryState state) override
      {
        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > OnLowMemory() emitted", __MODULE__, __func__, __LINE__);

        mNewMemoryStatus = AppCore::GetMemoryStatus(static_cast<app_event_low_memory_status_e>(state));

        PostToUiThread(
          [](gpointer userData) -> gboolean {
            auto* task      = static_cast<Task*>(userData);
            auto* framework = static_cast<FrameworkTizen*>(task->mFramework);
            framework->mObserver.OnMemoryLow(task->mNewMemoryStatus);
            return G_SOURCE_REMOVE;
          });
        mFramework->mTaskObserver.OnTaskMemoryLow(mNewMemoryStatus);
        AppCoreTaskBase::OnLowMemory(state);
      }

      void OnDeviceOrientationChanged(AppCoreTaskBase::DeviceOrientationState state) override
      {
        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > OnDeviceOrientationChanged() emitted, orientation :%d", __MODULE__, __func__, __LINE__, state);

        mNewDeviceOrientationStatus = AppCore::GetOrientationStatus(static_cast<app_device_orientation_e>(state));

        PostToUiThread(
          [](gpointer userData) -> gboolean {
            auto* task      = static_cast<Task*>(userData);
            auto* framework = static_cast<FrameworkTizen*>(task->mFramework);
            framework->mObserver.OnDeviceOrientationChanged(task->mNewDeviceOrientationStatus);
            return G_SOURCE_REMOVE;
          });

        mFramework->mTaskObserver.OnTaskDeviceOrientationChanged(mNewDeviceOrientationStatus);

        AppCoreTaskBase::OnDeviceOrientationChanged(state);
      }
      void OnLowBattery(AppCoreTaskBase::LowBatteryState state) override
      {
        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > OnLowBattery() emitted", __MODULE__, __func__, __LINE__);
        mNewBatteryStatus = AppCore::GetBatteryStatus(static_cast<app_event_low_battery_status_e>(state));

        PostToUiThread(
          [](gpointer userData) -> gboolean {
            auto* task      = static_cast<Task*>(userData);
            auto* framework = static_cast<FrameworkTizen*>(task->mFramework);
            framework->mObserver.OnBatteryLow(task->mNewBatteryStatus);
            return G_SOURCE_REMOVE;
          });
        mFramework->mTaskObserver.OnTaskBatteryLow(mNewBatteryStatus);
        AppCoreTaskBase::OnLowBattery(state);
      }

      void OnLangChanged(const std::string& lang) override
      {
        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > OnLangChanged() emitted", __MODULE__, __func__, __LINE__);
        mNewLanguage = lang;
        mFramework->SetLanguage(mNewLanguage);

        PostToUiThread(
          [](gpointer userData) -> gboolean {
            auto* task      = static_cast<Task*>(userData);
            auto* framework = static_cast<FrameworkTizen*>(task->mFramework);
            framework->mObserver.OnLanguageChanged();
            return G_SOURCE_REMOVE;
          });

        mFramework->mTaskObserver.OnTaskLanguageChanged();
        AppCoreTaskBase::OnLangChanged(lang);
      }

      void OnRegionChanged(const std::string& region) override
      {
        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > nRegionChanged() emitted", __MODULE__, __func__, __LINE__);
        mNewRegion = region;
        mFramework->SetRegion(mNewRegion);

        PostToUiThread(
          [](gpointer userData) -> gboolean {
            auto* task      = static_cast<Task*>(userData);
            auto* framework = static_cast<FrameworkTizen*>(task->mFramework);
            framework->mObserver.OnRegionChanged();
            return G_SOURCE_REMOVE;
          });

        mFramework->mTaskObserver.OnTaskRegionChanged();
        AppCoreTaskBase::OnRegionChanged(mNewRegion);
      }

    private:
      GMainContext* GetTizenGlibContext()
      {
        GMainContext* context;
        const char*   env = getenv("TIZEN_GLIB_CONTEXT");
        if(env)
        {
          context = (GMainContext*)strtoul(env, nullptr, 10);
        }
        else
        {
          context = nullptr;
        }

        return context;
      }

      void PostToUiThread(GSourceFunc func)
      {
        GSource* source = g_idle_source_new();
        g_source_set_callback(source, func, this, nullptr);
        g_source_attach(source, GetTizenGlibContext());
        g_source_unref(source);
      }

    private:
      FrameworkTizen*                         mFramework;
      std::string                             mNewLanguage;
      std::string                             mNewRegion;
      Dali::DeviceStatus::Battery::Status     mNewBatteryStatus;
      Dali::DeviceStatus::Memory::Status      mNewMemoryStatus;
      Dali::DeviceStatus::Orientation::Status mNewDeviceOrientationStatus;
    };

    explicit UiAppContext(unsigned int hint, FrameworkTizen* framework)
    : AppCoreUiBase(hint),
      mFramework(framework),
      mUseUiThread(false)
    {
      if(hint & AppCoreUiBase::HINT_DUAL_THREAD)
      {
        mUseUiThread = true;
      }

      if(!mUseUiThread)
      {
        mLanguageChanged = std::make_shared<AppEvent>(IAppCore::IEvent::Type::LANG_CHANGE, OnLanguageChanged, this);
        AddEvent(mLanguageChanged);

        mDeviceOrientationChanged = std::make_shared<AppEvent>(IAppCore::IEvent::Type::DEVICE_ORIENTATION_CHANGED, OnDeviceOrientationChanged, this);
        AddEvent(mDeviceOrientationChanged);

        mRegionFormatChanged = std::make_shared<AppEvent>(IAppCore::IEvent::Type::REGION_CHANGE, OnRegionFormatChanged, this);
        AddEvent(mRegionFormatChanged);

        mLowMemory = std::make_shared<AppEvent>(IAppCore::IEvent::Type::LOW_MEMORY, OnLowMemory, this);
        AddEvent(mLowMemory);

        mLowBattery = std::make_shared<AppEvent>(IAppCore::IEvent::Type::LOW_BATTERY, OnLowBattery, this);
        AddEvent(mLowBattery);
      }
    }

    virtual ~UiAppContext()
    {
      if(!mUseUiThread)
      {
        RemoveEvent(mLowBattery);
        RemoveEvent(mLowMemory);
        RemoveEvent(mRegionFormatChanged);
        RemoveEvent(mDeviceOrientationChanged);
        RemoveEvent(mLanguageChanged);
      }
    }

    std::unique_ptr<AppCoreTaskBase> CreateTask() override
    {
      return std::unique_ptr<AppCoreTaskBase>(
        new Task(mFramework));
    }

    int OnCreate() override
    {
      AppCoreUiBase::OnCreate();
      mFramework->Create();
      return 0;
    }

    int OnTerminate() override
    {
      AppCoreUiBase::OnTerminate();
      auto* observer = &mFramework->mObserver;
      observer->OnTerminate();
      return 0;
    }

    int OnPause() override
    {
      AppCoreUiBase::OnPause();
      auto* observer = &mFramework->mObserver;
      observer->OnPause();
      return 0;
    }

    int OnResume() override
    {
      AppCoreUiBase::OnResume();
      auto* observer = &mFramework->mObserver;
      observer->OnResume();
      return 0;
    }

    int OnControl(tizen_base::Bundle b) override
    {
      AppCoreUiBase::OnControl(b);

      app_control_h appControl = nullptr;

      auto* bundleData = b.GetHandle();
      if(bundleData)
      {
        if(app_control_create_event(bundleData, &appControl) != TIZEN_ERROR_NONE)
        {
          DALI_LOG_ERROR("Failed to create an app_control handle");
          return 0;
        }
      }
      else
      {
        if(app_control_create(&appControl) != TIZEN_ERROR_NONE)
        {
          DALI_LOG_ERROR("Failed to create an app_control handle");
          return 0;
        }
      }

      auto* observer = &mFramework->mObserver;
      ProcessBundle(mFramework, bundleData);
      observer->OnReset();
      observer->OnAppControl(appControl);
      app_control_destroy(appControl);
      return 0;
    }

    void OnLoopInit(int argc, char** argv) override
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

    void OnLoopFinish() override
    {
      ecore_shutdown();

      if(getenv("AUL_LOADER_INIT"))
      {
        setenv("AUL_LOADER_INIT", "0", 1);
        ecore_shutdown();
      }

#ifdef DALI_ELDBUS_AVAILABLE
      // Shutdown ELDBus.
      DALI_LOG_INFO(gDBusLogging, Debug::General, "Shutting down DBus\n");
      eldbus_shutdown();
#endif
    }

    void OnLoopRun() override
    {
      ecore_main_loop_begin();
    }

    void OnLoopExit() override
    {
      ecore_main_loop_quit();
    }

  private:
    static void OnLanguageChanged(app_event_info_h event_info, void* user_data)
    {
      auto*     context   = static_cast<UiAppContext*>(user_data);
      auto*     framework = context->mFramework;
      Observer* observer  = &framework->mObserver;

      char* lang           = nullptr;
      auto  appEventReturn = app_event_get_language(event_info, &lang);
      if(appEventReturn == APP_ERROR_NONE && lang)
      {
        framework->SetLanguage(std::string(lang));
        observer->OnLanguageChanged();
        free(lang);
      }
      else
      {
        DALI_LOG_ERROR("NULL pointer in Language changed event. Error code : %d\n", static_cast<int>(appEventReturn));
      }
    }

    static void OnRegionFormatChanged(app_event_info_h event_info, void* user_data)
    {
      auto*     context   = static_cast<UiAppContext*>(user_data);
      auto*     framework = context->mFramework;
      Observer* observer  = &framework->mObserver;

      char* region         = nullptr;
      auto  appEventReturn = app_event_get_region_format(event_info, &region);
      if(appEventReturn == APP_ERROR_NONE && region)
      {
        framework->SetRegion(std::string(region));
        observer->OnRegionChanged();
        free(region);
      }
      else
      {
        DALI_LOG_ERROR("NULL pointer in Region changed event. Error code : %d\n", static_cast<int>(appEventReturn));
      }
    }

    static void OnLowBattery(app_event_info_h event_info, void* user_data)
    {
      auto*     context   = static_cast<UiAppContext*>(user_data);
      auto*     framework = context->mFramework;
      Observer* observer  = &framework->mObserver;

      app_event_low_battery_status_e status;
      auto                           appEventReturn = app_event_get_low_battery_status(event_info, &status);
      if(appEventReturn == APP_ERROR_NONE)
      {
        Dali::DeviceStatus::Battery::Status result = AppCore::GetBatteryStatus(status);
        observer->OnBatteryLow(result);
      }
      else
      {
        DALI_LOG_ERROR("Fail to get low battery status event. Error code : %d\n", static_cast<int>(appEventReturn));
      }
    }

    static void OnLowMemory(app_event_info_h event_info, void* user_data)
    {
      auto*     context   = static_cast<UiAppContext*>(user_data);
      auto*     framework = context->mFramework;
      Observer* observer  = &framework->mObserver;

      app_event_low_memory_status_e status;
      auto                          appEventReturn = app_event_get_low_memory_status(event_info, &status);
      if(appEventReturn == APP_ERROR_NONE)
      {
        Dali::DeviceStatus::Memory::Status result = AppCore::GetMemoryStatus(status);
        observer->OnMemoryLow(result);
      }
      else
      {
        DALI_LOG_ERROR("Fail to get low memory status event. Error code : %d\n", static_cast<int>(appEventReturn));
      }
    }

    static void OnDeviceOrientationChanged(app_event_info_h event_info, void* user_data)
    {
      auto*     context   = static_cast<UiAppContext*>(user_data);
      auto*     framework = context->mFramework;
      Observer* observer  = &framework->mObserver;

      app_device_orientation_e status;
      auto                     appEventReturn = app_event_get_device_orientation(event_info, &status);
      if(appEventReturn == APP_ERROR_NONE)
      {
        Dali::DeviceStatus::Orientation::Status result = AppCore::GetOrientationStatus(status);
        observer->OnDeviceOrientationChanged(result);
      }
      else
      {
        DALI_LOG_ERROR("Fail to get device orientation event. Error code : %d\n", static_cast<int>(appEventReturn));
      }
    }

    void ProcessBundle(FrameworkTizen* framework, bundle* bundleData)
    {
      if(bundleData == nullptr)
      {
        return;
      }

      // get bundle name
      char* bundleName = const_cast<char*>(bundle_get_val(bundleData, "name"));
      if(bundleName != nullptr)
      {
        framework->SetBundleName(bundleName);
      }

      // get bundle? id
      char* bundleId = const_cast<char*>(bundle_get_val(bundleData, "id"));
      if(bundleId != nullptr)
      {
        framework->SetBundleId(bundleId);
      }
    }

  private:
    FrameworkTizen*           mFramework;
    std::shared_ptr<AppEvent> mLanguageChanged;
    std::shared_ptr<AppEvent> mDeviceOrientationChanged;
    std::shared_ptr<AppEvent> mRegionFormatChanged;
    std::shared_ptr<AppEvent> mLowBattery;
    std::shared_ptr<AppEvent> mLowMemory;
    bool                      mUseUiThread;
  };

  // Constructor
  Impl(void* data, Type type, bool useUiThread)
  : handlers{nullptr, nullptr, nullptr, nullptr, nullptr},
    mUseUiThread(useUiThread)
#ifdef APPCORE_WATCH_AVAILABLE
    ,
    mWatchCallback()
#endif
  {
    mFramework = static_cast<FrameworkTizen*>(data);

#ifndef APPCORE_WATCH_AVAILABLE
    if(type == WATCH)
    {
      throw Dali::DaliException("", "Watch Application is not supported.");
    }
#endif
    mApplicationType = type;
  }

  ~Impl()
  {
  }

  int AppMain()
  {
    // TODO: The app-core-cpp has to be applied to the other app types.
    int ret;
    switch(mApplicationType)
    {
      case NORMAL:
      {
        ret = AppNormalMain();
        break;
      }
      case WIDGET:
      {
        ret = AppWidgetMain();
        break;
      }
      case WATCH:
      {
        ret = AppWatchMain();
        break;
      }
#ifdef COMPONENT_APPLICATION_SUPPORT
      case COMPONENT:
      {
        ret = AppComponentMain();
        break;
      }
#endif
    }
    return ret;
  }

  void AppExit()
  {
    switch(mApplicationType)
    {
      case NORMAL:
      {
        AppNormalExit();
        break;
      }
      case WIDGET:
      {
        AppWidgetExit();
        break;
      }
      case WATCH:
      {
        AppWatchExit();
        break;
      }
#ifdef COMPONENT_APPLICATION_SUPPORT
      case COMPONENT:
      {
        AppComponentExit();
        break;
      }
#endif
    }
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

  // Data
  Type        mApplicationType;
  std::string mLanguage{};
  std::string mRegion{};

  FrameworkTizen*               mFramework;
  AppCore::AppEventHandlerPtr   handlers[5];
  std::unique_ptr<UiAppContext> mUiAppContext{nullptr};
  bool                          mUseUiThread{false};
#ifdef APPCORE_WATCH_AVAILABLE
  watch_app_lifecycle_callback_s mWatchCallback;
#endif

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

    if(getenv("AUL_LOADER_INIT"))
    {
      setenv("AUL_LOADER_INIT", "0", 1);
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

  static void AppLanguageChanged(AppCore::AppEventInfoPtr event, void* data)
  {
    FrameworkTizen* framework = static_cast<FrameworkTizen*>(data);
    Observer*       observer  = &framework->mObserver;

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

  static void AppRegionChanged(AppCore::AppEventInfoPtr event, void* data)
  {
    FrameworkTizen* framework = static_cast<FrameworkTizen*>(data);
    Observer*       observer  = &framework->mObserver;

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

  static void AppBatteryLow(AppCore::AppEventInfoPtr event, void* data)
  {
    Observer*                           observer = &static_cast<FrameworkTizen*>(data)->mObserver;
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

  static void AppMemoryLow(AppCore::AppEventInfoPtr event, void* data)
  {
    Observer*                          observer = &static_cast<FrameworkTizen*>(data)->mObserver;
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

  static void AppDeviceOrientationChanged(AppCore::AppEventInfoPtr event, void* data)
  {
    Observer*                               observer = &static_cast<FrameworkTizen*>(data)->mObserver;
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

  int AppNormalMain()
  {
    if(mUiAppContext.get() == nullptr)
    {
      unsigned int hint = AppCoreUiBase::HINT_WINDOW_GROUP_CONTROL |
                          AppCoreUiBase::HINT_WINDOW_STACK_CONTROL |
                          AppCoreUiBase::HINT_BG_LAUNCH_CONTROL |
                          AppCoreUiBase::HINT_HW_ACC_CONTROL |
                          AppCoreUiBase::HINT_WINDOW_AUTO_CONTROL;

      // For testing UIThread model, This code turns on the UI Thread feature forcibly.
      //  ex) app_launcher -e [APPID] __K_UI_THREAD enable
      // This code doesn't change mUseUiThread in Internal::Application
      bundle* b = bundle_import_from_argv(*mFramework->mArgc, *mFramework->mArgv);
      if(b != nullptr)
      {
        const char* val = bundle_get_val(b, "__K_UI_THREAD");
        if(val != nullptr && strcmp(val, "enable") == 0)
        {
          mUseUiThread = true;
        }

        bundle_free(b);
      }

      if(mUseUiThread)
      {
        hint |= AppCoreUiBase::HINT_DUAL_THREAD;
      }

      mUiAppContext = std::make_unique<UiAppContext>(hint, mFramework);
    }

    mUiAppContext->Run(*mFramework->mArgc, *mFramework->mArgv);
    return TIZEN_ERROR_NONE;
  }

  void AppNormalExit()
  {
    if(mUiAppContext.get() == nullptr)
    {
      return;
    }

    mUiAppContext->Exit();
  }

  void AppWidgetExit()
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
    Observer* observer = &static_cast<FrameworkTizen*>(data)->mObserver;
    observer->OnTerminate();

    widget_base_on_terminate();
    return 0;
  }

  int AppWidgetMain()
  {
    if(!IsWidgetFeatureEnabled())
    {
      DALI_LOG_ERROR("widget feature is not supported");
      return 0;
    }

    AppCore::AppAddEventHandler(&handlers[AppCore::LOW_BATTERY], AppCore::LOW_BATTERY, AppBatteryLow, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::LOW_MEMORY], AppCore::LOW_MEMORY, AppMemoryLow, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::DEVICE_ORIENTATION_CHANGED], AppCore::DEVICE_ORIENTATION_CHANGED, AppDeviceOrientationChanged, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::LANGUAGE_CHANGED], AppCore::LANGUAGE_CHANGED, AppLanguageChanged, mFramework);
    AppCore::AppAddEventHandler(&handlers[AppCore::REGION_FORMAT_CHANGED], AppCore::REGION_FORMAT_CHANGED, AppRegionChanged, mFramework);

    widget_base_ops ops = widget_base_get_default_ops();

    /* override methods */
    ops.create    = WidgetAppCreate;
    ops.terminate = WidgetAppTerminate;
    ops.init      = AppInit;
    ops.finish    = AppFinish;
    ops.run       = AppRun;
    ops.exit      = AppExit;

    int result = widget_base_init(ops, *mFramework->mArgc, *mFramework->mArgv, mFramework);

    widget_base_fini();

    return result;
  }

#ifdef APPCORE_WATCH_AVAILABLE
  static bool WatchAppCreate(int width, int height, void* data)
  {
    return static_cast<FrameworkTizen*>(data)->Create();
  }

  static void WatchAppTimeTick(watch_time_h time, void* data)
  {
    Observer* observer = &static_cast<FrameworkTizen*>(data)->mObserver;
    WatchTime curTime(time);

    observer->OnTimeTick(curTime);
  }

  static void WatchAppAmbientTick(watch_time_h time, void* data)
  {
    Observer* observer = &static_cast<FrameworkTizen*>(data)->mObserver;
    WatchTime curTime(time);

    observer->OnAmbientTick(curTime);
  }

  static void WatchAppAmbientChanged(bool ambient, void* data)
  {
    Observer* observer = &static_cast<FrameworkTizen*>(data)->mObserver;

    observer->OnAmbientChanged(ambient);
  }

  static void WatchAppControl(app_control_h app_control, void* data)
  {
    FrameworkTizen* framework  = static_cast<FrameworkTizen*>(data);
    Observer*       observer   = &framework->mObserver;
    bundle*         bundleData = NULL;

    app_control_to_bundle(app_control, &bundleData);
    ProcessBundle(framework, bundleData);

    observer->OnReset();
    observer->OnAppControl(app_control);
  }

  static void WatchAppTerminate(void* data)
  {
    Observer* observer = &static_cast<FrameworkTizen*>(data)->mObserver;

    observer->OnTerminate();
  }

  static void WatchAppPause(void* data)
  {
    Observer* observer = &static_cast<FrameworkTizen*>(data)->mObserver;

    observer->OnPause();
  }

  static void WatchAppResume(void* data)
  {
    Observer* observer = &static_cast<FrameworkTizen*>(data)->mObserver;

    observer->OnResume();
  }

#endif

  int AppWatchMain()
  {
    int ret = true;

#ifdef APPCORE_WATCH_AVAILABLE
    mWatchCallback.create          = WatchAppCreate;
    mWatchCallback.app_control     = WatchAppControl;
    mWatchCallback.terminate       = WatchAppTerminate;
    mWatchCallback.pause           = WatchAppPause;
    mWatchCallback.resume          = WatchAppResume;
    mWatchCallback.time_tick       = WatchAppTimeTick;
    mWatchCallback.ambient_tick    = WatchAppAmbientTick;
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

#ifdef COMPONENT_APPLICATION_SUPPORT
  int AppComponentMain()
  {
    /*Crate component_based_app_base_lifecycle_callback*/
    component_based_app_base_lifecycle_callback_s callback;
    callback.init      = AppInit;
    callback.run       = AppRun;
    callback.exit      = AppExit;
    callback.create    = ComponentAppCreate;
    callback.terminate = ComponentAppTerminate;
    callback.fini      = ComponentAppFinish;

    return component_based_app_base_main(*mFramework->mArgc, *mFramework->mArgv, &callback, mFramework);
  }

  static void* ComponentAppCreate(void* data)
  {
    FrameworkTizen* framework = static_cast<FrameworkTizen*>(data);
    Observer*       observer  = &framework->mObserver;
    observer->OnInit();

    return Dali::AnyCast<void*>(observer->OnCreate());
  }

  static void ComponentAppTerminate(void* data)
  {
    Observer* observer = &static_cast<FrameworkTizen*>(data)->mObserver;
    observer->OnTerminate();
  }

  static void ComponentAppFinish(void* data)
  {
    ecore_shutdown();

    if(getenv("AUL_LOADER_INIT"))
    {
      setenv("AUL_LOADER_INIT", "0", 1);
      ecore_shutdown();
    }
  }

  void AppComponentExit()
  {
    component_based_app_base_exit();
  }

#endif

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
  if(ret != APP_ERROR_NONE)
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
  Impl(void *data)
  {
    mUIThreadLoader = static_cast<UIThreadLoader*>(data);
    mAppCoreUiThreadBase = new AppCoreUiThreadBase();
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > Create mAppCoreUiThreadBase(%p)", __MODULE__, __func__, __LINE__, mAppCoreUiThreadBase);
  }

  // Destructor
  ~Impl()
  {
    if(mAppCoreUiThreadBase)
    {
      mAppCoreUiThreadBase->Exit();
      delete mAppCoreUiThreadBase;
    }
  }

  /**
   * Runs to work create window in UI thread when application is pre-initialized.
   */
  void Run(Runner runner)
  {
    mAppCoreUiThreadBase->Post(runner);
    mAppCoreUiThreadBase->Run(*(mUIThreadLoader->mArgc), *(mUIThreadLoader->mArgv));
  }

private:
  // Undefined
  Impl(const Impl& impl);
  Impl& operator=(const Impl& impl);

  // Data
  AppCoreUiThreadBase* mAppCoreUiThreadBase;
  UIThreadLoader*      mUIThreadLoader;
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
