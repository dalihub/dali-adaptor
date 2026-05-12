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
#include <dali/internal/adaptor/common/application-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/trace.h>
#include <dali/public-api/object/object-registry.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/style-monitor.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/framework-factory.h>
#include <dali/internal/adaptor/common/framework.h>
#include <dali/internal/system/common/command-line-options.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/common/system-settings.h>
#include <dali/internal/window-system/common/render-surface-factory.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>

// To disable a macro with the same name from one of OpenGL headers
#undef Status

namespace Dali
{
namespace TizenPlatform
{
class TizenPlatformAbstraction;
}

namespace Integration
{
class Core;
}

namespace Internal
{
namespace Adaptor
{
namespace
{
DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_APPLICATION, true);
} // namespace

ApplicationPtr Application::New(
  int*               argc,
  char**             argv[],
  const std::string& stylesheet,
  Framework::Type    applicationType,
  bool               useUiThread,
  const WindowData&  windowData)
{
  ApplicationPtr application(new Application(argc, argv, stylesheet, applicationType, useUiThread, windowData));
  return application;
}

Application::Application(int* argc, char** argv[], const std::string& stylesheet, Framework::Type applicationType, bool useUiThread, const WindowData& windowData)
: mInitSignal(),
  mTerminateSignal(),
  mPauseSignal(),
  mResumeSignal(),
  mResetSignal(),
  mAppControlSignal(),
  mLanguageChangedSignal(),
  mRegionChangedSignal(),
  mStylesheet(stylesheet),
  mSlotDelegate(this),
  mFramework(nullptr),
  mApplicationController(nullptr),
  mUIThreadLoader(nullptr),
  mUseRemoteSurface(false),
  mUseUiThread(useUiThread)
{
  PositionSize initSize = windowData.GetPositionSize();
  if(initSize.width == 0 && initSize.height == 0)
  {
    CommandLineOptions commandLineOptions(argc, argv);

    if(commandLineOptions.stageWidth > 0 && commandLineOptions.stageHeight > 0)
    {
      initSize.width  = commandLineOptions.stageWidth;
      initSize.height = commandLineOptions.stageHeight;
    }
  }

  mApplicationController = ApplicationController::New(initSize);

  if(argc && *argc > 0)
  {
    mApplicationController->SetWindowName((*argv)[0]);
  }
  mApplicationController->SetWindowData(windowData);
  mApplicationController->SetWindowPositionSize(initSize); // Use the updated window size
  mApplicationController->SetDeferCompletePreInitialize(true);

  const char* uiThreadEnabled = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_ENABLE_UI_THREAD);
  if(uiThreadEnabled && std::atoi(uiThreadEnabled) != 0)
  {
    mUseUiThread = true;
  }

  mFramework = mApplicationController->GetFrameworkFactory()->CreateFramework(FrameworkBackend::DEFAULT, *this, *this, argc, argv, applicationType, mUseUiThread);

