#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_IMPL_FRAMEBUFFER_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_IMPL_FRAMEBUFFER_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{

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
  friend class Device;

public:

  static FramebufferAttachment* NewColorAttachment( ImageView* imageView,
                                                   vk::ClearColorValue clearColorValue,
                                                   bool presentable );

  static FramebufferAttachment* NewDepthAttachment( ImageView* imageView,
                                                   vk::ClearDepthStencilValue clearDepthStencilValue );

  ImageView* GetImageView() const;

  const vk::AttachmentDescription& GetDescription() const;

  const vk::ClearValue& GetClearValue() const;

  AttachmentType GetType() const;

  bool IsValid() const;

private:
  FramebufferAttachment() = default;

  FramebufferAttachment( ImageView* imageView,
                         vk::ClearValue clearColor,
                         AttachmentType type,
                         bool presentable  );

  ImageView* mImageView;
  vk::AttachmentDescription mDescription;
  vk::ClearValue mClearValue;
  AttachmentType mType;
};

/**
 * Framebuffer encapsulates following objects:
 * - Images ( attachments )
 * - Framebuffer
 * - ImageViews
 */
class Framebuffer : public VkManaged
{
public:
  friend class Device;

  uint32_t GetWidth() const;

  uint32_t GetHeight() const;

  FramebufferAttachment* GetAttachment( AttachmentType type, uint32_t index ) const;

  std::vector<FramebufferAttachment*> GetAttachments( AttachmentType type ) const;

  uint32_t GetAttachmentCount( AttachmentType type ) const;

  vk::RenderPass GetRenderPass() const;

  vk::Framebuffer GetVkHandle() const;

  std::vector< vk::ClearValue > GetClearValues() const;

  bool OnDestroy() override;

private:

  Framebuffer( Device& graphicsDevice,
               const std::vector<FramebufferAttachment*>& colorAttachments,
               FramebufferAttachment* depthAttachment,
               vk::Framebuffer vkHandle,
               vk::RenderPass renderPass,
               uint32_t width,
               uint32_t height,
               bool externalRenderPass = false );

private:
  Device* mGraphicsDevice;

  uint32_t mWidth;
  uint32_t mHeight;

  std::vector<FramebufferAttachment*> mColorAttachments;
  FramebufferAttachment* mDepthAttachment;
  vk::Framebuffer mFramebuffer;
  vk::RenderPass mRenderPass;
  bool mExternalRenderPass;
};


} // Namespace Vulkan

} // Namespace Graphics

} // Namespace Dali

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_IMPL_FRAMEBUFFER_H
