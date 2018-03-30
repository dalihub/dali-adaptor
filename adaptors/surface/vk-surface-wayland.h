#ifndef DALI_GRAPHICS_VULKAN_VKSURFACEWAYLAND_H
#define DALI_GRAPHICS_VULKAN_VKSURFACEWAYLAND_H

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#ifndef VK_USE_PLATFORM_WAYLAND_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif

// INTERNAL INCLUDES
#include <dali/integration-api/graphics/vulkan/vk-surface-factory.h>
#include <adaptors/ecore/wayland/window-render-surface.h>
#include <adaptors/surface/wayland_native.h>


// EXTERNAL INCLUDES
#include <vulkan/vulkan.hpp>


namespace Dali
{
class RenderSurface;

namespace Graphics
{
namespace Vulkan
{
class VkSurfaceWayland final : public Dali::Integration::Graphics::Vulkan::VkSurfaceFactory 
{
public:

  VkSurfaceWayland(Dali::RenderSurface& renderSurface);

  VkSurfaceWayland(::wl_display* display, ::wl_surface* surface);

  virtual vk::SurfaceKHR Create(vk::Instance instance, vk::AllocationCallbacks* allocCallbacks,
                                vk::PhysicalDevice physicalDevice) const override;


private:
//Wayland_data *mData;
  wl_display *w_display;
  wl_surface *w_surface;
  vk::SurfaceKHR  mSurface;
};

} // Namespace Vulkan
} // Namespace Graphics
} // Namespace Dali

#endif // DALI_GRAPHICS_VULKAN_VKSURFACEXCB_H
