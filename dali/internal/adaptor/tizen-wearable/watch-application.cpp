/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/watch/watch-application.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/tizen-wearable/watch-application-impl.h>

namespace Dali
{

WatchApplication WatchApplication::New()
{
  return New( NULL, NULL );
}

WatchApplication WatchApplication::New( int* argc, char **argv[] )
{
  Internal::Adaptor::WatchApplicationPtr internal = Internal::Adaptor::WatchApplication::New( argc, argv, "", OPAQUE );
  return WatchApplication(internal.Get());
}

WatchApplication WatchApplication::New( int* argc, char **argv[], const std::string& stylesheet )
{
  Internal::Adaptor::WatchApplicationPtr internal = Internal::Adaptor::WatchApplication::New( argc, argv, stylesheet, OPAQUE );
  return WatchApplication(internal.Get());
}

WatchApplication::~WatchApplication()
{
}

WatchApplication::WatchApplication()
{
}

WatchApplication::WatchApplication(const WatchApplication& implementation)
: Application(implementation)
{
}

WatchApplication& WatchApplication::operator=(const WatchApplication& application)
{
  if( *this != application )
  {
    BaseHandle::operator=( application );
  }
  return *this;
}

WatchApplication::WatchTimeSignal& WatchApplication::TimeTickSignal()
{
  return Internal::Adaptor::GetImplementation(*this).mTickSignal;
}

WatchApplication::WatchTimeSignal& WatchApplication::AmbientTickSignal()
{
  return Internal::Adaptor::GetImplementation(*this).mAmbientTickSignal;
}

WatchApplication::WatchBoolSignal& WatchApplication::AmbientChangedSignal()
{
  return Internal::Adaptor::GetImplementation(*this).mAmbientChangeSignal;
}

WatchApplication::WatchApplication(Internal::Adaptor::WatchApplication* implementation)
: Application(implementation)
{
}


} // namespace Dali
