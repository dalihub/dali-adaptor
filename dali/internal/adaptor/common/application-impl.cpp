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
 *
 */

// CLASS HEADER
#include <dali/internal/adaptor/common/application-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/trace.h>
#include <dali/public-api/object/object-registry.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/style-monitor.h>
#include <dali/devel-api/atspi-interfaces/accessible.h>
#include <dali/devel-api/text-abstraction/font-client.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/framework-factory.h>
#include <dali/internal/adaptor/common/framework.h>
#include <dali/internal/adaptor/common/lifecycle-controller-impl.h>
#include <dali/internal/system/common/command-line-options.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/common/system-settings.h>
#include <dali/internal/window-system/common/render-surface-factory.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-system.h>

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
DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_APPLICATION, true);

ApplicationPtr Application::gPreInitializedApplication(NULL);

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

void Application::PreInitialize(int* argc, char** argv[])
{
  if(!gPreInitializedApplication)
  {
    char* retEnv        = std::getenv("TIZEN_UI_THREAD");
    bool  isUseUIThread = false;
    if(retEnv)
    {
      std::string uiThreadEnv   = retEnv;
      std::string enabledString = "true";
      if(uiThreadEnv == enabledString)
      {
        isUseUIThread = true;
      }
    }

    Dali::TextAbstraction::FontClientPreInitialize();
    WindowData windowData;
    gPreInitializedApplication                  = new Application(argc, argv, "", Framework::NORMAL, isUseUIThread, windowData);
    gPreInitializedApplication->mLaunchpadState = Launchpad::PRE_INITIALIZED;
    if(isUseUIThread)
    {
      DALI_LOG_RELEASE_INFO("PRE_INITIALIZED with UI Threading");
      gPreInitializedApplication->mUIThreadLoader = new UIThreadLoader(argc, argv);
      gPreInitializedApplication->mUIThreadLoader->Run([&]() { gPreInitializedApplication->CreateWindow(); });
    }
    else
    {
      DALI_LOG_RELEASE_INFO("Only PRE_INITIALIZED");
      gPreInitializedApplication->CreateWindow(); // Only create window
    }
  }
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
  mFramework(nullptr),
  mFrameworkFactory(nullptr),
  mCommandLineOptions(nullptr),
  mAdaptorBuilder(nullptr),
  mAdaptor(nullptr),
  mEnvironmentOptions(nullptr),
  mMainWindow(),
  mMainWindowMode(windowData.GetTransparency() ? WINDOW_MODE::TRANSPARENT : WINDOW_MODE::OPAQUE),
  mMainWindowName(),
  mStylesheet(stylesheet),
  mWindowPositionSize(windowData.GetPositionSize()),
  mLaunchpadState(Launchpad::NONE),
  mDefaultWindowType(windowData.GetWindowType()),
  mUseUiThread(useUiThread),
  mIsSystemInitialized(false),
  mSlotDelegate(this),
  mUIThreadLoader(nullptr)
{
  // Set mName from command-line args
  if(argc && (*argc > 0))
  {
    mMainWindowName = (*argv)[0];
  }

  const char* uiThreadEnabled = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_ENABLE_UI_THREAD);
  if(uiThreadEnabled && std::atoi(uiThreadEnabled) != 0)
  {
    mUseUiThread = true;
  }

  mCommandLineOptions = new CommandLineOptions(argc, argv);

  mFrameworkFactory = std::unique_ptr<FrameworkFactory>(Dali::Internal::Adaptor::CreateFrameworkFactory());
  mFramework        = mFrameworkFactory->CreateFramework(FrameworkBackend::DEFAULT, *this, *this, argc, argv, applicationType, mUseUiThread);

  mUseRemoteSurface = (applicationType == Framework::WATCH);
}

Application::~Application()
{
  SingletonService service = SingletonService::Get();
  // Note this can be false i.e. if Application has never created a Core instance
  if(service)
  {
    service.UnregisterAll();
  }

  mMainWindow.Reset();

  delete mCommandLineOptions;

  // Application is created in Main thread whether UI Threading is enabled or not.
  // But some resources are created in Main thread or UI thread.
  // The below code is for the resource are created in Main thread.
  if(!mUseUiThread)
  {
    delete mAdaptor;
    delete mAdaptorBuilder;
    if(mIsSystemInitialized)
    {
      WindowSystem::Shutdown();
    }
  }
  else
  {
    if(mUIThreadLoader)
    {
      delete mUIThreadLoader;
    }
  }
}

void Application::StoreWindowPositionSize(PositionSize positionSize)
{
  mWindowPositionSize = positionSize;
}

