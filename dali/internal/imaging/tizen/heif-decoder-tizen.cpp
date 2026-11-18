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
#include <dali/internal/imaging/common/pixel-buffer-operations.h>
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
 * @param[out] orientation The number of 90-degree counterclockwise turns
 * @param[out] mirrorMode  The mirror operation to apply after rotation
 * @param[out] buffer      The raw buffer of file
 * @param[out] fileSize    The byte of file
 * @return                 True if the file is valid, false otherwise
 */
bool LoadHeicHeaderHelper(FILE* const filePointer, unsigned int& width, unsigned int& height, unsigned int& orientation, heif_mirror_mode_e& mirrorMode, Dali::Vector<uint8_t>& buffer, uint32_t& fileSize)
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

  int signedWidth       = 0;
  int signedHeight      = 0;
  int signedOrientation = 0;
  int signedMirrorMode  = 0;

  /* get basic information */
  ret = heif_image_info_get_attrs(imageInfo,
                                  HEIF_IMAGE_WIDTH, &signedWidth,
                                  HEIF_IMAGE_HEIGHT, &signedHeight,
                                  HEIF_IMAGE_ORIENTATION, &signedOrientation,
                                  HEIF_IMAGE_MIRROR, &signedMirrorMode,
                                  nullptr);

  if(DALI_UNLIKELY(ret != LIBHEIF_ERROR_NONE))
  {
    DALI_LOG_ERROR("[HEIF] heif_image_info_get_attrs fail %d\n", ret);
    DALI_PRINT_SYSTEM_ERROR_LOG();
    heif_image_info_free(imageInfo);
    return false;
  }

  if(DALI_UNLIKELY(signedWidth <= 0 || signedHeight <= 0 || signedOrientation < 0 || signedOrientation > 3 || signedMirrorMode < HEIF_MIRROR_NONE || signedMirrorMode > HEIF_MIRROR_HORIZONTAL))
  {
    DALI_LOG_ERROR("[HEIF] Invalid image dimensions or transform: %d x %d, orientation %d, mirror %d.\n", signedWidth, signedHeight, signedOrientation, signedMirrorMode);
    heif_image_info_free(imageInfo);
    return false;
  }

  width       = static_cast<uint32_t>(signedWidth);
  height      = static_cast<uint32_t>(signedHeight);
  orientation = static_cast<unsigned int>(signedOrientation);
  mirrorMode  = static_cast<heif_mirror_mode_e>(signedMirrorMode);
  heif_image_info_free(imageInfo);
  return true;
}

bool ApplyTransformations(Dali::PixelBuffer& bitmap, unsigned int orientation, heif_mirror_mode_e mirrorMode)
{
  if(orientation != 0u && !Dali::Internal::Adaptor::PixelBufferOperations::Rotate(bitmap, Degree(static_cast<float>(orientation * 90u))))
  {
    DALI_LOG_ERROR("[HEIF] Failed to rotate decoded image.\n");
    return false;
  }
  if(mirrorMode == HEIF_MIRROR_VERTICAL && !Dali::Internal::Adaptor::PixelBufferOperations::FlipVertical(bitmap))
  {
    DALI_LOG_ERROR("[HEIF] Failed to flip decoded image vertically.\n");
    return false;
  }
  if(mirrorMode == HEIF_MIRROR_HORIZONTAL && !Dali::Internal::Adaptor::PixelBufferOperations::FlipHorizontal(bitmap))
  {
    DALI_LOG_ERROR("[HEIF] Failed to flip decoded image horizontally.\n");
    return false;
  }
  return true;
}

} // Unnamed namespace.

// File loading API entry-point:
bool ReadHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height)
{
  Dali::Vector<uint8_t> buffer;
  uint32_t              fileSize;
  unsigned int          orientation = 0u;
  heif_mirror_mode_e    mirrorMode  = HEIF_MIRROR_NONE;

  FILE* const filePointer = input.file;

  if(DALI_UNLIKELY(!LoadHeicHeaderHelper(filePointer, width, height, orientation, mirrorMode, buffer, fileSize)))
  {
    DALI_LOG_ERROR("Error loading heic header! Maybe this image is not heic/heif format.\n");
    return false;
  }
  if(DALI_LIKELY(input.reorientationRequested) && (orientation & 1u))
  {
    // Swap width and height for 90 or 270 degree rotations
    std::swap(width, height);
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
  unsigned int          orientation = 0u;
  heif_mirror_mode_e    mirrorMode  = HEIF_MIRROR_NONE;

  FILE* const filePointer = input.file;

  if(DALI_UNLIKELY(!LoadHeicHeaderHelper(filePointer, width, height, orientation, mirrorMode, buffer, fileSize)))
  {
    DALI_LOG_ERROR("Could not load HEIC Header from file.\n");
    return false;
  }

  if(DALI_UNLIKELY(!input.reorientationRequested))
  {
    orientation = 0u;
    mirrorMode  = HEIF_MIRROR_NONE;
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
  if(DALI_UNLIKELY(pixelFormat == Pixel::Format::INVALID))
  {
    DALI_LOG_ERROR("[HEIF] Not supported color format. : %d\n", static_cast<int>(colorFormat));
    heif_image_free(heifImage);
    return false;
  }

  // decode the whole image into bitmap buffer
  bitmap = Dali::PixelBuffer::New(width, height, pixelFormat);
  if(DALI_UNLIKELY(!bitmap))
  {
    DALI_LOG_ERROR("[HEIF] Failed to allocate pixel buffer: %u x %u.\n", width, height);
    heif_image_free(heifImage);
    return false;
  }

  auto* pixels = bitmap.GetBuffer();
  if(DALI_UNLIKELY(pixels == nullptr))
  {
    DALI_LOG_ERROR("[HEIF] Allocated pixel buffer has no data: %u x %u.\n", width, height);
    heif_image_free(heifImage);
    return false;
  }

  const uint32_t bitmapBufferSize = bitmap.GetBufferSize();

  if(bitmapBufferSize != rawDataSize)
  {
    DALI_LOG_ERROR("[HEIF] raw buffer is not matched with pixel buffer size. %u vs %zu\n", bitmapBufferSize, rawDataSize);
    rawDataSize = std::min(rawDataSize, static_cast<size_t>(bitmapBufferSize));
  }

  std::memcpy(pixels, rawDataPtr, rawDataSize);
  heif_image_free(heifImage);
  return ApplyTransformations(bitmap, orientation, mirrorMode);
}

} // namespace HeifDecoder

} // namespace TizenPlatform

} // namespace DALI_NAMESPACE
