/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

#include <dali/internal/imaging/common/image-operations.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/image-loading.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/math/vector2.h>
#include <stddef.h>
#include <third-party/resampler/resampler.h>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>

// INTERNAL INCLUDES

namespace Dali
{
namespace Internal
{
namespace Platform
{
namespace
{
// The BORDER_FILL_VALUE is a single byte value that is used for horizontal and vertical borders.
// A value of 0x00 gives us transparency for pixel buffers with an alpha channel, or black otherwise.
// We can optionally use a Vector4 color here, but at reduced fill speed.
const uint8_t BORDER_FILL_VALUE(0x00);
// A maximum size limit for newly created bitmaps. ( 1u << 16 ) - 1 is chosen as we are using 16bit words for dimensions.
const uint32_t MAXIMUM_TARGET_BITMAP_SIZE((1u << 16) - 1);

// Constants used by the ImageResampler.
const float DEFAULT_SOURCE_GAMMA = 1.75f; ///< Default source gamma value used in the Resampler() function. Partial gamma correction looks better on mips. Set to 1.0 to disable gamma correction.
const float FILTER_SCALE         = 1.f;   ///< Default filter scale value used in the Resampler() function. Filter scale - values < 1.0 cause aliasing, but create sharper looking mips.

const float RAD_135 = Math::PI_2 + Math::PI_4; ///< 135 degrees in radians;
const float RAD_225 = RAD_135 + Math::PI_2;    ///< 225 degrees in radians;
const float RAD_270 = 3.f * Math::PI_2;        ///< 270 degrees in radians;
const float RAD_315 = RAD_225 + Math::PI_2;    ///< 315 degrees in radians;

using Integration::Bitmap;
using Integration::BitmapPtr;
typedef uint8_t PixelBuffer;

/**
 * @brief 4 byte pixel structure.
 */
struct Pixel4Bytes
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} __attribute__((packed, aligned(4))); //< Tell the compiler it is okay to use a single 32 bit load.

/**
 * @brief RGB888 pixel structure.
 */
struct Pixel3Bytes
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
} __attribute__((packed, aligned(1)));

/**
 * @brief RGB565 pixel typedefed from a short.
 *
 * Access fields by manual shifting and masking.
 */
typedef uint16_t PixelRGB565;

/**
 * @brief a Pixel composed of two independent byte components.
 */
struct Pixel2Bytes
{
  uint8_t l;
  uint8_t a;
} __attribute__((packed, aligned(2))); //< Tell the compiler it is okay to use a single 16 bit load.

#if defined(DEBUG_ENABLED)
/**
 * Disable logging of image operations or make it verbose from the commandline
 * as follows (e.g., for dali demo app):
 * <code>
 * LOG_IMAGE_OPERATIONS=0 dali-demo #< off
 * LOG_IMAGE_OPERATIONS=3 dali-demo #< on, verbose
 * </code>
 */
Debug::Filter* gImageOpsLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_IMAGE_OPERATIONS");
#endif

/** @return The greatest even number less than or equal to the argument. */
inline uint32_t EvenDown(const uint32_t a)
{
  const uint32_t evened = a & ~1u;
  return evened;
}

/**
 * @brief Log bad parameters.
 */
void ValidateScalingParameters(const uint32_t inputWidth,
                               const uint32_t inputHeight,
                               const uint32_t desiredWidth,
                               const uint32_t desiredHeight)
{
  if(desiredWidth > inputWidth || desiredHeight > inputHeight)
  {
    DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "Upscaling not supported (%u, %u -> %u, %u).\n", inputWidth, inputHeight, desiredWidth, desiredHeight);
  }

  if(desiredWidth == 0u || desiredHeight == 0u)
  {
    DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "Downscaling to a zero-area target is pointless.\n");
  }

  if(inputWidth == 0u || inputHeight == 0u)
  {
    DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "Zero area images cannot be scaled\n");
  }
}

/**
 * @brief Do debug assertions common to all scanline halving functions.
 * @note Inline and in anon namespace so should boil away in release builds.
 */
inline void DebugAssertScanlineParameters(const uint8_t* const pixels, const uint32_t width)
{
  DALI_ASSERT_DEBUG(pixels && "Null pointer.");
  DALI_ASSERT_DEBUG(width > 1u && "Can't average fewer than two pixels.");
  DALI_ASSERT_DEBUG(width < 131072u && "Unusually wide image: are you sure you meant to pass that value in?");
}

/**
 * @brief Assertions on params to functions averaging pairs of scanlines.
 * @note Inline as intended to boil away in release.
 */
inline void DebugAssertDualScanlineParameters(const uint8_t* const scanline1,
                                              const uint8_t* const scanline2,
                                              uint8_t* const       outputScanline,
                                              const size_t         widthInComponents)
{
  DALI_ASSERT_DEBUG(scanline1 && "Null pointer.");
  DALI_ASSERT_DEBUG(scanline2 && "Null pointer.");
  DALI_ASSERT_DEBUG(outputScanline && "Null pointer.");
  DALI_ASSERT_DEBUG(((scanline1 >= scanline2 + widthInComponents) || (scanline2 >= scanline1 + widthInComponents)) && "Scanlines alias.");
  DALI_ASSERT_DEBUG(((outputScanline >= (scanline2 + widthInComponents)) || (scanline2 >= (scanline1 + widthInComponents))) && "Scanline 2 aliases output.");
}

/**
 * @brief Converts a scaling mode to the definition of which dimensions matter when box filtering as a part of that mode.
 */
BoxDimensionTest DimensionTestForScalingMode(FittingMode::Type fittingMode)
{
  BoxDimensionTest dimensionTest;
  dimensionTest = BoxDimensionTestEither;

  switch(fittingMode)
  {
    // Shrink to fit attempts to make one or zero dimensions smaller than the
    // desired dimensions and one or two dimensions exactly the same as the desired
    // ones, so as long as one dimension is larger than the desired size, box
    // filtering can continue even if the second dimension is smaller than the
    // desired dimensions:
    case FittingMode::SHRINK_TO_FIT:
    {
      dimensionTest = BoxDimensionTestEither;
      break;
    }
    // Scale to fill mode keeps both dimensions at least as large as desired:
    case FittingMode::SCALE_TO_FILL:
    {
      dimensionTest = BoxDimensionTestBoth;
      break;
    }
    // Y dimension is irrelevant when downscaling in FIT_WIDTH mode:
    case FittingMode::FIT_WIDTH:
    {
      dimensionTest = BoxDimensionTestX;
      break;
    }
    // X Dimension is ignored by definition in FIT_HEIGHT mode:
    case FittingMode::FIT_HEIGHT:
    {
      dimensionTest = BoxDimensionTestY;
      break;
    }
  }

  return dimensionTest;
}

/**
 * @brief Work out the dimensions for a uniform scaling of the input to map it
 * into the target while effecting ShinkToFit scaling mode.
 */
ImageDimensions FitForShrinkToFit(ImageDimensions target, ImageDimensions source)
{
  // Scale the input by the least extreme of the two dimensions:
  const float widthScale  = target.GetX() / float(source.GetX());
  const float heightScale = target.GetY() / float(source.GetY());
  const float scale       = widthScale < heightScale ? widthScale : heightScale;

  // Do no scaling at all if the result would increase area:
  if(scale >= 1.0f)
  {
    return source;
  }

  return ImageDimensions(source.GetX() * scale + 0.5f, source.GetY() * scale + 0.5f);
}

/**
 * @brief Work out the dimensions for a uniform scaling of the input to map it
 * into the target while effecting SCALE_TO_FILL scaling mode.
 * @note An image scaled into the output dimensions will need either top and
 * bottom or left and right to be cropped away unless the source was pre-cropped
 * to match the destination aspect ratio.
 */
ImageDimensions FitForScaleToFill(ImageDimensions target, ImageDimensions source)
{
  DALI_ASSERT_DEBUG(source.GetX() > 0 && source.GetY() > 0 && "Zero-area rectangles should not be passed-in");
  // Scale the input by the least extreme of the two dimensions:
  const float widthScale  = target.GetX() / float(source.GetX());
  const float heightScale = target.GetY() / float(source.GetY());
  const float scale       = widthScale > heightScale ? widthScale : heightScale;

  // Do no scaling at all if the result would increase area:
  if(scale >= 1.0f)
  {
    return source;
  }

  return ImageDimensions(source.GetX() * scale + 0.5f, source.GetY() * scale + 0.5f);
}

/**
 * @brief Work out the dimensions for a uniform scaling of the input to map it
 * into the target while effecting FIT_WIDTH scaling mode.
 */
ImageDimensions FitForFitWidth(ImageDimensions target, ImageDimensions source)
{
  DALI_ASSERT_DEBUG(source.GetX() > 0 && "Cant fit a zero-dimension rectangle.");
  const float scale = target.GetX() / float(source.GetX());

  // Do no scaling at all if the result would increase area:
  if(scale >= 1.0f)
  {
    return source;
  }
  return ImageDimensions(source.GetX() * scale + 0.5f, source.GetY() * scale + 0.5f);
}

/**
 * @brief Work out the dimensions for a uniform scaling of the input to map it
 * into the target while effecting FIT_HEIGHT scaling mode.
 */
ImageDimensions FitForFitHeight(ImageDimensions target, ImageDimensions source)
{
  DALI_ASSERT_DEBUG(source.GetY() > 0 && "Cant fit a zero-dimension rectangle.");
  const float scale = target.GetY() / float(source.GetY());

  // Do no scaling at all if the result would increase area:
  if(scale >= 1.0f)
  {
    return source;
  }

  return ImageDimensions(source.GetX() * scale + 0.5f, source.GetY() * scale + 0.5f);
}

/**
 * @brief Generate the rectangle to use as the target of a pixel sampling pass
 * (e.g., nearest or linear).
 */
ImageDimensions FitToScalingMode(ImageDimensions requestedSize, ImageDimensions sourceSize, FittingMode::Type fittingMode)
{
  ImageDimensions fitDimensions;
  switch(fittingMode)
  {
    case FittingMode::SHRINK_TO_FIT:
    {
      fitDimensions = FitForShrinkToFit(requestedSize, sourceSize);
      break;
    }
    case FittingMode::SCALE_TO_FILL:
    {
      fitDimensions = FitForScaleToFill(requestedSize, sourceSize);
      break;
    }
    case FittingMode::FIT_WIDTH:
    {
      fitDimensions = FitForFitWidth(requestedSize, sourceSize);
      break;
    }
    case FittingMode::FIT_HEIGHT:
    {
      fitDimensions = FitForFitHeight(requestedSize, sourceSize);
      break;
    }
  }

  return fitDimensions;
}

/**
 * @brief Calculate the number of lines on the X and Y axis that need to be
 * either added or removed with repect to the specified fitting mode.
 * (e.g., nearest or linear).
 * @param[in]     sourceSize      The size of the source image
 * @param[in]     fittingMode     The fitting mode to use
 * @param[in/out] requestedSize   The target size that the image will be fitted to.
 *                                If the source image is smaller than the requested size, the source is not scaled up.
 *                                So we reduce the target size while keeping aspect by lowering resolution.
 * @param[out]    scanlinesToCrop The number of scanlines to remove from the image (can be negative to represent Y borders required)
 * @param[out]    columnsToCrop   The number of columns to remove from the image (can be negative to represent X borders required)
 */
void CalculateBordersFromFittingMode(ImageDimensions sourceSize, FittingMode::Type fittingMode, ImageDimensions& requestedSize, int& scanlinesToCrop, int& columnsToCrop)
{
  const int   sourceWidth(static_cast<int>(sourceSize.GetWidth()));
  const int   sourceHeight(static_cast<int>(sourceSize.GetHeight()));
  const float targetAspect(static_cast<float>(requestedSize.GetWidth()) / static_cast<float>(requestedSize.GetHeight()));
  int         finalWidth  = 0;
  int         finalHeight = 0;

  switch(fittingMode)
  {
    case FittingMode::FIT_WIDTH:
    {
      finalWidth  = sourceWidth;
      finalHeight = static_cast<int>(static_cast<float>(sourceWidth) / targetAspect);
      break;
    }

    case FittingMode::FIT_HEIGHT:
    {
      finalWidth  = static_cast<int>(static_cast<float>(sourceHeight) * targetAspect);
      finalHeight = sourceHeight;
      break;
    }

    case FittingMode::SHRINK_TO_FIT:
    {
      const float sourceAspect(static_cast<float>(sourceWidth) / static_cast<float>(sourceHeight));
      if(sourceAspect > targetAspect)
      {
        finalWidth  = sourceWidth;
        finalHeight = static_cast<int>(static_cast<float>(sourceWidth) / targetAspect);
      }
      else
      {
        finalWidth  = static_cast<int>(static_cast<float>(sourceHeight) * targetAspect);
        finalHeight = sourceHeight;
      }
      break;
    }

    case FittingMode::SCALE_TO_FILL:
    {
      const float sourceAspect(static_cast<float>(sourceWidth) / static_cast<float>(sourceHeight));
      if(sourceAspect > targetAspect)
      {
        finalWidth  = static_cast<int>(static_cast<float>(sourceHeight) * targetAspect);
        finalHeight = sourceHeight;
      }
      else
      {
        finalWidth  = sourceWidth;
        finalHeight = static_cast<int>(static_cast<float>(sourceWidth) / targetAspect);
      }
      break;
    }
  }

  // Clamp if overflowed
  if(DALI_UNLIKELY(finalWidth > std::numeric_limits<uint16_t>::max()))
  {
    finalWidth = std::numeric_limits<uint16_t>::max();
  }
  if(DALI_UNLIKELY(finalHeight > std::numeric_limits<uint16_t>::max()))
  {
    finalHeight = std::numeric_limits<uint16_t>::max();
  }

  columnsToCrop   = -(finalWidth - sourceWidth);
  scanlinesToCrop = -(finalHeight - sourceHeight);

  requestedSize.SetWidth(static_cast<uint16_t>(finalWidth));
  requestedSize.SetHeight(static_cast<uint16_t>(finalHeight));
}

