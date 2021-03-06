#ifndef DALI_INTERNAL_ADAPTOR_GAUSSIAN_BLUR_H
#define DALI_INTERNAL_ADAPTOR_GAUSSIAN_BLUR_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
 */

#include <dali/internal/imaging/common/pixel-buffer-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Perform a one dimension Gaussian blur convolution and write its output buffer transposed.
 *
 * @param[in] inBuffer The input buffer with the source image
 * @param[in] outBuffer The output buffer with the Gaussian blur applied and transposed
 * @param[in] bufferWidth The width of the buffer
 * @param[in] bufferHeight The height of the buffer
 * @param[in] blurRadius The radius for Gaussian blur
 */
void ConvoluteAndTranspose(unsigned char* inBuffer, unsigned char* outBuffer, const unsigned int bufferWidth, const unsigned int bufferHeight, const float blurRadius);

/**
 * Perform Gaussian blur on a buffer.
 *
 * A Gaussian blur is generated by replacing each pixel’s color values with the average of the surrounding pixels’
 * colors. This region is a circle with the given radius. Thus, a bigger radius yields a blurrier image.
 *
 * @note The pixel format of the buffer must be RGBA8888
 *
 * @param[in] buffer The buffer to apply the Gaussian blur to
 * @param[in] blurRadius The radius for Gaussian blur
 */
void PerformGaussianBlurRGBA(PixelBuffer& buffer, const float blurRadius);

} //namespace Adaptor

} //namespace Internal

} //namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_GAUSSIAN_BLUR_H
