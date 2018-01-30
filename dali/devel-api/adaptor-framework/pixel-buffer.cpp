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
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>

// EXTERNAL INLCUDES
#include <stdlib.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/pixel-buffer-impl.h>

namespace Dali
{
namespace Devel
{

PixelBuffer PixelBuffer::New( unsigned int width,
                              unsigned int height,
                              Dali::Pixel::Format pixelFormat )
{
  Internal::Adaptor::PixelBufferPtr internal =
    Internal::Adaptor::PixelBuffer::New( width, height, pixelFormat );
  return Devel::PixelBuffer( internal.Get() );
}

Dali::PixelData PixelBuffer::Convert( PixelBuffer& pixelBuffer )
{
  Dali::PixelData pixelData =
    Internal::Adaptor::PixelBuffer::Convert( GetImplementation(pixelBuffer) );
  pixelBuffer.Reset();
  return pixelData;
}

Dali::PixelData PixelBuffer::CreatePixelData() const
{
  return GetImplementation(*this).CreatePixelData();
}


PixelBuffer::PixelBuffer()
{
}

PixelBuffer::~PixelBuffer()
{
}

PixelBuffer::PixelBuffer( Internal::Adaptor::PixelBuffer* internal )
: BaseHandle( internal )
{
}

PixelBuffer::PixelBuffer(const PixelBuffer& handle)
: BaseHandle( handle )
{
}

PixelBuffer& PixelBuffer::operator=(const PixelBuffer& rhs)
{
  BaseHandle::operator=(rhs);
  return *this;
}

unsigned int PixelBuffer::GetWidth() const
{
  return GetImplementation(*this).GetWidth();
}

unsigned int PixelBuffer::GetHeight() const
{
  return GetImplementation(*this).GetHeight();
}

Pixel::Format PixelBuffer::GetPixelFormat() const
{
  return GetImplementation(*this).GetPixelFormat();
}

unsigned char* PixelBuffer::GetBuffer()
{
  return GetImplementation(*this).GetBuffer();
}

void PixelBuffer::ApplyMask( PixelBuffer mask, float contentScale, bool cropToMask )
{
  GetImplementation(*this).ApplyMask( GetImplementation( mask ), contentScale, cropToMask );
}

void PixelBuffer::ApplyGaussianBlur( const float blurRadius )
{
  GetImplementation(*this).ApplyGaussianBlur( blurRadius );
}

void PixelBuffer::Crop( uint16_t x, uint16_t y, uint16_t width, uint16_t height )
{
  GetImplementation(*this).Crop( x, y, ImageDimensions( width, height ) );
}

void PixelBuffer::Resize( uint16_t width, uint16_t height )
{
  GetImplementation(*this).Resize( ImageDimensions( width, height ) );
}

void PixelBuffer::MultiplyColorByAlpha()
{
  GetImplementation(*this).MultiplyColorByAlpha();
}

bool PixelBuffer::GetMetadata( Property::Map& metadata ) const
{
  return GetImplementation(*this).GetMetadata(metadata);
}

} // namespace Devel

} // namespace Dali