/**
 * @brief Construct a pixel buffer object from a copy of the pixel array passed in.
 */
Dali::Devel::PixelBuffer MakePixelBuffer(const uint8_t* const pixels, Pixel::Format pixelFormat, uint32_t width, uint32_t height)
{
  DALI_ASSERT_DEBUG(pixels && "Null bitmap buffer to copy.");

  // Allocate a pixel buffer to hold the image passed in:
  auto newBitmap = Dali::Devel::PixelBuffer::New(width, height, pixelFormat);

  // Copy over the pixels from the downscaled image that was generated in-place in the pixel buffer of the input bitmap:
  memcpy(newBitmap.GetBuffer(), pixels, width * height * Pixel::GetBytesPerPixel(pixelFormat));
  return newBitmap;
}

/**
 * @brief Work out the desired width and height, accounting for zeros.
 *
 * @param[in] bitmapWidth Width of image before processing.
 * @param[in] bitmapHeight Height of image before processing.
 * @param[in] requestedWidth Width of area to scale image into. Can be zero.
 * @param[in] requestedHeight Height of area to scale image into. Can be zero.
 * @return Dimensions of area to scale image into after special rules are applied.
 */
ImageDimensions CalculateDesiredDimensions(uint32_t bitmapWidth, uint32_t bitmapHeight, uint32_t requestedWidth, uint32_t requestedHeight)
{
  uint32_t maxSize = Dali::GetMaxTextureSize();

  // If no dimensions have been requested, default to the source ones:
  if(requestedWidth == 0 && requestedHeight == 0)
  {
    if(bitmapWidth <= maxSize && bitmapHeight <= maxSize)
    {
      return ImageDimensions(bitmapWidth, bitmapHeight);
    }
    else
    {
      // Calculate the size from the max texture size and the source image aspect ratio
      if(bitmapWidth > bitmapHeight)
      {
        return ImageDimensions(maxSize, bitmapHeight * maxSize / static_cast<float>(bitmapWidth) + 0.5f);
      }
      else
      {
        return ImageDimensions(bitmapWidth * maxSize / static_cast<float>(bitmapHeight) + 0.5f, maxSize);
      }
    }
  }

  // If both dimensions have values requested, use them both:
  if(requestedWidth != 0 && requestedHeight != 0)
  {
    if(requestedWidth <= maxSize && requestedHeight <= maxSize)
    {
      return ImageDimensions(requestedWidth, requestedHeight);
    }
    else
    {
      // Calculate the size from the max texture size and the source image aspect ratio
      if(requestedWidth > requestedHeight)
      {
        return ImageDimensions(maxSize, requestedHeight * maxSize / static_cast<float>(requestedWidth) + 0.5f);
      }
      else
      {
        return ImageDimensions(requestedWidth * maxSize / static_cast<float>(requestedHeight) + 0.5f, maxSize);
      }
    }
  }

  // Only one of the dimensions has been requested. Calculate the other from
  // the requested one and the source image aspect ratio:
  if(requestedWidth != 0)
  {
    requestedWidth = std::min(requestedWidth, maxSize);
    return ImageDimensions(requestedWidth, bitmapHeight / float(bitmapWidth) * requestedWidth + 0.5f);
  }

  requestedHeight = std::min(requestedHeight, maxSize);
  return ImageDimensions(bitmapWidth / float(bitmapHeight) * requestedHeight + 0.5f, requestedHeight);
}

/**
 * @brief Rotates the given buffer @p pixelsIn 90 degrees counter clockwise.
 *
 * @note It allocates memory for the returned @p pixelsOut buffer.
 * @note Code got from https://www.codeproject.com/Articles/202/High-quality-image-rotation-rotate-by-shear by Eran Yariv.
 * @note It may fail if malloc() fails to allocate memory.
 *
 * @param[in] pixelsIn The input buffer.
 * @param[in] widthIn The width of the input buffer.
 * @param[in] heightIn The height of the input buffer.
 * @param[in] strideIn The stride of the input buffer.
 * @param[in] pixelSize The size of the pixel.
 * @param[out] pixelsOut The rotated output buffer.
 * @param[out] widthOut The width of the output buffer.
 * @param[out] heightOut The height of the output buffer.
 *
 * @return Whether the rotation succeeded.
 */
bool Rotate90(const uint8_t* const pixelsIn,
              uint32_t             widthIn,
              uint32_t             heightIn,
              uint32_t             strideIn,
              uint32_t             pixelSize,
              uint8_t*&            pixelsOut,
              uint32_t&            widthOut,
              uint32_t&            heightOut)
{
  // The new size of the image.
  widthOut  = heightIn;
  heightOut = widthIn;

  // Allocate memory for the rotated buffer.
  // Output buffer is tightly packed
  pixelsOut = static_cast<uint8_t*>(malloc(widthOut * heightOut * pixelSize));
  if(nullptr == pixelsOut)
  {
    widthOut  = 0u;
    heightOut = 0u;

    // Return if the memory allocations fails.
    return false;
  }

  // Rotate the buffer.
  for(uint32_t y = 0u; y < heightIn; ++y)
  {
    const uint32_t srcLineIndex = y * strideIn;
    const uint32_t dstX         = y;
    for(uint32_t x = 0u; x < widthIn; ++x)
    {
      const uint32_t dstY     = heightOut - x - 1u;
      const uint32_t dstIndex = pixelSize * (dstY * widthOut + dstX);
      const uint32_t srcIndex = pixelSize * (srcLineIndex + x);

      for(uint32_t channel = 0u; channel < pixelSize; ++channel)
      {
        *(pixelsOut + dstIndex + channel) = *(pixelsIn + srcIndex + channel);
      }
    }
  }

  return true;
}

/**
 * @brief Rotates the given buffer @p pixelsIn 180 degrees counter clockwise.
 *
 * @note It allocates memory for the returned @p pixelsOut buffer.
 * @note Code got from https://www.codeproject.com/Articles/202/High-quality-image-rotation-rotate-by-shear by Eran Yariv.
 * @note It may fail if malloc() fails to allocate memory.
 *
 * @param[in] pixelsIn The input buffer.
 * @param[in] widthIn The width of the input buffer.
 * @param[in] heightIn The height of the input buffer.
 * @param[in] strideIn The stride of the input buffer.
 * @param[in] pixelSize The size of the pixel.
 * @param[out] pixelsOut The rotated output buffer.
 *
 * @return Whether the rotation succeeded.
 */
bool Rotate180(const uint8_t* const pixelsIn,
               uint32_t             widthIn,
               uint32_t             heightIn,
               uint32_t             strideIn,
               uint32_t             pixelSize,
               uint8_t*&            pixelsOut)
{
  // Allocate memory for the rotated buffer.
  // Output buffer is tightly packed
  pixelsOut = static_cast<uint8_t*>(malloc(widthIn * heightIn * pixelSize));
  if(nullptr == pixelsOut)
  {
    // Return if the memory allocations fails.
    return false;
  }

  // Rotate the buffer.
  for(uint32_t y = 0u; y < heightIn; ++y)
  {
    const uint32_t srcLineIndex = y * strideIn;
    const uint32_t dstY         = heightIn - y - 1u;
    for(uint32_t x = 0u; x < widthIn; ++x)
    {
      const uint32_t dstX     = widthIn - x - 1u;
      const uint32_t dstIndex = pixelSize * (dstY * widthIn + dstX);
      const uint32_t srcIndex = pixelSize * (srcLineIndex + x);

      for(uint32_t channel = 0u; channel < pixelSize; ++channel)
      {
        *(pixelsOut + dstIndex + channel) = *(pixelsIn + srcIndex + channel);
      }
    }
  }

  return true;
}

/**
 * @brief Rotates the given buffer @p pixelsIn 270 degrees counter clockwise.
 *
 * @note It allocates memory for the returned @p pixelsOut buffer.
 * @note Code got from https://www.codeproject.com/Articles/202/High-quality-image-rotation-rotate-by-shear by Eran Yariv.
 * @note It may fail if malloc() fails to allocate memory.
 *
 * @param[in] pixelsIn The input buffer.
 * @param[in] widthIn The width of the input buffer.
 * @param[in] heightIn The height of the input buffer.
 * @param[in] strideIn The stride of the input buffer.
 * @param[in] pixelSize The size of the pixel.
 * @param[out] pixelsOut The rotated output buffer.
 * @param[out] widthOut The width of the output buffer.
 * @param[out] heightOut The height of the output buffer.
 *
 * @return Whether the rotation succeeded.
 */
bool Rotate270(const uint8_t* const pixelsIn,
               uint32_t             widthIn,
               uint32_t             heightIn,
               uint32_t             strideIn,
               uint32_t             pixelSize,
               uint8_t*&            pixelsOut,
               uint32_t&            widthOut,
               uint32_t&            heightOut)
{
  // The new size of the image.
  widthOut  = heightIn;
  heightOut = widthIn;

  // Allocate memory for the rotated buffer.
  // Output buffer is tightly packed
  pixelsOut = static_cast<uint8_t*>(malloc(widthOut * heightOut * pixelSize));
  if(nullptr == pixelsOut)
  {
    widthOut  = 0u;
    heightOut = 0u;

    // Return if the memory allocations fails.
    return false;
  }

  // Rotate the buffer.
  for(uint32_t y = 0u; y < heightIn; ++y)
  {
    const uint32_t srcLineIndex = y * strideIn;
    const uint32_t dstX         = widthOut - y - 1u;
    for(uint32_t x = 0u; x < widthIn; ++x)
    {
      const uint32_t dstY     = x;
      const uint32_t dstIndex = pixelSize * (dstY * widthOut + dstX);
      const uint32_t srcIndex = pixelSize * (srcLineIndex + x);

      for(uint32_t channel = 0u; channel < pixelSize; ++channel)
      {
        *(pixelsOut + dstIndex + channel) = *(pixelsIn + srcIndex + channel);
      }
    }
  }

  return true;
}

/**
 * @brief Skews a row horizontally (with filtered weights)
 *
 * @note Limited to 45 degree skewing only.
 * @note Code got from https://www.codeproject.com/Articles/202/High-quality-image-rotation-rotate-by-shear by Eran Yariv.
 *
 * @param[in] srcBufferPtr Pointer to the input pixel buffer.
 * @param[in] srcWidth The width of the input pixel buffer.
 * @param[in] srcStride The stride of the input pixel buffer.
 * @param[in] pixelSize The size of the pixel.
 * @param[in,out] dstPixelBuffer Pointer to the output pixel buffer.
 * @param[in] dstWidth The width of the output pixel buffer.
 * @param[in] row The row index.
 * @param[in] offset The skew offset.
 * @param[in] weight The relative weight of right pixel.
 */
void HorizontalSkew(const uint8_t* const srcBufferPtr,
                    uint32_t             srcWidth,
                    uint32_t             srcStride,
                    uint32_t             pixelSize,
                    uint8_t*&            dstBufferPtr,
                    uint32_t             dstWidth,
                    uint32_t             row,
                    int32_t              offset,
                    float                weight)
{
  if(offset > 0)
  {
    // Fill gap left of skew with background.
    memset(dstBufferPtr + row * pixelSize * dstWidth, 0u, pixelSize * offset);
  }

  uint8_t oldLeft[4u] = {0u, 0u, 0u, 0u};

  for(uint32_t i = 0u; i < srcWidth; ++i)
  {
    // Loop through row pixels
    const uint32_t srcIndex = pixelSize * (row * srcStride + i);

    uint8_t src[4u] = {0u, 0u, 0u, 0u};
    for(uint32_t channel = 0u; channel < pixelSize; ++channel)
    {
      src[channel] = *(srcBufferPtr + srcIndex + channel);
    }

    // Calculate weights
    uint8_t left[4u] = {0u, 0u, 0u, 0u};
    for(uint32_t channel = 0u; channel < pixelSize; ++channel)
    {
      left[channel] = static_cast<uint8_t>(static_cast<float>(src[channel]) * weight);

      // Update left over on source
      src[channel] -= (left[channel] - oldLeft[channel]);
    }

    // Check boundaries
    if((static_cast<int32_t>(i) + offset >= 0) && (i + offset < dstWidth))
    {
      const uint32_t dstIndex = pixelSize * (row * dstWidth + i + offset);

      for(uint32_t channel = 0u; channel < pixelSize; ++channel)
      {
        *(dstBufferPtr + dstIndex + channel) = src[channel];
      }
    }

    // Save leftover for next pixel in scan
    for(uint32_t channel = 0u; channel < pixelSize; ++channel)
    {
      oldLeft[channel] = left[channel];
    }
  }

  // Go to rightmost point of skew
  int32_t i = std::max(static_cast<int32_t>(srcWidth) + offset, -static_cast<int32_t>(dstWidth * row));
  if(i < static_cast<int32_t>(dstWidth))
  {
    // If still in image bounds, put leftovers there
    const uint32_t dstIndex = pixelSize * (row * dstWidth + i);

    for(uint32_t channel = 0u; channel < pixelSize; ++channel)
    {
      *(dstBufferPtr + dstIndex + channel) = oldLeft[channel];
    }

    // Clear to the right of the skewed line with background
    ++i;
    memset(dstBufferPtr + pixelSize * (row * dstWidth + i), 0u, pixelSize * (dstWidth - i));
  }
}

/**
 * @brief Skews a column vertically (with filtered weights)
 *
 * @note Limited to 45 degree skewing only.
 * @note Code got from https://www.codeproject.com/Articles/202/High-quality-image-rotation-rotate-by-shear by Eran Yariv.
 *
 * @param[in] srcBufferPtr Pointer to the input pixel buffer.
 * @param[in] srcWidth The width of the input pixel buffer.
 * @param[in] srcHeight The height of the input pixel buffer.
 * @param[in] srcStride The stride of the input pixel buffer.
 * @param[in] pixelSize The size of the pixel.
 * @param[in,out] dstPixelBuffer Pointer to the output pixel buffer.
 * @param[in] dstWidth The width of the output pixel buffer.
 * @param[in] dstHeight The height of the output pixel buffer.
 * @param[in] column The column index.
 * @param[in] offset The skew offset.
 * @param[in] weight The relative weight of uppeer pixel.
 */
