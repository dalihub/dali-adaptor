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

// CLASS HEADER
#include <dali/internal/web-engine/common/web-engine-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/type-registry.h>
#include <dlfcn.h>
#include <sstream>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/web-engine-back-forward-list.h>
#include <dali/devel-api/adaptor-framework/web-engine-certificate.h>
#include <dali/devel-api/adaptor-framework/web-engine-console-message.h>
#include <dali/devel-api/adaptor-framework/web-engine-context-menu.h>
#include <dali/devel-api/adaptor-framework/web-engine-context.h>
#include <dali/devel-api/adaptor-framework/web-engine-cookie-manager.h>
#include <dali/devel-api/adaptor-framework/web-engine-http-auth-handler.h>
#include <dali/devel-api/adaptor-framework/web-engine-load-error.h>
#include <dali/devel-api/adaptor-framework/web-engine-policy-decision.h>
#include <dali/devel-api/adaptor-framework/web-engine-request-interceptor.h>
#include <dali/devel-api/adaptor-framework/web-engine-settings.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>
#include <dali/public-api/images/pixel-data.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace // unnamed namespace
{
constexpr char const* const kPluginFullNamePrefix  = "libdali2-web-engine-";
constexpr char const* const kPluginFullNamePostfix = "-plugin.so";
constexpr char const* const kPluginFullNameDefault = "libdali2-web-engine-plugin.so";

// Note: Dali WebView policy does not allow to use multiple web engines in an application.
// So once pluginName is set to non-empty string, it will not change.
std::string pluginName;

std::string MakePluginName(const char* environmentName)
{
  std::stringstream fullName;
  fullName << kPluginFullNamePrefix << environmentName << kPluginFullNamePostfix;
  return std::move(fullName.str());
}

Dali::BaseHandle Create()
{
  return Dali::WebEngine::New();
}

Dali::TypeRegistration type(typeid(Dali::WebEngine), typeid(Dali::BaseHandle), Create);

} // unnamed namespace

WebEnginePtr WebEngine::New()
{
  WebEngine* instance = new WebEngine();

  if(!instance->Initialize())
  {
    delete instance;
    return nullptr;
  }

  return instance;
}

WebEngine::WebEngine()
: mPlugin(NULL),
  mHandle(NULL),
  mCreateWebEnginePtr(NULL),
  mDestroyWebEnginePtr(NULL)
{
}

WebEngine::~WebEngine()
{
  if(mHandle != NULL)
  {
    if(mDestroyWebEnginePtr != NULL)
    {
      mPlugin->Destroy();
      mDestroyWebEnginePtr(mPlugin);
    }

    dlclose(mHandle);
  }
}

bool WebEngine::InitializePluginHandle()
{
  if(pluginName.length() == 0)
  {
    // pluginName is not initialized yet.
    const char* name = EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_WEB_ENGINE_NAME);
    if(name)
    {
      pluginName = MakePluginName(name);
      mHandle    = dlopen(pluginName.c_str(), RTLD_LAZY);
      if(mHandle)
      {
        return true;
      }
    }
    pluginName = std::string(kPluginFullNameDefault);
  }

  mHandle = dlopen(pluginName.c_str(), RTLD_LAZY);
  if(!mHandle)
  {
    DALI_LOG_ERROR("Can't load %s : %s\n", pluginName.c_str(), dlerror());
    return false;
  }

  return true;
}

bool WebEngine::Initialize()
{
  char* error = NULL;

  if(!InitializePluginHandle())
  {
    return false;
  }

  mCreateWebEnginePtr = reinterpret_cast<CreateWebEngineFunction>(dlsym(mHandle, "CreateWebEnginePlugin"));
  if(mCreateWebEnginePtr == NULL)
  {
    DALI_LOG_ERROR("Can't load symbol CreateWebEnginePlugin(), error: %s\n", error);
    return false;
  }

  mDestroyWebEnginePtr = reinterpret_cast<DestroyWebEngineFunction>(dlsym(mHandle, "DestroyWebEnginePlugin"));

  if(mDestroyWebEnginePtr == NULL)
  {
    DALI_LOG_ERROR("Can't load symbol DestroyWebEnginePlugin(), error: %s\n", error);
    return false;
  }

  mPlugin = mCreateWebEnginePtr();

  if(mPlugin == NULL)
  {
    DALI_LOG_ERROR("Can't create the WebEnginePlugin object\n");
    return false;
  }

  return true;
}

void WebEngine::Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId)
{
  mPlugin->Create(width, height, locale, timezoneId);
}

