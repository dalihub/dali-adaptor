/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/common/graphics-backend-impl.h> ///< For Dali::Graphics::Internal::IsGraphicsBackendSet and etc
#include <dali/internal/graphics/common/graphics-factory-interface.h>
#include <dali/internal/graphics/common/graphics-factory.h> ///< For Dali::Internal::Adaptor::ResetGraphicsLibrary
#include <dali/internal/system/common/command-line-options.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/system/common/system-settings.h>
#include <dali/internal/window-system/common/render-surface-factory.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/window-system.h>
#include <dali/public-api/adaptor-framework/graphics-backend.h>

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
#ifdef UI_THREAD_AVAILABLE
const char* TIZEN_UI_THREAD_ENV = "TIZEN_UI_THREAD";
#endif
DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_APPLICATION, true);
} // namespace

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
    bool isUseUIThread = false;

#ifdef UI_THREAD_AVAILABLE
    const char* retEnv = Dali::EnvironmentVariable::GetEnvironmentVariable(TIZEN_UI_THREAD_ENV);
    if(retEnv)
    {
      std::string uiThreadEnv   = retEnv;
      std::string enabledString = "true";
      if(uiThreadEnv == enabledString)
      {
        isUseUIThread = true;
      }
    }
#endif

    Dali::TextAbstraction::FontClientPreInitialize();
    WindowData windowData;

#ifdef DALI_PROFILE_UBUNTU
    windowData.SetTransparency(false); ///< Transparent window is not supported in Ubuntu platform. Let we set to false by default.
#endif

    gPreInitializedApplication                  = new Application(argc, argv, "", Framework::NORMAL, isUseUIThread, windowData);
    gPreInitializedApplication->mLaunchpadState = Launchpad::PRE_INITIALIZED;

#ifdef UI_THREAD_AVAILABLE
    if(isUseUIThread)
    {
      DALI_LOG_RELEASE_INFO("PRE_INITIALIZED with UI Threading\n");
      gPreInitializedApplication->mUIThreadLoader = new UIThreadLoader(argc, argv);
      gPreInitializedApplication->mUIThreadLoader->Run([&]()
      {
        gPreInitializedApplication->CreateWindow();

#ifdef PREINITIALIZE_ADAPTOR_CREATION_ENABLED
        // Start Adaptor now...? TODO : Please check it is valid thread.
        // TODO : POC for create view at preinitialize timing.
        gPreInitializedApplication->CreateAdaptor();
#endif // PREINITIALIZE_ADAPTOR_CREATION_ENABLED
      });
    }
    else
#endif
    {
      DALI_LOG_RELEASE_INFO("Only PRE_INITIALIZED\n");
      gPreInitializedApplication->CreateWindow(); // Only create window
#ifdef PREINITIALIZE_ADAPTOR_CREATION_ENABLED
      gPreInitializedApplication->CreateAdaptor();
#endif // PREINITIALIZE_ADAPTOR_CREATION_ENABLED
    }
  }
}

