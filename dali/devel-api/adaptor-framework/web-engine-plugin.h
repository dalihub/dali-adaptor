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

  typedef Signal< void( const std::string& ) > WebEngineSignalType;

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
    */
  virtual void EvaluateJavaScript( const std::string& script ) = 0;

  /**
    * @brief Adds a JavaScript interface.
    *
    * @param[in] exposedObjectName The name of exposed object
    * @param[in] jsFunctionName The name of JavaScript function
    * @param[in] cb The callback function
    */
  virtual void AddJavaScriptInterface( const std::string& exposedObjectName, const std::string& jsFunctionName, std::function< std::string(const std::string&) > cb ) = 0;

  /**
    * @brief Removes a JavaScript interface.
    *
    * @param[in] exposedObjectName The name of exposed object
    * @param[in] jsFunctionName The name of JavaScript function
    */
  virtual void RemoveJavascriptInterface( const std::string& exposedObjectName, const std::string& jsFunctionName ) = 0;

  /**
   * @brief Clears the history of Web.
   */
  virtual void ClearHistory() = 0;

  /**
   * @brief Clears the cache of Web.
   */
  virtual void ClearCache() = 0;

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
  virtual WebEngineSignalType& PageLoadStartedSignal() = 0;

  /**
   * @brief Connects to this signal to be notified when page loading is finished.
   *
   * @return A signal object to connect with.
   */
  virtual WebEngineSignalType& PageLoadFinishedSignal() = 0;

};

} // namespace Dali;

#endif
