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
#include <dali/internal/imaging/common/pixel-buffer-operations.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/pixel-buffer-integ.h>
#include <dali/integration-api/trace.h>
#include <dali/public-api/common/dali-utility.h>
#include <stdlib.h>
#include <cstring>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/alpha-mask.h>
#include <dali/internal/imaging/common/gaussian-blur.h>
#include <dali/internal/imaging/common/image-operations.h>
#include <dali/internal/imaging/common/pixel-manipulation.h>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
namespace PixelBufferOperations
{
namespace
{
DALI_INIT_TRACE_FILTER(gTraceFilter, DALI_TRACE_IMAGE_PERFORMANCE_MARKER, false);

const float TWO_PI = 2.f * Math::PI; ///< 360 degrees in radians
// based on W3C Recommendations (https://www.w3.org/TR/AERT/#color-contrast)
constexpr uint32_t BRIGHTNESS_CONSTANT_R = 299;
constexpr uint32_t BRIGHTNESS_CONSTANT_G = 587;
constexpr uint32_t BRIGHTNESS_CONSTANT_B = 114;

/**
 * @brief Whether the resampler used by NewResize() supports the given pixel format.
 * It only really works for 8 bit wide channels. (But could be expanded to work)
 */
bool IsResizeSupportedPixelFormat(Pixel::Format pixelFormat)
{
  switch(pixelFormat)
  {
    case Pixel::A8:
    case Pixel::L8:
    case Pixel::CHROMINANCE_U:
    case Pixel::CHROMINANCE_V:
    case Pixel::LA88:
    case Pixel::RGB888:
    case Pixel::RGB8888:
    case Pixel::BGR8888:
    case Pixel::RGBA8888:
    case Pixel::BGRA8888: // FALL THROUGH
    {
      return true;
    }
    default:
    {
      // This pixel format is not supported for this operation.
      return false;
    }
  }
}

/**
 * Internal method to apply the mask to a buffer. Expects that they are the same size.
 */
void ApplyMaskInternal(Dali::PixelBuffer& pixelBuffer, Dali::PixelBuffer& mask)
{
  int byteOffset = 0;
  int bitMask    = 0;

  Dali::Pixel::GetAlphaOffsetAndMask(pixelBuffer.GetPixelFormat(), byteOffset, bitMask);
  if(Dali::Pixel::HasAlpha(pixelBuffer.GetPixelFormat()) && bitMask == 255)
  {
    ApplyMaskToAlphaChannel(pixelBuffer, mask);
  }
  else
  {
    Dali::PixelBuffer newPixelBuffer = CreateNewMaskedBuffer(pixelBuffer, mask);
    Dali::Integration::TakePixelBufferOwnership(pixelBuffer, newPixelBuffer);
    // On leaving scope, newPixelBuffer will get destroyed.
  }
}

/**
 * Scales a buffer by the given factor, and crops at the center to the
 * given dimensions.
 */
void ScaleAndCrop(Dali::PixelBuffer& pixelBuffer, float scaleFactor, ImageDimensions cropDimensions)
{
  ImageDimensions outDimensions(static_cast<uint32_t>(float(pixelBuffer.GetWidth()) * scaleFactor),
                                static_cast<uint32_t>(float(pixelBuffer.GetHeight()) * scaleFactor));

  if(outDimensions.GetWidth() != pixelBuffer.GetWidth() || outDimensions.GetHeight() != pixelBuffer.GetHeight())
  {
    Resize(pixelBuffer, outDimensions);
  }

  ImageDimensions postCropDimensions(
    Min(cropDimensions.GetWidth(), outDimensions.GetWidth()),
    Min(cropDimensions.GetHeight(), outDimensions.GetHeight()));

  if(postCropDimensions.GetWidth() < outDimensions.GetWidth() ||
     postCropDimensions.GetHeight() < outDimensions.GetHeight())
  {
    uint16_t x = (outDimensions.GetWidth() - postCropDimensions.GetWidth()) / 2;
    uint16_t y = (outDimensions.GetHeight() - postCropDimensions.GetHeight()) / 2;
    Crop(pixelBuffer, x, y, postCropDimensions);
  }
}

} // namespace

bool ApplyGaussianBlur(Dali::PixelBuffer& pixelBuffer, float blurRadius)
{
  DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_PIXEL_BUFFER_APPLY_GAUSSIAN_BLUR", [&](std::ostringstream& oss)
  { oss << "[" << pixelBuffer.GetWidth() << "x" << pixelBuffer.GetHeight() << " format " << pixelBuffer.GetPixelFormat() << " blurRadius:" << blurRadius << "]"; });
  // Check first if ApplyGaussianBlur() can perform the operation in the current pixel buffer.

