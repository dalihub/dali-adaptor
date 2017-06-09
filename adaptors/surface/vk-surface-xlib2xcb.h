#ifndef DALI_GRAPHICS_VULKAN_VKSURFACEXLIB2XCB_H
#define DALI_GRAPHICS_VULKAN_VKSURFACEXLIB2XCB_H

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

#ifndef VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#ifndef VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#endif

// INTERNAL INCLUDES
#include <dali/integration-api/graphics/vk-surface-factory.h>

// EXTERNAL INCLUDES
#include <vulkan/vulkan.hpp>

namespace Dali
{
class RenderSurface;

namespace Graphics
{
namespace Vulkan
{
/**
 * This surface exists only because of ( probably ) Nvidia driver bug.
 * Presenting swapchain that uses Xlib surface crashes. Class VkSurfaceXlib2Xcb
 * takes Xlib window arguments but creates Xcb surface. It's a workaround.
 */
class VkSurfaceXlib2Xcb final : public Dali::Integration::Graphics::Vulkan::VkSurfaceFactory
{
public:
  VkSurfaceXlib2Xcb(Dali::RenderSurface& renderSurface);

  VkSurfaceXlib2Xcb(Display* display, Window window);

  virtual vk::SurfaceKHR Create(vk::Instance instance, vk::AllocationCallbacks* allocCallbacks,
                                vk::PhysicalDevice physicalDevice) const override;

private:
  xcb_connection_t* mConnection;
  xcb_window_t      mWindow;
  vk::SurfaceKHR    mSurface;
};

} // Namespace Vulkan
} // Namespace Graphics
} // Namespace Dali

#endif // DALI_GRAPHICS_VULKAN_VKSURFACEXLIB2XCB_H
