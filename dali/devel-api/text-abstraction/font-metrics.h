#ifndef DALI_TEXT_ABSTRACTION_FONT_METRICS_H
#define DALI_TEXT_ABSTRACTION_FONT_METRICS_H

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
  FontMetrics( float ascenderPixels,
               float descenderPixels,
               float heightPixels,
               float underlinePositionPixels,
               float underlineThicknessPixels );

  float ascender;             ///< The ascender in pixels.
  float descender;            ///< The descender in pixels.
  float height;               ///< The height in pixels.
  float underlinePosition;    ///< The underline position in pixels.
  float underlineThickness;   ///< The vertical height of the underline in pixels.
};

} // Dali

} // TextAbstraction

#endif //DALI_TEXT_ABSTRACTION_FONT_METRICS_H
