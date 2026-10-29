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
#include <dali/public-api/adaptor-framework/pixel-buffer-operations.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/pixel-buffer-integ.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/pixel-buffer-operations.h>

namespace DALI_NAMESPACE
{
namespace
{
// Keeps the deprecated member functions Dali::PixelBuffer::ApplyGaussianBlur() and
// Dali::PixelBuffer::Resize() working: their implementations live here, and dali-core
// calls them through this registration, made when this library is loaded.
bool ApplyGaussianBlurOperation(Dali::PixelBuffer& pixelBuffer, float blurRadius)
{
  return Internal::Adaptor::PixelBufferOperations::ApplyGaussianBlur(pixelBuffer, blurRadius);
}

bool ResizeOperation(Dali::PixelBuffer& pixelBuffer, uint16_t width, uint16_t height)
{
  return Internal::Adaptor::PixelBufferOperations::Resize(pixelBuffer, ImageDimensions(width, height));
}

struct ImageOperationsRegistrar
{
  ImageOperationsRegistrar()
  {
    Dali::Integration::RegisterPixelBufferImageOperations(ApplyGaussianBlurOperation, ResizeOperation);
  }
};
ImageOperationsRegistrar gImageOperationsRegistrar;
} // namespace

namespace PixelBufferOperations
{
bool ApplyGaussianBlur(PixelBuffer pixelBuffer, float blurRadius)
{
  return Internal::Adaptor::PixelBufferOperations::ApplyGaussianBlur(pixelBuffer, blurRadius);
}

bool Resize(PixelBuffer pixelBuffer, uint16_t width, uint16_t height)
{
  return Internal::Adaptor::PixelBufferOperations::Resize(pixelBuffer, ImageDimensions(width, height));
}

} // namespace PixelBufferOperations

} //namespace DALI_NAMESPACE
