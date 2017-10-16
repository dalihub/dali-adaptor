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
#include <web-engine-lite-impl.h>

// EXTERNAL INCLUDES
#include <dlfcn.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/type-registry.h>
#include <dali/public-api/object/any.h>


namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace // unnamed namespace
{
const char* WEB_ENGINE_LITE_PLUGIN_SO( "libdali-web-engine-lite-plugin.so" );

Dali::BaseHandle Create()
{
  return Dali::WebEngineLite::New();
}

Dali::TypeRegistration type( typeid( Dali::WebEngineLite ), typeid( Dali::BaseHandle ), Create );

} // unnamed namespace

WebEngineLitePtr WebEngineLite::New()
{
  WebEngineLitePtr webEngineLite = new WebEngineLite();
  return webEngineLite;
}

WebEngineLite::WebEngineLite()
: mPlugin( NULL ),
  mHandle( NULL ),
  mCreateWebEngineLitePtr( NULL ),
  mDestroyWebEngineLitePtr( NULL )
{
}

WebEngineLite::~WebEngineLite()
{
  if( mHandle != NULL )
  {
    if( mDestroyWebEngineLitePtr != NULL )
    {
      mDestroyWebEngineLitePtr( mPlugin );
    }

    dlclose( mHandle );
  }
}

void WebEngineLite::Initialize()
{
  char* error = NULL;

  mHandle = dlopen( WEB_ENGINE_LITE_PLUGIN_SO, RTLD_LAZY );

  error = dlerror();
  if( mHandle == NULL || error != NULL )
  {
    DALI_LOG_ERROR( "WebEngineLite::Initialize(), dlopen error: %s\n", error );
    return;
  }

  mCreateWebEngineLitePtr = reinterpret_cast< CreateWebEngineLiteFunction >( dlsym( mHandle, "CreateWebEngineLitePlugin" ) );
  if( mCreateWebEngineLitePtr == NULL )
  {
    DALI_LOG_ERROR( "Can't load symbol CreateWebEngineLitePlugin(), error: %s\n", error );
    return;
  }

  mPlugin = mCreateWebEngineLitePtr();

  if( mPlugin == NULL )
  {
    DALI_LOG_ERROR( "Can't create the WebEngineLitePlugin object\n" );
    return;
  }

  mDestroyWebEngineLitePtr = reinterpret_cast< DestroyWebEngineLiteFunction >( dlsym( mHandle, "DestroyWebEngineLitePlugin" ) );
  if( mDestroyWebEngineLitePtr == NULL )
  {
    DALI_LOG_ERROR( "Can't load symbol DestroyWebEngineLitePlugin(), error: %s\n", error );
    return;
  }

}

void WebEngineLite::CreateInstance(int width, int height, int windowX, int windowY, const std::string& locale, const std::string& timezoneID)
{
  if( mPlugin != NULL )
  {
    mPlugin->CreateInstance(width, height, windowX, windowY, locale, timezoneID);
  }
}

void WebEngineLite::DestroyInstance()
{
  if( mPlugin != NULL )
  {
    mPlugin->DestroyInstance();
  }
}

void WebEngineLite::LoadHtml(const std::string& path)
{
  if( mPlugin != NULL )
  {
    mPlugin->LoadHtml(path);
  }
}

Dali::WebEngineLitePlugin::WebEngineLiteSignalType& WebEngineLite::FinishedSignal()
{
  if( mPlugin != NULL )
  {
    return mPlugin->FinishedSignal();
  }

  return mFinishedSignal;
}

} // namespace Adaptor;
} // namespace Internal;
} // namespace Dali;

