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

// INTERNAL INCLUDES
#include <adaptors/devel-api/adaptor-framework/application-devel.h>
#include <adaptors/common/application-impl.h>

namespace Dali
{

namespace DevelApplication
{

Application New( int* argc, char **argv[], const std::string& stylesheet, Application::WINDOW_MODE windowMode, PositionSize positionSize )
{
  Internal::Adaptor::ApplicationPtr internal = Internal::Adaptor::Application::New( argc, argv, stylesheet, windowMode, positionSize, Internal::Adaptor::Framework::NORMAL );
  return Application( internal.Get() );
}

} // namespace DevelApplication

} // namespace Dali
