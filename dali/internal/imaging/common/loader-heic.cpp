/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

// CLASS HEADER
#include <dali/internal/imaging/common/loader-heic.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/heif-decoder.h>

namespace DALI_NAMESPACE
{
namespace TizenPlatform
{
bool LoadHeicHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height)
{
  return HeifDecoder::ReadHeader(input, width, height);
}

bool LoadBitmapFromHeic(const Dali::ImageLoader::Input& input, Dali::PixelBuffer& bitmap)
{
  return HeifDecoder::Decode(input, bitmap);
}

bool LoadPlanesFromHeic(const Dali::ImageLoader::Input& input, std::vector<Dali::PixelBuffer>& pixelBuffers)
{
  pixelBuffers.clear();
  if(HeifDecoder::DecodePlanes(input, pixelBuffers))
  {
    return true;
  }

  pixelBuffers.clear();
  Dali::PixelBuffer bitmap;
  if(!HeifDecoder::Decode(input, bitmap))
  {
    DALI_LOG_ERROR("[HEIF] RGB fallback after YUV plane decoding failed.\n");
    return false;
  }
  pixelBuffers.push_back(bitmap);
  return true;
}
} // namespace TizenPlatform
} // namespace DALI_NAMESPACE
