#ifndef DALI_GRAPHICS_VULKAN_IMAGE_VIEW
#define DALI_GRAPHICS_VULKAN_IMAGE_VIEW

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

class Device;

/*
 * ImageView
 */
class ImageView : public VkManaged
{
  friend class Device;

public:

  ~ImageView() override;

  /**
   *
   * @return
   */
  vk::ImageView GetVkHandle() const;

  /**
   * Returns bound Image
   * @return
   */
  const Image* GetImage() const;

  /**
   *
   * @return
   */
  uint32_t GetLayerCount() const;

  /**
   *
   * @return
   */
  uint32_t GetMipLevelCount() const;

  /**
   *
   * @return
   */
  vk::ImageAspectFlags GetImageAspectMask() const;

  const ImageView& ConstRef();

  ImageView& Ref();

  bool OnDestroy() override;

private:
  ImageView(Device& graphicsDevice,
            const Image* image,
            vk::ImageViewCreateInfo createInfo );

private:
  Device* mGraphicsDevice;
  const Image* mImage;
  vk::ImageViewCreateInfo mCreateInfo;
  vk::ImageView mImageView;
};

} //namespace Vulkan
} //namespace Graphics
} //namespace Dali

#endif //DALI_GRAPHICS_VULKAN_IMAGE_VIEW
