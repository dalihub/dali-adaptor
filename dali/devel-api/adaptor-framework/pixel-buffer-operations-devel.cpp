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
#include <dali/devel-api/adaptor-framework/pixel-buffer-operations-devel.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/pixel-buffer-operations.h>

namespace DALI_NAMESPACE
{
namespace DevelPixelBufferOperations
{
void ApplyMask(PixelBuffer pixelBuffer, PixelBuffer mask, float contentScale, bool cropToMask)
{
  Internal::Adaptor::PixelBufferOperations::ApplyMask(pixelBuffer, mask, contentScale, cropToMask);
}

void Crop(PixelBuffer pixelBuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
  Internal::Adaptor::PixelBufferOperations::Crop(pixelBuffer, x, y, ImageDimensions(width, height));
}

void ApplyCenterCrop(PixelBuffer pixelBuffer, uint16_t width, uint16_t height)
{
  Internal::Adaptor::PixelBufferOperations::ApplyCenterCrop(pixelBuffer, width, height);
}

void ApplyLetterbox(PixelBuffer pixelBuffer, uint16_t width, uint16_t height)
{
  Internal::Adaptor::PixelBufferOperations::ApplyLetterbox(pixelBuffer, width, height);
}

void MultiplyColorByAlpha(PixelBuffer pixelBuffer)
{
  Internal::Adaptor::PixelBufferOperations::MultiplyColorByAlpha(pixelBuffer);
}

bool Rotate(PixelBuffer pixelBuffer, Degree angle)
{
  return Internal::Adaptor::PixelBufferOperations::Rotate(pixelBuffer, angle);
}

uint32_t GetBrightness(PixelBuffer pixelBuffer)
{
  return Internal::Adaptor::PixelBufferOperations::GetBrightness(pixelBuffer);
}

} // namespace DevelPixelBufferOperations

} //namespace DALI_NAMESPACE
