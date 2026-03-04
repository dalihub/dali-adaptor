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

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-common.h>
#include <cmath>
#include <new> ///< for std::bad_alloc

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/gaussian-blur.h>
#include <dali/internal/imaging/common/pixel-buffer-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{

/**
 * Helper to finalize memory automatically even if throw exception.
 */
template<typename T>
struct MemoryFinalizer
{
  MemoryFinalizer(T*& array)
  : mArray(array)
  {
  }
  ~MemoryFinalizer()
  {
    if(DALI_LIKELY(mArray))
    {
      delete[] mArray;
    }
  }

  T*& mArray;
};

/**
 * Perform a one dimension Gaussian blur convolution and write its output buffer transposed.
 *
 * @param[in] inBuffer The input buffer with the source image
 * @param[in] outBuffer The output buffer with the Gaussian blur applied and transposed
 * @param[in] bufferWidth The width of the buffer
 * @param[in] bufferHeight The height of the buffer
 * @param[in] inBufferStrideBytes The stride byte of input buffer
 * @param[in] outBufferStrideBytes The stride byte of output buffer
 * @param[in] inBytesPerPixel The bytes per pixels of input buffer
 * @param[in] outBytesPerPixel The bytes per pixels of output buffer
 * @param[in] blurRadius The radius for Gaussian blur
 *
 * @return @e false if convolute fails (invalid pixel format or memory issues).
 */
bool ConvoluteAndTranspose(uint8_t*       inBuffer,
                           uint8_t*       outBuffer,
                           const uint32_t bufferWidth,
                           const uint32_t bufferHeight,
                           const uint32_t inBufferStrideBytes,
                           const uint32_t outBufferStrideBytes,
                           const uint32_t inBytesPerPixel,
                           const uint32_t outBytesPerPixel,
                           const float    blurRadius)
{
  if(DALI_UNLIKELY(inBytesPerPixel != outBytesPerPixel))
  {
    DALI_LOG_ERROR("Invalid operation!\n");
    return false;
  }

  // Calculate the weights for gaussian blur
  int32_t radius = static_cast<int32_t>(std::ceil(blurRadius));
  if(DALI_UNLIKELY(radius < 0))
  {
    DALI_LOG_ERROR("Blur radius could not be negative!\n");
    return false;
  }

  uint32_t rows = static_cast<uint32_t>(radius) * 2u + 1u;

  const float sigma        = (blurRadius < Math::MACHINE_EPSILON_1) ? 0.0f : blurRadius * 0.4f + 0.6f; // The same equation used by Android
  const float sigma22      = 2.0f * sigma * sigma;
  const float sqrtSigmaPi2 = std::sqrt(2.0f * Math::PI) * sigma;

  float normalizeFactor = 0.0f;

  float* weightMatrix = nullptr;
  float* channelSum   = nullptr;

  try
  {
    // Automatically delete memory array
    MemoryFinalizer weightMatrixFinalizer(weightMatrix);
    MemoryFinalizer channelSumFinalizer(channelSum);

    if(DALI_UNLIKELY(radius <= 0 || sigma22 < Math::MACHINE_EPSILON_1 || sqrtSigmaPi2 < Math::MACHINE_EPSILON_1))
    {
      rows            = 1u;
      weightMatrix    = new float[rows];
      weightMatrix[0] = 1.0f;
      normalizeFactor = 1.0f;
    }
    else
    {
      weightMatrix        = new float[rows];
      const float radius2 = radius * radius;

      int32_t index = 0;
      for(int32_t row = -radius; row <= radius; row++)
      {
        const float distance = static_cast<float>(row) * static_cast<float>(row);
        if(distance > radius2)
        {
          weightMatrix[index] = 0.0f;
        }
        else
        {
          weightMatrix[index] = static_cast<float>(std::exp(-(distance) / sigma22) / sqrtSigmaPi2);
        }
        normalizeFactor += weightMatrix[index];
        index++;
      }

      if(DALI_UNLIKELY(normalizeFactor < Math::MACHINE_EPSILON_1))
      {
        DALI_LOG_ERROR("Blur radius is too small!\n");
        return false;
      }

      for(uint32_t i = 0; i < rows; i++)
      {
        weightMatrix[i] /= normalizeFactor;
      }
    }

    channelSum = new float[inBytesPerPixel];

    // Perform the convolution and transposition using the weights
    const int32_t columns  = static_cast<int32_t>(rows);
    const int32_t columns2 = columns / 2; // == radius
    for(uint32_t y = 0; y < bufferHeight; y++)
    {
      uint32_t       targetPixelByte = y * outBytesPerPixel;
      const uint32_t ioffset         = y * inBufferStrideBytes;
      for(uint32_t x = 0; x < bufferWidth; x++)
      {
        for(uint32_t i = 0; i < inBytesPerPixel; ++i)
        {
          channelSum[i] = 0.0f;
        }
        const int32_t weightColumnOffset = columns2;
        for(int32_t column = -columns2; column <= columns2; column++)
        {
          const float weight = weightMatrix[static_cast<uint32_t>(weightColumnOffset + column)];
          if(fabsf(weight) > Math::MACHINE_EPSILON_1)
          {
            int32_t ix = static_cast<int32_t>(x) + column;

            // Mirror Repeat
            ix %= (2 * static_cast<int32_t>(bufferWidth));
            if(ix < 0)
            {
              ix += 2 * static_cast<int32_t>(bufferWidth);
            }
            ix = (ix < static_cast<int32_t>(bufferWidth)) ? (ix) : (2 * static_cast<int32_t>(bufferWidth) - ix - 1);

            DALI_ASSERT_DEBUG(ix >= 0 && ix < static_cast<int32_t>(bufferWidth));

            const uint32_t sourcePixelByte = ioffset + (static_cast<uint32_t>(ix) * inBytesPerPixel);
            for(uint32_t i = 0; i < inBytesPerPixel; ++i)
            {
              channelSum[i] += weight * static_cast<float>(inBuffer[sourcePixelByte + i]);
            }
          }
        }

        for(uint32_t i = 0; i < inBytesPerPixel; ++i)
        {
          outBuffer[targetPixelByte + i] = static_cast<uint8_t>(std::min(static_cast<uint32_t>(std::max(0, static_cast<int32_t>(channelSum[i] + 0.5f))), 255u));
        }

        targetPixelByte += outBufferStrideBytes;
      }
    }
  }
  catch(Dali::DaliException& e)
  {
    DALI_LOG_ERROR("DaliException! Assertion %s failed at %s\n", e.condition, e.location);
    return false;
  }
  catch(const std::bad_alloc& e)
  {
    DALI_LOG_ERROR("Could not allocate temporary memory. (%u byte) e.what() : %s\n", (inBytesPerPixel + rows), e.what());
    return false;
  }

  return true;
}
} // namespace