void WebEngine::Create(uint32_t width, uint32_t height, uint32_t argc, char** argv)
{
  mPlugin->Create(width, height, argc, argv);
}

void WebEngine::Destroy()
{
  mPlugin->Destroy();
}

Dali::NativeImageInterfacePtr WebEngine::GetNativeImageSource()
{
  return mPlugin->GetNativeImageSource();
}

Dali::WebEngineSettings& WebEngine::GetSettings() const
{
  return mPlugin->GetSettings();
}

Dali::WebEngineContext& WebEngine::GetContext() const
{
  return mPlugin->GetContext();
}

Dali::WebEngineCookieManager& WebEngine::GetCookieManager() const
{
  return mPlugin->GetCookieManager();
}

Dali::WebEngineBackForwardList& WebEngine::GetBackForwardList() const
{
  return mPlugin->GetBackForwardList();
}

void WebEngine::LoadUrl(const std::string& url)
{
  mPlugin->LoadUrl(url);
}

std::string WebEngine::GetTitle() const
{
  return mPlugin->GetTitle();
}

Dali::PixelData WebEngine::GetFavicon() const
{
  return mPlugin->GetFavicon();
}

const std::string& WebEngine::GetUrl()
{
  return mPlugin->GetUrl();
}

const std::string& WebEngine::GetUserAgent() const
{
  return mPlugin->GetUserAgent();
}

void WebEngine::SetUserAgent(const std::string& userAgent)
{
  mPlugin->SetUserAgent(userAgent);
}

void WebEngine::LoadHtmlString(const std::string& htmlString)
{
  mPlugin->LoadHtmlString(htmlString);
}

bool WebEngine::LoadHtmlStringOverrideCurrentEntry(const std::string& html, const std::string& basicUri, const std::string& unreachableUrl)
{
  return mPlugin->LoadHtmlStringOverrideCurrentEntry(html, basicUri, unreachableUrl);
}

bool WebEngine::LoadContents(const std::string& contents, uint32_t contentSize, const std::string& mimeType, const std::string& encoding, const std::string& baseUri)
{
  return mPlugin->LoadContents(contents, contentSize, mimeType, encoding, baseUri);
}

void WebEngine::Reload()
{
  mPlugin->Reload();
}

