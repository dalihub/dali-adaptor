#ifndef DALI_WEB_ENGINE_PLUGIN_H
#define DALI_WEB_ENGINE_PLUGIN_H

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
#include <dali/public-api/images/native-image-interface.h>
#include <dali/public-api/math/rect.h>
#include <functional>
#include <memory>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/accessibility.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-hit-test.h>
#include <dali/devel-api/common/bitwise-enum.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>

namespace Dali
{
class KeyEvent;
class PixelData;
class TouchEvent;
class WebEngineBackForwardList;
class WebEngineCertificate;
class WebEngineConsoleMessage;
class WebEngineContextMenu;
class WebEngineFormRepostDecision;
class WebEngineHitTest;
class WebEngineHttpAuthHandler;
class WebEngineLoadError;
class WebEnginePolicyDecision;
class WebEngineSettings;
class HoverEvent;
class WheelEvent;
class WebEngineUserMediaPermissionRequest;
class WebEngineDeviceListGet;

/**
 * @brief WebEnginePlugin is an abstract interface, used by dali-adaptor to access WebEngine plugin.
 * A concrete implementation must be created for each platform and provided as dynamic library.
 */
class WebEnginePlugin
{
public:
  /**
   * @brief WebEngine callback related with page loading.
   */
  using WebEnginePageLoadCallback = std::function<void(const std::string&)>;

  /**
   * @brief WebView callback related with page loading error.
   */
  using WebEnginePageLoadErrorCallback = std::function<void(std::unique_ptr<Dali::WebEngineLoadError>)>;

  // forward declaration.
  enum class ScrollEdge;

  /**
   * @brief WebView callback related with scroll edge reached.
   */
  using WebEngineScrollEdgeReachedCallback = std::function<void(const ScrollEdge)>;

  /**
   * @brief WebView callback related with page url changed.
   */
  using WebEngineUrlChangedCallback = std::function<void(const std::string&)>;

  /**
   * @brief WebView callback related with screen captured.
   */
  using ScreenshotCapturedCallback = std::function<void(Dali::PixelData)>;

  /**
   * @brief WebView callback related with geolocation permission.
   *  Host and protocol of security origin will be provided when requesting
   *  geolocation permission.
   *  It returns true if a pop-up is created successfully, false otherwise.
   */
  using GeolocationPermissionCallback = std::function<bool(const std::string&, const std::string&)>;

  /**
   * @brief WebView callback related with video playing.
   */
  using VideoPlayingCallback = std::function<void(bool)>;

  /**
   * @brief WebView callback related with console message logged.
   */
  using WebEngineConsoleMessageReceivedCallback = std::function<void(std::unique_ptr<Dali::WebEngineConsoleMessage>)>;

  /**
   * @brief WebView callback related with certificate changed.
   */
  using WebEngineCertificateCallback = std::function<void(std::unique_ptr<Dali::WebEngineCertificate>)>;

  /**
   * @brief WebView callback related with http authentication.
   */
  using WebEngineHttpAuthHandlerCallback = std::function<void(std::unique_ptr<Dali::WebEngineHttpAuthHandler>)>;

  /**
   * @brief WebView callback related with context menu shown.
   */
  using WebEngineContextMenuShownCallback = std::function<void(std::unique_ptr<Dali::WebEngineContextMenu>)>;

  /**
   * @brief WebView callback related with context menu hidden.
   */
  using WebEngineContextMenuHiddenCallback = std::function<void(std::unique_ptr<Dali::WebEngineContextMenu>)>;

  /**
   * @brief Message result callback when JavaScript is executed with a message.
   */
  using JavaScriptMessageHandlerCallback = std::function<void(const std::string&)>;

  /**
   * @brief Message result callback when JavaScript is executed with message name and body.
   */
  using JavaScriptEntireMessageHandlerCallback = std::function<void(const std::string&, const std::string&)>;

  /**
   * @brief Alert callback when JavaScript alert is called with a message.
   *  It returns true if a pop-up is created successfully, false otherwise.
   */
  using JavaScriptAlertCallback = std::function<bool(const std::string&)>;

