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
#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-plugin.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine.h>

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
   * @brief Create a new WebEngine handle
   *
   * @return WebEngine pointer
   */
  static WebEnginePtr New();

  /**
   * @brief Get context of web engine
   *
   */
  static Dali::WebEngineContext* GetContext();

  /**
   * @brief Get cookie manager of web engine
   *
   */
  static Dali::WebEngineCookieManager* GetCookieManager();

  /**
   * @copydoc Dali::WebEngine::Create()
   */
  void Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId);

  /**
   * @copydoc Dali::WebEngine::Create()
   */
  void Create(uint32_t width, uint32_t height, uint32_t argc, char** argv);

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
  Dali::NativeImageSourcePtr GetNativeImageSource();

  /**
   * @copydoc Dali::WebEngine::GetSettings()
   */
  Dali::WebEngineSettings& GetSettings() const;

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
   * @copydoc Dali::WebEngine::LoadHtmlStringOverrideCurrentEntry()
   */
  bool LoadHtmlStringOverrideCurrentEntry(const std::string& html, const std::string& basicUri, const std::string& unreachableUrl);

  /**
   * @copydoc Dali::WebEngine::LoadContents()
   */
  bool LoadContents(const std::string& contents, uint32_t contentSize, const std::string& mimeType, const std::string& encoding, const std::string& baseUri);

  /**
   * @copydoc Dali::WebEngine::Reload()
   */
  void Reload();

  /**
   * @copydoc Dali::WebEngine::ReloadWithoutCache()
   */
  bool ReloadWithoutCache();

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
   * @copydoc Dali::WebEngine::SuspendNetworkLoading()
   */
  void SuspendNetworkLoading();

  /**
   * @copydoc Dali::WebEngine::ResumeNetworkLoading()
   */
  void ResumeNetworkLoading();

  /**
   * @copydoc Dali::WebEngine::AddCustomHeader()
   */
  bool AddCustomHeader(const std::string& name, const std::string& value);

  /**
   * @copydoc Dali::WebEngine::RemoveCustomHeader()
   */
  bool RemoveCustomHeader(const std::string& name);

  /**
   * @copydoc Dali::WebEngine::StartInspectorServer()
   */
  uint32_t StartInspectorServer(uint32_t port);

  /**
   * @copydoc Dali::WebEngine::StopInspectorServer()
   */
  bool StopInspectorServer();

  /**
   * @copydoc Dali::WebEngine::ScrollBy()
   */
  void ScrollBy(int32_t deltaX, int32_t deltaY);

  /**
   * @copydoc Dali::WebEngine::ScrollEdgeBy()
   */
  bool ScrollEdgeBy(int32_t deltaX, int32_t deltaY);

  /**
   * @copydoc Dali::WebEngine::SetScrollPosition()
   */
  void SetScrollPosition(int32_t x, int32_t y);

  /**
   * @copydoc Dali::WebEngine::GetScrollPosition()
   */
  Dali::Vector2 GetScrollPosition() const;

  /**
   * @copydoc Dali::WebEngine::GetScrollSize()
   */
  Dali::Vector2 GetScrollSize() const;

  /**
   * @copydoc Dali::WebEngine::GetContentSize()
   */
  Dali::Vector2 GetContentSize() const;

  /**
   * @copydoc Dali::WebEngine::RegisterJavaScriptAlertCallback()
   */
  void RegisterJavaScriptAlertCallback(Dali::WebEnginePlugin::JavaScriptAlertCallback callback);

  /**
   * @copydoc Dali::WebEngine::JavaScriptAlertReply()
   */
  void JavaScriptAlertReply();

  /**
   * @copydoc Dali::WebEngine::RegisterJavaScriptConfirmCallback()
   */
  void RegisterJavaScriptConfirmCallback(Dali::WebEnginePlugin::JavaScriptConfirmCallback callback);

  /**
   * @copydoc Dali::WebEngine::JavaScriptConfirmReply()
   */
  void JavaScriptConfirmReply(bool confirmed);

  /**
   * @copydoc Dali::WebEngine::RegisterJavaScriptPromptCallback()
   */
  void RegisterJavaScriptPromptCallback(Dali::WebEnginePlugin::JavaScriptPromptCallback callback);

  /**
   * @copydoc Dali::WebEngine::JavaScriptPromptReply()
   */
  void JavaScriptPromptReply(const std::string& result);

  /**
   * @copydoc Dali::WebEngine::CreateHitTest()
   */
  std::unique_ptr<Dali::WebEngineHitTest> CreateHitTest(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode);

  /**
   * @copydoc Dali::WebEngine::CreateHitTestAsynchronously()
   */
  bool CreateHitTestAsynchronously(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode, Dali::WebEnginePlugin::WebEngineHitTestCreatedCallback callback);

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
  void SetSize(uint32_t width, uint32_t height);

  /**
   * @copydoc Dali::WebEngine::EnableMouseEvents()
   */
  void EnableMouseEvents(bool enabled);

  /**
   * @copydoc Dali::WebEngine::EnableKeyEvents()
   */
  void EnableKeyEvents(bool enabled);

  /**
   * @copydoc Dali::WebEngine::SetDocumentBackgroundColor()
   */
  void SetDocumentBackgroundColor(Dali::Vector4 color);

  /**
   * @copydoc Dali::WebEngine::ClearTilesWhenHidden()
   */
  void ClearTilesWhenHidden(bool cleared);

  /**
   * @copydoc Dali::WebEngine::SetTileCoverAreaMultiplier()
   */
  void SetTileCoverAreaMultiplier(float multiplier);

  /**
   * @copydoc Dali::WebEngine::EnableCursorByClient()
   */
  void EnableCursorByClient(bool enabled);

  /**
   * @copydoc Dali::WebEngine::GetSelectedText()
   */
  std::string GetSelectedText() const;

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
   * @copydoc Dali::WebEngine::SetPageZoomFactor()
   */
  void SetPageZoomFactor(float zoomFactor);

  /**
   * @copydoc Dali::WebEngine::GetPageZoomFactor()
   */
  float GetPageZoomFactor() const;

  /**
   * @copydoc Dali::WebEngine::SetTextZoomFactor()
   */
  void SetTextZoomFactor(float zoomFactor);

  /**
   * @copydoc Dali::WebEngine::GetTextZoomFactor()
   */
  float GetTextZoomFactor() const;

  /**
   * @copydoc Dali::WebEngine::GetLoadProgressPercentage()
   */
  float GetLoadProgressPercentage() const;

  /**
   * @copydoc Dali::WebEngine::SetScaleFactor()
   */
  void SetScaleFactor(float scaleFactor, Dali::Vector2 point);

  /**
   * @copydoc Dali::WebEngine::GetScaleFactor()
   */
  float GetScaleFactor() const;

  /**
   * @copydoc Dali::WebEngine::ActivateAccessibility()
   */
  void ActivateAccessibility(bool activated);

  /**
   * @copydoc Dali::WebEngine::GetAccessibilityAddress()
   */
  Accessibility::Address GetAccessibilityAddress();

  /**
   * @copydoc Dali::WebEngine::SetVisibility()
   */
  bool SetVisibility(bool visible);

  /**
   * @copydoc Dali::WebEngine::HighlightText()
   */
  bool HighlightText(const std::string& text, Dali::WebEnginePlugin::FindOption options, uint32_t maxMatchCount);

  /**
   * @copydoc Dali::WebEngine::AddDynamicCertificatePath()
   */
  void AddDynamicCertificatePath(const std::string& host, const std::string& certPath);

  /**
   * @copydoc Dali::WebEngine::GetScreenshot()
   */
  Dali::PixelData GetScreenshot(Dali::Rect<int32_t> viewArea, float scaleFactor);

  /**
   * @copydoc Dali::WebEngine::GetScreenshotAsync()
   */
  bool GetScreenshotAsynchronously(Dali::Rect<int32_t> viewArea, float scaleFactor, Dali::WebEnginePlugin::ScreenshotCapturedCallback callback);

  /**
   * @copydoc Dali::WebEngine::IsVideoPlaying()
   */
  bool CheckVideoPlayingAsynchronously(Dali::WebEnginePlugin::VideoPlayingCallback callback);

  /**
   * @copydoc Dali::WebEngine::RegisterGeolocationPermissionCallback()
   */
  void RegisterGeolocationPermissionCallback(Dali::WebEnginePlugin::GeolocationPermissionCallback callback);

  /**
   * @copydoc Dali::WebEngine::UpdateDisplayArea()
   */
  void UpdateDisplayArea(Dali::Rect<int32_t> displayArea);

  /**
   * @copydoc Dali::WebEngine::EnableVideoHole()
   */
  void EnableVideoHole(bool enabled);

  /**
   * @copydoc Dali::WebEngine::SendHoverEvent()
   */
  bool SendHoverEvent(const Dali::HoverEvent& event);

  /**
   * @copydoc Dali::WebEngine::SendWheelEvent()
   */
  bool SendWheelEvent(const Dali::WheelEvent& event);

  /**
   @copydoc Dali::WebEngine::RegisterFrameRenderedCallback()
   */
  void RegisterFrameRenderedCallback(Dali::WebEnginePlugin::WebEngineFrameRenderedCallback callback);

  /**
   * @copydoc Dali::WebEngine::RegisterPageLoadStartedCallback()
   */
  void RegisterPageLoadStartedCallback(Dali::WebEnginePlugin::WebEnginePageLoadCallback callback);

  /**
   * @copydoc Dali::WebEngine::RegisterPageLoadInProgressCallback()
   */
  void RegisterPageLoadInProgressCallback(Dali::WebEnginePlugin::WebEnginePageLoadCallback callback);

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
   * @copydoc Dali::WebEngine::RegisterFormRepostDecidedCallback()
   */
  void RegisterFormRepostDecidedCallback(Dali::WebEnginePlugin::WebEngineFormRepostDecidedCallback callback);

  /**
   @copydoc Dali::WebEngine::RegisterConsoleMessageReceivedCallback()
   */
  void RegisterConsoleMessageReceivedCallback(Dali::WebEnginePlugin::WebEngineConsoleMessageReceivedCallback callback);

  /**
   @copydoc Dali::WebEngine::RegisterResponsePolicyDecidedCallback()
   */
  void RegisterResponsePolicyDecidedCallback(Dali::WebEnginePlugin::WebEngineResponsePolicyDecidedCallback callback);

  /**
   @copydoc Dali::WebEngine::RegisterNavigationPolicyDecidedCallback()
   */
  void RegisterNavigationPolicyDecidedCallback(Dali::WebEnginePlugin::WebEngineNavigationPolicyDecidedCallback callback);

  /**
   @copydoc Dali::WebEngine::RegisterNewWindowCreatedCallback()
   */
  void RegisterNewWindowCreatedCallback(Dali::WebEnginePlugin::WebEngineNewWindowCreatedCallback callback);

  /**
   * @copydoc Dali::WebEngine::RegisterCertificateConfirmedCallback()
   */
  void RegisterCertificateConfirmedCallback(Dali::WebEnginePlugin::WebEngineCertificateCallback callback);

  /**
   * @copydoc Dali::WebEngine::RegisterSslCertificateChangedCallback()
   */
  void RegisterSslCertificateChangedCallback(Dali::WebEnginePlugin::WebEngineCertificateCallback callback);

  /**
   * @copydoc Dali::WebEngine::RegisterHttpAuthHandlerCallback()
   */
  void RegisterHttpAuthHandlerCallback(Dali::WebEnginePlugin::WebEngineHttpAuthHandlerCallback callback);

  /**
   * @copydoc Dali::WebEngine::RegisterContextMenuShownCallback()
   */
  void RegisterContextMenuShownCallback(Dali::WebEnginePlugin::WebEngineContextMenuShownCallback callback);

  /**
   * @copydoc Dali::WebEngine::RegisterContextMenuHiddenCallback()
   */
  void RegisterContextMenuHiddenCallback(Dali::WebEnginePlugin::WebEngineContextMenuHiddenCallback callback);

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
   * @brief Initialize member data.
   *
   * @return Whether the initialization succeed or not.
   */
  bool Initialize();

private:
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
