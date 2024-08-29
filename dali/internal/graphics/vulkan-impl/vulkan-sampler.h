#ifndef DALI_GRAPHICS_VULKAN_SAMPLER_H
#define DALI_GRAPHICS_VULKAN_SAMPLER_H

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
#include <dali/graphics-api/graphics-sampler-create-info.h>
#include <dali/graphics-api/graphics-sampler.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>

namespace Dali::Graphics::Vulkan
{
using SamplerResource = Resource<Graphics::Sampler, Graphics::SamplerCreateInfo>;

class Sampler : public SamplerResource
{
public:
  /**
   * @brief Constructor
   * @param[in] createInfo Valid createInfo structure
   * @param[in] controller Reference to the controller
   */
  Sampler(const Graphics::SamplerCreateInfo& createInfo, VulkanGraphicsController& controller);

  /**
   * @brief Destructor
   */
  ~Sampler() override;

  /**
   * @brief Called when GPU resources are destroyed
   */
  void DestroyResource();

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

#endif