void VerticalSkew(const uint8_t* const srcBufferPtr,
                  uint32_t             srcWidth,
                  uint32_t             srcHeight,
                  uint32_t             srcStride,
                  uint32_t             pixelSize,
                  uint8_t*&            dstBufferPtr,
                  uint32_t             dstWidth,
                  uint32_t             dstHeight,
                  uint32_t             column,
                  int32_t              offset,
                  float                weight)
{
  for(int32_t i = 0; i < offset; ++i)
  {
    // Fill gap above skew with background
    const uint32_t dstIndex = pixelSize * (i * dstWidth + column);

    for(uint32_t channel = 0u; channel < pixelSize; ++channel)
    {
      *(dstBufferPtr + dstIndex + channel) = 0u;
    }
  }

  uint8_t oldLeft[4u] = {0u, 0u, 0u, 0u};

  int32_t yPos = 0;

  for(uint32_t i = 0u; i < srcHeight; ++i)
  {
    // Loop through column pixels
    const uint32_t srcIndex = pixelSize * (i * srcStride + column);

    uint8_t src[4u] = {0u, 0u, 0u, 0u};
    for(uint32_t channel = 0u; channel < pixelSize; ++channel)
    {
      src[channel] = *(srcBufferPtr + srcIndex + channel);
    }

    yPos = static_cast<int32_t>(i) + offset;

    // Calculate weights
    uint8_t left[4u] = {0u, 0u, 0u, 0u};
    for(uint32_t channel = 0u; channel < pixelSize; ++channel)
    {
      left[channel] = static_cast<uint8_t>(static_cast<float>(src[channel]) * weight);
      // Update left over on source
      src[channel] -= (left[channel] - oldLeft[channel]);
    }

    // Check boundaries
    if((yPos >= 0) && (yPos < static_cast<int32_t>(dstHeight)))
    {
      const uint32_t dstIndex = pixelSize * (yPos * dstWidth + column);

      for(uint32_t channel = 0u; channel < pixelSize; ++channel)
      {
        *(dstBufferPtr + dstIndex + channel) = src[channel];
      }
    }

    // Save leftover for next pixel in scan
    for(uint32_t channel = 0u; channel < pixelSize; ++channel)
    {
      oldLeft[channel] = left[channel];
    }
  }

  // Go to bottom point of skew
  uint32_t i = 0;

  if(yPos >= 0)
  {
    i = static_cast<uint32_t>(yPos);
    if(i < dstHeight)
    {
      // If still in image bounds, put leftovers there
      const uint32_t dstIndex = pixelSize * (i * dstWidth + column);

      for(uint32_t channel = 0u; channel < pixelSize; ++channel)
      {
        *(dstBufferPtr + dstIndex + channel) = oldLeft[channel];
      }
      ++i;
    }
  }

  while(i < dstHeight)
  {
    // Clear below skewed line with background
    const uint32_t dstIndex = pixelSize * (i * dstWidth + column);

    for(uint32_t channel = 0u; channel < pixelSize; ++channel)
    {
      *(dstBufferPtr + dstIndex + channel) = 0u;
    }
    ++i;
  }
}

} // namespace

ImageDimensions CalculateDesiredDimensions(ImageDimensions rawDimensions, ImageDimensions requestedDimensions)
{
  return CalculateDesiredDimensions(rawDimensions.GetWidth(), rawDimensions.GetHeight(), requestedDimensions.GetWidth(), requestedDimensions.GetHeight());
}

/**
 * @brief Apply cropping and padding for specified fitting mode.
 *
 * Once the bitmap has been (optionally) downscaled to an appropriate size, this method performs alterations
 * based on the fitting mode.
 *
 * This will add vertical or horizontal borders if necessary.
 * Crop the source image data vertically or horizontally if necessary.
 * The aspect of the source image is preserved.
 * If the source image is smaller than the desired size, the algorithm will modify the the newly created
 *   bitmaps dimensions to only be as large as necessary, as a memory saving optimization. This will cause
 *   GPU scaling to be performed at render time giving the same result with less texture traversal.
 *
 * @param[in] bitmap            The source pixel buffer to perform modifications on.
 * @param[in] desiredDimensions The target dimensions to aim to fill based on the fitting mode.
 * @param[in] fittingMode       The fitting mode to use.
 *
 * @return                      A new bitmap with the padding and cropping required for fitting mode applied.
 *                              If no modification is needed or possible, the passed in bitmap is returned.
 */
Dali::Devel::PixelBuffer CropAndPadForFittingMode(Dali::Devel::PixelBuffer& bitmap, ImageDimensions desiredDimensions, FittingMode::Type fittingMode);

/**
 * @brief Adds horizontal or vertical borders to the source image.
 *
 * @param[in] targetPixels     The destination image pointer to draw the borders on.
 * @param[in] bytesPerPixel    The number of bytes per pixel of the target pixel buffer.
 * @param[in] targetDimensions The dimensions of the destination image.
 * @param[in] padDimensions    The columns and scanlines to pad with borders.
 */
void AddBorders(PixelBuffer* targetPixels, const uint32_t bytesPerPixel, const ImageDimensions targetDimensions, const ImageDimensions padDimensions);

Dali::Devel::PixelBuffer ApplyAttributesToBitmap(Dali::Devel::PixelBuffer bitmap, ImageDimensions dimensions, FittingMode::Type fittingMode, SamplingMode::Type samplingMode)
{
  if(bitmap)
  {
    // Calculate the desired box, accounting for a possible zero component:
    const ImageDimensions desiredDimensions = CalculateDesiredDimensions(bitmap.GetWidth(), bitmap.GetHeight(), dimensions.GetWidth(), dimensions.GetHeight());

    // If a different size than the raw one has been requested, resize the image
    // maximally using a repeated box filter without making it smaller than the
    // requested size in either dimension:
    bitmap = DownscaleBitmap(bitmap, desiredDimensions, fittingMode, samplingMode);

    // Cut the bitmap according to the desired width and height so that the
    // resulting bitmap has the same aspect ratio as the desired dimensions.
    // Add crop and add borders if necessary depending on fitting mode.
    if(bitmap)
    {
      bitmap = CropAndPadForFittingMode(bitmap, desiredDimensions, fittingMode);
    }
  }

  return bitmap;
}

Dali::Devel::PixelBuffer CropAndPadForFittingMode(Dali::Devel::PixelBuffer& bitmap, ImageDimensions desiredDimensions, FittingMode::Type fittingMode)
{
  const uint32_t inputWidth  = bitmap.GetWidth();
  const uint32_t inputHeight = bitmap.GetHeight();
  const uint32_t inputStride = bitmap.GetStride();

  if(desiredDimensions.GetWidth() < 1u || desiredDimensions.GetHeight() < 1u)
  {
    DALI_LOG_WARNING("Image scaling aborted as desired dimensions too small (%u, %u).\n", desiredDimensions.GetWidth(), desiredDimensions.GetHeight());
  }
  else if(inputWidth != desiredDimensions.GetWidth() || inputHeight != desiredDimensions.GetHeight())
  {
    // Calculate any padding or cropping that needs to be done based on the fitting mode.
    // Note: If the desired size is larger than the original image, the desired size will be
    // reduced while maintaining the aspect, in order to save unnecessary memory usage.
    int scanlinesToCrop = 0;
    int columnsToCrop   = 0;

    CalculateBordersFromFittingMode(ImageDimensions(inputWidth, inputHeight), fittingMode, desiredDimensions, scanlinesToCrop, columnsToCrop);

    uint32_t desiredWidth(desiredDimensions.GetWidth());
    uint32_t desiredHeight(desiredDimensions.GetHeight());

    // Action the changes by making a new bitmap with the central part of the loaded one if required.
    if(scanlinesToCrop != 0 || columnsToCrop != 0)
    {
      // Split the adding and removing of scanlines and columns into separate variables,
      // so we can use one piece of generic code to action the changes.
      uint32_t scanlinesToPad = 0;
      uint32_t columnsToPad   = 0;
      if(scanlinesToCrop < 0)
      {
        scanlinesToPad  = -scanlinesToCrop;
        scanlinesToCrop = 0;
      }
      if(columnsToCrop < 0)
      {
        columnsToPad  = -columnsToCrop;
        columnsToCrop = 0;
      }

      // If there is no filtering, then the final image size can become very large, exit if larger than maximum.
      if((desiredWidth > MAXIMUM_TARGET_BITMAP_SIZE) || (desiredHeight > MAXIMUM_TARGET_BITMAP_SIZE) ||
         (columnsToPad > MAXIMUM_TARGET_BITMAP_SIZE) || (scanlinesToPad > MAXIMUM_TARGET_BITMAP_SIZE))
      {
        DALI_LOG_WARNING("Image scaling aborted as final dimensions too large (%u, %u).\n", desiredWidth, desiredHeight);
        return bitmap;
      }

      // Create new PixelBuffer with the desired size.
      const auto pixelFormat = bitmap.GetPixelFormat();

      auto croppedBitmap = Devel::PixelBuffer::New(desiredWidth, desiredHeight, pixelFormat);

      // Add some pre-calculated offsets to the bitmap pointers so this is not done within a loop.
      // The cropping is added to the source pointer, and the padding is added to the destination.
      const auto               bytesPerPixel      = Pixel::GetBytesPerPixel(pixelFormat);
      const PixelBuffer* const sourcePixels       = bitmap.GetBuffer() + ((((scanlinesToCrop / 2) * inputStride) + (columnsToCrop / 2)) * bytesPerPixel);
      PixelBuffer* const       targetPixels       = croppedBitmap.GetBuffer();
      PixelBuffer* const       targetPixelsActive = targetPixels + ((((scanlinesToPad / 2) * desiredWidth) + (columnsToPad / 2)) * bytesPerPixel);
      DALI_ASSERT_DEBUG(sourcePixels && targetPixels);

      // Copy the image data to the new bitmap.
      // Optimize to a single memcpy if the left and right edges don't need a crop or a pad.
      uint32_t outputSpan(desiredWidth * bytesPerPixel);
      if(columnsToCrop == 0 && columnsToPad == 0 && inputStride == inputWidth)
      {
        memcpy(targetPixelsActive, sourcePixels, (desiredHeight - scanlinesToPad) * outputSpan);
      }
      else
      {
        // The width needs to change (due to either a crop or a pad), so we copy a scanline at a time.
        // Precalculate any constants to optimize the inner loop.
        const uint32_t inputSpan(inputStride * bytesPerPixel);
        const uint32_t copySpan((desiredWidth - columnsToPad) * bytesPerPixel);
        const uint32_t scanlinesToCopy(desiredHeight - scanlinesToPad);

        for(uint32_t y = 0; y < scanlinesToCopy; ++y)
        {
          memcpy(&targetPixelsActive[y * outputSpan], &sourcePixels[y * inputSpan], copySpan);
        }
      }

      // Add vertical or horizontal borders to the final image (if required).
      desiredDimensions.SetWidth(desiredWidth);
      desiredDimensions.SetHeight(desiredHeight);
      AddBorders(croppedBitmap.GetBuffer(), bytesPerPixel, desiredDimensions, ImageDimensions(columnsToPad, scanlinesToPad));
      // Overwrite the loaded bitmap with the cropped version
      bitmap = croppedBitmap;
    }
  }

  return bitmap;
}

void AddBorders(PixelBuffer* targetPixels, const uint32_t bytesPerPixel, const ImageDimensions targetDimensions, const ImageDimensions padDimensions)
{
  // Assign ints for faster access.
  uint32_t desiredWidth(targetDimensions.GetWidth());
  uint32_t desiredHeight(targetDimensions.GetHeight());
  uint32_t columnsToPad(padDimensions.GetWidth());
  uint32_t scanlinesToPad(padDimensions.GetHeight());
  uint32_t outputSpan(desiredWidth * bytesPerPixel);

  // Add letterboxing (symmetrical borders) if needed.
  if(scanlinesToPad > 0)
  {
    // Add a top border. Note: This is (deliberately) rounded down if padding is an odd number.
    memset(targetPixels, BORDER_FILL_VALUE, (scanlinesToPad / 2) * outputSpan);

    // We subtract scanlinesToPad/2 from scanlinesToPad so that we have the correct
    // offset for odd numbers (as the top border is 1 pixel smaller in these cases.
    uint32_t bottomBorderHeight = scanlinesToPad - (scanlinesToPad / 2);

    // Bottom border.
    memset(&targetPixels[(desiredHeight - bottomBorderHeight) * outputSpan], BORDER_FILL_VALUE, bottomBorderHeight * outputSpan);
  }
  else if(columnsToPad > 0)
  {
    // Add a left and right border.
    // Left:
    // Pre-calculate span size outside of loop.
    uint32_t leftBorderSpanWidth((columnsToPad / 2) * bytesPerPixel);
    for(uint32_t y = 0; y < desiredHeight; ++y)
    {
      memset(&targetPixels[y * outputSpan], BORDER_FILL_VALUE, leftBorderSpanWidth);
    }

    // Right:
    // Pre-calculate the initial x offset as it is always the same for a small optimization.
    // We subtract columnsToPad/2 from columnsToPad so that we have the correct
    // offset for odd numbers (as the left border is 1 pixel smaller in these cases.
    uint32_t           rightBorderWidth = columnsToPad - (columnsToPad / 2);
    PixelBuffer* const destPixelsRightBorder(targetPixels + ((desiredWidth - rightBorderWidth) * bytesPerPixel));
    uint32_t           rightBorderSpanWidth = rightBorderWidth * bytesPerPixel;

    for(uint32_t y = 0; y < desiredHeight; ++y)
    {
      memset(&destPixelsRightBorder[y * outputSpan], BORDER_FILL_VALUE, rightBorderSpanWidth);
    }
  }
}