  bool validPixelFormat = false;
  switch(pixelBuffer.GetPixelFormat())
  {
    case Pixel::A8:
    case Pixel::L8:
    case Pixel::CHROMINANCE_U:
    case Pixel::CHROMINANCE_V:
    case Pixel::LA88:
    case Pixel::RGB888:
    case Pixel::RGB8888:
    case Pixel::BGR8888:
    case Pixel::RGBA8888:
    case Pixel::BGRA8888: // FALL THROUGH
    {
      validPixelFormat = true;
      break;
    }
    default:
    {
      // This pixel format is not supported for this operation.
      validPixelFormat = false;
      break;
    }
  }

  bool applied = false;

  if(validPixelFormat)
  {
    if(pixelBuffer.GetWidth() > 0 && pixelBuffer.GetHeight() > 0)
    {
      applied = PerformGaussianBlur(pixelBuffer, blurRadius);
    }
    else
    {
      DALI_LOG_ERROR("Trying to apply gaussian blur to an empty pixel buffer");
    }
  }
  else
  {
    // Can't apply gaussin blur the pixel buffer with the current pixel format.
    DALI_LOG_ERROR("Can't apply gaussian blur to the pixel buffer with the current pixel format : %s\n", Platform::GetPixelFormatName(pixelBuffer.GetPixelFormat()));
  }
  DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_PIXEL_BUFFER_APPLY_GAUSSIAN_BLUR", [&](std::ostringstream& oss)
  { oss << "[valid? " << validPixelFormat << " applied? " << applied << "]"; });

  return applied;
}

bool Resize(Dali::PixelBuffer& pixelBuffer, ImageDimensions outDimensions)
{
  if(pixelBuffer.GetWidth() == outDimensions.GetWidth() && pixelBuffer.GetHeight() == outDimensions.GetHeight())
  {
    // Nothing to do.
    return true;
  }

  if(DALI_UNLIKELY(!IsResizeSupportedPixelFormat(pixelBuffer.GetPixelFormat()) || pixelBuffer.GetBuffer() == nullptr))
  {
    DALI_LOG_ERROR("Trying to resize an image with unsupported pixel format: %s or null buffer: %p\n", Platform::GetPixelFormatName(pixelBuffer.GetPixelFormat()), pixelBuffer.GetBuffer());
    return false;
  }

  Dali::PixelBuffer outBuffer = NewResize(pixelBuffer, outDimensions);
  if(DALI_UNLIKELY(!outBuffer || outBuffer.GetBuffer() == nullptr))
  {
    DALI_LOG_ERROR("Failed to resize the image to %ux%u\n", outDimensions.GetWidth(), outDimensions.GetHeight());
    return false;
  }

  Dali::Integration::TakePixelBufferOwnership(pixelBuffer, outBuffer);
  return true;
}

