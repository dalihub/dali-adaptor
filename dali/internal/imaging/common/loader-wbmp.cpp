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

// HEADER
#include <dali/internal/imaging/common/loader-wbmp.h>

// EXTERNAL INCLUDES
#include <cstdio>
#include <cstdlib>
#include <cstring>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/integration-api/debug.h>
#include <dali/internal/system/common/system-error-print.h>

namespace Dali
{
namespace TizenPlatform
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_LOADER_WBMP");
#endif

// TODO : We need to determine it in dali-common.h or something else. Currently, we set this value in code level.
#define DALI_BYTE_ORDER_BIG_ENDIAN 0

#define IMG_MAX_SIZE 65536

#define IMG_TOO_BIG(w, h)                                 \
  ((((unsigned long long)w) * ((unsigned long long)h)) >= \
   ((1ULL << (29)) - 2048))

//extract multiple bytes integer , and saved in *data
int extractMultiByteInteger(uint32_t* data, const uint8_t* const& map, size_t length, size_t* position)
{
  // the header field contains an image type indentifier of multi-byte length(TypeField), an octet of general header info(FixHeaderField)
  //,  a multi-byte width field(Width) and a multi-byte height field(Height) and so on.
  // The actual organisation of the image data depends on the image type
  // for Ext Headers flag (7th bit), 1 = More will follow, 0 = Last octet
  // so in the for loop, if(buf & 0x80 == 0), loop will be exited
  int     targetMultiByteInteger = 0, readBufCount;
  uint8_t buf;

  for(readBufCount = 0;;)
  {
    // readBufCount means the count that fetched data from map
    // extractMultiByteInteger() is to fetch wbmp type , width, and height
    // for wbmp type, when readBufCount == 1, buf = 0x00, it will exit the loop
    // for width, it have 4 bytes, so when readBufCount == 4, it must exit the loop
    // for general width and height, if(buf & 0x80) == 0, then the next byte does not need to fetch again
    // first step, readBufCount = 1 , read int(4 bytes) to buf, if buf & 0x80 !=0, the buf need to continue to fetch
    // second step, readBufCount = 2, read next( 4 bytes) to buf, if buf & 0x80 == 0, then assigned the buf to target
    if(DALI_UNLIKELY((readBufCount++) == 4))
    {
      return -1;
    }
    if(DALI_UNLIKELY(*position > length))
    {
      return -1;
    }
    buf                    = map[(*position)++];
    targetMultiByteInteger = (targetMultiByteInteger << 7) | (buf & 0x7f);

    if((buf & 0x80) == 0)
    {
      DALI_LOG_INFO(gLogFilter, Debug::Verbose, "position: %d, readBufCount: %d\n", *position, readBufCount);
      break;
    }
  }
  *data = targetMultiByteInteger;
  return 0;
}

// Calculate 4bit integer into 4byte integer
constexpr uint32_t Calculate4BitTo4Byte(const uint8_t& input)
{
  uint32_t output = 0;
#if DALI_BYTE_ORDER_BIG_ENDIAN
  output |= static_cast<uint32_t>(input & 0x08) << 21;
  output |= static_cast<uint32_t>(input & 0x04) << 14;
  output |= static_cast<uint32_t>(input & 0x02) << 7;
  output |= static_cast<uint32_t>(input & 0x01);
#else
  output |= static_cast<uint32_t>(input & 0x08) >> 3;
  output |= static_cast<uint32_t>(input & 0x04) << 6;
  output |= static_cast<uint32_t>(input & 0x02) << 15;
  output |= static_cast<uint32_t>(input & 0x01) << 24;
#endif
  return output * 0xff;
}

/**
 * @brief Calculation result bit-->byte table in compile.
 * Required memory = 16 * 4byte = 64byte
 */
// clang-format off
constexpr std::uint32_t cachedCalculation4BitTo4ByteTable[16] = {
  Calculate4BitTo4Byte(0x00), Calculate4BitTo4Byte(0x01), Calculate4BitTo4Byte(0x02), Calculate4BitTo4Byte(0x03),
  Calculate4BitTo4Byte(0x04), Calculate4BitTo4Byte(0x05), Calculate4BitTo4Byte(0x06), Calculate4BitTo4Byte(0x07),
  Calculate4BitTo4Byte(0x08), Calculate4BitTo4Byte(0x09), Calculate4BitTo4Byte(0x0a), Calculate4BitTo4Byte(0x0b),
  Calculate4BitTo4Byte(0x0c), Calculate4BitTo4Byte(0x0d), Calculate4BitTo4Byte(0x0e), Calculate4BitTo4Byte(0x0f)};
// clang-format on

bool LoadWbmpHeader(FILE* const fp, unsigned int& width, unsigned int& height, uint32_t& type, uint32_t& fsize, Dali::Vector<uint8_t>& map, size_t& position, bool loadHeaderOnly)
{
  if(DALI_UNLIKELY(fp == NULL))
  {
    DALI_LOG_ERROR("Error loading bitmap\n");
    return false;
  }
  position = 0;

  uint32_t w, h;
  if(DALI_UNLIKELY(fseek(fp, 0, SEEK_END)))
  {
    DALI_LOG_ERROR("Error seeking WBMP data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }
  long positionIndicator = ftell(fp);

  fsize = 0u;
  if(positionIndicator > -1L)
  {
    fsize = static_cast<uint32_t>(positionIndicator);
  }

  if(DALI_UNLIKELY(0u == fsize))
  {
    DALI_LOG_ERROR("Error: filesize is 0!\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  if(DALI_UNLIKELY(fseek(fp, 0, SEEK_SET)))
  {
    DALI_LOG_ERROR("Error seeking WBMP data\n");
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }
  if(DALI_UNLIKELY(fsize <= 4))
  {
    DALI_LOG_ERROR("Error: WBMP Raw Data Not Found! Maybe this image is not wbmp format. fileSize : %u\n", fsize);
    return false;
  }
  if(DALI_UNLIKELY(fsize > 4096 * 4096 * 4))
  {
    DALI_LOG_ERROR("Error: WBMP size is too large! fileSize : %u\n", fsize);
    return false;
  }

  // Read only 80 bytes if we are try to getting header file now.
  // Else, read whole file data into map
  uint32_t readDataSize = fsize;

  if(loadHeaderOnly)
  {
    // type(1 byte) + fixedheader(1 byte) + width(uint) + height(uint)
    uint32_t headerSize = 1 + 1 + 4 + 4; // 8 + 8 + 32 + 32;

    readDataSize = std::min(headerSize, fsize);
  }

  map.ResizeUninitialized(fsize);
  if(DALI_UNLIKELY(fread(&map[0], 1, readDataSize, fp) != readDataSize))
  {
    DALI_LOG_ERROR("image file read opeation error! fileSize : %u, readDataSize : %u\n", fsize, readDataSize);
    DALI_PRINT_SYSTEM_ERROR_LOG();
    return false;
  }

  const std::uint8_t* const inputBufferPtr = &map[0];

  if(DALI_UNLIKELY(extractMultiByteInteger(&type, inputBufferPtr, readDataSize, &position) < 0))
  {
    DALI_LOG_ERROR("Error: unable to read type! Maybe this image is not wbmp format. fileSize : %u, readDataSize : %u\n", fsize, readDataSize);
    return false;
  }

  position++; /* skipping one byte */

  if(DALI_UNLIKELY(type != 0))
  {
    DALI_LOG_ERROR("Error: unknown wbmp format! Maybe this image is not wbmp format. type : %u, fileSize : %u, readDataSize : %u\n", type, fsize, readDataSize);
    return false;
  }
  if(DALI_UNLIKELY(extractMultiByteInteger(&w, inputBufferPtr, readDataSize, &position) < 0))
  {
    DALI_LOG_ERROR("Error: can not read width! Maybe this image is not wbmp format. fileSize : %u, readDataSize : %u\n", fsize, readDataSize);
    return false;
  }
  if(DALI_UNLIKELY(extractMultiByteInteger(&h, inputBufferPtr, readDataSize, &position) < 0))
  {
    DALI_LOG_ERROR("Error: can not read height! Maybe this image is not wbmp format. fileSize : %u, readDataSize : %u\n", fsize, readDataSize);
    return false;
  }

  if(DALI_UNLIKELY((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE)))
  {
    DALI_LOG_ERROR("Error: file size is not supported! Maybe this image is not wbmp format. fileSize : %u, readDataSize : %u, width : %u, height : %u\n", fsize, readDataSize, w, h);
    return false;
  }

  width  = w;
  height = h;
  return true;
}
} // end unnamed namespace

bool LoadBitmapFromWbmp(const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap)
{
  FILE* const fp = input.file;

  Dali::Vector<uint8_t> map;
  uint32_t              fsize;
  size_t                position = 0;
  uint32_t              w, h;
  uint32_t              type;
  if(DALI_UNLIKELY(!LoadWbmpHeader(fp, w, h, type, fsize, map, position, false)))
  {
    DALI_LOG_ERROR("Error loading wbmp header\n");
    return false;
  }

  uint32_t lineByteLength = (w + 7) >> 3;

  // fsize was wrong! Load failed.
  if(DALI_UNLIKELY(position + h * lineByteLength > fsize))
  {
    DALI_LOG_ERROR("Pixel infomation is bigger than file size! (%u + %u * %u > %u)\n", static_cast<std::uint32_t>(position), h, lineByteLength, fsize);
    return false;
  }
  const uint8_t* const inputBufferPtr = &map[0];

  // w >= 1 and h >= 1. So we can assume that outputPixels is not null.
  auto outputPixels = (bitmap = Dali::Devel::PixelBuffer::New(w, h, Pixel::L8)).GetBuffer();

  /**
   * @code
   * std::uint8_t* line = NULL;
   * std::uint32_t cur  = 0, x, y;
   * for(y = 0; y < h; y++)
   * {
   *   line = &map[0] + position;
   *   position += lineByteLength;
   *   for(x = 0; x < w; x++)
   *   {
   *     int idx    = x >> 3;
   *     int offset = 1 << (0x07 - (x & 0x07));
   *     if(line[idx] & offset)
   *     {
   *       outputPixels[cur] = 0xff; //0xffffffff;
   *     }
   *     else
   *     {
   *       outputPixels[cur] = 0x00; //0xff000000;
   *     }
   *     cur++;
   *   }
   * }
   * @endcode
   */

  const uint8_t* inputPixels                 = inputBufferPtr + position;
  const uint32_t lineBitLengthWithoutPadding = (w >> 3) << 3;

  for(uint32_t y = 0; y < h; ++y)
  {
    uint32_t x = 0;
    if((reinterpret_cast<std::ptrdiff_t>(outputPixels) & (sizeof(uint32_t) - 1)) == 0)
    {
      for(; x < lineBitLengthWithoutPadding; x += 8)
      {
        // memset whole 8 bits
        // outputPixels filled 4 bytes in one operation.
        // cachedCalculation4BitTo4ByteTable calculated in compile-time.
        *(reinterpret_cast<uint32_t*>(outputPixels + 0)) = cachedCalculation4BitTo4ByteTable[((*inputPixels) >> 4) & 0x0f];
        *(reinterpret_cast<uint32_t*>(outputPixels + 4)) = cachedCalculation4BitTo4ByteTable[(*inputPixels) & 0x0f];
        outputPixels += 8;
        ++inputPixels;
      }
    }
    {
      // memset linePadding bits naive.
      for(; x < w; ++x)
      {
        const uint8_t offset = (0x07 - (x & 0x07));
        *outputPixels        = ((*inputPixels) >> offset) & 1 ? 0xff : 0x00;
        ++outputPixels;
        if(offset == 0)
        {
          ++inputPixels;
        }
      }
    }
  }

  return true;
}

bool LoadWbmpHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height)
{
  FILE* const fp = input.file;

  Dali::Vector<uint8_t> map;
  size_t                position = 0;
  uint32_t              type;
  uint32_t              fsize;

  if(DALI_UNLIKELY(!LoadWbmpHeader(fp, width, height, type, fsize, map, position, true)))
  {
    DALI_LOG_ERROR("Error loading wbmp header! Maybe this image is not wbmp format.\n");
    return false;
  }

  return true;
}

} // namespace TizenPlatform
} // namespace Dali
