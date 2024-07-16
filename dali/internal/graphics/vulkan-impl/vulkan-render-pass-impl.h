#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_RENDER_PASS_IMPL_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_RENDER_PASS_IMPL_H

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

#include <dali/graphics-api/graphics-render-pass-create-info.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/public-api/common/vector-wrapper.h>

namespace Dali::Graphics::Vulkan
{
class Device;
class VulkanGraphicsController;
class RenderTarget;

class RenderPassImpl final : public Dali::Graphics::Vulkan::VkManaged
{
public:
  static RenderPassImpl* New(
    Vulkan::Device&                            device,
    const std::vector<FramebufferAttachment*>& colorAttachments,
    FramebufferAttachment*                     depthAttachment);

  RenderPassImpl(Vulkan::Device& device, vk::RenderPass renderPass);

  RenderPassImpl(Vulkan::Device& device, const Graphics::RenderPassCreateInfo& createInfo, std::vector<FramebufferAttachment*>& colorAttachments, FramebufferAttachment* depthAttachment);

  ~RenderPassImpl() override;

  vk::RenderPass GetVkHandle();

  std::vector<vk::ImageView>& GetAttachments();

  void SetAttachments(std::vector<vk::ImageView>& attachments);

  bool OnDestroy() override;

private:
  Device*                    mGraphicsDevice;
  vk::RenderPass             mVkRenderPass;
  std::vector<vk::ImageView> mAttachments{};
};
} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_RENDER_PASS_IMPL_H
