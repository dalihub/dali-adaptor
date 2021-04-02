#ifndef DALI_WEB_ENGINE_H
#define DALI_WEB_ENGINE_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/object/base-handle.h>

//INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/web-engine-plugin.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
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
   * @brief Creates a new instance of a WebEngine.
   */
  static WebEngine New();

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
   * @brief Creates WebEngine instance.
   *
   * @param [in] width The width of Web
   * @param [in] height The height of Web
   * @param [in] locale The locale of Web
   * @param [in] timezoneId The timezoneID of Web
   */
  void Create(int width, int height, const std::string& locale, const std::string& timezoneId);

  /**
   * @brief Creates WebEngine instance.
   *
   * @param [in] width The width of Web
   * @param [in] height The height of Web
   * @param [in] argc The count of application arguments
   * @param [in] argv The string array of application arguments
   */
  void Create(int width, int height, int argc, char** argv);

  /**
   * @brief Destroys WebEngine instance.
   */
  void Destroy();

  /**
   * @brief Gets native image source to render.
   */
  NativeImageInterfacePtr GetNativeImageSource();

  /**
   * @brief Get settings of WebEngine.
   */
  Dali::WebEngineSettings& GetSettings() const;

  /**
   * @brief Get context of WebEngine.
   */
  Dali::WebEngineContext& GetContext() const;

  /**
   * @brief Get cookie manager of WebEngine.
   */
  Dali::WebEngineCookieManager& GetCookieManager() const;

  /**
   * @brief Get back-forward list of WebEngine.
   */
  Dali::WebEngineBackForwardList& GetBackForwardList() const;

  /**
   * @brief Loads a web page based on a given URL.
   *
   * @param [in] url The URL of the resource to load
   */
  void LoadUrl(const std::string& url);

  /**
   * @brief Returns the title of the Web.
   *
   * @return The title of web page
   */
  std::string GetTitle() const;

  /**
   * @brief Returns the Favicon of the Web.
   *
   * @return FavIcon of Dali::PixelData& type
   */
  Dali::PixelData GetFavicon() const;

  /**
   * @brief Gets the url.
   */
  const std::string& GetUrl();

  /**
   * @brief Loads a given string as web contents.
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
   * @brief Requests loading the given contents by MIME type into the view object
   *
   * @param[in] contents The content to load
   * @param[in] contentSize The size of contents (in bytes)
   * @param[in] mimeType The type of contents, if 0 is given "text/html" is assumed
   * @param[in] encoding The encoding for contents, if 0 is given "UTF-8" is assumed
   * @param[in] baseUri The base URI to use for relative resources
   *
   * @return true if successfully request, false otherwise
   */
  bool LoadContents(const std::string& contents, uint32_t contentSize, const std::string& mimeType, const std::string& encoding, const std::string& baseUri);

  /**
   * @brief Reloads the Web.
   */
  void Reload();

  /**
   * @brief Reloads the current page's document without cache
   */
  bool ReloadWithoutCache();

  /**
   * @brief Stops loading web contents on the current page.
   */
  void StopLoading();

  /**
   * @brief Suspends the operation associated with the view.
   */
  void Suspend();

  /**
   * @brief Resumes the operation associated with the view object after calling Suspend().
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
   * @brief Scrolls web page of view by deltaX and deltaY.
   *
   * @param[in] deltaX horizontal offset to scroll
   * @param[in] deltaY vertical offset to scroll
   */
  void ScrollBy(int deltaX, int deltaY);

  /**
   * @brief Scrolls edge of view by deltaX and deltaY.
   *
   * @param[in] deltaX horizontal offset to scroll
   * @param[in] deltaY vertical offset to scroll
   *
   * @return true if succeeded, false otherwise
   */
  bool ScrollEdgeBy(int deltaX, int deltaY);

  /**
   * @brief Sets an absolute scroll of the given view.
   */
  void SetScrollPosition(int x, int y);

  /**
   * @brief Gets the current scroll position of the given view.
   */
  Dali::Vector2 GetScrollPosition() const;

  /**
   * @brief Gets the possible scroll size of the given view.
   */
  Dali::Vector2 GetScrollSize() const;

  /**
   * @brief Gets the last known content's size.
   */
  Dali::Vector2 GetContentSize() const;

  /**
   * @brief Returns whether forward is possible.
   *
   * @return True if forward is possible, false otherwise
   */
  bool CanGoForward();

  /**
   * @brief Goes to forward.
   */
  void GoForward();

  /**
   * @brief Returns whether backward is possible.
   *
   * @return True if backward is possible, false otherwise
   */
  bool CanGoBack();

  /**
   * @brief Goes to back.
   */
  void GoBack();

  /**
   * @brief Evaluates JavaScript code represented as a string.
   *
   * @param[in] script The JavaScript code
   * @param[in] resultHandler The callback function to be called by the JavaScript runtime. This carries evaluation result.
   */
  void EvaluateJavaScript(const std::string& script, std::function<void(const std::string&)> resultHandler);

  /**
   * @brief Add a message handler into JavaScript.
   *
   * @param[in] exposedObjectName The name of exposed object
   * @param[in] handler The callback function
   */
  void AddJavaScriptMessageHandler(const std::string& exposedObjectName, std::function<void(const std::string&)> handler);

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
   * @brief Clears the history of Web.
   */
  void ClearHistory();

  /**
   * @brief Clears all tiles resources of Web.
   */
  void ClearAllTilesResources();

  /**
   * @brief Get user agent string.
   *
   * @return The string value of user agent
   */
  const std::string& GetUserAgent() const;

  /**
   * @brief Set user agent string.
   *
   * @param[in] userAgent The string value of user agent
   */
  void SetUserAgent(const std::string& userAgent);

  /**
   * @brief Sets the size of Web Pages.
   */
  void SetSize(int width, int height);

  /**
   * @brief Sets background color of web page.
   *
   * @param[in] color Background color
   */
  void SetDocumentBackgroundColor(Dali::Vector4 color);

  /**
   * @brief Clears tiles when hidden.
   *
   * @param[in] cleared Whether tiles are cleared or not
   */
  void ClearTilesWhenHidden(bool cleared);

  /**
   * @brief Sets multiplier of cover area of tile.
   *
   * @param[in] multiplier The multiplier of cover area
   */
  void SetTileCoverAreaMultiplier(float multiplier);

  /**
   * @brief Enables cursor by client.
   *
   * @param[in] enabled Whether cursor is enabled or not
   */
  void EnableCursorByClient(bool enabled);

  /**
   * @brief Gets the selected text.
   *
   * @return the selected text
   */
  std::string GetSelectedText() const;

  /**
   * @brief Sends Touch Events.
   */
  bool SendTouchEvent(const TouchEvent& touch);

  /**
   * @brief Sends key Events.
   */
  bool SendKeyEvent(const KeyEvent& event);

  /**
   * @brief Set focus.
   * @param[in] focused True if web view is focused, false otherwise
   */
  void SetFocus(bool focused);

  /**
   * @brief Enables/disables mouse events. The default is enabled.
   *
   * @param[in] enabled True if mouse events are enabled, false otherwise
   */
  void EnableMouseEvents( bool enabled );

  /**
   * @brief Enables/disables key events. The default is enabled.
   *
   * @param[in] enabled True if key events are enabled, false otherwise
   */
  void EnableKeyEvents( bool enabled );

  /**
   * @brief Sets zoom factor of the current page.
   * @param[in] zoomFactor a new factor to be set.
   */
  void SetPageZoomFactor(float zoomFactor);

  /**
   * @brief Queries the current zoom factor of the page。
   * @return The current page zoom factor.
   */
  float GetPageZoomFactor() const;

  /**
   * @brief Sets the current text zoom level。.
   * @param[in] zoomFactor a new factor to be set.
   */
  void SetTextZoomFactor(float zoomFactor);

  /**
   * @brief Gets the current text zoom level.
   * @return The current text zoom factor.
   */
  float GetTextZoomFactor() const;

  /**
   * @brief Gets the current load progress of the page.
   * @return The load progress of the page.
   */
  float GetLoadProgressPercentage() const;

  /**
   * @brief Scales the current page, centered at the given point.
   * @param[in] scaleFactor a new factor to be scaled.
   * @param[in] point a center coordinate.
   */
  void SetScaleFactor(float scaleFactor, Dali::Vector2 point);

  /**
   * @brief Gets the current scale factor of the page.
   * @return The current scale factor.
   */
  float GetScaleFactor() const;

  /**
   * @brief Request to activate/deactivate the accessibility usage set by web app.
   * @param[in] activated Activate accessibility or not.
   */
  void ActivateAccessibility(bool activated);

  /**
   * @brief Request to set the current page's visibility.
   * @param[in] visible Visible or not.
   *
   * @return true if changed successfully, false otherwise
   */
  bool SetVisibility(bool visible);

  /**
   * @brief Searches and highlights the given string in the document.
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
  Dali::PixelData GetScreenshot(Dali::Rect<int> viewArea, float scaleFactor);

  /**
   * @brief Request to get snapshot of the specified viewArea of page asynchronously.
   *
   * @param[in] viewArea The rectangle of screen shot
   * @param[in] scaleFactor The scale factor
   * @param[in] callback The callback for screen shot
   *
   * @return true if requested successfully, false otherwise
   */
  bool GetScreenshotAsynchronously(Dali::Rect<int> viewArea, float scaleFactor, Dali::WebEnginePlugin::ScreenshotCapturedCallback callback);

  /**
   * @brief Asynchronous request to check if there is a video playing in the given view.
   *
   * @param[in] callback The callback called after checking if video is playing or not
   *
   * @return true if requested successfully, false otherwise
   */
  bool CheckVideoPlayingAsynchronously(Dali::WebEnginePlugin::VideoPlayingCallback callback);

  /**
   * @brief Sets callback which alled upon geolocation permission request.
   *
   * @param[in] callback The callback for requesting geolocation permission
   */
  void RegisterGeolocationPermissionCallback(Dali::WebEnginePlugin::GeolocationPermissionCallback callback);

  /**
   * @brief Update display area.
   * @param[in] displayArea The area to display web page
   */
  void UpdateDisplayArea(Dali::Rect<int> displayArea);

  /**
   * @brief Enable video hole.
   * @param[in] enabled True if video hole is enabled, false otherwise
   */
  void EnableVideoHole(bool enabled);

  /**
   * @brief Sends hover events.
   * @param[in] event The hover event would be sent.
   */
  bool SendHoverEvent( const HoverEvent& event );

  /**
   * @brief Sends wheel events.
   * @param[in] event The wheel event would be sent.
   */
  bool SendWheelEvent( const WheelEvent& event );

  /**
   * @brief Connects to this signal to be notified when page loading is started.
   *
   * @return A signal object to connect with
   */
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType& PageLoadStartedSignal();

  /**
   * @brief Connects to this signal to be notified when page loading is in progress.
   *
   * @return A signal object to connect with
   */
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType& PageLoadInProgressSignal();

  /**
   * @brief Connects to this signal to be notified when page loading is finished.
   *
   * @return A signal object to connect with
   */
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType& PageLoadFinishedSignal();

  /**
   * @brief Connects to this signal to be notified when an error occurs in page loading.
   *
   * @return A signal object to connect with
   */
  Dali::WebEnginePlugin::WebEnginePageLoadErrorSignalType& PageLoadErrorSignal();

  /**
   * @brief Connects to this signal to be notified when scroll edge is reached.
   *
   * @return A signal object to connect with
   */
  Dali::WebEnginePlugin::WebEngineScrollEdgeReachedSignalType& ScrollEdgeReachedSignal();

  /**
   * @brief Connects to this signal to be notified when url is changed.
   *
   * @return A signal object to connect with
   */
  Dali::WebEnginePlugin::WebEngineUrlChangedSignalType& UrlChangedSignal();

  /**
   * @brief Connects to this signal to be notified when form repost decision is requested.
   *
   * @return A signal object to connect with.
   */
  Dali::WebEnginePlugin::WebEngineFormRepostDecisionSignalType& FormRepostDecisionSignal();

  /**
   * @brief Connects to this signal to be notified when frame is rendered.
   *
   * @return A signal object to connect with.
   */
  Dali::WebEnginePlugin::WebEngineFrameRenderedSignalType& FrameRenderedSignal();

  /**
   * @brief Connects to this signal to be notified when http request need be intercepted.
   *
   * @return A signal object to connect with.
   */
  Dali::WebEnginePlugin::WebEngineRequestInterceptorSignalType& RequestInterceptorSignal();

private: // Not intended for application developers
  /**
   * @brief Internal constructor
   */
  explicit DALI_INTERNAL WebEngine(Internal::Adaptor::WebEngine* internal);
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_H