  /**
   * @brief Confirm callback when JavaScript confirm is called with a message.
   *  It returns true if a pop-up is created successfully, false otherwise.
   */
  using JavaScriptConfirmCallback = std::function<bool(const std::string&)>;

  /**
   * @brief Prompt callback when JavaScript prompt is called with a message
   *  and an optional value that is the default value for the input field.
   *  It returns true if a pop-up is created successfully, false otherwise.
   */
  using JavaScriptPromptCallback = std::function<bool(const std::string&, const std::string&)>;

  /**
   * @brief WebView callback related with form repost decision.
   */
  using WebEngineFormRepostDecidedCallback = std::function<void(std::unique_ptr<Dali::WebEngineFormRepostDecision>)>;

  /**
   * @brief WebView callback related with frame rendered.
   */
  using WebEngineFrameRenderedCallback = std::function<void(void)>;

  /**
   * @brief WebView callback related with response policy would be decided.
   */
  using WebEngineResponsePolicyDecidedCallback = std::function<void(std::unique_ptr<Dali::WebEnginePolicyDecision>)>;

  /**
   * @brief WebView callback related with navigation policy would be decided.
   */
  using WebEngineNavigationPolicyDecidedCallback = std::function<void(std::unique_ptr<Dali::WebEnginePolicyDecision>)>;

  /**
   * @brief WebView callback related with new window policy would be decided.
   */
  using WebEngineNewWindowPolicyDecidedCallback = std::function<void(std::unique_ptr<Dali::WebEnginePolicyDecision>)>;

  /**
   * @brief WebView callback related with a new window would be created.
   */
  using WebEngineNewWindowCreatedCallback = std::function<void(Dali::WebEnginePlugin*&)>;

  /**
   * @brief Hit test callback called after hit test is created asynchronously.
   */
  using WebEngineHitTestCreatedCallback = std::function<bool(std::unique_ptr<Dali::WebEngineHitTest>)>;

  /**
   * @brief WebView callback called after fullscreen is entered.
   */
  using WebEngineFullscreenEnteredCallback = std::function<void(void)>;

  /**
   * @brief WebView callback called after fullscreen is exited.
   */
  using WebEngineFullscreenExitedCallback = std::function<void(void)>;

  /**
   * @brief WebView callback called after text is found.
   */
  using WebEngineTextFoundCallback = std::function<void(uint32_t)>;

  /**
   * @brief The callback to be called when the web engine received a plain text of current web page.
   */
  using PlainTextReceivedCallback = std::function<void(const std::string&)>;

  /**
   * @brief WebView callback informs browser app to display QB code popup for passkey scenario.
   */
  using WebEngineWebAuthDisplayQRCallback = std::function<void(const std::string&)>;

  /**
   * @brief WebView callback informs browser app that the passkey registration and authentication has been successful and app can close QR popup.
   */
  using WebEngineWebAuthResponseCallback = std::function<void(void)>;

  /**
   * @brief The callback to be called when the web engine received a user media permission reqeust from user application.
   */
  using WebEngineUserMediaPermissionRequestCallback = std::function<void(Dali::WebEngineUserMediaPermissionRequest*, const std::string&)>;

  /**
   * @brief The callback to be called when the web engine received a device connection changed event.
   */
  using WebEngineDeviceConnectionChangedCallback = std::function<void(int32_t)>;

  /**
   * @brief The callback to be called when the web engine received a device list.
   */
  using WebEngineDeviceListGetCallback = std::function<void(Dali::WebEngineDeviceListGet*, int32_t)>;


  /**
   * @brief Enumeration for the scroll edge.
   */
  enum class ScrollEdge
  {
    LEFT,   ///< Left edge reached.
    RIGHT,  ///< Right edge reached.
    TOP,    ///< Top edge reached.
    BOTTOM, ///< Bottom edge reached.
  };

