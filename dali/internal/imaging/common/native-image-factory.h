#ifndef DALI_INTERNAL_NATIVE_IMAGE_FACTORY_H
#define DALI_INTERNAL_NATIVE_IMAGE_FACTORY_H

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

// EXTERNAL INCLUDES
#include <memory>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-queue.h>
#include <dali/public-api/adaptor-framework/native-image.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class NativeImage;
class NativeImageQueue;

class NativeImageFactory
{
public:
  NativeImageFactory()          = default;
  virtual ~NativeImageFactory() = default;

  virtual std::unique_ptr<NativeImage> CreateNativeImage(uint32_t width, uint32_t height, Dali::NativeImage::ColorDepth depth, Any nativeImage) = 0;

  virtual std::unique_ptr<NativeImageQueue> CreateNativeImageQueue(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageQueue::ColorFormat colorFormat, Any nativeImageQueue) = 0;
};

// Needs exporting as it's called by the Graphics Libraries
extern DALI_ADAPTOR_API std::unique_ptr<NativeImageFactory> GetNativeImageFactory();

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_FACTORY_H
