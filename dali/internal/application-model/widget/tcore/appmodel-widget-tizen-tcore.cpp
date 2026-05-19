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
#include <dali/internal/application-model/widget/tcore/appmodel-widget-tizen-tcore.h>

// EXTERNAL INCLUDES
#include <bundle.h>
#include <bundle_internal.h>
#include <dali/integration-api/debug.h>
#include <dlog.h>
#include <glib.h>
#include <system_info.h>
#include <system_settings.h>
#include <tizen.h>
#include <widget_base.hh>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/framework.h>
#include <dali/internal/adaptor/tizen/framework-tizen.h>
#include <dali/internal/system/common/event-loop.h>
#include <dali/internal/system/common/system-factory.h>

#define DEBUG_PRINTF(fmt, arg...) LOGD(" " fmt, ##arg)

namespace Dali
{
namespace Internal
{

namespace Adaptor
{

tizen_cpp::WidgetBase* gWidgetBase = nullptr;

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

class IEvents {
 public:
  virtual ~IEvents() = default;

  virtual void OnLowBattery(int status) = 0;
  virtual void OnLowMemory(int status) = 0;
  virtual void OnDeviceOrientationChanged(int status) = 0;
  virtual void OnLanguageChanged(const std::string& val) = 0;
  virtual void OnRegionFormatChanged(const std::string& val) = 0;
};

class LowBatteryEvent : public tizen_cpp::WidgetBase::EventBase {
 public:
  LowBatteryEvent(IEvents* listener)
  : EventBase(IEvent::Type::LOW_BATTERY),
    mEvents(listener)
  {
  }
  void OnEvent(const std::string& val) override
  {
  }

  void OnEvent(int val) override
  {
    mEvents->OnLowBattery(val);
  }

 private:
  IEvents* mEvents;
};

class LowMemoryEvent : public tizen_cpp::WidgetBase::EventBase {
 public:
  LowMemoryEvent(IEvents* listener)
  : EventBase(IEvent::Type::LOW_MEMORY),
    mEvents(listener)
  {
  }
  void OnEvent(const std::string& val) override
  {
  }

  void OnEvent(int val) override
  {
    mEvents->OnLowMemory(val);
  }

 private:
  IEvents* mEvents;
};

class DeviceOrientationChangedEvent : public tizen_cpp::WidgetBase::EventBase {
 public:
  DeviceOrientationChangedEvent(IEvents* listener)
  : EventBase(IEvent::Type::DEVICE_ORIENTATION_CHANGED),
    mEvents(listener)
  {
  }
  void OnEvent(const std::string& val) override
  {
  }

  void OnEvent(int val) override
  {
    mEvents->OnDeviceOrientationChanged(val);
  }

 private:
  IEvents* mEvents;
};

class LanguageChangedEvent : public tizen_cpp::WidgetBase::EventBase {
 public:
  LanguageChangedEvent(IEvents* listener)
  : EventBase(IEvent::Type::LANG_CHANGE),
    mEvents(listener)
  {
  }
  void OnEvent(const std::string& val) override
  {
    mEvents->OnLanguageChanged(val);
  }

  void OnEvent(int val) override
  {
  }

 private:
  IEvents* mEvents;
};

class RegionFormatChangedEvent : public tizen_cpp::WidgetBase::EventBase {
 public:
  RegionFormatChangedEvent(IEvents* listener)
  : EventBase(IEvent::Type::REGION_CHANGE),
    mEvents(listener)
  {
  }
  void OnEvent(const std::string& val) override
  {
    mEvents->OnRegionFormatChanged(val);
  }

  void OnEvent(int val) override
  {
  }

 private:
  IEvents* mEvents;
};

class DALI_ADAPTOR_API AppModelWidget::Impl : public tizen_cpp::WidgetBase, public IEvents
{
 public:
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

  int OnCreate() override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > WidgetAppCreate() emitted", __MODULE__, __func__, __LINE__);
    WidgetBase::OnCreate();
    return static_cast<int>(mFramework->Create());
  }

  int OnTerminate() override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > WidgetAppTerminate() emitted", __MODULE__, __func__, __LINE__);
    auto& observer = mFramework->GetObserver();
    observer.OnTerminate();

    return WidgetBase::OnTerminate();
  }

  void OnLoopInit(int argc, char** argv) override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppInit() emitted", __MODULE__, __func__, __LINE__);
    mEventLoop = Dali::Internal::Adaptor::GetSystemFactory()->CreateEventLoop();
    if(mEventLoop)
    {
      mEventLoop->Initialize(argc, argv);
    }
    else
    {
      print_log(DLOG_INFO, "DALI", "%s: %s(%d) > Failed to create EventLoop", __MODULE__, __func__, __LINE__);
    }
  }