  /**
   * @brief Enumeration that provides the option to find text.
   */
  enum class FindOption
  {
    NONE                               = 0,      ///<  No search flags, this means a case sensitive, no wrap, forward only search
    CASE_INSENSITIVE                   = 1 << 0, ///<  Case insensitive search
    AT_WORD_STARTS                     = 1 << 1, ///<  Search text only at the beginning of the words
    TREAT_MEDIAL_CAPITAL_AS_WORD_START = 1 << 2, ///<  Treat capital letters in the middle of words as word start
    BACKWARDS                          = 1 << 3, ///<  Search backwards
    WRAP_AROUND                        = 1 << 4, ///<  If not present the search stops at the end of the document
    SHOW_OVERLAY                       = 1 << 5, ///<  Show overlay
    SHOW_FIND_INDICATOR                = 1 << 6, ///<  Show indicator
    SHOW_HIGHLIGHT                     = 1 << 7, ///<  Show highlight
  };

  /**
   * @brief Constructor.
   */
  WebEnginePlugin() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEnginePlugin() = default;

  /**
   * @brief Create WebEngine instance.
   *
   * @param [in] width The width of Web
   * @param [in] height The height of Web
   * @param [in] locale The locale of Web
   * @param [in] timezoneId The timezoneID of Web
   */
  virtual void Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId) = 0;

  /**
   * @brief Create WebEngine instance.
   *
   * @param [in] width The width of Web
   * @param [in] height The height of Web
   * @param [in] argc The count of application arguments
   * @param [in] argv The string array of application arguments
   */
  virtual void Create(uint32_t width, uint32_t height, uint32_t argc, char** argv) = 0;

  /**
   * @brief Destroy WebEngine instance.
   */
  virtual void Destroy() = 0;

  /**
   * @brief Get settings of WebEngine.
   */
  virtual WebEngineSettings& GetSettings() const = 0;

  /**
   * @brief Get back-forward list of WebEngine.
   */
  virtual WebEngineBackForwardList& GetBackForwardList() const = 0;

  /**
   * @brief Load a web page based on a given URL.
   *
   * @param [in] url The URL of the resource to load
   */
  virtual void LoadUrl(const std::string& url) = 0;

  /**
   * @brief Return the title of the Web.
   *
   * @return The title of web page
   */
  virtual std::string GetTitle() const = 0;

  /**
   * @brief Return the Favicon of the Web.
   *
   * @return Favicon of Dali::PixelData& type
   */
  virtual Dali::PixelData GetFavicon() const = 0;

  /**
   * @brief Get image to render.
   */
  virtual NativeImageSourcePtr GetNativeImageSource() = 0;

  /**
   * @brief Change orientation.
   */
  virtual void ChangeOrientation(int orientation) = 0;

  /**
   * @brief Return the URL of the Web.
   *
   * @return Url of string type
   */
  virtual std::string GetUrl() const = 0;

  /**
   * @brief Load a given string as web contents.
   *
   * @param [in] htmlString The string to use as the contents of the web page
   */
  virtual void LoadHtmlString(const std::string& htmlString) = 0;

  /**
   * @brief Load the specified html string as the content of the view overriding current history entry
   *
   * @param[in] html HTML data to load
   * @param[in] basicUri Base URL used for relative paths to external objects
   * @param[in] unreachableUrl URL that could not be reached
   *
   * @return true if successfully loaded, false otherwise
   */
  virtual bool LoadHtmlStringOverrideCurrentEntry(const std::string& html, const std::string& basicUri, const std::string& unreachableUrl) = 0;

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
  virtual bool LoadContents(const int8_t* contents, uint32_t contentSize, const std::string& mimeType, const std::string& encoding, const std::string& baseUri) = 0;

  /**
   * @brief Reload the Web.
   */
  virtual void Reload() = 0;

  /**
   * @brief Reload the current page's document without cache
   */
  virtual bool ReloadWithoutCache() = 0;

  /**
   * @brief Stop loading web contents on the current page.
   */
  virtual void StopLoading() = 0;