Dali::Window Application::GetPreInitializeWindow()
{
  Dali::Window result;
  if(gPreInitializedApplication)
  {
    result = gPreInitializedApplication->GetWindow();
  }
  return result;
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
  mAdaptor(nullptr),
  mEnvironmentOptions(nullptr),
  mMainWindow(),
  mMainWindowMode(windowData.GetTransparency() ? WINDOW_MODE::TRANSPARENT : WINDOW_MODE::OPAQUE),
  mMainWindowName(),
  mIsMainWindowFrontBufferRendering(windowData.GetFrontBufferRendering()),
  mStylesheet(stylesheet),
  mWindowPositionSize(windowData.GetPositionSize()),
  mLaunchpadState(Launchpad::NONE),
  mDefaultWindowType(windowData.GetWindowType()),
  mUseUiThread(useUiThread),
  mIsSystemInitialized(false),
  mSlotDelegate(this),
  mUIThreadLoader(nullptr),
  mScreen(windowData.GetScreen())
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
  DALI_LOG_RELEASE_INFO("Application::~Application\n");
  SingletonService service = SingletonService::Get();
  // Note this can be false i.e. if Application has never created a Core instance
  if(service)
  {
    service.UnregisterAll();
  }

  if(mMainWindow)
  {
    DALI_LOG_RELEASE_INFO("Application terminate not comes. Main window reset now\n");
    mMainWindow.Reset();
  }

  delete mCommandLineOptions;

  // Application is created in Main thread whether UI Threading is enabled or not.
  // But some resources are created in Main thread or UI thread.
  // The below code is for the resource are created in Main thread.
  if(!mUseUiThread)
  {
    delete mAdaptor;
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
  DALI_LOG_RELEASE_INFO("Application::StoreWindowPositionSize, (%d,%d)(%d x %d)\n", mWindowPositionSize.x, mWindowPositionSize.y, mWindowPositionSize.width, mWindowPositionSize.height);
  if(mMainWindow)
  {
    if(mWindowPositionSize != PositionSize(0, 0, 0, 0))
    {
      Dali::Internal::Adaptor::Window& windowImpl = Dali::GetImplementation(mMainWindow);
      windowImpl.SetUserGeometryPolicy();
    }
  }
}

void Application::StoreFrontBufferRendering(bool enable)
{
  mIsMainWindowFrontBufferRendering = enable;
}

void Application::StoreWindowScreen(const std::string& screen)
{
  mScreen = screen;
  if(!mScreen.empty())
  {
    DALI_LOG_RELEASE_INFO("Application::StoreWindowScreen, (%s)\n", mScreen.c_str());
  }
  if(mMainWindow)
  {
    Dali::Internal::Adaptor::Window& windowImpl = Dali::GetImplementation(mMainWindow);
    windowImpl.SetScreen(mScreen);
  }
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

  // Set front buffer rendering
  Dali::DevelWindow::SetFrontBufferRendering(mMainWindow, mIsMainWindowFrontBufferRendering);
}

void Application::CreateWindow()
{
  Internal::Adaptor::Window* window;
  WindowData                 windowData;
  windowData.SetTransparency(mMainWindowMode);
  windowData.SetWindowType(mDefaultWindowType);
  windowData.SetFrontBufferRendering(mIsMainWindowFrontBufferRendering);
  windowData.SetScreen(mScreen);

  DALI_LOG_RELEASE_INFO("Create Default Window\n");

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
    // Get environments now, for check preference graphics backend when we create window.
    if(!mEnvironmentOptions)
    {
      mEnvironmentOptions = std::unique_ptr<EnvironmentOptions>(new EnvironmentOptions());

      // Backend could be changed.
      Graphics::Internal::SetPreferredGraphicsBackend(mEnvironmentOptions->GetGraphicsBackend());
    }

    // The position, size, window name, and frontbuffering of the pre-initialized application
    // will be updated in ChangePreInitializedWindowInfo() when the real application is launched.
    windowData.SetPositionSize(mWindowPositionSize);
    window = Internal::Adaptor::Window::New("", "", windowData);
  }

  mMainWindow = Dali::Window(window);
  DALI_LOG_RELEASE_INFO("Main window created done\n");

  // Quit the application when the window is closed
  GetImplementation(mMainWindow).DeleteRequestSignal().Connect(mSlotDelegate, &Application::Quit);
}

void Application::CreateAdaptor()
{
  DALI_ASSERT_ALWAYS(mMainWindow && "Window required to create adaptor");

  DALI_LOG_RELEASE_INFO("CreateAdaptor!\n");

  DALI_ASSERT_ALWAYS(mEnvironmentOptions && "Should have environment options before create adaptor!");

  auto& adaptorBuilder  = AdaptorBuilder::Get(*mEnvironmentOptions);
  auto& graphicsFactory = adaptorBuilder.GetGraphicsFactory();

  Integration::SceneHolder sceneHolder = Integration::SceneHolder(&Dali::GetImplementation(mMainWindow));

  mAdaptor = Adaptor::New(graphicsFactory, sceneHolder, mEnvironmentOptions.get());

  Adaptor::GetImplementation(*mAdaptor).SetUseRemoteSurface(mUseRemoteSurface);

  // adaptorBuilder invalidate after now.
  AdaptorBuilder::Finalize();
}

