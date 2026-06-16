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
#include <string.h>
#include <system_info.h>
#include <system_settings.h>
#include <tizen.h>
#include <unistd.h>
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
  print_log(DLOG_INFO, "DALI", "DALIWidgetAPP, AppModelWidget is created");
  return new AppModelWidget();
}

extern "C" DALI_ADAPTOR_API void Destroy(void* p)
{
  AppModelWidget* appWidget = static_cast<AppModelWidget*>(p);
  delete appWidget;
}

extern "C" DALI_ADAPTOR_API int AppMain(bool isUiThread, void* data, void* pData)
{
  print_log(DLOG_INFO, "DALI", "[WIDGET_APP] === AppMain() ENTERING - pid=%d ===", getpid());
  print_log(DLOG_INFO, "DALI", "[WIDGET_APP] AppMain() - isUiThread=%d, data=%p, pData=%p", isUiThread, data, pData);

  print_log(DLOG_INFO, "DALI", "DALIWidgetAPP, AppMain is called for appWidget");
  AppModelWidget* appWidget = static_cast<AppModelWidget*>(pData);
  int             ret       = 0;
  if(appWidget != nullptr)
  {
    print_log(DLOG_INFO, "DALI", "[WIDGET_APP] AppMain() - appWidget is valid, calling AppMain(data)...");
    ret = appWidget->AppMain(data);
    print_log(DLOG_INFO, "DALI", "[WIDGET_APP] AppMain() - appWidget->AppMain() returned %d", ret);
  }
  else
  {
    print_log(DLOG_ERROR, "DALI", "[WIDGET_APP] AppMain() - ERROR: appWidget is nullptr!");
  }
  print_log(DLOG_INFO, "DALI", "[WIDGET_APP] === AppMain() EXITING with ret=%d ===", ret);
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

class LowBatteryEvent : public tizen_cpp::WidgetBase::EventBase
{
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

class LowMemoryEvent : public tizen_cpp::WidgetBase::EventBase
{
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

class DeviceOrientationChangedEvent : public tizen_cpp::WidgetBase::EventBase
{
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

class LanguageChangedEvent : public tizen_cpp::WidgetBase::EventBase
{
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

class RegionFormatChangedEvent : public tizen_cpp::WidgetBase::EventBase
{
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
      print_log(DLOG_ERROR, "DALI", "failed to get system info");
      return false;
    }

    retrieved = true;
    return feature;
  }

  int OnCreate() override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > DALIWidgetAPP, WidgetAppCreate() emitted", __MODULE__, __func__, __LINE__);
    print_log(DLOG_INFO, "DALI", "[WIDGET_APP] AppModelWidget::Impl::OnCreate() - Entering\n");

    print_log(DLOG_INFO, "DALI", "[WIDGET_APP] AppModelWidget::Impl::OnCreate() - Calling WidgetBase::OnCreate()\n");
    WidgetBase::OnCreate();

    print_log(DLOG_INFO, "DALI", "[WIDGET_APP] AppModelWidget::Impl::OnCreate() - Calling mFramework->Create()\n");
    int result = static_cast<int>(mFramework->Create());
    print_log(DLOG_INFO, "DALI", "[WIDGET_APP] AppModelWidget::Impl::OnCreate() - Completed with result=%d\n", result);
    return result;
  }

  int OnTerminate() override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > DALIWidgetAPP, WidgetAppTerminate() emitted", __MODULE__, __func__, __LINE__);
    auto& observer = mFramework->GetObserver();
    observer.OnTerminate();

    return WidgetBase::OnTerminate();
  }

  void OnLoopInit(int argc, char** argv) override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > DALIWidgetAPP, AppInit() emitted", __MODULE__, __func__, __LINE__);
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
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > DALIWidgetAPP, AppFinish() emitted", __MODULE__, __func__, __LINE__);
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
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > DALIWidgetAPP, AppRun() emitted", __MODULE__, __func__, __LINE__);
    print_log(DLOG_INFO, "DALI", "[WIDGET_APP] AppModelWidget::Impl::OnLoopRun() - Entering\n");

    if(mEventLoop)
    {
      print_log(DLOG_INFO, "DALI", "[WIDGET_APP] AppModelWidget::Impl::OnLoopRun() - Starting EventLoop::Run()\n");
      mEventLoop->Run();
    }
    else
    {
      print_log(DLOG_ERROR, "DALI", "[WIDGET_APP] AppModelWidget::Impl::OnLoopRun() - EventLoop is not created\n");
    }
    print_log(DLOG_INFO, "DALI", "[WIDGET_APP] AppModelWidget::Impl::OnLoopRun() - Exiting\n");
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
    Framework::Observer& observer  = mFramework->GetObserver();

    if(!val.empty())
    {
      mFramework->SetLanguage(val);
      observer.OnLanguageChanged();
    }
    else
    {
      print_log(DLOG_ERROR, "DALI", "NULL pointer in Language changed event\n");
    }
  }

  void OnRegionFormatChanged(const std::string& val) override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppRegionChanged() emitted", __MODULE__, __func__, __LINE__);
    Framework::Observer& observer  = mFramework->GetObserver();

    if(!val.empty())
    {
      mFramework->SetRegion(val);
      observer.OnRegionChanged();
    }
    else
    {
      print_log(DLOG_ERROR, "DALI", "NULL pointer in Region changed event\n");
    }
  }

  void OnLowBattery(int status) override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppBatteryLow() emitted", __MODULE__, __func__, __LINE__);
    Framework::Observer&                observer = mFramework->GetObserver();
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
    observer.OnBatteryLow(result);
  }

  void OnLowMemory(int status) override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppMemoryLow() emitted", __MODULE__, __func__, __LINE__);
    Framework::Observer&               observer = mFramework->GetObserver();
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
    observer.OnMemoryLow(result);
  }

  void OnDeviceOrientationChanged(int status) override
  {
    print_log(DLOG_INFO, "DALI", "%s: %s(%d) > AppDeviceOrientationChanged() emitted", __MODULE__, __func__, __LINE__);
    Framework::Observer&                    observer = mFramework->GetObserver();
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
    observer.OnDeviceOrientationChanged(result);
  }

  int AppMain(void* data)
  {
    print_log(DLOG_INFO, "DALI", "AppModelWidget AppMain 3");
    print_log(DLOG_INFO, "DALI", "AppModelWidget::Impl::AppMain() - Entering\n");

    if(!IsWidgetFeatureEnabled())
    {
      print_log(DLOG_INFO, "DALI", "widget feature is not supported");
      return TIZEN_ERROR_NOT_SUPPORTED;
    }

    mFramework = static_cast<FrameworkTizen*>(data);
    if(!mFramework)
    {
      print_log(DLOG_INFO, "DALI", " Widget Framework is null!");
      return TIZEN_ERROR_INVALID_PARAMETER;
    }

    print_log(DLOG_INFO, "DALI", "AppModelWidget::Impl::AppMain() - Framework set (argc=%d)\n",
                         mFramework->GetArgc() ? *mFramework->GetArgc() : 0);

    AddEvent(std::shared_ptr<EventBase>(new LowBatteryEvent(this)));
    AddEvent(std::shared_ptr<EventBase>(new LowMemoryEvent(this)));
    AddEvent(std::shared_ptr<EventBase>(new DeviceOrientationChangedEvent(this)));
    AddEvent(std::shared_ptr<EventBase>(new LanguageChangedEvent(this)));
    AddEvent(std::shared_ptr<EventBase>(new RegionFormatChangedEvent(this)));

    print_log(DLOG_INFO, "DALI", "AppModelWidget AppMain 5");

    int argc = 0;
    char** argv = nullptr;
    if(mFramework->GetArgc() && mFramework->GetArgv())
    {
      argc = *mFramework->GetArgc();
      argv = *mFramework->GetArgv();
    }

    print_log(DLOG_INFO, "DALI", "[WIDGET_APP] AppModelWidget::Impl::AppMain() - About to call WidgetBase::Run() with argc=%d\n", argc);

    Run(argc, argv);

    print_log(DLOG_INFO, "DALI", "AppModelWidget AppMain 6");
    print_log(DLOG_INFO, "DALI", "[WIDGET_APP] AppModelWidget::Impl::AppMain() - Run() completed, exiting\n");
    return 0;
  }

  void AppExit()
  {
  }

  Impl(void* data)
  : mAppModelWidget(static_cast<AppModelWidget*>(data)),
    mFramework(nullptr),
    mEventLoop()
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
  print_log(DLOG_INFO, "DALI", "AppModelWidget Constructor");
  mImpl = new Impl(this);
}

AppModelWidget::~AppModelWidget()
{
  delete mImpl;
}

int AppModelWidget::AppMain(void* data)
{
  print_log(DLOG_INFO, "DALI", "AppModelWidget AppMain");
  return mImpl->AppMain(data);
}

void AppModelWidget::AppExit()
{
  mImpl->AppExit();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