void Application::ChangePreInitializedWindowInfo()
{
  // Set window name
  auto windowClassName = mEnvironmentOptions->GetWindowClassName();
  auto windowName      = mEnvironmentOptions->GetWindowName();
  if(!windowName.empty())
  {
    mMainWindowName = windowName;
  }
  mMainWindow.SetClass(mMainWindowName, windowClassName);

  // The real screen size may be different from the value of the preinitialized state. Update it.
  Dali::Internal::Adaptor::WindowSystem::UpdateScreenSize();

  int screenWidth, screenHeight;
  Dali::Internal::Adaptor::WindowSystem::GetScreenSize(screenWidth, screenHeight);

  // Set window position / size
  if(mWindowPositionSize != PositionSize(0, 0, 0, 0))
  {
    Dali::DevelWindow::SetPositionSize(mMainWindow, mWindowPositionSize);
  }
  else if(mCommandLineOptions->stageWidth > 0 && mCommandLineOptions->stageHeight > 0)
  {
    // Command line options override environment options and full screen
    mWindowPositionSize.width  = mCommandLineOptions->stageWidth;
    mWindowPositionSize.height = mCommandLineOptions->stageHeight;
    mMainWindow.SetSize(Dali::Window::WindowSize(mWindowPositionSize.width, mWindowPositionSize.height));
  }
  else if(mEnvironmentOptions->GetWindowWidth() && mEnvironmentOptions->GetWindowHeight())
  {
    // Environment options override full screen functionality if command line arguments not provided
    mWindowPositionSize.width  = mEnvironmentOptions->GetWindowWidth();
    mWindowPositionSize.height = mEnvironmentOptions->GetWindowHeight();
    mMainWindow.SetSize(Dali::Window::WindowSize(mWindowPositionSize.width, mWindowPositionSize.height));
  }
  else if(screenWidth != mWindowPositionSize.width || screenHeight != mWindowPositionSize.height)
  {
    // Some apps can receive screen size differently after launching by specifying size in manifest.
    mWindowPositionSize.width  = screenWidth;
    mWindowPositionSize.height = screenHeight;
    mMainWindow.SetSize(Dali::Window::WindowSize(mWindowPositionSize.width, mWindowPositionSize.height));
  }
}

void Application::CreateWindow()
{
  Internal::Adaptor::Window* window;
  WindowData                 windowData;
  windowData.SetTransparency(mMainWindowMode);
  windowData.SetWindowType(mDefaultWindowType);

  DALI_LOG_RELEASE_INFO("Create Default Window");

  WindowSystem::Initialize();
  mIsSystemInitialized = true;

  if(mLaunchpadState != Launchpad::PRE_INITIALIZED)
  {
    if(mWindowPositionSize.width == 0 && mWindowPositionSize.height == 0)
    {
      if(mCommandLineOptions->stageWidth > 0 && mCommandLineOptions->stageHeight > 0)
      {
        // Command line options override environment options and full screen
        mWindowPositionSize.width  = mCommandLineOptions->stageWidth;
        mWindowPositionSize.height = mCommandLineOptions->stageHeight;
      }
      else if(mEnvironmentOptions->GetWindowWidth() && mEnvironmentOptions->GetWindowHeight())
      {
        // Environment options override full screen functionality if command line arguments not provided
        mWindowPositionSize.width  = mEnvironmentOptions->GetWindowWidth();
        mWindowPositionSize.height = mEnvironmentOptions->GetWindowHeight();
      }
    }

    auto windowClassName = mEnvironmentOptions->GetWindowClassName();
    auto windowName      = mEnvironmentOptions->GetWindowName();
    if(!windowName.empty())
    {
      mMainWindowName = windowName;
    }

    windowData.SetPositionSize(mWindowPositionSize);
    window = Internal::Adaptor::Window::New(mMainWindowName, windowClassName, windowData);
  }
  else
  {
    // The position, size and the window name of the pre-initialized application will be updated in ChangePreInitializedWindowInfo()
    // when the real application is launched.
    windowData.SetPositionSize(mWindowPositionSize);
    window = Internal::Adaptor::Window::New("", "", windowData);
  }

  mMainWindow = Dali::Window(window);

  // Quit the application when the window is closed
  GetImplementation(mMainWindow).DeleteRequestSignal().Connect(mSlotDelegate, &Application::Quit);
}

void Application::CreateAdaptor()
{
  DALI_ASSERT_ALWAYS(mMainWindow && "Window required to create adaptor");

  auto graphicsFactory = mAdaptorBuilder->GetGraphicsFactory();

  Integration::SceneHolder sceneHolder = Integration::SceneHolder(&Dali::GetImplementation(mMainWindow));

  mAdaptor = Adaptor::New(graphicsFactory, sceneHolder, mEnvironmentOptions.get());

  Adaptor::GetImplementation(*mAdaptor).SetUseRemoteSurface(mUseRemoteSurface);
}