  /**
   * @brief Suspend the operation associated with the view.
   */
  virtual void Suspend() = 0;

  /**
   * @brief Resume the operation associated with the view object after calling Suspend().
   */
  virtual void Resume() = 0;

  /**
   * @brief To suspend all url loading
   */
  virtual void SuspendNetworkLoading() = 0;

  /**
   * @brief To resume new url network loading
   */
  virtual void ResumeNetworkLoading() = 0;

  /**
   * @brief Add custom header
   *
   * @param[in] name custom header name to add the custom header
   * @param[in] value custom header value to add the custom header
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool AddCustomHeader(const std::string& name, const std::string& value) = 0;

  /**
   * @brief Remove custom header
   *
   * @param[in] name custom header name to remove the custom header
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool RemoveCustomHeader(const std::string& name) = 0;

  /**
   * @brief Start the inspector server
   *
   * @param[in] port port number
   *
   * @return the port number
   */
  virtual uint32_t StartInspectorServer(uint32_t port) = 0;

  /**
   * @brief Stop the inspector server
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool StopInspectorServer() = 0;

  /**
   * @brief Scroll web page of view by deltaX and deltaY.
   *
   * @param[in] deltaX horizontal offset to scroll
   * @param[in] deltaY vertical offset to scroll
   */
  virtual void ScrollBy(int32_t deltaX, int32_t deltaY) = 0;

  /**
   * @brief Scroll edge of view by deltaX and deltaY.
   *
   * @param[in] deltaX horizontal offset to scroll
   * @param[in] deltaY vertical offset to scroll
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool ScrollEdgeBy(int32_t deltaX, int32_t deltaY) = 0;

  /**
   * @brief Scroll to the specified position of the given view.
   */
  virtual void SetScrollPosition(int32_t x, int32_t y) = 0;

  /**
   * @brief Get the current scroll position of the given view.
   */
  virtual Dali::Vector2 GetScrollPosition() const = 0;

  /**
   * @brief Get the possible scroll size of the given view.
   */
  virtual Dali::Vector2 GetScrollSize() const = 0;

  /**
   * @brief Get the last known content's size.
   */
  virtual Dali::Vector2 GetContentSize() const = 0;

  /**
   * @brief Return whether forward is possible.
   *
   * @return True if forward is possible, false otherwise
   */
  virtual bool CanGoForward() = 0;

  /**
   * @brief Go to forward.
   */
  virtual void GoForward() = 0;

  /**
   * @brief Return whether backward is possible.
   *
   * @return True if backward is possible, false otherwise
   */
  virtual bool CanGoBack() = 0;

  /**
   * @brief Go to back.
   */
  virtual void GoBack() = 0;

  /**
   * @brief Evaluate JavaScript code represented as a string.
   *
   * @param[in] script The JavaScript code
   * @param[in] resultHandler The callback function to be called by the JavaScript runtime. This carries evaluation result.
   */
  virtual void EvaluateJavaScript(const std::string& script, JavaScriptMessageHandlerCallback resultHandler) = 0;

  /**
   * @brief Add a message handler into JavaScript.
   *
   * @param[in] exposedObjectName The name of exposed object
   * @param[in] handler The callback function
   */
  virtual void AddJavaScriptMessageHandler(const std::string& exposedObjectName, JavaScriptMessageHandlerCallback handler) = 0;

  /**
   * @brief Add a message handler into JavaScript.
   *
   * @param[in] exposedObjectName The name of exposed object
   * @param[in] handler The callback function
   */
  virtual void AddJavaScriptEntireMessageHandler(const std::string& exposedObjectName, JavaScriptEntireMessageHandlerCallback handler) = 0;

  /**
   * @brief Register a callback for JavaScript alert.
   *
   * @param[in] callback The callback function
   */
  virtual void RegisterJavaScriptAlertCallback(JavaScriptAlertCallback callback) = 0;

  /**
   * @brief Reply for JavaScript alert.
   */
  virtual void JavaScriptAlertReply() = 0;

