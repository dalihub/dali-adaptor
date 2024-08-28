#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_RENDER_TARGET_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_RENDER_TARGET_H

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

#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>

#include <dali/graphics-api/graphics-render-target-create-info.h>
#include <dali/graphics-api/graphics-render-target.h>
#include <dali/integration-api/adaptor-framework/render-surface-interface.h>

namespace Dali::Graphics::Vulkan
{
class Framebuffer;
class Surface;
class RenderPassImpl;

using RenderTargetResource = Resource<Graphics::RenderTarget, Graphics::RenderTargetCreateInfo>;

class RenderTarget : public RenderTargetResource
{
public:
  RenderTarget(const Graphics::RenderTargetCreateInfo& createInfo, VulkanGraphicsController& controller);

  ~RenderTarget() override;

  /**
   * @brief Called when GL resources are destroyed
   */
  void DestroyResource() override;

  /**
   * @brief Called when initializing the resource
   *
   * @return True on success
   */
  bool InitializeResource() override
  {
    // There is no graphic resource here. return true.
    return true;
  }

  /**
   * @brief Called when UniquePtr<> on client-side dies
   */
  void DiscardResource() override;

  /**
   * @copydoc Graphics::Vulkan::Resource::GetAllocationCallbacks()
   */
  [[nodiscard]] const Graphics::AllocationCallbacks* GetAllocationCallbacks() const override
  {
    return mCreateInfo.allocationCallbacks;
  }

  /**
   * @copydoc Graphics::Vulkan::Resource::InvokeDeleter()
   * Only intended for use by discard queue.
   */
  void InvokeDeleter() override
  {
    this->~RenderTarget();
  }

  /**
   * @brief Returns framebuffer associated with the render target
   */
  [[nodiscard]] Vulkan::Framebuffer* GetFramebuffer() const;

  /**
   * @brief Returns surface associated with the render target
   */
  [[nodiscard]] Integration::RenderSurfaceInterface* GetSurface() const;

  /**
   * @brief Returns the current framebuffer impl for this frame.
   * (May be either the swapchain's current fb, or the offscreen's fb).
   * @return the current framebuffer
   */
  [[nodiscard]] Vulkan::FramebufferImpl* GetCurrentFramebufferImpl() const;

  /**
   * Find a matching render pass for this render target
   * @param[in] renderPass A render pass to search for
   * @return a matching render pass implementation from the current framebuffer
   */
  [[nodiscard]] Vulkan::RenderPassImpl* GetRenderPass(const Graphics::RenderPass* renderPass) const;
  // Get Swapchain?

private:
  //  UniquePtr<Swapchain> mSwapchain;
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_RENDER_TARGET_H
