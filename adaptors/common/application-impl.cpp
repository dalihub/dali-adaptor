/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <style-monitor.h>

// INTERNAL INCLUDES
#include <command-line-options.h>
#include <common/adaptor-impl.h>
#include <singleton-service-impl.h>
#include <lifecycle-controller-impl.h>

namespace Dali
{

namespace SlpPlatform
{
class SlpPlatformAbstraction;
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
// Defaults taken from H2 device
const unsigned int DEFAULT_WINDOW_WIDTH   = 480;
const unsigned int DEFAULT_WINDOW_HEIGHT  = 800;
const float        DEFAULT_HORIZONTAL_DPI = 220;
const float        DEFAULT_VERTICAL_DPI   = 217;
}

ApplicationPtr Application::New(
  int* argc,
  char **argv[],
  const std::string& name,
  const DeviceLayout& baseLayout,
  Dali::Application::WINDOW_MODE windowMode)
{
  ApplicationPtr application ( new Application (argc, argv, name, baseLayout, windowMode ) );
  return application;
}

Application::Application( int* argc, char** argv[], const std::string& name, const DeviceLayout& baseLayout, Dali::Application::WINDOW_MODE windowMode)
: mInitSignalV2(),
  mTerminateSignalV2(),
  mPauseSignalV2(),
  mResumeSignalV2(),
  mResetSignalV2(),
  mResizeSignalV2(),
  mLanguageChangedSignalV2(),
  mEventLoop( NULL ),
  mFramework( NULL ),
  mCommandLineOptions( NULL ),
  mSingletonService( SingletonService::New() ),
  mAdaptor( NULL ),
  mWindow(),
  mWindowMode( windowMode ),
  mName( name ),
  mInitialized( false ),
  mBaseLayout( baseLayout ),
  mSlotDelegate( this )
{
  mCommandLineOptions = new CommandLineOptions(argc, argv);

  mFramework = new Framework(*this, argc, argv, name);
}

Application::~Application()
{
  mSingletonService.UnregisterAll();

  delete mFramework;
  delete mCommandLineOptions;
  delete mAdaptor;
  mWindow.Reset();
}

void Application::CreateWindow()
{
#ifndef __arm__
   PositionSize windowPosition(0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
#else
   PositionSize windowPosition(0, 0, 0, 0);  // this will use full screen
#endif
  if (mCommandLineOptions->stageWidth > 0 && mCommandLineOptions->stageHeight > 0)
  {
    // let the command line options over ride
    windowPosition = PositionSize(0,0,mCommandLineOptions->stageWidth,mCommandLineOptions->stageHeight);
  }

  mWindow = Dali::Window::New( windowPosition, mName, mWindowMode == Dali::Application::TRANSPARENT );
}

void Application::CreateAdaptor()
{
  DALI_ASSERT_ALWAYS( mWindow && "Window required to create adaptor" );

  mAdaptor = &Dali::Adaptor::New( mWindow, mBaseLayout, mContextLossConfiguration );

  // Allow DPI to be overridden from command line.
  unsigned int hDPI=DEFAULT_HORIZONTAL_DPI;
  unsigned int vDPI=DEFAULT_VERTICAL_DPI;

  std::string dpiStr = mCommandLineOptions->stageDPI;
  if(!dpiStr.empty())
  {
    sscanf(dpiStr.c_str(), "%ux%u", &hDPI, &vDPI);
  }
  Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).SetDpi(hDPI, vDPI);

  mAdaptor->ResizedSignal().Connect( mSlotDelegate, &Application::OnResize );
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
  AddIdle(boost::bind(&Application::QuitFromMainLoop, this));
}

void Application::QuitFromMainLoop()
{
  mAdaptor->Stop();

  Dali::Application application(this);
  mTerminateSignalV2.Emit( application );

  mFramework->Quit();
  // This will trigger OnTerminate(), below, after the main loop has completed.
  mInitialized = false;
}

void Application::OnInit()
{
  mFramework->AddAbortCallback(boost::bind(&Application::QuitFromMainLoop, this));

  CreateWindow();
  CreateAdaptor();

  // Run the adaptor
  mAdaptor->Start();

  // Check if user requires no vsyncing and set on X11 Adaptor
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

  mInitialized = true;

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
  mInitSignalV2.Emit( application );

  Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).GetCore().SceneCreated();
}

void Application::OnTerminate()
{
  // we've been told to quit by AppCore, ecore_x_destroy has been called, need to quit synchronously
  // delete the window as ecore_x has been destroyed by AppCore

  mWindow.Reset();
  mInitialized = false;
}

void Application::OnPause()
{
  mAdaptor->Pause();
  Dali::Application application(this);
  mPauseSignalV2.Emit( application );
}

void Application::OnResume()
{
  mAdaptor->Resume();
  Dali::Application application(this);
  mResumeSignalV2.Emit( application );
}

void Application::OnReset()
{
  /*
   * usually, reset callback was called when a caller request to launch this application via aul.
   * because Application class already handled initialization in OnInit(), OnReset do nothing.
   */
  Dali::Application application(this);
  mResetSignalV2.Emit( application );

  mWindow.Raise();
}

void Application::OnLanguageChanged()
{
  mAdaptor->NotifyLanguageChanged();
}

void Application::OnResize(Dali::Adaptor& adaptor)
{
  Dali::Application application(this);
  mResizeSignalV2.Emit( application );
}

bool Application::AddIdle(boost::function<void(void)> callBack)
{
  return mAdaptor->AddIdle(callBack);
}

Dali::Adaptor& Application::GetAdaptor()
{
  return *mAdaptor;
}

Dali::Window Application::GetWindow()
{
  return mWindow;
}

const std::string& Application::GetTheme()
{
  return Dali::StyleMonitor::Get().GetTheme();
}

void Application::SetTheme(const std::string& themeFilePath)
{
  return Dali::StyleMonitor::Get().SetTheme(themeFilePath);
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


void Application::ReplaceWindow(PositionSize windowPosition, const std::string& name)
{
  Dali::Window newWindow = Dali::Window::New( windowPosition, name, mWindowMode == Dali::Application::TRANSPARENT );
  Window& windowImpl = GetImplementation(newWindow);
  windowImpl.SetAdaptor(*mAdaptor);
  newWindow.ShowIndicator(Dali::Window::INVISIBLE);
  Dali::RenderSurface* renderSurface = windowImpl.GetSurface();
  Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).ReplaceSurface(*renderSurface);
  mWindow = newWindow;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
