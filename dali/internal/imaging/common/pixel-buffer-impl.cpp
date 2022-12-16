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
#include <dali/internal/imaging/common/pixel-buffer-impl.h>

// EXTERNAL INCLUDES
#include <stdlib.h>
#include <cstring>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/internal/imaging/common/alpha-mask.h>
#include <dali/internal/imaging/common/gaussian-blur.h>
#include <dali/internal/imaging/common/image-operations.h>
#include <dali/internal/imaging/common/pixel-manipulation.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{

#if defined(DEBUG_ENABLED)
Debug::Filter* gPixelBufferFilter = Debug::Filter::New(Debug::NoLogging, false, "DALI_LOG_PIXEL_BUFFER_SIZE");
#endif

const float TWO_PI = 2.f * Math::PI; ///< 360 degrees in radians
// based on W3C Recommendations (https://www.w3.org/TR/AERT/#color-contrast)
constexpr uint32_t BRIGHTNESS_CONSTANT_R = 299;
constexpr uint32_t BRIGHTNESS_CONSTANT_G = 587;
constexpr uint32_t BRIGHTNESS_CONSTANT_B = 114;
} // namespace

#if defined(DEBUG_ENABLED)
uint32_t PixelBuffer::gPixelBufferAllocationTotal{0};
#endif

PixelBuffer::PixelBuffer(uint8_t*            buffer,
                         uint32_t            bufferSize,
                         uint32_t            width,
                         uint32_t            height,
                         uint32_t            stride,
                         Dali::Pixel::Format pixelFormat)
: mMetadata(),
  mBuffer(buffer),
  mBufferSize(bufferSize),
  mWidth(width),
  mHeight(height),
  mStride(stride ? stride : width),
  mPixelFormat(pixelFormat),
  mPreMultiplied(false)
{
}

PixelBuffer::~PixelBuffer()
{
  ReleaseBuffer();
}

PixelBufferPtr PixelBuffer::New(uint32_t            width,
                                uint32_t            height,
                                Dali::Pixel::Format pixelFormat)
{
  uint32_t bufferSize = width * height * Dali::Pixel::GetBytesPerPixel(pixelFormat);
  uint8_t* buffer     = NULL;
  if(bufferSize > 0)
  {
    buffer = static_cast<uint8_t*>(malloc(bufferSize));
#if defined(DEBUG_ENABLED)
    gPixelBufferAllocationTotal += bufferSize;
#endif
  }
  DALI_LOG_INFO(gPixelBufferFilter, Debug::Concise, "Allocated PixelBuffer of size %u\n", bufferSize);

  return new PixelBuffer(buffer, bufferSize, width, height, width, pixelFormat);
}

PixelBufferPtr PixelBuffer::New(uint8_t*            buffer,
                                uint32_t            bufferSize,
                                uint32_t            width,
                                uint32_t            height,
                                uint32_t            stride,
                                Dali::Pixel::Format pixelFormat)
{
  return new PixelBuffer(buffer, bufferSize, width, height, stride, pixelFormat);
}

Dali::PixelData PixelBuffer::Convert(PixelBuffer& pixelBuffer)
{
#if defined(DEBUG_ENABLED)
  gPixelBufferAllocationTotal -= pixelBuffer.mBufferSize;
#endif
  Dali::PixelData pixelData = Dali::PixelData::New(pixelBuffer.mBuffer,
                                                   pixelBuffer.mBufferSize,
                                                   pixelBuffer.mWidth,
                                                   pixelBuffer.mHeight,
                                                   pixelBuffer.mStride,
                                                   pixelBuffer.mPixelFormat,
                                                   Dali::PixelData::FREE);
  pixelBuffer.mBuffer       = NULL;
  pixelBuffer.mWidth        = 0;
  pixelBuffer.mHeight       = 0;
  pixelBuffer.mBufferSize   = 0;
  pixelBuffer.mStride       = 0;

  return pixelData;
}

uint32_t PixelBuffer::GetWidth() const
{
  return mWidth;
}

uint32_t PixelBuffer::GetHeight() const
{
  return mHeight;
}

uint32_t PixelBuffer::GetStride() const
{
  return mStride;
}

Dali::Pixel::Format PixelBuffer::GetPixelFormat() const
{
  return mPixelFormat;
}

uint8_t* PixelBuffer::GetBuffer() const
{
  return mBuffer;
}

const uint8_t* PixelBuffer::GetConstBuffer() const
{
  return mBuffer;
}

uint32_t PixelBuffer::GetBufferSize() const
{
  return mBufferSize;
}

