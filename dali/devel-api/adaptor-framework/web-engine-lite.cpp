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

// CLASS HEADER
#include "web-engine-lite.h"

// INTERNAL INCLUDES
#include <dali/internal/web-engine-lite/common/web-engine-lite-impl.h>

namespace Dali
{

WebEngineLite::WebEngineLite()
{
}

WebEngineLite::WebEngineLite( Internal::Adaptor::WebEngineLite* internal )
: BaseHandle( internal )
{
}

WebEngineLite::~WebEngineLite()
{
}

WebEngineLite WebEngineLite::New()
{
  Internal::Adaptor::WebEngineLitePtr webEngineLite = Internal::Adaptor::WebEngineLite::New();

  if( webEngineLite )
  {
    webEngineLite->Initialize();
  }

  return WebEngineLite( webEngineLite.Get() );
}

WebEngineLite::WebEngineLite( const WebEngineLite& webEngineLite )
: BaseHandle( webEngineLite )
{
}

WebEngineLite& WebEngineLite::operator=( const WebEngineLite& webEngineLite )
{
  if( *this != webEngineLite )
  {
    BaseHandle::operator=( webEngineLite );
  }
  return *this;
}

WebEngineLite WebEngineLite::DownCast( BaseHandle handle )
{
  return WebEngineLite( dynamic_cast< Internal::Adaptor::WebEngineLite* >( handle.GetObjectPtr() ) );
}

void WebEngineLite::CreateInstance(int width, int height, int windowX, int windowY, const std::string& locale, const std::string& timezoneID)
{
  GetImplementation( *this ).CreateInstance(width, height, windowX, windowY, locale, timezoneID);
}

void WebEngineLite::DestroyInstance()
{
  GetImplementation( *this ).DestroyInstance();
}

void WebEngineLite::LoadHtml(const std::string& path)
{
  return GetImplementation( *this ).LoadHtml(path);
}

Dali::WebEngineLitePlugin::WebEngineLiteSignalType& WebEngineLite::FinishedSignal()
{
  return GetImplementation( *this ).FinishedSignal();
}

} // namespace Dali;

