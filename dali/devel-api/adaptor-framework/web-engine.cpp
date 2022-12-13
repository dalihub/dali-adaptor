/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/web-engine-back-forward-list.h>
#include <dali/devel-api/adaptor-framework/web-engine-context.h>
#include <dali/devel-api/adaptor-framework/web-engine-cookie-manager.h>
#include <dali/devel-api/adaptor-framework/web-engine-policy-decision.h>
#include <dali/devel-api/adaptor-framework/web-engine-settings.h>
#include <dali/internal/web-engine/common/web-engine-impl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/images/pixel-data.h>

namespace Dali
{
WebEngine::WebEngine()
{
}

WebEngine::WebEngine(Internal::Adaptor::WebEngine* internal)
: BaseHandle(internal)
{
}

WebEngine::~WebEngine()
{
}

WebEngine WebEngine::New()
{
  Internal::Adaptor::WebEnginePtr engine = Internal::Adaptor::WebEngine::New();

  return WebEngine(engine.Get());
}

WebEngine::WebEngine(const WebEngine& webEngine)
: BaseHandle(webEngine)
{
}

WebEngine& WebEngine::operator=(const WebEngine& webEngine)
{
  if(*this != webEngine)
  {
    BaseHandle::operator=(webEngine);
  }
  return *this;
}

WebEngine WebEngine::DownCast(BaseHandle handle)
{
  return WebEngine(dynamic_cast<Internal::Adaptor::WebEngine*>(handle.GetObjectPtr()));
}

void WebEngine::Create(int width, int height, const std::string& locale, const std::string& timezoneId)
{
  GetImplementation(*this).Create(width, height, locale, timezoneId);
}

void WebEngine::Create(int width, int height, int argc, char** argv)
{
  GetImplementation(*this).Create(width, height, argc, argv);
}

void WebEngine::Destroy()
{
  GetImplementation(*this).Destroy();
}

WebEnginePlugin* WebEngine::GetPlugin() const
{
  return GetImplementation(*this).GetPlugin();
}

NativeImageInterfacePtr WebEngine::GetNativeImageSource()
{
  return GetImplementation(*this).GetNativeImageSource();
}

Dali::WebEngineSettings& WebEngine::GetSettings() const
{
  return GetImplementation(*this).GetSettings();
}

Dali::WebEngineContext& WebEngine::GetContext() const
{
  return GetImplementation(*this).GetContext();
}

Dali::WebEngineCookieManager& WebEngine::GetCookieManager() const
{
  return GetImplementation(*this).GetCookieManager();
}

Dali::WebEngineBackForwardList& WebEngine::GetBackForwardList() const
{
  return GetImplementation(*this).GetBackForwardList();
}

void WebEngine::LoadUrl(const std::string& url)
{
  return GetImplementation(*this).LoadUrl(url);
}

std::string WebEngine::GetTitle() const
{
  return GetImplementation(*this).GetTitle();
}

Dali::PixelData WebEngine::GetFavicon() const
{
  return GetImplementation(*this).GetFavicon();
}

std::string WebEngine::GetUrl() const
{
  return GetImplementation(*this).GetUrl();
}

void WebEngine::LoadHtmlString(const std::string& htmlString)
{
  GetImplementation(*this).LoadHtmlString(htmlString);
}

void WebEngine::Reload()
{
  GetImplementation(*this).Reload();
}

void WebEngine::StopLoading()
{
  GetImplementation(*this).StopLoading();
}

void WebEngine::Suspend()
{
  GetImplementation(*this).Suspend();
}

void WebEngine::Resume()
{
  GetImplementation(*this).Resume();
}

void WebEngine::ScrollBy(int deltaX, int deltaY)
{
  GetImplementation(*this).ScrollBy(deltaX, deltaY);
}

void WebEngine::SetScrollPosition(int x, int y)
{
  GetImplementation(*this).SetScrollPosition(x, y);
}

void WebEngine::GetScrollPosition(int& x, int& y) const
{
  GetImplementation(*this).GetScrollPosition(x, y);
}

void WebEngine::GetScrollSize(int& width, int& height) const
{
  GetImplementation(*this).GetScrollSize(width, height);
}

void WebEngine::GetContentSize(int& width, int& height) const
{
  GetImplementation(*this).GetContentSize(width, height);
}

bool WebEngine::CanGoForward()
{
  return GetImplementation(*this).CanGoForward();
}

void WebEngine::GoForward()
{
  GetImplementation(*this).GoForward();
}

bool WebEngine::CanGoBack()
{
  return GetImplementation(*this).CanGoBack();
}

void WebEngine::GoBack()
{
  GetImplementation(*this).GoBack();
}

void WebEngine::EvaluateJavaScript(const std::string& script, Dali::WebEnginePlugin::JavaScriptMessageHandlerCallback resultHandler)
{
  GetImplementation(*this).EvaluateJavaScript(script, resultHandler);
}

void WebEngine::AddJavaScriptMessageHandler(const std::string& exposedObjectName, Dali::WebEnginePlugin::JavaScriptMessageHandlerCallback handler)
{
  GetImplementation(*this).AddJavaScriptMessageHandler(exposedObjectName, handler);
}

void WebEngine::ClearAllTilesResources()
{
  GetImplementation(*this).ClearAllTilesResources();
}

void WebEngine::ClearHistory()
{
  return GetImplementation(*this).ClearHistory();
}

std::string WebEngine::GetUserAgent() const
{
  return GetImplementation(*this).GetUserAgent();
}

void WebEngine::SetUserAgent(const std::string& userAgent)
{
  GetImplementation(*this).SetUserAgent(userAgent);
}

void WebEngine::SetSize(int width, int height)
{
  return GetImplementation(*this).SetSize(width, height);
}

bool WebEngine::SendTouchEvent(const TouchEvent& touch)
{
  return GetImplementation(*this).SendTouchEvent(touch);
}

bool WebEngine::SendKeyEvent(const KeyEvent& event)
{
  return GetImplementation(*this).SendKeyEvent(event);
}

void WebEngine::SetFocus(bool focused)
{
  GetImplementation(*this).SetFocus(focused);
}

void WebEngine::UpdateDisplayArea(Dali::Rect<int> displayArea)
{
  GetImplementation(*this).UpdateDisplayArea(displayArea);
}

void WebEngine::EnableVideoHole(bool enabled)
{
  GetImplementation(*this).EnableVideoHole(enabled);
}

void WebEngine::RegisterPageLoadStartedCallback(Dali::WebEnginePlugin::WebEnginePageLoadCallback callback)
{
  GetImplementation(*this).RegisterPageLoadStartedCallback(callback);
}

void WebEngine::RegisterPageLoadFinishedCallback(Dali::WebEnginePlugin::WebEnginePageLoadCallback callback)
{
  GetImplementation(*this).RegisterPageLoadFinishedCallback(callback);
}

void WebEngine::RegisterPageLoadErrorCallback(Dali::WebEnginePlugin::WebEnginePageLoadErrorCallback callback)
{
  GetImplementation(*this).RegisterPageLoadErrorCallback(callback);
}

void WebEngine::RegisterScrollEdgeReachedCallback(Dali::WebEnginePlugin::WebEngineScrollEdgeReachedCallback callback)
{
  GetImplementation(*this).RegisterScrollEdgeReachedCallback(callback);
}

void WebEngine::RegisterUrlChangedCallback(Dali::WebEnginePlugin::WebEngineUrlChangedCallback callback)
{
  GetImplementation(*this).RegisterUrlChangedCallback(callback);
}

void WebEngine::RegisterNavigationPolicyDecidedCallback(Dali::WebEnginePlugin::WebEngineNavigationPolicyDecidedCallback callback)
{
  GetImplementation(*this).RegisterNavigationPolicyDecidedCallback(callback);
}

void WebEngine::RegisterNewWindowCreatedCallback(Dali::WebEnginePlugin::WebEngineNewWindowCreatedCallback callback)
{
  GetImplementation(*this).RegisterNewWindowCreatedCallback(callback);
}

void WebEngine::RegisterFrameRenderedCallback(Dali::WebEnginePlugin::WebEngineFrameRenderedCallback callback)
{
  GetImplementation(*this).RegisterFrameRenderedCallback(callback);
}

void WebEngine::GetPlainTextAsynchronously(Dali::WebEnginePlugin::PlainTextReceivedCallback callback)
{
  GetImplementation(*this).GetPlainTextAsynchronously(callback);
}

} // namespace Dali
