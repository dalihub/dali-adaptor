#ifndef DALI_WEB_ENGINE_IMPL_H
#define DALI_WEB_ENGINE_IMPL_H

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
#include <dali/public-api/object/base-object.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/web-engine.h>
#include <dali/devel-api/adaptor-framework/web-engine-plugin.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

class WebEngine;

typedef IntrusivePtr< WebEngine > WebEnginePtr;

/**
 * @brief WebEngine class is used for Web.
 */
class WebEngine : public Dali::BaseObject
{
public:

  /**
   * @brief Creates a new WebEngine handle
   *
   * @return WebEngine pointer
   */
  static WebEnginePtr New();

  /**
   * @copydoc Dali::WebEngine::Create()
   */
  void Create( int width, int height, const std::string& locale, const std::string& timezoneId );

  /**
   * @copydoc Dali::WebEngine::Destroy()
   */
  void Destroy();

  /**
   * @copydoc Dali::WebEngine::GetNativeImageSource()
   */
  Dali::NativeImageInterfacePtr GetNativeImageSource();

  /**
   * @copydoc Dali::WebEngine::LoadUrl()
   */
  void LoadUrl( const std::string& url );

  /**
   * @copydoc Dali::WebEngine::GetUrl()
   */
  const std::string& GetUrl();

  /**
   * @copydoc Dali::WebEngine::LoadHTMLString()
   */
  void LoadHTMLString( const std::string& htmlString );

  /**
   * @copydoc Dali::WebEngine::Reload()
   */
  void Reload();

  /**
   * @copydoc Dali::WebEngine::StopLoading()
   */
  void StopLoading();

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
  void EvaluateJavaScript( const std::string& script );

  /**
   * @copydoc Dali::WebEngine::AddJavaScriptInterface()
   */
  void AddJavaScriptInterface( const std::string& exposedObjectName, const std::string& jsFunctionName, std::function< std::string(const std::string&) > cb );

  /**
   * @copydoc Dali::WebEngine::RemoveJavascriptInterface()
   */
  void RemoveJavascriptInterface( const std::string& exposedObjectName, const std::string& jsFunctionName );

  /**
   * @copydoc Dali::WebEngine::ClearHistory()
   */
  void ClearHistory();

  /**
   * @copydoc Dali::WebEngine::ClearCache()
   */
  void ClearCache();

  /**
   * @copydoc Dali::WebEngine::SetSize()
   */
  void SetSize( int width, int height );

  /**
   * @copydoc Dali::WebEngine::SendTouchEvent()
   */
  bool SendTouchEvent( const Dali::TouchData& touch );

  /**
   * @copydoc Dali::WebEngine::SendKeyEvent()
   */
  bool SendKeyEvent( const Dali::KeyEvent& event );

  /**
   * @copydoc Dali::WebEngine::PageLoadStartedSignal()
   */
  Dali::WebEnginePlugin::WebEngineSignalType& PageLoadStartedSignal();

  /**
   * @copydoc Dali::WebEngine::PageLoadFinishedSignal()
   */
  Dali::WebEnginePlugin::WebEngineSignalType& PageLoadFinishedSignal();

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
  WebEngine( const WebEngine& WebEngine );

  // Undefined assignment operator
  WebEngine& operator=( const WebEngine& WebEngine );

  /**
   * @brief Initializes member data.
   *
   * @return Whether the initialization succeed or not.
   */
  bool Initialize();

private:

  Dali::WebEnginePlugin* mPlugin; ///< WebEngine plugin handle
  void* mHandle; ///< Handle for the loaded library

  typedef Dali::WebEnginePlugin* (*CreateWebEngineFunction)();
  typedef void (*DestroyWebEngineFunction)( Dali::WebEnginePlugin* plugin );

  CreateWebEngineFunction mCreateWebEnginePtr;
  DestroyWebEngineFunction mDestroyWebEnginePtr;
};

} // namespace Adaptor
} // namespace Internal

inline static Internal::Adaptor::WebEngine& GetImplementation( Dali::WebEngine& webEngine )
{
  DALI_ASSERT_ALWAYS( webEngine && "WebEngine handle is empty." );

  BaseObject& handle = webEngine.GetBaseObject();

  return static_cast< Internal::Adaptor::WebEngine& >( handle );
}

inline static const Internal::Adaptor::WebEngine& GetImplementation( const Dali::WebEngine& webEngine )
{
  DALI_ASSERT_ALWAYS( webEngine && "WebEngine handle is empty." );

  const BaseObject& handle = webEngine.GetBaseObject();

  return static_cast< const Internal::Adaptor::WebEngine& >( handle );
}

} // namespace Dali;

#endif