  mUseRemoteSurface = (applicationType == Framework::WATCH);
}

Application::~Application()
{
  DALI_LOG_RELEASE_INFO("Application::~Application\n");

  if(mUIThreadLoader)
  {
    delete mUIThreadLoader;
  }
}

void Application::MainLoop()
{
  // Run the application
  mFramework->Run();
}

void Application::Lower()
{
  // Lower the application without quitting it.
  mApplicationController->GetWindow().Lower();
}

void Application::Quit()
{
  DALI_LOG_RELEASE_INFO("Application::Quit requested!\n");
  // Actually quit the application.
  // Force a call to Quit even if adaptor is not running.
  Internal::Adaptor::Adaptor::GetImplementation(*mApplicationController->GetAdaptor()).AddIdle(MakeCallback(this, &Application::QuitFromMainLoop), false);
}

void Application::QuitFromMainLoop()
{
  DALI_LOG_RELEASE_INFO("Application::Quit processing\n");
  if(auto bridge = Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->Terminate();
  }

  mApplicationController->GetAdaptor()->Stop();

  mFramework->Quit();
  // This will trigger OnTerminate(), below, after the main loop has completed.
  DALI_LOG_RELEASE_INFO("Application::Quit finished\n");
}

void Application::OnInit()
{
  DALI_LOG_RELEASE_INFO("Application::OnInit\n");

  mApplicationController->PreInitialize();

  mFramework->AddAbortCallback(MakeCallback(this, &Application::QuitFromMainLoop));

  Adaptor::GetImplementation(*mApplicationController->GetAdaptor()).SetUseRemoteSurface(mUseRemoteSurface);

  // Initialize StyleMonitor here.
  Dali::StyleMonitor styleMonitor = Dali::StyleMonitor::Get();
  if(!mStylesheet.empty())
  {
    styleMonitor.SetTheme(mStylesheet);
  }

  mApplicationController->CompletePreInitialize();

  Dali::Application application(this);

  DALI_TRACE_BEGIN(gTraceFilter, "DALI_APP_EMIT_INIT_SIGNAL");
  mInitSignal.Emit(application);
  DALI_TRACE_END(gTraceFilter, "DALI_APP_EMIT_INIT_SIGNAL");

  mApplicationController->PostInitialize();

  Dali::Window window = mApplicationController->GetWindow();
  GetImplementation(window).DeleteRequestSignal().Connect(mSlotDelegate, &Application::Quit);
}

void Application::OnTerminate()
{
  DALI_LOG_RELEASE_INFO("Application::OnTerminate\n");

  // We've been told to quit by AppCore, ecore_x_destroy has been called, need to quit synchronously
  // delete the window as ecore_x has been destroyed by AppCore

  mApplicationController->PreTerminate();

  Dali::Application application(this);
  mTerminateSignal.Emit(application);

  mApplicationController->PostTerminate();

  // If DALi's UI Thread works, some resources are created in UI Thread, not Main thread.
  // For that case, these resource should be deleted in UI Thread.
  if(mUseUiThread)
  {
    mApplicationController.Reset();
  }
}

void Application::OnPause()
{
  DALI_LOG_RELEASE_INFO("Application::OnPause\n");

  mApplicationController->PrePause();

  // A DALi app should handle Pause/Resume events.
  // DALi just delivers the framework Pause event to the application, but not actually pause DALi core.
  // Pausing DALi core only occurs on the Window Hidden framework event
  Dali::Application application(this);
  mPauseSignal.Emit(application);

  mApplicationController->PostPause();
}

void Application::OnResume()
{
  DALI_LOG_RELEASE_INFO("Application::OnResume\n");

  mApplicationController->PreResume();

  // Emit the signal first so the application can queue any messages before we do an update/render
  // This ensures we do not just redraw the last frame before pausing if that's not required
  Dali::Application application(this);
  mResumeSignal.Emit(application);

  mApplicationController->PostResume();
}

void Application::OnReset()
{
  DALI_LOG_RELEASE_INFO("Application::OnReset\n");

  mApplicationController->PreReset();

  /*
   * usually, reset callback was called when a caller request to launch this application via aul.
   * because Application class already handled initialization in OnInit(), OnReset do nothing.
   */
  Dali::Application application(this);
  mResetSignal.Emit(application);
}

void Application::OnAppControl(void* data)
{
  DALI_LOG_RELEASE_INFO("Application::OnAppControl\n");
  Dali::Application application(this);
  mAppControlSignal.Emit(application, data);
}

void Application::OnLanguageChanged()
{
  DALI_LOG_RELEASE_INFO("Application::OnLanguageChanged\n");

  mApplicationController->PreLanguageChanged(GetLanguage());

  Dali::Application application(this);
  mLanguageChangedSignal.Emit(application);
}

void Application::OnRegionChanged()
{
  DALI_LOG_RELEASE_INFO("Application::OnRegionChanged\n");
  Dali::Application application(this);
  mRegionChangedSignal.Emit(application);
}

void Application::OnBatteryLow(Dali::DeviceStatus::Battery::Status status)
{
  DALI_LOG_RELEASE_INFO("Application::OnBatteryLow\n");
  Dali::Application application(this);
  mLowBatterySignal.Emit(status);
}

void Application::OnMemoryLow(Dali::DeviceStatus::Memory::Status status)
{
  DALI_LOG_RELEASE_INFO("Application::OnMemoryLow\n");
  Dali::Application application(this);
  mLowMemorySignal.Emit(status);
}

void Application::OnDeviceOrientationChanged(Dali::DeviceStatus::Orientation::Status status)
{
  DALI_LOG_RELEASE_INFO("Application::OnDeviceOrientationChanged\n");
  Dali::Application application(this);
  mDeviceOrientationChangedSignal.Emit(status);
}

void Application::OnSurfaceCreated(Any newSurface)
{
  DALI_LOG_RELEASE_INFO("Application::OnSurfaceCreated\n");
  void* newWindow = AnyCast<void*>(newSurface);
  void* oldWindow = AnyCast<void*>(mApplicationController->GetWindow().GetNativeHandle());
  if(oldWindow != newWindow)
  {
    auto                                 renderSurfaceFactory = Dali::Internal::Adaptor::GetRenderSurfaceFactory();
    std::unique_ptr<WindowRenderSurface> newSurfacePtr        = renderSurfaceFactory->CreateWindowRenderSurface(PositionSize(), newSurface, true);

    mApplicationController->GetAdaptor()->ReplaceSurface(mApplicationController->GetWindow(), *newSurfacePtr.release());
  }
}

void Application::OnSurfaceDestroyed(Any surface)
{
}

void Application::OnTaskInit()
{
  DALI_LOG_RELEASE_INFO("Application::OnTaskInit\n");
  Dali::Application application(this);
  mTaskInitSignal.Emit(application);
}

void Application::OnTaskTerminate()
{
  DALI_LOG_RELEASE_INFO("Application::OnTaskTerminate\n");
  Dali::Application application(this);
  mTaskTerminateSignal.Emit(application);
}

void Application::OnTaskAppControl(void* data)
{
  DALI_LOG_RELEASE_INFO("Application::OnTaskAppControl\n");
  Dali::Application application(this);
  mTaskAppControlSignal.Emit(application, data);
}

void Application::OnTaskLanguageChanged()
{
  DALI_LOG_RELEASE_INFO("Application::OnTaskLanguageChanged\n");
  Dali::Application application(this);
  mTaskLanguageChangedSignal.Emit(application);
}

void Application::OnTaskRegionChanged()
{
  DALI_LOG_RELEASE_INFO("Application::OnTaskRegionChanged\n");
  Dali::Application application(this);
  mTaskRegionChangedSignal.Emit(application);
}

void Application::OnTaskBatteryLow(Dali::DeviceStatus::Battery::Status status)
{
  DALI_LOG_RELEASE_INFO("Application::OnTaskBatteryLow\n");
  Dali::Application application(this);
  mTaskLowBatterySignal.Emit(status);
}

void Application::OnTaskMemoryLow(Dali::DeviceStatus::Memory::Status status)
{
  DALI_LOG_RELEASE_INFO("Application::OnTaskMemoryLow\n");
  Dali::Application application(this);
  mTaskLowMemorySignal.Emit(status);
}

void Application::OnTaskDeviceOrientationChanged(Dali::DeviceStatus::Orientation::Status status)
{
  DALI_LOG_RELEASE_INFO("Application::OnTaskDeviceOrientationChanged\n");
  Dali::Application application(this);
  mTaskDeviceOrientationChangedSignal.Emit(status);
}

bool Application::AddIdle(CallbackBase* callback, bool hasReturnValue)
{
  return mApplicationController->GetAdaptor()->AddIdle(callback, hasReturnValue);
}

std::string Application::GetRegion() const
{
  return mFramework->GetRegion();
}

std::string Application::GetLanguage() const
{
  return mFramework->GetLanguage();
}

Dali::ObjectRegistry Application::GetObjectRegistry() const
{
  Dali::ObjectRegistry objectRegistry;
  if(auto adaptor = mApplicationController->GetAdaptor())
  {
    objectRegistry = adaptor->GetObjectRegistry();
  }
  return objectRegistry;
}

Dali::Adaptor& Application::GetAdaptor()
{
  return *(mApplicationController->GetAdaptor());
}

Dali::Window Application::GetWindow()
{
  return mApplicationController->GetWindow();
}

std::string Application::GetResourcePath()
{
  return SystemSettings::GetResourcePath();
}

std::string Application::GetDataPath()
{
  return SystemSettings::GetDataPath();
}

void Application::FlushUpdateMessages()
{
  mApplicationController->GetAdaptor()->FlushUpdateMessages();
}

void Application::SetApplicationLocale(const std::string& locale)
{
  Internal::Adaptor::Adaptor::GetImplementation(*(mApplicationController->GetAdaptor())).SetApplicationLocale(locale);
}

int32_t Application::GetRenderThreadId() const
{
  if(mApplicationController->GetAdaptor())
  {
    return mApplicationController->GetAdaptor()->GetRenderThreadId();
  }
  return 0;
}

int32_t Application::GetUiThreadId() const
{
  if(mApplicationController->GetAdaptor())
  {
    return mApplicationController->GetAdaptor()->GetUiThreadId();
  }
  return 0;
}

int32_t Application::GetMainThreadId() const
{
  if(mApplicationController->GetAdaptor())
  {
    return mApplicationController->GetAdaptor()->GetMainThreadId();
  }
  return 0;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
