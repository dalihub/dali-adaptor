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
   * @brief Reloads the Web.
   */
  void Reload();

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
   * @brief Scrolls the webpage of view by deltaX and deltaY.
   */
  void ScrollBy(int deltaX, int deltaY);

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
  void RegisterJavaScriptAlertCallback( Dali::WebEnginePlugin::JavaScriptAlertCallback callback);

  /**
   * @brief Reply for JavaScript alert.
   */
  void JavaScriptAlertReply();

  /**
   * @brief Register a callback for JavaScript confirm.
   *
   * @param[in] callback The callback function
   */
  void RegisterJavaScriptConfirmCallback( Dali::WebEnginePlugin::JavaScriptConfirmCallback callback);

  /**
   * @brief Reply for JavaScript confirm.
   */
  void JavaScriptConfirmReply( bool confirmed );

  /**
   * @brief Register a callback for JavaScript prompt.
   *
   * @param[in] callback The callback function
   */
  void RegisterJavaScriptPromptCallback( Dali::WebEnginePlugin::JavaScriptPromptCallback callback );

  /**
   * @brief Reply for JavaScript prompt.
   */
  void JavaScriptPromptReply( const std::string& result );

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
   * @brief Update display area.
   * @param[in] displayArea The area to display web page.
   */
  void UpdateDisplayArea(Dali::Rect<int> displayArea);

  /**
   * @brief Enable video hole.
   * @param[in] enabled True if video hole is enabled, false otherwise
   */
  void EnableVideoHole(bool enabled);

  /**
   * @brief Connects to this signal to be notified when page loading is started.
   *
   * @return A signal object to connect with.
   */
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType& PageLoadStartedSignal();

  /**
   * @brief Connects to this signal to be notified when page loading is in progress.
   *
   * @return A signal object to connect with.
   */
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType& PageLoadInProgressSignal();

  /**
   * @brief Connects to this signal to be notified when page loading is finished.
   *
   * @return A signal object to connect with.
   */
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType& PageLoadFinishedSignal();

  /**
   * @brief Connects to this signal to be notified when an error occurs in page loading.
   *
   * @return A signal object to connect with.
   */
  Dali::WebEnginePlugin::WebEnginePageLoadErrorSignalType& PageLoadErrorSignal();

  /**
   * @brief Connects to this signal to be notified when scroll edge is reached.
   *
   * @return A signal object to connect with.
   */
  Dali::WebEnginePlugin::WebEngineScrollEdgeReachedSignalType& ScrollEdgeReachedSignal();

  /**
   * @brief Connects to this signal to be notified when url is changed.
   *
   * @return A signal object to connect with.
   */
  Dali::WebEnginePlugin::WebEngineUrlChangedSignalType& UrlChangedSignal();

private: // Not intended for application developers
  /**
   * @brief Internal constructor
   */
  explicit DALI_INTERNAL WebEngine(Internal::Adaptor::WebEngine* internal);
};

} // namespace Dali

#endif // DALI_WEB_ENGINE_H