void Crop(Dali::PixelBuffer& pixelBuffer, uint16_t x, uint16_t y, ImageDimensions cropDimensions)
{
  // Apply crop only if something changed)
  if(!(x == 0u && y == 0u && pixelBuffer.GetWidth() == cropDimensions.GetWidth() && pixelBuffer.GetHeight() == cropDimensions.GetHeight()))
  {
    Dali::PixelBuffer outBuffer = NewCrop(pixelBuffer, x, y, cropDimensions);
    Dali::Integration::TakePixelBufferOwnership(pixelBuffer, outBuffer);
  }
}

void ApplyMask(Dali::PixelBuffer& pixelBuffer, Dali::PixelBuffer mask, float contentScale, bool cropToMask)
{
  DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_PIXEL_BUFFER_APPLY_MASK", [&](std::ostringstream& oss)
  {
    oss << "[image:" << pixelBuffer.GetWidth() << "x" << pixelBuffer.GetHeight() << " bufferSize " << pixelBuffer.GetBufferSize() << " format " << pixelBuffer.GetPixelFormat();
    oss << " mask:" << mask.GetWidth() << "x" << mask.GetHeight() << " contentScale:" << contentScale << " cropToMask: " << cropToMask << "]"; });
  if(cropToMask)
  {
    // First scale this buffer by the contentScale, and crop to the mask size
    // If it's too small, then scale the mask to match the image size
    // Then apply the mask
    ScaleAndCrop(pixelBuffer, contentScale, ImageDimensions(mask.GetWidth(), mask.GetHeight()));

    if(mask.GetWidth() > pixelBuffer.GetWidth() || mask.GetHeight() > pixelBuffer.GetHeight())
    {
      Dali::PixelBuffer resizedMask = NewResize(mask, ImageDimensions(pixelBuffer.GetWidth(), pixelBuffer.GetHeight()));
      ApplyMaskInternal(pixelBuffer, resizedMask);
    }
    else
    {
      ApplyMaskInternal(pixelBuffer, mask);
    }
  }
  else
  {
    // First, scale the mask to match the image size,
    // then apply the mask.
    Dali::PixelBuffer resizedMask = NewResize(mask, ImageDimensions(pixelBuffer.GetWidth(), pixelBuffer.GetHeight()));
    ApplyMaskInternal(pixelBuffer, resizedMask);
  }
  DALI_TRACE_END(gTraceFilter, "DALI_PIXEL_BUFFER_APPLY_MASK");
}

void ApplyCenterCrop(Dali::PixelBuffer& pixelBuffer, uint16_t width, uint16_t height)
{
  const uint32_t selfWidth  = pixelBuffer.GetWidth();
  const uint32_t selfHeight = pixelBuffer.GetHeight();

  DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_PIXEL_BUFFER_APPLY_CROP", [&](std::ostringstream& oss)
  {
    oss << "[image:" << selfWidth << "x" << selfHeight << " bufferSize " << pixelBuffer.GetBufferSize() << " format " << pixelBuffer.GetPixelFormat();
    oss << " target:" << width << "x" << height << "]"; });

  if(selfWidth == 0u || selfHeight == 0u || width == 0u || height == 0u)
  {
    DALI_TRACE_END(gTraceFilter, "DALI_PIXEL_BUFFER_APPLY_CROP");
    return;
  }

  // Use integer cross-multiplication to avoid floating-point precision errors.
  // selfWidth * height > width * selfHeight  ⟺  sourceAspect > targetAspect
  // uint16_t max is 65535; 65535^2 = 4,294,836,225 < UINT32_MAX, so no overflow.
  uint16_t cropWidth;
  uint16_t cropHeight;
  if(selfWidth * static_cast<uint32_t>(height) >
     static_cast<uint32_t>(width) * selfHeight)
  {
    // Source is wider than target: crop the sides
    cropWidth  = static_cast<uint16_t>(static_cast<float>(selfHeight) * static_cast<float>(width) / static_cast<float>(height) + 0.5f);
    cropHeight = static_cast<uint16_t>(selfHeight);
  }
  else
  {
    // Source is taller than target: crop top and bottom
    cropWidth  = static_cast<uint16_t>(selfWidth);
    cropHeight = static_cast<uint16_t>(static_cast<float>(selfWidth) * static_cast<float>(height) / static_cast<float>(width) + 0.5f);
  }

  if(DALI_UNLIKELY(!(static_cast<uint32_t>(cropWidth) <= selfWidth && static_cast<uint32_t>(cropHeight) <= selfHeight)))
  {
    DALI_LOG_ERROR("Cropped image should be smaller than self! something has problem.");
    DALI_TRACE_END(gTraceFilter, "DALI_PIXEL_BUFFER_APPLY_CROP");
    return;
  }

  const uint16_t offsetX = static_cast<uint16_t>((selfWidth - cropWidth) / 2u);
  const uint16_t offsetY = static_cast<uint16_t>((selfHeight - cropHeight) / 2u);

  Crop(pixelBuffer, offsetX, offsetY, ImageDimensions(cropWidth, cropHeight));
  Resize(pixelBuffer, ImageDimensions(width, height));

  DALI_TRACE_END(gTraceFilter, "DALI_PIXEL_BUFFER_APPLY_CROP");
}

