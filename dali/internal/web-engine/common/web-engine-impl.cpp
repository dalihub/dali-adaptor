/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/internal/web-engine/common/web-engine-impl.h>

// EXTERNAL INCLUDES
#include <dlfcn.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>
#include <dali/public-api/object/type-registry.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace // unnamed namespace
{

const char* WEB_ENGINE_PLUGIN_SO( "libdali-web-engine-plugin.so" );

Dali::BaseHandle Create()
{
  return Dali::WebEngine::New();
}

Dali::TypeRegistration type( typeid( Dali::WebEngine ), typeid( Dali::BaseHandle ), Create );

} // unnamed namespace

WebEnginePtr WebEngine::New()
{
  WebEnginePtr ptr;
  WebEngine* engine = new WebEngine();

  if ( !engine->Initialize() )
  {
    delete engine;
    engine = nullptr;
  }

  ptr = engine;
  return ptr;
}

WebEngine::WebEngine()
: mPlugin( NULL ),
  mHandle( NULL ),
  mCreateWebEnginePtr( NULL ),
  mDestroyWebEnginePtr( NULL )
{
}

WebEngine::~WebEngine()
{
  if( mHandle != NULL )
  {
    if( mDestroyWebEnginePtr != NULL )
    {
      mPlugin->Destroy();
      mDestroyWebEnginePtr( mPlugin );
    }

    dlclose( mHandle );
  }
}

bool WebEngine::Initialize()
{
  char* error = NULL;

  mHandle = dlopen( WEB_ENGINE_PLUGIN_SO, RTLD_LAZY );

  error = dlerror();
  if( mHandle == NULL || error != NULL )
  {
    DALI_LOG_ERROR( "WebEngine::Initialize(), dlopen error: %s\n", error );
    return false;
  }

  mCreateWebEnginePtr = reinterpret_cast< CreateWebEngineFunction >( dlsym( mHandle, "CreateWebEnginePlugin" ) );
  if( mCreateWebEnginePtr == NULL )
  {
    DALI_LOG_ERROR( "Can't load symbol CreateWebEnginePlugin(), error: %s\n", error );
    return false;
  }

  mPlugin = mCreateWebEnginePtr();

  if( mPlugin == NULL )
  {
    DALI_LOG_ERROR( "Can't create the WebEnginePlugin object\n" );
    return false;
  }

  mDestroyWebEnginePtr = reinterpret_cast< DestroyWebEngineFunction >( dlsym( mHandle, "DestroyWebEnginePlugin" ) );

  if( mDestroyWebEnginePtr == NULL )
  {

    DALI_LOG_ERROR( "Can't load symbol DestroyWebEnginePlugin(), error: %s\n", error );
    return false;
  }

  return true;
}

void WebEngine::Create( int width, int height, const std::string& locale, const std::string& timezoneId )
{
  if( mPlugin != NULL )
  {
    mPlugin->Create( width, height, locale, timezoneId );
  }
}

void WebEngine::Destroy()
{
  if( mPlugin != NULL )
  {
    mPlugin->Destroy();
  }
}

Dali::NativeImageInterfacePtr WebEngine::GetNativeImageSource()
{
  return mPlugin->GetNativeImageSource();
}

void WebEngine::LoadUrl( const std::string& url )
{
  if( mPlugin != NULL )
  {
    mPlugin->LoadUrl( url );
  }
}

const std::string& WebEngine::GetUrl()
{
  static std::string emptyUrl;
  return mPlugin ? mPlugin->GetUrl() : emptyUrl;
}

void WebEngine::LoadHTMLString( const std::string& htmlString )
{
  if( mPlugin != NULL )
  {
    mPlugin->LoadHTMLString( htmlString );
  }
}

void WebEngine::Reload()
{
  if( mPlugin != NULL )
  {
    mPlugin->Reload();
  }
}

void WebEngine::StopLoading()
{
  if( mPlugin != NULL )
  {
    mPlugin->StopLoading();
  }
}

bool WebEngine::CanGoForward()
{
  return mPlugin ? mPlugin->CanGoForward() : false;
}

void WebEngine::GoForward()
{
  if( mPlugin != NULL )
  {
    mPlugin->GoForward();
  }
}

bool WebEngine::CanGoBack()
{
  return mPlugin ? mPlugin->CanGoBack() : false;
}

void WebEngine::GoBack()
{
  if( mPlugin != NULL )
  {
    mPlugin->GoBack();
  }
}

void WebEngine::EvaluateJavaScript( const std::string& script )
{
  if( mPlugin != NULL )
  {
    mPlugin->EvaluateJavaScript( script );
  }
}

void WebEngine::AddJavaScriptInterface( const std::string& exposedObjectName, const std::string& jsFunctionName, std::function< std::string(const std::string&) > cb )
{
  if( mPlugin != NULL )
  {
    mPlugin->AddJavaScriptInterface( exposedObjectName, jsFunctionName, cb );
  }
}

void WebEngine::RemoveJavascriptInterface( const std::string& exposedObjectName, const std::string& jsFunctionName )
{
  if( mPlugin != NULL )
  {
    mPlugin->RemoveJavascriptInterface( exposedObjectName, jsFunctionName );
  }
}

void WebEngine::ClearHistory()
{
  if( mPlugin != NULL )
  {
    mPlugin->ClearHistory();
  }
}

void WebEngine::ClearCache()
{
  if( mPlugin != NULL )
  {
    mPlugin->ClearCache();
  }
}

void WebEngine::SetSize( int width, int height )
{
  if( mPlugin != NULL )
  {
    mPlugin->SetSize( width, height );
  }
}

bool WebEngine::SendTouchEvent( const Dali::TouchData& touch )
{
  if( mPlugin != NULL )
  {
    return mPlugin->SendTouchEvent( touch );
  }

  return false;
}

bool WebEngine::SendKeyEvent( const Dali::KeyEvent& event )
{
  if( mPlugin != NULL )
  {
    return mPlugin->SendKeyEvent( event );
  }

  return false;
}

Dali::WebEnginePlugin::WebEngineSignalType& WebEngine::PageLoadStartedSignal()
{
  return mPlugin->PageLoadStartedSignal();
}

Dali::WebEnginePlugin::WebEngineSignalType& WebEngine::PageLoadFinishedSignal()
{
  return mPlugin->PageLoadFinishedSignal();
}

} // namespace Adaptor;
} // namespace Internal;
} // namespace Dali;

