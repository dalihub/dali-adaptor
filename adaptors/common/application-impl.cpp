/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include "application-impl.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <style-monitor.h>
#include <command-line-options.h>
#include <common/adaptor-impl.h>
#include <common/framework.h>
#include <singleton-service-impl.h>
#include <lifecycle-controller-impl.h>

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

Application::Application( int* argc, char** argv[], const std::string& stylesheet,
  Dali::Application::WINDOW_MODE windowMode, const PositionSize& positionSize, Framework::Type applicationType )
: mInitSignal(),
  mTerminateSignal(),
  mPauseSignal(),
  mResumeSignal(),
  mResetSignal(),
  mResizeSignal(),
  mAppControlSignal(),
  mLanguageChangedSignal(),
  mRegionChangedSignal(),
  mBatteryLowSignal(),
  mMemoryLowSignal(),
  mEventLoop( NULL ),
  mFramework( NULL ),
  mContextLossConfiguration( Configuration::APPLICATION_DOES_NOT_HANDLE_CONTEXT_LOSS ),
  mCommandLineOptions( NULL ),
  mSingletonService( SingletonService::New() ),
  mAdaptor( NULL ),
  mWindow(),
  mWindowMode( windowMode ),
  mName(),
  mStylesheet( stylesheet ),
  mEnvironmentOptions(),
  mWindowPositionSize( positionSize ),
  mSlotDelegate( this )
{
  // Get mName from environment options
  mName = mEnvironmentOptions.GetWindowName();
  if( mName.empty() && argc && ( *argc > 0 ) )
  {
    // Set mName from command-line args if environment option not set
    mName = (*argv)[0];
  }

  mCommandLineOptions = new CommandLineOptions(argc, argv);
  mFramework = new Framework( *this, argc, argv, applicationType );
  mUseRemoteSurface = (applicationType == Framework::WATCH);
}

Application::~Application()
{
  mSingletonService.UnregisterAll();

  mWindow.Reset();
  delete mAdaptor;
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
  mWindow = Dali::Window::New( mWindowPositionSize, mName, windowClassName, mWindowMode == Dali::Application::TRANSPARENT );

  // Quit the application when the window is closed
  GetImplementation( mWindow ).DeleteRequestSignal().Connect( mSlotDelegate, &Application::Quit );
}

void Application::CreateAdaptor()
{
  DALI_ASSERT_ALWAYS( mWindow && "Window required to create adaptor" );

  mAdaptor = Dali::Internal::Adaptor::Adaptor::New( mWindow, mContextLossConfiguration, &mEnvironmentOptions );

  mAdaptor->ResizedSignal().Connect( mSlotDelegate, &Application::OnResize );

  Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).SetUseRemoteSurface( mUseRemoteSurface );
}

void Application::MainLoop(Dali::Configuration::ContextLoss configuration)
{
  mContextLossConfiguration = configuration;

  // Run the application
  mFramework->Run();
}

void Application::Lower()
{
  // Lower the application without quitting it.
  mWindow.Lower();
}

void Application::Quit()
{
  // Actually quit the application.
  AddIdle( MakeCallback( this, &Application::QuitFromMainLoop ) );
}

void Application::QuitFromMainLoop()
{
  mAdaptor->Stop();

  mFramework->Quit();
  // This will trigger OnTerminate(), below, after the main loop has completed.
}

void Application::DoInit()
{
  CreateWindow();
  CreateAdaptor();

  // Run the adaptor
  mAdaptor->Start();

  // Check if user requires no vsyncing and set Adaptor
  if (mCommandLineOptions->noVSyncOnRender)
  {
    mAdaptor->SetUseHardwareVSync(false);
  }

  Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).SetStereoBase( mCommandLineOptions->stereoBase );
  if( mCommandLineOptions->viewMode != 0 )
  {
    ViewMode viewMode = MONO;
    if( mCommandLineOptions->viewMode <= STEREO_INTERLACED )
    {
      viewMode = static_cast<ViewMode>( mCommandLineOptions->viewMode );
    }
    Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).SetViewMode( viewMode );
  }

  if( ! mStylesheet.empty() )
  {
    Dali::StyleMonitor::Get().SetTheme( mStylesheet );
  }
}

