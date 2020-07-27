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
#include <dali/internal/offscreen/common/offscreen-application-impl.h>

// CLASS HEADER
#include <dali/devel-api/adaptor-framework/offscreen-application.h>

namespace Dali
{
OffscreenApplication OffscreenApplication::New( uint16_t width, uint16_t height, bool isTranslucent )
{
  Dali::Any surface;
  IntrusivePtr< Internal::OffscreenApplication > impl = Internal::OffscreenApplication::New( width, height, surface, isTranslucent );

  OffscreenApplication offscreenApplication = OffscreenApplication( impl.Get() );

  return offscreenApplication;
}

OffscreenApplication OffscreenApplication::New( Dali::Any surface, bool isTranslucent )
{
  IntrusivePtr< Internal::OffscreenApplication > impl = Internal::OffscreenApplication::New( 0, 0, surface, isTranslucent );

  OffscreenApplication offscreenApplication = OffscreenApplication( impl.Get() );

  return offscreenApplication;
}

OffscreenApplication::OffscreenApplication()
{
}

OffscreenApplication::OffscreenApplication( const OffscreenApplication& offscreenApplication )
: BaseHandle( offscreenApplication )
{
}

OffscreenApplication& OffscreenApplication::operator=( const OffscreenApplication& offscreenApplication )
{
  if( *this != offscreenApplication )
  {
    BaseHandle::operator=( offscreenApplication );
  }
  return *this;
}

OffscreenApplication::~OffscreenApplication()
{
}

void OffscreenApplication::Run()
{
  Internal::GetImplementation( *this ).Run();
}

void OffscreenApplication::Stop()
{
  Internal::GetImplementation( *this ).Stop();
}

Dali::OffscreenWindow OffscreenApplication::GetWindow()
{
  return Internal::GetImplementation( *this ).GetWindow();
}

OffscreenApplication::OffscreenApplicationSignalType& OffscreenApplication::InitSignal()
{
  return Internal::GetImplementation( *this ).InitSignal();
}

OffscreenApplication::OffscreenApplicationSignalType& OffscreenApplication::TerminateSignal()
{
  return Internal::GetImplementation( *this ).TerminateSignal();
}

OffscreenApplication::OffscreenApplication( Internal::OffscreenApplication* offscreenApplication )
: BaseHandle( offscreenApplication )
{
}

}  // namespace Dali
