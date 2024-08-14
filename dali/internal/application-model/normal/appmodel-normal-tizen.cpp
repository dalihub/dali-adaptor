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
#include <dali/internal/application-model/normal/appmodel-normal-tizen.h>

// EXTERNAL INCLUDES
#include <app_common.h>
#include <app_control_internal.h>
#include <appcore_base.h>
#include <bundle.h>
#include <bundle_internal.h>
#include <dlog.h>
#include <glib.h>
#include <system_info.h>
#include <system_settings.h>
#include <app_core_task_base.hh>
#include <app_core_ui_base.hh>
#include <app_event_internal.hh>

// CONDITIONAL INCLUDES
#ifdef DALI_ELDBUS_AVAILABLE
#include <Eldbus.h>
#endif // DALI_ELDBUS_AVAILABLE

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/adaptor/common/framework.h>
#include <dali/internal/adaptor/tizen-wayland/framework-tizen.h>
#include <dali/internal/system/linux/dali-ecore.h>

using namespace tizen_cpp;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
extern "C" DALI_ADAPTOR_API AppModelNormal* Create()
{
  return new AppModelNormal(false);
}

extern "C" DALI_ADAPTOR_API void Destroy(void* p)
{
  AppModelNormal* appNormal = static_cast<AppModelNormal*>(p);
  delete appNormal;
}

extern "C" DALI_ADAPTOR_API int AppMain(bool isUiThread, void* data, void* pData)
{
  AppModelNormal* appNormal = static_cast<AppModelNormal*>(pData);
  int             ret       = 0;
  if(appNormal != nullptr)
  {
    ret = appNormal->AppMain(data);
  }
  else
  {
    print_log(DLOG_INFO, "DALI", "appNormal is nullptr");
  }
  return ret;
}

extern "C" DALI_ADAPTOR_API void AppExit(AppModelNormal* p)
{
  p->AppExit();
}

