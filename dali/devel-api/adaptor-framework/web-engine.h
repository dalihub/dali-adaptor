#ifndef DALI_WEB_ENGINE_H
#define DALI_WEB_ENGINE_H

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
  WebEngine( const WebEngine& WebEngine );

  /**
   * @brief Assignment operator.
   *
   * @param[in] WebEngine The WebEngine to assign from.
   * @return The updated WebEngine.
   */
  WebEngine& operator=( const WebEngine& WebEngine );

  /**
   * @brief Downcast a handle to WebEngine handle.
   *
   * If handle points to a WebEngine the downcast produces valid
   * handle. If not the returned handle is left uninitialized.
   *
   * @param[in] handle Handle to an object
   * @return Handle to a WebEngine or an uninitialized handle
   */
  static WebEngine DownCast( BaseHandle handle );

  /**
   * @brief Creates WebEngine instance.
   *
   * @param [in] width The width of Web
   * @param [in] height The height of Web
   * @param [in] locale The locale of Web
   * @param [in] timezoneId The timezoneID of Web
   */
  void Create( int width, int height, const std::string& locale, const std::string& timezoneId );

  /**
   * @brief Destroys WebEngine instance.
   */
  void Destroy();

  /**
   * @brief Gets native image source to render.
   */
  NativeImageInterfacePtr GetNativeImageSource();

  /**
   * @brief Loads a web page based on a given URL.
   *
   * @param [in] url The URL of the resource to load
   */
  void LoadUrl( const std::string& url );

  /**
   * @brief Gets the url.
   */
  const std::string& GetUrl();

  /**
   * @brief Loads a given string as web contents.
   *
   * @param [in] htmlString The string to use as the contents of the web page
   */
  void LoadHTMLString( const std::string& htmlString );

  /**
   * @brief Reloads the Web.
   */
  void Reload();

  /**
   * @brief Stops loading web contents on the current page.
   */
  void StopLoading();

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
    */
  void EvaluateJavaScript( const std::string& script );

  /**
    * @brief Adds a JavaScript interface.
    *
    * @param[in] exposedObjectName The name of exposed object
    * @param[in] jsFunctionName The name of JavaScript function
    * @param[in] cb The callback function
    */
  void AddJavaScriptInterface( const std::string& exposedObjectName, const std::string& jsFunctionName, std::function< std::string(const std::string&) > cb );

  /**
    * @brief Removes a JavaScript interface.
    *
    * @param[in] exposedObjectName The name of exposed object
    * @param[in] jsFunctionName The name of JavaScript function
    */
  void RemoveJavascriptInterface( const std::string& exposedObjectName, const std::string& jsFunctionName );

  /**
   * @brief Clears the history of Web.
   */
  void ClearHistory();

  /**
   * @brief Clears the cache of Web.
   */
  void ClearCache();

  /**
   * @brief Sets the size of Web Pages.
   */
  void SetSize( int width, int height );

  /**
   * @brief Sends Touch Events.
   */
  bool SendTouchEvent( const TouchData& touch );

  /**
   * @brief Sends key Events.
   */
  bool SendKeyEvent( const KeyEvent& event );

  /**
   * @brief Connects to this signal to be notified when page loading is started.
   *
   * @return A signal object to connect with.
   */
  Dali::WebEnginePlugin::WebEngineSignalType& PageLoadStartedSignal();

  /**
   * @brief Connects to this signal to be notified when page loading is finished.
   *
   * @return A signal object to connect with.
   */
  Dali::WebEnginePlugin::WebEngineSignalType& PageLoadFinishedSignal();

private: // Not intended for application developers

  /**
   * @brief Internal constructor
   */
  explicit DALI_INTERNAL WebEngine( Internal::Adaptor::WebEngine* internal );
};

} // namespace Dali;

#endif // DALI_WEB_ENGINE_H
