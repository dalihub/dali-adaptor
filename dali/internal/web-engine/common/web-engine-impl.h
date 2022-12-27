#ifndef DALI_WEB_ENGINE_IMPL_H
#define DALI_WEB_ENGINE_IMPL_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/web-engine-plugin.h>
#include <dali/devel-api/adaptor-framework/web-engine.h>

namespace Dali
{
// forward declaration
class WebEngineBackForwardList;
class WebEngineContext;
class WebEngineCookieManager;
class WebEngineSettings;

namespace Internal
{
namespace Adaptor
{
class WebEngine;

typedef IntrusivePtr<WebEngine> WebEnginePtr;

/**
 * @brief WebEngine class is used for Web.
 */
class WebEngine : public Dali::BaseObject
{
public:
  /**
   * @brief Creates a new WebEngine handle
   *
   * @return WebEngine pointer
   */
  static WebEnginePtr New();

  /**
   * @copydoc Dali::WebEngine::Create()
   */
  void Create(int width, int height, const std::string& locale, const std::string& timezoneId);

  /**
   * @copydoc Dali::WebEngine::Create()
   */
  void Create(int width, int height, int argc, char** argv);

  /**
   * @copydoc Dali::WebEngine::Destroy()
   */
  void Destroy();

  /**
   * @copydoc Dali::WebEngine::GetPlugin()
   */
  Dali::WebEnginePlugin* GetPlugin() const;

  /**
   * @copydoc Dali::WebEngine::GetNativeImageSource()
   */
  Dali::NativeImageInterfacePtr GetNativeImageSource();

  /**
   * @copydoc Dali::WebEngine::GetSettings()
   */
  Dali::WebEngineSettings& GetSettings() const;

  /**
   * @copydoc Dali::WebEngine::GetContext()
   */
  Dali::WebEngineContext& GetContext() const;

  /**
   * @copydoc Dali::WebEngine::GetCookieManager()
   */
  Dali::WebEngineCookieManager& GetCookieManager() const;

  /**
   * @copydoc Dali::WebEngine::GetBackForwardList()
   */
  Dali::WebEngineBackForwardList& GetBackForwardList() const;

  /**
   * @copydoc Dali::WebEngine::LoadUrl()
   */
  void LoadUrl(const std::string& url);

  /**
   * @copydoc Dali::WebEngine::GetTitle()
   */
  std::string GetTitle() const;

  /**
   * @copydoc Dali::WebEngine::GetFavicon()
   */
  Dali::PixelData GetFavicon() const;

  /**
   * @copydoc Dali::WebEngine::GetUrl()
   */
  std::string GetUrl() const;

  /**
   * @copydoc Dali::WebEngine::GetUserAgent()
   */
  std::string GetUserAgent() const;

  /**
   * @copydoc Dali::WebEngine::SetUserAgent()
   */
  void SetUserAgent(const std::string& userAgent);

  /**
   * @copydoc Dali::WebEngine::LoadHtmlString()
   */
  void LoadHtmlString(const std::string& htmlString);

  /**
   * @copydoc Dali::WebEngine::Reload()
   */
  void Reload();

  /**
   * @copydoc Dali::WebEngine::StopLoading()
   */
  void StopLoading();

  /**
   * @copydoc Dali::WebEngine::Suspend()
   */
  void Suspend();

  /**
   * @copydoc Dali::WebEngine::Resume()
   */
  void Resume();

  /**
   * @copydoc Dali::WebEngine::ScrollBy()
   */
  void ScrollBy(int deltaX, int deltaY);

  /**
   * @copydoc Dali::WebEngine::SetScrollPosition()
   */
  void SetScrollPosition(int x, int y);

  /**
   * @copydoc Dali::WebEngine::GetScrollPosition()
   */
  void GetScrollPosition(int& x, int& y) const;

  /**
   * @copydoc Dali::WebEngine::GetScrollSize()
   */
  void GetScrollSize(int& width, int& height) const;

  /**
   * @copydoc Dali::WebEngine::GetContentSize()
   */
  void GetContentSize(int& width, int& height) const;

  /**
   * @copydoc Dali::WebEngine::CanGoForward()
   */
  bool CanGoForward();

  /**
   * @copydoc Dali::WebEngine::GoForward()
   */
  void GoForward();

  /**
   * @copydoc Dali::WebEngine::CanGoBack()
   */
  bool CanGoBack();

  /**
   * @copydoc Dali::WebEngine::GoBack()
   */
  void GoBack();

  /**
   * @copydoc Dali::WebEngine::EvaluateJavaScript()
   */
  void EvaluateJavaScript(const std::string& script, Dali::WebEnginePlugin::JavaScriptMessageHandlerCallback resultHandler);

