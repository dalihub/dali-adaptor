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

/**
 * Holder class for Vulkan RenderPass object.
 *
 * We need a render pass to create a Framebuffer; this can create a
 * compatible render pass given the specific attachments.
 *
 * FramebufferImpl will own it's RenderPassImpl(s), NOT the Graphics::RenderPass
 * implementation.
 *
 * When we want to use the FramebufferImpl (CommandBuffer::BeginRenderPass), then
 * we try and match the supplied Graphics::RenderPass to the creating render pass.
 * FramebufferImpl will create a separate compatible RenderPassImpl if a matching
 * render pass is NOT found.
 */
class RenderPassImpl final : public Dali::Graphics::Vulkan::VkManaged
{
public:
  struct CreateInfo
  {
    std::vector<vk::AttachmentReference>   colorAttachmentReferences;
    vk::AttachmentReference                depthAttachmentReference;
    std::vector<vk::AttachmentDescription> attachmentDescriptions;
    vk::SubpassDescription                 subpassDesc;
    std::array<vk::SubpassDependency, 2>   subpassDependencies;
    vk::RenderPassCreateInfo               createInfo;
  };

  static RenderPassImpl* New(
    Vulkan::Device&                            device,
    const std::vector<FramebufferAttachment*>& colorAttachments,
    FramebufferAttachment*                     depthAttachment);

  RenderPassImpl(Vulkan::Device& device, const std::vector<FramebufferAttachment*>& colorAttachments, FramebufferAttachment* depthAttachment);

  ~RenderPassImpl() override;

  vk::RenderPass GetVkHandle();

  bool OnDestroy() override;

  std::vector<vk::ImageView>& GetAttachments();

  CreateInfo& GetCreateInfo()
  {
    return mCreateInfo;
  }

private:
  void CreateCompatibleCreateInfo(
    const std::vector<FramebufferAttachment*>& colorAttachments,
    FramebufferAttachment*                     depthAttachment);

  void CreateRenderPass();

private:
  Device*                    mGraphicsDevice;
  CreateInfo                 mCreateInfo;
  vk::RenderPass             mVkRenderPass;
  std::vector<vk::ImageView> mAttachments{};
};
} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_RENDER_PASS_IMPL_H
