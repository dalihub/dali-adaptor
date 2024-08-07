#ifndef DALI_WINDOW_BLUR_INFO_H
#define DALI_WINDOW_BLUR_INFO_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/public-api/dali-adaptor-common.h>


namespace Dali
{
/**
 * The WindowBlurInfo class is used as a parameter to support window blur.
 * Window blur supports background blur and behind.
 * Background blur means the blur region is same window size.
 * Behind blur measns the blur works excepts window region.
 * The corner radius works only for background blur.
 *
 * @SINCE_2_3.38
 */
struct DALI_ADAPTOR_API WindowBlurInfo
{
public:
  /**
   * @brief Default constructor.
   * It is set with the defaults as none values.
   *
   * @SINCE_2_3.38
   */
  WindowBlurInfo()
  : windowBlurType(WindowBlurType::NONE),
    windowBlurRadius(0),
    backgroundBlurRadius(0)
  {
  }

  /**
   * @brief Conversion constructor with three input params.
   *
   * @SINCE_2_3.38
   * @param[in] type window blur type
   * @param[in] blurRadius window blur radius
   * @param[in] cornerRadius background window corner radius
   */
  explicit constexpr WindowBlurInfo(WindowBlurType type, int blurRadius, int cornerRadius)
  : windowBlurType(type),
    windowBlurRadius(blurRadius),
    backgroundBlurRadius(cornerRadius)
  {
  }

  /**
   * @brief Conversion constructor with two input params.
   * corner radius is disabled.
   *
   * @SINCE_2_3.38
   * @param[in] type window blur type
   * @param[in] blurRadius window blur radius value.
   */
  explicit constexpr WindowBlurInfo(WindowBlurType type, int blurRadius)
  : windowBlurType(type),
    windowBlurRadius(blurRadius),
    backgroundBlurRadius(0)
  {
  }

  /**
   * @brief Equality operator.
   *
   * Utilizes appropriate machine epsilon values.
   *
   * @SINCE_2_3.38
   * @param[in] blurInfo The Window blur info to test against
   * @return True if the infos are equal
   */
  bool operator==(const WindowBlurInfo& blurInfo) const;

  /**
   * @brief Sets the blur type
   *
   * @SINCE_2_3.38
   * @param[in] type window blur type
   */
  void SetBlurType(WindowBlurType type);

  /**
   * @brief Gets the blur type
   *
   * @SINCE_2_3.38
   * @return The window blur type
   */
  WindowBlurType GetBlurType() const;

  /**
   * @brief Sets the blur radius.
   * If the radius is 0, window blur is disabled.
   *
   * @SINCE_2_3.38
   * @param[in] blurRadius window blur radius value.
   */
  void SetBlurRadius(int blurRadius);

  /**
   * @brief Gets the blur radius
   *
   * @SINCE_2_3.38
   * @return The window blur radius value.
   */
  int GetBlurRadius() const;

  /**
   * @brief Sets the corner radius for background blur.
   * It can be only used when window background blur is set.
   *
   * @SINCE_2_3.38
   * @param[in] cornerRadius the corner radius value for window background blur.
   */
  void SetCornerRadiusForBackground(int cornerRadius);

  /**
   * @brief Gets the corner radius value for window background blur.
   *
   * @SINCE_2_3.38
   * @return the corner radius value for window background blur.
   */
  int GetCornerRadiusForBackground() const;

  WindowBlurInfo(const WindowBlurInfo&)     = default;            ///< Default copy constructor
  WindowBlurInfo(WindowBlurInfo&&) noexcept = default;            ///< Default move constructor
  WindowBlurInfo& operator=(const WindowBlurInfo&) = default;     ///< Default copy assignment operator
  WindowBlurInfo& operator=(WindowBlurInfo&&) noexcept = default; ///< Default move assignment operator

public:
  WindowBlurType windowBlurType;
  int windowBlurRadius;
  int backgroundBlurRadius;
};

} // namespace Dali

#endif // DALI_WINDOW_BLUR_INFO_H
