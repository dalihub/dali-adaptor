/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <dali/internal/imaging/common/pixel-manipulation.h>
#include <dali/internal/imaging/common/alpha-mask.h>
#include <dali/internal/imaging/common/pixel-buffer-impl.h>
#include <dali/public-api/images/image-operations.h> // For ImageDimensions
#include <dali/internal/imaging/common/image-operations.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

void ApplyMaskToAlphaChannel( PixelBuffer& buffer, const PixelBuffer& mask )
{
  int srcAlphaByteOffset=0;
  int srcAlphaMask=0;
  Dali::Pixel::Format srcPixelFormat = mask.GetPixelFormat();

  if( Pixel::HasAlpha(srcPixelFormat) )
  {
    Dali::Pixel::GetAlphaOffsetAndMask( srcPixelFormat, srcAlphaByteOffset, srcAlphaMask );
  }
  else if( srcPixelFormat == Pixel::L8 )
  {
    srcAlphaMask=0xFF;
  }

  int destAlphaByteOffset=0;
  int destAlphaMask=0;
  Dali::Pixel::Format destPixelFormat = buffer.GetPixelFormat();
  Dali::Pixel::GetAlphaOffsetAndMask( destPixelFormat, destAlphaByteOffset, destAlphaMask );

  unsigned int srcBytesPerPixel = Dali::Pixel::GetBytesPerPixel( srcPixelFormat );
  unsigned char* srcBuffer = mask.GetBuffer();
  unsigned char* destBuffer = buffer.GetBuffer();

  unsigned int destBytesPerPixel = Dali::Pixel::GetBytesPerPixel( buffer.GetPixelFormat() );

  int srcOffset=0;
  int destOffset=0;

  float srcAlphaValue = 1.0f;

  // if image is premultiplied, the other channels of the image need to multiply by alpha.
  if( buffer.IsAlphaPreMultiplied() )
  {
    for( unsigned int row = 0; row < buffer.GetHeight(); ++row )
    {
      for( unsigned int col = 0; col < buffer.GetWidth(); ++col )
      {
        auto srcAlpha      = ReadChannel( srcBuffer + srcOffset, srcPixelFormat, Adaptor::ALPHA);
        auto destRed       = ReadChannel( destBuffer + destOffset, destPixelFormat, Adaptor::RED);
        auto destGreen     = ReadChannel( destBuffer + destOffset, destPixelFormat, Adaptor::GREEN);
        auto destBlue      = ReadChannel( destBuffer + destOffset, destPixelFormat, Adaptor::BLUE);
        auto destLuminance = ReadChannel( destBuffer + destOffset, destPixelFormat, Adaptor::LUMINANCE);
        auto destAlpha     = ReadChannel( destBuffer + destOffset, destPixelFormat, Adaptor::ALPHA);

        WriteChannel( destBuffer + destOffset, destPixelFormat, Adaptor::RED, destRed*srcAlpha / 255 );
        WriteChannel( destBuffer + destOffset, destPixelFormat, Adaptor::GREEN, destGreen*srcAlpha/255 );
        WriteChannel( destBuffer + destOffset, destPixelFormat, Adaptor::BLUE, destBlue*srcAlpha/255 );
        WriteChannel( destBuffer + destOffset, destPixelFormat, Adaptor::LUMINANCE, destLuminance*srcAlpha/255 );
        WriteChannel( destBuffer + destOffset, destPixelFormat, Adaptor::ALPHA, destAlpha*srcAlpha/255 );

        srcOffset  += srcBytesPerPixel;
        destOffset += destBytesPerPixel;
      }
    }
  }
  else
  {
    for( unsigned int row = 0; row < buffer.GetHeight(); ++row )
    {
      for( unsigned int col = 0; col < buffer.GetWidth(); ++col )
      {
        unsigned char alpha = srcBuffer[srcOffset + srcAlphaByteOffset] & srcAlphaMask;
        srcAlphaValue = float(alpha)/255.0f;

        unsigned char destAlpha = destBuffer[destOffset + destAlphaByteOffset] & destAlphaMask;
        float destAlphaValue = Clamp(float(destAlpha) * srcAlphaValue, 0.0f, 255.0f);
        destAlpha = destAlphaValue;
        destBuffer[destOffset + destAlphaByteOffset] &= ~destAlphaMask;
        destBuffer[destOffset + destAlphaByteOffset] |= ( destAlpha & destAlphaMask );

        srcOffset  += srcBytesPerPixel;
        destOffset += destBytesPerPixel;
      }
    }
  }
}

