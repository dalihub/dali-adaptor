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
#include <dali/devel-api/adaptor-framework/web-engine.h>

// INTERNAL INCLUDES
#include <dali/internal/web-engine/common/web-engine-impl.h>

namespace Dali
{

WebEngine::WebEngine()
{
}

WebEngine::WebEngine( Internal::Adaptor::WebEngine* internal )
: BaseHandle( internal )
{
}

WebEngine::~WebEngine()
{
}

WebEngine WebEngine::New()
{
  Internal::Adaptor::WebEnginePtr engine = Internal::Adaptor::WebEngine::New();

  return WebEngine( engine.Get() );
}

WebEngine::WebEngine( const WebEngine& webEngine )
: BaseHandle( webEngine )
{
}

WebEngine& WebEngine::operator=( const WebEngine& webEngine )
{
  if( *this != webEngine )
  {
    BaseHandle::operator=( webEngine );
  }
  return *this;
}

WebEngine WebEngine::DownCast( BaseHandle handle )
{
  return WebEngine( dynamic_cast< Internal::Adaptor::WebEngine* >( handle.GetObjectPtr() ) );
}

void WebEngine::Create( int width, int height, const std::string& locale, const std::string& timezoneId )
{
  GetImplementation( *this ).Create( width, height, locale, timezoneId );
}

void WebEngine::Destroy()
{
  GetImplementation( *this ).Destroy();
}

NativeImageInterfacePtr WebEngine::GetNativeImageSource()
{
  return GetImplementation( *this ).GetNativeImageSource();
}

void WebEngine::LoadUrl( const std::string& url )
{
  return GetImplementation( *this ).LoadUrl( url );
}

const std::string& WebEngine::GetUrl()
{
  return GetImplementation( *this ).GetUrl();
}

void WebEngine::LoadHTMLString( const std::string& htmlString )
{
  GetImplementation( *this ).LoadHTMLString( htmlString );
}

void WebEngine::Reload()
{
  GetImplementation( *this ).Reload();
}

void WebEngine::StopLoading()
{
  GetImplementation( *this ).StopLoading();
}

bool WebEngine::CanGoForward()
{
  return GetImplementation( *this ).CanGoForward();
}

void WebEngine::GoForward()
{
  GetImplementation( *this ).GoForward();
}

bool WebEngine::CanGoBack()
{
  return GetImplementation( *this ).CanGoBack();
}

void WebEngine::GoBack()
{
  GetImplementation( *this ).GoBack();
}

void WebEngine::EvaluateJavaScript( const std::string& script )
{
  GetImplementation( *this ).EvaluateJavaScript( script );
}

void WebEngine::AddJavaScriptMessageHandler( const std::string& exposedObjectName, std::function< void( const std::string& ) > handler )
{
  GetImplementation( *this ).AddJavaScriptMessageHandler( exposedObjectName, handler );
}

void WebEngine::ClearHistory()
{
  return GetImplementation( *this ).ClearHistory();
}

void WebEngine::ClearCache()
{
  return GetImplementation( *this ).ClearCache();
}

void WebEngine::SetSize( int width, int height )
{
  return GetImplementation( *this ).SetSize( width, height );
}

bool WebEngine::SendTouchEvent( const TouchData& touch )
{
  return GetImplementation( *this ).SendTouchEvent( touch );
}

bool WebEngine::SendKeyEvent( const KeyEvent& event )
{
  return GetImplementation( *this ).SendKeyEvent( event );
}

Dali::WebEnginePlugin::WebEngineSignalType& WebEngine::PageLoadStartedSignal()
{
  return GetImplementation( *this ).PageLoadStartedSignal();
}

Dali::WebEnginePlugin::WebEngineSignalType& WebEngine::PageLoadFinishedSignal()
{
  return GetImplementation( *this ).PageLoadFinishedSignal();
}

} // namespace Dali;