void ApplyLetterbox(Dali::PixelBuffer& pixelBuffer, uint16_t width, uint16_t height)
{
  DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_PIXEL_BUFFER_APPLY_LETTER_BOX", [&](std::ostringstream& oss)
  {
    oss << "[image:" << pixelBuffer.GetWidth() << "x" << pixelBuffer.GetHeight() << " bufferSize " << pixelBuffer.GetBufferSize() << " format " << pixelBuffer.GetPixelFormat();
    oss << " target:" << width << "x" << height << "]"; });

  if(pixelBuffer.GetWidth() == 0u || pixelBuffer.GetHeight() == 0u || width == 0u || height == 0u)
  {
    DALI_TRACE_END(gTraceFilter, "DALI_PIXEL_BUFFER_APPLY_LETTER_BOX");
    return;
  }

  // Downscale to fit within desired box while preserving aspect ratio (no upscaling)
  const float scaleX = static_cast<float>(width) / static_cast<float>(pixelBuffer.GetWidth());
  const float scaleY = static_cast<float>(height) / static_cast<float>(pixelBuffer.GetHeight());
  const float scale  = Min(scaleX, scaleY);

  if(scale < 1.0f)
  {
    const uint16_t scaledWidth  = static_cast<uint16_t>(static_cast<float>(pixelBuffer.GetWidth()) * scale + 0.5f);
    const uint16_t scaledHeight = static_cast<uint16_t>(static_cast<float>(pixelBuffer.GetHeight()) * scale + 0.5f);
    Resize(pixelBuffer, ImageDimensions(scaledWidth, scaledHeight));
  }

  const uint32_t selfWidth  = pixelBuffer.GetWidth();
  const uint32_t selfHeight = pixelBuffer.GetHeight();

  if(selfWidth == width && selfHeight == height)
  {
    DALI_TRACE_END(gTraceFilter, "DALI_PIXEL_BUFFER_APPLY_LETTER_BOX");
    return;
  }

  if(DALI_UNLIKELY(!(static_cast<uint32_t>(width) >= selfWidth && static_cast<uint32_t>(height) >= selfHeight)))
  {
    DALI_LOG_ERROR("LetterBox should be bigger than self! something has problem.");
    DALI_TRACE_END(gTraceFilter, "DALI_PIXEL_BUFFER_APPLY_LETTER_BOX");
    return;
  }

  const uint32_t bytesPerPixel = Pixel::GetBytesPerPixel(pixelBuffer.GetPixelFormat());

  if(DALI_LIKELY(bytesPerPixel != 0u && pixelBuffer.GetBuffer() != nullptr))
  {
    // Create target buffer filled with black (0x00) for padding
    Dali::PixelBuffer newBuffer = Dali::PixelBuffer::New(width, height, pixelBuffer.GetPixelFormat());
    memset(newBuffer.GetBuffer(), 0, newBuffer.GetBufferSize());

    // Copy the (scaled or original) image centered into the new buffer
    const uint32_t offsetX         = (static_cast<uint32_t>(width) - selfWidth) / 2u;
    const uint32_t offsetY         = (static_cast<uint32_t>(height) - selfHeight) / 2u;
    const uint32_t srcRowBytes     = selfWidth * bytesPerPixel;
    const uint32_t srcStrideBytes  = pixelBuffer.GetStrideBytes();
    const uint32_t destStrideBytes = newBuffer.GetStrideBytes();

    if(width == selfWidth && destStrideBytes == srcStrideBytes)
    {
      // We can use single memcpy
      const uint8_t* srcBegin  = pixelBuffer.GetBuffer();
      uint8_t*       destBegin = newBuffer.GetBuffer() + offsetY * destStrideBytes;
      memcpy(destBegin, srcBegin, selfHeight * srcStrideBytes);
    }
    else
    {
      for(uint32_t row = 0u; row < selfHeight; ++row)
      {
        const uint8_t* srcRow  = pixelBuffer.GetBuffer() + row * srcStrideBytes;
        uint8_t*       destRow = newBuffer.GetBuffer() + (offsetY + row) * destStrideBytes + offsetX * bytesPerPixel;
        memcpy(destRow, srcRow, srcRowBytes);
      }
    }

    Dali::Integration::TakePixelBufferOwnership(pixelBuffer, newBuffer);
  }
  else
  {
    DALI_LOG_ERROR("Trying to apply letter box an image with unsupported pixel format: %s or null buffer: %p\n", Platform::GetPixelFormatName(pixelBuffer.GetPixelFormat()), pixelBuffer.GetBuffer());
  }

  DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_PIXEL_BUFFER_APPLY_LETTER_BOX", [&](std::ostringstream& oss)
  { oss << "[valid? " << (bytesPerPixel != 0u) << "]"; });
}

