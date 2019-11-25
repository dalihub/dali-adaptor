/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#ifdef COMPONENT_APPLICATION_SUPPORT

// CLASS HEADER
#include <dali/internal/adaptor/tizen-wayland/component-application-impl.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/environment-variables.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
ComponentApplicationPtr ComponentApplication::New(
  int* argc,
  char **argv[],
  const std::string& stylesheet,
  Dali::Application::WINDOW_MODE windowMode)
{
  ComponentApplicationPtr application ( new ComponentApplication (argc, argv, stylesheet, windowMode ) );
  return application;
}

ComponentApplication::ComponentApplication( int* argc, char** argv[], const std::string& stylesheet, Dali::Application::WINDOW_MODE windowMode )
: Application(argc, argv, stylesheet, windowMode, PositionSize(), Framework::COMPONENT)
{
}

ComponentApplication::~ComponentApplication()
{
}

component_class_h ComponentApplication::OnCreate(void* user_data)
{
  return mCreateSignal.Emit( user_data );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
#endif
