/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/object-registry.h>
#include <dali/integration-api/debug.h>
#include <dali/devel-api/common/singleton-service.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/style-monitor.h>
#include <dali/devel-api/text-abstraction/font-client.h>
#include <dali/devel-api/adaptor-framework/accessibility-impl.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/command-line-options.h>
#include <dali/internal/adaptor/common/framework.h>
#include <dali/internal/adaptor/common/lifecycle-controller-impl.h>
#include <dali/internal/window-system/common/window-impl.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/common/render-surface-factory.h>

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

ApplicationPtr Application::gPreInitializedApplication( NULL );

ApplicationPtr Application::New(
  int* argc,
  char **argv[],
  const std::string& stylesheet,
  Dali::Application::WINDOW_MODE windowMode,
  const PositionSize& positionSize,
  Framework::Type applicationType)
{
  ApplicationPtr application ( new Application (argc, argv, stylesheet, windowMode, positionSize, applicationType ) );
  return application;
}

void Application::PreInitialize( int* argc, char** argv[] )
{
  if( !gPreInitializedApplication )
  {
    Dali::TextAbstraction::FontClientPreInitialize();

    gPreInitializedApplication = new Application ( argc, argv, "", Dali::Application::OPAQUE, PositionSize(), Framework::NORMAL );
    gPreInitializedApplication->CreateWindow();    // Only create window
    gPreInitializedApplication->mLaunchpadState = Launchpad::PRE_INITIALIZED;
  }
}

Application::Application( int* argc, char** argv[], const std::string& stylesheet,
  Dali::Application::WINDOW_MODE windowMode, const PositionSize& positionSize, Framework::Type applicationType )
: mInitSignal(),
  mTerminateSignal(),
  mPauseSignal(),
  mResumeSignal(),
  mResetSignal(),
  mAppControlSignal(),
  mLanguageChangedSignal(),
  mRegionChangedSignal(),
  mEventLoop( nullptr ),
  mFramework( nullptr ),
  mCommandLineOptions( nullptr ),
  mAdaptorBuilder( nullptr ),
  mAdaptor( nullptr ),
  mMainWindow(),
  mMainWindowMode( windowMode ),
  mMainWindowName(),
  mStylesheet( stylesheet ),
  mEnvironmentOptions(),
  mWindowPositionSize( positionSize ),
  mLaunchpadState( Launchpad::NONE ),
  mSlotDelegate( this )
{
  // Get mName from environment options
  mMainWindowName = mEnvironmentOptions.GetWindowName();
  if( mMainWindowName.empty() && argc && ( *argc > 0 ) )
  {
    // Set mName from command-line args if environment option not set
    mMainWindowName = (*argv)[0];
  }

  mCommandLineOptions = new CommandLineOptions(argc, argv);
  mFramework = new Framework( *this, argc, argv, applicationType );
  mUseRemoteSurface = (applicationType == Framework::WATCH);
}

Application::~Application()
{
  SingletonService service = SingletonService::Get();
  // Note this can be false i.e. if Application has never created a Core instance
  if( service )
  {
    service.UnregisterAll();
  }

  mMainWindow.Reset();
  delete mAdaptor;
  delete mAdaptorBuilder;
  delete mCommandLineOptions;
  delete mFramework;
}

void Application::CreateWindow()
{
  if( mWindowPositionSize.width == 0 && mWindowPositionSize.height == 0 )
  {
    if( mCommandLineOptions->stageWidth > 0 && mCommandLineOptions->stageHeight > 0 )
    {
      // Command line options override environment options and full screen
      mWindowPositionSize.width = mCommandLineOptions->stageWidth;
      mWindowPositionSize.height = mCommandLineOptions->stageHeight;
    }
    else if( mEnvironmentOptions.GetWindowWidth() && mEnvironmentOptions.GetWindowHeight() )
    {
      // Environment options override full screen functionality if command line arguments not provided
      mWindowPositionSize.width = mEnvironmentOptions.GetWindowWidth();
      mWindowPositionSize.height = mEnvironmentOptions.GetWindowHeight();
    }
  }

  const std::string& windowClassName = mEnvironmentOptions.GetWindowClassName();

  Internal::Adaptor::Window* window = Internal::Adaptor::Window::New(mWindowPositionSize, mMainWindowName, windowClassName, mMainWindowMode == Dali::Application::TRANSPARENT);
  mMainWindow = Dali::Window( window );

  // Quit the application when the window is closed
  GetImplementation( mMainWindow ).DeleteRequestSignal().Connect( mSlotDelegate, &Application::Quit );
}

void Application::CreateAdaptor()
{
  DALI_ASSERT_ALWAYS( mMainWindow && "Window required to create adaptor" );

  auto graphicsFactory = mAdaptorBuilder->GetGraphicsFactory();

  Integration::SceneHolder sceneHolder = Integration::SceneHolder( &Dali::GetImplementation( mMainWindow ) );

  mAdaptor = Adaptor::New( graphicsFactory, sceneHolder, &mEnvironmentOptions );

  Adaptor::GetImplementation( *mAdaptor ).SetUseRemoteSurface( mUseRemoteSurface );
}

