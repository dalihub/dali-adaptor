#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_IMAGE_IMPL_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_IMAGE_IMPL_H

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

namespace Dali::Graphics::Vulkan
{

class Device;
class Memory;

class Image : public VkManaged
{
  friend class Device;

public:

  /**
   * Returns underlying Vulkan object
   * @return
   */
  [[nodiscard]] vk::Image GetVkHandle() const;

  /**
   * Returns VkImageLayout associated with the image
   * @return
   */
  [[nodiscard]] vk::ImageLayout GetImageLayout() const;

  /**
   * Returns width in pixels
   * @return
   */
  [[nodiscard]] uint32_t GetWidth() const;

  /**
   * Returns height in pixels
   * @return
   */
  [[nodiscard]] uint32_t GetHeight() const;

  /**
   * Returns number of layers
   * @return
   */
  [[nodiscard]] uint32_t GetLayerCount() const;

  /**
   * Returns number of mipmap levels
   * @return
   */
  [[nodiscard]] uint32_t GetMipLevelCount() const;

  /**
   * Returns pixel format
   * @return
   */
  [[nodiscard]] vk::Format GetFormat() const;

  /**
   * returns image type ( VkImageType)
   * @return
   */
  [[nodiscard]] vk::ImageType GetImageType() const;

  /**
   * Returns used image tiling mode
   * @return
   */
  [[nodiscard]] vk::ImageTiling GetImageTiling() const;

  [[nodiscard]] vk::ImageAspectFlags GetAspectFlags() const;

  /**
   *
   * @return
   */
  [[nodiscard]] vk::ImageUsageFlags GetUsageFlags() const;

  [[nodiscard]] vk::SampleCountFlagBits GetSampleCount() const;

  void SetImageLayout( vk::ImageLayout imageLayout );

  const Image& ConstRef();

  Image& Ref();

  bool OnDestroy() override;

  [[nodiscard]] Memory* GetMemory() const
  {
    return mDeviceMemory.get();
  }

  /**
   * Destroys underlying Vulkan resources on the caller thread.
   *
   * @note Calling this function is unsafe and makes any further use of
   * image invalid.
   */
  void DestroyNow();

private:

  /**
   * Creates new VkImage with given specification, it doesn't
   * bind the memory.
   * @param graphics
   * @param createInfo
   */
  Image( Device& graphicsDevice, const vk::ImageCreateInfo& createInfo, vk::Image externalImage = nullptr );

  /**
   * Destroys used Vulkan resource objects
   * @param device Vulkan device
   * @param image Vulkan image
   * @param memory Vulkan device memory
   * @param allocator Pointer to the Vulkan allocator callbacks
   */
  static void DestroyVulkanResources( vk::Device device, vk::Image image, vk::DeviceMemory memory, const vk::AllocationCallbacks* allocator );

private:
  Device* mGraphicsDevice;
  vk::ImageCreateInfo mCreateInfo;
  vk::Image mImage;
  vk::ImageLayout mImageLayout;
  vk::ImageAspectFlags mAspectFlags;

  std::unique_ptr<Memory> mDeviceMemory;
  bool mIsExternal;
};

} // namespace Dali::Graphics::Vulkan





#endif // DALI_INTERNAL_GRAPHICS_VULKAN_IMAGE_IMPL_H