bool Rotate(Dali::PixelBuffer& pixelBuffer, Degree angle)
{
  // Check first if Rotate() can perform the operation in the current pixel buffer.

  bool validPixelFormat = false;
  switch(pixelBuffer.GetPixelFormat())
  {
    case Pixel::A8:
    case Pixel::L8:
    case Pixel::CHROMINANCE_U:
    case Pixel::CHROMINANCE_V:
    case Pixel::LA88:
    case Pixel::RGB888:
    case Pixel::RGB8888:
    case Pixel::BGR8888:
    case Pixel::RGBA8888:
    case Pixel::BGRA8888: // FALL THROUGH
    {
      validPixelFormat = true;
      break;
    }
    default:
    {
      // This pixel format is not supported for this operation.
      validPixelFormat = false;
      break;
    }
  }

  if(!validPixelFormat)
  {
    // Can't rotate the pixel buffer with the current pixel format.
    DALI_LOG_ERROR("Can't rotate the pixel buffer with the current pixel format : %s\n", Platform::GetPixelFormatName(pixelBuffer.GetPixelFormat()));
    return false;
  }

  float radians = Radian(angle).radian;

  // Transform the input angle into the range [0..2PI]
  radians = fmod(radians, TWO_PI);
  radians += (radians < 0.f) ? TWO_PI : 0.f;

  if(radians < Dali::Math::MACHINE_EPSILON_10)
  {
    // Nothing to do if the angle is zero.
    return true;
  }

  const uint32_t pixelSize = Pixel::GetBytesPerPixel(pixelBuffer.GetPixelFormat());

  uint32_t width  = pixelBuffer.GetWidth();
  uint32_t height = pixelBuffer.GetHeight();

  uint8_t* pixelsOut = nullptr;
  Platform::RotateByShear(pixelBuffer.GetBuffer(),
                          width,
                          height,
                          pixelBuffer.GetStrideBytes(),
                          pixelSize,
                          radians,
                          pixelsOut,
                          width,
                          height);

  // Check whether the rotation succedded and set the new pixel buffer data.
  const bool success = nullptr != pixelsOut;

  if(success)
  {
    // Set the new pixel buffer. The buffer is tightly packed.
    // The metadata and the pre-multiplied flag are kept.
    Dali::Integration::ResetPixelBufferData(pixelBuffer,
                                            pixelsOut,
                                            width * height * pixelSize,
                                            width,
                                            height,
                                            width * pixelSize,
                                            pixelBuffer.GetPixelFormat());
  }

  return success;
}

