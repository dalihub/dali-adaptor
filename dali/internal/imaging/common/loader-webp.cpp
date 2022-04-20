/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

// HEADER
#include <dali/internal/imaging/common/loader-webp.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/internal/imaging/common/webp-loading.h>

namespace Dali
{
namespace TizenPlatform
{
namespace
{
constexpr uint32_t FIRST_FRAME_INDEX = 0u;
}

bool LoadWebpHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height)
{
  FILE* const                fp          = input.file;
  Dali::AnimatedImageLoading webPLoading = Dali::AnimatedImageLoading(Dali::Internal::Adaptor::WebPLoading::New(fp).Get());
  if(webPLoading)
  {
    ImageDimensions imageSize = webPLoading.GetImageSize();
    if(webPLoading.HasLoadingSucceeded())
    {
      width  = imageSize.GetWidth();
      height = imageSize.GetHeight();
      return true;
    }
  }
  return false;
}

bool LoadBitmapFromWebp(const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap)
{
  FILE* const                fp          = input.file;
  Dali::AnimatedImageLoading webPLoading = Dali::AnimatedImageLoading(Dali::Internal::Adaptor::WebPLoading::New(fp).Get());
  if(webPLoading)
  {
    Dali::Devel::PixelBuffer pixelBuffer = webPLoading.LoadFrame(FIRST_FRAME_INDEX);
    if(pixelBuffer && webPLoading.HasLoadingSucceeded())
    {
      bitmap = pixelBuffer;
      return true;
    }
  }
  return false;
}

} // namespace TizenPlatform

} // namespace Dali
