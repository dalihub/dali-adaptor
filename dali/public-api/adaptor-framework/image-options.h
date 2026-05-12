#ifndef DALI_IMAGE_OPTIONS_H
#define DALI_IMAGE_OPTIONS_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/math/int-pair.h>

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

/**
 * @brief The integer dimensions of an image or a region of an image packed into
 *        16 bits per component.
 *
 * This can only be used for images of up to 65535 x 65535 pixels.
 * @SINCE_2_5.14
 */
using ImageDimensions = Dali::Uint16Pair;

/**
 * @brief Filtering options, used when resizing images to sample original pixels.
 *
 * A SamplingMode controls how pixels in an input image are sampled and
 * combined to generate each pixel of a destination image during a scaling.
 *
 * NoFilter and Box modes do not guarantee that the output pixel array
 * exactly matches the rectangle specified by the desired dimensions,
 * but all other filter modes do if the desired dimensions are
 * `<=` the raw dimensions of the input image file.
 * @SINCE_2_5.14
 */
namespace SamplingMode
{
/**
 * @brief Enumeration for SamplingMode type.
 * @SINCE_2_5.14
 */
enum Type
{
  BOX,              ///< Iteratively box filter to generate an image of 1/2, 1/4,
                    ///  1/8, etc width and height and approximately the desired
                    ///  size. This is the default.
                    ///  @SINCE_2_5.14
  NEAREST,          ///< For each output pixel, read one input pixel.
                    ///  @SINCE_2_5.14
  LINEAR,           ///< For each output pixel, read a quad of four input pixels
                    ///  and write a weighted average of them.
                    ///  @SINCE_2_5.14
  BOX_THEN_NEAREST, ///< Iteratively box filter to generate an image of 1/2, 1/4,
                    ///  1/8 etc width and height and approximately the desired
                    ///  size, then for each output pixel, read one pixel from the
                    ///  last level of box filtering.
                    ///  @SINCE_2_5.14
  BOX_THEN_LINEAR,  ///< Iteratively box filter to almost the right size, then for
                    ///  each output pixel, read four pixels from the last level of
                    ///  box filtering and write their weighted average.
                    ///  @SINCE_2_5.14
  NO_FILTER,        ///< No filtering is performed. If the SCALE_TO_FILL scaling mode
                    ///  is enabled, the borders of the image may be trimmed to
                    ///  match the aspect ratio of the desired dimensions.
                    ///  @SINCE_2_5.14
  DONT_CARE,        ///< For caching algorithms where a client strongly prefers a
                    ///  cache-hit to reuse a cached image.
                    ///  @SINCE_2_5.14
  LANCZOS,          ///< Use filter with Lanczos resample algorithm.
                    ///  @SINCE_2_3.43
  BOX_THEN_LANCZOS, ///< Iteratively box filter to generate an image of 1/2, 1/4,
                    ///  1/8 etc width and height and approximately the desired
                    ///  size, then apply Lanczos resample algorithm.
                    ///  @SINCE_2_3.43
};
const Type DEFAULT = BOX;
} // namespace SamplingMode

/**
 * @}
 */
} // namespace Dali

#endif // DALI_IMAGE_OPTIONS_H
