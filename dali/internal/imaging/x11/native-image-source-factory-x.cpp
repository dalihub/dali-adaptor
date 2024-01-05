/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/imaging/x11/native-image-source-factory-x.h>

// INTERNAL HEADERS
#include <dali/internal/imaging/x11/native-image-source-impl-x.h>
#include <dali/internal/imaging/x11/native-image-source-queue-impl-x.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
std::unique_ptr<NativeImageSource> NativeImageSourceFactoryX::CreateNativeImageSource(uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource)
{
  return std::unique_ptr<NativeImageSource>(NativeImageSourceX::New(width, height, depth, nativeImageSource));
}

std::unique_ptr<NativeImageSourceQueue> NativeImageSourceFactoryX::CreateNativeImageSourceQueue(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorFormat colorFormat, Any nativeImageSourceQueue)
{
  return std::unique_ptr<NativeImageSourceQueue>(NativeImageSourceQueueX::New(queueCount, width, height, colorFormat, nativeImageSourceQueue));
}

// this should be created from somewhere
std::unique_ptr<NativeImageSourceFactory> GetNativeImageSourceFactory()
{
  // returns native image source factory
  return std::unique_ptr<NativeImageSourceFactoryX>(new NativeImageSourceFactoryX());
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