bool WebEngine::ReloadWithoutCache()
{
  return mPlugin->ReloadWithoutCache();
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

void WebEngine::SuspendNetworkLoading()
{
  mPlugin->SuspendNetworkLoading();
}

void WebEngine::ResumeNetworkLoading()
{
  mPlugin->ResumeNetworkLoading();
}

bool WebEngine::AddCustomHeader(const std::string& name, const std::string& value)
{
  return mPlugin->AddCustomHeader(name, value);
}

bool WebEngine::RemoveCustomHeader(const std::string& name)
{
  return mPlugin->RemoveCustomHeader(name);
}

uint32_t WebEngine::StartInspectorServer(uint32_t port)
{
  return mPlugin->StartInspectorServer(port);
}

bool WebEngine::StopInspectorServer()
{
  return mPlugin->StopInspectorServer();
}

void WebEngine::ScrollBy(int32_t deltaX, int32_t deltaY)
{
  mPlugin->ScrollBy(deltaX, deltaY);
}

bool WebEngine::ScrollEdgeBy(int32_t deltaX, int32_t deltaY)
{
  return mPlugin->ScrollEdgeBy(deltaX, deltaY);
}

void WebEngine::SetScrollPosition(int32_t x, int32_t y)
{
  mPlugin->SetScrollPosition(x, y);
}

Dali::Vector2 WebEngine::GetScrollPosition() const
{
  return mPlugin->GetScrollPosition();
}

Dali::Vector2 WebEngine::GetScrollSize() const
{
  return mPlugin->GetScrollSize();
}

Dali::Vector2 WebEngine::GetContentSize() const
{
  return mPlugin->GetContentSize();
}

void WebEngine::RegisterJavaScriptAlertCallback(Dali::WebEnginePlugin::JavaScriptAlertCallback callback)
{
  mPlugin->RegisterJavaScriptAlertCallback(callback);
}

void WebEngine::JavaScriptAlertReply()
{
  mPlugin->JavaScriptAlertReply();
}

void WebEngine::RegisterJavaScriptConfirmCallback(Dali::WebEnginePlugin::JavaScriptConfirmCallback callback)
{
  mPlugin->RegisterJavaScriptConfirmCallback(callback);
}

void WebEngine::JavaScriptConfirmReply(bool confirmed)
{
  mPlugin->JavaScriptConfirmReply(confirmed);
}

void WebEngine::RegisterJavaScriptPromptCallback(Dali::WebEnginePlugin::JavaScriptPromptCallback callback)
{
  mPlugin->RegisterJavaScriptPromptCallback(callback);
}

void WebEngine::JavaScriptPromptReply(const std::string& result)
{
  mPlugin->JavaScriptPromptReply(result);
}

std::unique_ptr<Dali::WebEngineHitTest> WebEngine::CreateHitTest(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode)
{
  return mPlugin->CreateHitTest(x, y, mode);
}

bool WebEngine::CreateHitTestAsynchronously(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode, Dali::WebEnginePlugin::WebEngineHitTestCreatedCallback callback)
{
  return mPlugin->CreateHitTestAsynchronously(x, y, mode, callback);
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

void WebEngine::EvaluateJavaScript(const std::string& script, std::function<void(const std::string&)> resultHandler)
{
  mPlugin->EvaluateJavaScript(script, resultHandler);
}

void WebEngine::AddJavaScriptMessageHandler(const std::string& exposedObjectName, std::function<void(const std::string&)> handler)
{
  mPlugin->AddJavaScriptMessageHandler(exposedObjectName, handler);
}

void WebEngine::ClearAllTilesResources()
{
  mPlugin->ClearAllTilesResources();
}

void WebEngine::ClearHistory()
{
  mPlugin->ClearHistory();
}

void WebEngine::SetSize(uint32_t width, uint32_t height)
{
  mPlugin->SetSize(width, height);
}

void WebEngine::EnableMouseEvents(bool enabled)
{
  mPlugin->EnableMouseEvents(enabled);
}

void WebEngine::EnableKeyEvents(bool enabled)
{
  mPlugin->EnableKeyEvents(enabled);
}

bool WebEngine::SendTouchEvent(const Dali::TouchEvent& touch)
{
  return mPlugin->SendTouchEvent(touch);
}

bool WebEngine::SendKeyEvent(const Dali::KeyEvent& event)
{
  return mPlugin->SendKeyEvent(event);
}

void WebEngine::SetFocus(bool focused)
{
  mPlugin->SetFocus(focused);
}

void WebEngine::SetDocumentBackgroundColor(Dali::Vector4 color)
{
  mPlugin->SetDocumentBackgroundColor(color);
}

void WebEngine::ClearTilesWhenHidden(bool cleared)
{
  mPlugin->ClearTilesWhenHidden(cleared);
}

void WebEngine::SetTileCoverAreaMultiplier(float multiplier)
{
  mPlugin->SetTileCoverAreaMultiplier(multiplier);
}

void WebEngine::EnableCursorByClient(bool enabled)
{
  mPlugin->EnableCursorByClient(enabled);
}

std::string WebEngine::GetSelectedText() const
{
  return mPlugin->GetSelectedText();
}

void WebEngine::SetPageZoomFactor(float zoomFactor)
{
  mPlugin->SetPageZoomFactor(zoomFactor);
}

float WebEngine::GetPageZoomFactor() const
{
  return mPlugin->GetPageZoomFactor();
}

void WebEngine::SetTextZoomFactor(float zoomFactor)
{
  mPlugin->SetTextZoomFactor(zoomFactor);
}

float WebEngine::GetTextZoomFactor() const
{
  return mPlugin->GetTextZoomFactor();
}

float WebEngine::GetLoadProgressPercentage() const
{
  return mPlugin->GetLoadProgressPercentage();
}

void WebEngine::SetScaleFactor(float scaleFactor, Dali::Vector2 point)
{
  mPlugin->SetScaleFactor(scaleFactor, point);
}

float WebEngine::GetScaleFactor() const
{
  return mPlugin->GetScaleFactor();
}

void WebEngine::ActivateAccessibility(bool activated)
{
  mPlugin->ActivateAccessibility(activated);
}

bool WebEngine::SetVisibility(bool visible)
{
  return mPlugin->SetVisibility(visible);
}

bool WebEngine::HighlightText(const std::string& text, Dali::WebEnginePlugin::FindOption options, uint32_t maxMatchCount)
{
  return mPlugin->HighlightText(text, options, maxMatchCount);
}

void WebEngine::AddDynamicCertificatePath(const std::string& host, const std::string& certPath)
{
  mPlugin->AddDynamicCertificatePath(host, certPath);
}

Dali::PixelData WebEngine::GetScreenshot(Dali::Rect<int32_t> viewArea, float scaleFactor)
{
  return mPlugin->GetScreenshot(viewArea, scaleFactor);
}

bool WebEngine::GetScreenshotAsynchronously(Dali::Rect<int32_t> viewArea, float scaleFactor, Dali::WebEnginePlugin::ScreenshotCapturedCallback callback)
{
  return mPlugin->GetScreenshotAsynchronously(viewArea, scaleFactor, callback);
}

bool WebEngine::CheckVideoPlayingAsynchronously(Dali::WebEnginePlugin::VideoPlayingCallback callback)
{
  return mPlugin->CheckVideoPlayingAsynchronously(callback);
}

void WebEngine::RegisterGeolocationPermissionCallback(Dali::WebEnginePlugin::GeolocationPermissionCallback callback)
{
  mPlugin->RegisterGeolocationPermissionCallback(callback);
}

void WebEngine::UpdateDisplayArea(Dali::Rect<int32_t> displayArea)
{
  mPlugin->UpdateDisplayArea(displayArea);
}

void WebEngine::EnableVideoHole(bool enabled)
{
  mPlugin->EnableVideoHole(enabled);
}

bool WebEngine::SendHoverEvent(const Dali::HoverEvent& event)
{
  return mPlugin->SendHoverEvent(event);
}

bool WebEngine::SendWheelEvent(const Dali::WheelEvent& event)
{
  return mPlugin->SendWheelEvent(event);
}

Dali::WebEnginePlugin::WebEnginePageLoadSignalType& WebEngine::PageLoadStartedSignal()
{
  return mPlugin->PageLoadStartedSignal();
}

Dali::WebEnginePlugin::WebEnginePageLoadSignalType& WebEngine::PageLoadInProgressSignal()
{
  return mPlugin->PageLoadInProgressSignal();
}

Dali::WebEnginePlugin::WebEnginePageLoadSignalType& WebEngine::PageLoadFinishedSignal()
{
  return mPlugin->PageLoadFinishedSignal();
}

Dali::WebEnginePlugin::WebEnginePageLoadErrorSignalType& WebEngine::PageLoadErrorSignal()
{
  return mPlugin->PageLoadErrorSignal();
}

Dali::WebEnginePlugin::WebEngineScrollEdgeReachedSignalType& WebEngine::ScrollEdgeReachedSignal()
{
  return mPlugin->ScrollEdgeReachedSignal();
}

Dali::WebEnginePlugin::WebEngineUrlChangedSignalType& WebEngine::UrlChangedSignal()
{
  return mPlugin->UrlChangedSignal();
}

Dali::WebEnginePlugin::WebEngineFormRepostDecisionSignalType& WebEngine::FormRepostDecisionSignal()
{
  return mPlugin->FormRepostDecisionSignal();
}

Dali::WebEnginePlugin::WebEngineFrameRenderedSignalType& WebEngine::FrameRenderedSignal()
{
  return mPlugin->FrameRenderedSignal();
}

Dali::WebEnginePlugin::WebEngineRequestInterceptorSignalType& WebEngine::RequestInterceptorSignal()
{
  return mPlugin->RequestInterceptorSignal();
}

Dali::WebEnginePlugin::WebEngineConsoleMessageSignalType& WebEngine::ConsoleMessageSignal()
{
  return mPlugin->ConsoleMessageSignal();
}

Dali::WebEnginePlugin::WebEngineResponsePolicyDecisionSignalType& WebEngine::ResponsePolicyDecisionSignal()
{
  return mPlugin->ResponsePolicyDecisionSignal();
}

Dali::WebEnginePlugin::WebEngineCertificateSignalType& WebEngine::CertificateConfirmSignal()
{
  return mPlugin->CertificateConfirmSignal();
}

Dali::WebEnginePlugin::WebEngineCertificateSignalType& WebEngine::SslCertificateChangedSignal()
{
  return mPlugin->SslCertificateChangedSignal();
}

Dali::WebEnginePlugin::WebEngineHttpAuthHandlerSignalType& WebEngine::HttpAuthHandlerSignal()
{
  return mPlugin->HttpAuthHandlerSignal();
}

Dali::WebEnginePlugin::WebEngineContextMenuCustomizedSignalType& WebEngine::ContextMenuCustomizedSignal()
{
  return mPlugin->ContextMenuCustomizedSignal();
}

Dali::WebEnginePlugin::WebEngineContextMenuItemSelectedSignalType& WebEngine::ContextMenuItemSelectedSignal()
{
  return mPlugin->ContextMenuItemSelectedSignal();
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
