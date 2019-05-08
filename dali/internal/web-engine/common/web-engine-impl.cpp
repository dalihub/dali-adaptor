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
#include <dali/public-api/object/type-registry.h>
#include <sstream>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace // unnamed namespace
{

constexpr char const * const kPluginFullNamePrefix = "libdali-web-engine-";
constexpr char const * const kPluginFullNamePostfix = "-plugin.so";
constexpr char const * const kPluginFullNameDefault = "libdali-web-engine-plugin.so";

// Note: Dali WebView policy does not allow to use multiple web engines in an application.
// So once pluginName is set to non-empty string, it will not change.
std::string pluginName;

std::string MakePluginName( const char* environmentName )
{
  std::stringstream fullName;
  fullName << kPluginFullNamePrefix << environmentName << kPluginFullNamePostfix;
  return std::move( fullName.str() );
}

Dali::BaseHandle Create()
{
  return Dali::WebEngine::New();
}

Dali::TypeRegistration type( typeid( Dali::WebEngine ), typeid( Dali::BaseHandle ), Create );

} // unnamed namespace

WebEnginePtr WebEngine::New()
{
  WebEngine* instance = new WebEngine();

  if( !instance->Initialize() )
  {
    delete instance;
    return nullptr;
  }

  return instance;
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

bool WebEngine::InitializePluginHandle()
{
  if( pluginName.length() == 0 )
  {
    // pluginName is not initialized yet.
    const char* name = EnvironmentVariable::GetEnvironmentVariable( DALI_ENV_WEB_ENGINE_NAME );
    if( name )
    {
      pluginName = MakePluginName( name );
      mHandle = dlopen( pluginName.c_str(), RTLD_LAZY );
      if( mHandle )
      {
        return true;
      }
    }
    pluginName = std::string( kPluginFullNameDefault );
  }

  mHandle = dlopen( pluginName.c_str(), RTLD_LAZY );
  if( !mHandle )
  {
    DALI_LOG_ERROR( "Can't load %s : %s\n", pluginName.c_str(), dlerror() );
    return false;
  }

  return true;
}

bool WebEngine::Initialize()
{
  char* error = NULL;

  if( !InitializePluginHandle() )
  {
    return false;
  }

  mCreateWebEnginePtr = reinterpret_cast< CreateWebEngineFunction >( dlsym( mHandle, "CreateWebEnginePlugin" ) );
  if( mCreateWebEnginePtr == NULL )
  {
    DALI_LOG_ERROR( "Can't load symbol CreateWebEnginePlugin(), error: %s\n", error );
    return false;
  }

  mDestroyWebEnginePtr = reinterpret_cast< DestroyWebEngineFunction >( dlsym( mHandle, "DestroyWebEnginePlugin" ) );

  if( mDestroyWebEnginePtr == NULL )
  {
    DALI_LOG_ERROR( "Can't load symbol DestroyWebEnginePlugin(), error: %s\n", error );
    return false;
  }

  mPlugin = mCreateWebEnginePtr();

  if( mPlugin == NULL )
  {
    DALI_LOG_ERROR( "Can't create the WebEnginePlugin object\n" );
    return false;
  }

  return true;
}

void WebEngine::Create( int width, int height, const std::string& locale, const std::string& timezoneId )
{
  mPlugin->Create( width, height, locale, timezoneId );
}

void WebEngine::Destroy()
{
  mPlugin->Destroy();
}

Dali::NativeImageInterfacePtr WebEngine::GetNativeImageSource()
{
  return mPlugin->GetNativeImageSource();
}

void WebEngine::LoadUrl( const std::string& url )
{
  mPlugin->LoadUrl( url );
}

const std::string& WebEngine::GetUrl()
{
  return mPlugin->GetUrl();
}

void WebEngine::LoadHTMLString( const std::string& htmlString )
{
  mPlugin->LoadHTMLString( htmlString );
}

void WebEngine::Reload()
{
  mPlugin->Reload();
}

void WebEngine::StopLoading()
{
  mPlugin->StopLoading();
}

void WebEngine::Suspend()
{
  mPlugin->Suspend();
}

void WebEngine::Resume()
{
  mPlugin->Resume();
}

bool WebEngine::CanGoForward()
{
  return mPlugin->CanGoForward();
}

void WebEngine::GoForward()
{
  mPlugin->GoForward();
}

bool WebEngine::CanGoBack()
{
  return mPlugin->CanGoBack();
}

void WebEngine::GoBack()
{
  mPlugin->GoBack();
}

void WebEngine::EvaluateJavaScript( const std::string& script, std::function< void( const std::string& ) > resultHandler )
{
  mPlugin->EvaluateJavaScript( script, resultHandler );
}

void WebEngine::AddJavaScriptMessageHandler( const std::string& exposedObjectName, std::function< void(const std::string&) > handler )
{
  mPlugin->AddJavaScriptMessageHandler( exposedObjectName, handler );
}

void WebEngine::ClearHistory()
{
  mPlugin->ClearHistory();
}

void WebEngine::ClearCache()
{
  mPlugin->ClearCache();
}

void WebEngine::ClearCookies()
{
  mPlugin->ClearCookies();
}

Dali::WebEnginePlugin::CacheModel WebEngine::GetCacheModel() const
{
  return mPlugin->GetCacheModel();
}

void WebEngine::SetCacheModel( Dali::WebEnginePlugin::CacheModel cacheModel )
{
  mPlugin->SetCacheModel( cacheModel );
}

Dali::WebEnginePlugin::CookieAcceptPolicy WebEngine::GetCookieAcceptPolicy() const
{
  return mPlugin->GetCookieAcceptPolicy();
}

void WebEngine::SetCookieAcceptPolicy( Dali::WebEnginePlugin::CookieAcceptPolicy policy )
{
  mPlugin->SetCookieAcceptPolicy( policy );
}

const std::string& WebEngine::GetUserAgent() const
{
  return mPlugin->GetUserAgent();
}

void WebEngine::SetUserAgent( const std::string& userAgent )
{
  mPlugin->SetUserAgent( userAgent );
}

bool WebEngine::IsJavaScriptEnabled() const
{
  return mPlugin->IsJavaScriptEnabled();
}

void WebEngine::EnableJavaScript( bool enabled )
{
  mPlugin->EnableJavaScript( enabled );
}

bool WebEngine::AreImagesAutomaticallyLoaded() const
{
  return mPlugin->AreImagesAutomaticallyLoaded();
}

void WebEngine::LoadImagesAutomatically( bool automatic )
{
  mPlugin->LoadImagesAutomatically( automatic );
}

const std::string& WebEngine::GetDefaultTextEncodingName() const
{
  return mPlugin->GetDefaultTextEncodingName();
}

void WebEngine::SetDefaultTextEncodingName( const std::string& defaultTextEncodingName )
{
  mPlugin->SetDefaultTextEncodingName( defaultTextEncodingName );
}

int WebEngine::GetDefaultFontSize() const
{
  return mPlugin->GetDefaultFontSize();
}

void WebEngine::SetDefaultFontSize( int defaultFontSize )
{
  mPlugin->SetDefaultFontSize( defaultFontSize );
}

void WebEngine::SetSize( int width, int height )
{
  mPlugin->SetSize( width, height );
}

bool WebEngine::SendTouchEvent( const Dali::TouchData& touch )
{
  return mPlugin->SendTouchEvent( touch );
}

bool WebEngine::SendKeyEvent( const Dali::KeyEvent& event )
{
  return mPlugin->SendKeyEvent( event );
}

Dali::WebEnginePlugin::WebEnginePageLoadSignalType& WebEngine::PageLoadStartedSignal()
{
  return mPlugin->PageLoadStartedSignal();
}

Dali::WebEnginePlugin::WebEnginePageLoadSignalType& WebEngine::PageLoadFinishedSignal()
{
  return mPlugin->PageLoadFinishedSignal();
}

Dali::WebEnginePlugin::WebEnginePageLoadErrorSignalType& WebEngine::PageLoadErrorSignal()
{
  return mPlugin->PageLoadErrorSignal();
}

} // namespace Adaptor;
} // namespace Internal;
} // namespace Dali;