Dali::Devel::PixelBuffer DownscaleBitmap(Dali::Devel::PixelBuffer bitmap,
                                         ImageDimensions          desired,
                                         FittingMode::Type        fittingMode,
                                         SamplingMode::Type       samplingMode)
{
  // Source dimensions as loaded from resources (e.g. filesystem):
  auto bitmapWidth  = bitmap.GetWidth();
  auto bitmapHeight = bitmap.GetHeight();
  auto bitmapStride = bitmap.GetStride();
  // Desired dimensions (the rectangle to fit the source image to):
  auto desiredWidth  = desired.GetWidth();
  auto desiredHeight = desired.GetHeight();

  Dali::Devel::PixelBuffer outputBitmap{bitmap};

  // If a different size than the raw one has been requested, resize the image:
  if(
    (desiredWidth > 0.0f) && (desiredHeight > 0.0f) &&
    ((desiredWidth < bitmapWidth) || (desiredHeight < bitmapHeight)))
  {
    auto pixelFormat = bitmap.GetPixelFormat();

    // Do the fast power of 2 iterated box filter to get to roughly the right side if the filter mode requests that:
    uint32_t shrunkWidth = -1, shrunkHeight = -1, outStride = -1;
    DownscaleInPlacePow2(bitmap.GetBuffer(), pixelFormat, bitmapWidth, bitmapHeight, bitmapStride, desiredWidth, desiredHeight, fittingMode, samplingMode, shrunkWidth, shrunkHeight, outStride);

    // Work out the dimensions of the downscaled bitmap, given the scaling mode and desired dimensions:
    const ImageDimensions filteredDimensions = FitToScalingMode(ImageDimensions(desiredWidth, desiredHeight), ImageDimensions(shrunkWidth, shrunkHeight), fittingMode);
    const uint32_t        filteredWidth      = filteredDimensions.GetWidth();
    const uint32_t        filteredHeight     = filteredDimensions.GetHeight();

    // Run a filter to scale down the bitmap if it needs it:
    bool filtered = false;
    if(filteredWidth < shrunkWidth || filteredHeight < shrunkHeight)
    {
      if(samplingMode == SamplingMode::LINEAR || samplingMode == SamplingMode::BOX_THEN_LINEAR ||
         samplingMode == SamplingMode::NEAREST || samplingMode == SamplingMode::BOX_THEN_NEAREST)
      {
        outputBitmap = Dali::Devel::PixelBuffer::New(filteredWidth, filteredHeight, pixelFormat);

        if(outputBitmap)
        {
          if(samplingMode == SamplingMode::LINEAR || samplingMode == SamplingMode::BOX_THEN_LINEAR)
          {
            LinearSample(bitmap.GetBuffer(), ImageDimensions(shrunkWidth, shrunkHeight), outStride, pixelFormat, outputBitmap.GetBuffer(), filteredDimensions);
          }
          else
          {
            PointSample(bitmap.GetBuffer(), shrunkWidth, shrunkHeight, outStride, pixelFormat, outputBitmap.GetBuffer(), filteredWidth, filteredHeight);
          }
          filtered = true;
        }
      }
    }
    // Copy out the 2^x downscaled, box-filtered pixels if no secondary filter (point or linear) was applied:
    if(filtered == false && (shrunkWidth < bitmapWidth || shrunkHeight < bitmapHeight))
    {
      // The buffer is downscaled and it is tightly packed. We don't need to set a stride.
      outputBitmap = MakePixelBuffer(bitmap.GetBuffer(), pixelFormat, shrunkWidth, shrunkHeight);
    }
  }

  return outputBitmap;
}

namespace
{
/**
 * @brief Returns whether to keep box filtering based on whether downscaled dimensions will overshoot the desired ones aty the next step.
 * @param test Which combination of the two dimensions matter for terminating the filtering.
 * @param scaledWidth The width of the current downscaled image.
 * @param scaledHeight The height of the current downscaled image.
 * @param desiredWidth The target width for the downscaling.
 * @param desiredHeight The target height for the downscaling.
 */
bool ContinueScaling(BoxDimensionTest test, uint32_t scaledWidth, uint32_t scaledHeight, uint32_t desiredWidth, uint32_t desiredHeight)
{
  bool           keepScaling = false;
  const uint32_t nextWidth   = scaledWidth >> 1u;
  const uint32_t nextHeight  = scaledHeight >> 1u;

  if(nextWidth >= 1u && nextHeight >= 1u)
  {
    switch(test)
    {
      case BoxDimensionTestEither:
      {
        keepScaling = nextWidth >= desiredWidth || nextHeight >= desiredHeight;
        break;
      }
      case BoxDimensionTestBoth:
      {
        keepScaling = nextWidth >= desiredWidth && nextHeight >= desiredHeight;
        break;
      }
      case BoxDimensionTestX:
      {
        keepScaling = nextWidth >= desiredWidth;
        break;
      }
      case BoxDimensionTestY:
      {
        keepScaling = nextHeight >= desiredHeight;
        break;
      }
    }
  }

  return keepScaling;
}

/**
 * @brief A shared implementation of the overall iterative box filter
 * downscaling algorithm.
 *
 * Specialise this for particular pixel formats by supplying the number of bytes
 * per pixel and two functions: one for averaging pairs of neighbouring pixels
 * on a single scanline, and a second for averaging pixels at corresponding
 * positions on different scanlines.
 **/
template<
  int BYTES_PER_PIXEL,
  void (*HalveScanlineInPlace)(uint8_t* const pixels, const uint32_t width),
  void (*AverageScanlines)(const uint8_t* const scanline1, const uint8_t* const __restrict__ scanline2, uint8_t* const outputScanline, const uint32_t width)>
void DownscaleInPlacePow2Generic(uint8_t* const   pixels,
                                 const uint32_t   inputWidth,
                                 const uint32_t   inputHeight,
                                 const uint32_t   inputStride,
                                 const uint32_t   desiredWidth,
                                 const uint32_t   desiredHeight,
                                 BoxDimensionTest dimensionTest,
                                 uint32_t&        outWidth,
                                 uint32_t&        outHeight,
                                 uint32_t&        outStride)
{
  if(pixels == 0)
  {
    return;
  }
  ValidateScalingParameters(inputWidth, inputHeight, desiredWidth, desiredHeight);

  // Scale the image until it would be smaller than desired, stopping if the
  // resulting height or width would be less than 1:
  uint32_t scaledWidth = inputWidth, scaledHeight = inputHeight, stride = inputStride;
  while(ContinueScaling(dimensionTest, scaledWidth, scaledHeight, desiredWidth, desiredHeight))
  {
    const uint32_t lastWidth  = scaledWidth;
    const uint32_t lastStride = stride;
    scaledWidth >>= 1u;
    scaledHeight >>= 1u;
    stride = scaledWidth;

    DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "Scaling to %u\t%u.\n", scaledWidth, scaledHeight);

    const uint32_t lastScanlinePair = scaledHeight - 1;

    // Scale pairs of scanlines until any spare one at the end is dropped:
    for(uint32_t y = 0; y <= lastScanlinePair; ++y)
    {
      // Scale two scanlines horizontally:
      HalveScanlineInPlace(&pixels[y * 2 * lastStride * BYTES_PER_PIXEL], lastWidth);
      HalveScanlineInPlace(&pixels[(y * 2 + 1) * lastStride * BYTES_PER_PIXEL], lastWidth);

      // Scale vertical pairs of pixels while the last two scanlines are still warm in
      // the CPU cache(s):
      // Note, better access patterns for cache-coherence are possible for very large
      // images but even a 4k wide RGB888 image will use just 24kB of cache (4k pixels
      // * 3 Bpp * 2 scanlines) for two scanlines on the first iteration.
      AverageScanlines(
        &pixels[y * 2 * lastStride * BYTES_PER_PIXEL],
        &pixels[(y * 2 + 1) * lastStride * BYTES_PER_PIXEL],
        &pixels[y * scaledWidth * BYTES_PER_PIXEL],
        scaledWidth);
    }
  }

  ///@note: we could finish off with one of two mutually exclusive passes, one squashing horizontally as far as possible, and the other vertically, if we knew a following cpu point or bilinear filter would restore the desired aspect ratio.
  outWidth  = scaledWidth;
  outHeight = scaledHeight;
  outStride = stride;
}

} // namespace

void HalveScanlineInPlaceRGB888(uint8_t* const pixels, const uint32_t width)
{
  DebugAssertScanlineParameters(pixels, width);

  const uint32_t lastPair = EvenDown(width - 2);

  /**
   * @code
   *  for(uint32_t pixel = 0, outPixel = 0; pixel <= lastPair; pixel += 2, ++outPixel)
   * {
   *   // Load all the byte pixel components we need:
   *   const uint32_t c11 = pixels[pixel * 3];
   *   const uint32_t c12 = pixels[pixel * 3 + 1];
   *   const uint32_t c13 = pixels[pixel * 3 + 2];
   *   const uint32_t c21 = pixels[pixel * 3 + 3];
   *   const uint32_t c22 = pixels[pixel * 3 + 4];
   *   const uint32_t c23 = pixels[pixel * 3 + 5];
   *
   *   // Save the averaged byte pixel components:
   *   pixels[outPixel * 3]     = static_cast<uint8_t>(AverageComponent(c11, c21));
   *   pixels[outPixel * 3 + 1] = static_cast<uint8_t>(AverageComponent(c12, c22));
   *   pixels[outPixel * 3 + 2] = static_cast<uint8_t>(AverageComponent(c13, c23));
   * }
   *   @endcode
   */
  //@ToDo : Fix here if we found that collect 12 bytes == 3 uint32_t with 4 colors, and calculate in one-operation
  std::uint8_t* inPixelPtr  = pixels;
  std::uint8_t* outPixelPtr = pixels;
  for(std::uint32_t scanedPixelCount = 0; scanedPixelCount <= lastPair; scanedPixelCount += 2)
  {
    *(outPixelPtr + 0) = ((*(inPixelPtr + 0) ^ *(inPixelPtr + 3)) >> 1) + (*(inPixelPtr + 0) & *(inPixelPtr + 3));
    *(outPixelPtr + 1) = ((*(inPixelPtr + 1) ^ *(inPixelPtr + 4)) >> 1) + (*(inPixelPtr + 1) & *(inPixelPtr + 4));
    *(outPixelPtr + 2) = ((*(inPixelPtr + 2) ^ *(inPixelPtr + 5)) >> 1) + (*(inPixelPtr + 2) & *(inPixelPtr + 5));
    inPixelPtr += 6;
    outPixelPtr += 3;
  }
}

void HalveScanlineInPlaceRGBA8888(uint8_t* const pixels, const uint32_t width)
{
  DebugAssertScanlineParameters(pixels, width);
  DALI_ASSERT_DEBUG(((reinterpret_cast<ptrdiff_t>(pixels) & 3u) == 0u) && "Pointer should be 4-byte aligned for performance on some platforms.");

  uint32_t* const alignedPixels = reinterpret_cast<uint32_t*>(pixels);

  const uint32_t lastPair = EvenDown(width - 2);

  for(uint32_t pixel = 0, outPixel = 0; pixel <= lastPair; pixel += 2, ++outPixel)
  {
    const uint32_t averaged = AveragePixelRGBA8888(alignedPixels[pixel], alignedPixels[pixel + 1]);
    alignedPixels[outPixel] = averaged;
  }
}

void HalveScanlineInPlaceRGB565(uint8_t* pixels, uint32_t width)
{
  DebugAssertScanlineParameters(pixels, width);
  DALI_ASSERT_DEBUG(((reinterpret_cast<ptrdiff_t>(pixels) & 1u) == 0u) && "Pointer should be 2-byte aligned for performance on some platforms.");

  uint16_t* const alignedPixels = reinterpret_cast<uint16_t*>(pixels);

  const uint32_t lastPair = EvenDown(width - 2);

  for(uint32_t pixel = 0, outPixel = 0; pixel <= lastPair; pixel += 2, ++outPixel)
  {
    const uint16_t averaged = AveragePixelRGB565(alignedPixels[pixel], alignedPixels[pixel + 1]);
    alignedPixels[outPixel] = averaged;
  }
}

void HalveScanlineInPlace2Bytes(uint8_t* const pixels, const uint32_t width)
{
  DebugAssertScanlineParameters(pixels, width);

  const uint32_t lastPair = EvenDown(width - 2);

  for(uint32_t pixel = 0, outPixel = 0; pixel <= lastPair; pixel += 2, ++outPixel)
  {
    /**
     * @code
     * // Load all the byte pixel components we need:
     * const uint32_t c11 = pixels[pixel * 2];
     * const uint32_t c12 = pixels[pixel * 2 + 1];
     * const uint32_t c21 = pixels[pixel * 2 + 2];
     * const uint32_t c22 = pixels[pixel * 2 + 3];
     *
     * // Save the averaged byte pixel components:
     * pixels[outPixel * 2]     = static_cast<uint8_t>(AverageComponent(c11, c21));
     * pixels[outPixel * 2 + 1] = static_cast<uint8_t>(AverageComponent(c12, c22));
     * @endcode
     */
    // Note : We can assume that pixel is even number. So we can use | operation instead of + operation.
    pixels[(outPixel << 1)]     = ((pixels[(pixel << 1)] ^ pixels[(pixel << 1) | 2]) >> 1) + (pixels[(pixel << 1)] & pixels[(pixel << 1) | 2]);
    pixels[(outPixel << 1) | 1] = ((pixels[(pixel << 1) | 1] ^ pixels[(pixel << 1) | 3]) >> 1) + (pixels[(pixel << 1) | 1] & pixels[(pixel << 1) | 3]);
  }
}

void HalveScanlineInPlace1Byte(uint8_t* const pixels, const uint32_t width)
{
  DebugAssertScanlineParameters(pixels, width);

  const uint32_t lastPair = EvenDown(width - 2);

  for(uint32_t pixel = 0, outPixel = 0; pixel <= lastPair; pixel += 2, ++outPixel)
  {
    /**
     * @code
     * // Load all the byte pixel components we need:
     * const uint32_t c1 = pixels[pixel];
     * const uint32_t c2 = pixels[pixel + 1];
     *
     * // Save the averaged byte pixel component:
     * pixels[outPixel] = static_cast<uint8_t>(AverageComponent(c1, c2));
     * @endcode
     */
    // Note : We can assume that pixel is even number. So we can use | operation instead of + operation.
    pixels[outPixel] = ((pixels[pixel] ^ pixels[pixel | 1]) >> 1) + (pixels[pixel] & pixels[pixel | 1]);
  }
}

