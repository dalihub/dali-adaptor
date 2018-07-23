#ifndef __DALI_WEBENGINELITE_PLUGIN_H__
#define __DALI_WEBENGINELITE_PLUGIN_H__

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
#include <dali/public-api/signals/dali-signal.h>

namespace Dali
{

/**
 * @brief WebEngineLitePlugin is an abstract interface, used by dali-adaptor to access WebEngineLite plugin.
 * A concrete implementation must be created for each platform and provided as dynamic library.
 */
class WebEngineLitePlugin
{
public:

  typedef Signal< void () > WebEngineLiteSignalType;

  /**
   * @brief Constructor.
   */
  WebEngineLitePlugin(){}

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineLitePlugin(){}

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
  virtual void CreateInstance(int width, int height, int windowX, int windowY, const std::string& locale, const std::string& timezoneID) = 0;

  /**
   * @brief Destroys WebEngineLite instance.
   */
  virtual void DestroyInstance() = 0;

  /**
   * @brief Loads a html.
   *
   * @param [in] path The path of Web
   */
  virtual void LoadHtml(const std::string& path) = 0;

  /**
   * @brief Connect to this signal to be notified when a web view has finished.
   *
   * @return A signal object to connect with.
   */
  virtual WebEngineLiteSignalType& FinishedSignal() = 0;

};

} // namespace Dali;

#endif
