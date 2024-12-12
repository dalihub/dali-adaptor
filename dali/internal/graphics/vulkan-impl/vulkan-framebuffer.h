#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_FRAMEBUFFER_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_FRAMEBUFFER_H

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
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>

#include <dali/graphics-api/graphics-framebuffer-create-info.h>
#include <dali/graphics-api/graphics-framebuffer.h>

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{
class FramebufferImpl;

using FramebufferResource = Resource<Graphics::Framebuffer, Graphics::FramebufferCreateInfo>;

class Framebuffer : public FramebufferResource
{
public:
  Framebuffer(const Graphics::FramebufferCreateInfo& createInfo, VulkanGraphicsController& controller);

  ~Framebuffer() override;

  /**
   * @brief Called when resources are destroyed
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

  /**
   * @copydoc Graphics::Vulkan::ResourceBase::GetAllocationCallbacks()
   */
  [[nodiscard]] const Graphics::AllocationCallbacks* GetAllocationCallbacks() const override
  {
    return mCreateInfo.allocationCallbacks;
  }

  /**
   * @copydoc Graphics::Vulkan::ResourceBase::InvokeDeleter()
   * Only intended for use by discard queue.
   */
  void InvokeDeleter() override
  {
    this->~Framebuffer();
  }

  FramebufferImpl* GetImpl()
  {
    return mFramebufferImpl;
  }

private:
  FramebufferImpl*              mFramebufferImpl;
  std::vector<RenderPassHandle> mRenderPasses;
};

} // namespace Vulkan
} // namespace Graphics
} // namespace Dali

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_FRAMEBUFFER_H