void MultiplyColorByAlpha(Dali::PixelBuffer& pixelBuffer)
{
  if(pixelBuffer.IsAlphaPreMultiplied())
  {
    // Already pre-multiplied; multiplying again would darken the colors.
    return;
  }

  const Pixel::Format pixelFormat = pixelBuffer.GetPixelFormat();

  // Do not allow premultiply alpha feature if pixel format compressed.
  if(!Pixel::IsCompressed(pixelFormat))
  {
    if(Pixel::HasAlpha(pixelFormat))
    {
      // Compressed textures have unknown size of the pixel. Alpha premultiplication
      // must be skipped in such case
      auto bytesPerPixel = Pixel::GetBytesPerPixel(pixelFormat);
      DALI_ASSERT_DEBUG(bytesPerPixel > 0 && "Pixel format is invalid!");

      DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_PIXEL_BUFFER_MULTIPLY_ALPHA", [&](std::ostringstream& oss)
      { oss << "[" << pixelBuffer.GetWidth() << "x" << pixelBuffer.GetHeight() << " format " << pixelFormat << " bpp " << bytesPerPixel << "]"; });

      uint8_t*       pixel       = pixelBuffer.GetBuffer();
      const uint32_t strideBytes = pixelBuffer.GetStrideBytes();
      const uint32_t widthBytes  = pixelBuffer.GetWidth() * bytesPerPixel;
      const uint32_t height      = pixelBuffer.GetHeight();

      // Collect all valid channel list before lookup whole buffer
      std::vector<Channel> validChannelList;
      for(const Channel& channel : {Adaptor::RED, Adaptor::GREEN, Adaptor::BLUE, Adaptor::LUMINANCE})
      {
        if(HasChannel(pixelFormat, channel))
        {
          validChannelList.emplace_back(channel);
        }
      }

      if(DALI_LIKELY(!validChannelList.empty()))
      {
        for(uint32_t y = 0; y < height; y++)
        {
          for(uint32_t x = 0; x < widthBytes; x += bytesPerPixel)
          {
            uint32_t alpha = ReadChannel(&pixel[x], pixelFormat, Adaptor::ALPHA);
            if(alpha < 255)
            {
              // If alpha is 255, we don't need to change color. Skip current pixel
              // But if alpha is not 255, we should change color.
              if(alpha > 0)
              {
                for(const Channel& channel : validChannelList)
                {
                  auto color = ReadChannel(&pixel[x], pixelFormat, channel);
                  WriteChannel(&pixel[x], pixelFormat, channel, Platform::MultiplyAndNormalizeColor(color, alpha));
                }
              }
              else
              {
                // If alpha is 0, just set all pixel as zero.
                memset(&pixel[x], 0, bytesPerPixel);
              }
            }
          }
          pixel += strideBytes;
        }
      }

      DALI_TRACE_END(gTraceFilter, "DALI_PIXEL_BUFFER_MULTIPLY_ALPHA");
    }

    // Note : Mark as premultiplied even if alpha channel is not present.
    Dali::Integration::SetPixelBufferPreMultiplied(pixelBuffer, true);
  }
}

