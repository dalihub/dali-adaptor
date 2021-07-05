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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/application-devel.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>
#include <dali/internal/adaptor/common/application-impl.h>

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
    if(argc && (*argc > 0))
    {
      internal->GetWindow().SetClass((*argv)[0], "");
    }
    internal->SetStyleSheet(stylesheet);

    internal->GetWindow().SetTransparency((windowMode == Application::OPAQUE ? false : true));

    //Store only the value before adaptor is created
    internal->StoreWindowPositionSize(positionSize);
  }
  else
  {
    internal = Internal::Adaptor::Application::New(argc, argv, stylesheet, windowMode, positionSize, Internal::Adaptor::Framework::NORMAL, type);
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

} // namespace DevelApplication

} // namespace Dali

extern "C" void ApplicationPreInitialize(int* argc, char** argv[])
{
  Dali::Internal::Adaptor::Application::PreInitialize(argc, argv);
}