namespace
{
#if defined(DEBUG_ENABLED)
Integration::Log::Filter* gDBusLogging = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_ADAPTOR_EVENTS_DBUS");
#endif

const char* TIZEN_GLIB_CONTEXT_ENV        = "TIZEN_GLIB_CONTEXT";
const char* AUL_LOADER_INIT_ENV           = "AUL_LOADER_INIT";
const char* AUL_LOADER_INIT_DEFAULT_VALUE = "0";
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

struct DALI_ADAPTOR_API AppModelNormal::Impl
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
        mFramework->GetTaskObserver().OnTaskInit();
        return AppCoreTaskBase::OnCreate();
      }

      int OnTerminate() override
      {
        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > OnTerminate() emitted", __MODULE__, __func__, __LINE__);
        mFramework->GetTaskObserver().OnTaskTerminate();
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
        mFramework->GetTaskObserver().OnTaskAppControl(appControl);

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
            framework->GetObserver().OnMemoryLow(task->mNewMemoryStatus);
            return G_SOURCE_REMOVE;
          });
        mFramework->GetTaskObserver().OnTaskMemoryLow(mNewMemoryStatus);
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
            framework->GetObserver().OnDeviceOrientationChanged(task->mNewDeviceOrientationStatus);
            return G_SOURCE_REMOVE;
          });

        mFramework->GetTaskObserver().OnTaskDeviceOrientationChanged(mNewDeviceOrientationStatus);

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
            framework->GetObserver().OnBatteryLow(task->mNewBatteryStatus);
            return G_SOURCE_REMOVE;
          });
        mFramework->GetTaskObserver().OnTaskBatteryLow(mNewBatteryStatus);
        AppCoreTaskBase::OnLowBattery(state);
      }

      void OnLangChanged(const std::string& lang) override
      {
        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > OnLangChanged() emitted 1", __MODULE__, __func__, __LINE__);
        mNewLanguage = lang;
        mFramework->SetLanguage(mNewLanguage);

        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > OnLangChanged() emitted 2", __MODULE__, __func__, __LINE__);
        PostToUiThread(
          [](gpointer userData) -> gboolean {
            auto* task      = static_cast<Task*>(userData);
            auto* framework = static_cast<FrameworkTizen*>(task->mFramework);
            framework->GetObserver().OnLanguageChanged();
            return G_SOURCE_REMOVE;
          });
        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > OnLangChanged() emitted 3", __MODULE__, __func__, __LINE__);

        mFramework->GetTaskObserver().OnTaskLanguageChanged();
        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > OnLangChanged() emitted 4", __MODULE__, __func__, __LINE__);
        AppCoreTaskBase::OnLangChanged(lang);

        print_log(DLOG_INFO, "DALI", "%s: %s(%d) > OnLangChanged() emitted 5", __MODULE__, __func__, __LINE__);
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
            framework->GetObserver().OnRegionChanged();
            return G_SOURCE_REMOVE;
          });

        mFramework->GetTaskObserver().OnTaskRegionChanged();
        AppCoreTaskBase::OnRegionChanged(mNewRegion);
      }

    private:
      GMainContext* GetTizenGlibContext()
      {
        GMainContext* context;
        const char*   env = Dali::EnvironmentVariable::GetEnvironmentVariable(TIZEN_GLIB_CONTEXT_ENV);
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
      auto* observer = &mFramework->GetObserver();
      observer->OnTerminate();
      return 0;
    }

    int OnPause() override
    {
      AppCoreUiBase::OnPause();
      auto* observer = &mFramework->GetObserver();
      observer->OnPause();
      return 0;
    }

    int OnResume() override
    {
      AppCoreUiBase::OnResume();
      auto* observer = &mFramework->GetObserver();
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

      auto* observer = &mFramework->GetObserver();
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
      auto*                context   = static_cast<UiAppContext*>(user_data);
      auto*                framework = context->mFramework;
      Framework::Observer* observer  = &framework->GetObserver();

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
      auto*                context   = static_cast<UiAppContext*>(user_data);
      auto*                framework = context->mFramework;
      Framework::Observer* observer  = &framework->GetObserver();

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
      auto*                context   = static_cast<UiAppContext*>(user_data);
      auto*                framework = context->mFramework;
      Framework::Observer* observer  = &framework->GetObserver();

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
      auto*                context   = static_cast<UiAppContext*>(user_data);
      auto*                framework = context->mFramework;
      Framework::Observer* observer  = &framework->GetObserver();

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
      auto*                context   = static_cast<UiAppContext*>(user_data);
      auto*                framework = context->mFramework;
      Framework::Observer* observer  = &framework->GetObserver();

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

  int AppMain(void* data)
  {
    FrameworkTizen* mFramework = static_cast<FrameworkTizen*>(data);
    if(mUiAppContext.get() == nullptr)
    {
      unsigned int hint = AppCoreUiBase::HINT_WINDOW_GROUP_CONTROL |
                          AppCoreUiBase::HINT_WINDOW_STACK_CONTROL |
                          AppCoreUiBase::HINT_BG_LAUNCH_CONTROL |
                          AppCoreUiBase::HINT_HW_ACC_CONTROL |
                          AppCoreUiBase::HINT_WINDOW_AUTO_CONTROL;

#ifdef UI_THREAD_AVAILABLE
      // For testing UIThread model, This code turns on the UI Thread feature forcibly.
      //  ex) app_launcher -e [APPID] __K_UI_THREAD enable
      // This code doesn't change mUseUiThread in Internal::Application
      bundle* b = bundle_import_from_argv(*mFramework->GetArgc(), *mFramework->GetArgv());
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
#endif

      mUiAppContext = std::make_unique<UiAppContext>(hint, mFramework);
    }

    mUiAppContext->Run(*mFramework->GetArgc(), *mFramework->GetArgv());
    return APP_ERROR_NONE;
  }

  void AppExit()
  {
    if(mUiAppContext.get() == nullptr)
    {
      return;
    }

    mUiAppContext->Exit();
  }

  Impl(void* data, bool useUiThread)
  : mUseUiThread(useUiThread)
  {
    mAppModelNormal = static_cast<AppModelNormal*>(data);
  }

  ~Impl()
  {
  }

  AppModelNormal*               mAppModelNormal;
  std::unique_ptr<UiAppContext> mUiAppContext{nullptr};
  bool                          mUseUiThread{false};
};

AppModelNormal::AppModelNormal(bool isUiThread)
{
  mImpl = new Impl(this, isUiThread);
}

AppModelNormal::~AppModelNormal()
{
  delete mImpl;
}

int AppModelNormal::AppMain(void* data)
{
  return mImpl->AppMain(data);
}

void AppModelNormal::AppExit()
{
  mImpl->AppExit();
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
