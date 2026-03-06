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
#include <dali/devel-api/adaptor-framework/native-image-devel.h>

//INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-impl.h>

namespace Dali
{
namespace DevelNativeImage
{
bool EncodeToFile(NativeImage& image, const std::string& filename, const uint32_t quality)
{
  return Dali::Internal::Adaptor::NativeImage::GetImplementation(image).EncodeToFile(filename, quality);
}

uint8_t* AcquireBuffer(NativeImage& image, uint32_t& width, uint32_t& height, uint32_t& stride)
{
  return Dali::Internal::Adaptor::NativeImage::GetImplementation(image).AcquireBuffer(width, height, stride);
}

bool ReleaseBuffer(NativeImage& image, const Rect<uint32_t>& updatedArea)
{
  return Dali::Internal::Adaptor::NativeImage::GetImplementation(image).ReleaseBuffer(updatedArea);
}

bool SetPixels(NativeImage& image, uint8_t* pixbuf, const Pixel::Format& pixelFormat)
{
  return Dali::Internal::Adaptor::NativeImage::GetImplementation(image).SetPixels(pixbuf, pixelFormat);
}

void SetResourceDestructionCallback(NativeImage& image, EventThreadCallback* callback, bool ownedCallback)
{
  Dali::Internal::Adaptor::NativeImage::GetImplementation(image).SetResourceDestructionCallback(callback, ownedCallback);
}

void EnableBackBuffer(NativeImage& image, bool enable)
{
  Dali::Internal::Adaptor::NativeImage::GetImplementation(image).EnableBackBuffer(enable);
}

} // namespace DevelNativeImage

} // namespace Dali
