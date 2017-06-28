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
 *
 */

// CLASS HEADER
#include "pixel-buffer-impl.h"

// EXTERNAL INCLUDES
#include <stdlib.h>
#include <cstring>

// INTERNAL INCLUDES
#include "pixel-manipulation.h"
#include "alpha-mask.h"

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

PixelBuffer::PixelBuffer( unsigned char* buffer,
                          unsigned int bufferSize,
                          unsigned int width,
                          unsigned int height,
                          Dali::Pixel::Format pixelFormat )
: mBuffer( buffer ),
  mBufferSize( bufferSize ),
  mWidth( width ),
  mHeight( height ),
  mPixelFormat( pixelFormat )
{
}

PixelBuffer::~PixelBuffer()
{
  ReleaseBuffer();
}

PixelBufferPtr PixelBuffer::New( unsigned int width,
                                 unsigned int height,
                                 Dali::Pixel::Format pixelFormat )
{
  unsigned int bufferSize = width * height * Dali::Pixel::GetBytesPerPixel( pixelFormat );
  unsigned char* buffer = NULL;
  if( bufferSize > 0 )
  {
    buffer = static_cast<unsigned char*>( malloc ( bufferSize ) );
  }
  return new PixelBuffer( buffer, bufferSize, width, height, pixelFormat );
}

PixelBufferPtr PixelBuffer::New( unsigned char* buffer,
                                 unsigned int bufferSize,
                                 unsigned int width,
                                 unsigned int height,
                                 Dali::Pixel::Format pixelFormat )
{
  return new PixelBuffer( buffer, bufferSize, width, height, pixelFormat );
}

Dali::PixelData PixelBuffer::Convert( PixelBuffer& pixelBuffer )
{
  Dali::PixelData pixelData = Dali::PixelData::New( pixelBuffer.mBuffer,
                                                    pixelBuffer.mBufferSize,
                                                    pixelBuffer.mWidth,
                                                    pixelBuffer.mHeight,
                                                    pixelBuffer.mPixelFormat,
                                                    Dali::PixelData::FREE );
  pixelBuffer.mBuffer = NULL;
  pixelBuffer.mWidth = 0;
  pixelBuffer.mHeight = 0;
  pixelBuffer.mBufferSize = 0;

  return pixelData;
}

unsigned int PixelBuffer::GetWidth() const
{
  return mWidth;
}

unsigned int PixelBuffer::GetHeight() const
{
  return mHeight;
}

Dali::Pixel::Format PixelBuffer::GetPixelFormat() const
{
  return mPixelFormat;
}

unsigned char* PixelBuffer::GetBuffer() const
{
  return mBuffer;
}

unsigned int PixelBuffer::GetBufferSize() const
{
  return mBufferSize;
}

Dali::PixelData PixelBuffer::CreatePixelData() const
{
  unsigned char* destBuffer = NULL;

  if( mBufferSize > 0 )
  {
    destBuffer = static_cast<unsigned char*>( malloc( mBufferSize ) );
    memcpy( destBuffer, mBuffer, mBufferSize );
  }

  Dali::PixelData pixelData = Dali::PixelData::New( destBuffer, mBufferSize,
                                                    mWidth, mHeight,
                                                    mPixelFormat,
                                                    Dali::PixelData::FREE );
  return pixelData;
}

void PixelBuffer::ApplyMask( const PixelBuffer& mask )
{
  int byteOffset=0;
  int bitMask=0;

  Dali::Pixel::GetAlphaOffsetAndMask(mPixelFormat, byteOffset, bitMask);

  if( Dali::Pixel::HasAlpha( mPixelFormat ) && bitMask == 255 )
  {
    ApplyMaskToAlphaChannel( *this, mask );
  }
  else
  {
    PixelBufferPtr newPixelBuffer = CreateNewMaskedBuffer( *this, mask );
    ReleaseBuffer();

    // Take ownership of new buffer
    mBuffer = newPixelBuffer->mBuffer;
    newPixelBuffer->mBuffer = NULL;
    mPixelFormat = newPixelBuffer->mPixelFormat;
    mBufferSize = newPixelBuffer->mBufferSize;

    // On leaving scope, newPixelBuffer will get destroyed.
  }
}

void PixelBuffer::ReleaseBuffer()
{
  if( mBuffer )
  {
    free( mBuffer );
  }
}


}// namespace Adaptor
}// namespace Internal
}// namespace Dali
