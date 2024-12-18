#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_FACTORY_VULKAN_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_FACTORY_VULKAN_H

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

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-source-factory.h>
#include <dali/internal/imaging/common/native-image-source-impl.h>
#include <dali/internal/imaging/common/native-image-source-queue-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class NativeImageSourceFactoryTizenVulkan : public NativeImageSourceFactory
{
public:
  std::unique_ptr<NativeImageSource> CreateNativeImageSource(uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource) override;

  std::unique_ptr<NativeImageSourceQueue> CreateNativeImageSourceQueue(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorFormat colorFormat, Any nativeImageSourceQueue) override;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_FACTORY_VULKAN_H