Dali::PixelData PixelBuffer::CreatePixelData() const
{
  uint8_t* destBuffer = NULL;

  if(mBufferSize > 0)
  {
    destBuffer = static_cast<uint8_t*>(malloc(mBufferSize));
    memcpy(destBuffer, mBuffer, mBufferSize);
  }

  Dali::PixelData pixelData = Dali::PixelData::New(destBuffer, mBufferSize, mWidth, mHeight, mStride, mPixelFormat, Dali::PixelData::FREE);
  return pixelData;
}

void PixelBuffer::ApplyMask(const PixelBuffer& inMask, float contentScale, bool cropToMask)
{
  if(cropToMask)
  {
    // First scale this buffer by the contentScale, and crop to the mask size
    // If it's too small, then scale the mask to match the image size
    // Then apply the mask
    ScaleAndCrop(contentScale, ImageDimensions(inMask.GetWidth(), inMask.GetHeight()));

    if(inMask.mWidth > mWidth || inMask.mHeight > mHeight)
    {
      PixelBufferPtr mask = NewResize(inMask, ImageDimensions(mWidth, mHeight));
      ApplyMaskInternal(*mask);
    }
    else
    {
      ApplyMaskInternal(inMask);
    }
  }
  else
  {
    // First, scale the mask to match the image size,
    // then apply the mask.
    PixelBufferPtr mask = NewResize(inMask, ImageDimensions(mWidth, mHeight));
    ApplyMaskInternal(*mask);
  }
}

void PixelBuffer::ApplyMaskInternal(const PixelBuffer& mask)
{
  int byteOffset = 0;
  int bitMask    = 0;

  Dali::Pixel::GetAlphaOffsetAndMask(mPixelFormat, byteOffset, bitMask);
  if(Dali::Pixel::HasAlpha(mPixelFormat) && bitMask == 255)
  {
    ApplyMaskToAlphaChannel(*this, mask);
  }
  else
  {
    PixelBufferPtr newPixelBuffer = CreateNewMaskedBuffer(*this, mask);
    TakeOwnershipOfBuffer(*newPixelBuffer);
    // On leaving scope, newPixelBuffer will get destroyed.
  }
}

void PixelBuffer::TakeOwnershipOfBuffer(PixelBuffer& pixelBuffer)
{
  ReleaseBuffer();

  // Take ownership of new buffer
  mBuffer             = pixelBuffer.mBuffer;
  pixelBuffer.mBuffer = NULL;
  mBufferSize         = pixelBuffer.mBufferSize;
  mWidth              = pixelBuffer.mWidth;
  mHeight             = pixelBuffer.mHeight;
  mStride             = pixelBuffer.mStride;
  mPixelFormat        = pixelBuffer.mPixelFormat;
}

void PixelBuffer::ReleaseBuffer()
{
  if(mBuffer)
  {
#if defined(DEBUG_ENABLED)
    gPixelBufferAllocationTotal -= mBufferSize;
#endif
    free(mBuffer);
  }
}

void PixelBuffer::AllocateFixedSize(uint32_t size)
{
  ReleaseBuffer();
  mBuffer     = reinterpret_cast<unsigned char*>(malloc(size));
  mBufferSize = size;
#if defined(DEBUG_ENABLED)
  gPixelBufferAllocationTotal += size;
#endif
}

