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

// CLASS HEADER
#include <dali/devel-api/text-abstraction/font-client.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/image-operations.h>
#include <dali/internal/text/text-abstraction/font-client-impl.h>

namespace Dali
{
namespace TextAbstraction
{
const PointSize26Dot6 FontClient::DEFAULT_POINT_SIZE   = 768u;                           // 12*64
const float           FontClient::DEFAULT_ITALIC_ANGLE = 12.f * Dali::Math::PI_OVER_180; // FreeType documentation states the software italic is done by doing a horizontal shear of 12 degrees (file ftsynth.h).

//Default atlas block
const bool     FontClient::DEFAULT_ATLAS_LIMITATION_ENABLED = true;
const uint32_t FontClient::DEFAULT_TEXT_ATLAS_WIDTH         = 512u;
const uint32_t FontClient::DEFAULT_TEXT_ATLAS_HEIGHT        = 512u;
const Size     FontClient::DEFAULT_TEXT_ATLAS_SIZE(DEFAULT_TEXT_ATLAS_WIDTH, DEFAULT_TEXT_ATLAS_HEIGHT);

//Maximum atlas block
const uint32_t FontClient::MAX_TEXT_ATLAS_WIDTH  = 1024u;
const uint32_t FontClient::MAX_TEXT_ATLAS_HEIGHT = 1024u;
const Size     FontClient::MAX_TEXT_ATLAS_SIZE(MAX_TEXT_ATLAS_WIDTH, MAX_TEXT_ATLAS_HEIGHT);

//MAX_WIDTH_FIT_IN_ATLAS: blockWidth + 2 * DOUBLE_PIXEL_PADDING + 1u <= atlasWidth
//MAX_HEIGHT_FIT_IN_ATLAS: blockHeight + 2 * DOUBLE_PIXEL_PADDING + 1u <= atlasHeight
const uint16_t FontClient::PADDING_TEXT_ATLAS_BLOCK = 5u; // 2 * DOUBLE_PIXEL_PADDING + 1u

//Maximum block size to fit into atlas block
const Size FontClient::MAX_SIZE_FIT_IN_ATLAS(MAX_TEXT_ATLAS_WIDTH - PADDING_TEXT_ATLAS_BLOCK, MAX_TEXT_ATLAS_HEIGHT - PADDING_TEXT_ATLAS_BLOCK);

const uint32_t FontClient::NUMBER_OF_POINTS_PER_ONE_UNIT_OF_POINT_SIZE = 64u; //Found this value from toolkit

// FontClient::GlyphBufferData

FontClient::GlyphBufferData::GlyphBufferData()
: buffer{nullptr},
  width{0u},
  height{0u},
  outlineOffsetX{0},
  outlineOffsetY{0},
  format{Pixel::A8},
  compressionType{CompressionType::NO_COMPRESSION},
  isColorEmoji{false},
  isColorBitmap{false},
  isBufferOwned{false}
{
}

FontClient::GlyphBufferData::~GlyphBufferData()
{
  if(isBufferOwned)
  {
    free(buffer);
  }
}

FontClient::GlyphBufferData::GlyphBufferData(FontClient::GlyphBufferData&& rhs) noexcept
: buffer{rhs.buffer},
  width{rhs.width},
  height{rhs.height},
  outlineOffsetX{rhs.outlineOffsetX},
  outlineOffsetY{rhs.outlineOffsetY},
  format{rhs.format},
  compressionType{rhs.compressionType},
  isColorEmoji{rhs.isColorEmoji},
  isColorBitmap{rhs.isColorBitmap},
  isBufferOwned{rhs.isBufferOwned}
{
  // Remove moved data
  rhs.buffer        = nullptr;
  rhs.isBufferOwned = false;
}

FontClient::GlyphBufferData& FontClient::GlyphBufferData::operator=(FontClient::GlyphBufferData&& rhs) noexcept
{
  buffer          = rhs.buffer;
  width           = rhs.width;
  height          = rhs.height;
  outlineOffsetX  = rhs.outlineOffsetX;
  outlineOffsetY  = rhs.outlineOffsetY;
  format          = rhs.format;
  compressionType = rhs.compressionType;
  isColorEmoji    = rhs.isColorEmoji;
  isColorBitmap   = rhs.isColorBitmap;
  isBufferOwned   = rhs.isBufferOwned;

  // Remove moved data
  rhs.buffer        = nullptr;
  rhs.isBufferOwned = false;

  return *this;
}

size_t FontClient::GlyphBufferData::Compress(const uint8_t* const __restrict__ inBuffer, GlyphBufferData& __restrict__ outBufferData)
{
  size_t bufferSize                       = 0u;
  uint8_t*& __restrict__ compressedBuffer = outBufferData.buffer;
  switch(outBufferData.compressionType)
  {
    case TextAbstraction::FontClient::GlyphBufferData::CompressionType::NO_COMPRESSION:
    {
      bufferSize = static_cast<size_t>(outBufferData.width) * static_cast<size_t>(outBufferData.height) * static_cast<size_t>(Pixel::GetBytesPerPixel(outBufferData.format));

      compressedBuffer = (uint8_t*)malloc(bufferSize);
      if(DALI_UNLIKELY(compressedBuffer == nullptr))
      {
        return 0u;
      }
      outBufferData.isBufferOwned = true;

      // Copy buffer without compress
      memcpy(compressedBuffer, inBuffer, bufferSize);
      break;
    }
    case TextAbstraction::FontClient::GlyphBufferData::CompressionType::BPP_4:
    {
      const uint32_t widthByte       = outBufferData.width * Pixel::GetBytesPerPixel(outBufferData.format);
      const uint32_t componentCount  = (widthByte >> 1);
      const bool     considerPadding = (widthByte & 1) ? true : false;

      // For BIT_PER_PIXEL_4 type, we can know final compressed buffer size immediatly.
      bufferSize       = static_cast<size_t>(outBufferData.height) * static_cast<size_t>(componentCount + (considerPadding ? 1 : 0));
      compressedBuffer = (uint8_t*)malloc(bufferSize);
      if(DALI_UNLIKELY(compressedBuffer == nullptr))
      {
        return 0u;
      }
      outBufferData.isBufferOwned = true;

      uint8_t* __restrict__ outBufferPtr      = compressedBuffer;
      const uint8_t* __restrict__ inBufferPtr = inBuffer;

      // Compress for each line
      for(uint32_t y = 0; y < outBufferData.height; ++y)
      {
        for(uint32_t x = 0; x < componentCount; ++x)
        {
          const uint8_t v0 = Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr++));
          const uint8_t v1 = Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr++));

          *(outBufferPtr++) = (v0 << 4) | v1;
        }
        if(considerPadding)
        {
          *(outBufferPtr++) = Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr++));
        }
      }
      break;
    }
    case TextAbstraction::FontClient::GlyphBufferData::CompressionType::RLE_4:
    {
      const uint32_t widthByte = outBufferData.width * Pixel::GetBytesPerPixel(outBufferData.format);

      // Allocate temperal buffer. Note that RLE4 can be bigger than original buffer.
      uint8_t* __restrict__ tempBuffer = (uint8_t*)malloc(outBufferData.height * (widthByte + 1));
      if(DALI_UNLIKELY(tempBuffer == nullptr))
      {
        return 0u;
      }

      uint8_t* __restrict__ outBufferPtr      = tempBuffer;
      const uint8_t* __restrict__ inBufferPtr = inBuffer;

      bufferSize = 0u;

      // Compress for each line
      for(uint32_t y = 0; y < outBufferData.height; ++y)
      {
        uint32_t encodedByte = 0;
        while(encodedByte < widthByte)
        {
          // Case 1 : Remain only 1 byte
          if(DALI_UNLIKELY(encodedByte + 1 == widthByte))
          {
            const uint8_t prev0 = DALI_UNLIKELY(y == 0) ? 0 : Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr - widthByte));
            const uint8_t v0    = (Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr++)) - prev0) & 0x0f; // Intented underflow
            *(outBufferPtr++)   = v0;
            ++encodedByte;
            ++bufferSize;
          }
          // Case 2 : Remain only 2 byte
          else if(DALI_UNLIKELY(encodedByte + 2 == widthByte))
          {
            const uint8_t prev0 = DALI_UNLIKELY(y == 0) ? 0 : Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr - widthByte));
            const uint8_t v0    = (Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr++)) - prev0) & 0x0f; // Intented underflow
            const uint8_t prev1 = DALI_UNLIKELY(y == 0) ? 0 : Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr - widthByte));
            const uint8_t v1    = (Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr++)) - prev1) & 0x0f; // Intented underflow
            encodedByte += 2;
            if(v0 == v1)
            {
              *(outBufferPtr++) = 0x80 | v0;
              ++bufferSize;
            }
            else
            {
              *(outBufferPtr++) = 0x10 | v0;
              *(outBufferPtr++) = v1 << 4;
              bufferSize += 2;
            }
          }
          // Case 3 : Normal case. Remain byte bigger or equal than 3.
          else
          {
            // Compress rule -
            // Read 2 byte as v0 and v1.
            // - If v0 == v1, We can compress. mark the first bit as 1. and remain 3 bit mark as the "runLength - 2".
            //   runLength can be maximum 9.
            // - If v0 != v1, We cannot compress. mark the first bit as 0. and remain 3 bit mark as the "(nonRunLength - 1) / 2"
            //   Due to the BitPerPixel is 4, nonRunLength should be odd value.
            //   nonRunLength cutted if v0 == v1.
            //   nonRunLength can be maximum 15.

            const uint8_t prev0 = DALI_UNLIKELY(y == 0) ? 0 : Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr - widthByte));
            const uint8_t v0    = (Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr++)) - prev0) & 0x0f; // Intented underflow
            const uint8_t prev1 = DALI_UNLIKELY(y == 0) ? 0 : Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr - widthByte));
            const uint8_t v1    = (Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr++)) - prev1) & 0x0f; // Intented underflow
            encodedByte += 2;
            // We can compress by RLE
            if(v0 == v1)
            {
              uint8_t runLength = 2;
              while(encodedByte < widthByte && runLength < 9)
              {
                const uint8_t prev2 = DALI_UNLIKELY(y == 0) ? 0 : Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr - widthByte));
                const uint8_t v2    = (Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr)) - prev2) & 0x0f; // Intented underflow
                if(v2 == v0)
                {
                  ++inBufferPtr;
                  ++encodedByte;
                  ++runLength;
                }
                else
                {
                  break;
                }
              }

              // Update (runLength - 2) result.
              *(outBufferPtr++) = ((0x8 | (runLength - 2)) << 4) | v0;
              ++bufferSize;
            }
            // We cannot compress by RLE.
            else
            {
              // Read one more value.
              const uint8_t prev2 = DALI_UNLIKELY(y == 0) ? 0 : Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr - widthByte));
              const uint8_t v2    = (Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr++)) - prev2) & 0x0f; // Intented underflow
              ++encodedByte;

              uint8_t  nonRunLength          = 3;
              uint8_t* nonRunLengthHeaderPtr = outBufferPtr;
              *(outBufferPtr++)              = v0;
              *(outBufferPtr++)              = (v1 << 4) | v2;
              bufferSize += 2;
              while(encodedByte < widthByte && nonRunLength < 15)
              {
                if(DALI_LIKELY(encodedByte + 1 < widthByte))
                {
                  const uint8_t prew0 = DALI_UNLIKELY(y == 0) ? 0 : Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr - widthByte));
                  const uint8_t w0    = (Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr)) - prew0) & 0x0f; // Intented underflow
                  const uint8_t prew1 = DALI_UNLIKELY(y == 0) ? 0 : Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr + 1 - widthByte));
                  const uint8_t w1    = (Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr + 1)) - prew1) & 0x0f; // Intented underflow
                  if(w0 == w1)
                  {
                    // Stop non-compress logic.
                    break;
                  }
                  else
                  {
                    ++bufferSize;
                    *(outBufferPtr++) = (w0 << 4) | w1;
                    inBufferPtr += 2;
                    encodedByte += 2;
                    nonRunLength += 2;
                  }
                }
                else
                {
                  // Edge case. There is only one pixel remained.
                  const uint8_t prew0 = DALI_UNLIKELY(y == 0) ? 0 : Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr - widthByte));
                  const uint8_t w0    = (Dali::Internal::Platform::CompressBitPerPixel8To4(*(inBufferPtr)) - prew0) & 0x0f; // Intented underflow
                  {
                    ++bufferSize;
                    *(outBufferPtr++) = (w0 << 4);
                    ++encodedByte;
                    ++inBufferPtr;
                    // Increase nonRunLength 2 even latest value is invalid.
                    nonRunLength += 2;
                  }
                }
              }

              // Update (nonRunLength-1)/2 result into header.
              *(nonRunLengthHeaderPtr) |= (nonRunLength >> 1) << 4;
            }
          }
        }
      }

      // Allocate and copy data
      compressedBuffer = (uint8_t*)malloc(bufferSize);
      if(DALI_UNLIKELY(compressedBuffer == nullptr))
      {
        free(tempBuffer);
        return 0u;
      }
      outBufferData.isBufferOwned = true;

      memcpy(compressedBuffer, tempBuffer, bufferSize);
      free(tempBuffer);

      break;
    }
    default:
    {
      break;
    }
  }

  return bufferSize;
}

