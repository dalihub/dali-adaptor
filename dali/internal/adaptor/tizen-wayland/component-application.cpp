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
#include <dali/devel-api/adaptor-framework/component-application.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/tizen-wayland/component-application-impl.h>

namespace Dali
{
ComponentApplication ComponentApplication::New()
{
  return New(NULL, NULL);
}

ComponentApplication ComponentApplication::New(int* argc, char** argv[])
{
  return New(argc, argv, "");
}

ComponentApplication ComponentApplication::New(int* argc, char** argv[], const std::string& stylesheet)
{
  WindowData                                 windowData;
  Internal::Adaptor::ComponentApplicationPtr internal = Internal::Adaptor::ComponentApplication::New(argc, argv, stylesheet, windowData);
  return ComponentApplication(internal.Get());
}

ComponentApplication::CreateSignalType& ComponentApplication::CreateSignal()
{
  return Internal::Adaptor::GetImplementation(*this).mCreateSignal;
}

ComponentApplication::ComponentApplication(Internal::Adaptor::ComponentApplication* implementation)
: Application(implementation)
{
}

} // namespace Dali
