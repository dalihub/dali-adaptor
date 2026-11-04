#ifndef DALI_INTERNAL_ADAPTOR_PIXEL_BUFFER_OPERATIONS_H
#define DALI_INTERNAL_ADAPTOR_PIXEL_BUFFER_OPERATIONS_H

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
#include <dali/public-api/math/degree.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/image-options.h>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
/**
 * The image operations working on a PixelBuffer. The PixelBuffer class itself is a
 * plain pixel container owned by dali-core; these functions implement the public and
 * devel image-operation API on top of it.
 */
namespace PixelBufferOperations
{
/**
 * @copydoc Dali::PixelBuffer::ApplyGaussianBlur()
 */
bool ApplyGaussianBlur(Dali::PixelBuffer& pixelBuffer, float blurRadius);

/**
 * Resizes the buffer to the given dimensions. Uses either Lanczos4 for downscaling
 * or Mitchell for upscaling.
 * @param[in] pixelBuffer The pixel buffer to resize
 * @param[in] outDimensions The new dimensions
 * @return @e false if the resize fails (invalid dimensions, invalid pixel format,
 * empty buffer or memory issues).
 */
bool Resize(Dali::PixelBuffer& pixelBuffer, ImageDimensions outDimensions);

/**
 * Crops the buffer to the given crop rectangle. Assumes the crop rectangle
 * is within the bounds of this size.
 * @param[in] pixelBuffer The pixel buffer to crop
 * @param[in] x The top left corner's X
 * @param[in] y The top left corner's y
 * @param[in] cropDimensions The dimensions of the crop
 */
void Crop(Dali::PixelBuffer& pixelBuffer, uint16_t x, uint16_t y, ImageDimensions cropDimensions);

/**
 * @copydoc Dali::DevelPixelBufferOperations::ApplyMask()
 */
void ApplyMask(Dali::PixelBuffer& pixelBuffer, Dali::PixelBuffer mask, float contentScale, bool cropToMask);

/**
 * @copydoc Dali::DevelPixelBufferOperations::ApplyCenterCrop()
 */
void ApplyCenterCrop(Dali::PixelBuffer& pixelBuffer, uint16_t width, uint16_t height);

/**
 * @copydoc Dali::DevelPixelBufferOperations::ApplyLetterbox()
 */
void ApplyLetterbox(Dali::PixelBuffer& pixelBuffer, uint16_t width, uint16_t height);

/**
 * @copydoc Dali::DevelPixelBufferOperations::Rotate()
 */
bool Rotate(Dali::PixelBuffer& pixelBuffer, Degree angle);

/**
 * @copydoc Dali::DevelPixelBufferOperations::MultiplyColorByAlpha()
 */
void MultiplyColorByAlpha(Dali::PixelBuffer& pixelBuffer);

/**
 * @copydoc Dali::DevelPixelBufferOperations::GetBrightness()
 */
uint32_t GetBrightness(Dali::PixelBuffer& pixelBuffer);

/**
 * Creates a new buffer which is a crop of the passed in buffer,
 * using the given crop rectangle. Assumes the crop rectangle is
 * within the bounds of this size.
 * @param[in] inBuffer The source buffer
 * @param[in] x The top left corner's X
 * @param[in] y The top left corner's y
 * @param[in] cropDimensions The dimensions of the crop
 * @return the new pixel buffer
 */
Dali::PixelBuffer NewCrop(Dali::PixelBuffer& inBuffer, uint16_t x, uint16_t y, ImageDimensions cropDimensions);

/**
 * Creates a new buffer which is a resized version of the passed in buffer.
 * Uses either Lanczos4 for downscaling, or Mitchell for upscaling.
 * @param[in] inBuffer The source buffer
 * @param[in] outDimensions The new dimensions
 * @return a new buffer of the given size.
 */
Dali::PixelBuffer NewResize(Dali::PixelBuffer& inBuffer, ImageDimensions outDimensions);

} // namespace PixelBufferOperations

} // namespace Adaptor

} // namespace Internal

} //namespace DALI_NAMESPACE

#endif // DALI_INTERNAL_ADAPTOR_PIXEL_BUFFER_OPERATIONS_H
