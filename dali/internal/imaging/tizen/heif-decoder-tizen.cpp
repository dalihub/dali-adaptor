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
#include <dali/internal/imaging/common/heif-decoder.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/images/pixel-buffer.h>
#include <dali/public-api/images/pixel.h>
#include <algorithm>
#include <cstring> ///< for memcmp
#include <sstream>

extern "C" {
#include <heif.h>
}

// INTERNAL INCLUDES
#include <dali/internal/system/common/system-error-print.h>

namespace DALI_NAMESPACE
{
namespace TizenPlatform
{
namespace HeifDecoder
{
namespace
{
static constexpr Dali::Pixel::Format GetPixelFormat(heif_color_format_e colorFormat)
{
  switch(colorFormat)
  {
    case HEIF_COLOR_FORMAT_RGB24:
    {
      return Dali::Pixel::Format::RGB888;
    }
    case HEIF_COLOR_FORMAT_BGRA:
    {
      return Dali::Pixel::Format::BGRA8888;
    }
    case HEIF_COLOR_FORMAT_RGBA:
    {
      return Dali::Pixel::Format::RGBA8888;
    }
    case HEIF_COLOR_FORMAT_ARGB:    ///< TODO : need to be support
    case HEIF_COLOR_FORMAT_YUV420P: ///< TODO : need to be support
    default:
    {
      DALI_LOG_ERROR("Not support HEIC color format : %d\n", colorFormat);
      break;
    }
  }
  return Dali::Pixel::Format::INVALID;
}

/**
 * @brief Internal method to load HEIC header info from a file.
 *
 * @param[in]  filePointer The file pointer to the HEIC file to read
 * @param[out] width       The width is output to this value
 * @param[out] height      The height is output to this value
 * @param[out] buffer      The raw buffer of file
 * @param[out] fileSize    The byte of file
 * @return                 True if the file is valid, false otherwise
 */
bool LoadHeicHeaderHelper(FILE* const filePointer, unsigned int& width, unsigned int& height, Dali::Vector<uint8_t>& buffer, uint32_t& fileSize)
{
  if(DALI_UNLIKELY(filePointer == nullptr))
  {
    DALI_LOG_ERROR("Null file handle passed to HEIC bitmap file loader.\n");
    return false;
  }

  if(DALI_UNLIKELY(fseek(filePointer, 0, SEEK_END)))
  {
    DALI_LOG_ERROR("Error seeking HEIC data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  long positionIndicator = ftell(filePointer);
  fileSize               = 0u;

  if(positionIndicator > -1L)
  {
    fileSize = static_cast<unsigned int>(positionIndicator);
  }

  if(DALI_UNLIKELY(0u == fileSize))
  {
    DALI_LOG_ERROR("Error HEIC data size is zero!\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  if(DALI_UNLIKELY(fseek(filePointer, 0, SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking HEIC data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  buffer.ResizeUninitialized(fileSize);
  if(DALI_UNLIKELY(fread(&buffer[0], 1, fileSize, filePointer) != fileSize))
  {
    DALI_LOG_ERROR("image file read opeation error!\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  const uint8_t* const inputBufferPtr = &buffer[0];

  int               ret       = 0;
  heif_image_info_h imageInfo = nullptr;

  ret = heif_get_image_info_from_buffer((unsigned char*)(inputBufferPtr), (size_t)fileSize, &imageInfo);
  if(DALI_UNLIKELY(ret != LIBHEIF_ERROR_NONE))
  {
    DALI_LOG_ERROR("[HEIF] heif_get_image_info_from_buffer fail %d\n", ret);
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  int signedWidth  = 0;
  int signedHeight = 0;

  /* get basic information */
  ret = heif_image_info_get_attrs(imageInfo,
                                  HEIF_IMAGE_WIDTH, &signedWidth,
                                  HEIF_IMAGE_HEIGHT, &signedHeight,
                                  nullptr);

  if(DALI_UNLIKELY(ret != LIBHEIF_ERROR_NONE))
  {
    DALI_LOG_ERROR("[HEIF] heif_image_info_get_attrs fail %d\n", ret);
    DALI_PRINT_SYSTEM_ERROR_LOG();
    heif_image_info_free(imageInfo);
    return false;
  }

  width  = static_cast<uint32_t>(signedWidth);
  height = static_cast<uint32_t>(signedHeight);
  heif_image_info_free(imageInfo);
  return true;
}

} // Unnamed namespace.

// File loading API entry-point:
bool ReadHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height)
{
  Dali::Vector<uint8_t> buffer;
  uint32_t              fileSize;

  FILE* const filePointer = input.file;

  if(DALI_UNLIKELY(!LoadHeicHeaderHelper(filePointer, width, height, buffer, fileSize)))
  {
    DALI_LOG_ERROR("Error loading heic header! Maybe this image is not heic/heif format.\n");
    return false;
  }
  return true;
}

// File loading API entry-point:
bool Decode(const Dali::ImageLoader::Input& input, Dali::PixelBuffer& bitmap)
{
  // Load the header info
  Dali::Vector<uint8_t> buffer;
  uint32_t              fileSize;
  uint32_t              width, height;

  FILE* const filePointer = input.file;

  if(DALI_UNLIKELY(!LoadHeicHeaderHelper(filePointer, width, height, buffer, fileSize)))
  {
    DALI_LOG_ERROR("Could not load HEIC Header from file.\n");
    return false;
  }

  const uint8_t* const inputBufferPtr = &buffer[0];

  int                 ret         = 0;
  heif_image_h        heifImage   = nullptr;
  heif_color_format_e colorFormat = HEIF_COLOR_FORMAT_RGB24; // HEIF_COLOR_FORMAT_RGBA;

  ret = heif_decode_image_from_buffer((unsigned char*)(inputBufferPtr), (size_t)fileSize, colorFormat, &heifImage);
  if(DALI_UNLIKELY(ret != LIBHEIF_ERROR_NONE))
  {
    DALI_LOG_ERROR("[HEIF] heif_decode_image_from_buffer fail %d\n", ret);
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  // Get raw decoded buffer from heifImage
  uint32_t            actualWidth = 0u, actualHeight = 0u;
  heif_color_format_e actualColorFormat = HEIF_COLOR_FORMAT_NONE;
  uint8_t*            rawDataPtr        = nullptr;
  size_t              rawDataSize       = 0u;
  ret                                   = heif_image_get_image(heifImage, &actualWidth, &actualHeight, &actualColorFormat, &rawDataPtr, &rawDataSize);

  if(DALI_UNLIKELY(ret != LIBHEIF_ERROR_NONE))
  {
    DALI_LOG_ERROR("[HEIF] heif_image_get_image fail %d\n", ret);
    DALI_PRINT_SYSTEM_ERROR_LOG();
    heif_image_free(heifImage);
    return false;
  }

  if(DALI_UNLIKELY(width != actualWidth || height != actualHeight || colorFormat != actualColorFormat))
  {
    DALI_LOG_ERROR("[HEIF] Header and Source image infomation not mached!\n");
    DALI_LOG_ERROR("[HEIF][HEADER] width: %u, height: %u format: %u\n", width, height, colorFormat);
    DALI_LOG_ERROR("[HEIF][SOURCE] width: %u, height: %u format: %u data: %p, size: %zu\n", actualWidth, actualHeight, actualColorFormat, rawDataPtr, rawDataSize);
    heif_image_free(heifImage);
    return false;
  }

  // Allocate the decoded pixel buffer:
  Pixel::Format pixelFormat = GetPixelFormat(colorFormat);
  if(pixelFormat == Pixel::Format::INVALID)
  {
    DALI_LOG_ERROR("[HEIF] Not supported color format.\n");
    heif_image_free(heifImage);
    return false;
  }

  // decode the whole image into bitmap buffer
  auto pixels = (bitmap = Dali::PixelBuffer::New(width, height, pixelFormat)).GetBuffer();

  const uint32_t bitmapBufferSize = bitmap.GetBufferSize();

  if(bitmapBufferSize != rawDataSize)
  {
    DALI_LOG_ERROR("[HEIF] raw buffer is not matched with pixel buffer size. %u vs %zu\n", bitmapBufferSize, rawDataSize);
    rawDataSize = std::min(rawDataSize, static_cast<size_t>(bitmapBufferSize));
  }

  memcpy(pixels, rawDataPtr, rawDataSize);
  heif_image_free(heifImage);
  return true;
}

} // namespace HeifDecoder

} // namespace TizenPlatform

} // namespace DALI_NAMESPACE
