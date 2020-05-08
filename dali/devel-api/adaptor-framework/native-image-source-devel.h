#ifndef DALI_NATIVE_IMAGE_SOURCE_DEVEL_H
#define DALI_NATIVE_IMAGE_SOURCE_DEVEL_H
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

// EXTERNAL INCLUDES
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
DALI_ADAPTOR_API bool EncodeToFile( NativeImageSource& image, const std::string& filename, const uint32_t quality );

/**
 * @brief Acquire buffer and information of an internal native image.
 *
 * AcquireBuffer() and ReleaseBuffer() are a pair.
 * It should be call ReleaseBuffer() after AcquireBuffer().
 * @param[in] image The instance of NativeImageSource.
 * @param[out] width The width of image
 * @param[out] height The height of image
 * @param[out] stride The stride of image
 * @return     The buffer of an internal native image
 */
DALI_ADAPTOR_API uint8_t* AcquireBuffer( NativeImageSource& image, uint16_t& width, uint16_t& height, uint16_t& stride );

/**
 * @brief Release information of an internal native image.
 *
 * AcquireBuffer() and ReleaseBuffer() are a pair.
 * It should be call ReleaseBuffer() after AcquireBuffer().
 * @param[in] image The instance of NativeImageSource.
 * @return     @c true If the buffer is released successfully, and @c false otherwise
 */
DALI_ADAPTOR_API bool ReleaseBuffer( NativeImageSource& image );

} // namespace DevelNativeImageSource

} // namespace Dali

#endif // DALI_NATIVE_IMAGE_SOURCE_DEVEL_H
