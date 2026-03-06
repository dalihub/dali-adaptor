#ifndef DALI_INTERNAL_NATIVE_IMAGE_FACTORY_ANDROID_H
#define DALI_INTERNAL_NATIVE_IMAGE_FACTORY_ANDROID_H

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

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-factory.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class NativeImageFactoryAndroid : public NativeImageFactory
{
public:
  std::unique_ptr<NativeImage> CreateNativeImage(uint32_t width, uint32_t height, Dali::NativeImage::ColorDepth depth, Any nativeImage) override;

  std::unique_ptr<NativeImageQueue> CreateNativeImageQueue(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageQueue::ColorFormat colorFormat, Any nativeImageQueue) override;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_FACTORY_ANDROID_H
