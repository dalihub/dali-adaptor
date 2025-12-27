#ifndef DALI_WINDOW_BLUR_INFO_H
#define DALI_WINDOW_BLUR_INFO_H

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

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/window-enumerations.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/math/vector4.h>

namespace Dali
{

/**
 * @brief The WindowDimInfo structure contains information for dimming effect applied for window or related region.
 *
 * This structure is used to configure dimming effects when WindowBlurType::BEHIND is applied.
 * Behind blur affects the area behind the window (excluding the window region itself), and this
 * structure allows additional dimming to be applied to that blurred area for better visual contrast
 * and user experience.
 *
 * The dimming effect uses a color overlay with alpha transparency to darken the blurred background.
 * This is particularly useful when the blurred background content is too bright or distracting,
 * making it difficult to see the foreground window content clearly.
 *
 * @SINCE_2_5.5
 */
struct DALI_ADAPTOR_API WindowDimInfo
{
public:
  /**
   * @brief Constructor with enable flag and dim color.
   *
   * @SINCE_2_5.5
   * @param[in] enable True to enable dimming effect, false to disable
   * @param[in] dim The dimming color (RGBA) to apply. The alpha component controls the dimming intensity.
   */
  explicit constexpr WindowDimInfo(bool enable, Vector4 dim)
: isEnabled(enable),
  dimColor(dim)
{
}
  /**
   * @brief Equality comparison operator.
   *
   * @SINCE_2_5.5
   * @param[in] dimInfo The WindowDimInfo to compare against
   * @return True if both objects have the same enable state and dim color
   */
  bool operator==(const WindowDimInfo& dimInfo) const
  {
    if(isEnabled != dimInfo.isEnabled)
    {
      return false;
    }
    if(dimColor != dimInfo.dimColor)
    {
      return false;
    }
    return true;
  }

  WindowDimInfo(const WindowDimInfo&)                = default; ///< Default copy constructor
  WindowDimInfo(WindowDimInfo&&) noexcept            = default; ///< Default move constructor
  WindowDimInfo& operator=(const WindowDimInfo&)     = default; ///< Default copy assignment operator
  WindowDimInfo& operator=(WindowDimInfo&&) noexcept = default; ///< Default move assignment operator

public:
  /**
   * @brief Flag to enable or disable the dimming effect.
   * This is used together with the blur effect.
   *
   * When true, the dimming effect is applied to the related region.
   * When false, no dimming is applied and only the blur effect is visible.
   *
   * Default value: false
   *
   * @SINCE_2_5.5
   */
  bool isEnabled;

  /**
   * @brief The dimming color to apply to the related region.
   *
   * This is an RGBA color value where:
   * - RGB components define the dimming color (typically black for darkening)
   * - Alpha component controls the dimming intensity (0.0 = transparent, 1.0 = fully opaque)
   *
   * Common usage examples:
   * - Vector4(0.0f, 0.0f, 0.0f, 0.3f) for subtle darkening
   * - Vector4(0.0f, 0.0f, 0.0f, 0.7f) for strong darkening
   * - Vector4(1.0f, 0.0f, 0.0f, 0.2f) for reddish tint
   *
   * Default value: Vector4(0.0f, 0.0f, 0.0f, 0.0f) (fully transparent)
   *
   * @SINCE_2_5.5
   */
  Vector4 dimColor;
};

/**
 * @brief The WindowBlurInfo class contains configuration information for window blur effects.
 *
 * This class provides parameters to configure different types of window blur effects:
 * - Background blur: Applies blur effect to the entire window region with the same size as the window
 * - Behind blur: Applies blur effect to the area behind the window, excluding the window region itself
 *
 * The corner radius feature is only applicable to background blur effects, allowing for
 * rounded corners in the blurred window area.
 *
 * @SINCE_2_3.38
 */
struct DALI_ADAPTOR_API WindowBlurInfo
{
public:
  /**
   * @brief Default constructor.
   * Initializes all values to their defaults (no blur effect).
   *
   * @SINCE_2_3.38
   */
  WindowBlurInfo()
  : windowBlurType(WindowBlurType::NONE),
    windowBlurRadius(0),
    backgroundCornerRadius(0),
    behindDimInfo(false, Vector4(0.0f, 0.0f, 0.0f, 0.0f))
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
    backgroundCornerRadius(cornerRadius),
    behindDimInfo(false, Vector4(0.0f, 0.0f, 0.0f, 0.0f))
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
    backgroundCornerRadius(0),
    behindDimInfo(false, Vector4(0.0f, 0.0f, 0.0f, 0.0f))
  {
  }

