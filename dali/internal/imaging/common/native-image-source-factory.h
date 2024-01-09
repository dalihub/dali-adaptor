#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_FACTORY_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_FACTORY_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <memory>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class NativeImageSource;
class NativeImageSourceQueue;

class NativeImageSourceFactory
{
public:
  NativeImageSourceFactory()          = default;
  virtual ~NativeImageSourceFactory() = default;

  virtual std::unique_ptr<NativeImageSource> CreateNativeImageSource(uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource) = 0;

  virtual std::unique_ptr<NativeImageSourceQueue> CreateNativeImageSourceQueue(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorFormat colorFormat, Any nativeImageSourceQueue) = 0;
};

extern std::unique_ptr<NativeImageSourceFactory> GetNativeImageSourceFactory();

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_FACTORY_H