void Application::DoStart()
{
  mAdaptor->NotifySceneCreated();
}

void Application::DoTerminate()
{
  if( mAdaptor )
  {
    // Ensure that the render-thread is not using the surface(window) after we delete it
    mAdaptor->Stop();
  }

  mWindow.Reset();
}

void Application::DoPause()
{
  mAdaptor->Pause();
}

void Application::DoResume()
{
  mAdaptor->Resume();
}

void Application::DoLanguageChange()
{
  mAdaptor->NotifyLanguageChanged();
}

void Application::OnInit()
{
  mFramework->AddAbortCallback( MakeCallback( this, &Application::QuitFromMainLoop ) );

  DoInit();

  // Wire up the LifecycleController
  Dali::LifecycleController lifecycleController = Dali::LifecycleController::Get();

  InitSignal().Connect( &GetImplementation( lifecycleController ), &LifecycleController::OnInit );
  TerminateSignal().Connect( &GetImplementation( lifecycleController ), &LifecycleController::OnTerminate );
  PauseSignal().Connect( &GetImplementation( lifecycleController ), &LifecycleController::OnPause );
  ResumeSignal().Connect( &GetImplementation( lifecycleController ), &LifecycleController::OnResume );
  ResetSignal().Connect( &GetImplementation( lifecycleController ), &LifecycleController::OnReset );
  ResizeSignal().Connect( &GetImplementation( lifecycleController ), &LifecycleController::OnResize );
  LanguageChangedSignal().Connect( &GetImplementation( lifecycleController ), &LifecycleController::OnLanguageChanged );

  Dali::Application application(this);
  mInitSignal.Emit( application );

  DoStart();
}

void Application::OnTerminate()
{
  // we've been told to quit by AppCore, ecore_x_destroy has been called, need to quit synchronously
  // delete the window as ecore_x has been destroyed by AppCore

  Dali::Application application(this);
  mTerminateSignal.Emit( application );

  DoTerminate();
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
  DoLanguageChange();
  Dali::Application application(this);
  mLanguageChangedSignal.Emit( application );
}

void Application::OnRegionChanged()
{
  Dali::Application application(this);
  mRegionChangedSignal.Emit( application );
}

void Application::OnBatteryLow()
{
  Dali::Application application(this);
  mBatteryLowSignal.Emit( application );
}

void Application::OnMemoryLow()
{
  Dali::Application application(this);
  mMemoryLowSignal.Emit( application );
}

void Application::OnResize(Dali::Adaptor& adaptor)
{
  Dali::Application application(this);
  mResizeSignal.Emit( application );
}

bool Application::AddIdle( CallbackBase* callback )
{
  return mAdaptor->AddIdle( callback );
}

Dali::Adaptor& Application::GetAdaptor()
{
  return *mAdaptor;
}

Dali::Window Application::GetWindow()
{
  return mWindow;
}

// Stereoscopy

void Application::SetViewMode( ViewMode viewMode )
{
  Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).SetViewMode( viewMode );
}

ViewMode Application::GetViewMode() const
{
  return Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).GetViewMode();
}

void Application::SetStereoBase( float stereoBase )
{
  Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).SetStereoBase( stereoBase );
}

float Application::GetStereoBase() const
{
  return Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).GetStereoBase();
}


void Application::ReplaceWindow( const PositionSize& positionSize, const std::string& name )
{
  Dali::Window newWindow = Dali::Window::New( positionSize, name, mWindowMode == Dali::Application::TRANSPARENT );
  Window& windowImpl = GetImplementation(newWindow);
  windowImpl.SetAdaptor(*mAdaptor);
  newWindow.ShowIndicator(Dali::Window::INVISIBLE);
  Dali::RenderSurface* renderSurface = windowImpl.GetSurface();

  Any nativeWindow = newWindow.GetNativeHandle();
  Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).SurfaceSizeChanged( Dali::Adaptor::SurfaceSize( positionSize.width, positionSize.height ) );
  Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).ReplaceSurface(nativeWindow, *renderSurface);
  mWindow = newWindow;
  mWindowPositionSize = positionSize;
}

std::string Application::GetResourcePath()
{
  return Internal::Adaptor::Framework::GetResourcePath();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
