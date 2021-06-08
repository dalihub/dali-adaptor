/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

// CLASSHEADER
#include <dali/devel-api/adaptor-framework/web-engine.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/web-engine-back-forward-list.h>
#include <dali/devel-api/adaptor-framework/web-engine-certificate.h>
#include <dali/devel-api/adaptor-framework/web-engine-console-message.h>
#include <dali/devel-api/adaptor-framework/web-engine-context-menu-item.h>
#include <dali/devel-api/adaptor-framework/web-engine-context-menu.h>
#include <dali/devel-api/adaptor-framework/web-engine-context.h>
#include <dali/devel-api/adaptor-framework/web-engine-cookie-manager.h>
#include <dali/devel-api/adaptor-framework/web-engine-http-auth-handler.h>
#include <dali/devel-api/adaptor-framework/web-engine-load-error.h>
#include <dali/devel-api/adaptor-framework/web-engine-policy-decision.h>
#include <dali/devel-api/adaptor-framework/web-engine-request-interceptor.h>
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

void WebEngine::Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId)
{
  GetImplementation(*this).Create(width, height, locale, timezoneId);
}

void WebEngine::Create(uint32_t width, uint32_t height, uint32_t argc, char** argv)
{
  GetImplementation(*this).Create(width, height, argc, argv);
}

