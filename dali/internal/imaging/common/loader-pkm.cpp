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
 *
 */

// CLASS HEADER
#include <dali/internal/imaging/common/loader-pkm.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/images/pixel.h>
#include <cstring> ///< for memcmp

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/pixel-buffer-impl.h> ///< for Internal::Adaptor::PixelBuffer::New()
#include <dali/internal/system/common/system-error-print.h>

namespace Dali
{
namespace TizenPlatform
{
namespace
{
// Max width or height of an image.
const unsigned MAX_TEXTURE_DIMENSION = 4096;
// Max bytes of image data allowed. Not a precise number, just a sanity check.
const unsigned MAX_IMAGE_DATA_SIZE = MAX_TEXTURE_DIMENSION * MAX_TEXTURE_DIMENSION;

const uint8_t PKM_10_VERSION_MAJOR = '1';
const uint8_t PKM_10_VERSION_MINOR = '0';

const uint8_t PKM_20_VERSION_MAJOR = '2';
const uint8_t PKM_20_VERSION_MINOR = '0';

typedef uint8_t Byte;

// This bytes identify an PKM native file.
const Byte FileIdentifier[] =
  {
    0x50,
    0x4B,
    0x4D,
    0x20,
};

using namespace Pixel;

// Convert from data type to Dali::Pixel:Format.
const Pixel::Format PKM_FORMAT_TABLE[] =
  {
    Pixel::Format::COMPRESSED_RGB8_ETC1,      ///< 0x0000
    Pixel::Format::COMPRESSED_RGB8_ETC2,      ///< 0x0001
    Pixel::Format::COMPRESSED_SRGB8_ETC2,     ///< 0x0002
    Pixel::Format::COMPRESSED_RGBA8_ETC2_EAC, ///< 0x0003

    Pixel::Format::COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, ///< 0x0004
    Pixel::Format::COMPRESSED_R11_EAC,                       ///< 0x0005
    Pixel::Format::COMPRESSED_RG11_EAC,                      ///< 0x0006
    Pixel::Format::COMPRESSED_SIGNED_R11_EAC,                ///< 0x0007
    Pixel::Format::COMPRESSED_SIGNED_RG11_EAC,               ///< 0x0008
};

/**
 * @brief This struct defines the PKM file header values. From PKM specifications.
 * Packed attribute stops the structure from being aligned to compiler defaults
 * so we can be sure of reading the whole header from file in one call to fread().
 * Note: members to not conform to coding standards in order to be consistent with PKM spec.
 */
struct PkmFileHeader
{
  uint8_t magic[4];
  uint8_t versionMajor;
  uint8_t versionMinor;
  uint8_t dataType[2];       // Big Endian
  uint8_t extendedWidth[2];  // Big Endian
  uint8_t extendedHeight[2]; // Big Endian
  uint8_t originalWidth[2];  // Big Endian
  uint8_t originalHeight[2]; // Big Endian
} __attribute__((__packed__));

/**
 * @brief Helper function to get the integer value from Big Endian data array.
 *
 * @param[in] data 2-byte data
 * @return The value of input data said
 */
inline uint32_t GetBigEndianValue(const uint8_t data[2])
{
  return (static_cast<uint32_t>(data[0]) << 8) | data[1];
}

/**
 * @brief Uses header information to return the respective PKM pixel format.
 *
 * @param[in] header A populated PkmFileHeader struct
 * @return    The pixel format, or INVALID if there is no valid pixel format.
 */
Pixel::Format GetPkmPixelFormat(PkmFileHeader& header)
{
  uint32_t pkmFormat = GetBigEndianValue(header.dataType);
  if(DALI_LIKELY(pkmFormat < sizeof(PKM_FORMAT_TABLE) / sizeof(PKM_FORMAT_TABLE[0])))
  {
    return PKM_FORMAT_TABLE[pkmFormat];
  }
  return Pixel::INVALID;
}

/**
 * @brief Internal method to load PKM header info from a file.
 *
 * @param[in]  filePointer The file pointer to the PKM file to read
 * @param[out] width       The width is output to this value
 * @param[out] height      The height is output to this value
 * @param[out] fileHeader  This will be populated with the header data
 * @return                 True if the file is valid, false otherwise
 */
bool LoadPkmHeader(FILE* const filePointer, unsigned int& width, unsigned int& height, PkmFileHeader& fileHeader)
{
  // Pull the bytes of the file header in as a block:
  const unsigned int readLength = sizeof(PkmFileHeader);
  if(DALI_UNLIKELY(fread(&fileHeader, 1, readLength, filePointer) != readLength))
  {
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  // Check the header contains the PKM native file identifier.
  bool headerIsValid = memcmp(fileHeader.magic, FileIdentifier, sizeof(fileHeader.magic)) == 0;
  if(DALI_UNLIKELY(!headerIsValid))
  {
    DALI_LOG_ERROR("File is not a valid PKM native file\n");
    // Return here as otherwise, if not a valid PKM file, we are likely to pick up other header errors spuriously.
    return false;
  }

  headerIsValid &= (fileHeader.versionMajor == PKM_10_VERSION_MAJOR) || (fileHeader.versionMajor == PKM_20_VERSION_MAJOR);
  if(DALI_UNLIKELY(!headerIsValid))
  {
    DALI_LOG_ERROR("PKM version doesn't support. file version : %c.%c\n", static_cast<char>(fileHeader.versionMajor), static_cast<char>(fileHeader.versionMinor));
    return false;
  }

  // Convert the 2-byte values for width and height to a single resultant value.
  width  = GetBigEndianValue(fileHeader.originalWidth);
  height = GetBigEndianValue(fileHeader.originalHeight);

  // Check image dimensions are within limits.
  if(DALI_UNLIKELY((width > MAX_TEXTURE_DIMENSION) || (height > MAX_TEXTURE_DIMENSION)))
  {
    DALI_LOG_ERROR("PKM file has larger than supported dimensions: %d,%d\n", width, height);
    headerIsValid = false;
  }

  return headerIsValid;
}

} // Unnamed namespace.

// File loading API entry-point:
bool LoadPkmHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height)
{
  PkmFileHeader fileHeader;
  return LoadPkmHeader(input.file, width, height, fileHeader);
}

// File loading API entry-point:
bool LoadBitmapFromPkm(const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap)
{
  FILE* const filePointer = input.file;
  if(DALI_UNLIKELY(!filePointer))
  {
    DALI_LOG_ERROR("Null file handle passed to PKM compressed bitmap file loader.\n");
    return false;
  }

  // Load the header info.
  PkmFileHeader fileHeader;
  unsigned int  width, height;

  if(DALI_UNLIKELY(!LoadPkmHeader(filePointer, width, height, fileHeader)))
  {
    DALI_LOG_ERROR("Could not load PKM Header from file.\n");
    return false;
  }

  // Retrieve the pixel format from the PKM header.
  Pixel::Format pixelFormat = GetPkmPixelFormat(fileHeader);
  if(DALI_UNLIKELY(pixelFormat == Pixel::INVALID))
  {
    DALI_LOG_ERROR("No internal pixel format supported for PKM file pixel format.\n");
    return false;
  }

  // Retrieve the file size.
  if(DALI_UNLIKELY(fseek(filePointer, 0L, SEEK_END)))
  {
    DALI_LOG_ERROR("Could not seek through file.\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  off_t fileSize = ftell(filePointer);
  if(DALI_UNLIKELY(fileSize == -1L))
  {
    DALI_LOG_ERROR("Could not determine PKM file size.\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  if(DALI_UNLIKELY(fseek(filePointer, sizeof(PkmFileHeader), SEEK_SET)))
  {
    DALI_LOG_ERROR("Could not seek through file.\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  // Data size is file size - header size.
  size_t imageByteCount = fileSize - sizeof(PkmFileHeader);

  // Sanity-check the image data is not too large:
  if(DALI_UNLIKELY(imageByteCount > MAX_IMAGE_DATA_SIZE))
  {
    DALI_LOG_ERROR("PKM file has too large image-data field.\n");
    return false;
  }

  // allocate pixel data
  auto* pixels = static_cast<uint8_t*>(malloc(imageByteCount));

  if(DALI_UNLIKELY(pixels == nullptr))
  {
    DALI_LOG_ERROR("Buffer allocation failed. (required memory : %zu byte)\n", imageByteCount);
    return false;
  }

  // Create bitmap who will use allocated buffer.
  const auto& bitmapInternal = Internal::Adaptor::PixelBuffer::New(pixels, imageByteCount, width, height, 0, pixelFormat);
  bitmap                     = Dali::Devel::PixelBuffer(bitmapInternal.Get());

  // Load the image data.
  const size_t bytesRead = fread(pixels, 1, imageByteCount, filePointer);

  // Check the size of loaded data is what we expected.
  if(DALI_UNLIKELY(bytesRead != imageByteCount))
  {
    DALI_LOG_ERROR("Read of image pixel data failed. (required image bytes : %zu, actual read from file : %zu\n", imageByteCount, bytesRead);
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  return true;
}

} // namespace TizenPlatform

} // namespace Dali
