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

#include <dali/internal/graphics/vulkan-impl/vulkan-image-view-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>

namespace Dali::Graphics::Vulkan
{
class RenderPass;

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

class FramebufferAttachment
{
public:
  /**
   * Constructor
   *
   * @param[in] imageView The imageview of the attachment
   * @param[in] clearColor The color used to clear this attachment during CLEAR_OP
   * @param[in] type The attachment type (usually COLOR or DEPTH_STENCIL)
   * @param[in] presentable Whether the attachment is presentable (changes final layout)
   */
  FramebufferAttachment(std::unique_ptr<ImageView>& imageView,
                        vk::ClearValue              clearColor,
                        AttachmentType              type,
                        bool                        presentable);

  /**
   * Creates a new color attachment.
   *
   * @param[in] imageView The imageview of the attachment
   * @param[in] clearColorValue The color used to clear this attachment during CLEAR_OP
   * @param[in] presentable Whether the attachment is presentable (changes final layout)
   */
  static FramebufferAttachment* NewColorAttachment(std::unique_ptr<ImageView>& imageView,
                                                   vk::ClearColorValue         clearColorValue,
                                                   bool                        presentable);

  /**
   * Creates a new depth attachment.
   *
   * @param[in] imageView The imageview of the attachment
   * @param[in] clearDepthStencilValue The value used to clear this attachment during CLEAR_OP
   */
  static FramebufferAttachment* NewDepthAttachment(std::unique_ptr<ImageView>& imageView,
                                                   vk::ClearDepthStencilValue  clearDepthStencilValue);

  [[nodiscard]] ImageView* GetImageView() const;

  [[nodiscard]] const vk::AttachmentDescription& GetDescription() const;

  [[nodiscard]] const vk::ClearValue& GetClearValue() const;

  [[nodiscard]] AttachmentType GetType() const;

  [[nodiscard]] bool IsValid() const;

private:
  FramebufferAttachment() = default;

  std::unique_ptr<ImageView> mImageView;
  vk::AttachmentDescription  mDescription;
  vk::ClearValue             mClearValue;
  AttachmentType             mType{AttachmentType::UNDEFINED};
};

using OwnedAttachments = std::vector<std::unique_ptr<FramebufferAttachment>>;

/**
 * FramebufferImpl encapsulates following objects:
 * - Images ( attachments )
 * - FramebufferImpl
 * - ImageViews
 * - RenderPasses
 */
class FramebufferImpl
{
public:
  /**
   * @brief Create a new Framebuffer
   *
   * @param[in] device The vulkan device
   * @param[in] renderPass A shared handle to a compatible render pass.
   * @param[in] attachments The attachments. Framebuffer takes ownership of these
   * @param[in] width Width of the framebuffer
   * @param[in] height Height of the framebuffer
   * @param[in] hasDepthAttachment True if the last attachment is a depth buffer
   *
   * @return A new framebuffer object
   */
  static FramebufferImpl* New(
    Vulkan::Device&   device,
    RenderPassHandle  renderPass,
    OwnedAttachments& attachments,
    uint32_t          width,
    uint32_t          height,
    bool              hasDepthAttachment);

  /**
   * @brief Create a new Framebuffer
   *
   * @param[in] device The vulkan device
   * @param[in] renderPass A shared handle to a compatible render pass.
   * @param[in] attachments The attachments. Framebuffer takes ownership of these
   * @param[in] width Width of the framebuffer
   * @param[in] height Height of the framebuffer
   * @param[in] hasDepthAttachment True if the last attachment is a depth buffer
   *
   * @return A new framebuffer object
   */
  static FramebufferImpl* New(
    Vulkan::Device&                         device,
    RenderPassHandle                        renderPass,
    OwnedAttachments&                       colorAttachments,
    std::unique_ptr<FramebufferAttachment>& depthAttachment,
    uint32_t                                width,
    uint32_t                                height);

  /**
   * @brief Constructor
   *
   * @param[in] graphicsDevice The vulkan device
   * @param[in] attachments The attachments - framebuffer takes ownership
   * @param[in] vkHandle a handle to a created framebuffer
   * @param[in] renderPass a handle to a compatible render pass
   * @param[in] width Width of the framebuffer
   * @param[in] height Height of the framebuffer
   * @param[in] hasDepthAttachment True if the last attachment is a depth buffer
   */
  FramebufferImpl(Device&           graphicsDevice,
                  OwnedAttachments& attachments,
                  vk::Framebuffer   vkHandle,
                  RenderPassHandle  renderPass,
                  uint32_t          width,
                  uint32_t          height,
                  bool              hasDepthAttachment);

  void Destroy();

  [[nodiscard]] uint32_t GetWidth() const;

  [[nodiscard]] uint32_t GetHeight() const;

  [[nodiscard]] FramebufferAttachment* GetAttachment(AttachmentType type, uint32_t index) const;

  [[nodiscard]] std::vector<FramebufferAttachment*> GetAttachments(AttachmentType type) const;

  [[nodiscard]] uint32_t GetAttachmentCount(AttachmentType type) const;

  [[nodiscard]] RenderPassHandle GetImplFromRenderPass(RenderPass* renderPass); // May mutate mRenderPasses

  [[nodiscard]] RenderPassHandle GetRenderPass(uint32_t index) const;

  [[nodiscard]] uint32_t GetRenderPassCount() const;

  [[nodiscard]] vk::Framebuffer GetVkHandle() const;

  [[nodiscard]] std::vector<vk::ClearValue> GetClearValues() const;

private:
  Device* mGraphicsDevice;

  uint32_t mWidth;
  uint32_t mHeight;

  /**
   * Structure to map RenderPass to RenderPassImpl.
   */
  struct RenderPassMapElement
  {
    RenderPass*      renderPass{nullptr};
    RenderPassHandle renderPassImpl{nullptr};
  };
  using RenderPasses = std::vector<RenderPassMapElement>;

  OwnedAttachments mAttachments;
  vk::Framebuffer  mFramebuffer;
  RenderPasses     mRenderPasses;
  bool             mHasDepthAttachment{false};
};

} // Namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_FRAMEBUFFER_IMPL_H
