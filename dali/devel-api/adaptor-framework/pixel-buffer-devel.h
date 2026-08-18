#ifndef DALI_PIXEL_BUFFER_DEVEL_H
#define DALI_PIXEL_BUFFER_DEVEL_H

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
#include <dali/public-api/math/degree.h>
#include <dali/public-api/object/property-map.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/pixel-buffer.h>

namespace Dali
{
namespace DevelPixelBuffer
{
/**
 * @brief Converts a PixelBuffer into a PixelData, which can be uploaded to a texture.
 *
 * The buffer is handed over to the new PixelData without being copied, and the given
 * handle is reset to an empty handle.
 *
 * @warning Any other handle that keeps a reference to the same object is left with no
 * buffer. GetBuffer() returns NULL for such a handle, and the sizes it reports are 0.
 *
 * @param[in,out] pixelBuffer The pixel buffer to convert. Reset to an empty handle on return
 * @param[in] releaseAfterUpload Whether the converted PixelData releases its buffer as soon
 * as it has been uploaded to a texture. Such a PixelData cannot be uploaded twice
 * @return A new PixelData which owns the pixel buffer's buffer
 */
DALI_ADAPTOR_API PixelData Convert(PixelBuffer& pixelBuffer, bool releaseAfterUpload);

/**
 * @brief Copies the data of the given pixel buffer into a new PixelData object, which
 * could be used for uploading to a texture.
 *
 * Unlike Convert(), the given pixel buffer keeps its buffer, so it can be edited and
 * copied again.
 *
 * @param[in] pixelBuffer The pixel buffer to copy
 * @return A new PixelData object containing a copy of the pixel buffer's data
 */
DALI_ADAPTOR_API PixelData CreatePixelData(PixelBuffer pixelBuffer);

/**
 * @brief Applies the mask to the given pixel buffer.
 *
 * If the pixel buffer doesn't have an alpha channel, then it will be converted to a
 * format that supports at least the width of the colour channels and the alpha channel
 * from the mask.
 *
 * If cropToMask is set to true, then the contentScale is applied first to the pixel
 * buffer, and the target buffer is cropped to the size of the mask. If it's set to false,
 * then the mask is scaled to match the pixel buffer's size before the mask is applied.
 *
 * @param[in] pixelBuffer The pixel buffer to apply the mask to
 * @param[in] mask The mask to apply
 * @param[in] contentScale The scaling factor to apply to the content
 * @param[in] cropToMask Whether to crop the output to the mask size (true) or scale the
 * mask to the content size (false)
 */
DALI_ADAPTOR_API void ApplyMask(PixelBuffer pixelBuffer, PixelBuffer mask, float contentScale = 1.0f, bool cropToMask = false);

/**
 * @brief Crops the given pixel buffer to the given crop rectangle.
 *
 * The crop rectangle will be clamped to the edges of the buffer if it is larger.
 *
 * @param[in] pixelBuffer The pixel buffer to crop
 * @param[in] x The top left corner's X
 * @param[in] y The top left corner's y
 * @param[in] width The crop width
 * @param[in] height The crop height
 */
DALI_ADAPTOR_API void Crop(PixelBuffer pixelBuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/**
 * @brief Crops the given pixel buffer centered to match the aspect ratio of
 * (width, height), then resizes it to (width, height).
 *
 * Equivalent to the old SCALE_TO_FILL FittingMode behavior at load time.
 * If the source has a different aspect ratio than the target, the excess portion
 * is removed symmetrically from the longer axis before resizing.
 *
 * @param[in] pixelBuffer The pixel buffer to crop and resize
 * @param[in] width  Target width in pixels
 * @param[in] height Target height in pixels
 */
DALI_ADAPTOR_API void ApplyCenterCrop(PixelBuffer pixelBuffer, uint16_t width, uint16_t height);

/**
 * @brief Scales the given pixel buffer to fit within (width, height) preserving aspect
 * ratio (no upscaling), then pads the remaining area with black (0x00).
 *
 * Equivalent to the old SHRINK_TO_FIT FittingMode behavior at load time.
 * If the source is already smaller than the target, no scaling is performed
 * and only padding is added.
 *
 * @param[in] pixelBuffer The pixel buffer to scale and pad
 * @param[in] width  Target width in pixels
 * @param[in] height Target height in pixels
 */
DALI_ADAPTOR_API void ApplyLetterbox(PixelBuffer pixelBuffer, uint16_t width, uint16_t height);

/**
 * @brief Returns the Exif metadata of the given pixel buffer as a property map.
 *
 * @param[in] pixelBuffer The pixel buffer to read the metadata from
 * @param[out] metadata Property map object to write into
 * @return True on success
 */
DALI_ADAPTOR_API bool GetMetadata(PixelBuffer pixelBuffer, Property::Map& metadata);

/**
 * @brief Multiplies the image's colour values by the alpha value. This provides better
 * blending capability.
 *
 * @note Compressed pixel format doesn't have any efforts.
 * @note Even if pixel format doesn't have an alpha channel, it will be marked as
 * PreMultiplied.
 *
 * @param[in] pixelBuffer The pixel buffer to pre-multiply
 */
DALI_ADAPTOR_API void MultiplyColorByAlpha(PixelBuffer pixelBuffer);

/**
 * @brief Returns whether the given pixel buffer is pre-multiplied or not.
 *
 * @note This only reports whether MultiplyColorByAlpha() has been applied to the pixel
 * buffer. A buffer the application filled with pre-multiplied pixels itself is not
 * reported as pre-multiplied.
 *
 * @param[in] pixelBuffer The pixel buffer to query
 * @return true if alpha is pre-multiplied
 */
DALI_ADAPTOR_API bool IsAlphaPreMultiplied(PixelBuffer pixelBuffer);

/**
 * @brief Rotates the given pixel buffer by the given angle.
 *
 * @note Operation valid for pixel formats: A8, L8, LA88, RGB888, RGB8888, BGR8888,
 * RGBA8888 and BGRA8888. Fails otherwise.
 * @note The operation does nothing for angles equivalent to 0 degrees: -360, 360, 720, etc.
 * @note If the pixel buffer does rotate, all the pointers to the internal pixel buffer
 * retrieved by the method GetBuffer() become invalid.
 *
 * @param[in] pixelBuffer The pixel buffer to rotate
 * @param[in] angle The angle in degrees
 * @return @c false if the rotation fails (invalid pixel format or memory issues)
 */
DALI_ADAPTOR_API bool Rotate(PixelBuffer pixelBuffer, Degree angle);

/**
 * @brief Gets the brightness of the given pixel buffer.
 *
 * @note The range is 255 to 0. The closer to 255, the brighter. 0 is not bright.
 *
 * @param[in] pixelBuffer The pixel buffer to query
 * @return The brightness of the pixel buffer
 */
DALI_ADAPTOR_API uint32_t GetBrightness(PixelBuffer pixelBuffer);

} // namespace DevelPixelBuffer

} // namespace Dali

#endif // DALI_PIXEL_BUFFER_DEVEL_H
