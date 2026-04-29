#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_RENDERPASS_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_RENDERPASS_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include <dali/graphics-api/graphics-render-pass-create-info.h>
#include <dali/graphics-api/graphics-render-pass.h>

#include <unordered_set>

namespace Dali::Graphics::Vulkan
{
class RenderPassImpl;

/**
 * This class represents a set of render pass operations.
 *
 * This class is not directly associated with a graphics resource, and is not
 * responsible for the lifetime of actual vulkan render pass objects. That is
 * instead the responsibility of the Framebuffer implementation.
 *
 * When this render pass is used, the Framebuffer will find or create a compatible
 * render pass implementation, and cache it.
 */
class RenderPass : public Graphics::RenderPass
{
public:
  /**
   * Observer interface for objects that cache state derived from this
   * RenderPass (e.g. FramebufferImpl's RenderPass→RenderPassImpl lookup).
   * Implementations opt in via AddLifecycleObserver.
   */
  class LifecycleObserver
  {
  public:
    /**
     * Called when the RenderPass is reinitialized in place or is about to be
     * destroyed. Observers should drop any cached state keyed on this object.
     */
    virtual void RenderPassInvalidated(const RenderPass* renderPass) = 0;

  protected:
    virtual ~LifecycleObserver() = default;
  };

  RenderPass(const Graphics::RenderPassCreateInfo& createInfo, VulkanGraphicsController& controller);

  ~RenderPass() override;

  [[nodiscard]] const Graphics::RenderPassCreateInfo& GetCreateInfo() const
  {
    return mCreateInfo;
  }

  /**
   * Replace the attachments and create-info in place. Called by the controller
   * when a caller passes this object as the oldRenderPass argument to
   * CreateRenderPass — reusing the object preserves identity, so raw pointers
   * cached elsewhere (notably in FramebufferImpl::mRenderPasses) stay valid.
   *
   * Observers are notified so they can drop any cached state (e.g. backend
   * RenderPassImpl handles keyed off the previous load/store ops or
   * attachment shape).
   */
  void Reinitialize(const Graphics::RenderPassCreateInfo& createInfo);

  /**
   * Register an observer. Safe to call more than once with the same observer
   * — only one entry is kept.
   */
  void AddLifecycleObserver(LifecycleObserver& observer);

  /**
   * Deregister. Must be called from the observer's destructor to avoid
   * dangling notifications.
   */
  void RemoveLifecycleObserver(LifecycleObserver& observer);

private:
  Graphics::RenderPassCreateInfo               mCreateInfo;
  VulkanGraphicsController&                    mController;
  std::vector<Graphics::AttachmentDescription> mAttachments;
  std::unordered_set<LifecycleObserver*>       mLifecycleObservers;
  bool                                         mObserverNotifying{false};
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_RENDERPASS_H
