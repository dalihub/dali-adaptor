/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/adaptor-framework/application.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/application-impl.h>

namespace Dali
{

Application Application::New()
{
  return New( NULL, NULL );
}

Application Application::New( int* argc, char **argv[] )
{
  Internal::Adaptor::ApplicationPtr internal = Internal::Adaptor::Application::GetPreInitializedApplication();
  if( internal )
  {
    if( argc && ( *argc > 0 ) )
    {
      internal->GetWindow().SetClass( (*argv)[0], "" );
    }

    return Application( internal.Get() );
  }
  else
  {
    internal = Internal::Adaptor::Application::New( argc, argv, "", OPAQUE, PositionSize(),
      Internal::Adaptor::Framework::NORMAL);
    return Application(internal.Get());
  }
}

Application Application::New( int* argc, char **argv[], const std::string& stylesheet )
{
  Internal::Adaptor::ApplicationPtr internal = Internal::Adaptor::Application::GetPreInitializedApplication();
  if( internal )
  {
    if( argc && ( *argc > 0 ) )
    {
      internal->GetWindow().SetClass( (*argv)[0], "" );
    }
    internal->SetStyleSheet( stylesheet );

    return Application( internal.Get() );
  }
  else
  {
    internal = Internal::Adaptor::Application::New( argc, argv, stylesheet, OPAQUE, PositionSize(),
      Internal::Adaptor::Framework::NORMAL);
    return Application(internal.Get());
  }
}

Application Application::New( int* argc, char **argv[], const std::string& stylesheet, WINDOW_MODE windowMode )
{
  Internal::Adaptor::ApplicationPtr internal = Internal::Adaptor::Application::GetPreInitializedApplication();
  if( internal )
  {
    if( argc && ( *argc > 0 ) )
    {
      internal->GetWindow().SetClass( (*argv)[0], "" );
    }
    internal->SetStyleSheet( stylesheet );

    internal->GetWindow().SetTransparency( ( windowMode == Application::OPAQUE ? false : true ) );

    return Application( internal.Get() );
  }
  else
  {
    internal = Internal::Adaptor::Application::New( argc, argv, stylesheet, windowMode, PositionSize(),
      Internal::Adaptor::Framework::NORMAL);
    return Application(internal.Get());
  }
}

Application Application::New( int* argc, char **argv[], const std::string& stylesheet, Application::WINDOW_MODE windowMode, PositionSize positionSize )
{
  Internal::Adaptor::ApplicationPtr internal = Internal::Adaptor::Application::GetPreInitializedApplication();
  if( internal )
  {
    if( argc && ( *argc > 0 ) )
    {
      internal->GetWindow().SetClass( (*argv)[0], "" );
    }
    internal->SetStyleSheet( stylesheet );

    internal->GetWindow().SetTransparency( ( windowMode == Application::OPAQUE ? false : true ) );
    internal->GetWindow().SetSize( Window::WindowSize( positionSize.width, positionSize.height ) );
    internal->GetWindow().SetPosition( Window::WindowPosition( positionSize.x, positionSize.y ) );

    return Application( internal.Get() );
  }
  else
  {
    internal = Internal::Adaptor::Application::New( argc, argv, stylesheet, windowMode, positionSize, Internal::Adaptor::Framework::NORMAL );
    return Application( internal.Get() );
  }
}

Application::~Application()
{
}

Application::Application()
{
}

Application::Application(const Application& application)
: BaseHandle(application)
{
}

Application& Application::operator=(const Application& application)
{
  if( *this != application )
  {
    BaseHandle::operator=( application );
  }
  return *this;
}

void Application::MainLoop()
{
  Internal::Adaptor::GetImplementation(*this).MainLoop(Configuration::APPLICATION_HANDLES_CONTEXT_LOSS);
}

void Application::MainLoop(Configuration::ContextLoss configuration)
{
  Internal::Adaptor::GetImplementation(*this).MainLoop(configuration);
}

void Application::Lower()
{
  Internal::Adaptor::GetImplementation(*this).Lower();
}

void Application::Quit()
{
  Internal::Adaptor::GetImplementation(*this).Quit();
}

bool Application::AddIdle( CallbackBase* callback )
{
  return Internal::Adaptor::GetImplementation(*this).AddIdle( callback, false );
}

Window Application::GetWindow()
{
  return Internal::Adaptor::GetImplementation(*this).GetWindow();
}

void Application::ReplaceWindow(PositionSize windowPosition, const std::string& name)
{
  Internal::Adaptor::GetImplementation(*this).ReplaceWindow(windowPosition, name);
}

std::string Application::GetResourcePath()
{
  return Internal::Adaptor::Application::GetResourcePath();
}

std::string Application::GetRegion() const
{
  return Internal::Adaptor::GetImplementation(*this).GetRegion();
}

std::string Application::GetLanguage() const
{
  return Internal::Adaptor::GetImplementation(*this).GetLanguage();
}

void Application::SetViewMode( ViewMode viewMode )
{
  Internal::Adaptor::GetImplementation(*this).SetViewMode( viewMode );
}

ViewMode Application::GetViewMode() const
{
  return Internal::Adaptor::GetImplementation(*this).GetViewMode();
}

void Application::SetStereoBase( float stereoBase )
{
  Internal::Adaptor::GetImplementation(*this).SetStereoBase( stereoBase );
}

float Application::GetStereoBase() const
{
  return Internal::Adaptor::GetImplementation(*this).GetStereoBase();
}

Application::AppSignalType& Application::InitSignal()
{
  return Internal::Adaptor::GetImplementation(*this).InitSignal();
}

Application::AppSignalType& Application::TerminateSignal()
{
  return Internal::Adaptor::GetImplementation(*this).TerminateSignal();
}

Application::AppSignalType& Application::PauseSignal()
{
  return Internal::Adaptor::GetImplementation(*this).PauseSignal();
}

Application::AppSignalType& Application::ResumeSignal()
{
  return Internal::Adaptor::GetImplementation(*this).ResumeSignal();
}

Application::AppSignalType& Application::ResetSignal()
{
  return Internal::Adaptor::GetImplementation(*this).ResetSignal();
}

Application::AppSignalType& Application::ResizeSignal()
{
  DALI_LOG_WARNING_NOFN( "DEPRECATION WARNING: ResizeSignal() is deprecated and will be removed from next release. Use Window::ResizedSignal() instead.\n" );

  return Internal::Adaptor::GetImplementation(*this).ResizeSignal();
}

Application::AppControlSignalType & Application::AppControlSignal()
{
  return Internal::Adaptor::GetImplementation(*this).AppControlSignal();
}

Application::AppSignalType& Application::LanguageChangedSignal()
{
  return Internal::Adaptor::GetImplementation(*this).LanguageChangedSignal();
}

Application::AppSignalType& Application::RegionChangedSignal()
{
  return Internal::Adaptor::GetImplementation(*this).RegionChangedSignal();
}

Application::AppSignalType& Application::BatteryLowSignal()
{
  DALI_LOG_WARNING_NOFN( "DEPRECATION WARNING: BatteryLowSignal() is deprecated and will be removed from next release. Use Application::LowBatterySignal() instead.\n" );
  return Internal::Adaptor::GetImplementation(*this).BatteryLowSignal();
}

Application::AppSignalType& Application::MemoryLowSignal()
{
  DALI_LOG_WARNING_NOFN( "DEPRECATION WARNING: MemoryLowSignal() is deprecated and will be removed from next release. Use Application::LowMemorySignal() instead.\n" );
  return Internal::Adaptor::GetImplementation(*this).MemoryLowSignal();
}

Application::LowBatterySignalType& Application::LowBatterySignal()
{
  return Internal::Adaptor::GetImplementation(*this).LowBatterySignal();
}

Application::LowMemorySignalType& Application::LowMemorySignal()
{
  return Internal::Adaptor::GetImplementation(*this).LowMemorySignal();
}

Application::Application(Internal::Adaptor::Application* application)
: BaseHandle(application)
{
}


} // namespace Dali
