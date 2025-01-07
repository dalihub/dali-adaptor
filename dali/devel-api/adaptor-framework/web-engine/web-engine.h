#ifndef DALI_WEB_ENGINE_H
#define DALI_WEB_ENGINE_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/object/base-handle.h>

//INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-plugin.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
class WebEngineContext;
class WebEngineCookieManager;

namespace Internal
{
namespace Adaptor
{
class WebEngine;
} // namespace Adaptor

} // namespace Internal

/**
 * @brief Proxy class to dynamically load, use and unload web engine plugin.
 *
 * The purpose of this class is to dynamically load the web engine plugin if and when its needed.
 * So we don't slow down every application startup if they never need web engine.
 */
class DALI_ADAPTOR_API WebEngine : public BaseHandle
{
public:
  /**
   * @brief Constructor.
   */
  WebEngine();

  /**
   * @brief Destructor.
   */
  ~WebEngine();

  /**
   * @brief Create a new instance of a WebEngine.
   */
  static WebEngine New();

  /**
   * @brief Create a new instance of a WebEngine with type (0: Chromium, 1: LWE, otherwise: depend on system environment).
   * @param[in] type The WebEngine type (0: Chromium, 1: LWE, otherwise: depend on system environment).
   */
  static WebEngine New(int type);

  /**
   * @brief Get context of WebEngine.
   */
  static Dali::WebEngineContext* GetContext();

  /**
   * @brief Get cookie manager of WebEngine.
   */
  static Dali::WebEngineCookieManager* GetCookieManager();

  /**
   * @brief Copy constructor.
   *
   * @param[in] WebEngine WebEngine to copy. The copied WebEngine will point at the same implementation
   */
  WebEngine(const WebEngine& WebEngine);

  /**
   * @brief Assignment operator.
   *
   * @param[in] WebEngine The WebEngine to assign from.
   * @return The updated WebEngine.
   */
  WebEngine& operator=(const WebEngine& WebEngine);

  /**
   * @brief Move constructor.
   *
   * @param[in] WebEngine WebEngine to move. The moved WebEngine will point at the same implementation
   */
  WebEngine(WebEngine&& WebEngine);

  /**
   * @brief Move assignment operator.
   *
   * @param[in] WebEngine The WebEngine to assign from.
   * @return The updated WebEngine.
   */
  WebEngine& operator=(WebEngine&& WebEngine);

  /**
   * @brief Downcast a handle to WebEngine handle.
   *
   * If handle points to a WebEngine the downcast produces valid
   * handle. If not the returned handle is left uninitialized.
   *
   * @param[in] handle Handle to an object
   * @return Handle to a WebEngine or an uninitialized handle
   */
  static WebEngine DownCast(BaseHandle handle);