  /**
   * @brief Register a callback for JavaScript confirm.
   *
   * @param[in] callback The callback function
   */
  virtual void RegisterJavaScriptConfirmCallback(JavaScriptConfirmCallback callback) = 0;

  /**
   * @brief Reply for JavaScript confirm.
   */
  virtual void JavaScriptConfirmReply(bool confirmed) = 0;

  /**
   * @brief Register a callback for JavaScript prompt.
   *
   * @param[in] callback The callback function
   */
  virtual void RegisterJavaScriptPromptCallback(JavaScriptPromptCallback callback) = 0;

  /**
   * @brief Reply for JavaScript prompt.
   */
  virtual void JavaScriptPromptReply(const std::string& result) = 0;

  /**
   * @brief Create a new hit test.
   *
   * @param[in] x the horizontal position to query
   * @param[in] y the vertical position to query
   * @param[in] mode the mode of hit test
   *
   * @return a new hit test object.
   */
  virtual std::unique_ptr<Dali::WebEngineHitTest> CreateHitTest(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode) = 0;

  /**
   * @brief create a hit test asynchronously.
   *
   * @param[in] x the horizontal position to query
   * @param[in] y the vertical position to query
   * @param[in] mode the mode of hit test
   * @param[in] callback The callback function
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool CreateHitTestAsynchronously(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode, WebEngineHitTestCreatedCallback callback) = 0;

  /**
   * @brief Clear the history of Web.
   */
  virtual void ClearHistory() = 0;

  /**
   * @brief Clear all tiles resources of Web.
   */
  virtual void ClearAllTilesResources() = 0;

  /**
   * @brief Get user agent string.
   *
   * @return The string value of user agent
   */
  virtual std::string GetUserAgent() const = 0;

  /**
   * @brief Set user agent string.
   *
   * @param[in] userAgent The string value of user agent
   */
  virtual void SetUserAgent(const std::string& userAgent) = 0;

  /**
   * @brief Set size of Web Page.
   */
  virtual void SetSize(uint32_t width, uint32_t height) = 0;

  /**
   * @brief Set background color of web page.
   *
   * @param[in] color Background color
   */
  virtual void SetDocumentBackgroundColor(Dali::Vector4 color) = 0;

  /**
   * @brief Clear tiles when hidden.
   *
   * @param[in] cleared Whether tiles are cleared or not
   */
  virtual void ClearTilesWhenHidden(bool cleared) = 0;

  /**
   * @brief Set multiplier of cover area of tile.
   *
   * @param[in] multiplier The multiplier of cover area
   */
  virtual void SetTileCoverAreaMultiplier(float multiplier) = 0;

  /**
   * @brief Enable cursor by client.
   *
   * @param[in] enabled Whether cursor is enabled or not
   */
  virtual void EnableCursorByClient(bool enabled) = 0;

  /**
   * @brief Get the selected text.
   *
   * @return the selected text
   */
  virtual std::string GetSelectedText() const = 0;

  /**
   * @brief Send Touch Events.
   */
  virtual bool SendTouchEvent(const TouchEvent& touch) = 0;

  /**
   * @brief Send Key Events.
   */
  virtual bool SendKeyEvent(const KeyEvent& event) = 0;

  /**
   * @brief Support mouse events or not.
   * @param[in] enabled True if enabled, false othewise.
   */
  virtual void EnableMouseEvents(bool enabled) = 0;

  /**
   * @brief Support key events or not.
   * @param[in] enabled True if enabled, false othewise.
   */
  virtual void EnableKeyEvents(bool enabled) = 0;

  /**
   * @brief Set focus.
   * @param[in] focused True if focus is gained, false lost.
   */
  virtual void SetFocus(bool focused) = 0;

  /**
   * @brief Set the style of IME.
   * @param[in] position Position of IME.
   * @param[in] alignment Alignment of IME.
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool SetImePositionAndAlignment(Dali::Vector2 position, int alignment) = 0;

  /**
   * @brief Set the theme name of cursor.
   * @param[in] themeName The name of theme of cursor.
   */
  virtual void SetCursorThemeName(const std::string themeName) = 0;

