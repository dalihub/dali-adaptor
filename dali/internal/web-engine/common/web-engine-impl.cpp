/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-back-forward-list.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-certificate.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-console-message.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-context-menu.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-context.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-cookie-manager.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-http-auth-handler.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-load-error.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-policy-decision.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-settings.h>
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

std::string MakePluginName(const char* environmentName)
{
  std::stringstream fullName;
  fullName << kPluginFullNamePrefix << environmentName << kPluginFullNamePostfix;
  return fullName.str();
}

Dali::BaseHandle Create()
{
  return Dali::WebEngine::New();
}

Dali::TypeRegistration type(typeid(Dali::WebEngine), typeid(Dali::BaseHandle), Create);

/**
 * @brief Control the WebEnginePlugin library lifecycle.
 * Hold the plugin library handle in static singletone.
 * It will makes library handle alives during all WebEngine resources create & destory.
 */
struct WebEnginePluginObject
{
public:
  static WebEnginePluginObject& GetInstance()
  {
    static WebEnginePluginObject gPluginHandle;
    return gPluginHandle;
  }

  /**
   * @brief Converts an handle to a bool.
   *
   * This is useful for checking whether the WebEnginePluginObject succes to load library.
   * @note We don't check mHandle because it is possible that mHandle load is success but
   * Create/Destroy API load failed.
   */
  explicit operator bool() const
  {
    return mLoadSucceeded;
  }

  bool InitializeContextHandle()
  {
    if(!mHandle)
    {
      return false;
    }

    if(!mGetWebEngineContextPtr)
    {
      mGetWebEngineContextPtr = reinterpret_cast<GetWebEngineContext>(dlsym(mHandle, "GetWebEngineContext"));
      if(!mGetWebEngineContextPtr)
      {
        DALI_LOG_ERROR("Can't load symbol GetWebEngineContext(), error: %s\n", dlerror());
        return false;
      }
    }

    return true;
  }

  bool InitializeCookieManagerHandle()
  {
    if(!mHandle)
    {
      return false;
    }

    if(!mGetWebEngineCookieManagerPtr)
    {
      mGetWebEngineCookieManagerPtr = reinterpret_cast<GetWebEngineCookieManager>(dlsym(mHandle, "GetWebEngineCookieManager"));
      if(!mGetWebEngineCookieManagerPtr)
      {
        DALI_LOG_ERROR("Can't load symbol GetWebEngineCookieManager(), error: %s\n", dlerror());
        return false;
      }
    }

    return true;
  }

private:
  // Private constructor / destructor
  WebEnginePluginObject()
  : mLoadSucceeded{false},
    mHandle{nullptr},
    mCreateWebEnginePtr{nullptr},
    mDestroyWebEnginePtr{nullptr},
    mGetWebEngineContextPtr{nullptr},
    mGetWebEngineCookieManagerPtr{nullptr}
  {
    std::string pluginName;
    const char* name = EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_WEB_ENGINE_NAME);
    if(name)
    {
      pluginName = MakePluginName(name);
    }
    else
    {
      pluginName = std::string(kPluginFullNameDefault);
    }

    mHandle = dlopen(pluginName.c_str(), RTLD_LAZY);
    if(!mHandle)
    {
      DALI_LOG_ERROR("Can't load %s : %s\n", pluginName.c_str(), dlerror());
      return;
    }

    mCreateWebEnginePtr = reinterpret_cast<CreateWebEngineFunction>(dlsym(mHandle, "CreateWebEnginePlugin"));
    if(mCreateWebEnginePtr == nullptr)
    {
      DALI_LOG_ERROR("Can't load symbol CreateWebEnginePlugin(), error: %s\n", dlerror());
      return;
    }

    mDestroyWebEnginePtr = reinterpret_cast<DestroyWebEngineFunction>(dlsym(mHandle, "DestroyWebEnginePlugin"));
    if(mDestroyWebEnginePtr == nullptr)
    {
      DALI_LOG_ERROR("Can't load symbol DestroyWebEnginePlugin(), error: %s\n", dlerror());
      return;
    }