  /**
   * @brief Create WebEngine instance.
   *
   * @param [in] width The width of Web
   * @param [in] height The height of Web
   * @param [in] locale The locale of Web
   * @param [in] timezoneId The timezoneID of Web
   */
  void Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId);

  /**
   * @brief Create WebEngine instance.
   *
   * @param [in] width The width of Web
   * @param [in] height The height of Web
   * @param [in] argc The count of application arguments
   * @param [in] argv The string array of application arguments
   */
  void Create(uint32_t width, uint32_t height, uint32_t argc, char** argv);

  /**
   * @brief Destroy WebEngine instance.
   */
  void Destroy();

  /**
   * @brief Gets web engine plugin.
   */
  Dali::WebEnginePlugin* GetPlugin() const;

  /**
   * @brief Get native image source to render.
   */
  NativeImageSourcePtr GetNativeImageSource();

  /**
   * @brief Change orientation.
   */
  void ChangeOrientation(int orientation);

  /**
   * @brief Get settings of WebEngine.
   */
  Dali::WebEngineSettings& GetSettings() const;

  /**
   * @brief Get back-forward list of WebEngine.
   */
  Dali::WebEngineBackForwardList& GetBackForwardList() const;

  /**
   * @brief Load a web page based on a given URL.
   *
   * @param [in] url The URL of the resource to load
   */
  void LoadUrl(const std::string& url);

  /**
   * @brief Return the title of the Web.
   *
   * @return The title of web page
   */
  std::string GetTitle() const;

  /**
   * @brief Return the Favicon of the Web.
   *
   * @return FavIcon of Dali::PixelData& type
   */
  Dali::PixelData GetFavicon() const;

  /**
   * @brief Get the url.
   */
  std::string GetUrl() const;

  /**
   * @brief Load a given string as web contents.
   *
   * @param [in] htmlString The string to use as the contents of the web page
   */
  void LoadHtmlString(const std::string& htmlString);

  /**
   * @brief Load the specified html string as the content of the view overriding current history entry
   *
   * @param[in] html HTML data to load
   * @param[in] basicUri Base URL used for relative paths to external objects
   * @param[in] unreachableUrl URL that could not be reached
   *
   * @return true if successfully loaded, false otherwise
   */
  bool LoadHtmlStringOverrideCurrentEntry(const std::string& html, const std::string& basicUri, const std::string& unreachableUrl);

  /**
   * @brief Request loading the given contents by MIME type into the view object
   *
   * @param[in] contents The content to load
   * @param[in] contentSize The size of contents (in bytes)
   * @param[in] mimeType The type of contents, if 0 is given "text/html" is assumed
   * @param[in] encoding The encoding for contents, if 0 is given "UTF-8" is assumed
   * @param[in] baseUri The base URI to use for relative resources
   *
   * @return true if successfully request, false otherwise
   */
  bool LoadContents(const int8_t* contents, uint32_t contentSize, const std::string& mimeType, const std::string& encoding, const std::string& baseUri);

  /**
   * @brief Reload the Web.
   */
  void Reload();

  /**
   * @brief Reload the current page's document without cache
   */
  bool ReloadWithoutCache();

  /**
   * @brief Stop loading web contents on the current page.
   */
  void StopLoading();

  /**
   * @brief Suspend the operation associated with the view.
   */
  void Suspend();

  /**
   * @brief Resume the operation associated with the view object after calling Suspend().
   */
  void Resume();

  /**
   * @brief To suspend all url loading
   */
  void SuspendNetworkLoading();

  /**
   * @brief To resume new url network loading
   */
  void ResumeNetworkLoading();

  /**
   * @brief Add custom header
   *
   * @param[in] name custom header name to add the custom header
   * @param[in] value custom header value to add the custom header
   *
   * @return true if succeeded, false otherwise
   */
  bool AddCustomHeader(const std::string& name, const std::string& value);

  /**
   * @brief Remove custom header
   *
   * @param[in] name custom header name to remove the custom header
   *
   * @return true if succeeded, false otherwise
   */
  bool RemoveCustomHeader(const std::string& name);

  /**
   * @brief Start the inspector server
   *
   * @param[in] port port number
   *
   * @return the port number
   */
  uint32_t StartInspectorServer(uint32_t port);

  /**
   * @brief Stop the inspector server
   *
   * @return true if succeeded, false otherwise
   */
  bool StopInspectorServer();

  /**
   * @brief Scroll web page of view by deltaX and deltaY.
   *
   * @param[in] deltaX horizontal offset to scroll
   * @param[in] deltaY vertical offset to scroll
   */
  void ScrollBy(int32_t deltaX, int32_t deltaY);

  /**
   * @brief Scroll edge of view by deltaX and deltaY.
   *
   * @param[in] deltaX horizontal offset to scroll
   * @param[in] deltaY vertical offset to scroll
   *
   * @return true if succeeded, false otherwise
   */
  bool ScrollEdgeBy(int32_t deltaX, int32_t deltaY);

  /**
   * @brief Set an absolute scroll of the given view.
   */
  void SetScrollPosition(int32_t x, int32_t y);

  /**
   * @brief Get the current scroll position of the given view.
   */
  Dali::Vector2 GetScrollPosition() const;

  /**
   * @brief Get the possible scroll size of the given view.
   */
  Dali::Vector2 GetScrollSize() const;

  /**
   * @brief Get the last known content's size.
   */
  Dali::Vector2 GetContentSize() const;

  /**
   * @brief Return whether forward is possible.
   *
   * @return True if forward is possible, false otherwise
   */
  bool CanGoForward();

  /**
   * @brief Go to forward.
   */
  void GoForward();

  /**
   * @brief Return whether backward is possible.
   *
   * @return True if backward is possible, false otherwise
   */
  bool CanGoBack();

  /**
   * @brief Go to back.
   */
  void GoBack();

  /**
   * @brief Evaluate JavaScript code represented as a string.
   *
   * @param[in] script The JavaScript code
   * @param[in] resultHandler The callback function to be called by the JavaScript runtime. This carries evaluation result.
   */
  void EvaluateJavaScript(const std::string& script, Dali::WebEnginePlugin::JavaScriptMessageHandlerCallback resultHandler);

  /**
   * @brief Add a message handler into JavaScript.
   *
   * @param[in] exposedObjectName The name of exposed object
   * @param[in] handler The callback function
   */
  void AddJavaScriptMessageHandler(const std::string& exposedObjectName, Dali::WebEnginePlugin::JavaScriptMessageHandlerCallback handler);

  /**
   * @brief Add a message handler into JavaScript.
   *
   * @param[in] exposedObjectName The name of exposed object
   * @param[in] handler The callback function
   */
  void AddJavaScriptEntireMessageHandler(const std::string& exposedObjectName, Dali::WebEnginePlugin::JavaScriptEntireMessageHandlerCallback handler);

  /**
   * @brief Register a callback for JavaScript alert.
   *
   * @param[in] callback The callback function
   */
  void RegisterJavaScriptAlertCallback(Dali::WebEnginePlugin::JavaScriptAlertCallback callback);

  /**
   * @brief Reply for JavaScript alert.
   */
  void JavaScriptAlertReply();

  /**
   * @brief Register a callback for JavaScript confirm.
   *
   * @param[in] callback The callback function
   */
  void RegisterJavaScriptConfirmCallback(Dali::WebEnginePlugin::JavaScriptConfirmCallback callback);

  /**
   * @brief Reply for JavaScript confirm.
   * @param[in] confirmed True if confirmed, false otherwise.
   */
  void JavaScriptConfirmReply(bool confirmed);

  /**
   * @brief Register a callback for JavaScript prompt.
   *
   * @param[in] callback The callback function
   */
  void RegisterJavaScriptPromptCallback(Dali::WebEnginePlugin::JavaScriptPromptCallback callback);

  /**
   * @brief Reply for JavaScript prompt.
   * @param[in] result The result returned from input-field in prompt popup.
   */
  void JavaScriptPromptReply(const std::string& result);

  /**
   * @brief Create a new hit test.
   *
   * @param[in] x the horizontal position to query
   * @param[in] y the vertical position to query
   * @param[in] mode the mode of hit test
   *
   * @return a new hit test object
   */
  std::unique_ptr<Dali::WebEngineHitTest> CreateHitTest(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode);

  /**
   * @brief create a hit test asynchronously.
   *
   * @param[in] x the horizontal position to query
   * @param[in] y the vertical position to query
   * @param[in] mode the mode of hit test
   * @param[in] callback the callback function
   *
   * @return true if succeeded, false otherwise
   */
  bool CreateHitTestAsynchronously(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode, Dali::WebEnginePlugin::WebEngineHitTestCreatedCallback callback);

  /**
   * @brief Clear the history of Web.
   */
  void ClearHistory();

  /**
   * @brief Clear all tiles resources of Web.
   */
  void ClearAllTilesResources();

  /**
   * @brief Get user agent string.
   *
   * @return The string value of user agent
   */
  std::string GetUserAgent() const;

  /**
   * @brief Set user agent string.
   *
   * @param[in] userAgent The string value of user agent
   */
  void SetUserAgent(const std::string& userAgent);

  /**
   * @brief Set the size of Web Pages.
   */
  void SetSize(uint32_t width, uint32_t height);

  /**
   * @brief Set background color of web page.
   *
   * @param[in] color Background color
   */
  void SetDocumentBackgroundColor(Dali::Vector4 color);

  /**
   * @brief Clear tiles when hidden.
   *
   * @param[in] cleared Whether tiles are cleared or not
   */
  void ClearTilesWhenHidden(bool cleared);

  /**
   * @brief Set multiplier of cover area of tile.
   *
   * @param[in] multiplier The multiplier of cover area
   */
  void SetTileCoverAreaMultiplier(float multiplier);

  /**
   * @brief Enable cursor by client.
   *
   * @param[in] enabled Whether cursor is enabled or not
   */
  void EnableCursorByClient(bool enabled);

  /**
   * @brief Get the selected text.
   *
   * @return the selected text
   */
  std::string GetSelectedText() const;

  /**
   * @brief Send Touch Events.
   */
  bool SendTouchEvent(const TouchEvent& touch);

  /**
   * @brief Send key Events.
   */
  bool SendKeyEvent(const KeyEvent& event);

  /**
   * @brief Set focus.
   * @param[in] focused True if web view is focused, false otherwise
   */
  void SetFocus(bool focused);

  /**
   * @brief Set the style of IME.
   * @param[in] position Position of IME.
   * @param[in] alignment Alignment of IME.
   *
   * @return true if succeeded, false otherwise
   */
  bool SetImePositionAndAlignment(Dali::Vector2 position, int alignment);

  /**
   * @brief Set the theme name of cursor.
   * @param[in] themeName The name of theme of cursor.
   */
  void SetCursorThemeName(const std::string themeName);

  /**
   * @brief Enable/disable mouse events. The default is enabled.
   *
   * @param[in] enabled True if mouse events are enabled, false otherwise
   */
  void EnableMouseEvents(bool enabled);

  /**
   * @brief Enable/disable key events. The default is enabled.
   *
   * @param[in] enabled True if key events are enabled, false otherwise
   */
  void EnableKeyEvents(bool enabled);

  /**
   * @brief Set zoom factor of the current page.
   * @param[in] zoomFactor a new factor to be set.
   */
  void SetPageZoomFactor(float zoomFactor);

  /**
   * @brief Query the current zoom factor of the page。
   * @return The current page zoom factor.
   */
  float GetPageZoomFactor() const;

  /**
   * @brief Set the current text zoom level。.
   * @param[in] zoomFactor a new factor to be set.
   */
  void SetTextZoomFactor(float zoomFactor);

  /**
   * @brief Get the current text zoom level.
   * @return The current text zoom factor.
   */
  float GetTextZoomFactor() const;

  /**
   * @brief Get the current load progress of the page.
   * @return The load progress of the page.
   */
  float GetLoadProgressPercentage() const;

  /**
   * @brief Scale the current page, centered at the given point.
   * @param[in] scaleFactor a new factor to be scaled.
   * @param[in] point a center coordinate.
   */
  void SetScaleFactor(float scaleFactor, Dali::Vector2 point);

  /**
   * @brief Get the current scale factor of the page.
   * @return The current scale factor.
   */
  float GetScaleFactor() const;

  /**
   * @brief Request to activate/deactivate the accessibility usage set by web app.
   * @param[in] activated Activate accessibility or not.
   */
  void ActivateAccessibility(bool activated);

  /**
   * @brief Get the accessibility address (bus and path) for embedding.
   * @return Accessibility address of the root web content element.
   */
  Accessibility::Address GetAccessibilityAddress();

  /**
   * @brief Request to set the current page's visibility.
   * @param[in] visible Visible or not.
   *
   * @return true if changed successfully, false otherwise
   */
  bool SetVisibility(bool visible);

  /**
   * @brief Search and highlights the given string in the document.
   * @param[in] text The text to find
   * @param[in] options The options to find
   * @param[in] maxMatchCount The maximum match count to find
   *
   * @return true if found & highlighted, false otherwise
   */
  bool HighlightText(const std::string& text, Dali::WebEnginePlugin::FindOption options, uint32_t maxMatchCount);

  /**
   * @brief Add dynamic certificate path.
   * @param[in] host host that required client authentication
   * @param[in] certPath the file path stored certificate
   */
  void AddDynamicCertificatePath(const std::string& host, const std::string& certPath);

  /**
   * @brief Get snapshot of the specified viewArea of page.
   *
   * @param[in] viewArea The rectangle of screen shot
   * @param[in] scaleFactor The scale factor
   *
   * @return pixel data of screen shot
   */
  Dali::PixelData GetScreenshot(Dali::Rect<int32_t> viewArea, float scaleFactor);

  /**
   * @brief Request to get snapshot of the specified viewArea of page asynchronously.
   *
   * @param[in] viewArea The rectangle of screen shot
   * @param[in] scaleFactor The scale factor
   * @param[in] callback The callback for screen shot
   *
   * @return true if requested successfully, false otherwise
   */
  bool GetScreenshotAsynchronously(Dali::Rect<int32_t> viewArea, float scaleFactor, Dali::WebEnginePlugin::ScreenshotCapturedCallback callback);

  /**
   * @brief Asynchronous request to check if there is a video playing in the given view.
   *
   * @param[in] callback The callback called after checking if video is playing or not
   *
   * @return true if requested successfully, false otherwise
   */
  bool CheckVideoPlayingAsynchronously(Dali::WebEnginePlugin::VideoPlayingCallback callback);

  /**
   * @brief Set callback which alled upon geolocation permission request.
   *
   * @param[in] callback The callback for requesting geolocation permission
   */
  void RegisterGeolocationPermissionCallback(Dali::WebEnginePlugin::GeolocationPermissionCallback callback);

  /**
   * @brief Update display area.
   * @param[in] displayArea The area to display web page
   */
  void UpdateDisplayArea(Dali::Rect<int32_t> displayArea);

  /**
   * @brief Enable video hole.
   * @param[in] enabled True if video hole is enabled, false otherwise
   */
  void EnableVideoHole(bool enabled);

  /**
   * @brief Send hover events.
   * @param[in] event The hover event would be sent.
   */
  bool SendHoverEvent(const HoverEvent& event);

  /**
   * @brief Send wheel events.
   * @param[in] event The wheel event would be sent.
   */
  bool SendWheelEvent(const WheelEvent& event);

  /**
   * @brief Exit full-screen.
   */
  void ExitFullscreen();

  /**
   * @brief Callback to be called when frame would be rendered.
   *
   * @param[in] callback
   */
  void RegisterFrameRenderedCallback(Dali::WebEnginePlugin::WebEngineFrameRenderedCallback callback);

  /**
   * @brief Callback to be called when page loading is started.
   *
   * @param[in] callback
   */
  void RegisterPageLoadStartedCallback(Dali::WebEnginePlugin::WebEnginePageLoadCallback callback);

  /**
   * @brief Callback to be called when page loading is in progress.
   *
   * @param[in] callback
   */
  void RegisterPageLoadInProgressCallback(Dali::WebEnginePlugin::WebEnginePageLoadCallback callback);

  /**
   * @brief Callback to be called when page loading is finished.
   *
   * @param[in] callback
   */
  void RegisterPageLoadFinishedCallback(Dali::WebEnginePlugin::WebEnginePageLoadCallback callback);

  /**
   * @brief Callback to be called when an error occurs in page loading.
   *
   * @param[in] callback
   */
  void RegisterPageLoadErrorCallback(Dali::WebEnginePlugin::WebEnginePageLoadErrorCallback callback);

  /**
   * @brief Callback to be called when scroll edge is reached.
   *
   * @param[in] callback
   */
  void RegisterScrollEdgeReachedCallback(Dali::WebEnginePlugin::WebEngineScrollEdgeReachedCallback callback);

  /**
   * @brief Callback to be called when url is changed.
   *
   * @param[in] callback
   */
  void RegisterUrlChangedCallback(Dali::WebEnginePlugin::WebEngineUrlChangedCallback callback);

  /**
   * @brief Callback to be called when form repost decision is requested.
   *
   * @param[in] callback
   */
  void RegisterFormRepostDecidedCallback(Dali::WebEnginePlugin::WebEngineFormRepostDecidedCallback callback);

  /**
   * @brief Callback to be called when console message will be logged.
   *
   * @param[in] callback
   */
  void RegisterConsoleMessageReceivedCallback(Dali::WebEnginePlugin::WebEngineConsoleMessageReceivedCallback callback);

  /**
   * @brief Callback to be called when response policy would be decided.
   *
   * @param[in] callback
   */
  void RegisterResponsePolicyDecidedCallback(Dali::WebEnginePlugin::WebEngineResponsePolicyDecidedCallback callback);

  /**
   * @brief Callback to be called when navigation policy would be decided.
   *
   * @param[in] callback
   */
  void RegisterNavigationPolicyDecidedCallback(Dali::WebEnginePlugin::WebEngineNavigationPolicyDecidedCallback callback);

  /**
   * @brief Callback to be called when new window policy would be decided.
   *
   * @param[in] callback
   */
  void RegisterNewWindowPolicyDecidedCallback(Dali::WebEnginePlugin::WebEngineNewWindowPolicyDecidedCallback callback);

  /**
   * @brief Callback to be called when a new window would be created.
   *
   * @param[in] callback
   */
  void RegisterNewWindowCreatedCallback(Dali::WebEnginePlugin::WebEngineNewWindowCreatedCallback callback);

  /**
   * @brief Callback to be called when certificate need be confirmed.
   *
   * @param[in] callback
   */
  void RegisterCertificateConfirmedCallback(Dali::WebEnginePlugin::WebEngineCertificateCallback callback);

  /**
   * @brief Callback to be called when ssl certificate is changed.
   *
   * @param[in] callback
   */
  void RegisterSslCertificateChangedCallback(Dali::WebEnginePlugin::WebEngineCertificateCallback callback);

  /**
   * @brief Callback to be called when http authentication need be confirmed.
   *
   * @param[in] callback
   */
  void RegisterHttpAuthHandlerCallback(Dali::WebEnginePlugin::WebEngineHttpAuthHandlerCallback callback);

  /**
   * @brief Callback to be called when context menu would be shown.
   *
   * @param[in] callback
   */
  void RegisterContextMenuShownCallback(Dali::WebEnginePlugin::WebEngineContextMenuShownCallback callback);

  /**
   * @brief Callback to be called when context menu would be hidden.
   *
   * @param[in] callback
   */
  void RegisterContextMenuHiddenCallback(Dali::WebEnginePlugin::WebEngineContextMenuHiddenCallback callback);

  /**
   * @brief Callback to be called when fullscreen would be entered.
   *
   * @param[in] callback
   */
  void RegisterFullscreenEnteredCallback(Dali::WebEnginePlugin::WebEngineFullscreenEnteredCallback callback);

  /**
   * @brief Callback to be called when fullscreen would be exited.
   *
   * @param[in] callback
   */
  void RegisterFullscreenExitedCallback(Dali::WebEnginePlugin::WebEngineFullscreenExitedCallback callback);

  /**
   * @brief Callback to be called when text would be found.
   *
   * @param[in] callback
   */
  void RegisterTextFoundCallback(Dali::WebEnginePlugin::WebEngineTextFoundCallback callback);

  /**
   * @brief Get a plain text of current web page asynchronously.
   *
   * @param[in] callback The callback function called asynchronously.
   */
  void GetPlainTextAsynchronously(Dali::WebEnginePlugin::PlainTextReceivedCallback callback);

  /**
   * @brief Cancel WebAuthentication(cancel in progress passkey operation).
   */
  void WebAuthenticationCancel();

  /**
   * @brief Get a plain text of current web page asynchronously.
   *
   * @param[in] callback The callback function called asynchronously.

   */
  void RegisterWebAuthDisplayQRCallback(Dali::WebEnginePlugin::WebEngineWebAuthDisplayQRCallback callback);

  /**
   * @brief Get a plain text of current web page asynchronously.
   *
   * @param[in] callback The callback function called asynchronously.

   */
  void RegisterWebAuthResponseCallback(Dali::WebEnginePlugin::WebEngineWebAuthResponseCallback callback);

  /**
   * @brief Register UserMediaPermissionRequest callback.
   *
   * @param[in] callback The callback to be called for handling user media permission.
   */
  void RegisterUserMediaPermissionRequestCallback(Dali::WebEnginePlugin::WebEngineUserMediaPermissionRequestCallback callback);


private: // Not intended for application developers
  /**
   * @brief Internal constructor
   */
  explicit DALI_INTERNAL WebEngine(Internal::Adaptor::WebEngine* internal);
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_H
