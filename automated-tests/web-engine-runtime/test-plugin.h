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

#pragma once

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-plugin.h>
#include <dali/public-api/images/pixel-data.h>
#include <dali/public-api/adaptor-framework/native-image.h>
#include <stdexcept>

namespace Dali
{
// Only construction, profile-based creation and destruction are used here.
class RuntimeTestPlugin : public WebEnginePlugin
{
public:
  bool InitializeNativeImageRendering(uint32_t, uint32_t, const std::string&, const std::string&) override { return true; }
  WebEngineDirectRenderingSessionPtr InitializeDirectRendering(uint32_t, uint32_t, const std::string&, const std::string&) override { return {}; }
  void Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void Create(uint32_t width, uint32_t height, uint32_t argc, char** argv) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void Destroy() override
  {
  }

  bool IsIncognito() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  WebEngineSettings& GetSettings() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  WebEngineBackForwardList& GetBackForwardList() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void LoadUrl(const std::string& url) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  std::string GetTitle() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  Dali::PixelData GetFavicon() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  NativeImagePtr GetNativeImage() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void ChangeOrientation(int orientation) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  std::string GetUrl() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void LoadHtmlString(const std::string& htmlString) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool LoadHtmlStringOverrideCurrentEntry(const std::string& html, const std::string& basicUri, const std::string& unreachableUrl) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool LoadContents(const int8_t* contents, uint32_t contentSize, const std::string& mimeType, const std::string& encoding, const std::string& baseUri) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void Reload() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool ReloadWithoutCache() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void StopLoading() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void Suspend() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void Resume() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void SuspendNetworkLoading() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void ResumeNetworkLoading() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool AddCustomHeader(const std::string& name, const std::string& value) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool RemoveCustomHeader(const std::string& name) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  uint32_t StartInspectorServer(uint32_t port) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool StopInspectorServer() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void ScrollBy(int32_t deltaX, int32_t deltaY) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool ScrollEdgeBy(int32_t deltaX, int32_t deltaY) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void SetScrollPosition(int32_t x, int32_t y) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  Dali::Vector2 GetScrollPosition() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  Dali::Vector2 GetScrollSize() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  Dali::Vector2 GetContentSize() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool CanGoForward() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void GoForward() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool CanGoBack() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void GoBack() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void EvaluateJavaScript(const std::string& script, JavaScriptMessageHandlerCallback resultHandler) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void AddJavaScriptMessageHandler(const std::string& exposedObjectName, JavaScriptMessageHandlerCallback handler) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void AddJavaScriptEntireMessageHandler(const std::string& exposedObjectName, JavaScriptEntireMessageHandlerCallback handler) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterJavaScriptAlertCallback(JavaScriptAlertCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void JavaScriptAlertReply() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterJavaScriptConfirmCallback(JavaScriptConfirmCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void JavaScriptConfirmReply(bool confirmed) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterJavaScriptPromptCallback(JavaScriptPromptCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void JavaScriptPromptReply(const std::string& result) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  std::unique_ptr<Dali::WebEngineHitTest> CreateHitTest(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool CreateHitTestAsynchronously(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode, WebEngineHitTestCreatedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void ClearHistory() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void ClearAllTilesResources() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  std::string GetUserAgent() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void SetUserAgent(const std::string& userAgent) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void SetSize(uint32_t width, uint32_t height) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void SetDocumentBackgroundColor(Dali::Vector4 color) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void ClearTilesWhenHidden(bool cleared) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void SetTileCoverAreaMultiplier(float multiplier) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void EnableCursorByClient(bool enabled) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  std::string GetSelectedText() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool SendTouchEvent(const TouchEvent& touch) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool SendKeyEvent(const KeyEvent& event) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void EnableMouseEvents(bool enabled) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void EnableKeyEvents(bool enabled) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void SetFocus(bool focused) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool SetImePositionAndAlignment(Dali::Vector2 position, int alignment) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void SetCursorThemeName(const std::string themeName) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void SetPageZoomFactor(float zoomFactor) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  float GetPageZoomFactor() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void SetTextZoomFactor(float zoomFactor) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  float GetTextZoomFactor() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  float GetLoadProgressPercentage() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void SetScaleFactor(float scaleFactor, Dali::Vector2 point) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  float GetScaleFactor() const override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void ActivateAccessibility(bool activated) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  Dali::Devel::Accessibility::Address GetAccessibilityAddress() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool SetVisibility(bool visible) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool HighlightText(const std::string& text, FindOption options, uint32_t maxMatchCount) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void AddDynamicCertificatePath(const std::string& host, const std::string& certPath) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  Dali::PixelData GetScreenshot(Dali::BoundsInteger viewArea, float scaleFactor) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool GetScreenshotAsynchronously(Dali::BoundsInteger viewArea, float scaleFactor, ScreenshotCapturedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool CheckVideoPlayingAsynchronously(VideoPlayingCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterGeolocationPermissionCallback(GeolocationPermissionCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void UpdateDisplayArea(Dali::BoundsInteger displayArea) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void EnableVideoHole(bool enabled) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool SendHoverEvent(const HoverEvent& event) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  bool SendWheelEvent(const WheelEvent& event) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void ExitFullscreen() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterFrameRenderedCallback(WebEngineFrameRenderedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterPageLoadStartedCallback(WebEnginePageLoadCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterPageLoadInProgressCallback(WebEnginePageLoadCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterPageLoadFinishedCallback(WebEnginePageLoadCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterPageLoadErrorCallback(WebEnginePageLoadErrorCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterScrollEdgeReachedCallback(WebEngineScrollEdgeReachedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterOverScrolledCallback(WebEngineOverScrolledCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterUrlChangedCallback(WebEngineUrlChangedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterFormRepostDecidedCallback(WebEngineFormRepostDecidedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterConsoleMessageReceivedCallback(WebEngineConsoleMessageReceivedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterResponsePolicyDecidedCallback(WebEngineResponsePolicyDecidedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterNavigationPolicyDecidedCallback(WebEngineNavigationPolicyDecidedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterNewWindowPolicyDecidedCallback(WebEngineNewWindowPolicyDecidedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterNewWindowCreatedCallback(WebEngineNewWindowCreatedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterCertificateConfirmedCallback(WebEngineCertificateCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterSslCertificateChangedCallback(WebEngineCertificateCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterHttpAuthHandlerCallback(WebEngineHttpAuthHandlerCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterContextMenuShownCallback(WebEngineContextMenuShownCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterContextMenuHiddenCallback(WebEngineContextMenuHiddenCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterFullscreenEnteredCallback(WebEngineFullscreenEnteredCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterFullscreenExitedCallback(WebEngineFullscreenExitedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterTextFoundCallback(WebEngineTextFoundCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void GetPlainTextAsynchronously(PlainTextReceivedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void WebAuthenticationCancel() override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterWebAuthDisplayQRCallback(WebEngineWebAuthDisplayQRCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterWebAuthResponseCallback(WebEngineWebAuthResponseCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterFileChooserRequestedCallback(WebEngineFileChooserRequestedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterWebProcessCrashedCallback(WebEngineWebProcessCrashedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterUserMediaPermissionRequestCallback(WebEngineUserMediaPermissionRequestCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterDeviceConnectionChangedCallback(WebEngineDeviceConnectionChangedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterDeviceListGetCallback(WebEngineDeviceListGetCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void FeedMouseWheel(bool yDirection, int step, int x, int y) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void SetVideoHole(bool enabled, bool isWaylandWindow) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RemoveJavaScriptMessageHandler(const std::string& exposedObjectName) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterPlaybackVideoReadyCallback(WebEnginePlaybackVideoReadyCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterPlaybackVideoStartedCallback(WebEnginePlaybackVideoStartedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterPlaybackVideoFinishedCallback(WebEnginePlaybackVideoFinishedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterPlaybackVideoStoppedCallback(WebEnginePlaybackVideoStoppedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

  void RegisterPlaybackVideoPausedCallback(WebEnginePlaybackVideoPausedCallback callback) override
  {
    throw std::logic_error("Unexpected WebEnginePlugin call");
  }

};
} // namespace Dali
