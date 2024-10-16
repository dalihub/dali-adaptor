#ifndef DALI_GRAPHICS_VULKAN_VKSURFACEXCB_H
#define DALI_GRAPHICS_VULKAN_VKSURFACEXCB_H

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
#ifndef VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#endif

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan/vulkan-hpp-wrapper.h>
#include <dali/internal/graphics/vulkan/vulkan-surface-factory.h>

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{

class VkSurfaceXcb final : public SurfaceFactory
{
public:
  /**
   * Instantiates surface factory
   * @param[in] renderSurface
   */
  VkSurfaceXcb(NativeWindowInterface& nativeWindow);

  virtual vk::SurfaceKHR Create(vk::Instance                   instance,
                                const vk::AllocationCallbacks* allocCallbacks) const override;

private:
  xcb_connection_t* mConnection;
  xcb_window_t      mWindow;
  vk::SurfaceKHR    mSurface;
};

} // Namespace Vulkan
} // Namespace Graphics
} // Namespace Dali

#endif // DALI_GRAPHICS_VULKAN_VKSURFACEXCB_H