void Application::UpdateEnvironmentOptions()
{
  if(mLaunchpadState == Launchpad::NONE)
  {
    DALI_ASSERT_ALWAYS(!mEnvironmentOptions && "Should not call OnInit() multiple times");
    mEnvironmentOptions = std::unique_ptr<EnvironmentOptions>(new EnvironmentOptions());
  }
  else if(mLaunchpadState == Launchpad::PRE_INITIALIZED)
  {
    // PreInitialize case.
    DALI_ASSERT_ALWAYS(mEnvironmentOptions && "Should have environment options at pre-initialized phase");

    DALI_ASSERT_ALWAYS(mMainWindow && "Window required to create pre-initialized case");

    // Compare with previous environment options, and replace it.
    auto latestEnvironmentOptions = std::unique_ptr<EnvironmentOptions>(new EnvironmentOptions());

#ifdef PREINITIALIZE_ADAPTOR_CREATION_ENABLED
    DALI_LOG_RELEASE_INFO("pre-initialized Application with pre-initialized Adaptor\n");

    auto& mainWindowImpl = GetImplementation(mMainWindow);
    DALI_ASSERT_ALWAYS(mainWindowImpl.GetSurface() && "Surface should be initialized pre-initialized case");

    if(DALI_UNLIKELY(mainWindowImpl.GetSurface()->GetSurfaceType() == Dali::Integration::RenderSurfaceInterface::NATIVE_RENDER_SURFACE))
    {
      DALI_LOG_RELEASE_INFO("Re-create DisplayConnector by native render surface\n");
      Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).GenerateDisplayConnector(mainWindowImpl.GetSurface()->GetSurfaceType());
    }

    Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).UpdateEnvironmentOptions(*latestEnvironmentOptions);
#else
    mEnvironmentOptions->CopyEnvironmentOptions(*latestEnvironmentOptions);
#endif // PREINITIALIZE_ADAPTOR_CREATION_ENABLED
  }

  // Call will be ignored if this function has already been called by the application.
  if(!Graphics::Internal::IsGraphicsBackendSet())
  {
    Graphics::SetGraphicsBackend(mEnvironmentOptions->GetGraphicsBackend());
  }

  DALI_ASSERT_ALWAYS(!(mLaunchpadState == Launchpad::NONE && Graphics::Internal::IsGraphicsResetRequired()) && "Normal launch case should never call preferred graphics backend!");

  // Reload graphics library if need
  if(DALI_UNLIKELY(mLaunchpadState == Launchpad::PRE_INITIALIZED && Graphics::Internal::IsGraphicsResetRequired()))
  {
    // dlclose for previous loader and re-load if dynamic graphics backed case.
    Dali::Internal::Adaptor::ResetGraphicsLibrary();

    // Fix the graphics backend as current graphics now.
    Graphics::Internal::GraphicsResetCompleted();
  }
}

void Application::CompleteAdaptorAndWindowCreate()
{
  // If an application was pre-initialized, a window was made in advance
  if(mLaunchpadState == Launchpad::NONE)
  {
    DALI_LOG_RELEASE_INFO("default Window is created in standalone\n");
    CreateWindow();

    CreateAdaptor();
    DALI_LOG_RELEASE_INFO("Standalone Application with standalone Adaptor\n");
  }
  else if(mLaunchpadState == Launchpad::PRE_INITIALIZED)
  {
#ifdef PREINITIALIZE_ADAPTOR_CREATION_ENABLED
    // Send to Core that pre-initialized adaptor ready to used.
    Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).PreInitializeAdaptorCompleted();
#else
    // Must create adaptor before change pre-initialized windows
    CreateAdaptor();

    DALI_LOG_RELEASE_INFO("pre-initialized Application with standalone Adaptor\n");
#endif // PREINITIALIZE_ADAPTOR_CREATION_ENABLED

    ChangePreInitializedWindowInfo();
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
  mMainWindow.Lower();
}