  /**
   * @brief Set zoom factor of the current page.
   * @param[in] zoomFactor a new factor to be set.
   */
  virtual void SetPageZoomFactor(float zoomFactor) = 0;

  /**
   * @brief Query the current zoom factor of the page。
   * @return The current page zoom factor.
   */
  virtual float GetPageZoomFactor() const = 0;

  /**
   * @brief Set the current text zoom level。.
   * @param[in] zoomFactor a new factor to be set.
   */
  virtual void SetTextZoomFactor(float zoomFactor) = 0;

  /**
   * @brief Get the current text zoom level.
   * @return The current text zoom factor.
   */
  virtual float GetTextZoomFactor() const = 0;

  /**
   * @brief Get the current load progress of the page.
   * @return The load progress of the page.
   */
  virtual float GetLoadProgressPercentage() const = 0;

  /**
   * @brief Scale the current page, centered at the given point.
   * @param[in] scaleFactor a new factor to be scaled.
   * @param[in] point a center coordinate.
   */
  virtual void SetScaleFactor(float scaleFactor, Dali::Vector2 point) = 0;

  /**
   * @brief Get the current scale factor of the page.
   * @return The current scale factor.
   */
  virtual float GetScaleFactor() const = 0;

  /**
   * @brief Request to activate/deactivate the accessibility usage set by web app.
   * @param[in] activated Activate accessibility or not.
   */
  virtual void ActivateAccessibility(bool activated) = 0;

  /**
   * @brief Get the accessibility address (bus and path) for embedding.
   * @return Accessibility address of the root web content element.
   */
  virtual Accessibility::Address GetAccessibilityAddress() = 0;

  /**
   * @brief Request to set the current page's visibility.
   * @param[in] visible Visible or not.
   *
   * @return true if succeeded, false otherwise
   */
  virtual bool SetVisibility(bool visible) = 0;

  /**
   * @brief Search and highlight the given string in the document.
   * @param[in] text The text to find
   * @param[in] options The options to find
   * @param[in] maxMatchCount The maximum match count to find
   *
   * @return true if found & highlighted, false otherwise
   */
  virtual bool HighlightText(const std::string& text, FindOption options, uint32_t maxMatchCount) = 0;

  /**
   * @brief Add dynamic certificate path.
   * @param[in] host host that required client authentication
   * @param[in] certPath the file path stored certificate
   */
  virtual void AddDynamicCertificatePath(const std::string& host, const std::string& certPath) = 0;

  /**
   * @brief Get snapshot of the specified viewArea of page.
   *
   * @param[in] viewArea The rectangle of screen shot
   * @param[in] scaleFactor The scale factor
   *
   * @return pixel data of screen shot
   */
  virtual Dali::PixelData GetScreenshot(Dali::Rect<int32_t> viewArea, float scaleFactor) = 0;

  /**
   * @brief Request to get snapshot of the specified viewArea of page asynchronously.
   *
   * @param[in] viewArea The rectangle of screen shot
   * @param[in] scaleFactor The scale factor
   * @param[in] callback The callback for screen shot
   *
   * @return true if requested successfully, false otherwise
   */
  virtual bool GetScreenshotAsynchronously(Dali::Rect<int32_t> viewArea, float scaleFactor, ScreenshotCapturedCallback callback) = 0;

  /**
   * @brief Asynchronously request to check if there is a video playing in the given view.
   *
   * @param[in] callback The callback called after checking if video is playing or not
   *
   * @return true if requested successfully, false otherwise
   */
  virtual bool CheckVideoPlayingAsynchronously(VideoPlayingCallback callback) = 0;

  /**
   * @brief Set callback which will be called upon geolocation permission request.
   *
   * @param[in] callback The callback for requesting geolocation permission
   */
  virtual void RegisterGeolocationPermissionCallback(GeolocationPermissionCallback callback) = 0;

