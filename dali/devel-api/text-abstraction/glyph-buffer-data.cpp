/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/text-abstraction/glyph-buffer-data.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/image-operations.h>

namespace Dali
{
namespace TextAbstraction
{
GlyphBufferData::GlyphBufferData()
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

GlyphBufferData::~GlyphBufferData()
{
  if(isBufferOwned)
  {
    free(buffer);
  }
}

GlyphBufferData::GlyphBufferData(GlyphBufferData&& rhs) noexcept
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

GlyphBufferData& GlyphBufferData::operator=(GlyphBufferData&& rhs) noexcept
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

size_t GlyphBufferData::Compress(const uint8_t* const __restrict__ inBuffer, GlyphBufferData& __restrict__ outBufferData)
{
  size_t bufferSize                       = 0u;
  uint8_t*& __restrict__ compressedBuffer = outBufferData.buffer;
  switch(outBufferData.compressionType)
  {
    case TextAbstraction::GlyphBufferData::CompressionType::NO_COMPRESSION:
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
    case TextAbstraction::GlyphBufferData::CompressionType::BPP_4:
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
    case TextAbstraction::GlyphBufferData::CompressionType::RLE_4:
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

void GlyphBufferData::Decompress(const GlyphBufferData& __restrict__ inBufferData, uint8_t* __restrict__ outBuffer)
{
  if(DALI_UNLIKELY(outBuffer == nullptr))
  {
    return;
  }

  switch(inBufferData.compressionType)
  {
    case TextAbstraction::GlyphBufferData::CompressionType::NO_COMPRESSION:
    {
      const auto bufferSize = inBufferData.width * inBufferData.height * Pixel::GetBytesPerPixel(inBufferData.format);

      // Copy buffer without compress
      memcpy(outBuffer, inBufferData.buffer, bufferSize);
      break;
    }
    case TextAbstraction::GlyphBufferData::CompressionType::BPP_4:
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
    case TextAbstraction::GlyphBufferData::CompressionType::RLE_4:
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

void GlyphBufferData::DecompressScanline(const GlyphBufferData& __restrict__ inBufferData, uint8_t* __restrict__ outBuffer, uint32_t& __restrict__ offset)
{
  switch(inBufferData.compressionType)
  {
    case TextAbstraction::GlyphBufferData::CompressionType::NO_COMPRESSION:
    {
      const auto bufferSize = inBufferData.width * Pixel::GetBytesPerPixel(inBufferData.format);

      // Copy buffer without compress
      memcpy(outBuffer, inBufferData.buffer + offset, bufferSize);

      // Update offset
      offset += bufferSize;
      break;
    }
    case TextAbstraction::GlyphBufferData::CompressionType::BPP_4:
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
    case TextAbstraction::GlyphBufferData::CompressionType::RLE_4:
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

} // namespace TextAbstraction

} // namespace Dali
