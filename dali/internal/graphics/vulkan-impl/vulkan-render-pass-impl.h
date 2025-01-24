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
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-attachment.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-handle.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/public-api/common/vector-wrapper.h>

namespace Dali::Graphics::Vulkan
{
class Device;
class VulkanGraphicsController;
class RenderTarget;
class RenderPassImpl;

using RenderPassHandle = Handle<class RenderPassImpl>;

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
class RenderPassImpl : public VkSharedResource
{
public:
  class CreateInfo
  {
  public:
    SharedAttachments                      attachmentHandles;
    std::vector<vk::AttachmentReference>   colorAttachmentReferences;
    vk::AttachmentReference                depthAttachmentReference;
    std::vector<vk::AttachmentDescription> attachmentDescriptions;
    vk::SubpassDescription                 subpassDesc;
    std::array<vk::SubpassDependency, 2>   subpassDependencies;
    vk::RenderPassCreateInfo               createInfo;
  };

  static RenderPassHandle New(Vulkan::Device&             device,
                              const SharedAttachments&    colorAttachments,
                              FramebufferAttachmentHandle depthAttachment);

  static RenderPassHandle New(Vulkan::Device&                   device,
                              const RenderPassImpl::CreateInfo& createInfo);

  RenderPassImpl(Vulkan::Device& device, const SharedAttachments& colorAttachments, FramebufferAttachmentHandle depthAttachment);

  RenderPassImpl(Vulkan::Device& device, const RenderPassImpl::CreateInfo& createInfo);

  ~RenderPassImpl();

  bool OnDestroy() override;

  vk::RenderPass GetVkHandle();

  size_t GetAttachmentCount();

  bool HasDepthAttachment()
  {
    return mHasDepthAttachment;
  }

  CreateInfo& GetCreateInfo()
  {
    return mCreateInfo;
  }

  static void CreateCompatibleCreateInfo(
    CreateInfo&                        createInfo,
    const SharedAttachments&           colorAttachments,
    const FramebufferAttachmentHandle& depthAttachment,
    bool                               subpassForOffscreen);

private:
  void       CreateRenderPass();
  static int CreateSubPassDependencies(CreateInfo& createInfo, bool hasDepth, bool subpassForOffscreen);

private:
  Device*        mGraphicsDevice;
  CreateInfo     mCreateInfo;
  vk::RenderPass mVkRenderPass;
  bool           mHasDepthAttachment{false};
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_RENDER_PASS_IMPL_H
