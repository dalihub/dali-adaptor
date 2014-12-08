#ifndef __DALI_IMAGE_WRAPPER_H__
#define __DALI_IMAGE_WRAPPER_H__

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

// EXTERNAL INCLUDES
#include <dali/public-api/dali-core.h>
#include "emscripten/emscripten.h"
#include "emscripten/bind.h"

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

/**
 * Creates a new buffer image from raw data
 *
 * @param[in] data The image byte data
 * @param[in] width The width of the image
 * @param[in] height The height of the image
 * @param[in] pixelFormat The pixel format
 *
 * @returns A Dali BufferImage
 */
Dali::BufferImage BufferImageNew(const std::string& data, unsigned int width, unsigned int height, Dali::Pixel::Format pixelFormat);

/**
 * Gets an encoded buffer from encoded data
 *
 * @param[in] data The image data
 *
 * @returns A Dali EncodedBufferImage
 *
 */
Dali::EncodedBufferImage EncodedBufferImageNew(const std::string& data);

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali

#endif // header