// AverageScanline

namespace
{
/**
 * @copydoc AverageScanlines1
 * @note This API average eight components in one operation.
 * @note Only possible if each scanline pointer's address aligned
 * It will give performance benifit.
 */
inline void AverageScanlinesWithMultipleComponents(
  const uint8_t* const scanline1,
  const uint8_t* const __restrict__ scanline2,
  uint8_t* const outputScanline,
  const uint32_t totalComponentCount)
{
  uint32_t component = 0;
  if(DALI_LIKELY(totalComponentCount >= 16))
  {
    // Note reinsterpret_cast from uint8_t to uint64_t (or uint32_t) and read/write only allowed
    // If pointer of data is aligned well.
    // (to avoid SIGBUS)

    // To increase the percentage of optimized works, let we check pre-padding value of each pointer.
    auto scanline1Padding   = (reinterpret_cast<std::ptrdiff_t>(scanline1) & (sizeof(std::uint64_t) - 1));
    auto scanline2Padding   = (reinterpret_cast<std::ptrdiff_t>(scanline2) & (sizeof(std::uint64_t) - 1));
    auto outScanlinePadding = (reinterpret_cast<std::ptrdiff_t>(outputScanline) & (sizeof(std::uint64_t) - 1));
    if((scanline1Padding == scanline2Padding) && (scanline1Padding == outScanlinePadding))
    {
      const auto padding = (sizeof(std::uint64_t) - scanline1Padding) & (sizeof(std::uint64_t) - 1);

      // Prepadding range calculate
      for(std::uint32_t i = 0; i < padding; ++i)
      {
        const auto& c1    = scanline1[i];
        const auto& c2    = scanline2[i];
        outputScanline[i] = static_cast<std::uint8_t>(((c1 ^ c2) >> 1) + (c1 & c2));
      }

      // Jump 8 components in one step
      const std::uint64_t* const scanline18Step = reinterpret_cast<const std::uint64_t* const>(scanline1 + padding);
      const std::uint64_t* const scanline28Step = reinterpret_cast<const std::uint64_t* const>(scanline2 + padding);
      std::uint64_t* const       output8step    = reinterpret_cast<std::uint64_t* const>(outputScanline + padding);

      const std::uint32_t totalStepCount = (totalComponentCount) >> 3;
      component                          = (totalStepCount << 3) + padding;

      // and for each step, calculate average of 8 bytes.
      for(std::uint32_t i = 0; i < totalStepCount; ++i)
      {
        const auto& c1     = *(scanline18Step + i);
        const auto& c2     = *(scanline28Step + i);
        *(output8step + i) = static_cast<std::uint64_t>((((c1 ^ c2) & 0xfefefefefefefefeull) >> 1) + (c1 & c2));
      }
    }
    else if(((scanline1Padding & (sizeof(std::uint32_t) - 1)) == (scanline2Padding & (sizeof(std::uint32_t) - 1))) &&
            ((scanline1Padding & (sizeof(std::uint32_t) - 1)) == (outScanlinePadding & (sizeof(std::uint32_t) - 1))))
    {
      const auto padding = (sizeof(std::uint64_t) - scanline1Padding) & (sizeof(std::uint32_t) - 1);

      // Prepadding range calculate
      for(std::uint32_t i = 0; i < padding; ++i)
      {
        const auto& c1    = scanline1[i];
        const auto& c2    = scanline2[i];
        outputScanline[i] = static_cast<std::uint8_t>(((c1 ^ c2) >> 1) + (c1 & c2));
      }

      // Jump 4 components in one step
      const std::uint32_t* const scanline14Step = reinterpret_cast<const std::uint32_t* const>(scanline1 + padding);
      const std::uint32_t* const scanline24Step = reinterpret_cast<const std::uint32_t* const>(scanline2 + padding);
      std::uint32_t* const       output4step    = reinterpret_cast<std::uint32_t* const>(outputScanline + padding);

      const std::uint32_t totalStepCount = (totalComponentCount) >> 2;
      component                          = (totalStepCount << 2) + padding;

      // and for each step, calculate average of 4 bytes.
      for(std::uint32_t i = 0; i < totalStepCount; ++i)
      {
        const auto& c1     = *(scanline14Step + i);
        const auto& c2     = *(scanline24Step + i);
        *(output4step + i) = static_cast<std::uint32_t>((((c1 ^ c2) & 0xfefefefeu) >> 1) + (c1 & c2));
      }
    }
  }
  // remaining components calculate
  for(; component < totalComponentCount; ++component)
  {
    const auto& c1            = scanline1[component];
    const auto& c2            = scanline2[component];
    outputScanline[component] = static_cast<std::uint8_t>(((c1 ^ c2) >> 1) + (c1 & c2));
  }
}

} // namespace

void AverageScanlines1(const uint8_t* const scanline1,
                       const uint8_t* const __restrict__ scanline2,
                       uint8_t* const outputScanline,
                       const uint32_t width)
{
  DebugAssertDualScanlineParameters(scanline1, scanline2, outputScanline, width);

  /**
   * @code
   * for(uint32_t component = 0; component < width; ++component)
   * {
   *   outputScanline[component] = static_cast<uint8_t>(AverageComponent(scanline1[component], scanline2[component]));
   * }
   * @endcode
   */
  AverageScanlinesWithMultipleComponents(scanline1, scanline2, outputScanline, width);
}

void AverageScanlines2(const uint8_t* const scanline1,
                       const uint8_t* const __restrict__ scanline2,
                       uint8_t* const outputScanline,
                       const uint32_t width)
{
  DebugAssertDualScanlineParameters(scanline1, scanline2, outputScanline, width * 2);

  /**
   * @code
   * for(uint32_t component = 0; component < width * 2; ++component)
   * {
   *   outputScanline[component] = static_cast<uint8_t>(AverageComponent(scanline1[component], scanline2[component]));
   * }
   * @endcode
   */
  AverageScanlinesWithMultipleComponents(scanline1, scanline2, outputScanline, width * 2);
}

void AverageScanlines3(const uint8_t* const scanline1,
                       const uint8_t* const __restrict__ scanline2,
                       uint8_t* const outputScanline,
                       const uint32_t width)
{
  DebugAssertDualScanlineParameters(scanline1, scanline2, outputScanline, width * 3);

  /**
   * @code
   * for(uint32_t component = 0; component < width * 3; ++component)
   * {
   *   outputScanline[component] = static_cast<uint8_t>(AverageComponent(scanline1[component], scanline2[component]));
   * }
   * @endcode
   */
  AverageScanlinesWithMultipleComponents(scanline1, scanline2, outputScanline, width * 3);
}

void AverageScanlinesRGBA8888(const uint8_t* const scanline1,
                              const uint8_t* const __restrict__ scanline2,
                              uint8_t* const outputScanline,
                              const uint32_t width)
{
  DebugAssertDualScanlineParameters(scanline1, scanline2, outputScanline, width * 4);
  DALI_ASSERT_DEBUG(((reinterpret_cast<ptrdiff_t>(scanline1) & 3u) == 0u) && "Pointer should be 4-byte aligned for performance on some platforms.");
  DALI_ASSERT_DEBUG(((reinterpret_cast<ptrdiff_t>(scanline2) & 3u) == 0u) && "Pointer should be 4-byte aligned for performance on some platforms.");
  DALI_ASSERT_DEBUG(((reinterpret_cast<ptrdiff_t>(outputScanline) & 3u) == 0u) && "Pointer should be 4-byte aligned for performance on some platforms.");

  /**
   * @code
   * const uint32_t* const alignedScanline1 = reinterpret_cast<const uint32_t*>(scanline1);
   * const uint32_t* const alignedScanline2 = reinterpret_cast<const uint32_t*>(scanline2);
   * uint32_t* const       alignedOutput    = reinterpret_cast<uint32_t*>(outputScanline);
   *
   * for(uint32_t pixel = 0; pixel < width; ++pixel)
   * {
   *   alignedOutput[pixel] = AveragePixelRGBA8888(alignedScanline1[pixel], alignedScanline2[pixel]);
   * }
   * @endcode
   */

  AverageScanlinesWithMultipleComponents(scanline1, scanline2, outputScanline, width * 4u);
}

void AverageScanlinesRGB565(const uint8_t* const scanline1,
                            const uint8_t* const __restrict__ scanline2,
                            uint8_t* const outputScanline,
                            const uint32_t width)
{
  DebugAssertDualScanlineParameters(scanline1, scanline2, outputScanline, width * 2);
  DALI_ASSERT_DEBUG(((reinterpret_cast<ptrdiff_t>(scanline1) & 1u) == 0u) && "Pointer should be 2-byte aligned for performance on some platforms.");
  DALI_ASSERT_DEBUG(((reinterpret_cast<ptrdiff_t>(scanline2) & 1u) == 0u) && "Pointer should be 2-byte aligned for performance on some platforms.");
  DALI_ASSERT_DEBUG(((reinterpret_cast<ptrdiff_t>(outputScanline) & 1u) == 0u) && "Pointer should be 2-byte aligned for performance on some platforms.");

  const uint16_t* const alignedScanline1 = reinterpret_cast<const uint16_t*>(scanline1);
  const uint16_t* const alignedScanline2 = reinterpret_cast<const uint16_t*>(scanline2);
  uint16_t* const       alignedOutput    = reinterpret_cast<uint16_t*>(outputScanline);

  for(uint32_t pixel = 0; pixel < width; ++pixel)
  {
    alignedOutput[pixel] = AveragePixelRGB565(alignedScanline1[pixel], alignedScanline2[pixel]);
  }
}

/// Dispatch to pixel format appropriate box filter downscaling functions.
void DownscaleInPlacePow2(uint8_t* const     pixels,
                          Pixel::Format      pixelFormat,
                          uint32_t           inputWidth,
                          uint32_t           inputHeight,
                          uint32_t           inputStride,
                          uint32_t           desiredWidth,
                          uint32_t           desiredHeight,
                          FittingMode::Type  fittingMode,
                          SamplingMode::Type samplingMode,
                          uint32_t&          outWidth,
                          uint32_t&          outHeight,
                          uint32_t&          outStride)
{
  outWidth  = inputWidth;
  outHeight = inputHeight;
  outStride = inputStride;
  // Perform power of 2 iterated 4:1 box filtering if the requested filter mode requires it:
  if(samplingMode == SamplingMode::BOX || samplingMode == SamplingMode::BOX_THEN_NEAREST || samplingMode == SamplingMode::BOX_THEN_LINEAR)
  {
    // Check the pixel format is one that is supported:
    if(pixelFormat == Pixel::RGBA8888 || pixelFormat == Pixel::RGB888 || pixelFormat == Pixel::RGB565 || pixelFormat == Pixel::LA88 || pixelFormat == Pixel::L8 || pixelFormat == Pixel::A8)
    {
      const BoxDimensionTest dimensionTest = DimensionTestForScalingMode(fittingMode);

      switch(pixelFormat)
      {
        case Pixel::RGBA8888:
        {
          Internal::Platform::DownscaleInPlacePow2RGBA8888(pixels, inputWidth, inputHeight, inputStride, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight, outStride);
          break;
        }
        case Pixel::RGB888:
        {
          Internal::Platform::DownscaleInPlacePow2RGB888(pixels, inputWidth, inputHeight, inputStride, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight, outStride);
          break;
        }
        case Pixel::RGB565:
        {
          Internal::Platform::DownscaleInPlacePow2RGB565(pixels, inputWidth, inputHeight, inputStride, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight, outStride);
          break;
        }
        case Pixel::LA88:
        {
          Internal::Platform::DownscaleInPlacePow2ComponentPair(pixels, inputWidth, inputHeight, inputStride, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight, outStride);
          break;
        }
        case Pixel::L8:
        case Pixel::A8:
        {
          Internal::Platform::DownscaleInPlacePow2SingleBytePerPixel(pixels, inputWidth, inputHeight, inputStride, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight, outStride);
          break;
        }
        default:
        {
          DALI_ASSERT_DEBUG(false && "Inner branch conditions don't match outer branch.");
        }
      }
    }
  }
  else
  {
    DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "Bitmap was not shrunk: unsupported pixel format: %u.\n", uint32_t(pixelFormat));
  }
}

void DownscaleInPlacePow2RGB888(uint8_t*         pixels,
                                uint32_t         inputWidth,
                                uint32_t         inputHeight,
                                uint32_t         inputStride,
                                uint32_t         desiredWidth,
                                uint32_t         desiredHeight,
                                BoxDimensionTest dimensionTest,
                                uint32_t&        outWidth,
                                uint32_t&        outHeight,
                                uint32_t&        outStride)
{
  DownscaleInPlacePow2Generic<3, HalveScanlineInPlaceRGB888, AverageScanlines3>(pixels, inputWidth, inputHeight, inputStride, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight, outStride);
}

void DownscaleInPlacePow2RGBA8888(uint8_t*         pixels,
                                  uint32_t         inputWidth,
                                  uint32_t         inputHeight,
                                  uint32_t         inputStride,
                                  uint32_t         desiredWidth,
                                  uint32_t         desiredHeight,
                                  BoxDimensionTest dimensionTest,
                                  uint32_t&        outWidth,
                                  uint32_t&        outHeight,
                                  uint32_t&        outStride)
{
  DALI_ASSERT_DEBUG(((reinterpret_cast<ptrdiff_t>(pixels) & 3u) == 0u) && "Pointer should be 4-byte aligned for performance on some platforms.");
  DownscaleInPlacePow2Generic<4, HalveScanlineInPlaceRGBA8888, AverageScanlinesRGBA8888>(pixels, inputWidth, inputHeight, inputStride, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight, outStride);
}