  /**
   * @brief Constructor with blur type, radius, corner radius, and dimming information.
   *
   * This constructor creates a WindowBlurInfo with complete blur configuration including
   * optional dimming effect for behind blur. The dimming effect is particularly useful
   * when the blurred background content is too bright or distracting, helping to improve
   * visual contrast between the background and foreground window content.
   *
   * The corner radius parameter only applies to background blur (WindowBlurType::BACKGROUND)
   * and is ignored for behind blur. The dimming information only applies to behind blur
   * (WindowBlurType::BEHIND) and is ignored for background blur.
   *
   * @SINCE_2_5.5
   * @param[in] type The window blur type (NONE, BACKGROUND, or BEHIND)
   * @param[in] blurRadius The blur radius in pixels. Higher values create stronger blur effects
   * @param[in] cornerRadius The corner radius for background blur in pixels. Only used with BACKGROUND blur type
   * @param[in] dimInfo The dimming configuration for behind blur effect. Only used with BEHIND blur type
   */
  explicit constexpr WindowBlurInfo(WindowBlurType type, int blurRadius, int cornerRadius, WindowDimInfo dimInfo)
  : windowBlurType(type),
    windowBlurRadius(blurRadius),
    backgroundCornerRadius(cornerRadius),
    behindDimInfo(dimInfo)
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
   * This can only be used when window background blur is enabled.
   *
   * @SINCE_2_3.38
   * @param[in] cornerRadius The corner radius value for window background blur in pixels.
   */
  void SetCornerRadiusForBackground(int cornerRadius);

  /**
   * @brief Gets the corner radius value for window background blur.
   *
   * @SINCE_2_3.38
   * @return The corner radius value for window background blur in pixels.
   */
  int GetCornerRadiusForBackground() const;

  /**
   * @brief Sets the dimming information for behind blur effect.
   *
   * This method configures the dimming effect that will be applied to the behind blur region
   * when WindowBlurType::BEHIND is used. The dimming effect helps improve visual contrast
   * between the blurred background and the foreground window content.
   *
   * The dimming effect is only applied when the window blur type is set to BEHIND.
   * For other blur types (NONE or BACKGROUND), this setting has no visual effect.
   *
   * @SINCE_2_5.5
   * @param[in] dimInfo The WindowDimInfo containing enable flag and dim color settings
   *
   * @see GetBehindBlurDimInfo() to retrieve the current dimming configuration
   * @see WindowBlurType::BEHIND for the blur type that uses this dimming effect
   */
  void SetBehindBlurDimInfo(WindowDimInfo dimInfo);

  /**
   * @brief Gets the current dimming information for behind blur effect.
   *
   * This method returns the WindowDimInfo that contains the current dimming configuration
   * for the behind blur region. The returned information includes whether dimming is enabled
   * and the dimming color with its intensity.
   *
   * @SINCE_2_5.5
   * @return The current WindowDimInfo containing enable state and dim color
   *
   * @see SetBehindBlurDimInfo() to modify the dimming configuration
   * @see WindowDimInfo for details about the dimming parameters
   */
  WindowDimInfo GetBehindBlurDimInfo() const;

  WindowBlurInfo(const WindowBlurInfo&)                = default; ///< Default copy constructor
  WindowBlurInfo(WindowBlurInfo&&) noexcept            = default; ///< Default move constructor
  WindowBlurInfo& operator=(const WindowBlurInfo&)     = default; ///< Default copy assignment operator
  WindowBlurInfo& operator=(WindowBlurInfo&&) noexcept = default; ///< Default move assignment operator

public:
  WindowBlurType          windowBlurType;
  int                     windowBlurRadius;
  int                     backgroundCornerRadius;
  WindowDimInfo           behindDimInfo;
};

} // namespace Dali

#endif // DALI_WINDOW_BLUR_INFO_H
