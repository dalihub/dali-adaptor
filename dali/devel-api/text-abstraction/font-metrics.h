#ifndef DALI_TEXT_ABSTRACTION_FONT_METRICS_H
#define DALI_TEXT_ABSTRACTION_FONT_METRICS_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace TextAbstraction
{
/**
 * The metrics for a Font expressed in pixels.
 */
struct DALI_ADAPTOR_API FontMetrics
{
  /**
   * @brief Default constructor.
   */
  FontMetrics();

  /**
   * @brief Create the font metrics in pixels.
   */
  FontMetrics(float ascenderPixels,
              float descenderPixels,
              float heightPixels,
              float underlinePositionPixels,
              float underlineThicknessPixels);

  /**
   * @brief Adjust ascender/descender to enforce design line-height compatibility.
   *
   * Design tools are assumed to reference the font metric "height".
   * The goal is to make (ascender - descender) equal to or less than height.
   *
   * Behavior:
   * - Let lineHeight = (ascender - descender).
   * - If lineHeight < height: keep as-is (no even enforcement).
   * - If lineHeight == height:
   *     - If lineHeight is odd, shrink by 1px via descender (+1).
   *     - If even, no change.
   * - If lineHeight > height:
   *     - Clamp to the largest even value <= height.
   *     - Apply 2px shrink as a pair (ascender -1, descender +1) per step,
   *       and any remaining 1px via descender (+1).
   */
  void ApplyDesignLineHeightCompat();

  float ascender;           ///< The ascender in pixels.
  float descender;          ///< The descender in pixels.
  float height;             ///< The height in pixels.
  float underlinePosition;  ///< The underline position in pixels.
  float underlineThickness; ///< The vertical height of the underline in pixels.
};

} // namespace TextAbstraction

} // namespace Dali

#endif //DALI_TEXT_ABSTRACTION_FONT_METRICS_H
