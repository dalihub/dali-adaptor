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
 */

#include <dali/internal/imaging/common/alpha-mask.h>
#include <dali/internal/imaging/common/image-operations.h>
#include <dali/internal/imaging/common/pixel-buffer-impl.h>
#include <dali/internal/imaging/common/pixel-manipulation.h>
#include <dali/public-api/images/image-operations.h> // For ImageDimensions

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
void ApplyMaskToAlphaChannel(PixelBuffer& buffer, const PixelBuffer& mask)
{
  int                 srcAlphaByteOffset = 0;
  int                 srcAlphaMask       = 0;
  Dali::Pixel::Format srcPixelFormat     = mask.GetPixelFormat();

  if(Pixel::HasAlpha(srcPixelFormat))
  {
    Dali::Pixel::GetAlphaOffsetAndMask(srcPixelFormat, srcAlphaByteOffset, srcAlphaMask);
  }
  else if(srcPixelFormat == Pixel::L8)
  {
    srcAlphaMask = 0xFF;
  }

  int                 destAlphaByteOffset = 0;
  int                 destAlphaMask       = 0;
  Dali::Pixel::Format destPixelFormat     = buffer.GetPixelFormat();
  Dali::Pixel::GetAlphaOffsetAndMask(destPixelFormat, destAlphaByteOffset, destAlphaMask);

  unsigned int   srcBytesPerPixel = Dali::Pixel::GetBytesPerPixel(srcPixelFormat);
  unsigned char* srcBuffer        = mask.GetBuffer();
  unsigned char* destBuffer       = buffer.GetBuffer();

  unsigned int destBytesPerPixel = Dali::Pixel::GetBytesPerPixel(buffer.GetPixelFormat());

  int srcOffset  = 0;
  int destOffset = 0;

  // if image is premultiplied, the other channels of the image need to multiply by alpha.
  if(buffer.IsAlphaPreMultiplied())
  {
    // Collect all valid channel list before lookup whole buffer
    std::vector<Channel> validChannelList;
    for(const Channel& channel : {Adaptor::RED, Adaptor::GREEN, Adaptor::BLUE, Adaptor::LUMINANCE, Adaptor::ALPHA})
    {
      if(HasChannel(destPixelFormat, channel))
      {
        validChannelList.emplace_back(channel);
      }
    }
    if(DALI_LIKELY(!validChannelList.empty()))
    {
      for(unsigned int row = 0; row < buffer.GetHeight(); ++row)
      {
        for(unsigned int col = 0; col < buffer.GetWidth(); ++col)
        {
          auto srcAlpha = srcBuffer[srcOffset + srcAlphaByteOffset] & srcAlphaMask;
          if(srcAlpha < 255)
          {
            // If alpha is 255, we don't need to change color. Skip current pixel
            // But if alpha is not 255, we should change color.
            if(srcAlpha > 0)
            {
              for(const Channel& channel : validChannelList)
              {
                auto color = ReadChannel(destBuffer + destOffset, destPixelFormat, channel);
                WriteChannel(destBuffer + destOffset, destPixelFormat, channel, color * srcAlpha / 255);
              }
            }
            else
            {
              // If alpha is 0, just set all pixel as zero.
              memset(destBuffer + destOffset, 0, destBytesPerPixel);
            }
          }

          srcOffset += srcBytesPerPixel;
          destOffset += destBytesPerPixel;
        }
      }
    }
  }
  else
  {
    for(unsigned int row = 0; row < buffer.GetHeight(); ++row)
    {
      for(unsigned int col = 0; col < buffer.GetWidth(); ++col)
      {
        unsigned char srcAlpha  = srcBuffer[srcOffset + srcAlphaByteOffset] & srcAlphaMask;
        unsigned char destAlpha = destBuffer[destOffset + destAlphaByteOffset] & destAlphaMask;

        destAlpha = (static_cast<std::uint16_t>(destAlpha) * static_cast<std::uint16_t>(srcAlpha)) / 255;

        destBuffer[destOffset + destAlphaByteOffset] &= ~destAlphaMask;
        destBuffer[destOffset + destAlphaByteOffset] |= (destAlpha & destAlphaMask);

        srcOffset += srcBytesPerPixel;
        destOffset += destBytesPerPixel;
      }
    }
  }
}

PixelBufferPtr CreateNewMaskedBuffer(const PixelBuffer& buffer, const PixelBuffer& mask)
{
  // Set up source alpha offsets
  int                 srcAlphaByteOffset = 0;
  int                 srcAlphaMask       = 0;
  Dali::Pixel::Format srcPixelFormat     = mask.GetPixelFormat();

  if(Pixel::HasAlpha(srcPixelFormat))
  {
    Dali::Pixel::GetAlphaOffsetAndMask(srcPixelFormat, srcAlphaByteOffset, srcAlphaMask);
  }
  else if(srcPixelFormat == Pixel::L8)
  {
    srcAlphaMask = 0xFF;
  }

  unsigned int   srcBytesPerPixel = Dali::Pixel::GetBytesPerPixel(srcPixelFormat);
  unsigned char* srcBuffer        = mask.GetBuffer();

  // Set up source color offsets
  Dali::Pixel::Format srcColorPixelFormat   = buffer.GetPixelFormat();
  unsigned int        srcColorBytesPerPixel = Dali::Pixel::GetBytesPerPixel(srcColorPixelFormat);

  // Setup destination offsets
  Dali::Pixel::Format destPixelFormat     = Dali::Pixel::RGBA8888;
  unsigned int        destBytesPerPixel   = Dali::Pixel::GetBytesPerPixel(destPixelFormat);
  int                 destAlphaByteOffset = 0;
  int                 destAlphaMask       = 0;
  Dali::Pixel::GetAlphaOffsetAndMask(destPixelFormat, destAlphaByteOffset, destAlphaMask);

  PixelBufferPtr newPixelBuffer = PixelBuffer::New(buffer.GetWidth(), buffer.GetHeight(), destPixelFormat);
  unsigned char* destBuffer     = newPixelBuffer->GetBuffer();
  unsigned char* oldBuffer      = buffer.GetBuffer();

  int  srcAlphaOffset = 0;
  int  srcColorOffset = 0;
  int  destOffset     = 0;
  bool hasAlpha       = Dali::Pixel::HasAlpha(buffer.GetPixelFormat());

  unsigned char destAlpha = 0;

  for(unsigned int row = 0; row < buffer.GetHeight(); ++row)
  {
    for(unsigned int col = 0; col < buffer.GetWidth(); ++col)
    {
      unsigned char srcAlpha = srcBuffer[srcAlphaOffset + srcAlphaByteOffset] & srcAlphaMask;

      ConvertColorChannelsToRGBA8888(oldBuffer, srcColorOffset, srcColorPixelFormat, destBuffer, destOffset);

      if(hasAlpha)
      {
        destAlpha = ConvertAlphaChannelToA8(oldBuffer, srcColorOffset, srcColorPixelFormat);
        destAlpha = (static_cast<std::uint16_t>(destAlpha) * static_cast<std::uint16_t>(srcAlpha)) / 255;
      }
      else
      {
        destAlpha = srcAlpha;
      }

      destBuffer[destOffset + destAlphaByteOffset] &= ~destAlphaMask;
      destBuffer[destOffset + destAlphaByteOffset] |= (destAlpha & destAlphaMask);

      srcColorOffset += srcColorBytesPerPixel;
      srcAlphaOffset += srcBytesPerPixel;
      destOffset += destBytesPerPixel;
    }
  }

  return newPixelBuffer;
}

} //namespace Adaptor

} // namespace Internal

} // namespace Dali
