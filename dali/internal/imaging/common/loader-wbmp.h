#ifndef __DALI_TIZEN_PLATFORM_LOADER_WBMP_H__
#define __DALI_TIZEN_PLATFORM_LOADER_WBMP_H__

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

/**
 * @param[in]  input  Information about the input image (including file pointer)
 * @param[out] bitmap The bitmap class where the decoded image will be stored
 * @return  true if file decoded successfully, false otherwise
 */
bool LoadBitmapFromWbmp( const ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap );

/**
 * @param[in]  input  Information about the input image (including file pointer)
 * @param[out] width of image
 * @param[out] height of image
 * @return  true if header loaded successfully, false otherwise
 */
bool LoadWbmpHeader( const ImageLoader::Input& input, unsigned int& width, unsigned int& height );

}

}
#endif
