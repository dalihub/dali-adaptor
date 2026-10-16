/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/graphics-backend.h>
#include <dali/internal/web-engine/common/web-engine-direct-rendering-connection-impl.h>
#include <dali/internal/web-engine/common/web-engine-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/object/type-registry.h>
#include <dali/integration-api/debug.h>
#include <dlfcn.h>
#include <locale>
#include <sstream>
#include <climits>
#include <map>
#include <memory>
#include <mutex>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-profile.h>

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
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/public-api/adaptor-framework/native-image.h>
#include <dali/public-api/images/pixel-data.h>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
namespace // unnamed namespace
{
bool IsDirectRenderingSupported()
{
  if(Graphics::GetCurrentGraphicsBackend() != Graphics::Backend::GLES || !Adaptor::IsAvailable())
  {
    return false;
  }
  return Adaptor::GetImplementation(Adaptor::Get()).IsResourceContextSupported();
}

static constexpr int32_t USE_ENVIRONMENT_VALUE          = -1;
static constexpr int32_t USE_CHROMIUM_WEB_ENGINE        = 0;
static constexpr int32_t USE_LIGHT_WEIGHT_WEB_ENGINE    = 1;
static constexpr int32_t DEFAULT_WEB_ENGINE_PLUGIN_TYPE = USE_ENVIRONMENT_VALUE;
static int32_t           webEnginePluginType            = DEFAULT_WEB_ENGINE_PLUGIN_TYPE;

constexpr char const* const kPluginFullNamePrefix   = DALI_LIBRARY_PREFIX "web-engine-";
constexpr char const* const kPluginFullNamePostfix  = "-plugin.so";
constexpr char const* const kPluginFullNameDefault  = DALI_LIBRARY_PREFIX "web-engine-plugin.so";
constexpr char const* const kPluginFullNameChromium = DALI_LIBRARY_PREFIX "web-engine-chromium-plugin.so";
constexpr char const* const kPluginFullNameLwe      = DALI_LIBRARY_PREFIX "web-engine-lwe-plugin.so";

std::recursive_mutex runtimeMutex;
bool initializationStarted = false;
bool initializationSucceeded = false;

std::string MakePluginName(const char* environmentName)
{
  std::stringstream fullName;
  fullName.imbue(std::locale::classic());
  fullName << kPluginFullNamePrefix << environmentName << kPluginFullNamePostfix;
  return fullName.str();
}

Dali::BaseHandle Create()
{
  return Dali::WebEngine::New();
}

Dali::TypeRegistration typeRegistration(typeid(Dali::WebEngine), typeid(Dali::BaseHandle), Create);

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
    return *GetSharedInstance(webEnginePluginType);
  }

  static std::shared_ptr<WebEnginePluginObject> GetSharedInstance(int32_t type)
  {
    auto& library = Libraries()[type];
    if(!library)
    {
      auto candidate = std::shared_ptr<WebEnginePluginObject>(new WebEnginePluginObject(type));
      // dlopen resolves environment names and the default symlink to the same
      // handle. Share the wrapper too: profiles and views compare its identity.
      if(candidate->mHandle)
      {
        for(const auto& entry : Libraries())
        {
          if(entry.second && entry.second->mHandle == candidate->mHandle)
          {
            library = entry.second;
            break;
          }
        }
      }
      if(!library)
      {
        library = std::move(candidate);
      }
    }
    return library;
  }

  static std::shared_ptr<WebEnginePluginObject> FindInstance(int32_t type)
  {
    auto& libraries = Libraries();
    auto found = libraries.find(type);
    return found == libraries.end() ? nullptr : found->second;
  }

  static std::map<int32_t, std::shared_ptr<WebEnginePluginObject>>& Libraries()
  {
    static std::map<int32_t, std::shared_ptr<WebEnginePluginObject>> libraries;
    return libraries;
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

  const Dali::WebEngineProfileApi* GetProfileApi()
  {
    using GetApi = const Dali::WebEngineProfileApi* (*)(uint32_t);
    auto getApi = mHandle ? reinterpret_cast<GetApi>(dlsym(mHandle, "GetWebEngineProfileApi")) : nullptr;
    const auto* api = getApi ? getApi(1) : nullptr;
    if(!api || api->version != 1 || api->structSize < offsetof(Dali::WebEngineProfileApi, createDirectRenderingView) ||
       !api->storeArguments || !api->readArguments || !api->initialize ||
       !api->getDefaultProfile || !api->createProfile || !api->destroyProfile ||
       !api->getContext || !api->getCookieManager || !api->createView)
    {
      DALI_LOG_ERROR("Web engine profile API is unavailable\n");
      return nullptr;
    }
    return api;
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
  WebEnginePluginObject(int32_t type)
  : mLoadSucceeded{false},
    mHandle{nullptr},
    mCreateWebEnginePtr{nullptr},
    mDestroyWebEnginePtr{nullptr},
    mGetWebEngineContextPtr{nullptr},
    mGetWebEngineCookieManagerPtr{nullptr}
  {
    std::string pluginName;

    if(type == USE_CHROMIUM_WEB_ENGINE)
    {
      pluginName = kPluginFullNameChromium;
    }
    else if(type == USE_LIGHT_WEIGHT_WEB_ENGINE)
    {
      pluginName = kPluginFullNameLwe;
    }
    else
    {
      const char* name = EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_WEB_ENGINE_NAME);
      if(name)
      {
        pluginName = MakePluginName(name);
      }
      else
      {
        pluginName = std::string(kPluginFullNameDefault);
      }
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

public:
  ~WebEnginePluginObject()
  {
    if(mHandle)
    {
      dlclose(mHandle);
      mHandle        = nullptr;
      mLoadSucceeded = false;
    }
  }

  WebEnginePluginObject(const WebEnginePluginObject&)            = delete;
  WebEnginePluginObject(WebEnginePluginObject&&)                 = delete;
  WebEnginePluginObject& operator=(const WebEnginePluginObject&) = delete;
  WebEnginePluginObject& operator=(WebEnginePluginObject&&)      = delete;

private:
  bool mLoadSucceeded; ///< True if library loaded successfully. False otherwise.

public:
  using CreateWebEngineFunction  = Dali::WebEnginePlugin* (*)();
  using DestroyWebEngineFunction = void (*)(Dali::WebEnginePlugin* plugin);

  using GetWebEngineContext       = Dali::WebEngineContext* (*)(bool);
  using GetWebEngineCookieManager = Dali::WebEngineCookieManager* (*)(bool);

  void*                    mHandle;              ///< Handle for the loaded library
  CreateWebEngineFunction  mCreateWebEnginePtr;  ///< Function to create plugin instance
  DestroyWebEngineFunction mDestroyWebEnginePtr; ///< Function to destroy plugin instance

  GetWebEngineContext       mGetWebEngineContextPtr;       ///< Function to get WebEngineContext
  GetWebEngineCookieManager mGetWebEngineCookieManagerPtr; ///< Function to get WebEngineCookieManager
};

const Dali::WebEngineProfileApi* InitializeRuntime()
{
  auto& library = WebEnginePluginObject::GetInstance();
  const auto* api = library.GetProfileApi();
  if(!api)
  {
    return nullptr;
  }
  if(!initializationStarted)
  {
    initializationStarted = true;
    initializationSucceeded = api->initialize();
    if(!initializationSucceeded)
    {
      DALI_LOG_ERROR("Web engine initialization failed\n");
    }
  }
  return initializationSucceeded ? api : nullptr;
}

} // unnamed namespace

WebEnginePtr WebEngine::New(int32_t type)
{
  std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
  if(!Dali::WebEngineRuntime::SelectType(type))
  {
    return nullptr;
  }
  WebEngine* instance = new WebEngine(type);
  if(!instance->Initialize())
  {
    delete instance;
    return nullptr;
  }

  return instance;
}

Dali::WebEngineContext* WebEngine::GetContext(bool isIncognito)
{
  std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
  if(!InitializeRuntime())
  {
    return nullptr;
  }
  if(!WebEnginePluginObject::GetInstance().InitializeContextHandle())
  {
    return nullptr;
  }

  if(WebEnginePluginObject::GetInstance().mGetWebEngineContextPtr)
  {
    return WebEnginePluginObject::GetInstance().mGetWebEngineContextPtr(isIncognito);
  }

  return nullptr;
}

Dali::WebEngineCookieManager* WebEngine::GetCookieManager(bool isIncognito)
{
  std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
  if(!InitializeRuntime())
  {
    return nullptr;
  }
  if(!WebEnginePluginObject::GetInstance().InitializeCookieManagerHandle())
  {
    return nullptr;
  }

  if(WebEnginePluginObject::GetInstance().mGetWebEngineCookieManagerPtr)
  {
    return WebEnginePluginObject::GetInstance().mGetWebEngineCookieManagerPtr(isIncognito);
  }

  return nullptr;
}

WebEngine::WebEngine(int32_t type)
: mPlugin(nullptr)
{
  (void)type;
}

WebEngine::~WebEngine()
{
  if(mPlugin != nullptr)
  {
    if(mInitialized)
    {
      mPlugin->Destroy();
    }
    // Check whether plugin load sccess or not.
    mDestroyPlugin(mPlugin);
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

  auto library = WebEnginePluginObject::GetSharedInstance(webEnginePluginType);
  mPluginLibrary = library;
  mDestroyPlugin = library->mDestroyWebEnginePtr;
  mPlugin = library->mCreateWebEnginePtr();
  if(mPlugin == nullptr)
  {
    DALI_LOG_ERROR("Can't create the WebEnginePlugin object\n");
    return false;
  }
  return true;
}

void WebEngine::Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId)
{
  if(!InitializeNativeImageRendering(width, height, locale, timezoneId))
  {
    DALI_LOG_ERROR("WebEngine: native-image initialization failed\n");
  }
}

void WebEngine::Create(uint32_t width, uint32_t height, uint32_t /*argc*/, char** /*argv*/)
{
  std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
  if(!PrepareInitialization())
  {
    DALI_LOG_ERROR("WebEngine: native-image initialization failed\n");
    return;
  }
  auto profile = Dali::WebEngineRuntime::GetDefaultProfile();
  if(!Dali::WebEngineRuntime::CreateView(Dali::WebEngine::DownCast(Dali::BaseHandle(this)), profile, width, height))
  {
    DALI_LOG_ERROR("WebEngine: native-image initialization failed\n");
  }
}

void WebEngine::Destroy()
{
  if(mDirectRenderingState)
  {
    mDirectRenderingState->Stop();
    return;
  }
  if(mInitialized)
  {
    mPlugin->Destroy();
    mInitialized = false;
    mProfile.reset();
  }
}

void WebEngine::DirectRenderingStopped(WebEngineDirectRenderingState& state)
{
  DALI_ASSERT_ALWAYS(mDirectRenderingState == &state);
  mDirectRenderingState = nullptr;
  Destroy();
}

bool WebEngine::IsIncognito() const
{
  return mPlugin->IsIncognito();
}

Dali::WebEnginePlugin* WebEngine::GetPlugin() const
{
  return mPlugin;
}

Dali::NativeImagePtr WebEngine::GetNativeImage()
{
  return mPlugin->GetNativeImage();
}

Dali::NativeImageInterfacePtr WebEngine::GetNativeImageInterface()
{
  return mPlugin->GetNativeImageInterface();
}

void WebEngine::ChangeOrientation(int orientation)
{
  return mPlugin->ChangeOrientation(orientation);
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

void WebEngine::AddJavaScriptEntireMessageHandler(const std::string& exposedObjectName, Dali::WebEnginePlugin::JavaScriptEntireMessageHandlerCallback handler)
{
  mPlugin->AddJavaScriptEntireMessageHandler(exposedObjectName, handler);
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

bool WebEngine::SetImePositionAndAlignment(Dali::Vector2 position, int alignment)
{
  return mPlugin->SetImePositionAndAlignment(position, alignment);
}

void WebEngine::SetCursorThemeName(const std::string themeName)
{
  mPlugin->SetCursorThemeName(themeName);
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

Dali::Devel::Accessibility::Address WebEngine::GetAccessibilityAddress()
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

Dali::PixelData WebEngine::GetScreenshot(Dali::BoundsInteger viewArea, float scaleFactor)
{
  return mPlugin->GetScreenshot(viewArea, scaleFactor);
}

bool WebEngine::GetScreenshotAsynchronously(Dali::BoundsInteger viewArea, float scaleFactor, Dali::WebEnginePlugin::ScreenshotCapturedCallback callback)
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

void WebEngine::UpdateDisplayArea(Dali::BoundsInteger displayArea)
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

void WebEngine::ExitFullscreen()
{
  mPlugin->ExitFullscreen();
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

void WebEngine::RegisterOverScrolledCallback(Dali::WebEnginePlugin::WebEngineOverScrolledCallback callback)
{
  mPlugin->RegisterOverScrolledCallback(callback);
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

void WebEngine::RegisterNewWindowPolicyDecidedCallback(Dali::WebEnginePlugin::WebEngineNewWindowPolicyDecidedCallback callback)
{
  mPlugin->RegisterNewWindowPolicyDecidedCallback(callback);
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

void WebEngine::RegisterFullscreenEnteredCallback(Dali::WebEnginePlugin::WebEngineFullscreenEnteredCallback callback)
{
  mPlugin->RegisterFullscreenEnteredCallback(callback);
}

void WebEngine::RegisterFullscreenExitedCallback(Dali::WebEnginePlugin::WebEngineFullscreenExitedCallback callback)
{
  mPlugin->RegisterFullscreenExitedCallback(callback);
}

void WebEngine::RegisterTextFoundCallback(Dali::WebEnginePlugin::WebEngineTextFoundCallback callback)
{
  mPlugin->RegisterTextFoundCallback(callback);
}

void WebEngine::GetPlainTextAsynchronously(Dali::WebEnginePlugin::PlainTextReceivedCallback callback)
{
  mPlugin->GetPlainTextAsynchronously(callback);
}

void WebEngine::WebAuthenticationCancel()
{
  mPlugin->WebAuthenticationCancel();
}

void WebEngine::RegisterWebAuthDisplayQRCallback(Dali::WebEnginePlugin::WebEngineWebAuthDisplayQRCallback callback)
{
  mPlugin->RegisterWebAuthDisplayQRCallback(callback);
}

void WebEngine::RegisterWebAuthResponseCallback(Dali::WebEnginePlugin::WebEngineWebAuthResponseCallback callback)
{
  mPlugin->RegisterWebAuthResponseCallback(callback);
}

void WebEngine::RegisterFileChooserRequestedCallback(Dali::WebEnginePlugin::WebEngineFileChooserRequestedCallback callback)
{
  mPlugin->RegisterFileChooserRequestedCallback(callback);
}

void WebEngine::RegisterWebProcessCrashedCallback(Dali::WebEnginePlugin::WebEngineWebProcessCrashedCallback callback)
{
  mPlugin->RegisterWebProcessCrashedCallback(callback);
}

void WebEngine::RegisterUserMediaPermissionRequestCallback(Dali::WebEnginePlugin::WebEngineUserMediaPermissionRequestCallback callback)
{
  mPlugin->RegisterUserMediaPermissionRequestCallback(callback);
}

void WebEngine::RegisterDeviceConnectionChangedCallback(Dali::WebEnginePlugin::WebEngineDeviceConnectionChangedCallback callback)
{
  mPlugin->RegisterDeviceConnectionChangedCallback(callback);
}

void WebEngine::RegisterDeviceListGetCallback(Dali::WebEnginePlugin::WebEngineDeviceListGetCallback callback)
{
  mPlugin->RegisterDeviceListGetCallback(callback);
}

void WebEngine::RegisterPlaybackVideoReadyCallback(Dali::WebEnginePlugin::WebEnginePlaybackVideoReadyCallback callback)
{
  mPlugin->RegisterPlaybackVideoReadyCallback(callback);
}

void WebEngine::RegisterPlaybackVideoStartedCallback(Dali::WebEnginePlugin::WebEnginePlaybackVideoStartedCallback callback)
{
  mPlugin->RegisterPlaybackVideoStartedCallback(callback);
}

void WebEngine::RegisterPlaybackVideoFinishedCallback(Dali::WebEnginePlugin::WebEnginePlaybackVideoFinishedCallback callback)
{
  mPlugin->RegisterPlaybackVideoFinishedCallback(callback);
}

void WebEngine::RegisterPlaybackVideoStoppedCallback(Dali::WebEnginePlugin::WebEnginePlaybackVideoStoppedCallback callback)
{
  mPlugin->RegisterPlaybackVideoStoppedCallback(callback);
}

void WebEngine::RegisterPlaybackVideoPausedCallback(Dali::WebEnginePlugin::WebEnginePlaybackVideoPausedCallback callback)
{
  mPlugin->RegisterPlaybackVideoPausedCallback(callback);
}

void WebEngine::FeedMouseWheel(bool yDirection, int step, int x, int y)
{
  mPlugin->FeedMouseWheel(yDirection, step, x, y);
}

void WebEngine::SetVideoHole(bool enabled, bool isWaylandWindow)
{
  mPlugin->SetVideoHole(enabled, isWaylandWindow);
}

void WebEngine::RemoveJavaScriptMessageHandler(const std::string& exposedObjectName)
{
  mPlugin->RemoveJavaScriptMessageHandler(exposedObjectName);
}

bool WebEngine::PrepareInitialization()
{
  auto library = std::static_pointer_cast<WebEnginePluginObject>(mPluginLibrary);
  if(mInitialized || !mPlugin || library != WebEnginePluginObject::GetSharedInstance(webEnginePluginType))
  {
    DALI_LOG_ERROR("WebEngine: initialization requires an uninitialized engine from the active backend\n");
    return false;
  }
  return InitializeRuntime() != nullptr;
}

bool WebEngine::InitializeNativeImageRendering(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId)
{
  std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
  if(!PrepareInitialization())
  {
    return false;
  }
  mInitialized = mPlugin->InitializeNativeImageRendering(width, height, locale, timezoneId);
  return mInitialized;
}

Dali::WebEngineDirectRenderingConnection WebEngine::InitializeDirectRendering(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId)
{
  std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
  if(!IsDirectRenderingSupported() || !PrepareInitialization())
  {
    return {};
  }
  return ConnectDirectRendering(mPlugin->InitializeDirectRendering(width, height, locale, timezoneId));
}

bool WebEngine::CreateView(const Dali::WebEngineProfilePtr& profile, const Dali::WebEngineProfileApi& api, void* token, uint32_t width, uint32_t height)
{
  if(!PrepareInitialization())
  {
    return false;
  }
  mInitialized = api.createView(mPlugin, token, width, height);
  if(mInitialized)
  {
    mProfile = profile;
  }
  return mInitialized;
}

Dali::WebEngineDirectRenderingConnection WebEngine::CreateDirectRenderingView(const Dali::WebEngineProfilePtr& profile, const Dali::WebEngineProfileApi& api, void* token, uint32_t width, uint32_t height)
{
  if(api.structSize < sizeof(Dali::WebEngineProfileApi) || !api.createDirectRenderingView ||
     !IsDirectRenderingSupported() || !PrepareInitialization())
  {
    return {};
  }
  auto connection = ConnectDirectRendering(api.createDirectRenderingView(mPlugin, token, width, height));
  if(connection)
  {
    mProfile = profile;
  }
  return connection;
}

Dali::WebEngineDirectRenderingConnection WebEngine::ConnectDirectRendering(Dali::WebEngineDirectRenderingSessionPtr session)
{
  if(!session)
  {
    return {};
  }
  mInitialized = true;
  mDirectRenderingState = new WebEngineDirectRenderingState(std::move(session), [engine = IntrusivePtr<WebEngine>(this)](WebEngineDirectRenderingState& state)
  {
    engine->DirectRenderingStopped(state);
  });
  return Dali::WebEngineDirectRenderingConnection(new WebEngineDirectRenderingConnection(mDirectRenderingState));
}

} // namespace Adaptor
} // namespace Internal
} //namespace DALI_NAMESPACE

namespace DALI_NAMESPACE
{
namespace
{
using namespace Internal::Adaptor;

class OwnedWebEngineProfile : public WebEngineProfile
{
public:
  OwnedWebEngineProfile(std::shared_ptr<WebEnginePluginObject> library, const WebEngineProfileApi* api, void* token)
  : mLibrary(std::move(library)), mApi(api), mToken(token)
  {
  }

  ~OwnedWebEngineProfile() override
  {
    mApi->destroyProfile(mToken);
  }

  WebEngineContext* GetContext() const override
  {
    return mApi->getContext(mToken);
  }

  WebEngineCookieManager* GetCookieManager() const override
  {
    return mApi->getCookieManager(mToken);
  }

  std::shared_ptr<WebEnginePluginObject> mLibrary;
  const WebEngineProfileApi* mApi;
  void* mToken;
};


}

bool WebEngineRuntime::SelectType(int32_t type)
{
  std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
  if(type == webEnginePluginType)
  {
    return true;
  }
  auto previous = WebEnginePluginObject::FindInstance(webEnginePluginType);
  if(previous && *previous && WebEnginePluginObject::GetSharedInstance(type) == previous)
  {
    return true;
  }
  if(initializationStarted)
  {
    return false;
  }
  // Only load the old plugin when it has already been selected/accessed. A failed
  // initial load must not prevent selecting a working backend.
  const auto* oldApi = previous && *previous ? previous->GetProfileApi() : nullptr;
  if(oldApi)
  {
    uint32_t argc = 0;
    const char* const* argv = nullptr;
    oldApi->readArguments(&argc, &argv);
    auto next = WebEnginePluginObject::GetSharedInstance(type);
    const auto* newApi = next->GetProfileApi();
    if(!newApi || newApi->storeArguments(argc, argv) != WebEngineArgumentsResult::SUCCESS)
    {
      return false;
    }
  }
  webEnginePluginType = type;
  return true;
}

bool WebEngineRuntime::HasInitializationStarted()
{
  std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
  return initializationStarted;
}

WebEngineArgumentsResult WebEngineRuntime::SetArguments(uint32_t argc, char** argv)
{
  std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
  if(initializationStarted)
  {
    return WebEngineArgumentsResult::ALREADY_INITIALIZED;
  }
  if(argc > INT_MAX || (argc && !argv))
  {
    return WebEngineArgumentsResult::INVALID_ARGUMENT;
  }
  for(uint32_t i = 0; i < argc; ++i)
  {
    if(!argv[i])
    {
      return WebEngineArgumentsResult::INVALID_ARGUMENT;
    }
  }
  const auto* api = WebEnginePluginObject::GetInstance().GetProfileApi();
  return api ? api->storeArguments(argc, argv) : WebEngineArgumentsResult::BACKEND_UNAVAILABLE;
}

WebEngineProfilePtr WebEngineRuntime::GetDefaultProfile()
{
  std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
  static std::weak_ptr<WebEngineProfile> cached;
  if(auto profile = cached.lock())
  {
    return profile->GetContext() ? profile : WebEngineProfilePtr();
  }
  const auto* api = InitializeRuntime();
  void* token = api ? api->getDefaultProfile() : nullptr;
  if(!token)
  {
    DALI_LOG_ERROR("Cannot acquire default web profile\n");
    return {};
  }
  auto profile = std::make_shared<OwnedWebEngineProfile>(WebEnginePluginObject::GetSharedInstance(webEnginePluginType), api, token);
  cached = profile;
  return profile;
}

WebEngineProfilePtr WebEngineRuntime::CreateProfile(bool incognito, const char* bundlePath)
{
  std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
  const auto* api = InitializeRuntime();
  void* token = api ? api->createProfile(incognito, bundlePath ? bundlePath : "") : nullptr;
  if(!token)
  {
    DALI_LOG_ERROR("Cannot create web profile\n");
    return {};
  }
  return std::make_shared<OwnedWebEngineProfile>(WebEnginePluginObject::GetSharedInstance(webEnginePluginType), api, token);
}

bool WebEngineRuntime::CreateView(Dali::WebEngine engine, const WebEngineProfilePtr& profile, uint32_t width, uint32_t height)
{
  std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
  auto owned = std::dynamic_pointer_cast<OwnedWebEngineProfile>(profile);
  if(!engine || !owned || !owned->GetContext() ||
     Dali::GetImplementation(engine).GetPluginLibrary() != owned->mLibrary || !engine.GetPlugin())
  {
    return false;
  }
  return Dali::GetImplementation(engine).CreateView(profile, *owned->mApi, owned->mToken, width, height);
}
Dali::WebEngineDirectRenderingConnection WebEngineRuntime::CreateDirectRenderingView(Dali::WebEngine engine, const WebEngineProfilePtr& profile, uint32_t width, uint32_t height)
{
  std::lock_guard<std::recursive_mutex> lock(runtimeMutex);
  auto owned = std::dynamic_pointer_cast<OwnedWebEngineProfile>(profile);
  if(!engine || !owned || !owned->GetContext() ||
     Dali::GetImplementation(engine).GetPluginLibrary() != owned->mLibrary || !engine.GetPlugin())
  {
    return {};
  }
  return Dali::GetImplementation(engine).CreateDirectRenderingView(profile, *owned->mApi, owned->mToken, width, height);
}
} // namespace DALI_NAMESPACE
