#ifndef __DALI_SLP_PLATFORM_LOADER_KTX_H__
#define __DALI_SLP_PLATFORM_LOADER_KTX_H__

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

// INTERNAL INCLUDES
#include <dali/public-api/common/intrusive-ptr.h>

// EXTERNAL INCLUDES
#include <cstdio>

namespace Dali
{

namespace Integration
{
  class ImageData;
  typedef IntrusivePtr<ImageData> ImageDataPtr;
}

struct ImageAttributes;

namespace SlpPlatform
{

namespace Ktx
{
const unsigned char MAGIC_BYTE_1 = 0xAB;
const unsigned char MAGIC_BYTE_2 = 0x4B;
} // namespace Ktx

/**
 * Loads a compressed bitmap from a KTX file without decoding it.
 * This function checks the header first
 * and if it is not a KTX file, then it returns straight away.
 * @param[in]  fp      Pointer to the Image file
 * @param[in]  attributes  Describes the dimensions, pixel format and other details for loading the image data
 * @param[out] bitmap  A pointer to assign to a newly allocated ImageData object containing the result of the load.
 * @return  true if file loaded successfully, false otherwise
 */
bool LoadBitmapFromKtx(FILE * const fp, ImageAttributes& attributes, Integration::ImageDataPtr& bitmap);

/**
 * Loads the header of a KTX file and fills in the width and height appropriately.
 * @param[in]   fp      Pointer to the Image file
 * @param[in]  attributes  Describes the dimensions, pixel format and other details for loading the image data
 * @param[out]  width   Is set with the width of the image
 * @param[out]  height  Is set with the height of the image
 * @return true if the file's header was read successully, false otherwise
 */
bool LoadKtxHeader(FILE * const fp, const ImageAttributes& attributes, unsigned int &width, unsigned int &height);

} // namespace SlpPlatform

} // namespace Dali

#endif // __DALI_SLP_PLATFORM_LOADER_KTX_H__