  void OnLoopFinish() override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppFinish() emitted", __MODULE__, __func__, __LINE__);
    if(mEventLoop)
    {
      mEventLoop->Shutdown();
      mEventLoop.reset();
    }
    else
    {
      print_log(DLOG_INFO, "DALI", "%s: %s(%d) > EventLoop is not created", __MODULE__, __func__, __LINE__);
    }
  }

  void OnLoopRun() override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppRun() emitted", __MODULE__, __func__, __LINE__);
    if(mEventLoop)
    {
      mEventLoop->Run();
    }
    else
    {
      print_log(DLOG_INFO, "DALI", "%s: %s(%d) > EventLoop is not created", __MODULE__, __func__, __LINE__);
    }
  }

  void OnLoopExit() override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppExit() emitted", __MODULE__, __func__, __LINE__);
    if(mEventLoop)
    {
      mEventLoop->Quit();
    }
    else
    {
      print_log(DLOG_INFO, "DALI", "%s: %s(%d) > EventLoop is not created", __MODULE__, __func__, __LINE__);
    }
  }

  void OnLanguageChanged(const std::string& val) override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppLanguageChanged() emitted", __MODULE__, __func__, __LINE__);
    Framework::Observer& observer = mFramework->GetObserver();

    if(!val.empty())
    {
      mFramework->SetLanguage(val);
      observer.OnLanguageChanged();
    }
    else
    {
      DALI_LOG_ERROR("NULL pointer in Language changed event\n");
    }
  }

  void OnRegionFormatChanged(const std::string& val) override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppRegionChanged() emitted", __MODULE__, __func__, __LINE__);
    Framework::Observer& observer = mFramework->GetObserver();

    if(!val.empty())
    {
      mFramework->SetRegion(val);
      observer.OnRegionChanged();
    }
    else
    {
      DALI_LOG_ERROR("NULL pointer in Region changed event\n");
    }
  }

  void OnLowBattery(int status) override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppBatteryLow() emitted", __MODULE__, __func__, __LINE__);
    Framework::Observer& observer = mFramework->GetObserver();
    Dali::DeviceStatus::Battery::Status result = Dali::DeviceStatus::Battery::Status::NORMAL;

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
    observer.OnBatteryLow(result);
  }

  void OnLowMemory(int status) override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppMemoryLow() emitted", __MODULE__, __func__, __LINE__);
    Framework::Observer& observer = mFramework->GetObserver();
    Dali::DeviceStatus::Memory::Status result = Dali::DeviceStatus::Memory::Status::NORMAL;

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
    observer.OnMemoryLow(result);
  }

  void OnDeviceOrientationChanged(int status) override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppDeviceOrientationChanged() emitted", __MODULE__, __func__, __LINE__);
    Framework::Observer& observer = mFramework->GetObserver();
    Dali::DeviceStatus::Orientation::Status result = Dali::DeviceStatus::Orientation::Status::ORIENTATION_0;

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
    observer.OnDeviceOrientationChanged(result);
  }

  int AppMain(void* data)
  {
    print_log(DLOG_INFO, "DALI", "AppModelWidget AppMain 3");
    if(!IsWidgetFeatureEnabled())
    {
      DALI_LOG_ERROR("widget feature is not supported");
      return TIZEN_ERROR_NOT_SUPPORTED;
    }
    mFramework = static_cast<FrameworkTizen*>(data);
    AddEvent(std::shared_ptr<EventBase>(new LowBatteryEvent(this)));
    AddEvent(std::shared_ptr<EventBase>(new LowMemoryEvent(this)));
    AddEvent(std::shared_ptr<EventBase>(new DeviceOrientationChangedEvent(this)));
    AddEvent(std::shared_ptr<EventBase>(new LanguageChangedEvent(this)));
    AddEvent(std::shared_ptr<EventBase>(new RegionFormatChangedEvent(this)));
    print_log(DLOG_INFO, "DALI", "AppModelWidget AppMain 5");
    Run(*mFramework->GetArgc(), *mFramework->GetArgv());
    print_log(DLOG_INFO, "DALI", "AppModelWidget AppMain 6");
    return 0;
  }

  void AppExit()
  {
  }

  Impl(void* data)
  : mAppModelWidget(static_cast<AppModelWidget*>(data))
  {
    gWidgetBase = this;
  }

  ~Impl()
  {
    gWidgetBase = nullptr;
  }

  AppModelWidget*                   mAppModelWidget;
  FrameworkTizen*                   mFramework = nullptr;
  std::unique_ptr<EventLoop>        mEventLoop;
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
