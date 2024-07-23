#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_FRAMEBUFFER_IMPL_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_FRAMEBUFFER_IMPL_H

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

#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

namespace Dali::Graphics::Vulkan
{
class RenderPass;
class RenderPassImpl;

enum class AttachmentType
{
  COLOR,
  DEPTH_STENCIL,
  INPUT,
  RESOLVE,
  PRESERVE,
  UNDEFINED
};

class Device;

class FramebufferAttachment : public VkManaged
{
public:
  FramebufferAttachment(ImageView*     imageView,
                        vk::ClearValue clearColor,
                        AttachmentType type,
                        bool           presentable);

  static FramebufferAttachment* NewColorAttachment(ImageView*          imageView,
                                                   vk::ClearColorValue clearColorValue,
                                                   bool                presentable);

  static FramebufferAttachment* NewDepthAttachment(ImageView*                 imageView,
                                                   vk::ClearDepthStencilValue clearDepthStencilValue);

  [[nodiscard]] ImageView* GetImageView() const;

  [[nodiscard]] const vk::AttachmentDescription& GetDescription() const;

  [[nodiscard]] const vk::ClearValue& GetClearValue() const;

  [[nodiscard]] AttachmentType GetType() const;

  [[nodiscard]] bool IsValid() const;

private:
  FramebufferAttachment() = default;

  ImageView*                mImageView{nullptr};
  vk::AttachmentDescription mDescription;
  vk::ClearValue            mClearValue;
  AttachmentType            mType{AttachmentType::UNDEFINED};
};

/**
 * FramebufferImpl encapsulates following objects:
 * - Images ( attachments )
 * - FramebufferImpl
 * - ImageViews
 */
class FramebufferImpl : public VkManaged
{
public:
  static FramebufferImpl* New(
    Vulkan::Device&                      device,
    RenderPassImpl*                      renderPass,
    std::vector<FramebufferAttachment*>& attachments,
    uint32_t                             width,
    uint32_t                             height,
    bool                                 hasDepthAttachment);

  static FramebufferImpl* New(
    Vulkan::Device&                            device,
    RenderPassImpl*                            renderPass,
    const std::vector<FramebufferAttachment*>& colorAttachments,
    FramebufferAttachment*                     depthAttachment,
    uint32_t                                   width,
    uint32_t                                   height);

  FramebufferImpl(Device&                                    graphicsDevice,
                  const std::vector<FramebufferAttachment*>& attachments,
                  vk::Framebuffer                            vkHandle,
                  const RenderPassImpl&                      renderPass,
                  uint32_t                                   width,
                  uint32_t                                   height,
                  bool                                       hasDepthAttachment);

  [[nodiscard]] uint32_t GetWidth() const;

  [[nodiscard]] uint32_t GetHeight() const;

  [[nodiscard]] FramebufferAttachment* GetAttachment(AttachmentType type, uint32_t index) const;

  [[nodiscard]] std::vector<FramebufferAttachment*> GetAttachments(AttachmentType type) const;

  [[nodiscard]] uint32_t GetAttachmentCount(AttachmentType type) const;

  [[nodiscard]] RenderPassImpl* GetRenderPass(RenderPass* renderPass); // May mutate mRenderPasses

  [[nodiscard]] vk::Framebuffer GetVkHandle() const;

  [[nodiscard]] std::vector<vk::ClearValue> GetClearValues() const;

  bool OnDestroy() override;

private:
  Device* mGraphicsDevice;

  uint32_t mWidth;
  uint32_t mHeight;

  /**
   * Structure to map RenderPass to RenderPassImpl.
   */
  struct RenderPassMapElement
  {
    RenderPass*     renderPass{nullptr};
    RenderPassImpl* renderPassImpl{nullptr};
  };
  using RenderPasses = std::vector<RenderPassMapElement>;

  std::vector<FramebufferAttachment*> mAttachments;
  vk::Framebuffer                     mFramebuffer;
  RenderPasses                        mRenderPasses;
  bool                                mHasDepthAttachment{false};
};

} // Namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_FRAMEBUFFER_IMPL_H
