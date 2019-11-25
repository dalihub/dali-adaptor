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
#include <dali/devel-api/adaptor-framework/component-application.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/tizen-wayland/component-application-impl.h>

namespace Dali
{

ComponentApplication ComponentApplication::New()
{
  return New( NULL, NULL );
}

ComponentApplication ComponentApplication::New( int* argc, char **argv[] )
{
  Internal::Adaptor::ComponentApplicationPtr internal = Internal::Adaptor::ComponentApplication::New( argc, argv, "", OPAQUE );
  return ComponentApplication(internal.Get());
}

ComponentApplication ComponentApplication::New( int* argc, char **argv[], const std::string& stylesheet )
{
  Internal::Adaptor::ComponentApplicationPtr internal = Internal::Adaptor::ComponentApplication::New( argc, argv, stylesheet, OPAQUE );
  return ComponentApplication(internal.Get());
}

ComponentApplication::~ComponentApplication()
{
}

ComponentApplication::ComponentApplication()
{
}

ComponentApplication::ComponentApplication(const ComponentApplication& implementation)
: Application(implementation)
{
}

ComponentApplication& ComponentApplication::operator=(const ComponentApplication& application)
{
  if( *this != application )
  {
    BaseHandle::operator=( application );
  }
  return *this;
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

#endif
