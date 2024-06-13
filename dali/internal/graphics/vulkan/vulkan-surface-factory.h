#ifndef DALI_GRAPHICS_VULKAN_SURFACE_FACTORY_H
#define DALI_GRAPHICS_VULKAN_SURFACE_FACTORY_H

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

// EXTERNAL INCLUDES
#include <dali/internal/graphics/vulkan/vulkan-hpp-wrapper.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/surface-factory.h>

namespace Dali::Graphics::Vulkan
{

class SurfaceFactory : public Dali::Graphics::SurfaceFactory
{
public:

  SurfaceFactory() = default;

  virtual vk::SurfaceKHR Create( vk::Instance instance,
                                 const vk::AllocationCallbacks* allocCallbacks,
                                 vk::PhysicalDevice physicalDevice ) const = 0;
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_SURFACE_FACTORY_H