bool PixelBuffer::Rotate(Degree angle)
{
  // Check first if Rotate() can perform the operation in the current pixel buffer.

  bool validPixelFormat = false;
  switch(mPixelFormat)
  {
    case Pixel::A8:
    case Pixel::L8:
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
    DALI_LOG_ERROR("Can't rotate the pixel buffer with the current pixel format\n");
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

  const unsigned int pixelSize = Pixel::GetBytesPerPixel(mPixelFormat);

  uint8_t* pixelsOut = nullptr;
  Platform::RotateByShear(mBuffer,
                          mWidth,
                          mHeight,
                          mStride,
                          pixelSize,
                          radians,
                          pixelsOut,
                          mWidth,
                          mHeight);

  // Check whether the rotation succedded and set the new pixel buffer data.
  const bool success = nullptr != pixelsOut;

  if(success)
  {
    // Release the memory of the current pixel buffer.
    ReleaseBuffer();

    // Set the new pixel buffer.
    mBuffer     = pixelsOut;
    pixelsOut   = nullptr;
    mBufferSize = mWidth * mHeight * pixelSize;
    mStride     = mWidth; // The buffer is tightly packed.
  }

  return success;
}

void PixelBuffer::ScaleAndCrop(float scaleFactor, ImageDimensions cropDimensions)
{
  ImageDimensions outDimensions(float(mWidth) * scaleFactor,
                                float(mHeight) * scaleFactor);

  if(outDimensions.GetWidth() != mWidth || outDimensions.GetHeight() != mHeight)
  {
    Resize(outDimensions);
  }

  ImageDimensions postCropDimensions(
    std::min(cropDimensions.GetWidth(), outDimensions.GetWidth()),
    std::min(cropDimensions.GetHeight(), outDimensions.GetHeight()));

  if(postCropDimensions.GetWidth() < outDimensions.GetWidth() ||
     postCropDimensions.GetHeight() < outDimensions.GetHeight())
  {
    uint16_t x = (outDimensions.GetWidth() - postCropDimensions.GetWidth()) / 2;
    uint16_t y = (outDimensions.GetHeight() - postCropDimensions.GetHeight()) / 2;
    Crop(x, y, postCropDimensions);
  }
}

void PixelBuffer::Crop(uint16_t x, uint16_t y, ImageDimensions cropDimensions)
{
  PixelBufferPtr outBuffer = NewCrop(*this, x, y, cropDimensions);
  TakeOwnershipOfBuffer(*outBuffer);
}

PixelBufferPtr PixelBuffer::NewCrop(const PixelBuffer& inBuffer, uint16_t x, uint16_t y, ImageDimensions cropDimensions)
{
  PixelBufferPtr outBuffer     = PixelBuffer::New(cropDimensions.GetWidth(), cropDimensions.GetHeight(), inBuffer.GetPixelFormat());
  int            bytesPerPixel = Pixel::GetBytesPerPixel(inBuffer.mPixelFormat);
  int            srcStride     = inBuffer.mStride * bytesPerPixel;
  int            destStride    = cropDimensions.GetWidth() * bytesPerPixel; // The destination buffer is tightly packed

  // Clamp crop to right edge
  if(x + cropDimensions.GetWidth() > inBuffer.mWidth)
  {
    destStride = (inBuffer.mWidth - x) * bytesPerPixel;
  }

  int      srcOffset  = x * bytesPerPixel + y * srcStride;
  int      destOffset = 0;
  uint8_t* destBuffer = outBuffer->mBuffer;

  // Clamp crop to last row
  uint16_t endRow = y + cropDimensions.GetHeight();
  if(endRow > inBuffer.mHeight)
  {
    endRow = inBuffer.mHeight - 1;
  }
  for(uint16_t row = y; row < endRow; ++row)
  {
    memcpy(destBuffer + destOffset, inBuffer.mBuffer + srcOffset, destStride);
    srcOffset += srcStride;
    destOffset += destStride;
  }
  return outBuffer;
}

void PixelBuffer::SetMetadata(const Property::Map& map)
{
  mMetadata.reset(new Property::Map(map));
}

bool PixelBuffer::GetMetadata(Property::Map& outMetadata) const
{
  if(!mMetadata)
  {
    return false;
  }
  outMetadata = *mMetadata;
  return true;
}

void PixelBuffer::SetMetadata(std::unique_ptr<Property::Map> metadata)
{
  mMetadata = std::move(metadata);
}

void PixelBuffer::Resize(ImageDimensions outDimensions)
{
  if(mWidth != outDimensions.GetWidth() || mHeight != outDimensions.GetHeight())
  {
    PixelBufferPtr outBuffer = NewResize(*this, outDimensions);
    TakeOwnershipOfBuffer(*outBuffer);
  }
}

PixelBufferPtr PixelBuffer::NewResize(const PixelBuffer& inBuffer, ImageDimensions outDimensions)
{
  PixelBufferPtr  outBuffer = PixelBuffer::New(outDimensions.GetWidth(), outDimensions.GetHeight(), inBuffer.GetPixelFormat());
  ImageDimensions inDimensions(inBuffer.mWidth, inBuffer.mHeight);

  bool hasAlpha      = Pixel::HasAlpha(inBuffer.mPixelFormat);
  int  bytesPerPixel = Pixel::GetBytesPerPixel(inBuffer.mPixelFormat);

  Resampler::Filter filterType = Resampler::LANCZOS4;
  if(inDimensions.GetWidth() < outDimensions.GetWidth() && inDimensions.GetHeight() < outDimensions.GetHeight())
  {
    filterType = Resampler::MITCHELL;
  }

  // This method only really works for 8 bit wide channels.
  // (But could be expanded to work)
  if(inBuffer.mPixelFormat == Pixel::A8 ||
     inBuffer.mPixelFormat == Pixel::L8 ||
     inBuffer.mPixelFormat == Pixel::LA88 ||
     inBuffer.mPixelFormat == Pixel::RGB888 ||
     inBuffer.mPixelFormat == Pixel::RGB8888 ||
     inBuffer.mPixelFormat == Pixel::BGR8888 ||
     inBuffer.mPixelFormat == Pixel::RGBA8888 ||
     inBuffer.mPixelFormat == Pixel::BGRA8888)
  {
    Dali::Internal::Platform::Resample(inBuffer.mBuffer, inDimensions, inBuffer.mStride, outBuffer->GetBuffer(), outDimensions, filterType, bytesPerPixel, hasAlpha);
  }
  else
  {
    DALI_LOG_ERROR("Trying to resize an image with too narrow a channel width");
  }

  return outBuffer;
}

void PixelBuffer::ApplyGaussianBlur(const float blurRadius)
{
  // This method only works for pixel buffer in RGBA format.
  if(mWidth > 0 && mHeight > 0 && mPixelFormat == Pixel::RGBA8888)
  {
    if(blurRadius > Math::MACHINE_EPSILON_1)
    {
      PerformGaussianBlurRGBA(*this, blurRadius);
    }
  }
  else
  {
    DALI_LOG_ERROR("Trying to apply gaussian blur to an empty pixel buffer or a pixel buffer not in RGBA format");
  }
}

void PixelBuffer::MultiplyColorByAlpha()
{
  auto bytesPerPixel = Pixel::GetBytesPerPixel(mPixelFormat);

  // Compressed textures have unknown size of the pixel. Alpha premultiplication
  // must be skipped in such case
  if(Pixel::GetBytesPerPixel(mPixelFormat) && Pixel::HasAlpha(mPixelFormat))
  {
    uint8_t*       pixel       = mBuffer;
    const uint32_t strideBytes = mStride * bytesPerPixel;
    const uint32_t widthBytes  = mWidth * bytesPerPixel;

    // Collect all valid channel list before lookup whole buffer
    std::vector<Channel> validChannelList;
    for(const Channel& channel : {Adaptor::RED, Adaptor::GREEN, Adaptor::BLUE, Adaptor::LUMINANCE})
    {
      if(HasChannel(mPixelFormat, channel))
      {
        validChannelList.emplace_back(channel);
      }
    }

    if(DALI_LIKELY(!validChannelList.empty()))
    {
      for(uint32_t y = 0; y < mHeight; y++)
      {
        for(uint32_t x = 0; x < widthBytes; x += bytesPerPixel)
        {
          uint32_t alpha = ReadChannel(&pixel[x], mPixelFormat, Adaptor::ALPHA);
          if(alpha < 255)
          {
            // If alpha is 255, we don't need to change color. Skip current pixel
            // But if alpha is not 255, we should change color.
            if(alpha > 0)
            {
              for(const Channel& channel : validChannelList)
              {
                auto color = ReadChannel(&pixel[x], mPixelFormat, channel);
                WriteChannel(&pixel[x], mPixelFormat, channel, Platform::MultiplyAndNormalizeColor(color, alpha));
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
    mPreMultiplied = true;
  }
}

bool PixelBuffer::IsAlphaPreMultiplied() const
{
  return mPreMultiplied;
}

uint32_t PixelBuffer::GetBrightness() const
{
  uint32_t brightness    = 0;
  uint32_t bytesPerPixel = Pixel::GetBytesPerPixel(mPixelFormat);

  if(bytesPerPixel && mWidth && mHeight)
  {
    uint8_t*       pixel       = mBuffer;
    const uint32_t strideBytes = mStride * bytesPerPixel;
    const uint32_t widthBytes  = mWidth * bytesPerPixel;
    const uint32_t bufferSize  = mWidth * mHeight;

    uint64_t red   = 0;
    uint64_t green = 0;
    uint64_t blue  = 0;

    for(uint32_t y = 0; y < mHeight; y++)
    {
      for(uint32_t x = 0; x < widthBytes; x += bytesPerPixel)
      {
        red += ReadChannel(&pixel[x], mPixelFormat, Adaptor::RED);
        green += ReadChannel(&pixel[x], mPixelFormat, Adaptor::GREEN);
        blue += ReadChannel(&pixel[x], mPixelFormat, Adaptor::BLUE);
      }
      pixel += strideBytes;
    }

    // http://www.w3.org/TR/AERT#color-contrast
    brightness = (red * BRIGHTNESS_CONSTANT_R + green * BRIGHTNESS_CONSTANT_G + blue * BRIGHTNESS_CONSTANT_B) / (1000uLL * bufferSize);
  }

  return brightness;
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