bool PerformGaussianBlur(PixelBuffer& buffer, const float blurRadius)
{
  if(DALI_UNLIKELY(blurRadius < 0.0f))
  {
    DALI_LOG_ERROR("Blur radius could not be negative!\n");
    return false;
  }

  if(DALI_UNLIKELY(blurRadius < Math::MACHINE_EPSILON_1))
  {
    // If blur radius is exactly 0.0f, just skip operation and return as true.
    return true;
  }

  const uint32_t bufferWidth       = buffer.GetWidth();
  const uint32_t bufferHeight      = buffer.GetHeight();
  const uint32_t bufferStrideBytes = buffer.GetStrideBytes();

  const Pixel::Format bufferPixelFormat = buffer.GetPixelFormat();
  const uint32_t      bytesPerPixel     = Dali::Pixel::GetBytesPerPixel(bufferPixelFormat);

  if(bufferWidth == 0 || bufferHeight == 0 || bufferStrideBytes == 0 || bytesPerPixel == 0)
  {
    DALI_LOG_ERROR("Invalid buffer!\n");
    return false;
  }

  // Create a temporary buffer for the two-pass blur
  // On leaving scope, softShadowImageBuffer will get destroyed.
  PixelBufferPtr softShadowImageBuffer = PixelBuffer::New(bufferHeight, bufferWidth, bufferPixelFormat);

  // We perform the blur first but write its output image buffer transposed, so that we
  // can just do it in two passes. The first pass blurs horizontally and transposes, the
  // second pass does the same, but as the image is now transposed, it's really doing a
  // vertical blur. The second transposition makes the image the right way up again. This
  // is much faster than doing a 2D convolution.
  if(DALI_UNLIKELY(!ConvoluteAndTranspose(buffer.GetBuffer(), softShadowImageBuffer->GetBuffer(), bufferWidth, bufferHeight, bufferStrideBytes, softShadowImageBuffer->GetStrideBytes(), bytesPerPixel, bytesPerPixel, blurRadius)))
  {
    return false;
  }
  if(DALI_UNLIKELY(!ConvoluteAndTranspose(softShadowImageBuffer->GetBuffer(), buffer.GetBuffer(), bufferHeight, bufferWidth, softShadowImageBuffer->GetStrideBytes(), bufferStrideBytes, bytesPerPixel, bytesPerPixel, blurRadius)))
  {
    return false;
  }

  return true;
}

} //namespace Adaptor

} // namespace Internal

} // namespace Dali
