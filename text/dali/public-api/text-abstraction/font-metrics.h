#ifndef __DALI_TEXT_ABSTRACTION_FONT_METRICS_H__
#define __DALI_TEXT_ABSTRACTION_FONT_METRICS_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

namespace Dali
{

namespace TextAbstraction
{

/**
 * The metrics for a Font expressed in 26.6 fractional pixel format.
 */
struct FontMetrics
{
  /**
   * @brief Default constructor.
   */
  FontMetrics();

  /**
   * @brief Create the font metrics in 26.6 fractional pixel format.
   */
  FontMetrics( signed long ascender26Dot6,
               signed long descender26Dot6,
               signed long height26Dot6 );

  signed long ascender;   ///< The ascender in 26.6 fractional pixels.
  signed long descender;  ///< The descender in 26.6 fractional pixels.
  signed long height;     ///< The height in 26.6 fractional pixels.
};

} // Dali

} // TextAbstraction

#endif //__DALI_TEXT_ABSTRACTION_FONT_METRICS_H__
