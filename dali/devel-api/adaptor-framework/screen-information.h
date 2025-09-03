#ifndef DALI_SCREEN_INFORMATION_H
#define DALI_SCREEN_INFORMATION_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <string>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
/**
 * @brief A struct that holds screen information.
 * It contains the screen's name, width, and height.
 *
 * @SINCE_2_4.35
 */
struct DALI_ADAPTOR_API ScreenInformation
{
public:
  /**
   * @brief Default constructor.
   * It is set with the defaults as none values.
   *
   * @SINCE_2_4.35
   */
  ScreenInformation()
  : screenWidth(0),
    screenHeight(0)
  {
  }

  /**
   * @brief Conversion constructor with three input params.
   *
   * @SINCE_2_4.35
   * @param[in] name screen name
   * @param[in] width screen width
   * @param[in] height screen height
   */
  explicit ScreenInformation(std::string name, int width, int height)
  : screenName(std::move(name)),
    screenWidth(width),
    screenHeight(height)
  {
  }

  /**
   * @brief Equality operator.
   *
   * @SINCE_2_4.35
   * @param[in] screenInfo The Screen information to test against
   * @return True if the infos are equal
   */
  bool operator==(const ScreenInformation& screenInfo) const;

  /**
   * @brief Gets the screen name
   *
   * @SINCE_2_4.35
   * @return The screen name
   */
  std::string GetScreenName() const;

  /**
   * @brief Gets the screen width
   *
   * @SINCE_2_4.35
   * @return The screen width value.
   */
  int GetScreenWidth() const;

  /**
   * @brief Gets the screen height
   *
   * @SINCE_2_4.35
   * @return The screen height value.
   */
  int GetScreenHeight() const;

  ScreenInformation(const ScreenInformation&)                = default; ///< Default copy constructor
  ScreenInformation(ScreenInformation&&) noexcept            = default; ///< Default move constructor
  ScreenInformation& operator=(const ScreenInformation&)     = default; ///< Default copy assignment operator
  ScreenInformation& operator=(ScreenInformation&&) noexcept = default; ///< Default move assignment operator

private:
  std::string screenName;   ///< screen name.
  int         screenWidth;  ///< screen width
  int         screenHeight; ///< screen height
};

} // namespace Dali

#endif // DALI_SCREEN_INFORMATION_H
