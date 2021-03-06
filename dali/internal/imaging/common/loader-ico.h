#ifndef DALI_TIZEN_PLATFORM_LOADER_ICO_H
#define DALI_TIZEN_PLATFORM_LOADER_ICO_H

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

namespace Ico
{
//00 00 01 00 01 00 20 20
const unsigned char MAGIC_BYTE_1 = 0x00;
const unsigned char MAGIC_BYTE_2 = 0x00;
} // namespace Ico
/**
 * @param[in]  input  Information about the input image (including file pointer)
 * @param[out] bitmap The bitmap class where the decoded image will be stored
 * @return  true if file decoded successfully, false otherwise
 */
bool LoadBitmapFromIco(const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap);

/**
 * @param[in]  input  Information about the input image (including file pointer)
 * @param[out] width of image
 * @param[out] height of image
 * @return  true if header loaded successfully, false otherwise
 */
bool LoadIcoHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height);

} // namespace TizenPlatform

} // namespace Dali

#endif // DALI_TIZEN_PLATFORM_LOADER_ICO_H