void DownscaleInPlacePow2RGB565(uint8_t*         pixels,
                                uint32_t         inputWidth,
                                uint32_t         inputHeight,
                                uint32_t         inputStride,
                                uint32_t         desiredWidth,
                                uint32_t         desiredHeight,
                                BoxDimensionTest dimensionTest,
                                uint32_t&        outWidth,
                                uint32_t&        outHeight,
                                uint32_t&        outStride)
{
  DownscaleInPlacePow2Generic<2, HalveScanlineInPlaceRGB565, AverageScanlinesRGB565>(pixels, inputWidth, inputHeight, inputStride, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight, outStride);
}

/**
 * @copydoc DownscaleInPlacePow2RGB888
 *
 * For 2-byte formats such as lum8alpha8, but not packed 16 bit formats like RGB565.
 */
void DownscaleInPlacePow2ComponentPair(uint8_t*         pixels,
                                       uint32_t         inputWidth,
                                       uint32_t         inputHeight,
                                       uint32_t         inputStride,
                                       uint32_t         desiredWidth,
                                       uint32_t         desiredHeight,
                                       BoxDimensionTest dimensionTest,
                                       uint32_t&        outWidth,
                                       uint32_t&        outHeight,
                                       uint32_t&        outStride)
{
  DownscaleInPlacePow2Generic<2, HalveScanlineInPlace2Bytes, AverageScanlines2>(pixels, inputWidth, inputHeight, inputStride, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight, outStride);
}

void DownscaleInPlacePow2SingleBytePerPixel(uint8_t*         pixels,
                                            uint32_t         inputWidth,
                                            uint32_t         inputHeight,
                                            uint32_t         inputStride,
                                            uint32_t         desiredWidth,
                                            uint32_t         desiredHeight,
                                            BoxDimensionTest dimensionTest,
                                            uint32_t&        outWidth,
                                            uint32_t&        outHeight,
                                            uint32_t&        outStride)
{
  DownscaleInPlacePow2Generic<1, HalveScanlineInPlace1Byte, AverageScanlines1>(pixels, inputWidth, inputHeight, inputStride, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight, outStride);
}

// Point sampling group below

namespace
{
/**
 * @brief Point sample an image to a new resolution (like GL_NEAREST).
 *
 * Template is used purely as a type-safe code generator in this one
 * compilation unit. Generated code is inlined into type-specific wrapper
 * functions below which are exported to rest of module.
 */
template<typename PIXEL>
inline void PointSampleAddressablePixels(const uint8_t* inPixels,
                                         uint32_t       inputWidth,
                                         uint32_t       inputHeight,
                                         uint32_t       inputStride,
                                         uint8_t*       outPixels,
                                         uint32_t       desiredWidth,
                                         uint32_t       desiredHeight)
{
  DALI_ASSERT_DEBUG(((desiredWidth <= inputWidth && desiredHeight <= inputHeight) ||
                     outPixels >= inPixels + inputStride * inputHeight * sizeof(PIXEL) || outPixels <= inPixels - desiredWidth * desiredHeight * sizeof(PIXEL)) &&
                    "The input and output buffers must not overlap for an upscaling.");
  DALI_ASSERT_DEBUG(reinterpret_cast<uint64_t>(inPixels) % sizeof(PIXEL) == 0 && "Pixel pointers need to be aligned to the size of the pixels (E.g., 4 bytes for RGBA, 2 bytes for RGB565, ...).");
  DALI_ASSERT_DEBUG(reinterpret_cast<uint64_t>(outPixels) % sizeof(PIXEL) == 0 && "Pixel pointers need to be aligned to the size of the pixels (E.g., 4 bytes for RGBA, 2 bytes for RGB565, ...).");

  if(inputWidth < 1u || inputHeight < 1u || desiredWidth < 1u || desiredHeight < 1u)
  {
    return;
  }
  const PIXEL* const inAligned  = reinterpret_cast<const PIXEL*>(inPixels);
  PIXEL* const       outAligned = reinterpret_cast<PIXEL*>(outPixels);
  const uint32_t     deltaX     = (inputWidth << 16u) / desiredWidth;
  const uint32_t     deltaY     = (inputHeight << 16u) / desiredHeight;

  uint32_t inY = 0;
  for(uint32_t outY = 0; outY < desiredHeight; ++outY)
  {
    // Round fixed point y coordinate to nearest integer:
    const uint32_t     integerY    = (inY + (1u << 15u)) >> 16u;
    const PIXEL* const inScanline  = &inAligned[inputStride * integerY];
    PIXEL* const       outScanline = &outAligned[desiredWidth * outY];

    DALI_ASSERT_DEBUG(integerY < inputHeight);
    DALI_ASSERT_DEBUG(reinterpret_cast<const uint8_t*>(inScanline) < (inPixels + inputStride * inputHeight * sizeof(PIXEL)));
    DALI_ASSERT_DEBUG(reinterpret_cast<uint8_t*>(outScanline) < (outPixels + desiredWidth * desiredHeight * sizeof(PIXEL)));

    uint32_t inX = 0;
    for(uint32_t outX = 0; outX < desiredWidth; ++outX)
    {
      // Round the fixed-point x coordinate to an integer:
      const uint32_t     integerX       = (inX + (1u << 15u)) >> 16u;
      const PIXEL* const inPixelAddress = &inScanline[integerX];
      const PIXEL        pixel          = *inPixelAddress;
      outScanline[outX]                 = pixel;
      inX += deltaX;
    }
    inY += deltaY;
  }
}

} // namespace

// RGBA8888
void PointSample4BPP(const uint8_t* inPixels,
                     uint32_t       inputWidth,
                     uint32_t       inputHeight,
                     uint32_t       inputStride,
                     uint8_t*       outPixels,
                     uint32_t       desiredWidth,
                     uint32_t       desiredHeight)
{
  PointSampleAddressablePixels<uint32_t>(inPixels, inputWidth, inputHeight, inputStride, outPixels, desiredWidth, desiredHeight);
}

// RGB565, LA88
void PointSample2BPP(const uint8_t* inPixels,
                     uint32_t       inputWidth,
                     uint32_t       inputHeight,
                     uint32_t       inputStride,
                     uint8_t*       outPixels,
                     uint32_t       desiredWidth,
                     uint32_t       desiredHeight)
{
  PointSampleAddressablePixels<uint16_t>(inPixels, inputWidth, inputHeight, inputStride, outPixels, desiredWidth, desiredHeight);
}

// L8, A8
void PointSample1BPP(const uint8_t* inPixels,
                     uint32_t       inputWidth,
                     uint32_t       inputHeight,
                     uint32_t       inputStride,
                     uint8_t*       outPixels,
                     uint32_t       desiredWidth,
                     uint32_t       desiredHeight)
{
  PointSampleAddressablePixels<uint8_t>(inPixels, inputWidth, inputHeight, inputStride, outPixels, desiredWidth, desiredHeight);
}

/* RGB888
 * RGB888 is a special case as its pixels are not aligned addressable units.
 */
void PointSample3BPP(const uint8_t* inPixels,
                     uint32_t       inputWidth,
                     uint32_t       inputHeight,
                     uint32_t       inputStride,
                     uint8_t*       outPixels,
                     uint32_t       desiredWidth,
                     uint32_t       desiredHeight)
{
  if(inputWidth < 1u || inputHeight < 1u || desiredWidth < 1u || desiredHeight < 1u)
  {
    return;
  }
  const uint32_t BYTES_PER_PIXEL = 3;

  // Generate fixed-point 16.16 deltas in input image coordinates:
  const uint32_t deltaX = (inputWidth << 16u) / desiredWidth;
  const uint32_t deltaY = (inputHeight << 16u) / desiredHeight;

  // Step through output image in whole integer pixel steps while tracking the
  // corresponding locations in the input image using 16.16 fixed-point
  // coordinates:
  uint32_t inY = 0; //< 16.16 fixed-point input image y-coord.
  for(uint32_t outY = 0; outY < desiredHeight; ++outY)
  {
    const uint32_t       integerY    = (inY + (1u << 15u)) >> 16u;
    const uint8_t* const inScanline  = &inPixels[inputStride * integerY * BYTES_PER_PIXEL];
    uint8_t* const       outScanline = &outPixels[desiredWidth * outY * BYTES_PER_PIXEL];
    uint32_t             inX         = 0; //< 16.16 fixed-point input image x-coord.

    for(uint32_t outX = 0; outX < desiredWidth * BYTES_PER_PIXEL; outX += BYTES_PER_PIXEL)
    {
      // Round the fixed-point input coordinate to the address of the input pixel to sample:
      const uint32_t       integerX       = (inX + (1u << 15u)) >> 16u;
      const uint8_t* const inPixelAddress = &inScanline[integerX * BYTES_PER_PIXEL];

      // Issue loads for all pixel color components up-front:
      const uint32_t c0 = inPixelAddress[0];
      const uint32_t c1 = inPixelAddress[1];
      const uint32_t c2 = inPixelAddress[2];
      ///@ToDo: Optimise - Benchmark one 32bit load that will be unaligned 2/3 of the time + 3 rotate and masks, versus these three aligned byte loads, versus using an RGB packed, aligned(1) struct and letting compiler pick a strategy.

      // Output the pixel components:
      outScanline[outX]     = static_cast<uint8_t>(c0);
      outScanline[outX + 1] = static_cast<uint8_t>(c1);
      outScanline[outX + 2] = static_cast<uint8_t>(c2);

      // Increment the fixed-point input coordinate:
      inX += deltaX;
    }

    inY += deltaY;
  }
}

// Dispatch to a format-appropriate point sampling function:
void PointSample(const uint8_t* inPixels,
                 uint32_t       inputWidth,
                 uint32_t       inputHeight,
                 uint32_t       inputStride,
                 Pixel::Format  pixelFormat,
                 uint8_t*       outPixels,
                 uint32_t       desiredWidth,
                 uint32_t       desiredHeight)
{
  // Check the pixel format is one that is supported:
  if(pixelFormat == Pixel::RGBA8888 || pixelFormat == Pixel::RGB888 || pixelFormat == Pixel::RGB565 || pixelFormat == Pixel::LA88 || pixelFormat == Pixel::L8 || pixelFormat == Pixel::A8)
  {
    switch(pixelFormat)
    {
      case Pixel::RGB888:
      {
        PointSample3BPP(inPixels, inputWidth, inputHeight, inputStride, outPixels, desiredWidth, desiredHeight);
        break;
      }
      case Pixel::RGBA8888:
      {
        PointSample4BPP(inPixels, inputWidth, inputHeight, inputStride, outPixels, desiredWidth, desiredHeight);
        break;
      }
      case Pixel::RGB565:
      case Pixel::LA88:
      {
        PointSample2BPP(inPixels, inputWidth, inputHeight, inputStride, outPixels, desiredWidth, desiredHeight);
        break;
      }
      case Pixel::L8:
      case Pixel::A8:
      {
        PointSample1BPP(inPixels, inputWidth, inputHeight, inputStride, outPixels, desiredWidth, desiredHeight);
        break;
      }
      default:
      {
        DALI_ASSERT_DEBUG(0 == "Inner branch conditions don't match outer branch.");
      }
    }
  }
  else
  {
    DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "Bitmap was not point sampled: unsupported pixel format: %u.\n", uint32_t(pixelFormat));
  }
}

// Linear sampling group below

