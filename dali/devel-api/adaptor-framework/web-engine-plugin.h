#ifndef DALI_WEB_ENGINE_PLUGIN_H
#define DALI_WEB_ENGINE_PLUGIN_H

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
#include <dali/public-api/images/native-image-interface.h>
#include <dali/public-api/math/rect.h>
#include <dali/public-api/signals/dali-signal.h>

#include <functional>
#include <memory>

namespace Dali
{
class KeyEvent;
class PixelData;
class TouchEvent;
class WebEngineBackForwardList;
class WebEngineContext;
class WebEngineCookieManager;
class WebEnginePolicyDecision;
class WebEngineSettings;

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
  using WebEnginePageLoadErrorCallback = std::function<void(const std::string&, int)>;

  // forward declaration.
  enum class ScrollEdge;

  /**
   * @brief WebView callback related with scroll edge reached.
   */
  using WebEngineScrollEdgeReachedCallback = std::function<void(const ScrollEdge)>;

  /**
   * @brief The callback to be called when the web engine received a plain text of current web page.
   */
  using PlainTextReceivedCallback = std::function<void(const std::string&)>;

  /**
   * @brief Message result callback when JavaScript is executed with a message.
   */
  using JavaScriptMessageHandlerCallback = std::function<void(const std::string&)>;

  /**
   * @brief WebView callback related with page url changed.
   */
  using WebEngineUrlChangedCallback = std::function<void(const std::string&)>;

  /**
   * @brief WebView callback related with navigation policy would be decided.
   */
  using WebEngineNavigationPolicyDecidedCallback = std::function<void(std::unique_ptr<Dali::WebEnginePolicyDecision>)>;

  /**
   * @brief WebView callback related with a new window would be created.
   */
  using WebEngineNewWindowCreatedCallback = std::function<void(Dali::WebEnginePlugin*&)>;

  /**
   * @brief WebView callback related with frame would be rendered.
   */
  using WebEngineFrameRenderedCallback = std::function<void(void)>;

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
   * @brief Constructor.
   */
  WebEnginePlugin() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEnginePlugin() = default;

  /**
   * @brief Creates WebEngine instance.
   *
   * @param [in] width The width of Web
   * @param [in] height The height of Web
   * @param [in] locale The locale of Web
   * @param [in] timezoneId The timezoneID of Web
   */
  virtual void Create(int width, int height, const std::string& locale, const std::string& timezoneId) = 0;

  /**
   * @brief Creates WebEngine instance.
   *
   * @param [in] width The width of Web
   * @param [in] height The height of Web
   * @param [in] argc The count of application arguments
   * @param [in] argv The string array of application arguments
   */
  virtual void Create(int width, int height, int argc, char** argv) = 0;

  /**
   * @brief Destroys WebEngine instance.
   */
  virtual void Destroy() = 0;

  /**
   * @brief Get settings of WebEngine.
   */
  virtual WebEngineSettings& GetSettings() const = 0;

  /**
   * @brief Get context of WebEngine.
   */
  virtual WebEngineContext& GetContext() const = 0;

  /**
   * @brief Get cookie manager of WebEngine.
   */
  virtual WebEngineCookieManager& GetCookieManager() const = 0;

  /**
   * @brief Get back-forward list of WebEngine.
   */
  virtual WebEngineBackForwardList& GetBackForwardList() const = 0;

  /**
   * @brief Loads a web page based on a given URL.
   *
   * @param [in] url The URL of the resource to load
   */
  virtual void LoadUrl(const std::string& url) = 0;

  /**
   * @brief Returns the title of the Web.
   *
   * @return The title of web page
   */
  virtual std::string GetTitle() const = 0;

  /**
   * @brief Returns the Favicon of the Web.
   *
   * @return Favicon of Dali::PixelData& type
   */
  virtual Dali::PixelData GetFavicon() const = 0;

  /**
   * @brief Gets image to render.
   */
  virtual NativeImageInterfacePtr GetNativeImageSource() = 0;

  /**
   * @brief Returns the URL of the Web.
   *
   * @return Url of string type
   */
  virtual std::string GetUrl() const = 0;