void WebEngine::Destroy()
{
  GetImplementation(*this).Destroy();
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

const std::string& WebEngine::GetUrl()
{
  return GetImplementation(*this).GetUrl();
}

void WebEngine::LoadHtmlString(const std::string& htmlString)
{
  GetImplementation(*this).LoadHtmlString(htmlString);
}

bool WebEngine::LoadHtmlStringOverrideCurrentEntry(const std::string& html, const std::string& basicUri, const std::string& unreachableUrl)
{
  return GetImplementation(*this).LoadHtmlStringOverrideCurrentEntry(html, basicUri, unreachableUrl);
}

bool WebEngine::LoadContents(const std::string& contents, uint32_t contentSize, const std::string& mimeType, const std::string& encoding, const std::string& baseUri)
{
  return GetImplementation(*this).LoadContents(contents, contentSize, mimeType, encoding, baseUri);
}

void WebEngine::Reload()
{
  GetImplementation(*this).Reload();
}

bool WebEngine::ReloadWithoutCache()
{
  return GetImplementation(*this).ReloadWithoutCache();
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

void WebEngine::SuspendNetworkLoading()
{
  GetImplementation(*this).SuspendNetworkLoading();
}

void WebEngine::ResumeNetworkLoading()
{
  GetImplementation(*this).ResumeNetworkLoading();
}

bool WebEngine::AddCustomHeader(const std::string& name, const std::string& value)
{
  return GetImplementation(*this).AddCustomHeader(name, value);
}

bool WebEngine::RemoveCustomHeader(const std::string& name)
{
  return GetImplementation(*this).RemoveCustomHeader(name);
}

uint32_t WebEngine::StartInspectorServer(uint32_t port)
{
  return GetImplementation(*this).StartInspectorServer(port);
}

bool WebEngine::StopInspectorServer()
{
  return GetImplementation(*this).StopInspectorServer();
}

void WebEngine::ScrollBy(int32_t deltaX, int32_t deltaY)
{
  GetImplementation(*this).ScrollBy(deltaX, deltaY);
}

bool WebEngine::ScrollEdgeBy(int32_t deltaX, int32_t deltaY)
{
  return GetImplementation(*this).ScrollEdgeBy(deltaX, deltaY);
}

void WebEngine::SetScrollPosition(int32_t x, int32_t y)
{
  GetImplementation(*this).SetScrollPosition(x, y);
}

Dali::Vector2 WebEngine::GetScrollPosition() const
{
  return GetImplementation(*this).GetScrollPosition();
}

Dali::Vector2 WebEngine::GetScrollSize() const
{
  return GetImplementation(*this).GetScrollSize();
}

Dali::Vector2 WebEngine::GetContentSize() const
{
  return GetImplementation(*this).GetContentSize();
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

void WebEngine::EvaluateJavaScript(const std::string& script, std::function<void(const std::string&)> resultHandler)
{
  GetImplementation(*this).EvaluateJavaScript(script, resultHandler);
}

void WebEngine::AddJavaScriptMessageHandler(const std::string& exposedObjectName, std::function<void(const std::string&)> handler)
{
  GetImplementation(*this).AddJavaScriptMessageHandler(exposedObjectName, handler);
}

void WebEngine::RegisterJavaScriptAlertCallback(Dali::WebEnginePlugin::JavaScriptAlertCallback callback)
{
  GetImplementation(*this).RegisterJavaScriptAlertCallback(callback);
}

void WebEngine::JavaScriptAlertReply()
{
  GetImplementation(*this).JavaScriptAlertReply();
}

void WebEngine::RegisterJavaScriptConfirmCallback(Dali::WebEnginePlugin::JavaScriptConfirmCallback callback)
{
  GetImplementation(*this).RegisterJavaScriptConfirmCallback(callback);
}

void WebEngine::JavaScriptConfirmReply(bool confirmed)
{
  GetImplementation(*this).JavaScriptConfirmReply(confirmed);
}

void WebEngine::RegisterJavaScriptPromptCallback(Dali::WebEnginePlugin::JavaScriptPromptCallback callback)
{
  GetImplementation(*this).RegisterJavaScriptPromptCallback(callback);
}

void WebEngine::JavaScriptPromptReply(const std::string& result)
{
  GetImplementation(*this).JavaScriptPromptReply(result);
}

std::unique_ptr<Dali::WebEngineHitTest> WebEngine::CreateHitTest(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode)
{
  return GetImplementation(*this).CreateHitTest(x, y, mode);
}

bool WebEngine::CreateHitTestAsynchronously(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode, Dali::WebEnginePlugin::WebEngineHitTestCreatedCallback callback)
{
  return GetImplementation(*this).CreateHitTestAsynchronously(x, y, mode, callback);
}

void WebEngine::ClearHistory()
{
  GetImplementation(*this).ClearHistory();
}

void WebEngine::ClearAllTilesResources()
{
  GetImplementation(*this).ClearAllTilesResources();
}

const std::string& WebEngine::GetUserAgent() const
{
  return GetImplementation(*this).GetUserAgent();
}

void WebEngine::SetUserAgent(const std::string& userAgent)
{
  GetImplementation(*this).SetUserAgent(userAgent);
}

void WebEngine::SetSize(uint32_t width, uint32_t height)
{
  GetImplementation(*this).SetSize(width, height);
}

void WebEngine::SetDocumentBackgroundColor(Dali::Vector4 color)
{
  GetImplementation(*this).SetDocumentBackgroundColor(color);
}

void WebEngine::ClearTilesWhenHidden(bool cleared)
{
  GetImplementation(*this).ClearTilesWhenHidden(cleared);
}

void WebEngine::SetTileCoverAreaMultiplier(float multiplier)
{
  GetImplementation(*this).SetTileCoverAreaMultiplier(multiplier);
}

void WebEngine::EnableCursorByClient(bool enabled)
{
  GetImplementation(*this).EnableCursorByClient(enabled);
}

std::string WebEngine::GetSelectedText() const
{
  return GetImplementation(*this).GetSelectedText();
}

bool WebEngine::SendTouchEvent(const TouchEvent& touch)
{
  return GetImplementation(*this).SendTouchEvent(touch);
}

bool WebEngine::SendKeyEvent(const KeyEvent& event)
{
  return GetImplementation(*this).SendKeyEvent(event);
}

bool WebEngine::SendHoverEvent(const HoverEvent& event)
{
  return GetImplementation(*this).SendHoverEvent(event);
}

bool WebEngine::SendWheelEvent(const WheelEvent& event)
{
  return GetImplementation(*this).SendWheelEvent(event);
}

void WebEngine::SetFocus(bool focused)
{
  GetImplementation(*this).SetFocus(focused);
}

void WebEngine::SetPageZoomFactor(float zoomFactor)
{
  GetImplementation(*this).SetPageZoomFactor(zoomFactor);
}

float WebEngine::GetPageZoomFactor() const
{
  return GetImplementation(*this).GetPageZoomFactor();
}

void WebEngine::SetTextZoomFactor(float zoomFactor)
{
  GetImplementation(*this).SetTextZoomFactor(zoomFactor);
}

float WebEngine::GetTextZoomFactor() const
{
  return GetImplementation(*this).GetTextZoomFactor();
}

float WebEngine::GetLoadProgressPercentage() const
{
  return GetImplementation(*this).GetLoadProgressPercentage();
}

void WebEngine::SetScaleFactor(float scaleFactor, Dali::Vector2 point)
{
  GetImplementation(*this).SetScaleFactor(scaleFactor, point);
}

float WebEngine::GetScaleFactor() const
{
  return GetImplementation(*this).GetScaleFactor();
}

void WebEngine::ActivateAccessibility(bool activated)
{
  GetImplementation(*this).ActivateAccessibility(activated);
}

bool WebEngine::SetVisibility(bool visible)
{
  return GetImplementation(*this).SetVisibility(visible);
}

bool WebEngine::HighlightText(const std::string& text, Dali::WebEnginePlugin::FindOption options, uint32_t maxMatchCount)
{
  return GetImplementation(*this).HighlightText(text, options, maxMatchCount);
}

void WebEngine::AddDynamicCertificatePath(const std::string& host, const std::string& certPath)
{
  GetImplementation(*this).AddDynamicCertificatePath(host, certPath);
}

Dali::PixelData WebEngine::GetScreenshot(Dali::Rect<int32_t> viewArea, float scaleFactor)
{
  return GetImplementation(*this).GetScreenshot(viewArea, scaleFactor);
}

bool WebEngine::GetScreenshotAsynchronously(Dali::Rect<int32_t> viewArea, float scaleFactor, Dali::WebEnginePlugin::ScreenshotCapturedCallback callback)
{
  return GetImplementation(*this).GetScreenshotAsynchronously(viewArea, scaleFactor, callback);
}

bool WebEngine::CheckVideoPlayingAsynchronously(Dali::WebEnginePlugin::VideoPlayingCallback callback)
{
  return GetImplementation(*this).CheckVideoPlayingAsynchronously(callback);
}

void WebEngine::RegisterGeolocationPermissionCallback(Dali::WebEnginePlugin::GeolocationPermissionCallback callback)
{
  GetImplementation(*this).RegisterGeolocationPermissionCallback(callback);
}

void WebEngine::UpdateDisplayArea(Dali::Rect<int32_t> displayArea)
{
  GetImplementation(*this).UpdateDisplayArea(displayArea);
}

void WebEngine::EnableMouseEvents(bool enabled)
{
  GetImplementation(*this).EnableMouseEvents(enabled);
}

void WebEngine::EnableKeyEvents(bool enabled)
{
  GetImplementation(*this).EnableKeyEvents(enabled);
}

void WebEngine::EnableVideoHole(bool enabled)
{
  GetImplementation(*this).EnableVideoHole(enabled);
}

Dali::WebEnginePlugin::WebEnginePageLoadSignalType& WebEngine::PageLoadStartedSignal()
{
  return GetImplementation(*this).PageLoadStartedSignal();
}

Dali::WebEnginePlugin::WebEnginePageLoadSignalType& WebEngine::PageLoadInProgressSignal()
{
  return GetImplementation(*this).PageLoadInProgressSignal();
}

Dali::WebEnginePlugin::WebEnginePageLoadSignalType& WebEngine::PageLoadFinishedSignal()
{
  return GetImplementation(*this).PageLoadFinishedSignal();
}

Dali::WebEnginePlugin::WebEnginePageLoadErrorSignalType& WebEngine::PageLoadErrorSignal()
{
  return GetImplementation(*this).PageLoadErrorSignal();
}

Dali::WebEnginePlugin::WebEngineScrollEdgeReachedSignalType& WebEngine::ScrollEdgeReachedSignal()
{
  return GetImplementation(*this).ScrollEdgeReachedSignal();
}

Dali::WebEnginePlugin::WebEngineUrlChangedSignalType& WebEngine::UrlChangedSignal()
{
  return GetImplementation(*this).UrlChangedSignal();
}

Dali::WebEnginePlugin::WebEngineFormRepostDecisionSignalType& WebEngine::FormRepostDecisionSignal()
{
  return GetImplementation(*this).FormRepostDecisionSignal();
}

Dali::WebEnginePlugin::WebEngineFrameRenderedSignalType& WebEngine::FrameRenderedSignal()
{
  return GetImplementation(*this).FrameRenderedSignal();
}

Dali::WebEnginePlugin::WebEngineRequestInterceptorSignalType& WebEngine::RequestInterceptorSignal()
{
  return GetImplementation(*this).RequestInterceptorSignal();
}

Dali::WebEnginePlugin::WebEngineConsoleMessageSignalType& WebEngine::ConsoleMessageSignal()
{
  return GetImplementation(*this).ConsoleMessageSignal();
}

Dali::WebEnginePlugin::WebEngineResponsePolicyDecisionSignalType& WebEngine::ResponsePolicyDecisionSignal()
{
  return GetImplementation(*this).ResponsePolicyDecisionSignal();
}

Dali::WebEnginePlugin::WebEngineCertificateSignalType& WebEngine::CertificateConfirmSignal()
{
  return GetImplementation(*this).CertificateConfirmSignal();
}

Dali::WebEnginePlugin::WebEngineCertificateSignalType& WebEngine::SslCertificateChangedSignal()
{
  return GetImplementation(*this).SslCertificateChangedSignal();
}

Dali::WebEnginePlugin::WebEngineHttpAuthHandlerSignalType& WebEngine::HttpAuthHandlerSignal()
{
  return GetImplementation(*this).HttpAuthHandlerSignal();
}

Dali::WebEnginePlugin::WebEngineContextMenuCustomizedSignalType& WebEngine::ContextMenuCustomizedSignal()
{
  return GetImplementation(*this).ContextMenuCustomizedSignal();
}

Dali::WebEnginePlugin::WebEngineContextMenuItemSelectedSignalType& WebEngine::ContextMenuItemSelectedSignal()
{
  return GetImplementation(*this).ContextMenuItemSelectedSignal();
}

} // namespace Dali
