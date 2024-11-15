#ifndef DALI_NATIVE_IMAGE_SOURCE_DEVEL_H
#define DALI_NATIVE_IMAGE_SOURCE_DEVEL_H
/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/math/rect.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>

namespace Dali
{
namespace DevelNativeImageSource
{
/**
 * @brief Converts the current pixel contents to either a JPEG or PNG format
 * and write that to the filesystem.
 *
 * @param[in] image The instance of NativeImageSource.
 * @param[in] filename Identify the filesystem location at which to write the encoded image.
 *                     The extension determines the encoding used.
 *                     The two valid encoding are (".jpeg"|".jpg") and ".png".
 * @param[in] quality The value to control image quality for jpeg file format in the range [1, 100]
 * @return    @c true if the pixels were written, and @c false otherwise
 */
DALI_ADAPTOR_API bool EncodeToFile(NativeImageSource& image, const std::string& filename, const uint32_t quality);

/**
 * @brief Acquire buffer and information of an internal native image.
 *
 * AcquireBuffer() and ReleaseBuffer() are a pair.
 * ReleaseBuffer() MUST be called after AcquireBuffer().
 * @param[in] image The instance of NativeImageSource.
 * @param[out] width The width of image
 * @param[out] height The height of image
 * @param[out] stride The stride of image
 * @return     The buffer of an internal native image
 * @note This locks a mutex until ReleaseBuffer is called.
 */
DALI_ADAPTOR_API uint8_t* AcquireBuffer(NativeImageSource& image, uint32_t& width, uint32_t& height, uint32_t& stride);

/**
 * @brief Release information of an internal native image.
 *
 * AcquireBuffer() and ReleaseBuffer() are a pair.
 * ReleaseBuffer() MUST be called after AcquireBuffer().
 * @param[in] image The instance of NativeImageSource.
 * @param[in] updatedArea The updated area of the buffer.
 * @return @c true If the buffer is released successfully, and @c false otherwise
 * @note The empty updatedArea means that the entire area has been changed.
 * @note This unlocks the mutex locked by AcquireBuffer.
 */
DALI_ADAPTOR_API bool ReleaseBuffer(NativeImageSource& image, const Rect<uint32_t>& updatedArea);

/**
 * @brief Sets PixelBuffers to NativeImageSource
 *
 * @param[in] image The instance of NativeImageSource.
 * @param[in] pixbuf Pixel buffer to copy.
 * @param[in] pixelFormat Pixel format of the pixel buffer
 * @return @c true If the buffer is successfully set.
 * @note The width and height of the input pixel buffer should be same with those of NativeImageSource.
 * @note Only Pixel::Format::RGB888 and Pixel::Format::RGBA8888 are available as a input pixel format.
 */
DALI_ADAPTOR_API bool SetPixels(NativeImageSource& image, uint8_t* pixbuf, const Pixel::Format& pixelFormat);

/**
 * @brief Set the Resource Destruction Callback object
 *
 * @param[in] image The instance of NativeImageSource.
 * @param[in] callback The Resource Destruction callback
 * @note Ownership of the callback is passed onto this class.
 */
DALI_ADAPTOR_API void SetResourceDestructionCallback(NativeImageSource& image, EventThreadCallback* callback);

/**
 * @brief Enable a back buffer.
 * @param[in] image The instance of NativeImageSource.
 * @param[in] enable Whether a back buffer is enabled.
 * @note The front buffer will be copied to the back buffer before rendering if the front buffer is updated by AcquireBuffer and ReleaseBuffer.
 */
DALI_ADAPTOR_API void EnableBackBuffer(NativeImageSource& image, bool enable);

} // namespace DevelNativeImageSource

} // namespace Dali

#endif // DALI_NATIVE_IMAGE_SOURCE_DEVEL_H
