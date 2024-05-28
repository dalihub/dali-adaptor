#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_SURFACE_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_SURFACE_H

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
 */

#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>
#include <dali/graphics-api/graphics-surface.h>
#include <dali/graphics-api/graphics-controller.h>

namespace Dali::Graphics::Vulkan
{

using SurfaceResource = Resource<Graphics::Surface, Graphics::SurfaceCreateInfo>;

/**
 * Structure to manager lifecycle of graphics surface.
 */
class Surface : public SurfaceResource
{
public:
  Surface(const Graphics::SurfaceCreateInfo& createInfo,
          VulkanGraphicsController& graphicsController);

  ~Surface() override;

  /**
   * @brief Called when GPU resources are destroyed
   */
  void DestroyResource() override;

  /**
   * @brief Called when initializing the resource
   *
   * @return True on success
   */
  bool InitializeResource() override;

  /**
   * @brief Called when UniquePtr<> on client-side dies
   */
  void DiscardResource() override;
};

} // namespace Dali::Graphics::Vulkan

#endif //DALI_INTERNAL_GRAPHICS_VULKAN_SURFACE_H