    mLoadSucceeded = true;
  }

  ~WebEnginePluginObject()
  {
    if(mHandle)
    {
      dlclose(mHandle);
      mHandle        = nullptr;
      mLoadSucceeded = false;
    }
  }

  WebEnginePluginObject(const WebEnginePluginObject&) = delete;
  WebEnginePluginObject(WebEnginePluginObject&&)      = delete;
  WebEnginePluginObject& operator=(const WebEnginePluginObject&) = delete;
  WebEnginePluginObject& operator=(WebEnginePluginObject&&) = delete;

private:
  bool mLoadSucceeded; ///< True if library loaded successfully. False otherwise.

public:
  using CreateWebEngineFunction  = Dali::WebEnginePlugin* (*)();
  using DestroyWebEngineFunction = void (*)(Dali::WebEnginePlugin* plugin);

  using GetWebEngineContext       = Dali::WebEngineContext* (*)();
  using GetWebEngineCookieManager = Dali::WebEngineCookieManager* (*)();

  void*                    mHandle;              ///< Handle for the loaded library
  CreateWebEngineFunction  mCreateWebEnginePtr;  ///< Function to create plugin instance
  DestroyWebEngineFunction mDestroyWebEnginePtr; ///< Function to destroy plugin instance

  GetWebEngineContext       mGetWebEngineContextPtr;       ///< Function to get WebEngineContext
  GetWebEngineCookieManager mGetWebEngineCookieManagerPtr; ///< Function to get WebEngineCookieManager
};

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

Dali::WebEngineContext* WebEngine::GetContext()
{
  if(!WebEnginePluginObject::GetInstance().InitializeContextHandle())
  {
    return nullptr;
  }

  if(WebEnginePluginObject::GetInstance().mGetWebEngineContextPtr)
  {
    return WebEnginePluginObject::GetInstance().mGetWebEngineContextPtr();
  }

  return nullptr;
}

Dali::WebEngineCookieManager* WebEngine::GetCookieManager()
{
  if(!WebEnginePluginObject::GetInstance().InitializeCookieManagerHandle())
  {
    return nullptr;
  }

  if(WebEnginePluginObject::GetInstance().mGetWebEngineCookieManagerPtr)
  {
    return WebEnginePluginObject::GetInstance().mGetWebEngineCookieManagerPtr();
  }

  return nullptr;
}

WebEngine::WebEngine()
: mPlugin(nullptr)
{
}

WebEngine::~WebEngine()
{
  if(mPlugin != nullptr)
  {
    mPlugin->Destroy();
    // Check whether plugin load sccess or not.
    if(DALI_LIKELY(WebEnginePluginObject::GetInstance()))
    {
      WebEnginePluginObject::GetInstance().mDestroyWebEnginePtr(mPlugin);
    }
    mPlugin = nullptr;
  }
}

