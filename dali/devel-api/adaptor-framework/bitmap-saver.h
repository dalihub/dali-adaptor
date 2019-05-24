#ifndef DALI_ADAPTOR_BITMAP_SAVER_H
#define DALI_ADAPTOR_BITMAP_SAVER_H

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
 */

// EXTERNAL INCLUDES
#include <string>
#include <dali/public-api/images/pixel.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

/**
 * Store the given pixel data to a file.
 * The suffix of the filename determines what type of file will be stored,
 * currently only jpeg and png formats are supported.
 *
 * @param[in] pixelBuffer Pointer to the pixel data
 * @param[in] filename    Filename to save
 * @param[in] pixelFormat The format of the buffer's pixels
 * @param[in] width       The width of the image in pixels
 * @param[in] height      The height of the image in pixels
 *
 * @return true if the file was saved
 */
DALI_ADAPTOR_API bool EncodeToFile(const unsigned char* const pixelBuffer,
                                  const std::string& filename,
                                  const Pixel::Format pixelFormat,
                                  const std::size_t width,
                                  const std::size_t height);

} // namespace Dali


#endif // DALI_ADAPTOR_BITMAP_SAVER_H
