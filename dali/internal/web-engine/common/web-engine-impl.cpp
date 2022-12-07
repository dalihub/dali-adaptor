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
#include <dali/internal/web-engine/common/web-engine-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/object/type-registry.h>
#include <dlfcn.h>
#include <sstream>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/devel-api/adaptor-framework/web-engine-back-forward-list.h>
#include <dali/devel-api/adaptor-framework/web-engine-context.h>
#include <dali/devel-api/adaptor-framework/web-engine-cookie-manager.h>
#include <dali/devel-api/adaptor-framework/web-engine-policy-decision.h>
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

void WebEngine::Create(int width, int height, const std::string& locale, const std::string& timezoneId)
{
  mPlugin->Create(width, height, locale, timezoneId);
}

void WebEngine::Create(int width, int height, int argc, char** argv)
{
  mPlugin->Create(width, height, argc, argv);
}

Dali::WebEnginePlugin* WebEngine::GetPlugin() const
{
  return mPlugin;
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

void WebEngine::ScrollBy(int deltaX, int deltaY)
{
  mPlugin->ScrollBy(deltaX, deltaY);
}

void WebEngine::SetScrollPosition(int x, int y)
{
  mPlugin->SetScrollPosition(x, y);
}

void WebEngine::GetScrollPosition(int& x, int& y) const
{
  mPlugin->GetScrollPosition(x, y);
}

void WebEngine::GetScrollSize(int& width, int& height) const
{
  mPlugin->GetScrollSize(width, height);
}

void WebEngine::GetContentSize(int& width, int& height) const
{
  mPlugin->GetContentSize(width, height);
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

void WebEngine::SetSize(int width, int height)
{
  mPlugin->SetSize(width, height);
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

void WebEngine::UpdateDisplayArea(Dali::Rect<int> displayArea)
{
  mPlugin->UpdateDisplayArea(displayArea);
}

void WebEngine::EnableVideoHole(bool enabled)
{
  mPlugin->EnableVideoHole(enabled);
}

void WebEngine::RegisterPageLoadStartedCallback(Dali::WebEnginePlugin::WebEnginePageLoadCallback callback)
{
  mPlugin->RegisterPageLoadStartedCallback(callback);
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

void WebEngine::RegisterNavigationPolicyDecidedCallback(Dali::WebEnginePlugin::WebEngineNavigationPolicyDecidedCallback callback)
{
  mPlugin->RegisterNavigationPolicyDecidedCallback(callback);
}

void WebEngine::RegisterNewWindowCreatedCallback(Dali::WebEnginePlugin::WebEngineNewWindowCreatedCallback callback)
{
  mPlugin->RegisterNewWindowCreatedCallback(callback);
}

void WebEngine::GetPlainTextAsynchronously(Dali::WebEnginePlugin::PlainTextReceivedCallback callback)
{
  mPlugin->GetPlainTextAsynchronously(callback);
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
