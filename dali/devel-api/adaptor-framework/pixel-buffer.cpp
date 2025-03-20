/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
PixelBuffer PixelBuffer::New(uint32_t            width,
                             uint32_t            height,
                             Dali::Pixel::Format pixelFormat)
{
  Internal::Adaptor::PixelBufferPtr internal =
    Internal::Adaptor::PixelBuffer::New(width, height, pixelFormat);
  return Devel::PixelBuffer(internal.Get());
}

Dali::PixelData PixelBuffer::Convert(PixelBuffer& pixelBuffer)
{
  return Convert(pixelBuffer, false);
}

Dali::PixelData PixelBuffer::Convert(PixelBuffer& pixelBuffer, bool releaseAfterUpload)
{
  Dali::PixelData pixelData = Internal::Adaptor::PixelBuffer::Convert(GetImplementation(pixelBuffer), releaseAfterUpload);
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

PixelBuffer::PixelBuffer(Internal::Adaptor::PixelBuffer* internal)
: BaseHandle(internal)
{
}

PixelBuffer::PixelBuffer(const PixelBuffer& handle) = default;

PixelBuffer& PixelBuffer::operator=(const PixelBuffer& rhs) = default;

PixelBuffer::PixelBuffer(PixelBuffer&& handle) noexcept = default;

PixelBuffer& PixelBuffer::operator=(PixelBuffer&& rhs) noexcept = default;

uint32_t PixelBuffer::GetWidth() const
{
  return GetImplementation(*this).GetWidth();
}

uint32_t PixelBuffer::GetHeight() const
{
  return GetImplementation(*this).GetHeight();
}

uint32_t PixelBuffer::GetStride() const
{
  return GetImplementation(*this).GetStride();
}

uint32_t PixelBuffer::GetStrideBytes() const
{
  return GetImplementation(*this).GetStrideBytes();
}

Pixel::Format PixelBuffer::GetPixelFormat() const
{
  return GetImplementation(*this).GetPixelFormat();
}

uint8_t* PixelBuffer::GetBuffer()
{
  return GetImplementation(*this).GetBuffer();
}

const uint8_t* PixelBuffer::GetBuffer() const
{
  return GetImplementation(*this).GetConstBuffer();
}

void PixelBuffer::ApplyMask(PixelBuffer mask, float contentScale, bool cropToMask)
{
  GetImplementation(*this).ApplyMask(GetImplementation(mask), contentScale, cropToMask);
}

void PixelBuffer::ApplyGaussianBlur(const float blurRadius)
{
  GetImplementation(*this).ApplyGaussianBlur(blurRadius);
}

void PixelBuffer::Crop(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
  GetImplementation(*this).Crop(x, y, ImageDimensions(width, height));
}

void PixelBuffer::Resize(uint16_t width, uint16_t height)
{
  GetImplementation(*this).Resize(ImageDimensions(width, height));
}

void PixelBuffer::MultiplyColorByAlpha()
{
  GetImplementation(*this).MultiplyColorByAlpha();
}

bool PixelBuffer::GetMetadata(Property::Map& metadata) const
{
  return GetImplementation(*this).GetMetadata(metadata);
}

bool PixelBuffer::Rotate(Degree angle)
{
  return GetImplementation(*this).Rotate(angle);
}

bool PixelBuffer::IsAlphaPreMultiplied() const
{
  return GetImplementation(*this).IsAlphaPreMultiplied();
}

uint32_t PixelBuffer::GetBrightness() const
{
  return GetImplementation(*this).GetBrightness();
}

} // namespace Devel

} // namespace Dali
