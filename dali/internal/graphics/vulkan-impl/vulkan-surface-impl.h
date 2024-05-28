#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_SURFACE_IMPL_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_SURFACE_IMPL_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

namespace Dali::Graphics::Vulkan
{
class Device;

class SurfaceImpl final : public Dali::Graphics::Vulkan::VkManaged
{
public:
  explicit SurfaceImpl(Device& device, vk::SurfaceKHR surfaceKhr);

  ~SurfaceImpl() final;

  /**
   * @return the handle to this surface
   */
  [[nodiscard]] vk::SurfaceKHR GetVkHandle() const;

  /**
   * @return the capability structure
   */
  [[nodiscard]] const vk::SurfaceCapabilitiesKHR& GetCapabilities() const;

  /**
   * @return the capability structure
   */
  [[nodiscard]] vk::SurfaceCapabilitiesKHR& GetCapabilities();

  /**
   * Update size of surface
   */
  void UpdateSize(unsigned int width, unsigned int height);

  bool OnDestroy() override;

private:
  Device*                    mGraphicsDevice;
  vk::SurfaceKHR             mSurface;
  vk::SurfaceCapabilitiesKHR mCapabilities;
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_SURFACE_IMPL_H