uint32_t GetBrightness(Dali::PixelBuffer& pixelBuffer)
{
  uint32_t            brightness    = 0;
  const Pixel::Format pixelFormat   = pixelBuffer.GetPixelFormat();
  uint32_t            bytesPerPixel = Pixel::GetBytesPerPixel(pixelFormat);

  const uint32_t width  = pixelBuffer.GetWidth();
  const uint32_t height = pixelBuffer.GetHeight();

  if(bytesPerPixel && width && height)
  {
    DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_PIXEL_BUFFER_GET_BRIGHTNESS", [&](std::ostringstream& oss)
    { oss << "[" << width << "x" << height << " format " << pixelFormat << " bpp " << bytesPerPixel << "]"; });
    uint8_t*       pixel       = pixelBuffer.GetBuffer();
    const uint32_t strideBytes = pixelBuffer.GetStrideBytes();
    const uint32_t widthBytes  = width * bytesPerPixel;
    const uint32_t bufferSize  = width * height;

    uint64_t red   = 0;
    uint64_t green = 0;
    uint64_t blue  = 0;

    for(uint32_t y = 0; y < height; y++)
    {
      for(uint32_t x = 0; x < widthBytes; x += bytesPerPixel)
      {
        red += ReadChannel(&pixel[x], pixelFormat, Adaptor::RED);
        green += ReadChannel(&pixel[x], pixelFormat, Adaptor::GREEN);
        blue += ReadChannel(&pixel[x], pixelFormat, Adaptor::BLUE);
      }
      pixel += strideBytes;
    }

    // http://www.w3.org/TR/AERT#color-contrast
    brightness = static_cast<uint32_t>((red * BRIGHTNESS_CONSTANT_R + green * BRIGHTNESS_CONSTANT_G + blue * BRIGHTNESS_CONSTANT_B) / (1000uLL * bufferSize));

    DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_PIXEL_BUFFER_GET_BRIGHTNESS", [&](std::ostringstream& oss)
    { oss << "[r:" << static_cast<uint32_t>(red / bufferSize) << " g:" << static_cast<uint32_t>(green / bufferSize) << " b:" << static_cast<uint32_t>(blue / bufferSize) << " brightness " << brightness << "]"; });
  }

  return brightness;
}

Dali::PixelBuffer NewCrop(Dali::PixelBuffer& inBuffer, uint16_t x, uint16_t y, ImageDimensions cropDimensions)
{
  DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_PIXEL_BUFFER_NEW_CROP", [&](std::ostringstream& oss)
  {
    oss << "[origin:" << inBuffer.GetWidth() << "x" << inBuffer.GetHeight() << " format " << inBuffer.GetPixelFormat();
    oss << " offset:" << x << ", " << y;
    oss << " desired:" << cropDimensions.GetWidth() << "x" << cropDimensions.GetHeight() << "]"; });
  Dali::PixelBuffer outBuffer = Dali::PixelBuffer::New(cropDimensions.GetWidth(), cropDimensions.GetHeight(), inBuffer.GetPixelFormat());

  int bytesPerPixel = Pixel::GetBytesPerPixel(inBuffer.GetPixelFormat());

  // This method only really works if bytesPerPixel is exist. (~= not compressed)
  if(DALI_LIKELY(bytesPerPixel != 0u && inBuffer.GetBuffer() != nullptr && outBuffer.GetBuffer() != nullptr))
  {
    int srcStrideBytes  = inBuffer.GetStrideBytes();
    int destStrideBytes = cropDimensions.GetWidth() * bytesPerPixel; // The destination buffer is tightly packed
    int copyBytes       = destStrideBytes;

    // Clamp crop to right edge
    if(DALI_UNLIKELY(static_cast<uint32_t>(x) + cropDimensions.GetWidth() > inBuffer.GetWidth()))
    {
      copyBytes = (inBuffer.GetWidth() - x) * bytesPerPixel;
    }

    int      srcOffset  = x * bytesPerPixel + y * srcStrideBytes;
    int      destOffset = 0;
    uint8_t* destBuffer = outBuffer.GetBuffer();

    // Clamp crop to last row
    uint16_t endRow = y + cropDimensions.GetHeight();
    if(DALI_UNLIKELY(endRow > inBuffer.GetHeight()))
    {
      endRow = inBuffer.GetHeight() - 1;
    }

    if(srcStrideBytes == copyBytes && copyBytes == destStrideBytes)
    {
      // We can use single memcpy
      if(DALI_LIKELY(y < endRow))
      {
        memcpy(destBuffer, inBuffer.GetBuffer() + srcOffset, copyBytes * (endRow - y));
      }
    }
    else
    {
      for(uint16_t row = y; row < endRow; ++row)
      {
        memcpy(destBuffer + destOffset, inBuffer.GetBuffer() + srcOffset, copyBytes);
        srcOffset += srcStrideBytes;
        destOffset += destStrideBytes;
      }
    }
  }
  else
  {
    DALI_LOG_ERROR("Trying to crop an image with unsupported pixel format: %s or null buffer: in %p out %p\n", Platform::GetPixelFormatName(inBuffer.GetPixelFormat()), inBuffer.GetBuffer(), outBuffer.GetBuffer());
  }
  DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_PIXEL_BUFFER_NEW_CROP", [&](std::ostringstream& oss)
  { oss << "[valid? " << (bytesPerPixel != 0) << "]"; });
  return outBuffer;
}

