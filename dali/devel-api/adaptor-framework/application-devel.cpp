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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/application-devel.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/adaptor/common/application-impl.h>
#include <dali/internal/network/common/network-service-impl.h>

namespace Dali
{
namespace DevelApplication
{
Application New(int* argc, char** argv[], const std::string& stylesheet, Application::WINDOW_MODE windowMode, PositionSize positionSize, WindowType type)
{
  Internal::Adaptor::ApplicationPtr internal = Internal::Adaptor::Application::GetPreInitializedApplication();
  if(internal)
  {
    // Set Defaut Window type
    internal->SetDefaultWindowType(type);

    // pre-initialized application
    internal->SetCommandLineOptions(argc, argv);
    internal->SetStyleSheet(stylesheet);

    internal->GetWindow().SetTransparency((windowMode == Application::TRANSPARENT));

    // Store only the value before adaptor is created
    internal->StoreWindowPositionSize(positionSize);
  }
  else
  {
    WindowData windowData;
    windowData.SetPositionSize(positionSize);
    windowData.SetTransparency(windowMode == Application::TRANSPARENT);
    windowData.SetWindowType(type);

    internal = Internal::Adaptor::Application::New(argc, argv, stylesheet, Internal::Adaptor::Framework::NORMAL, false, windowData);
  }
  return Application(internal.Get());
}

bool AddIdleWithReturnValue(Application application, CallbackBase* callback)
{
  return Internal::Adaptor::GetImplementation(application).AddIdle(callback, true);
}

std::string GetDataPath()
{
  return Internal::Adaptor::Application::GetDataPath();
}

Application DownCast(Dali::RefObject* refObject)
{
  return Application(dynamic_cast<Dali::Internal::Adaptor::Application*>(refObject));
}

CustomCommandReceivedSignalType& CustomCommandReceivedSignal(Application application)
{
  DALI_ASSERT_ALWAYS(Adaptor::IsAvailable() && "Adaptor is not available")

  Internal::Adaptor::NetworkServicePtr networkService = Internal::Adaptor::NetworkService::Get();

  DALI_ASSERT_ALWAYS(networkService && "Network Service Unavailable");

  return networkService->CustomCommandReceivedSignal();
}

int32_t GetRenderThreadId(Application application)
{
  return Internal::Adaptor::GetImplementation(application).GetRenderThreadId();
}

void FlushUpdateMessages(Application application)
{
  Internal::Adaptor::GetImplementation(application).FlushUpdateMessages();
}

} // namespace DevelApplication

} // namespace Dali

extern "C" void ApplicationPreInitialize(int* argc, char** argv[])
{
  Dali::Internal::Adaptor::Application::PreInitialize(argc, argv);
}
