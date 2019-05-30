#ifndef DALI_TIZEN_PLATFORM_LOADER_WBMP_H
#define DALI_TIZEN_PLATFORM_LOADER_WBMP_H

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
 *
 */

#include <cstdio>
#include <dali/devel-api/adaptor-framework/image-loader-input.h>

namespace Dali
{

namespace Devel
{
class PixelBuffer;
}

namespace TizenPlatform
{

class ResourceLoadingClient;

/**
 * @param[in]  input  Information about the input image (including file pointer)
 * @param[out] bitmap The bitmap class where the decoded image will be stored
 * @return  true if file decoded successfully, false otherwise
 */
bool LoadBitmapFromWbmp( const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap );

/**
 * @param[in]  input  Information about the input image (including file pointer)
 * @param[out] width of image
 * @param[out] height of image
 * @return  true if header loaded successfully, false otherwise
 */
bool LoadWbmpHeader( const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height );

} // namespace TizenPlatform

} // namespace Dali

#endif // DALI_TIZEN_PLATFORM_LOADER_WBMP_H