  /**
   * @copydoc Dali::WebEngine::AddJavaScriptMessageHandler()
   */
  void AddJavaScriptMessageHandler(const std::string& exposedObjectName, Dali::WebEnginePlugin::JavaScriptMessageHandlerCallback handler);

  /**
   * @copydoc Dali::WebEngine::ClearAllTilesResources()
   */
  void ClearAllTilesResources();

  /**
   * @copydoc Dali::WebEngine::ClearHistory()
   */
  void ClearHistory();

  /**
   * @copydoc Dali::WebEngine::SetSize()
   */
  void SetSize(int width, int height);

  /**
   * @copydoc Dali::WebEngine::SendTouchEvent()
   */
  bool SendTouchEvent(const Dali::TouchEvent& touch);

  /**
   * @copydoc Dali::WebEngine::SendKeyEvent()
   */
  bool SendKeyEvent(const Dali::KeyEvent& event);

  /**
   * @copydoc Dali::WebEngine::SetFocus()
   */
  void SetFocus(bool focused);

  /**
   * @copydoc Dali::WebEngine::UpdateDisplayArea()
   */
  void UpdateDisplayArea(Dali::Rect<int> displayArea);

  /**
   * @copydoc Dali::WebEngine::EnableVideoHole()
   */
  void EnableVideoHole(bool enabled);

  /**
   * @copydoc Dali::WebEngine::RegisterPageLoadStartedCallback()
   */
  void RegisterPageLoadStartedCallback(Dali::WebEnginePlugin::WebEnginePageLoadCallback callback);

  /**
   * @copydoc Dali::WebEngine::RegisterPageLoadFinishedCallback()
   */
  void RegisterPageLoadFinishedCallback(Dali::WebEnginePlugin::WebEnginePageLoadCallback callback);

  /**
   * @copydoc Dali::WebEngine::RegisterPageLoadErrorCallback()
   */
  void RegisterPageLoadErrorCallback(Dali::WebEnginePlugin::WebEnginePageLoadErrorCallback callback);

  /**
   * @copydoc Dali::WebEngine::RegisterScrollEdgeReachedCallback()
   */
  void RegisterScrollEdgeReachedCallback(Dali::WebEnginePlugin::WebEngineScrollEdgeReachedCallback callback);

  /**
   * @copydoc Dali::WebEngine::RegisterUrlChangedCallback()
   */
  void RegisterUrlChangedCallback(Dali::WebEnginePlugin::WebEngineUrlChangedCallback callback);

  /**
   @copydoc Dali::WebEngine::RegisterNavigationPolicyDecidedCallback()
   */
  void RegisterNavigationPolicyDecidedCallback(Dali::WebEnginePlugin::WebEngineNavigationPolicyDecidedCallback callback);

  /**
   @copydoc Dali::WebEngine::RegisterNewWindowCreatedCallback()
   */
  void RegisterNewWindowCreatedCallback(Dali::WebEnginePlugin::WebEngineNewWindowCreatedCallback callback);

  /**
   @copydoc Dali::WebEngine::RegisterFrameRenderedCallback()
   */
  void RegisterFrameRenderedCallback(Dali::WebEnginePlugin::WebEngineFrameRenderedCallback callback);

  /**
   * @copydoc Dali::WebEngine::GetPlainTextAsynchronously()
   */
  void GetPlainTextAsynchronously(Dali::WebEnginePlugin::PlainTextReceivedCallback callback);

private:
  /**
   * @brief Constructor.
   */
  WebEngine();

  /**
   * @brief Destructor.
   */
  virtual ~WebEngine();

  // Undefined copy constructor
  WebEngine(const WebEngine& WebEngine);

  // Undefined assignment operator
  WebEngine& operator=(const WebEngine& WebEngine);

  /**
   * @brief Initializes member data.
   *
   * @return Whether the initialization succeed or not.
   */
  bool Initialize();

private:
  typedef Dali::WebEnginePlugin* (*CreateWebEngineFunction)();
  typedef void (*DestroyWebEngineFunction)(Dali::WebEnginePlugin* plugin);

  Dali::WebEnginePlugin* mPlugin; ///< WebEnginePlugin instance
};

} // namespace Adaptor
} // namespace Internal

inline static Internal::Adaptor::WebEngine& GetImplementation(Dali::WebEngine& webEngine)
{
  DALI_ASSERT_ALWAYS(webEngine && "WebEngine handle is empty.");

  BaseObject& handle = webEngine.GetBaseObject();

  return static_cast<Internal::Adaptor::WebEngine&>(handle);
}

inline static const Internal::Adaptor::WebEngine& GetImplementation(const Dali::WebEngine& webEngine)
{
  DALI_ASSERT_ALWAYS(webEngine && "WebEngine handle is empty.");

  const BaseObject& handle = webEngine.GetBaseObject();

  return static_cast<const Internal::Adaptor::WebEngine&>(handle);
}

} // namespace Dali

#endif
