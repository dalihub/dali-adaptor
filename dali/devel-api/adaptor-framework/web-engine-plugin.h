#ifndef DALI_WEB_ENGINE_PLUGIN_H
#define DALI_WEB_ENGINE_PLUGIN_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/signals/dali-signal.h>
#include <functional>

namespace Dali
{

class KeyEvent;
class TouchData;

/**
 * @brief WebEnginePlugin is an abstract interface, used by dali-adaptor to access WebEngine plugin.
 * A concrete implementation must be created for each platform and provided as dynamic library.
 */
class WebEnginePlugin
{
public:

  /**
   * @brief WebEngine signal type related with page loading.
   */
  typedef Signal< void( const std::string& ) > WebEnginePageLoadSignalType;

  /**
   * @brief WebView signal type related with page loading error.
   */
  typedef Signal< void( const std::string&, int ) > WebEnginePageLoadErrorSignalType;

  /**
   * @brief Enumeration for cache model options.
   */
  enum class CacheModel
  {
    /**
     * @brief Use the smallest cache capacity.
     */
    DOCUMENT_VIEWER,

    /**
     * @brief Use the bigger cache capacity than DocumentBrowser.
     */
    DOCUMENT_BROWSER,

    /**
     * @brief Use the biggest cache capacity.
     */
    PRIMARY_WEB_BROWSER
  };

  /**
   * @brief Enumeration for the cookies accept policies.
   */
  enum class CookieAcceptPolicy
  {
    /**
     * @brief Accepts every cookie sent from any page.
     */
    ALWAYS,

    /**
     * @brief Rejects all the cookies.
     */
    NEVER,

    /**
     * @brief Accepts only cookies set by the main document that is loaded.
     */
    NO_THIRD_PARTY
  };

  /**
   * @brief Constructor.
   */
  WebEnginePlugin()
  {
  }

  /**
   * @brief Destructor.
   */
  virtual ~WebEnginePlugin()
  {
  }

  /**
   * @brief Creates WebEngine instance.
   *
   * @param [in] width The width of Web
   * @param [in] height The height of Web
   * @param [in] locale The locale of Web
   * @param [in] timezoneId The timezoneID of Web
   */
  virtual void Create( int width, int height, const std::string& locale, const std::string& timezoneId ) = 0;

  /**
   * @brief Destroys WebEngine instance.
   */
  virtual void Destroy() = 0;

  /**
   * @brief Loads a web page based on a given URL.
   *
   * @param [in] url The URL of the resource to load
   */
  virtual void LoadUrl( const std::string& url ) = 0;

  /**
   * @brief Gets image to render.
   */
  virtual NativeImageInterfacePtr GetNativeImageSource() = 0;

  /**
   * @brief Returns the URL of the Web.
   *
   * @return Url of string type
   */
  virtual const std::string& GetUrl() = 0;

  /**
   * @brief Loads a given string as web contents.
   *
   * @param [in] htmlString The string to use as the contents of the web page
   */
  virtual void LoadHTMLString( const std::string& htmlString ) = 0;

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
  virtual void EvaluateJavaScript( const std::string& script, std::function< void( const std::string& ) > resultHandler ) = 0;

  /**
   * @brief Add a message handler into JavaScript.
   *
   * @param[in] exposedObjectName The name of exposed object
   * @param[in] handler The callback function
   */
  virtual void AddJavaScriptMessageHandler( const std::string& exposedObjectName, std::function< void( const std::string& ) > handler ) = 0;

  /**
   * @brief Clears the history of Web.
   */
  virtual void ClearHistory() = 0;

  /**
   * @brief Clears the cache of Web.
   */
  virtual void ClearCache() = 0;

  /**
   * @brief Clears all the cookies of Web.
   */
  virtual void ClearCookies() = 0;

  /**
   * @brief Get cache model option. The default is DOCUMENT_VIEWER.
   *
   * @return The cache model option
   */
  virtual CacheModel GetCacheModel() const = 0;