PixelBufferPtr CreateNewMaskedBuffer( const PixelBuffer& buffer, const PixelBuffer& mask )
{
  // Set up source alpha offsets
  int srcAlphaByteOffset=0;
  int srcAlphaMask=0;
  Dali::Pixel::Format srcPixelFormat = mask.GetPixelFormat();

  if( Pixel::HasAlpha(srcPixelFormat) )
  {
    Dali::Pixel::GetAlphaOffsetAndMask( srcPixelFormat, srcAlphaByteOffset, srcAlphaMask );
  }
  else if( srcPixelFormat == Pixel::L8 )
  {
    srcAlphaMask=0xFF;
  }

  unsigned int srcBytesPerPixel = Dali::Pixel::GetBytesPerPixel( srcPixelFormat );
  unsigned char* srcBuffer = mask.GetBuffer();

  // Set up source color offsets
  Dali::Pixel::Format srcColorPixelFormat = buffer.GetPixelFormat();
  unsigned int srcColorBytesPerPixel = Dali::Pixel::GetBytesPerPixel( srcColorPixelFormat );

  // Setup destination offsets
  Dali::Pixel::Format destPixelFormat = Dali::Pixel::RGBA8888;
  unsigned int destBytesPerPixel = Dali::Pixel::GetBytesPerPixel( destPixelFormat );
  int destAlphaByteOffset=0;
  int destAlphaMask=0;
  Dali::Pixel::GetAlphaOffsetAndMask( destPixelFormat, destAlphaByteOffset, destAlphaMask );

  PixelBufferPtr newPixelBuffer = PixelBuffer::New( buffer.GetWidth(), buffer.GetHeight(),
                                                    destPixelFormat );
  unsigned char* destBuffer = newPixelBuffer->GetBuffer();
  unsigned char* oldBuffer = buffer.GetBuffer();

  int srcAlphaOffset=0;
  int srcColorOffset=0;
  int destOffset=0;
  bool hasAlpha = Dali::Pixel::HasAlpha(buffer.GetPixelFormat());

  float srcAlphaValue = 1.0f;
  unsigned char destAlpha = 0;

  for( unsigned int row = 0; row < buffer.GetHeight(); ++row )
  {
    for( unsigned int col = 0; col < buffer.GetWidth(); ++col )
    {
      unsigned char alpha = srcBuffer[srcAlphaOffset + srcAlphaByteOffset] & srcAlphaMask;
      srcAlphaValue = float(alpha)/255.0f;

      ConvertColorChannelsToRGBA8888(oldBuffer, srcColorOffset, srcColorPixelFormat, destBuffer, destOffset );

      if( hasAlpha )
      {
        destAlpha = ConvertAlphaChannelToA8( oldBuffer, srcColorOffset, srcColorPixelFormat );
        float destAlphaValue = Clamp(float(destAlpha) * srcAlphaValue, 0.0f, 255.0f);
        destAlpha = destAlphaValue;
      }
      else
      {
        destAlpha = floorf(Clamp(srcAlphaValue * 255.0f, 0.0f, 255.0f));
      }

      destBuffer[destOffset + destAlphaByteOffset] &= ~destAlphaMask;
      destBuffer[destOffset + destAlphaByteOffset] |= ( destAlpha & destAlphaMask );

      srcColorOffset += srcColorBytesPerPixel;
      srcAlphaOffset += srcBytesPerPixel;
      destOffset += destBytesPerPixel;
    }
  }

  return newPixelBuffer;
}

} //namespace Adaptor

}// namespace Internal

}// namespace Dali
