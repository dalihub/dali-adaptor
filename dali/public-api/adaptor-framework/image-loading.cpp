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
 */

// CLASS HEADER
#include <dali/public-api/adaptor-framework/image-loading.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/string-utils.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/image-loader.h>
#include <dali/internal/system/common/file-reader.h>

namespace Dali
{
Dali::PixelBuffer LoadImageFromFile(StringView url, ImageDimensions size, SamplingMode::Type samplingMode, bool orientationCorrection)
{
  Integration::BitmapResourceType resourceType(size, samplingMode, orientationCorrection);

  // The loader needs a null-terminated path.
  const std::string stdUrl = Integration::ToStdString(url);

  Internal::Platform::FileReader fileReader(stdUrl);
  FILE* const                    fp = fileReader.GetFile();
  if(DALI_LIKELY(fp != NULL))
  {
    Dali::PixelBuffer bitmap;
    bool              success = TizenPlatform::ImageLoader::ConvertStreamToBitmap(resourceType, stdUrl, fp, bitmap);
    if(success && bitmap)
    {
      return bitmap;
    }
  }
  else
  {
    DALI_LOG_ERROR("Error reading file\n");
  }
  return Dali::PixelBuffer();
}

ImageDimensions GetOriginalImageSize(StringView filename, bool orientationCorrection)
{
  return TizenPlatform::ImageLoader::GetClosestImageSize(Integration::ToStdString(filename), ImageDimensions(0, 0), SamplingMode::BOX_THEN_LINEAR, orientationCorrection);
}

} // namespace Dali