  /**
   * @brief Set cache model option. The default is DOCUMENT_VIEWER.
   *
   * @param[in] cacheModel The cache model option
   */
  virtual void SetCacheModel( CacheModel cacheModel ) = 0;

  /**
   * @brief Gets the cookie acceptance policy. The default is NO_THIRD_PARTY.
   *
   * @return The cookie acceptance policy
   */
  virtual CookieAcceptPolicy GetCookieAcceptPolicy() const = 0;

  /**
   * @brief Sets the cookie acceptance policy. The default is NO_THIRD_PARTY.
   *
   * @param[in] policy The cookie acceptance policy
   */
  virtual void SetCookieAcceptPolicy( CookieAcceptPolicy policy ) = 0;

  /**
   * @brief Get user agent string.
   *
   * @return The string value of user agent
   */
  virtual const std::string& GetUserAgent() const = 0;

  /**
   * @brief Set user agent string.
   *
   * @param[in] userAgent The string value of user agent
   */
  virtual void SetUserAgent( const std::string& userAgent ) = 0;

  /**
   * @brief Returns whether JavaScript can be executable. The default is true.
   *
   * @return true if JavaScript executing is enabled, false otherwise
   */
  virtual bool IsJavaScriptEnabled() const = 0;

  /**
   * @brief Enables/disables JavaScript executing. The default is enabled.
   *
   * @param[in] enabled True if JavaScript executing is enabled, false otherwise
   */
  virtual void EnableJavaScript( bool enabled ) = 0;

  /**
   * @brief Returns whether images can be loaded automatically. The default is true.
   *
   * @return true if images are loaded automatically, false otherwise
   */
  virtual bool AreImagesAutomaticallyLoaded() const = 0;

  /**
   * @brief Enables/disables auto loading of images. The default is enabled.
   *
   * @param[in] automatic True if images are loaded automatically, false otherwise
   */
  virtual void LoadImagesAutomatically( bool automatic ) = 0;

  /**
   * @brief Gets the default text encoding name (e.g. UTF-8).
   *
   * @return The default text encoding name
   */
  virtual const std::string& GetDefaultTextEncodingName() const = 0;

  /**
   * @brief Sets the default text encoding name (e.g. UTF-8).
   *
   * @param[in] defaultTextEncodingName The default text encoding name
   */
  virtual void SetDefaultTextEncodingName( const std::string& defaultTextEncodingName ) = 0;

  /**
   * @brief Returns the default font size in pixel. The default value is 16.
   *
   * @return The default font size
   */
  virtual int GetDefaultFontSize() const = 0;

  /**
   * @brief Sets the default font size in pixel. The default value is 16.
   *
   * @param[in] defaultFontSize A new default font size to set
   */
  virtual void SetDefaultFontSize( int defaultFontSize ) = 0;

  /**
   * @brief Sets size of Web Page.
   */
  virtual void SetSize( int width, int height ) = 0;

  /**
   * @brief Sends Touch Events.
   */
  virtual bool SendTouchEvent( const TouchData& touch ) = 0;

  /**
   * @brief Sends Key Events.
   */
  virtual bool SendKeyEvent( const KeyEvent& event ) = 0;

  /**
   * @brief Connects to this signal to be notified when page loading is started.
   *
   * @return A signal object to connect with.
   */
  virtual WebEnginePageLoadSignalType& PageLoadStartedSignal() = 0;

  /**
   * @brief Connects to this signal to be notified when page loading is finished.
   *
   * @return A signal object to connect with.
   */
  virtual WebEnginePageLoadSignalType& PageLoadFinishedSignal() = 0;

  /**
   * @brief Connects to this signal to be notified when an error occurs in page loading.
   *
   * @return A signal object to connect with.
   */
  virtual WebEnginePageLoadErrorSignalType& PageLoadErrorSignal() = 0;

};

} // namespace Dali;

#endif
