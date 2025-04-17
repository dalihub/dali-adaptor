/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/image-loading.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/internal/imaging/common/file-download.h>
#include <dali/internal/imaging/common/image-loader.h>
#include <dali/internal/system/common/file-reader.h>
#include <dali/internal/system/common/system-error-print.h>
#include <dali/public-api/object/property-map.h>

namespace Dali
{
namespace
{
// limit maximum image down load size to 50 MB
const size_t MAXIMUM_DOWNLOAD_IMAGE_SIZE = 50 * 1024 * 1024;

std::string ConvertDataReadable(uint8_t* data, const size_t size, const size_t width)
{
  std::ostringstream oss;

  for(size_t i = 0u; i < size; ++i)
  {
    if(i > 0u && (i % width) == 0u)
    {
      oss << '\n';
    }
    oss << ((data[i] >= 0x20 && data[i] < 0x80) ? static_cast<char>(data[i]) : '.');
  }

  return oss.str();
}

} // namespace

Devel::PixelBuffer LoadImageFromFile(const std::string& url, ImageDimensions size, FittingMode::Type fittingMode, SamplingMode::Type samplingMode, bool orientationCorrection)
{
  Integration::BitmapResourceType resourceType(size, fittingMode, samplingMode, orientationCorrection);

  Internal::Platform::FileReader fileReader(url);
  FILE* const                    fp = fileReader.GetFile();
  if(DALI_LIKELY(fp != NULL))
  {
    Dali::Devel::PixelBuffer bitmap;
    bool                     success = TizenPlatform::ImageLoader::ConvertStreamToBitmap(resourceType, url, fp, bitmap);
    if(success && bitmap)
    {
      return bitmap;
    }
  }
  else
  {
    DALI_LOG_ERROR("Error reading file\n");
  }
  return Dali::Devel::PixelBuffer();
}

void LoadImagePlanesFromFile(const std::string& url, std::vector<Devel::PixelBuffer>& buffers, ImageDimensions size, FittingMode::Type fittingMode, SamplingMode::Type samplingMode, bool orientationCorrection)
{
  Integration::BitmapResourceType resourceType(size, fittingMode, samplingMode, orientationCorrection);

  Internal::Platform::FileReader fileReader(url);
  FILE* const                    fp = fileReader.GetFile();
  if(DALI_LIKELY(fp != NULL))
  {
    TizenPlatform::ImageLoader::ConvertStreamToPlanes(resourceType, url, fp, buffers);
  }
  else
  {
    DALI_LOG_ERROR("Error reading file\n");
  }
}

Devel::PixelBuffer LoadImageFromBuffer(const Dali::Vector<uint8_t>& buffer, ImageDimensions size, FittingMode::Type fittingMode, SamplingMode::Type samplingMode, bool orientationCorrection)
{
  if(buffer.Empty())
  {
    DALI_LOG_ERROR("buffer is empty!\n");
    return Dali::Devel::PixelBuffer();
  }
  Integration::BitmapResourceType resourceType(size, fittingMode, samplingMode, orientationCorrection);

  Internal::Platform::FileReader fileReader(buffer);
  FILE* const                    fp = fileReader.GetFile();
  if(DALI_LIKELY(fp != NULL))
  {
    Dali::Devel::PixelBuffer bitmap;
    // Make path as empty string. Path information just for file format hint.
    bool success = TizenPlatform::ImageLoader::ConvertStreamToBitmap(resourceType, std::string(""), fp, bitmap);
    if(success && bitmap)
    {
      return bitmap;
    }
  }
  else
  {
    DALI_LOG_ERROR("Error reading file\n");
  }
  return Dali::Devel::PixelBuffer();
}

Devel::PixelBuffer LoadImageFromBuffer(uint8_t* buffer, size_t bufferSize, ImageDimensions size, FittingMode::Type fittingMode, SamplingMode::Type samplingMode, bool orientationCorrection)
{
  if(buffer == nullptr)
  {
    DALI_LOG_ERROR("buffer is empty!\n");
    return Dali::Devel::PixelBuffer();
  }
  Integration::BitmapResourceType resourceType(size, fittingMode, samplingMode, orientationCorrection);

  Internal::Platform::FileReader fileReader(buffer, bufferSize);
  FILE* const                    fp = fileReader.GetFile();
  if(DALI_LIKELY(fp != NULL))
  {
    Dali::Devel::PixelBuffer bitmap;
    // Make path as empty string. Path information just for file format hint.
    bool success = TizenPlatform::ImageLoader::ConvertStreamToBitmap(resourceType, std::string(""), fp, bitmap);
    if(success && bitmap)
    {
      return bitmap;
    }
  }
  else
  {
    DALI_LOG_ERROR("Error reading file\n");
  }
  return Dali::Devel::PixelBuffer();
}

ImageDimensions GetClosestImageSize(const std::string& filename,
                                    ImageDimensions    size,
                                    FittingMode::Type  fittingMode,
                                    SamplingMode::Type samplingMode,
                                    bool               orientationCorrection)
{
  ImageDimensions dimension = TizenPlatform::ImageLoader::GetClosestImageSize(filename, size, fittingMode, samplingMode, orientationCorrection);

  dimension.SetWidth(std::min(dimension.GetWidth(), static_cast<uint16_t>(GetMaxTextureSize())));
  dimension.SetHeight(std::min(dimension.GetHeight(), static_cast<uint16_t>(GetMaxTextureSize())));

  return dimension;
}

ImageDimensions GetOriginalImageSize(const std::string& filename, bool orientationCorrection)
{
  return TizenPlatform::ImageLoader::GetClosestImageSize(filename, ImageDimensions(0, 0), FittingMode::DEFAULT, SamplingMode::BOX_THEN_LINEAR, orientationCorrection);
}

Devel::PixelBuffer DownloadImageSynchronously(const std::string& url, ImageDimensions size, FittingMode::Type fittingMode, SamplingMode::Type samplingMode, bool orientationCorrection)
{
  Integration::BitmapResourceType resourceType(size, fittingMode, samplingMode, orientationCorrection);

  bool                  succeeded;
  Dali::Vector<uint8_t> dataBuffer;
  size_t                dataSize;

  succeeded = TizenPlatform::Network::DownloadRemoteFileIntoMemory(url, dataBuffer, dataSize, MAXIMUM_DOWNLOAD_IMAGE_SIZE);
  if(succeeded)
  {
    size_t blobSize = dataBuffer.Size();

    if(DALI_LIKELY(blobSize > 0U))
    {
      // Open a file handle on the memory buffer:
      Dali::Internal::Platform::FileReader fileReader(dataBuffer, blobSize);
      FILE* const                          fp = fileReader.GetFile();
      if(DALI_LIKELY(NULL != fp))
      {
        Dali::Devel::PixelBuffer bitmap;
        bool                     result = TizenPlatform::ImageLoader::ConvertStreamToBitmap(
          resourceType,
          url,
          fp,
          bitmap);

        if(result && bitmap)
        {
          return bitmap;
        }
        else
        {
          DALI_LOG_ERROR("Unable to decode bitmap supplied as in-memory blob.\n");

          auto prefixSize  = std::min(static_cast<decltype(blobSize)>(0x200), blobSize); // maximum 512 bytes.
          auto errorString = ConvertDataReadable(reinterpret_cast<uint8_t*>(dataBuffer.Begin()), prefixSize, 0x40);
          DALI_LOG_DEBUG_INFO("URL: %s\n", url.c_str());
          DALI_LOG_DEBUG_INFO("Downloaded data (prefix %zu bytes of %zu bytes):\n", prefixSize, blobSize);
          DALI_LOG_DEBUG_INFO("%s\n", errorString.c_str());
        }
      }
      else
      {
        DALI_LOG_ERROR("Error reading file\n");
      }
    }
    else
    {
      DALI_LOG_ERROR("Error download empty buffer!\n");
    }
  }
  else
  {
    DALI_LOG_ERROR("Error download failed!\n");
  }
  return Dali::Devel::PixelBuffer();
}

unsigned int GetMaxTextureSize()
{
  return TizenPlatform::ImageLoader::GetMaxTextureSize();
}

} // namespace Dali