  /**
   * @brief Loads a given string as web contents.
   *
   * @param [in] htmlString The string to use as the contents of the web page
   */
  virtual void LoadHtmlString(const std::string& htmlString) = 0;

  /**
   * @brief Reloads the Web.
   */
  virtual void Reload() = 0;

  /**
   * @brief Stops loading web contents on the current page.
   */
  virtual void StopLoading() = 0;

  /**
   * @brief Suspends the operation associated with the view.
   */
  virtual void Suspend() = 0;

  /**
   * @brief Resumes the operation associated with the view object after calling Suspend().
   */
  virtual void Resume() = 0;

  /**
   * @brief Scrolls the webpage of view by deltaX and deltaY.
   */
  virtual void ScrollBy(int deltaX, int deltaY) = 0;

  /**
   * @brief Scroll to the specified position of the given view.
   */
  virtual void SetScrollPosition(int x, int y) = 0;

  /**
   * @brief Gets the current scroll position of the given view.
   */
  virtual void GetScrollPosition(int& x, int& y) const = 0;

  /**
   * @brief Gets the possible scroll size of the given view.
   */
  virtual void GetScrollSize(int& width, int& height) const = 0;

  /**
   * @brief Gets the last known content's size.
   */
  virtual void GetContentSize(int& width, int& height) const = 0;

  /**
   * @brief Returns whether forward is possible.
   *
   * @return True if forward is possible, false otherwise
   */
  virtual bool CanGoForward() = 0;

  /**
   * @brief Goes to forward.
   */
  virtual void GoForward() = 0;

  /**
   * @brief Returns whether backward is possible.
   *
   * @return True if backward is possible, false otherwise
   */
  virtual bool CanGoBack() = 0;

  /**
   * @brief Goes to back.
   */
  virtual void GoBack() = 0;

  /**
   * @brief Evaluates JavaScript code represented as a string.
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
   * @brief Clears all tiles resources of Web.
   */
  virtual void ClearAllTilesResources() = 0;

  /**
   * @brief Clears the history of Web.
   */
  virtual void ClearHistory() = 0;

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
   * @brief Sets size of Web Page.
   */
  virtual void SetSize(int width, int height) = 0;

  /**
   * @brief Sends Touch Events.
   */
  virtual bool SendTouchEvent(const TouchEvent& touch) = 0;

  /**
   * @brief Sends Key Events.
   */
  virtual bool SendKeyEvent(const KeyEvent& event) = 0;

  /**
   * @brief Sets focus.
   */
  virtual void SetFocus(bool focused) = 0;

  /**
   * @brief Update display area.
   * @param[in] displayArea The display area need be updated.
   */
  virtual void UpdateDisplayArea(Dali::Rect<int> displayArea) = 0;

  /**
   * @brief Enable video hole.
   * @param[in] enabled True if enabled, false othewise.
   */
  virtual void EnableVideoHole(bool enabled) = 0;

  /**
   * @brief Callback to be called when page loading is started.
   *
   * @param[in] callback
   */
  virtual void RegisterPageLoadStartedCallback(WebEnginePageLoadCallback callback) = 0;

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
   * @brief Callback to be called when navigation policy would be decided.
   *
   * @param[in] callback
   */
  virtual void RegisterNavigationPolicyDecidedCallback(WebEngineNavigationPolicyDecidedCallback callback) = 0;

  /**
   * @brief Callback to be called when a new window would be created.
   *
   * @param[in] callback
   */
  virtual void RegisterNewWindowCreatedCallback(WebEngineNewWindowCreatedCallback callback) = 0;

  /**
   * @brief Callback to be called when frame would be rendered.
   *
   * @param[in] callback
   */
  virtual void RegisterFrameRenderedCallback(WebEngineFrameRenderedCallback callback) = 0;

  /**
   * @brief Get a plain text of current web page asynchronously.
   *
   * @param[in] callback The callback function called asynchronously.
   */
  virtual void GetPlainTextAsynchronously(PlainTextReceivedCallback callback) = 0;
};

} // namespace Dali

#endif