void FontClient::GlyphBufferData::Decompress(const GlyphBufferData& __restrict__ inBufferData, uint8_t* __restrict__ outBuffer)
{
  if(DALI_UNLIKELY(outBuffer == nullptr))
  {
    return;
  }

  switch(inBufferData.compressionType)
  {
    case TextAbstraction::FontClient::GlyphBufferData::CompressionType::NO_COMPRESSION:
    {
      const auto bufferSize = inBufferData.width * inBufferData.height * Pixel::GetBytesPerPixel(inBufferData.format);

      // Copy buffer without compress
      memcpy(outBuffer, inBufferData.buffer, bufferSize);
      break;
    }
    case TextAbstraction::FontClient::GlyphBufferData::CompressionType::BPP_4:
    {
      const uint32_t widthByte       = inBufferData.width * Pixel::GetBytesPerPixel(inBufferData.format);
      const uint32_t componentCount  = (widthByte >> 1);
      const bool     considerPadding = (widthByte & 1) ? true : false;

      uint8_t* __restrict__ outBufferPtr      = outBuffer;
      const uint8_t* __restrict__ inBufferPtr = inBufferData.buffer;

      // Compress for each line
      for(uint32_t y = 0; y < inBufferData.height; ++y)
      {
        for(uint32_t x = 0; x < componentCount; ++x)
        {
          const uint8_t v  = *(inBufferPtr++);
          const uint8_t v0 = (v >> 4) & 0x0f;
          const uint8_t v1 = v & 0x0f;

          *(outBufferPtr++) = (v0 << 4) | v0;
          *(outBufferPtr++) = (v1 << 4) | v1;
        }
        if(considerPadding)
        {
          const uint8_t v   = *(inBufferPtr++);
          *(outBufferPtr++) = (v << 4) | v;
        }
      }
      break;
    }
    case TextAbstraction::FontClient::GlyphBufferData::CompressionType::RLE_4:
    {
      const uint32_t widthByte = inBufferData.width * Pixel::GetBytesPerPixel(inBufferData.format);

      uint8_t* __restrict__ outBufferPtr      = outBuffer;
      const uint8_t* __restrict__ inBufferPtr = inBufferData.buffer;
      // Compress for each line
      for(uint32_t y = 0; y < inBufferData.height; ++y)
      {
        uint32_t x           = 0;
        uint32_t decodedByte = 0;
        while(decodedByte < widthByte)
        {
          const uint8_t v = *(inBufferPtr++);
          ++x;
          // Compress by RLE
          if(v & 0x80)
          {
            const uint8_t runLength = ((v >> 4) & 0x07) + 2u;
            decodedByte += runLength;
            const uint8_t repeatValue = v & 0x0f;
            for(uint8_t iter = 0; iter < runLength; ++iter)
            {
              const uint8_t prev0 = DALI_UNLIKELY(y == 0) ? 0 : (*(outBufferPtr - widthByte)) & 0x0f;
              const uint8_t v0    = (prev0 + repeatValue) & 0x0f;
              *(outBufferPtr++)   = (v0 << 4) | v0;
            }
          }
          // Not compress by RLE
          else
          {
            const uint8_t nonRunLength = (((v >> 4) & 0x07) << 1u) + 1u;
            decodedByte += nonRunLength;
            // First value.
            const uint8_t prev0 = DALI_UNLIKELY(y == 0) ? 0 : (*(outBufferPtr - widthByte)) & 0x0f;
            const uint8_t v0    = (prev0 + (v & 0x0f)) & 0x0f;
            *(outBufferPtr++)   = (v0 << 4) | v0;

            const bool ignoreLastValue = decodedByte > widthByte ? true : false;
            if(DALI_UNLIKELY(ignoreLastValue))
            {
              --decodedByte;
              for(uint8_t iter = 1; iter + 2 < nonRunLength; iter += 2)
              {
                const uint8_t w     = *(inBufferPtr++);
                const uint8_t prew0 = DALI_UNLIKELY(y == 0) ? 0 : (*(outBufferPtr - widthByte)) & 0x0f;
                const uint8_t w0    = (prew0 + ((w >> 4) & 0x0f)) & 0x0f;
                const uint8_t prew1 = DALI_UNLIKELY(y == 0) ? 0 : (*(outBufferPtr - widthByte + 1)) & 0x0f;
                const uint8_t w1    = (prew1 + (w & 0x0f)) & 0x0f;
                ++x;

                *(outBufferPtr++) = (w0 << 4) | w0;
                *(outBufferPtr++) = (w1 << 4) | w1;
              }
              // Last value.
              {
                const uint8_t w     = ((*(inBufferPtr++)) >> 4) & 0x0f;
                const uint8_t prew0 = DALI_UNLIKELY(y == 0) ? 0 : (*(outBufferPtr - widthByte)) & 0x0f;
                const uint8_t w0    = (prew0 + w) & 0x0f;
                ++x;

                *(outBufferPtr++) = (w0 << 4) | w0;
              }
            }
            else
            {
              for(uint8_t iter = 1; iter < nonRunLength; iter += 2)
              {
                const uint8_t w     = *(inBufferPtr++);
                const uint8_t prew0 = DALI_UNLIKELY(y == 0) ? 0 : (*(outBufferPtr - widthByte)) & 0x0f;
                const uint8_t w0    = (prew0 + ((w >> 4) & 0x0f)) & 0x0f;
                const uint8_t prew1 = DALI_UNLIKELY(y == 0) ? 0 : (*(outBufferPtr - widthByte + 1)) & 0x0f;
                const uint8_t w1    = (prew1 + (w & 0x0f)) & 0x0f;
                ++x;

                *(outBufferPtr++) = (w0 << 4) | w0;
                *(outBufferPtr++) = (w1 << 4) | w1;
              }
            }
          }
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }
}

void FontClient::GlyphBufferData::DecompressScanline(const GlyphBufferData& __restrict__ inBufferData, uint8_t* __restrict__ outBuffer, uint32_t& __restrict__ offset)
{
  switch(inBufferData.compressionType)
  {
    case TextAbstraction::FontClient::GlyphBufferData::CompressionType::NO_COMPRESSION:
    {
      const auto bufferSize = inBufferData.width * Pixel::GetBytesPerPixel(inBufferData.format);

      // Copy buffer without compress
      memcpy(outBuffer, inBufferData.buffer + offset, bufferSize);

      // Update offset
      offset += bufferSize;
      break;
    }
    case TextAbstraction::FontClient::GlyphBufferData::CompressionType::BPP_4:
    {
      const uint32_t widthByte       = inBufferData.width * Pixel::GetBytesPerPixel(inBufferData.format);
      const uint32_t componentCount  = (widthByte >> 1);
      const bool     considerPadding = (widthByte & 1) ? true : false;

      uint8_t* __restrict__ outBufferPtr      = outBuffer;
      const uint8_t* __restrict__ inBufferPtr = inBufferData.buffer + offset;

      // Decompress scanline
      for(uint32_t x = 0; x < componentCount; ++x)
      {
        const uint8_t v  = *(inBufferPtr++);
        const uint8_t v0 = (v >> 4) & 0x0f;
        const uint8_t v1 = v & 0x0f;

        *(outBufferPtr++) = (v0 << 4) | v0;
        *(outBufferPtr++) = (v1 << 4) | v1;
      }
      if(considerPadding)
      {
        const uint8_t v   = *(inBufferPtr++);
        *(outBufferPtr++) = (v << 4) | v;
      }

      // Update offset
      offset += (widthByte + 1u) >> 1u;
      break;
    }
    case TextAbstraction::FontClient::GlyphBufferData::CompressionType::RLE_4:
    {
      const uint32_t widthByte = inBufferData.width * Pixel::GetBytesPerPixel(inBufferData.format);

      uint8_t* __restrict__ outBufferPtr      = outBuffer;
      const uint8_t* __restrict__ inBufferPtr = inBufferData.buffer + offset;

      // If offset is zero, fill outBuffer as 0 first.
      if(DALI_UNLIKELY(offset == 0))
      {
        memset(outBufferPtr, 0, widthByte);
      }

      // Decompress scanline
      uint32_t decodedByte = 0;
      while(decodedByte < widthByte)
      {
        const uint8_t v = *(inBufferPtr++);
        ++offset;
        // Compress by RLE
        if(v & 0x80)
        {
          const uint8_t runLength = ((v >> 4) & 0x07) + 2u;
          decodedByte += runLength;
          const uint8_t repeatValue = (v & 0x0f);
          for(uint8_t iter = 0; iter < runLength; ++iter)
          {
            const uint8_t prev0 = (*(outBufferPtr)) & 0x0f;
            const uint8_t v0    = (prev0 + repeatValue) & 0x0f;
            *(outBufferPtr++)   = (v0 << 4) | v0;
          }
        }
        // Not compress by RLE
        else
        {
          const uint8_t nonRunLength = (((v >> 4) & 0x07) << 1u) + 1u;
          decodedByte += nonRunLength;
          // First value.
          const uint8_t prev0 = (*(outBufferPtr)) & 0x0f;
          const uint8_t v0    = (prev0 + (v & 0x0f)) & 0x0f;
          *(outBufferPtr++)   = (v0 << 4) | v0;

          const bool ignoreLastValue = decodedByte > widthByte ? true : false;
          if(DALI_UNLIKELY(ignoreLastValue))
          {
            --decodedByte;
            for(uint8_t iter = 1; iter + 2 < nonRunLength; iter += 2)
            {
              const uint8_t w     = *(inBufferPtr++);
              const uint8_t prew0 = (*(outBufferPtr)) & 0x0f;
              const uint8_t w0    = (prew0 + ((w >> 4) & 0x0f)) & 0x0f;
              const uint8_t prew1 = (*(outBufferPtr + 1)) & 0x0f;
              const uint8_t w1    = (prew1 + (w & 0x0f)) & 0x0f;
              ++offset;

              *(outBufferPtr++) = (w0 << 4) | w0;
              *(outBufferPtr++) = (w1 << 4) | w1;
            }
            // Last value.
            {
              const uint8_t w     = ((*(inBufferPtr++)) >> 4) & 0x0f;
              const uint8_t prew0 = (*(outBufferPtr)) & 0x0f;
              const uint8_t w0    = (prew0 + w) & 0x0f;
              ++offset;

              *(outBufferPtr++) = (w0 << 4) | w0;
            }
          }
          else
          {
            for(uint8_t iter = 1; iter < nonRunLength; iter += 2)
            {
              const uint8_t w     = *(inBufferPtr++);
              const uint8_t prew0 = (*(outBufferPtr)) & 0x0f;
              const uint8_t w0    = (prew0 + ((w >> 4) & 0x0f)) & 0x0f;
              const uint8_t prew1 = (*(outBufferPtr + 1)) & 0x0f;
              const uint8_t w1    = (prew1 + (w & 0x0f)) & 0x0f;
              ++offset;

              *(outBufferPtr++) = (w0 << 4) | w0;
              *(outBufferPtr++) = (w1 << 4) | w1;
            }
          }
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }
}

// FontClient

FontClient FontClient::Get()
{
  return Internal::FontClient::Get();
}

FontClient::FontClient()
{
}

FontClient::~FontClient()
{
}

FontClient::FontClient(const FontClient& handle) = default;

FontClient& FontClient::operator=(const FontClient& handle) = default;

FontClient::FontClient(FontClient&& handle) = default;

FontClient& FontClient::operator=(FontClient&& handle) = default;

void FontClient::ClearCache()
{
  GetImplementation(*this).ClearCache();
}

void FontClient::SetDpi(unsigned int horizontalDpi, unsigned int verticalDpi)
{
  GetImplementation(*this).SetDpi(horizontalDpi, verticalDpi);
}

void FontClient::GetDpi(unsigned int& horizontalDpi, unsigned int& verticalDpi)
{
  GetImplementation(*this).GetDpi(horizontalDpi, verticalDpi);
}

int FontClient::GetDefaultFontSize()
{
  return GetImplementation(*this).GetDefaultFontSize();
}

void FontClient::ResetSystemDefaults()
{
  GetImplementation(*this).ResetSystemDefaults();
}

void FontClient::GetDefaultFonts(FontList& defaultFonts)
{
  GetImplementation(*this).GetDefaultFonts(defaultFonts);
}

void FontClient::InitDefaultFontDescription()
{
  GetImplementation(*this).InitDefaultFontDescription();
}

void FontClient::GetDefaultPlatformFontDescription(FontDescription& fontDescription)
{
  GetImplementation(*this).GetDefaultPlatformFontDescription(fontDescription);
}

void FontClient::GetSystemFonts(FontList& systemFonts)
{
  GetImplementation(*this).GetSystemFonts(systemFonts);
}

void FontClient::GetDescription(FontId fontId, FontDescription& fontDescription)
{
  GetImplementation(*this).GetDescription(fontId, fontDescription);
}

PointSize26Dot6 FontClient::GetPointSize(FontId fontId)
{
  return GetImplementation(*this).GetPointSize(fontId);
}

bool FontClient::IsCharacterSupportedByFont(FontId fontId, Character character)
{
  return GetImplementation(*this).IsCharacterSupportedByFont(fontId, character);
}

FontId FontClient::FindDefaultFont(Character       charcode,
                                   PointSize26Dot6 requestedPointSize,
                                   bool            preferColor)
{
  return GetImplementation(*this).FindDefaultFont(charcode,
                                                  requestedPointSize,
                                                  preferColor);
}

FontId FontClient::FindFallbackFont(Character              charcode,
                                    const FontDescription& preferredFontDescription,
                                    PointSize26Dot6        requestedPointSize,
                                    bool                   preferColor)
{
  return GetImplementation(*this).FindFallbackFont(charcode, preferredFontDescription, requestedPointSize, preferColor);
}

FontId FontClient::GetFontId(const FontPath& path, PointSize26Dot6 requestedPointSize, FaceIndex faceIndex)
{
  return GetImplementation(*this).GetFontId(path, requestedPointSize, faceIndex);
}

FontId FontClient::GetFontId(const FontDescription& fontDescription,
                             PointSize26Dot6        requestedPointSize,
                             FaceIndex              faceIndex)
{
  return GetImplementation(*this).GetFontId(fontDescription,
                                            requestedPointSize,
                                            faceIndex);
}

FontId FontClient::GetFontId(const BitmapFont& bitmapFont)
{
  return GetImplementation(*this).GetFontId(bitmapFont);
}

bool FontClient::IsScalable(const FontPath& path)
{
  return GetImplementation(*this).IsScalable(path);
}

bool FontClient::IsScalable(const FontDescription& fontDescription)
{
  return GetImplementation(*this).IsScalable(fontDescription);
}

void FontClient::GetFixedSizes(const FontPath& path, Dali::Vector<PointSize26Dot6>& sizes)
{
  GetImplementation(*this).GetFixedSizes(path, sizes);
}

void FontClient::GetFixedSizes(const FontDescription&         fontDescription,
                               Dali::Vector<PointSize26Dot6>& sizes)
{
  GetImplementation(*this).GetFixedSizes(fontDescription, sizes);
}

bool FontClient::HasItalicStyle(FontId fontId) const
{
  return GetImplementation(*this).HasItalicStyle(fontId);
}

void FontClient::GetFontMetrics(FontId fontId, FontMetrics& metrics)
{
  GetImplementation(*this).GetFontMetrics(fontId, metrics);
}

GlyphIndex FontClient::GetGlyphIndex(FontId fontId, Character charcode)
{
  return GetImplementation(*this).GetGlyphIndex(fontId, charcode);
}

GlyphIndex FontClient::GetGlyphIndex(FontId fontId, Character charcode, Character variantSelector)
{
  return GetImplementation(*this).GetGlyphIndex(fontId, charcode, variantSelector);
}

bool FontClient::GetGlyphMetrics(GlyphInfo* array, uint32_t size, GlyphType type, bool horizontal)
{
  return GetImplementation(*this).GetGlyphMetrics(array, size, type, horizontal);
}

void FontClient::CreateBitmap(FontId fontId, GlyphIndex glyphIndex, bool isItalicRequired, bool isBoldRequired, GlyphBufferData& data, int outlineWidth)
{
  GetImplementation(*this).CreateBitmap(fontId, glyphIndex, isItalicRequired, isBoldRequired, data, outlineWidth);
}

PixelData FontClient::CreateBitmap(FontId fontId, GlyphIndex glyphIndex, int outlineWidth)
{
  return GetImplementation(*this).CreateBitmap(fontId, glyphIndex, outlineWidth);
}

void FontClient::CreateVectorBlob(FontId fontId, GlyphIndex glyphIndex, VectorBlob*& blob, unsigned int& blobLength, unsigned int& nominalWidth, unsigned int& nominalHeight)
{
  GetImplementation(*this).CreateVectorBlob(fontId, glyphIndex, blob, blobLength, nominalWidth, nominalHeight);
}

const GlyphInfo& FontClient::GetEllipsisGlyph(PointSize26Dot6 requestedPointSize)
{
  return GetImplementation(*this).GetEllipsisGlyph(requestedPointSize);
}

bool FontClient::IsColorGlyph(FontId fontId, GlyphIndex glyphIndex)
{
  return GetImplementation(*this).IsColorGlyph(fontId, glyphIndex);
}

bool FontClient::AddCustomFontDirectory(const FontPath& path)
{
  return GetImplementation(*this).AddCustomFontDirectory(path);
}

GlyphIndex FontClient::CreateEmbeddedItem(const EmbeddedItemDescription& description, Pixel::Format& pixelFormat)
{
  return GetImplementation(*this).CreateEmbeddedItem(description, pixelFormat);
}

void FontClient::EnableAtlasLimitation(bool enabled)
{
  return GetImplementation(*this).EnableAtlasLimitation(enabled);
}

bool FontClient::IsAtlasLimitationEnabled() const
{
  return GetImplementation(*this).IsAtlasLimitationEnabled();
}

Size FontClient::GetMaximumTextAtlasSize() const
{
  return GetImplementation(*this).GetMaximumTextAtlasSize();
}

Size FontClient::GetDefaultTextAtlasSize() const
{
  return GetImplementation(*this).GetDefaultTextAtlasSize();
}

Size FontClient::GetCurrentMaximumBlockSizeFitInAtlas() const
{
  return GetImplementation(*this).GetCurrentMaximumBlockSizeFitInAtlas();
}

bool FontClient::SetCurrentMaximumBlockSizeFitInAtlas(const Size& currentMaximumBlockSizeFitInAtlas)
{
  return GetImplementation(*this).SetCurrentMaximumBlockSizeFitInAtlas(currentMaximumBlockSizeFitInAtlas);
}

uint32_t FontClient::GetNumberOfPointsPerOneUnitOfPointSize() const
{
  return GetImplementation(*this).GetNumberOfPointsPerOneUnitOfPointSize();
}

FontClient::FontClient(Internal::FontClient* internal)
: BaseHandle(internal)
{
}

FontClient FontClientPreInitialize()
{
  return Internal::FontClient::PreInitialize();
}

void FontClientPreCache(const FontFamilyList& fallbackFamilyList, const FontFamilyList& extraFamilyList, const FontFamily& localeFamily, bool useThread, bool syncCreation)
{
  Internal::FontClient::PreCache(fallbackFamilyList, extraFamilyList, localeFamily, useThread, syncCreation);
}

void FontClientFontPreLoad(const FontPathList& fontPathList, const FontPathList& memoryFontPathList, bool useThread, bool syncCreation)
{
  Internal::FontClient::PreLoad(fontPathList, memoryFontPathList, useThread, syncCreation);
}

void FontClientJoinFontThreads()
{
  Internal::FontClient::JoinFontThreads();
}

} // namespace TextAbstraction

} // namespace Dali