void Application::CreateAdaptorBuilder()
{
  mAdaptorBuilder = new AdaptorBuilder(*mEnvironmentOptions);
}

void Application::MainLoop()
{
  // Run the application
  mFramework->Run();
}

void Application::Lower()
{
  // Lower the application without quitting it.
  mMainWindow.Lower();
}

void Application::Quit()
{
  // Actually quit the application.
  // Force a call to Quit even if adaptor is not running.
  Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).AddIdle(MakeCallback(this, &Application::QuitFromMainLoop), false, true);
}

void Application::QuitFromMainLoop()
{
  Accessibility::Bridge::GetCurrentBridge()->Terminate();

  mAdaptor->Stop();

  mFramework->Quit();
  // This will trigger OnTerminate(), below, after the main loop has completed.
}

void Application::OnInit()
{
  mEnvironmentOptions = std::unique_ptr<EnvironmentOptions>(new EnvironmentOptions());

  mFramework->AddAbortCallback(MakeCallback(this, &Application::QuitFromMainLoop));

  CreateAdaptorBuilder();
  // If an application was pre-initialized, a window was made in advance
  if(mLaunchpadState == Launchpad::NONE)
  {
    DALI_LOG_RELEASE_INFO("default Window is created in standalone");
    CreateWindow();
  }

  CreateAdaptor();

  if(mLaunchpadState == Launchpad::PRE_INITIALIZED)
  {
    ChangePreInitializedWindowInfo();
  }

  // Run the adaptor
  DALI_TRACE_BEGIN(gTraceFilter, "DALI_APP_ADAPTOR_START");
  mAdaptor->Start();
  DALI_TRACE_END(gTraceFilter, "DALI_APP_ADAPTOR_START");
  Accessibility::Accessible::SetObjectRegistry(mAdaptor->GetObjectRegistry());

  if(!mStylesheet.empty())
  {
    Dali::StyleMonitor::Get().SetTheme(mStylesheet);
  }

  // Wire up the LifecycleController
  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();

  InitSignal().Connect(&GetImplementation(lifecycleController), &LifecycleController::OnInit);
  TerminateSignal().Connect(&GetImplementation(lifecycleController), &LifecycleController::OnTerminate);
  PauseSignal().Connect(&GetImplementation(lifecycleController), &LifecycleController::OnPause);
  ResumeSignal().Connect(&GetImplementation(lifecycleController), &LifecycleController::OnResume);
  ResetSignal().Connect(&GetImplementation(lifecycleController), &LifecycleController::OnReset);
  LanguageChangedSignal().Connect(&GetImplementation(lifecycleController), &LifecycleController::OnLanguageChanged);

  Dali::Application application(this);

  DALI_TRACE_BEGIN(gTraceFilter, "DALI_APP_EMIT_INIT_SIGNAL");
  mInitSignal.Emit(application);
  DALI_TRACE_END(gTraceFilter, "DALI_APP_EMIT_INIT_SIGNAL");

  mAdaptor->NotifySceneCreated();

  // Ensure the join of Font thread at this point
  Dali::TextAbstraction::FontClientJoinFontThreads();
}

void Application::OnTerminate()
{
  // We've been told to quit by AppCore, ecore_x_destroy has been called, need to quit synchronously
  // delete the window as ecore_x has been destroyed by AppCore

  Dali::Application application(this);
  mTerminateSignal.Emit(application);

  if(mAdaptor)
  {
    // Ensure that the render-thread is not using the surface(window) after we delete it
    mAdaptor->Stop();
  }

  mMainWindow.Reset(); // This only resets (clears) the default Window

  // If DALi's UI Thread works, some resources are created in UI Thread, not Main thread.
  // For that case, these resource should be deleted in UI Thread.
  if(mUseUiThread)
  {
    delete mAdaptor;
    delete mAdaptorBuilder;
    WindowSystem::Shutdown();
  }
}

void Application::OnPause()
{
  // A DALi app should handle Pause/Resume events.
  // DALi just delivers the framework Pause event to the application, but not actually pause DALi core.
  // Pausing DALi core only occurs on the Window Hidden framework event
  Dali::Application application(this);
  mPauseSignal.Emit(application);
}

void Application::OnResume()
{
  // Emit the signal first so the application can queue any messages before we do an update/render
  // This ensures we do not just redraw the last frame before pausing if that's not required
  Dali::Application application(this);
  mResumeSignal.Emit(application);

  // DALi just delivers the framework Resume event to the application.
  // Resuming DALi core only occurs on the Window Show framework event

  // Trigger processing of events queued up while paused
  CoreEventInterface& coreEventInterface = Internal::Adaptor::Adaptor::GetImplementation(GetAdaptor());
  coreEventInterface.ProcessCoreEvents();
}

