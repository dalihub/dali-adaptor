#ifndef __DALI_INTERNAL_GIF_LOADING_H__
#define __DALI_INTERNAL_GIF_LOADING_H__

/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <stdint.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/math/uint-16-pair.h>

namespace Dali
{
class PixelData;
typedef Dali::Uint16Pair ImageDimensions;

/**
 * @brief Load an animated gif file.
 *
 * @param[in] url The url of the gif to load.
 * @param[out] pixelData The loaded pixel data for each frame.
 * @param[out] frameDelays The loaded delay time for each frame.
 *
 * @return True if the loading succeeded, false otherwise.
 */
DALI_IMPORT_API bool LoadAnimatedGifFromFile( const std::string& url, std::vector<Dali::PixelData>& pixelData, Dali::Vector<uint32_t>& frameDelays );

/**
 * @brief Get the size of a gif image.
 *
 * This function will read the header info from file on disk.
 *
 * @param [in] url The URL of the gif file.
 * @return The width and height in pixels of the gif image.
 */
DALI_IMPORT_API ImageDimensions GetGifImageSize( const std::string& url );

} // namespace Dali

#endif // __DALI_INTERNAL_GIF_LOADING_H__
