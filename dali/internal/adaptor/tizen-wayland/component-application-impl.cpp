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
#include <dali/internal/adaptor/tizen-wayland/component-application-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
ComponentApplicationPtr ComponentApplication::New(
  int*               argc,
  char**             argv[],
  const std::string& stylesheet,
  const WindowData&  windowData)
{
  ComponentApplicationPtr application(new ComponentApplication(argc, argv, stylesheet, windowData));
  return application;
}

ComponentApplication::ComponentApplication(int* argc, char** argv[], const std::string& stylesheet, const WindowData& windowData)
: Application(argc, argv, stylesheet, Framework::COMPONENT, false, windowData)
{
}

Any ComponentApplication::OnCreate()
{
  return mCreateSignal.Emit();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