void Application::OnReset()
{
  /*
   * usually, reset callback was called when a caller request to launch this application via aul.
   * because Application class already handled initialization in OnInit(), OnReset do nothing.
   */
  Dali::Application application(this);
  mResetSignal.Emit(application);
}

void Application::OnAppControl(void* data)
{
  Dali::Application application(this);
  mAppControlSignal.Emit(application, data);
}

void Application::OnLanguageChanged()
{
  mAdaptor->NotifyLanguageChanged();
  Dali::Application application(this);
  mLanguageChangedSignal.Emit(application);
}

void Application::OnRegionChanged()
{
  Dali::Application application(this);
  mRegionChangedSignal.Emit(application);
}

void Application::OnBatteryLow(Dali::DeviceStatus::Battery::Status status)
{
  Dali::Application application(this);
  mLowBatterySignal.Emit(status);
}

void Application::OnMemoryLow(Dali::DeviceStatus::Memory::Status status)
{
  Dali::Application application(this);
  mLowMemorySignal.Emit(status);
}

void Application::OnDeviceOrientationChanged(Dali::DeviceStatus::Orientation::Status status)
{
  Dali::Application application(this);
  mDeviceOrientationChangedSignal.Emit(status);
}

void Application::OnSurfaceCreated(Any newSurface)
{
  void* newWindow = AnyCast<void*>(newSurface);
  void* oldWindow = AnyCast<void*>(mMainWindow.GetNativeHandle());
  if(oldWindow != newWindow)
  {
    auto                                 renderSurfaceFactory = Dali::Internal::Adaptor::GetRenderSurfaceFactory();
    std::unique_ptr<WindowRenderSurface> newSurfacePtr        = renderSurfaceFactory->CreateWindowRenderSurface(PositionSize(), newSurface, true);

    mAdaptor->ReplaceSurface(mMainWindow, *newSurfacePtr.release());
  }
}

void Application::OnSurfaceDestroyed(Any surface)
{
}

void Application::OnTaskInit()
{
  Dali::Application application(this);
  mTaskInitSignal.Emit(application);
}

void Application::OnTaskTerminate()
{
  Dali::Application application(this);
  mTaskTerminateSignal.Emit(application);
}

void Application::OnTaskAppControl(void* data)
{
  Dali::Application application(this);
  mTaskAppControlSignal.Emit(application, data);
}

void Application::OnTaskLanguageChanged()
{
  Dali::Application application(this);
  mTaskLanguageChangedSignal.Emit(application);
}

void Application::OnTaskRegionChanged()
{
  Dali::Application application(this);
  mTaskRegionChangedSignal.Emit(application);
}

void Application::OnTaskBatteryLow(Dali::DeviceStatus::Battery::Status status)
{
  Dali::Application application(this);
  mTaskLowBatterySignal.Emit(status);
}

void Application::OnTaskMemoryLow(Dali::DeviceStatus::Memory::Status status)
{
  Dali::Application application(this);
  mTaskLowMemorySignal.Emit(status);
}

void Application::OnTaskDeviceOrientationChanged(Dali::DeviceStatus::Orientation::Status status)
{
  Dali::Application application(this);
  mTaskDeviceOrientationChangedSignal.Emit(status);
}

bool Application::AddIdle(CallbackBase* callback, bool hasReturnValue)
{
  return mAdaptor->AddIdle(callback, hasReturnValue);
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
  if(mAdaptor)
  {
    objectRegistry = mAdaptor->GetObjectRegistry();
  }
  return objectRegistry;
}

Dali::Adaptor& Application::GetAdaptor()
{
  return *mAdaptor;
}

Dali::Window Application::GetWindow()
{
  return mMainWindow;
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
  Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).FlushUpdateMessages();
}

void Application::SetStyleSheet(const std::string& stylesheet)
{
  mStylesheet = stylesheet;
}

void Application::SetCommandLineOptions(int* argc, char** argv[])
{
  delete mCommandLineOptions;

  mCommandLineOptions = new CommandLineOptions(argc, argv);

  mFramework->SetCommandLineOptions(argc, argv);

  if(argc && (*argc > 0))
  {
    // Set mName from command-line args
    mMainWindowName = (*argv)[0];
  }
}

void Application::SetDefaultWindowType(WindowType type)
{
  mDefaultWindowType = type;
  mMainWindow.SetType(type);
}

int32_t Application::GetRenderThreadId() const
{
  if(mAdaptor)
  {
    return Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).GetRenderThreadId();
  }
  return 0;
}

ApplicationPtr Application::GetPreInitializedApplication()
{
  // Reset the handle to decrease the reference count
  ApplicationPtr application = gPreInitializedApplication;
  gPreInitializedApplication.Reset();

  return application;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
