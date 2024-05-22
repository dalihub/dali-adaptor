#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_SURFACE
#define DALI_INTERNAL_GRAPHICS_VULKAN_SURFACE

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

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{

class Device;

class Surface : public VkManaged
{
  friend class Device;

public:

  ~Surface() final;

  /**
   *
   * @return
   */
  vk::SurfaceKHR GetVkHandle() const;

  /**
   * Returns size of surface
   * @return
   */
  const vk::SurfaceCapabilitiesKHR& GetCapabilities() const;

  /**
   * Update size of surface
   */
  void UpdateSize( unsigned int width, unsigned int height );

  bool OnDestroy() override;

private:
  Surface( Device& device );

private:
  Device* mGraphicsDevice;
  vk::SurfaceKHR mSurface;
  vk::SurfaceCapabilitiesKHR mCapabilities;
};

} // namespace Vulkan
} // namespace Graphics
} // namespace Dali

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_SURFACE
