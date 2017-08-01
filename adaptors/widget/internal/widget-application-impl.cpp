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
#include "widget-application-impl.h"

// EXTERNAL INCLUDES
#include <appcore_multiwindow_base.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <style-monitor.h>
#include <command-line-options.h>
#include <common/adaptor-impl.h>
#include <common/framework.h>
#include <singleton-service-impl.h>
#include <window-impl.h>

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

WidgetApplicationPtr WidgetApplication::New(
  int* argc,
  char **argv[],
  const std::string& stylesheet)
{
  WidgetApplicationPtr widgetApplication( new WidgetApplication (argc, argv, stylesheet ) );
  return widgetApplication;
}

WidgetApplication::WidgetApplication( int* argc, char** argv[], const std::string& stylesheet )
: mInitSignal(),
  mTerminateSignal(),
  mLanguageChangedSignal(),
  mRegionChangedSignal(),
  mBatteryLowSignal(),
  mMemoryLowSignal(),
  mFramework( NULL ),
  mContextLossConfiguration( Configuration::APPLICATION_DOES_NOT_HANDLE_CONTEXT_LOSS ),
  mCommandLineOptions( NULL ),
  mSingletonService( SingletonService::New() ),
  mAdaptor( NULL ),
  mName(),
  mStylesheet( stylesheet ),
  mEnvironmentOptions(),
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
  mFramework = new Framework( *this, argc, argv, Framework::WIDGET );
}

WidgetApplication::~WidgetApplication()
{
  mSingletonService.UnregisterAll();

  delete mAdaptor;
  delete mCommandLineOptions;
  delete mFramework;
}

void WidgetApplication::CreateAdaptor()
{
  mAdaptor = Dali::Internal::Adaptor::Adaptor::New( mWindow, mContextLossConfiguration, &mEnvironmentOptions );

  Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).SetUseRemoteSurface( true );
}

void WidgetApplication::MainLoop()
{
  // Run the application
  mFramework->Run();
}

void WidgetApplication::Quit()
{
  // Actually quit the application.
  AddIdle( MakeCallback( this, &WidgetApplication::QuitFromMainLoop ) );
}

void WidgetApplication::QuitFromMainLoop()
{
  mAdaptor->Stop();

  mFramework->Quit();
  // This will trigger OnTerminate(), below, after the main loop has completed.
}

void WidgetApplication::DoInit()
{
  mWindow = Dali::Window::New( PositionSize(), "", mEnvironmentOptions.GetWindowClassName(), 1 );
  // Quit the application when the window is closed
  mWindow.ShowIndicator(Dali::Window::IndicatorVisibleMode::INVISIBLE);
  GetImplementation( mWindow ).DeleteRequestSignal().Connect( mSlotDelegate, &WidgetApplication::Quit );

  CreateAdaptor();
  // Run the adaptor
  mAdaptor->Start();

  // Check if user requires no vsyncing and set Adaptor
  if (mCommandLineOptions->noVSyncOnRender)
  {
    mAdaptor->SetUseHardwareVSync(false);
  }

  Internal::Adaptor::Adaptor::GetImplementation( *mAdaptor ).SetStereoBase( mCommandLineOptions->stereoBase );

  if( ! mStylesheet.empty() )
  {
    Dali::StyleMonitor::Get().SetTheme( mStylesheet );
  }
}

void WidgetApplication::DoStart()
{
  mAdaptor->NotifySceneCreated();
}

void WidgetApplication::DoTerminate()
{
  if( mAdaptor )
  {
    // Ensure that the render-thread is not using the surface(window) after we delete it
    mAdaptor->Stop();
  }

  mWindow.Reset();
}

void WidgetApplication::DoLanguageChange()
{
  mAdaptor->NotifyLanguageChanged();
}

void WidgetApplication::OnInit()
{
  mFramework->AddAbortCallback( MakeCallback( this, &WidgetApplication::QuitFromMainLoop ) );
  DoInit();

  Dali::WidgetApplication widgetApplication(this);
  mInitSignal.Emit( widgetApplication );

  DoStart();
}

void WidgetApplication::OnTerminate()
{
  Dali::WidgetApplication widgetApplication(this);
  mTerminateSignal.Emit( widgetApplication );

  DoTerminate();
}

void WidgetApplication::OnLanguageChanged()
{
  DoLanguageChange();

  Dali::WidgetApplication widgetApplication(this);
  mLanguageChangedSignal.Emit( widgetApplication );
}

void WidgetApplication::OnRegionChanged()
{
  Dali::WidgetApplication widgetApplication(this);
  mRegionChangedSignal.Emit( widgetApplication );
}

void WidgetApplication::OnBatteryLow()
{
  Dali::WidgetApplication widgetApplication(this);
  mBatteryLowSignal.Emit( widgetApplication );
}

void WidgetApplication::OnMemoryLow()
{
  Dali::WidgetApplication widgetApplication(this);
  mMemoryLowSignal.Emit( widgetApplication );
}

bool WidgetApplication::AddIdle( CallbackBase* callback )
{
  return mAdaptor->AddIdle( callback );
}

Dali::Window WidgetApplication::GetWindow()
{
  return mWindow;
}

Dali::Adaptor& WidgetApplication::GetAdaptor()
{
  return *mAdaptor;
}

std::string WidgetApplication::GetResourcePath()
{
  return Internal::Adaptor::Framework::GetResourcePath();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
