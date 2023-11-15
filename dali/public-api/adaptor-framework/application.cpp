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
#include <dali/public-api/adaptor-framework/application.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/object-registry.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/application-impl.h>

namespace Dali
{
Application Application::New()
{
  return New(NULL, NULL);
}

Application Application::New(int* argc, char** argv[])
{
  Internal::Adaptor::ApplicationPtr internal = Internal::Adaptor::Application::GetPreInitializedApplication();
  if(internal)
  {
    // pre-initialized application
    internal->SetCommandLineOptions(argc, argv);
  }
  else
  {
    WindowData windowData;
    windowData.SetTransparency(false);

    internal = Internal::Adaptor::Application::New(argc, argv, "", Internal::Adaptor::Framework::NORMAL, false, windowData);
  }
  return Application(internal.Get());
}

Application Application::New(int* argc, char** argv[], const std::string& stylesheet)
{
  Internal::Adaptor::ApplicationPtr internal = Internal::Adaptor::Application::GetPreInitializedApplication();
  if(internal)
  {
    // pre-initialized application
    internal->SetCommandLineOptions(argc, argv);
    internal->SetStyleSheet(stylesheet);
  }
  else
  {
    WindowData windowData;
    windowData.SetTransparency(false);

    internal = Internal::Adaptor::Application::New(argc, argv, stylesheet, Internal::Adaptor::Framework::NORMAL, false, windowData);
  }
  return Application(internal.Get());
}

Application Application::New(int* argc, char** argv[], const std::string& stylesheet, WINDOW_MODE windowMode)
{
  Internal::Adaptor::ApplicationPtr internal = Internal::Adaptor::Application::GetPreInitializedApplication();
  if(internal)
  {
    // pre-initialized application
    internal->SetCommandLineOptions(argc, argv);
    internal->SetStyleSheet(stylesheet);

    internal->GetWindow().SetTransparency((windowMode == Application::TRANSPARENT));
  }
  else
  {
    WindowData windowData;
    windowData.SetTransparency(windowMode == Application::TRANSPARENT);

    internal = Internal::Adaptor::Application::New(argc, argv, stylesheet, Internal::Adaptor::Framework::NORMAL, false, windowData);
  }
  return Application(internal.Get());
}

Application Application::New(int* argc, char** argv[], const std::string& stylesheet, Application::WINDOW_MODE windowMode, PositionSize positionSize)
{
  Internal::Adaptor::ApplicationPtr internal = Internal::Adaptor::Application::GetPreInitializedApplication();
  if(internal)
  {
    // pre-initialized application
    internal->SetCommandLineOptions(argc, argv);
    internal->SetStyleSheet(stylesheet);

    internal->GetWindow().SetTransparency(windowMode == Application::TRANSPARENT);

    // Store only the value before adaptor is created
    internal->StoreWindowPositionSize(positionSize);
  }
  else
  {
    WindowData windowData;
    windowData.SetPositionSize(positionSize);
    windowData.SetTransparency(windowMode == Application::TRANSPARENT);

    internal = Internal::Adaptor::Application::New(argc, argv, stylesheet, Internal::Adaptor::Framework::NORMAL, false, windowData);
  }
  return Application(internal.Get());
}

Application Application::New(int* argc, char** argv[], const std::string& stylesheet, Application::WINDOW_MODE windowMode, PositionSize positionSize, bool useUiThread)
{
  Internal::Adaptor::ApplicationPtr internal = Internal::Adaptor::Application::GetPreInitializedApplication();
  if(internal)
  {
    // pre-initialized application
    internal->SetCommandLineOptions(argc, argv);
    internal->SetStyleSheet(stylesheet);

    internal->GetWindow().SetTransparency(windowMode == Application::TRANSPARENT);

    // Store only the value before adaptor is created
    internal->StoreWindowPositionSize(positionSize);
  }
  else
  {
    WindowData windowData;
    windowData.SetPositionSize(positionSize);
    windowData.SetTransparency(windowMode == Application::TRANSPARENT);

    internal = Internal::Adaptor::Application::New(argc, argv, stylesheet, Internal::Adaptor::Framework::NORMAL, useUiThread, windowData);
  }
  return Application(internal.Get());
}

Application Application::New(int* argc, char** argv[], const std::string& stylesheet, bool useUiThread, WindowData& windowData)
{
  Internal::Adaptor::ApplicationPtr internal = Internal::Adaptor::Application::GetPreInitializedApplication();
  if(internal)
  {
    // pre-initialized application
    internal->SetCommandLineOptions(argc, argv);
    internal->SetStyleSheet(stylesheet);

    // Set defaut Window type
    internal->SetDefaultWindowType(windowData.GetWindowType());
    internal->GetWindow().SetTransparency(windowData.GetTransparency());

    // Store only the value before adaptor is created
    internal->StoreWindowPositionSize(windowData.GetPositionSize());

    // Set front buffer rendering
    internal->StoreFrontBufferRendering(windowData.GetFrontBufferRendering());
  }
  else
  {
    internal = Internal::Adaptor::Application::New(argc, argv, stylesheet, Internal::Adaptor::Framework::NORMAL, useUiThread, windowData);
  }
  return Application(internal.Get());
}

Application::~Application()
{
}

Application::Application()
{
}

Application::Application(const Application& copy) = default;

Application& Application::operator=(const Application& rhs) = default;

Application::Application(Application&& rhs) noexcept = default;

Application& Application::operator=(Application&& rhs) noexcept = default;

void Application::MainLoop()
{
  Internal::Adaptor::GetImplementation(*this).MainLoop();
}

void Application::Lower()
{
  Internal::Adaptor::GetImplementation(*this).Lower();
}

void Application::Quit()
{
  Internal::Adaptor::GetImplementation(*this).Quit();
}

bool Application::AddIdle(CallbackBase* callback)
{
  return Internal::Adaptor::GetImplementation(*this).AddIdle(callback, false);
}

Window Application::GetWindow()
{
  return Internal::Adaptor::GetImplementation(*this).GetWindow();
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

ObjectRegistry Application::GetObjectRegistry() const
{
  return Internal::Adaptor::GetImplementation(*this).GetObjectRegistry();
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

Application::AppControlSignalType& Application::AppControlSignal()
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

Application::LowBatterySignalType& Application::LowBatterySignal()
{
  return Internal::Adaptor::GetImplementation(*this).LowBatterySignal();
}

Application::LowMemorySignalType& Application::LowMemorySignal()
{
  return Internal::Adaptor::GetImplementation(*this).LowMemorySignal();
}

Application::DeviceOrientationChangedSignalType& Application::DeviceOrientationChangedSignal()
{
  return Internal::Adaptor::GetImplementation(*this).DeviceOrientationChangedSignal();
}

Application::AppSignalType& Application::TaskInitSignal()
{
  return Internal::Adaptor::GetImplementation(*this).TaskInitSignal();
}

Application::AppSignalType& Application::TaskTerminateSignal()
{
  return Internal::Adaptor::GetImplementation(*this).TaskTerminateSignal();
}

Application::AppControlSignalType& Application::TaskAppControlSignal()
{
  return Internal::Adaptor::GetImplementation(*this).TaskAppControlSignal();
}

Application::AppSignalType& Application::TaskLanguageChangedSignal()
{
  return Internal::Adaptor::GetImplementation(*this).TaskLanguageChangedSignal();
}

Application::AppSignalType& Application::TaskRegionChangedSignal()
{
  return Internal::Adaptor::GetImplementation(*this).TaskRegionChangedSignal();
}

Application::LowBatterySignalType& Application::TaskLowBatterySignal()
{
  return Internal::Adaptor::GetImplementation(*this).TaskLowBatterySignal();
}

Application::LowMemorySignalType& Application::TaskLowMemorySignal()
{
  return Internal::Adaptor::GetImplementation(*this).TaskLowMemorySignal();
}

Application::DeviceOrientationChangedSignalType& Application::TaskDeviceOrientationChangedSignal()
{
  return Internal::Adaptor::GetImplementation(*this).TaskDeviceOrientationChangedSignal();
}

Application::Application(Internal::Adaptor::Application* application)
: BaseHandle(application)
{
}

} // namespace Dali
