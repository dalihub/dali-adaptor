#ifndef __DALI_SLP_PLATFORM_LOADER_JPEG_H__
#define __DALI_SLP_PLATFORM_LOADER_JPEG_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <stdio.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/images/pixel.h>
#include "image-encoder.h"

namespace Dali
{

namespace Integration
{
  class Bitmap;
}

struct ImageAttributes;

namespace SlpPlatform
{

namespace Jpeg
{
const unsigned char MAGIC_BYTE_1 = 0xFF;
const unsigned char MAGIC_BYTE_2 = 0xD8;
} // namespace Jpeg

/**
 * Loads the bitmap from an JPEG file.  This function checks the header first
 * and if it is not a JPEG file, then it returns straight away.
 * @param[in]  fp      Pointer to the Image file
 * @param[in]  bitmap  The bitmap class where the decoded image will be stored
 * @param[in]  attributes  Describes the dimensions, pixel format and other details for loading the image data
 * @return  true if file decoded successfully, false otherwise
 */
bool LoadBitmapFromJpeg(FILE *fp, Integration::Bitmap& bitmap, ImageAttributes& attributes);

/**
 * Loads the header of a JPEG file and fills in the width and height appropriately.
 * @param[in]   fp      Pointer to the Image file
 * @param[out]  width   Is set with the width of the image
 * @param[out]  height  Is set with the height of the image
 * @return true if the file's header was read successully, false otherwise
 */
bool LoadJpegHeader(FILE *fp, unsigned int &width, unsigned int &height);

/**
 * Encode raw pixel data to JPEG format.
 * @param[in]  pixelBuffer    Pointer to raw pixel data to be encoded
 * @param[out] encodedPixels  Encoded pixel data. Existing contents will be overwritten
 * @param[in]  width          Image width
 * @param[in]  height         Image height
 * @param[in]  pixelFormat    Input pixel format (must be Pixel::RGB888)
 * @param[in]  quality        JPEG quality on usual 1 to 100 scale.
 */
bool EncodeToJpeg(const unsigned char* pixelBuffer, std::vector< unsigned char >& encodedPixels, std::size_t width, std::size_t height, Pixel::Format pixelFormat, unsigned quality = 80);

} // namespace SlpPlatform

} // namespace Dali

#endif // __DALI_SLP_PLATFORM_LOADER_JPEG_H__