namespace
{
/** @brief Blend 4 pixels together using horizontal and vertical weights. */
inline uint8_t BilinearFilter1BPPByte(uint8_t tl, uint8_t tr, uint8_t bl, uint8_t br, uint32_t fractBlendHorizontal, uint32_t fractBlendVertical)
{
  return static_cast<uint8_t>(BilinearFilter1Component(tl, tr, bl, br, fractBlendHorizontal, fractBlendVertical));
}

/** @copydoc BilinearFilter1BPPByte */
inline Pixel2Bytes BilinearFilter2Bytes(Pixel2Bytes tl, Pixel2Bytes tr, Pixel2Bytes bl, Pixel2Bytes br, uint32_t fractBlendHorizontal, uint32_t fractBlendVertical)
{
  Pixel2Bytes pixel;
  pixel.l = static_cast<uint8_t>(BilinearFilter1Component(tl.l, tr.l, bl.l, br.l, fractBlendHorizontal, fractBlendVertical));
  pixel.a = static_cast<uint8_t>(BilinearFilter1Component(tl.a, tr.a, bl.a, br.a, fractBlendHorizontal, fractBlendVertical));
  return pixel;
}

/** @copydoc BilinearFilter1BPPByte */
inline Pixel3Bytes BilinearFilterRGB888(Pixel3Bytes tl, Pixel3Bytes tr, Pixel3Bytes bl, Pixel3Bytes br, uint32_t fractBlendHorizontal, uint32_t fractBlendVertical)
{
  Pixel3Bytes pixel;
  pixel.r = static_cast<uint8_t>(BilinearFilter1Component(tl.r, tr.r, bl.r, br.r, fractBlendHorizontal, fractBlendVertical));
  pixel.g = static_cast<uint8_t>(BilinearFilter1Component(tl.g, tr.g, bl.g, br.g, fractBlendHorizontal, fractBlendVertical));
  pixel.b = static_cast<uint8_t>(BilinearFilter1Component(tl.b, tr.b, bl.b, br.b, fractBlendHorizontal, fractBlendVertical));
  return pixel;
}

/** @copydoc BilinearFilter1BPPByte */
inline PixelRGB565 BilinearFilterRGB565(PixelRGB565 tl, PixelRGB565 tr, PixelRGB565 bl, PixelRGB565 br, uint32_t fractBlendHorizontal, uint32_t fractBlendVertical)
{
  const PixelRGB565 pixel = static_cast<PixelRGB565>((BilinearFilter1Component(tl >> 11u, tr >> 11u, bl >> 11u, br >> 11u, fractBlendHorizontal, fractBlendVertical) << 11u) +
                                                     (BilinearFilter1Component((tl >> 5u) & 63u, (tr >> 5u) & 63u, (bl >> 5u) & 63u, (br >> 5u) & 63u, fractBlendHorizontal, fractBlendVertical) << 5u) +
                                                     BilinearFilter1Component(tl & 31u, tr & 31u, bl & 31u, br & 31u, fractBlendHorizontal, fractBlendVertical));
  return pixel;
}

/** @copydoc BilinearFilter1BPPByte */
inline Pixel4Bytes BilinearFilter4Bytes(Pixel4Bytes tl, Pixel4Bytes tr, Pixel4Bytes bl, Pixel4Bytes br, uint32_t fractBlendHorizontal, uint32_t fractBlendVertical)
{
  Pixel4Bytes pixel;
  pixel.r = static_cast<uint8_t>(BilinearFilter1Component(tl.r, tr.r, bl.r, br.r, fractBlendHorizontal, fractBlendVertical));
  pixel.g = static_cast<uint8_t>(BilinearFilter1Component(tl.g, tr.g, bl.g, br.g, fractBlendHorizontal, fractBlendVertical));
  pixel.b = static_cast<uint8_t>(BilinearFilter1Component(tl.b, tr.b, bl.b, br.b, fractBlendHorizontal, fractBlendVertical));
  pixel.a = static_cast<uint8_t>(BilinearFilter1Component(tl.a, tr.a, bl.a, br.a, fractBlendHorizontal, fractBlendVertical));
  return pixel;
}

/**
 * @brief Generic version of bilinear sampling image resize function.
 * @note Limited to one compilation unit and exposed through type-specific
 * wrapper functions below.
 */
template<
  typename PIXEL,
  PIXEL (*BilinearFilter)(PIXEL tl, PIXEL tr, PIXEL bl, PIXEL br, uint32_t fractBlendHorizontal, uint32_t fractBlendVertical),
  bool DEBUG_ASSERT_ALIGNMENT>
inline void LinearSampleGeneric(const uint8_t* __restrict__ inPixels,
                                ImageDimensions inputDimensions,
                                uint32_t        inputStride,
                                uint8_t* __restrict__ outPixels,
                                ImageDimensions desiredDimensions)
{
  const uint32_t inputWidth    = inputDimensions.GetWidth();
  const uint32_t inputHeight   = inputDimensions.GetHeight();
  const uint32_t desiredWidth  = desiredDimensions.GetWidth();
  const uint32_t desiredHeight = desiredDimensions.GetHeight();

  DALI_ASSERT_DEBUG(((outPixels >= inPixels + inputStride * inputHeight * sizeof(PIXEL)) ||
                     (inPixels >= outPixels + desiredWidth * desiredHeight * sizeof(PIXEL))) &&
                    "Input and output buffers cannot overlap.");
  if(DEBUG_ASSERT_ALIGNMENT)
  {
    DALI_ASSERT_DEBUG(reinterpret_cast<uint64_t>(inPixels) % sizeof(PIXEL) == 0 && "Pixel pointers need to be aligned to the size of the pixels (E.g., 4 bytes for RGBA, 2 bytes for RGB565, ...).");
    DALI_ASSERT_DEBUG(reinterpret_cast<uint64_t>(outPixels) % sizeof(PIXEL) == 0 && "Pixel pointers need to be aligned to the size of the pixels (E.g., 4 bytes for RGBA, 2 bytes for RGB565, ...).");
  }

  if(inputWidth < 1u || inputHeight < 1u || desiredWidth < 1u || desiredHeight < 1u)
  {
    return;
  }
  const PIXEL* const inAligned  = reinterpret_cast<const PIXEL*>(inPixels);
  PIXEL* const       outAligned = reinterpret_cast<PIXEL*>(outPixels);
  const uint32_t     deltaX     = (inputWidth << 16u) / desiredWidth;
  const uint32_t     deltaY     = (inputHeight << 16u) / desiredHeight;

  uint32_t inY = 0;
  for(uint32_t outY = 0; outY < desiredHeight; ++outY)
  {
    PIXEL* const outScanline = &outAligned[desiredWidth * outY];

    // Find the two scanlines to blend and the weight to blend with:
    const uint32_t integerY1    = inY >> 16u;
    const uint32_t integerY2    = integerY1 + 1 >= inputHeight ? integerY1 : integerY1 + 1;
    const uint32_t inputYWeight = inY & 65535u;

    DALI_ASSERT_DEBUG(integerY1 < inputHeight);
    DALI_ASSERT_DEBUG(integerY2 < inputHeight);

    const PIXEL* const inScanline1 = &inAligned[inputStride * integerY1];
    const PIXEL* const inScanline2 = &inAligned[inputStride * integerY2];

    uint32_t inX = 0;
    for(uint32_t outX = 0; outX < desiredWidth; ++outX)
    {
      // Work out the two pixel scanline offsets for this cluster of four samples:
      const uint32_t integerX1 = inX >> 16u;
      const uint32_t integerX2 = integerX1 + 1 >= inputWidth ? integerX1 : integerX1 + 1;

      // Execute the loads:
      const PIXEL pixel1 = inScanline1[integerX1];
      const PIXEL pixel2 = inScanline2[integerX1];
      const PIXEL pixel3 = inScanline1[integerX2];
      const PIXEL pixel4 = inScanline2[integerX2];
      ///@ToDo Optimise - for 1 and 2  and 4 byte types to execute a single 2, 4, or 8 byte load per pair (caveat clamping) and let half of them be unaligned.

      // Weighted bilinear filter:
      const uint32_t inputXWeight = inX & 65535u;
      outScanline[outX]           = BilinearFilter(pixel1, pixel3, pixel2, pixel4, inputXWeight, inputYWeight);

      inX += deltaX;
    }
    inY += deltaY;
  }
}

} // namespace

// Format-specific linear scaling instantiations:

void LinearSample1BPP(const uint8_t* __restrict__ inPixels,
                      ImageDimensions inputDimensions,
                      uint32_t        inputStride,
                      uint8_t* __restrict__ outPixels,
                      ImageDimensions desiredDimensions)
{
  LinearSampleGeneric<uint8_t, BilinearFilter1BPPByte, false>(inPixels, inputDimensions, inputStride, outPixels, desiredDimensions);
}

void LinearSample2BPP(const uint8_t* __restrict__ inPixels,
                      ImageDimensions inputDimensions,
                      uint32_t        inputStride,
                      uint8_t* __restrict__ outPixels,
                      ImageDimensions desiredDimensions)
{
  LinearSampleGeneric<Pixel2Bytes, BilinearFilter2Bytes, true>(inPixels, inputDimensions, inputStride, outPixels, desiredDimensions);
}

void LinearSampleRGB565(const uint8_t* __restrict__ inPixels,
                        ImageDimensions inputDimensions,
                        uint32_t        inputStride,
                        uint8_t* __restrict__ outPixels,
                        ImageDimensions desiredDimensions)
{
  LinearSampleGeneric<PixelRGB565, BilinearFilterRGB565, true>(inPixels, inputDimensions, inputStride, outPixels, desiredDimensions);
}

void LinearSample3BPP(const uint8_t* __restrict__ inPixels,
                      ImageDimensions inputDimensions,
                      uint32_t        inputStride,
                      uint8_t* __restrict__ outPixels,
                      ImageDimensions desiredDimensions)
{
  LinearSampleGeneric<Pixel3Bytes, BilinearFilterRGB888, false>(inPixels, inputDimensions, inputStride, outPixels, desiredDimensions);
}

void LinearSample4BPP(const uint8_t* __restrict__ inPixels,
                      ImageDimensions inputDimensions,
                      uint32_t        inputStride,
                      uint8_t* __restrict__ outPixels,
                      ImageDimensions desiredDimensions)
{
  LinearSampleGeneric<Pixel4Bytes, BilinearFilter4Bytes, true>(inPixels, inputDimensions, inputStride, outPixels, desiredDimensions);
}

// Dispatch to a format-appropriate linear sampling function:
void LinearSample(const uint8_t* __restrict__ inPixels,
                  ImageDimensions inDimensions,
                  uint32_t        inStride,
                  Pixel::Format   pixelFormat,
                  uint8_t* __restrict__ outPixels,
                  ImageDimensions outDimensions)
{
  // Check the pixel format is one that is supported:
  if(pixelFormat == Pixel::RGB888 || pixelFormat == Pixel::RGBA8888 || pixelFormat == Pixel::L8 || pixelFormat == Pixel::A8 || pixelFormat == Pixel::LA88 || pixelFormat == Pixel::RGB565)
  {
    switch(pixelFormat)
    {
      case Pixel::RGB888:
      {
        LinearSample3BPP(inPixels, inDimensions, inStride, outPixels, outDimensions);
        break;
      }
      case Pixel::RGBA8888:
      {
        LinearSample4BPP(inPixels, inDimensions, inStride, outPixels, outDimensions);
        break;
      }
      case Pixel::L8:
      case Pixel::A8:
      {
        LinearSample1BPP(inPixels, inDimensions, inStride, outPixels, outDimensions);
        break;
      }
      case Pixel::LA88:
      {
        LinearSample2BPP(inPixels, inDimensions, inStride, outPixels, outDimensions);
        break;
      }
      case Pixel::RGB565:
      {
        LinearSampleRGB565(inPixels, inDimensions, inStride, outPixels, outDimensions);
        break;
      }
      default:
      {
        DALI_ASSERT_DEBUG(0 == "Inner branch conditions don't match outer branch.");
      }
    }
  }
  else
  {
    DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "Bitmap was not linear sampled: unsupported pixel format: %u.\n", uint32_t(pixelFormat));
  }
}

void Resample(const uint8_t* __restrict__ inPixels,
              ImageDimensions inputDimensions,
              uint32_t        inputStride,
              uint8_t* __restrict__ outPixels,
              ImageDimensions   desiredDimensions,
              Resampler::Filter filterType,
              int               numChannels,
              bool              hasAlpha)
{
  // Got from the test.cpp of the ImageResampler lib.
  const float ONE_DIV_255               = 1.0f / 255.0f;
  const int   MAX_UNSIGNED_CHAR         = std::numeric_limits<uint8_t>::max();
  const int   LINEAR_TO_SRGB_TABLE_SIZE = 4096;
  const int   ALPHA_CHANNEL             = hasAlpha ? (numChannels - 1) : 0;

  static bool    loadColorSpaces = true;
  static float   srgbToLinear[MAX_UNSIGNED_CHAR + 1];
  static uint8_t linearToSrgb[LINEAR_TO_SRGB_TABLE_SIZE];

  if(loadColorSpaces) // Only create the color space conversions on the first execution
  {
    loadColorSpaces = false;

    for(int i = 0; i <= MAX_UNSIGNED_CHAR; ++i)
    {
      srgbToLinear[i] = pow(static_cast<float>(i) * ONE_DIV_255, DEFAULT_SOURCE_GAMMA);
    }

    const float invLinearToSrgbTableSize = 1.0f / static_cast<float>(LINEAR_TO_SRGB_TABLE_SIZE);
    const float invSourceGamma           = 1.0f / DEFAULT_SOURCE_GAMMA;

    for(int i = 0; i < LINEAR_TO_SRGB_TABLE_SIZE; ++i)
    {
      int k = static_cast<int>(255.0f * pow(static_cast<float>(i) * invLinearToSrgbTableSize, invSourceGamma) + 0.5f);
      if(k < 0)
      {
        k = 0;
      }
      else if(k > MAX_UNSIGNED_CHAR)
      {
        k = MAX_UNSIGNED_CHAR;
      }
      linearToSrgb[i] = static_cast<uint8_t>(k);
    }
  }

  std::vector<Resampler*>    resamplers(numChannels);
  std::vector<Vector<float>> samples(numChannels);

  const int srcWidth  = inputDimensions.GetWidth();
  const int srcHeight = inputDimensions.GetHeight();
  const int dstWidth  = desiredDimensions.GetWidth();
  const int dstHeight = desiredDimensions.GetHeight();

  // Now create a Resampler instance for each component to process. The first instance will create new contributor tables, which are shared by the resamplers
  // used for the other components (a memory and slight cache efficiency optimization).
  resamplers[0] = new Resampler(srcWidth,
                                srcHeight,
                                dstWidth,
                                dstHeight,
                                Resampler::BOUNDARY_CLAMP,
                                0.0f,          // sample_low,
                                1.0f,          // sample_high. Clamp output samples to specified range, or disable clamping if sample_low >= sample_high.
                                filterType,    // The type of filter.
                                NULL,          // Pclist_x,
                                NULL,          // Pclist_y. Optional pointers to contributor lists from another instance of a Resampler.
                                FILTER_SCALE,  // src_x_ofs,
                                FILTER_SCALE); // src_y_ofs. Offset input image by specified amount (fractional values okay).
  samples[0].ResizeUninitialized(srcWidth);
  for(int i = 1; i < numChannels; ++i)
  {
    resamplers[i] = new Resampler(srcWidth,
                                  srcHeight,
                                  dstWidth,
                                  dstHeight,
                                  Resampler::BOUNDARY_CLAMP,
                                  0.0f,
                                  1.0f,
                                  filterType,
                                  resamplers[0]->get_clist_x(),
                                  resamplers[0]->get_clist_y(),
                                  FILTER_SCALE,
                                  FILTER_SCALE);
    samples[i].ResizeUninitialized(srcWidth);
  }

  const int srcPitch = inputStride * numChannels;
  const int dstPitch = dstWidth * numChannels;
  int       dstY     = 0;

  for(int srcY = 0; srcY < srcHeight; ++srcY)
  {
    const uint8_t* pSrc = &inPixels[srcY * srcPitch];

    for(int x = 0; x < srcWidth; ++x)
    {
      for(int c = 0; c < numChannels; ++c)
      {
        if(c == ALPHA_CHANNEL && hasAlpha)
        {
          samples[c][x] = *pSrc++ * ONE_DIV_255;
        }
        else
        {
          samples[c][x] = srgbToLinear[*pSrc++];
        }
      }
    }

    for(int c = 0; c < numChannels; ++c)
    {
      if(!resamplers[c]->put_line(&samples[c][0]))
      {
        DALI_ASSERT_DEBUG(!"Out of memory");
      }
    }

    for(;;)
    {
      int compIndex;
      for(compIndex = 0; compIndex < numChannels; ++compIndex)
      {
        const float* pOutputSamples = resamplers[compIndex]->get_line();
        if(!pOutputSamples)
        {
          break;
        }

        const bool isAlphaChannel = (compIndex == ALPHA_CHANNEL && hasAlpha);
        DALI_ASSERT_DEBUG(dstY < dstHeight);
        uint8_t* pDst = &outPixels[dstY * dstPitch + compIndex];

        for(int x = 0; x < dstWidth; ++x)
        {
          if(isAlphaChannel)
          {
            int c = static_cast<int>(255.0f * pOutputSamples[x] + 0.5f);
            if(c < 0)
            {
              c = 0;
            }
            else if(c > MAX_UNSIGNED_CHAR)
            {
              c = MAX_UNSIGNED_CHAR;
            }
            *pDst = static_cast<uint8_t>(c);
          }
          else
          {
            int j = static_cast<int>(LINEAR_TO_SRGB_TABLE_SIZE * pOutputSamples[x] + 0.5f);
            if(j < 0)
            {
              j = 0;
            }
            else if(j >= LINEAR_TO_SRGB_TABLE_SIZE)
            {
              j = LINEAR_TO_SRGB_TABLE_SIZE - 1;
            }
            *pDst = linearToSrgb[j];
          }

          pDst += numChannels;
        }
      }
      if(compIndex < numChannels)
      {
        break;
      }

      ++dstY;
    }
  }

  // Delete the resamplers.
  for(int i = 0; i < numChannels; ++i)
  {
    delete resamplers[i];
  }
}

