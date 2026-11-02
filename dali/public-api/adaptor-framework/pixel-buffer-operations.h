#ifndef DALI_PIXEL_BUFFER_OPERATIONS_H
#define DALI_PIXEL_BUFFER_OPERATIONS_H

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
#include <dali/public-api/images/pixel-buffer.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace DALI_NAMESPACE
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

/**
 * @brief Image operations that work on a PixelBuffer.
 * @SINCE_2_5.47
 */
namespace PixelBufferOperations
{
/**
 * @brief Applies a Gaussian blur to the given pixel buffer with the given radius.
 *
 * The blur is applied in place, so the pointer returned by PixelBuffer::GetBuffer() and
 * the dimensions of the pixel buffer stay valid.
 *
 * @note A bigger radius will yield a blurrier image.
 * @note A radius of 0 does nothing and succeeds. A negative radius does nothing
 * and fails.
 * @note Valid for the pixel formats A8, L8, LA88, RGB888, RGB8888, BGR8888,
 * RGBA8888, BGRA8888, CHROMINANCE_U and CHROMINANCE_V. Fails otherwise.
 *
 * @SINCE_2_5.47
 * @param[in] pixelBuffer The pixel buffer to apply the Gaussian blur to
 * @param[in] blurRadius The radius for the Gaussian blur
 * @return @c false if the Gaussian blur fails (invalid radius, invalid pixel format,
 * empty buffer or memory issues)
 */
DALI_ADAPTOR_API bool ApplyGaussianBlur(PixelBuffer pixelBuffer, float blurRadius);

/**
 * @brief Resizes the given pixel buffer to the given dimensions.
 *
 * Uses either Lanczos4 for downscaling or Mitchell for upscaling.
 *
 * @warning The buffer is replaced, so the pointer returned by a previous
 * PixelBuffer::GetBuffer() call becomes invalid. Call GetBuffer() again after resizing.
 *
 * @note Resizing to the current dimensions does nothing and succeeds.
 * @note Valid for the pixel formats A8, L8, LA88, RGB888, RGB8888, BGR8888,
 * RGBA8888, BGRA8888, CHROMINANCE_U and CHROMINANCE_V. Fails otherwise.
 *
 * @SINCE_2_5.47
 * @param[in] pixelBuffer The pixel buffer to resize
 * @param[in] width The new width in pixels
 * @param[in] height The new height in pixels
 * @return @c false if the resize fails (invalid dimensions, invalid pixel format,
 * empty buffer or memory issues)
 */
DALI_ADAPTOR_API bool Resize(PixelBuffer pixelBuffer, uint16_t width, uint16_t height);

} // namespace PixelBufferOperations

/**
 * @}
 */
} //namespace DALI_NAMESPACE

#endif // DALI_PIXEL_BUFFER_OPERATIONS_H