  /**
   * @brief Update display area.
   * @param[in] displayArea The display area need be updated.
   */
  virtual void UpdateDisplayArea(Dali::Rect<int32_t> displayArea) = 0;

  /**
   * @brief Enable video hole.
   * @param[in] enabled True if enabled, false othewise.
   */
  virtual void EnableVideoHole(bool enabled) = 0;

  /**
   * @brief Send Hover Events.
   * @param[in] event The hover event would be sent.
   */
  virtual bool SendHoverEvent(const HoverEvent& event) = 0;

  /**
   * @brief Send Wheel Events.
   * @param[in] event The wheel event would be sent.
   */
  virtual bool SendWheelEvent(const WheelEvent& event) = 0;

  /**
   * @brief Exit fullscreen.
   */
  virtual void ExitFullscreen() = 0;

  /**
   * @brief Callback to be called when frame would be rendered.
   *
   * @param[in] callback
   */
  virtual void RegisterFrameRenderedCallback(WebEngineFrameRenderedCallback callback) = 0;

  /**
   * @brief Callback to be called when page loading is started.
   *
   * @param[in] callback
   */
  virtual void RegisterPageLoadStartedCallback(WebEnginePageLoadCallback callback) = 0;

  /**
   * @brief Callback to be called when page loading is in progress.
   *
   * @param[in] callback
   */
  virtual void RegisterPageLoadInProgressCallback(WebEnginePageLoadCallback callback) = 0;

  /**
   * @brief Callback to be called when page loading is finished.
   *
   * @param[in] callback
   */
  virtual void RegisterPageLoadFinishedCallback(WebEnginePageLoadCallback callback) = 0;

  /**
   * @brief Callback to be called when an error occurs in page loading.
   *
   * @param[in] callback
   */
  virtual void RegisterPageLoadErrorCallback(WebEnginePageLoadErrorCallback callback) = 0;

  /**
   * @brief Callback to be called when scroll edge is reached.
   *
   * @param[in] callback
   */
  virtual void RegisterScrollEdgeReachedCallback(WebEngineScrollEdgeReachedCallback callback) = 0;

  /**
   * @brief Callback to be called when url is changed.
   *
   * @param[in] callback
   */
  virtual void RegisterUrlChangedCallback(WebEngineUrlChangedCallback callback) = 0;

  /**
   * @brief Callback to be called when form repost decision is requested.
   *
   * @param[in] callback
   */
  virtual void RegisterFormRepostDecidedCallback(WebEngineFormRepostDecidedCallback callback) = 0;

  /**
   * @brief Callback to be called when console message will be logged.
   *
   * @param[in] callback
   */
  virtual void RegisterConsoleMessageReceivedCallback(WebEngineConsoleMessageReceivedCallback callback) = 0;

  /**
   * @brief Callback to be called when response policy would be decided.
   *
   * @param[in] callback
   */
  virtual void RegisterResponsePolicyDecidedCallback(WebEngineResponsePolicyDecidedCallback callback) = 0;

  /**
   * @brief Callback to be called when navigation policy would be decided.
   *
   * @param[in] callback
   */
  virtual void RegisterNavigationPolicyDecidedCallback(WebEngineNavigationPolicyDecidedCallback callback) = 0;

  /**
   * @brief Callback to be called when new window policy would be decided.
   *
   * @param[in] callback
   */
  virtual void RegisterNewWindowPolicyDecidedCallback(WebEngineNewWindowPolicyDecidedCallback callback) = 0;

  /**
   * @brief Callback to be called when a new window would be created.
   *
   * @param[in] callback
   */
  virtual void RegisterNewWindowCreatedCallback(WebEngineNewWindowCreatedCallback callback) = 0;

  /**
   * @brief Callback to be called when certificate need be confirmed.
   *
   * @param[in] callback
   */
  virtual void RegisterCertificateConfirmedCallback(WebEngineCertificateCallback callback) = 0;

