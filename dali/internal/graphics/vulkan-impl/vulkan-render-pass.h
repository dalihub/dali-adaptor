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

#include <dali/graphics-api/graphics-render-pass-create-info.h>
#include <dali/graphics-api/graphics-render-pass.h>

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
  RenderPass(const Graphics::RenderPassCreateInfo& createInfo, VulkanGraphicsController& controller);

  ~RenderPass() override;

  [[nodiscard]] const Graphics::RenderPassCreateInfo& GetCreateInfo() const
  {
    return mCreateInfo;
  }

private:
  Graphics::RenderPassCreateInfo               mCreateInfo;
  VulkanGraphicsController&                    mController;
  std::vector<Graphics::AttachmentDescription> mAttachments;
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_RENDERPASS_H
