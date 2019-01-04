#ifndef DALI_COLOR_CONTROLLER_PLUGIN_H
#define DALI_COLOR_CONTROLLER_PLUGIN_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <cstdlib>

namespace Dali
{

/**
 * @brief ColorControllerPlugin is an abstract interface, used by dali-adaptor to access Color Controller plugin.
 * A concrete implementation must be created for each platform and provided as dynamic library.
 */
class ColorControllerPlugin
{
public:

  /**
   * @brief Constructor.
   */
  ColorControllerPlugin(){}

  /**
   * @brief Destructor.
   */
  virtual ~ColorControllerPlugin(){}

  /**
   * @brief Retrieve the RGBA value by given the color code.
   *
   * @param[in] colorCode The color code string.
   * @param[out] colorValue The RGBA color
   * @return true if the color code exists, otherwise false
   */
  virtual bool RetrieveColor( const std::string& colorCode, Vector4& colorValue ) const = 0;

  /**
   * @brief Retrieve the RGBA value by given the color code.
   *
   * @param[in] colorCode The color code string.
   * @param[out] textColor The text color.
   * @param[out] textOutlineColor The text outline color.
   * @param[out] textShadowColor The text shadow color.
   * @return true if the color code exists, otherwise false
   */
  virtual bool RetrieveColor( const std::string& colorCode , Vector4& textColor, Vector4& textOutlineColor, Vector4& textShadowColor ) const = 0;
};

} // namespace Dali;

#endif // DALI_COLOR_CONTROLLER_PLUGIN_H
