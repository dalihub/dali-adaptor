#ifndef DALI_GRAPHICS_VULKAN_FRAMEBUFFER_ATTACHMENT_H
#define DALI_GRAPHICS_VULKAN_FRAMEBUFFER_ATTACHMENT_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

#include <dali/internal/graphics/vulkan-impl/vulkan-handle.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

namespace Dali::Graphics::Vulkan
{
class ImageView;

enum class AttachmentType
{
  COLOR,
  DEPTH_STENCIL,
  INPUT,
  RESOLVE,
  PRESERVE,
  UNDEFINED
};

class FramebufferAttachment : public VkSharedResource
{
public:
  /**
   * Constructor
   *
   * @param[in] imageView The imageview of the attachment
   * @param[in] clearColor The color used to clear this attachment during CLEAR_OP
   * @param[in] description Expected Load/Store ops
   * @param[in] type The attachment type (usually COLOR or DEPTH_STENCIL)
   * @param[in] presentable Whether the attachment is presentable (changes final layout)
   */
  FramebufferAttachment(
    std::unique_ptr<ImageView>&            imageView,
    vk::ClearValue                         clearColor,
    const Graphics::AttachmentDescription* description,
    AttachmentType                         type,
    bool                                   presentable);

  /**
   * Creates a new color attachment.
   *
   * @param[in] imageView The imageview of the attachment
   * @param[in] clearColorValue The color used to clear this attachment during CLEAR_OP
   * @param[in] description Expected Load/Store ops
   * @param[in] presentable Whether the attachment is presentable (changes final layout)
   */
  static FramebufferAttachment* NewColorAttachment(
    std::unique_ptr<ImageView>&            imageView,
    vk::ClearColorValue                    clearColorValue,
    const Graphics::AttachmentDescription* description,
    bool                                   presentable);

  /**
   * Creates a new depth attachment.
   *
   * @param[in] imageView The imageview of the attachment
   * @param[in] clearDepthStencilValue The value used to clear this attachment during CLEAR_OP
   * @param[in] description Expected Load/Store ops
   */
  static FramebufferAttachment* NewDepthAttachment(
    std::unique_ptr<ImageView>&            imageView,
    vk::ClearDepthStencilValue             clearDepthStencilValue,
    const Graphics::AttachmentDescription* description);

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

  AttachmentType mType{AttachmentType::UNDEFINED};
};

using FramebufferAttachmentHandle = Vulkan::Handle<FramebufferAttachment>; // Can share attachments
using SharedAttachments           = std::vector<FramebufferAttachmentHandle>;

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_FRAMEBUFFER_ATTACHMENT_H
