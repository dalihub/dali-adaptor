#ifndef DALI_TIZEN_PLATFORM_LOADER_KTX_H
#define DALI_TIZEN_PLATFORM_LOADER_KTX_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

#include <dali/devel-api/adaptor-framework/image-loader-input.h>
#include <cstdio>

namespace Dali
{
namespace Devel
{
class PixelBuffer;
}

namespace TizenPlatform
{
class ResourceLoadingClient;

namespace Ktx
{
const unsigned char MAGIC_BYTE_1 = 0xAB;
const unsigned char MAGIC_BYTE_2 = 0x4B;
} // namespace Ktx

/**
 * Loads a compressed bitmap from a KTX file without decoding it.
 * This function checks the header first
 * and if it is not a KTX file, then it returns straight away.
 * @param[in]  input  Information about the input image (including file pointer)
 * @param[out] bitmap The bitmap class where the decoded image will be stored
 * @return  true if file loaded successfully, false otherwise
 */
bool LoadBitmapFromKtx(const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap);

/**
 * Loads the header of a KTX file and fills in the width and height appropriately.
 * @param[in]   fp      Pointer to the Image file
 * @param[in]  attributes  Describes the dimensions, pixel format and other details for loading the image data
 * @param[out]  width   Is set with the width of the image
 * @param[out]  height  Is set with the height of the image
 * @return true if the file's header was read successully, false otherwise
 */
bool LoadKtxHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height);

} // namespace TizenPlatform

} // namespace Dali

#endif // DALI_TIZEN_PLATFORM_LOADER_KTX_H
