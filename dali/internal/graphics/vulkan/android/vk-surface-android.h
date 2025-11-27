#ifndef DALI_GRAPHICS_VULKAN_SURFACE_ANDROID_H
#define DALI_GRAPHICS_VULKAN_SURFACE_ANDROID_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/vulkan/vulkan-hpp-wrapper.h>
#include <dali/internal/graphics/vulkan/vulkan-surface-factory.h>

// EXTERNAL INCLUDES
#include <android_native_app_glue.h>

namespace Dali
{
class RenderSurface;

namespace Graphics
{
namespace Vulkan
{
class VkSurfaceAndroid final : public SurfaceFactory
{
public:
  VkSurfaceAndroid(NativeWindowInterface& renderSurface);
  VkSurfaceAndroid(ANativeWindow* window);

  virtual vk::SurfaceKHR Create(
    vk::Instance                   instance,
    const vk::AllocationCallbacks* allocCallbacks) const override;

private:
  ANativeWindow* mWindow;
};

} // Namespace Vulkan
} // Namespace Graphics
} // Namespace Dali

#endif // DALI_GRAPHICS_VULKAN_SURFACE_ANDROID_H