  /**
   * @brief Callback to be called when ssl certificate is changed.
   *
   * @param[in] callback
   */
  virtual void RegisterSslCertificateChangedCallback(WebEngineCertificateCallback callback) = 0;

  /**
   * @brief Callback to be called when http authentication need be confirmed.
   *
   * @param[in] callback
   */
  virtual void RegisterHttpAuthHandlerCallback(WebEngineHttpAuthHandlerCallback callback) = 0;

  /**
   * @brief Callback to be called when context menu would be shown.
   *
   * @param[in] callback
   */
  virtual void RegisterContextMenuShownCallback(WebEngineContextMenuShownCallback callback) = 0;

  /**
   * @brief Callback to be called when context menu would be hidden.
   *
   * @param[in] callback
   */
  virtual void RegisterContextMenuHiddenCallback(WebEngineContextMenuHiddenCallback callback) = 0;

  /**
   * @brief Callback to be called when fullscreen would be entered.
   *
   * @param[in] callback
   */
  virtual void RegisterFullscreenEnteredCallback(WebEngineFullscreenEnteredCallback callback) = 0;

  /**
   * @brief Callback to be called when fullscreen would be exited.
   *
   * @param[in] callback
   */
  virtual void RegisterFullscreenExitedCallback(WebEngineFullscreenExitedCallback callback) = 0;

  /**
   * @brief Callback to be called when text would be found.
   *
   * @param[in] callback
   */
  virtual void RegisterTextFoundCallback(WebEngineTextFoundCallback callback) = 0;

  /**
   * @brief Get a plain text of current web page asynchronously.
   *
   * @param[in] callback The callback function called asynchronously.
   */
  virtual void GetPlainTextAsynchronously(PlainTextReceivedCallback callback) = 0;

  /**
   * @brief Cancel WebAuthentication(cancel in progress passkey operation).
   */
  virtual void WebAuthenticationCancel() = 0;

  /**
   * @brief Register WebAuthDisplayQR callback.
   *
   * @param[in] callback The callback informs browser app to display QR code popup for passkey scenario.
   */
  virtual void RegisterWebAuthDisplayQRCallback(WebEngineWebAuthDisplayQRCallback callback) = 0;

  /**
   * @brief Register WebAuthResponse callback.
   *
   * @param[in] callback The callback informs browser app that the passkey registration and authentication has been successful and app can close QR popup.
   */
  virtual void RegisterWebAuthResponseCallback(WebEngineWebAuthResponseCallback callback) = 0;

  /**
   * @brief Register UserMediaPermissionRequest callback.
   *
   * @param[in] callback The callback to be called for handling user media permission.
   */
  virtual void RegisterUserMediaPermissionRequestCallback(WebEngineUserMediaPermissionRequestCallback callback) = 0;

  /**
   * @brief Callback to be called when device connection changed.
   *
   * @param[in] callback
   */
  virtual void RegisterDeviceConnectionChangedCallback(WebEngineDeviceConnectionChangedCallback callback) = 0;

  /**
   * @brief Callback to be called to get device list.
   *
   * @param[in] callback
   */
  virtual void RegisterDeviceListGetCallback(WebEngineDeviceListGetCallback callback) = 0;

  /**
   * @brief Feed mouse wheel event forcefully.
   *
   * @param[in] yDirection wheel event's y direction.
   * @param[in] step step of wheel event.
   * @param[in] x x value of wheel event.
   * @param[in] y y value of wheel event.
   */
  virtual void FeedMouseWheel(bool yDirection, int step, int x, int y) = 0;

  /**
   * @brief Enable video hole for a specific window type.
   * @param[in] enabled True if enabled, false othewise.
   * @param[in] isWaylandWindow True if wayland window, false if EFL window.
   */
  virtual void SetVideoHole(bool enabled, bool isWaylandWindow) = 0;
};

// specialization has to be done in the same namespace
template<>
struct EnableBitMaskOperators<WebEnginePlugin::FindOption>
{
  static const bool ENABLE = true;
};

} // namespace Dali

#endif