bool WebEngine::Initialize()
{
  // Check whether plugin load sccess or not.
  if(!WebEnginePluginObject::GetInstance())
  {
    return false;
  }

  mPlugin = WebEnginePluginObject::GetInstance().mCreateWebEnginePtr();
  if(mPlugin == nullptr)
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

Dali::WebEnginePlugin* WebEngine::GetPlugin() const
{
  return mPlugin;
}

Dali::NativeImageSourcePtr WebEngine::GetNativeImageSource()
{
  return mPlugin->GetNativeImageSource();
}

Dali::WebEngineSettings& WebEngine::GetSettings() const
{
  return mPlugin->GetSettings();
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

std::string WebEngine::GetUrl() const
{
  return mPlugin->GetUrl();
}

std::string WebEngine::GetUserAgent() const
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

bool WebEngine::LoadContents(const int8_t* contents, uint32_t contentSize, const std::string& mimeType, const std::string& encoding, const std::string& baseUri)
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

void WebEngine::EvaluateJavaScript(const std::string& script, Dali::WebEnginePlugin::JavaScriptMessageHandlerCallback resultHandler)
{
  mPlugin->EvaluateJavaScript(script, resultHandler);
}

void WebEngine::AddJavaScriptMessageHandler(const std::string& exposedObjectName, Dali::WebEnginePlugin::JavaScriptMessageHandlerCallback handler)
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

Accessibility::Address WebEngine::GetAccessibilityAddress()
{
  return mPlugin->GetAccessibilityAddress();
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

void WebEngine::RegisterFrameRenderedCallback(Dali::WebEnginePlugin::WebEngineFrameRenderedCallback callback)
{
  mPlugin->RegisterFrameRenderedCallback(callback);
}

void WebEngine::RegisterPageLoadStartedCallback(Dali::WebEnginePlugin::WebEnginePageLoadCallback callback)
{
  mPlugin->RegisterPageLoadStartedCallback(callback);
}

void WebEngine::RegisterPageLoadInProgressCallback(Dali::WebEnginePlugin::WebEnginePageLoadCallback callback)
{
  mPlugin->RegisterPageLoadInProgressCallback(callback);
}

void WebEngine::RegisterPageLoadFinishedCallback(Dali::WebEnginePlugin::WebEnginePageLoadCallback callback)
{
  mPlugin->RegisterPageLoadFinishedCallback(callback);
}

void WebEngine::RegisterPageLoadErrorCallback(Dali::WebEnginePlugin::WebEnginePageLoadErrorCallback callback)
{
  mPlugin->RegisterPageLoadErrorCallback(callback);
}

void WebEngine::RegisterScrollEdgeReachedCallback(Dali::WebEnginePlugin::WebEngineScrollEdgeReachedCallback callback)
{
  mPlugin->RegisterScrollEdgeReachedCallback(callback);
}

void WebEngine::RegisterUrlChangedCallback(Dali::WebEnginePlugin::WebEngineUrlChangedCallback callback)
{
  mPlugin->RegisterUrlChangedCallback(callback);
}

void WebEngine::RegisterFormRepostDecidedCallback(Dali::WebEnginePlugin::WebEngineFormRepostDecidedCallback callback)
{
  mPlugin->RegisterFormRepostDecidedCallback(callback);
}

void WebEngine::RegisterConsoleMessageReceivedCallback(Dali::WebEnginePlugin::WebEngineConsoleMessageReceivedCallback callback)
{
  mPlugin->RegisterConsoleMessageReceivedCallback(callback);
}

void WebEngine::RegisterResponsePolicyDecidedCallback(Dali::WebEnginePlugin::WebEngineResponsePolicyDecidedCallback callback)
{
  mPlugin->RegisterResponsePolicyDecidedCallback(callback);
}

void WebEngine::RegisterNavigationPolicyDecidedCallback(Dali::WebEnginePlugin::WebEngineNavigationPolicyDecidedCallback callback)
{
  mPlugin->RegisterNavigationPolicyDecidedCallback(callback);
}

void WebEngine::RegisterNewWindowCreatedCallback(Dali::WebEnginePlugin::WebEngineNewWindowCreatedCallback callback)
{
  mPlugin->RegisterNewWindowCreatedCallback(callback);
}

void WebEngine::RegisterCertificateConfirmedCallback(Dali::WebEnginePlugin::WebEngineCertificateCallback callback)
{
  mPlugin->RegisterCertificateConfirmedCallback(callback);
}

void WebEngine::RegisterSslCertificateChangedCallback(Dali::WebEnginePlugin::WebEngineCertificateCallback callback)
{
  mPlugin->RegisterSslCertificateChangedCallback(callback);
}

void WebEngine::RegisterHttpAuthHandlerCallback(Dali::WebEnginePlugin::WebEngineHttpAuthHandlerCallback callback)
{
  mPlugin->RegisterHttpAuthHandlerCallback(callback);
}

void WebEngine::RegisterContextMenuShownCallback(Dali::WebEnginePlugin::WebEngineContextMenuShownCallback callback)
{
  mPlugin->RegisterContextMenuShownCallback(callback);
}

void WebEngine::RegisterContextMenuHiddenCallback(Dali::WebEnginePlugin::WebEngineContextMenuHiddenCallback callback)
{
  mPlugin->RegisterContextMenuHiddenCallback(callback);
}

void WebEngine::GetPlainTextAsynchronously(Dali::WebEnginePlugin::PlainTextReceivedCallback callback)
{
  mPlugin->GetPlainTextAsynchronously(callback);
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
