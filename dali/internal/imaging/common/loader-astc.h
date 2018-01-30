#ifndef __DALI_TIZEN_PLATFORM_LOADER_ASTC_H__
#define __DALI_TIZEN_PLATFORM_LOADER_ASTC_H__

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <cstdio>
#include <dali/internal/imaging/common/image-loader-input.h>

namespace Dali
{
namespace Devel
{
class PixelBuffer;
}


namespace TizenPlatform
{

class ResourceLoadingClient;

namespace Astc
{
const unsigned char MAGIC_BYTE_1 = 0x13;
const unsigned char MAGIC_BYTE_2 = 0xAB;
} // namespace Astc


/**
 * Loads a compressed bitmap image from a ASTC file without decoding it.
 * This function checks the header first
 * and if it is not a ASTC file, or the header contents are invalid, it will return a failure.
 * @param[in]  input  Information about the input image (including file pointer)
 * @param[out] bitmap The bitmap class where the decoded image will be stored
 * @return True if file loaded successfully, false otherwise
 */
bool LoadBitmapFromAstc( const ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap );

/**
 * Loads the header of a ASTC file and fills in the width and height appropriately.
 * @param[in]  input  Information about the input image (including file pointer)
 * @param[out] width  Is set with the width of the image
 * @param[out] height Is set with the height of the image
 * @return            True if the header was read successfully, false otherwise
 */
bool LoadAstcHeader( const ImageLoader::Input& input, unsigned int& width, unsigned int& height );


} // namespace TizenPlatform

} // namespace Dali

#endif // __DALI_TIZEN_PLATFORM_LOADER_ASTC_H__
