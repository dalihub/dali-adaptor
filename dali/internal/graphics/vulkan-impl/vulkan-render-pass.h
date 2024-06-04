#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_RENDERPASS_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_RENDERPASS_H

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

#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>

#include <dali/graphics-api/graphics-render-pass.h>
#include <dali/graphics-api/graphics-render-pass-create-info.h>

namespace Dali::Graphics::Vulkan
{
class RenderPassImpl;

using RenderPassResource = Resource<Graphics::RenderPass, Graphics::RenderPassCreateInfo>;
class RenderPass : public RenderPassResource
{
public:
  RenderPass(const Graphics::RenderPassCreateInfo& createInfo, VulkanGraphicsController& controller);

  ~RenderPass() override;

  /**
   * @brief Called when GL resources are destroyed
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

  RenderPassImpl* GetImpl();

private:
  RenderPassImpl* mRenderPassImpl;
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_RENDERPASS_H