void Application::Quit()
{
  DALI_LOG_RELEASE_INFO("Application::Quit requested!\n");
  // Actually quit the application.
  // Force a call to Quit even if adaptor is not running.
  Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).AddIdle(MakeCallback(this, &Application::QuitFromMainLoop), false);
}

void Application::QuitFromMainLoop()
{
  DALI_LOG_RELEASE_INFO("Application::Quit processing\n");
  Accessibility::Bridge::GetCurrentBridge()->Terminate();

  mAdaptor->Stop();

  mFramework->Quit();
  // This will trigger OnTerminate(), below, after the main loop has completed.
  DALI_LOG_RELEASE_INFO("Application::Quit finished\n");
}

void Application::OnInit()
{
  DALI_LOG_RELEASE_INFO("Application::OnInit\n");

  // Let we get or update as latest environment options.
  UpdateEnvironmentOptions();

  mFramework->AddAbortCallback(MakeCallback(this, &Application::QuitFromMainLoop));

  // Let we ensure that window and adaptor created now.
  CompleteAdaptorAndWindowCreate();

  // Run the adaptor
  DALI_TRACE_BEGIN(gTraceFilter, "DALI_APP_ADAPTOR_START");
  mAdaptor->Start();
  DALI_TRACE_END(gTraceFilter, "DALI_APP_ADAPTOR_START");

  // Initialize StyleMonitor here.
  Dali::StyleMonitor styleMonitor = Dali::StyleMonitor::Get();
  if(!mStylesheet.empty())
  {
    styleMonitor.SetTheme(mStylesheet);
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

  DALI_TRACE_BEGIN(gTraceFilter, "DALI_APP_EMIT_PRE_INIT_SIGNAL");
  GetImplementation(lifecycleController).OnPreInit(application);
  DALI_TRACE_END(gTraceFilter, "DALI_APP_EMIT_PRE_INIT_SIGNAL");

  DALI_TRACE_BEGIN(gTraceFilter, "DALI_APP_EMIT_INIT_SIGNAL");
  mInitSignal.Emit(application);
  DALI_TRACE_END(gTraceFilter, "DALI_APP_EMIT_INIT_SIGNAL");

  mAdaptor->NotifySceneCreated();

  // Ensure the join of Font thread at this point
  Dali::TextAbstraction::FontClientJoinFontThreads();
}

void Application::OnTerminate()
{
  DALI_LOG_RELEASE_INFO("Application::OnTerminate\n");
  // We've been told to quit by AppCore, ecore_x_destroy has been called, need to quit synchronously
  // delete the window as ecore_x has been destroyed by AppCore

  Dali::Application application(this);
  mTerminateSignal.Emit(application);

  if(mAdaptor)
  {
    // Ensure that the render-thread is not using the surface(window) after we delete it
    mAdaptor->Stop();
  }

  if(mMainWindow)
  {
    DALI_LOG_RELEASE_INFO("Main window reset at app terminated case\n");
    mMainWindow.Reset(); // This only resets (clears) the default Window
  }

  // If DALi's UI Thread works, some resources are created in UI Thread, not Main thread.
  // For that case, these resource should be deleted in UI Thread.
  if(mUseUiThread)
  {
    delete mAdaptor;
    WindowSystem::Shutdown();
  }
}

void Application::OnPause()
{
  DALI_LOG_RELEASE_INFO("Application::OnPause\n");
  Accessibility::Bridge::GetCurrentBridge()->ApplicationPaused();

  // A DALi app should handle Pause/Resume events.
  // DALi just delivers the framework Pause event to the application, but not actually pause DALi core.
  // Pausing DALi core only occurs on the Window Hidden framework event
  Dali::Application application(this);
  mPauseSignal.Emit(application);
}

void Application::OnResume()
{
  DALI_LOG_RELEASE_INFO("Application::OnResume\n");
  Accessibility::Bridge::GetCurrentBridge()->ApplicationResumed();

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
  DALI_LOG_RELEASE_INFO("Application::OnReset\n");
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
  mAdaptor->NotifyLanguageChanged();
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

void Application::SetApplicationLocale(const std::string& locale)
{
  Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).SetApplicationLocale(locale);
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