void LanczosSample4BPP(const uint8_t* __restrict__ inPixels,
                       ImageDimensions inputDimensions,
                       uint32_t        inputStride,
                       uint8_t* __restrict__ outPixels,
                       ImageDimensions desiredDimensions)
{
  Resample(inPixels, inputDimensions, inputStride, outPixels, desiredDimensions, Resampler::LANCZOS4, 4, true);
}

void LanczosSample1BPP(const uint8_t* __restrict__ inPixels,
                       ImageDimensions inputDimensions,
                       uint32_t        inputStride,
                       uint8_t* __restrict__ outPixels,
                       ImageDimensions desiredDimensions)
{
  // For L8 images
  Resample(inPixels, inputDimensions, inputStride, outPixels, desiredDimensions, Resampler::LANCZOS4, 1, false);
}

// Dispatch to a format-appropriate third-party resampling function:
void LanczosSample(const uint8_t* __restrict__ inPixels,
                   ImageDimensions inDimensions,
                   uint32_t        inStride,
                   Pixel::Format   pixelFormat,
                   uint8_t* __restrict__ outPixels,
                   ImageDimensions outDimensions)
{
  // Check the pixel format is one that is supported:
  if(pixelFormat == Pixel::RGBA8888 || pixelFormat == Pixel::BGRA8888 || pixelFormat == Pixel::L8 || pixelFormat == Pixel::A8)
  {
    switch(pixelFormat)
    {
      case Pixel::RGBA8888:
      case Pixel::BGRA8888:
      {
        LanczosSample4BPP(inPixels, inDimensions, inStride, outPixels, outDimensions);
        break;
      }
      case Pixel::L8:
      case Pixel::A8:
      {
        LanczosSample1BPP(inPixels, inDimensions, inStride, outPixels, outDimensions);
        break;
      }
      default:
      {
        DALI_ASSERT_DEBUG(0 == "Inner branch conditions don't match outer branch.");
      }
    }
  }
  else
  {
    DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "Bitmap was not lanczos sampled: unsupported pixel format: %u.\n", static_cast<uint32_t>(pixelFormat));
  }
}

void RotateByShear(const uint8_t* const pixelsIn,
                   uint32_t             widthIn,
                   uint32_t             heightIn,
                   uint32_t             strideIn,
                   uint32_t             pixelSize,
                   float                radians,
                   uint8_t*&            pixelsOut,
                   uint32_t&            widthOut,
                   uint32_t&            heightOut)
{
  // @note Code got from https://www.codeproject.com/Articles/202/High-quality-image-rotation-rotate-by-shear by Eran Yariv.

  // Do first the fast rotations to transform the angle into a (-45..45] range.

  bool fastRotationPerformed = false;
  if((radians > Math::PI_4) && (radians <= RAD_135))
  {
    // Angle in (45.0 .. 135.0]
    // Rotate image by 90 degrees into temporary image,
    // so it requires only an extra rotation angle
    // of -45.0 .. +45.0 to complete rotation.
    fastRotationPerformed = Rotate90(pixelsIn,
                                     widthIn,
                                     heightIn,
                                     strideIn,
                                     pixelSize,
                                     pixelsOut,
                                     widthOut,
                                     heightOut);

    if(!fastRotationPerformed)
    {
      DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "fast rotation failed\n");
      // The fast rotation failed.
      return;
    }

    radians -= Math::PI_2;
  }
  else if((radians > RAD_135) && (radians <= RAD_225))
  {
    // Angle in (135.0 .. 225.0]
    // Rotate image by 180 degrees into temporary image,
    // so it requires only an extra rotation angle
    // of -45.0 .. +45.0 to complete rotation.

    fastRotationPerformed = Rotate180(pixelsIn,
                                      widthIn,
                                      heightIn,
                                      strideIn,
                                      pixelSize,
                                      pixelsOut);

    if(!fastRotationPerformed)
    {
      DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "fast rotation failed\n");
      // The fast rotation failed.
      return;
    }

    radians -= Math::PI;
    widthOut  = widthIn;
    heightOut = heightIn;
  }
  else if((radians > RAD_225) && (radians <= RAD_315))
  {
    // Angle in (225.0 .. 315.0]
    // Rotate image by 270 degrees into temporary image,
    // so it requires only an extra rotation angle
    // of -45.0 .. +45.0 to complete rotation.

    fastRotationPerformed = Rotate270(pixelsIn,
                                      widthIn,
                                      heightIn,
                                      strideIn,
                                      pixelSize,
                                      pixelsOut,
                                      widthOut,
                                      heightOut);

    if(!fastRotationPerformed)
    {
      DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "fast rotation failed\n");
      // The fast rotation failed.
      return;
    }

    radians -= RAD_270;
  }

  if(fabs(radians) < Dali::Math::MACHINE_EPSILON_10)
  {
    // Nothing else to do if the angle is zero.
    // The rotation angle was 90, 180 or 270.

    // @note Allocated memory by 'Fast Rotations', if any, has to be freed by the called to this function.
    return;
  }

  const uint8_t* const                      firstHorizontalSkewPixelsIn = fastRotationPerformed ? pixelsOut : pixelsIn;
  std::unique_ptr<uint8_t, void (*)(void*)> tmpPixelsInPtr((fastRotationPerformed ? pixelsOut : nullptr), free);

  uint32_t stride = fastRotationPerformed ? widthOut : strideIn;

  // Reset the input/output
  widthIn   = widthOut;
  heightIn  = heightOut;
  pixelsOut = nullptr;

  const float angleSinus   = sin(radians);
  const float angleCosinus = cos(radians);
  const float angleTangent = tan(0.5f * radians);

  ///////////////////////////////////////
  // Perform 1st shear (horizontal)
  ///////////////////////////////////////

  // Calculate first shear (horizontal) destination image dimensions

  widthOut  = widthIn + static_cast<uint32_t>(fabs(angleTangent) * static_cast<float>(heightIn));
  heightOut = heightIn;

  // Allocate the buffer for the 1st shear
  pixelsOut = static_cast<uint8_t*>(malloc(widthOut * heightOut * pixelSize));

  if(nullptr == pixelsOut)
  {
    widthOut  = 0u;
    heightOut = 0u;

    DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "malloc failed to allocate memory\n");

    // The deleter of the tmpPixelsInPtr unique pointer is called freeing the memory allocated by the 'Fast rotations'.
    // Nothing else to do if the memory allocation fails.
    return;
  }

  for(uint32_t y = 0u; y < heightOut; ++y)
  {
    const float shear = angleTangent * ((angleTangent >= 0.f) ? (0.5f + static_cast<float>(y)) : (0.5f + static_cast<float>(y) - static_cast<float>(heightOut)));

    const int intShear = static_cast<int>(floor(shear));
    HorizontalSkew(firstHorizontalSkewPixelsIn, widthIn, stride, pixelSize, pixelsOut, widthOut, y, intShear, shear - static_cast<float>(intShear));
  }

  // Reset the 'pixel in' pointer with the output of the 'First Horizontal Skew' and free the memory allocated by the 'Fast Rotations'.
  tmpPixelsInPtr.reset(pixelsOut);
  uint32_t tmpWidthIn  = widthOut;
  uint32_t tmpHeightIn = heightOut;

  // Reset the input/output
  pixelsOut = nullptr;

  ///////////////////////////////////////
  // Perform 2nd shear (vertical)
  ///////////////////////////////////////

  // Calc 2nd shear (vertical) destination image dimensions
  heightOut = static_cast<uint32_t>(static_cast<float>(widthIn) * fabs(angleSinus) + static_cast<float>(heightIn) * angleCosinus);

  // Allocate the buffer for the 2nd shear
  pixelsOut = static_cast<uint8_t*>(malloc(widthOut * heightOut * pixelSize));

  if(nullptr == pixelsOut)
  {
    widthOut  = 0u;
    heightOut = 0u;

    DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "malloc failed to allocate memory\n");
    // The deleter of the tmpPixelsInPtr unique pointer is called freeing the memory allocated by the 'First Horizontal Skew'.
    // Nothing else to do if the memory allocation fails.
    return;
  }

  // Variable skew offset
  float offset = angleSinus * ((angleSinus > 0.f) ? static_cast<float>(widthIn - 1u) : -(static_cast<float>(widthIn) - static_cast<float>(widthOut)));

  uint32_t column = 0u;
  for(column = 0u; column < widthOut; ++column, offset -= angleSinus)
  {
    const int32_t shear = static_cast<int32_t>(floor(offset));
    VerticalSkew(tmpPixelsInPtr.get(), tmpWidthIn, tmpHeightIn, tmpWidthIn, pixelSize, pixelsOut, widthOut, heightOut, column, shear, offset - static_cast<float>(shear));
  }
  // Reset the 'pixel in' pointer with the output of the 'Vertical Skew' and free the memory allocated by the 'First Horizontal Skew'.
  // Reset the input/output
  tmpPixelsInPtr.reset(pixelsOut);
  tmpWidthIn  = widthOut;
  tmpHeightIn = heightOut;
  pixelsOut   = nullptr;

  ///////////////////////////////////////
  // Perform 3rd shear (horizontal)
  ///////////////////////////////////////

  // Calc 3rd shear (horizontal) destination image dimensions
  widthOut = static_cast<uint32_t>(static_cast<float>(heightIn) * fabs(angleSinus) + static_cast<float>(widthIn) * angleCosinus) + 1u;

  // Allocate the buffer for the 3rd shear
  pixelsOut = static_cast<uint8_t*>(malloc(widthOut * heightOut * pixelSize));

  if(nullptr == pixelsOut)
  {
    widthOut  = 0u;
    heightOut = 0u;

    DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "malloc failed to allocate memory\n");
    // The deleter of the tmpPixelsInPtr unique pointer is called freeing the memory allocated by the 'Vertical Skew'.
    // Nothing else to do if the memory allocation fails.
    return;
  }

  offset = (angleSinus >= 0.f) ? -angleSinus * angleTangent * static_cast<float>(widthIn - 1u) : angleTangent * (static_cast<float>(widthIn - 1u) * -angleSinus + (1.f - static_cast<float>(heightOut)));

  for(uint32_t y = 0u; y < heightOut; ++y, offset += angleTangent)
  {
    const int32_t shear = static_cast<int32_t>(floor(offset));
    HorizontalSkew(tmpPixelsInPtr.get(), tmpWidthIn, tmpWidthIn, pixelSize, pixelsOut, widthOut, y, shear, offset - static_cast<float>(shear));
  }

  // The deleter of the tmpPixelsInPtr unique pointer is called freeing the memory allocated by the 'Vertical Skew'.
  // @note Allocated memory by the last 'Horizontal Skew' has to be freed by the caller to this function.
}

void HorizontalShear(const uint8_t* const pixelsIn,
                     uint32_t             widthIn,
                     uint32_t             heightIn,
                     uint32_t             strideIn,
                     uint32_t             pixelSize,
                     float                radians,
                     uint8_t*&            pixelsOut,
                     uint32_t&            widthOut,
                     uint32_t&            heightOut)
{
  // Calculate the destination image dimensions.

  const float absRadians = fabs(radians);

  if(absRadians > Math::PI_4)
  {
    // Can't shear more than 45 degrees.
    widthOut  = 0u;
    heightOut = 0u;

    DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "Can't shear more than 45 degrees (PI/4 radians). radians : %f\n", radians);
    return;
  }

  widthOut  = widthIn + static_cast<uint32_t>(ceil(absRadians * static_cast<float>(heightIn)));
  heightOut = heightIn;

  // Allocate the buffer for the shear.
  pixelsOut = static_cast<uint8_t*>(malloc(widthOut * heightOut * pixelSize));

  if(nullptr == pixelsOut)
  {
    widthOut  = 0u;
    heightOut = 0u;

    DALI_LOG_INFO(gImageOpsLogFilter, Dali::Integration::Log::Verbose, "malloc failed to allocate memory\n");
    return;
  }

  for(uint32_t y = 0u; y < heightOut; ++y)
  {
    const float shear = radians * ((radians >= 0.f) ? (0.5f + static_cast<float>(y)) : (0.5f + static_cast<float>(y) - static_cast<float>(heightOut)));

    const int32_t intShear = static_cast<int32_t>(floor(shear));
    HorizontalSkew(pixelsIn, widthIn, strideIn, pixelSize, pixelsOut, widthOut, y, intShear, shear - static_cast<float>(intShear));
  }
}

} /* namespace Platform */
} /* namespace Internal */
} /* namespace Dali */
