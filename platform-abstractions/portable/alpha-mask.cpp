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

#include "pixel-manipulation.h"
#include "alpha-mask.h"
#include "pixel-buffer-impl.h"

namespace Dali
{
namespace Internal
{
namespace Adaptor
{


void ApplyMaskToAlphaChannel( PixelBuffer& buffer, const PixelBuffer& mask )
{
  const float rowFactor = float(mask.GetHeight()) / (1.0f * buffer.GetHeight());
  const float colFactor = float(mask.GetWidth()) / (1.0f * buffer.GetWidth()) ;

  int numSamples = 1;
  if( mask.GetHeight() > buffer.GetHeight() || mask.GetWidth() > buffer.GetWidth() )
  {
    numSamples = 4;
  }

  int srcAlphaByteOffset=0;
  int srcAlphaMask=0;
  Dali::Pixel::Format srcPixelFormat = mask.GetPixelFormat();

  Channel alphaChannel = ALPHA;
  if( Pixel::HasAlpha(srcPixelFormat) )
  {
    Dali::Pixel::GetAlphaOffsetAndMask( srcPixelFormat, srcAlphaByteOffset, srcAlphaMask );
  }
  else if( srcPixelFormat == Pixel::L8 )
  {
    srcAlphaMask=0xFF;
    alphaChannel = LUMINANCE;
  }

  int destAlphaByteOffset=0;
  int destAlphaMask=0;
  Dali::Pixel::GetAlphaOffsetAndMask( buffer.GetPixelFormat(), destAlphaByteOffset, destAlphaMask );

  unsigned int srcBytesPerPixel = Dali::Pixel::GetBytesPerPixel( srcPixelFormat );
  int srcStride = mask.GetWidth() * srcBytesPerPixel;
  unsigned char* srcBuffer = mask.GetBuffer();
  unsigned char* destBuffer = buffer.GetBuffer();

  unsigned int destBytesPerPixel = Dali::Pixel::GetBytesPerPixel( buffer.GetPixelFormat() );

  int srcOffset=0;
  int destOffset=0;

  float srcAlphaValue = 1.0f;

  for( unsigned int row = 0; row < buffer.GetHeight(); ++row )
  {
    for( unsigned int col = 0; col < buffer.GetWidth(); ++col )
    {
      if( numSamples == 1 )
      {
        srcOffset = floorf(row * rowFactor) * srcStride + floorf(col * colFactor) * srcBytesPerPixel;
        unsigned char alpha = srcBuffer[srcOffset + srcAlphaByteOffset] & srcAlphaMask;
        srcAlphaValue = float(alpha)/255.0f;
      }
      else
      {
        srcAlphaValue = ReadWeightedSample( srcBuffer, srcPixelFormat, srcStride, col*colFactor, row*rowFactor, mask.GetWidth(), mask.GetHeight(), alphaChannel );
      }

      unsigned char destAlpha = destBuffer[destOffset + destAlphaByteOffset] & destAlphaMask;
      float destAlphaValue = Clamp(float(destAlpha) * srcAlphaValue, 0.0f, 255.0f);
      destAlpha = destAlphaValue;
      destBuffer[destOffset + destAlphaByteOffset] &= ~destAlphaMask;
      destBuffer[destOffset + destAlphaByteOffset] |= ( destAlpha & destAlphaMask );

      destOffset += destBytesPerPixel;
    }
  }
}

PixelBufferPtr CreateNewMaskedBuffer( const PixelBuffer& buffer, const PixelBuffer& mask )
{
  const float rowFactor = float(mask.GetHeight()) / (1.0f * buffer.GetHeight());
  const float colFactor = float(mask.GetWidth()) / (1.0f * buffer.GetWidth()) ;

  int numSamples = 1;
  if( mask.GetHeight() > buffer.GetHeight() || mask.GetWidth() > buffer.GetWidth() )
  {
    numSamples = 4;
  }

  // Set up source alpha offsets
  int srcAlphaByteOffset=0;
  int srcAlphaMask=0;
  Dali::Pixel::Format srcPixelFormat = mask.GetPixelFormat();
  Channel alphaChannel = ALPHA;
  if( Pixel::HasAlpha(srcPixelFormat) )
  {
    Dali::Pixel::GetAlphaOffsetAndMask( srcPixelFormat, srcAlphaByteOffset, srcAlphaMask );
  }
  else if( srcPixelFormat == Pixel::L8 )
  {
    srcAlphaMask=0xFF;
    alphaChannel = LUMINANCE;
  }

  unsigned int srcBytesPerPixel = Dali::Pixel::GetBytesPerPixel( srcPixelFormat );
  int srcStride = mask.GetWidth() * srcBytesPerPixel;
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
      if( numSamples == 1 )
      {
        srcAlphaOffset = floorf(row * rowFactor) * srcStride + floorf(col * colFactor) * srcBytesPerPixel;
        unsigned char alpha = srcBuffer[srcAlphaOffset + srcAlphaByteOffset] & srcAlphaMask;
        srcAlphaValue = float(alpha)/255.0f;
      }
      else
      {
        srcAlphaValue = ReadWeightedSample( srcBuffer, srcPixelFormat, srcStride, col*colFactor, row*rowFactor, mask.GetWidth(), mask.GetHeight(), alphaChannel );
      }

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
      destOffset += destBytesPerPixel;
    }
  }

  return newPixelBuffer;
}


float ReadWeightedSample( unsigned char* buffer, Pixel::Format pixelFormat, int stride, float x, float y, int width, int height, Channel alphaChannel )
{
  int srcRow = floorf( y );
  int srcCol = floorf( x );

  int bytesPerPixel = Dali::Pixel::GetBytesPerPixel( pixelFormat );
  int srcOffset = srcRow * stride + srcCol * bytesPerPixel;
  float samples[4];

  samples[0] = ReadChannel( buffer + srcOffset, pixelFormat, alphaChannel );

  if( srcCol < width-1 )
  {
    samples[1] = ReadChannel( buffer + srcOffset+bytesPerPixel, pixelFormat, alphaChannel );
  }
  else
  {
    samples[1] = samples[0];
  }

  if( srcRow < height-1 )
  {
    samples[2] = ReadChannel( buffer + stride + srcOffset, pixelFormat, alphaChannel );
  }
  else
  {
    samples[2] = samples[0];
  }

  if( srcRow < height-1 && srcCol < width-1 )
  {
    samples[3] = ReadChannel( buffer + stride + srcOffset + bytesPerPixel, pixelFormat, alphaChannel );
  }
  else
  {
    samples[3] = samples[2];
  }

  // Bilinear interpolation:
  float weight[4];
  weight[0] = float(srcRow+1.0f) - y;
  weight[1] = y - float(srcRow);
  weight[2] = float(srcCol+1.0f) - x;
  weight[3] = x - float(srcCol);

  return ( weight[2] * (samples[0] * weight[0] + samples[1] * weight[1]) +
           weight[3] * (samples[2] * weight[0] + samples[3] * weight[1]) ) / 255.0f;
}

} //namespace Adaptor

}// namespace Internal

}// namespace Dali
