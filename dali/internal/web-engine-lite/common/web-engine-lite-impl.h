#ifndef __DALI_WEBENGINELITE_IMPL_H__
#define __DALI_WEBENGINELITE_IMPL_H__

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/web-engine-lite.h>
#include <dali/devel-api/adaptor-framework/web-engine-lite-plugin.h>

namespace Dali
{
class Any;

namespace Internal
{

namespace Adaptor
{

class WebEngineLite;

typedef IntrusivePtr< WebEngineLite > WebEngineLitePtr;

/**
 * @brief WebEngineLite class is used for Web.
 */
class WebEngineLite: public Dali::BaseObject
{
public:

  /**
   * @brief Creates a new WebEngineLite handle
   *
   * @return WebEngineLite pointer
   */
  static WebEngineLitePtr New();

  /**
   * @copydoc Dali::WebEngineLite::CreateInstance()
   */
  void CreateInstance(int width, int height, int windowX, int windowY, const std::string& locale, const std::string& timezoneID);

  /**
   * @copydoc Dali::WebEngineLite::DestroyInstance()
   */
  void DestroyInstance();

  /**
   * @copydoc Dali::WebEngineLite::LoadHtml()
   */
  void LoadHtml(const std::string& path);

  /**
   * @copydoc Dali::WebEngineLite::FinishedSignal()
   */
  Dali::WebEngineLitePlugin::WebEngineLiteSignalType& FinishedSignal();

  /**
   * @brief Initializes member data.
   */
  void Initialize();

private:

  /**
   * @brief Constructor.
   */
  WebEngineLite();

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineLite();

  // Undefined copy constructor
  WebEngineLite( const WebEngineLite& webEngineLite );

  // Undefined assignment operator
  WebEngineLite& operator=( const WebEngineLite& webEngineLite );

private:

  Dali::WebEngineLitePlugin* mPlugin; ///< WebEngineLite plugin handle
  void* mHandle; ///< Handle for the loaded library

  typedef Dali::WebEngineLitePlugin* (*CreateWebEngineLiteFunction)();
  typedef void (*DestroyWebEngineLiteFunction)( Dali::WebEngineLitePlugin* plugin );

  CreateWebEngineLiteFunction mCreateWebEngineLitePtr;
  DestroyWebEngineLiteFunction mDestroyWebEngineLitePtr;

  Dali::WebEngineLitePlugin::WebEngineLiteSignalType mFinishedSignal;
};

} // namespace Adaptor
} // namespace Internal

inline static Internal::Adaptor::WebEngineLite& GetImplementation( Dali::WebEngineLite& webEngineLite )
{
  DALI_ASSERT_ALWAYS( webEngineLite && "WebEngineLite handle is empty." );

  BaseObject& handle = webEngineLite.GetBaseObject();

  return static_cast< Internal::Adaptor::WebEngineLite& >( handle );
}

inline static const Internal::Adaptor::WebEngineLite& GetImplementation( const Dali::WebEngineLite& webEngineLite )
{
  DALI_ASSERT_ALWAYS( webEngineLite && "WebEngineLite handle is empty." );

  const BaseObject& handle = webEngineLite.GetBaseObject();

  return static_cast< const Internal::Adaptor::WebEngineLite& >( handle );
}

} // namespace Dali;

#endif

