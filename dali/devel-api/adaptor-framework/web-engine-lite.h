#ifndef __DALI_WEBENGINELITE_H__
#define __DALI_WEBENGINELITE_H__

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
#include <dali/public-api/object/base-handle.h>

//INTERNAL INCLUDES
#include "web-engine-lite-plugin.h"

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
  class WebEngineLite;
} // namespace Adaptor

} // namespace Internal

/**
 * @brief WebEngineLite class is used for Web.
 */
class DALI_IMPORT_API WebEngineLite: public BaseHandle
{
public:

  /**
   * @brief Constructor.
   */
  WebEngineLite();

  /**
   * @brief Destructor.
   */
  ~WebEngineLite();

  /**
   * @brief Creates a new instance of a WebEngineLite.
   */
  static WebEngineLite New();

 /**
   * @brief Copy constructor.
   *
   * @param[in] webEngineLite WebEngineLite to copy. The copied webEngineLite will point at the same implementation
   */
  WebEngineLite( const WebEngineLite& webEngineLite );

 /**
   * @brief Assignment operator.
   *
   * @param[in] webEngineLite The WebEngineLite to assign from.
   * @return The updated WebEngineLite.
   */
  WebEngineLite& operator=( const WebEngineLite& webEngineLite );

  /**
   * @brief Downcast a handle to WebEngineLite handle.
   *
   * If handle points to a WebEngineLite the downcast produces valid
   * handle. If not the returned handle is left uninitialized.
   *
   * @param[in] handle Handle to an object
   * @return Handle to a WebEngineLite or an uninitialized handle
   */
  static WebEngineLite DownCast( BaseHandle handle );

  /**
   * @brief Creates WebEngineLite instance.
   *
   * @param [in] width The width of Web
   * @param [in] height The height of Web
   * @param [in] windowX The x position of window
   * @param [in] windowY The y position of window
   * @param [in] locale The locale of Web
   * @param [in] timezoneID The timezoneID of Web
   */
  void CreateInstance(int width, int height, int windowX, int windowY, const std::string& locale, const std::string& timezoneID);

  /**
   * @brief Destroys WebEngineLite instance.
   */
  void DestroyInstance();

  /**
   * @brief Loads a html.
   *
   * @param [in] path The path of Web
   */
  void LoadHtml(const std::string& path);

  /**
   * @brief Connect to this signal to be notified when a web view has finished.
   *
   * @return A signal object to connect with.
   */
  Dali::WebEngineLitePlugin::WebEngineLiteSignalType& FinishedSignal();

private: // Not intended for application developers

  /**
   * @brief Internal constructor
   */
  explicit DALI_INTERNAL WebEngineLite( Internal::Adaptor::WebEngineLite* internal );
};

} // namespace Dali;

#endif