Dali::PixelBuffer NewResize(Dali::PixelBuffer& inBuffer, ImageDimensions outDimensions)
{
  DALI_TRACE_BEGIN_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_PIXEL_BUFFER_NEW_RESIZE", [&](std::ostringstream& oss)
  {
    oss << "[origin:" << inBuffer.GetWidth() << "x" << inBuffer.GetHeight() << " format " << inBuffer.GetPixelFormat();
    oss << " desired:" << outDimensions.GetWidth() << "x" << outDimensions.GetHeight() << "]"; });
  Dali::PixelBuffer outBuffer = Dali::PixelBuffer::New(outDimensions.GetWidth(), outDimensions.GetHeight(), inBuffer.GetPixelFormat());

  const bool validPixelFormat = IsResizeSupportedPixelFormat(inBuffer.GetPixelFormat());

  if(DALI_LIKELY(validPixelFormat && inBuffer.GetBuffer() != nullptr && outBuffer.GetBuffer() != nullptr))
  {
    ImageDimensions inDimensions(inBuffer.GetWidth(), inBuffer.GetHeight());

    bool hasAlpha      = Pixel::HasAlpha(inBuffer.GetPixelFormat());
    int  bytesPerPixel = Pixel::GetBytesPerPixel(inBuffer.GetPixelFormat());

    Resampler::Filter filterType = Resampler::LANCZOS4;
    if(inDimensions.GetWidth() < outDimensions.GetWidth() && inDimensions.GetHeight() < outDimensions.GetHeight())
    {
      filterType = Resampler::MITCHELL;
    }
    Dali::Internal::Platform::Resample(inBuffer.GetBuffer(), inDimensions, inBuffer.GetStrideBytes(), outBuffer.GetBuffer(), outDimensions, filterType, bytesPerPixel, hasAlpha);
  }
  else
  {
    DALI_LOG_ERROR("Trying to resize an image with unsupported pixel format: %s or null buffer: in %p out %p\n", Platform::GetPixelFormatName(inBuffer.GetPixelFormat()), inBuffer.GetBuffer(), outBuffer.GetBuffer());
  }
  DALI_TRACE_END_WITH_MESSAGE_GENERATOR(gTraceFilter, "DALI_PIXEL_BUFFER_NEW_RESIZE", [&](std::ostringstream& oss)
  { oss << "[valid? " << validPixelFormat << "]"; });

  return outBuffer;
}

} // namespace PixelBufferOperations

} // namespace Adaptor

} // namespace Internal

} //namespace DALI_NAMESPACE
