//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// CLASS HEADER
#include "application-impl.h"

// EXTERNAL INCLUDES
#include <Ecore_X.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/common/style-monitor.h>

// INTERNAL INCLUDES
#include <internal/command-line-options.h>
#include <internal/common/adaptor-impl.h>
#include <internal/common/ecore-x/ecore-x-render-surface.h>

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

boost::thread_specific_ptr<Application> gThreadLocalApplication;
}

ApplicationPtr Application::New(
  int* argc,
  char **argv[],
  const std::string& name,
  const DeviceLayout& baseLayout )
{
  ApplicationPtr application ( new Application (argc, argv, name, baseLayout ) );
  return application;
}

Application::Application(
  int* argc,
  char** argv[],
  const std::string& name,
  const DeviceLayout& baseLayout )
: mFramework(NULL),
  mCommandLineOptions(NULL),
  mAdaptor(NULL),
  mWindow(),
  mName(name),
  mInitialized(false),
  mBaseLayout(baseLayout),
  mSlotDelegate( this )
{
  // make sure we don't create the local thread application instance twice
  DALI_ASSERT_ALWAYS(gThreadLocalApplication.get() == NULL && "Cannot create more than one Application per thread" );

  // reset is used to store a new value associated with this thread
  gThreadLocalApplication.reset(this);

  mCommandLineOptions = new CommandLineOptions(argc, argv);

  mFramework = new Framework(*this, argc, argv, name);
}

Application::~Application()
{
  delete mFramework;
  delete mCommandLineOptions;
  delete mAdaptor;
  mWindow.Reset();
  gThreadLocalApplication.release();
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

  mWindow = Dali::Window::New( windowPosition, mName );
}

void Application::CreateAdaptor()
{
  DALI_ASSERT_ALWAYS( mWindow && "Window required to create adaptor" );

  mAdaptor = &Dali::Adaptor::New( mWindow, mBaseLayout);

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

void Application::MainLoop()
{
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
    Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).DisableVSync();
  }

  mInitialized = true;

  Dali::Application application(this);
  mInitSignalV2.Emit( application );
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

Dali::Application Application::Get()
{
  DALI_ASSERT_ALWAYS( gThreadLocalApplication.get() != NULL && "Application not instantiated" );

  Dali::Application application(gThreadLocalApplication.get());

  return application;
}

const std::string& Application::GetTheme()
{
  return Dali::StyleMonitor::Get().GetTheme();
}

void Application::SetTheme(const std::string& themeFilePath)
{
  return Dali::StyleMonitor::Get().SetTheme(themeFilePath);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
