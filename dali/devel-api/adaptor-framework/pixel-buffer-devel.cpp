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
#include <dali/devel-api/adaptor-framework/pixel-buffer-devel.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/pixel-buffer-impl.h>

namespace Dali
{
namespace DevelPixelBuffer
{
PixelData Convert(PixelBuffer& pixelBuffer, bool releaseAfterUpload)
{
  Dali::PixelData pixelData = Internal::Adaptor::PixelBuffer::Convert(GetImplementation(pixelBuffer), releaseAfterUpload);
  pixelBuffer.Reset();
  return pixelData;
}

PixelData CreatePixelData(PixelBuffer pixelBuffer)
{
  return GetImplementation(pixelBuffer).CreatePixelData();
}

void ApplyMask(PixelBuffer pixelBuffer, PixelBuffer mask, float contentScale, bool cropToMask)
{
  GetImplementation(pixelBuffer).ApplyMask(GetImplementation(mask), contentScale, cropToMask);
}

void Crop(PixelBuffer pixelBuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
  GetImplementation(pixelBuffer).Crop(x, y, ImageDimensions(width, height));
}

void ApplyCenterCrop(PixelBuffer pixelBuffer, uint16_t width, uint16_t height)
{
  GetImplementation(pixelBuffer).ApplyCenterCrop(width, height);
}

void ApplyLetterbox(PixelBuffer pixelBuffer, uint16_t width, uint16_t height)
{
  GetImplementation(pixelBuffer).ApplyLetterbox(width, height);
}

bool GetMetadata(PixelBuffer pixelBuffer, Property::Map& metadata)
{
  return GetImplementation(pixelBuffer).GetMetadata(metadata);
}

void MultiplyColorByAlpha(PixelBuffer pixelBuffer)
{
  GetImplementation(pixelBuffer).MultiplyColorByAlpha();
}

bool IsAlphaPreMultiplied(PixelBuffer pixelBuffer)
{
  return GetImplementation(pixelBuffer).IsAlphaPreMultiplied();
}

bool Rotate(PixelBuffer pixelBuffer, Degree angle)
{
  return GetImplementation(pixelBuffer).Rotate(angle);
}

uint32_t GetBrightness(PixelBuffer pixelBuffer)
{
  return GetImplementation(pixelBuffer).GetBrightness();
}

} // namespace DevelPixelBuffer

} // namespace Dali
