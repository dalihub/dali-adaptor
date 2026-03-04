#ifndef DALI_INTERNAL_NATIVE_IMAGE_SURFACE_FACTORY_H
#define DALI_INTERNAL_NATIVE_IMAGE_SURFACE_FACTORY_H

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
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class NativeImageSurface;

class NativeImageSurfaceFactory
{
public:
  /**
   * Factory function for native image surface
   * A native image surface is created.
   *
   * @param [in] queue the native image surface handle
   * @return A pointer to a newly allocated surface
   *
   * Needs exporting as it's called by one of the the other libraries
   */
  static DALI_ADAPTOR_API std::unique_ptr<Dali::Internal::Adaptor::NativeImageSurface> CreateNativeImageSurface(Dali::NativeImageSourceQueuePtr queue);
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SURFACE_FACTORY_H
