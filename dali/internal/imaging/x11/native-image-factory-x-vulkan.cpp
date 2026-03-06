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
#include <dali/internal/imaging/x11/native-image-factory-x.h>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-impl.h>
#include <dali/internal/imaging/common/native-image-queue-impl.h>

namespace Dali::Internal::Adaptor
{
std::unique_ptr<NativeImage> NativeImageFactoryX::CreateNativeImage(uint32_t width, uint32_t height, Dali::NativeImage::ColorDepth depth, Any nativeImage)
{
  return nullptr;
}

std::unique_ptr<NativeImageQueue> NativeImageFactoryX::CreateNativeImageQueue(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageQueue::ColorFormat colorFormat, Any nativeImageQueue)
{
  return nullptr;
}

std::unique_ptr<NativeImageFactory> GetNativeImageFactory()
{
  // returns native image factory
  return std::unique_ptr<NativeImageFactoryX>(new NativeImageFactoryX());
}

} // namespace Dali::Internal::Adaptor
