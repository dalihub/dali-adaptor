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
#include <dali/internal/window-system/common/native-image-surface-factory.h>
#include <dali/internal/window-system/tizen-wayland/native-image-surface-impl-ecore-wl-egl.h>

namespace Dali::Internal::Adaptor
{
std::unique_ptr<Dali::Internal::Adaptor::NativeImageSurface> NativeImageSurfaceFactory::CreateNativeImageSurface(Dali::NativeImageSourceQueuePtr queue)
{
  return std::make_unique<Internal::Adaptor::NativeImageSurfaceEcoreWl>(queue);
}
} // namespace Dali::Internal::Adaptor