void Application::CreateAdaptorBuilder()
{
  mAdaptorBuilder = new AdaptorBuilder();
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
  Internal::Adaptor::Adaptor::GetImplementation(*mAdaptor).AddIdle( MakeCallback( this, &Application::QuitFromMainLoop ), false, true );
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
  mFramework->AddAbortCallback( MakeCallback( this, &Application::QuitFromMainLoop ) );

  CreateAdaptorBuilder();
  // If an application was pre-initialized, a window was made in advance
  if( mLaunchpadState == Launchpad::NONE )
  {
    CreateWindow();
  }

  CreateAdaptor();

  // Run the adaptor
  mAdaptor->Start();
  Accessibility::Accessible::SetObjectRegistry(mAdaptor->GetObjectRegistry());

  if( ! mStylesheet.empty() )
  {
    Dali::StyleMonitor::Get().SetTheme( mStylesheet );
  }

  // Wire up the LifecycleController
  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();

  InitSignal().Connect( &GetImplementation( lifecycleController ), &LifecycleController::OnInit );
  TerminateSignal().Connect( &GetImplementation( lifecycleController ), &LifecycleController::OnTerminate );
  PauseSignal().Connect( &GetImplementation( lifecycleController ), &LifecycleController::OnPause );
  ResumeSignal().Connect( &GetImplementation( lifecycleController ), &LifecycleController::OnResume );
  ResetSignal().Connect( &GetImplementation( lifecycleController ), &LifecycleController::OnReset );
  LanguageChangedSignal().Connect( &GetImplementation( lifecycleController ), &LifecycleController::OnLanguageChanged );

  Dali::Application application(this);
  mInitSignal.Emit( application );

  mAdaptor->NotifySceneCreated();
}

void Application::OnTerminate()
{
  // We've been told to quit by AppCore, ecore_x_destroy has been called, need to quit synchronously
  // delete the window as ecore_x has been destroyed by AppCore

  Dali::Application application(this);
  mTerminateSignal.Emit( application );

  if( mAdaptor )
  {
    // Ensure that the render-thread is not using the surface(window) after we delete it
    mAdaptor->Stop();
  }

  mMainWindow.Reset(); // This only resets (clears) the default Window
}

void Application::OnPause()
{
  // A DALi app should handle Pause/Resume events.
  // DALi just delivers the framework Pause event to the application, but not actually pause DALi core.
  // Pausing DALi core only occurs on the Window Hidden framework event
  Dali::Application application(this);
  mPauseSignal.Emit( application );
}

void Application::OnResume()
{
  // Emit the signal first so the application can queue any messages before we do an update/render
  // This ensures we do not just redraw the last frame before pausing if that's not required
  Dali::Application application(this);
  mResumeSignal.Emit( application );

  // DALi just delivers the framework Resume event to the application.
  // Resuming DALi core only occurs on the Window Show framework event

  // Trigger processing of events queued up while paused
  CoreEventInterface& coreEventInterface = Internal::Adaptor::Adaptor::GetImplementation( GetAdaptor() );
  coreEventInterface.ProcessCoreEvents();
}

void Application::OnReset()
{
  /*
   * usually, reset callback was called when a caller request to launch this application via aul.
   * because Application class already handled initialization in OnInit(), OnReset do nothing.
   */
  Dali::Application application(this);
  mResetSignal.Emit( application );
}

void Application::OnAppControl(void *data)
{
  Dali::Application application(this);
  mAppControlSignal.Emit( application , data );
}

void Application::OnLanguageChanged()
{
  mAdaptor->NotifyLanguageChanged();
  Dali::Application application(this);
  mLanguageChangedSignal.Emit( application );
}

void Application::OnRegionChanged()
{
  Dali::Application application(this);
  mRegionChangedSignal.Emit( application );
}

void Application::OnBatteryLow( Dali::DeviceStatus::Battery::Status status )
{
  Dali::Application application(this);
  mLowBatterySignal.Emit( status );
}

void Application::OnMemoryLow( Dali::DeviceStatus::Memory::Status status )
{
  Dali::Application application(this);
  mLowMemorySignal.Emit( status );
}

void Application::OnSurfaceCreated( Any newSurface )
{
  void* newWindow = AnyCast< void* >( newSurface );
  void* oldWindow = AnyCast< void* >( mMainWindow.GetNativeHandle() );
  if( oldWindow != newWindow )
  {
    auto renderSurfaceFactory = Dali::Internal::Adaptor::GetRenderSurfaceFactory();
    std::unique_ptr< WindowRenderSurface > newSurfacePtr
      = renderSurfaceFactory->CreateWindowRenderSurface( PositionSize(), newSurface, true );

    mAdaptor->ReplaceSurface( mMainWindow, *newSurfacePtr.release() );
  }
}

void Application::OnSurfaceDestroyed( Any surface )
{
}

bool Application::AddIdle( CallbackBase* callback, bool hasReturnValue )
{
  return mAdaptor->AddIdle( callback, hasReturnValue );
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
  if( mAdaptor )
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
  return Internal::Adaptor::Framework::GetResourcePath();
}

std::string Application::GetDataPath()
{
  return Internal::Adaptor::Framework::GetDataPath();
}

void Application::SetStyleSheet( const std::string& stylesheet )
{
  mStylesheet = stylesheet;
}

void Application::SetCommandLineOptions( int* argc, char **argv[] )
{
  delete mCommandLineOptions;

  mCommandLineOptions = new CommandLineOptions( argc, argv );

  mFramework->SetCommandLineOptions( argc, argv );
}

ApplicationPtr Application::GetPreInitializedApplication()
{
  return gPreInitializedApplication;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
