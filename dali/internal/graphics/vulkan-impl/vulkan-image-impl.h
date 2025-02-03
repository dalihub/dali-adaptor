#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_IMAGE_IMPL_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_IMAGE_IMPL_H

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
 *
 */

#include <dali/internal/graphics/vulkan-impl/vulkan-memory-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

namespace Dali::Graphics::Vulkan
{
class Device;
class MemoryImpl;

namespace vma
{
class Allocation;
}

// @todo use ImageImpl to make naming convention consistent

/**
 * Wrapper class for vk::Image
 */
class Image
{
public:
  static Image* New(Device& graphicsDevice, const vk::ImageCreateInfo& createInfo, vk::MemoryPropertyFlags memoryProperties);

  /**
   * Create the wrapper object, either for the given vkImage, or as a new image
   * that will get allocated.
   *
   * @param graphicsDevice The graphics Device
   * @param createInfo The creation structure
   * @param[in] externalImage  External image, or nullptr if not external
   */
  Image(Device& graphicsDevice, const vk::ImageCreateInfo& createInfo, vk::Image externalImage = nullptr);

  /**
   * @brief Destructor
   */
  ~Image();

  /**
   * Second stage initialization:
   * Creates new VkImage with given specification, allocates memory for the image,
   * and binds it.
   *
   * @param[in] memoryProperties The properties flags for the memory.
   */
  void Initialize(vk::MemoryPropertyFlags memoryProperties);

  /**
   * Destroys underlying Vulkan resources on the caller thread.
   *
   * @note Calling this function is unsafe and makes any further use of
   * image invalid.
   */
  void Destroy();

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
   * Set the image layout locally (Not in GPU)
   * @param[in] imageLayout The image layout.
   */
  void SetImageLayout(vk::ImageLayout imageLayout);

  /**
   * Create a memory barrier to transition from current layout to the new layout
   */
  vk::ImageMemoryBarrier CreateMemoryBarrier(vk::ImageLayout newLayout) const;

  /**
   * Create a memory barrier for (a future) transition from the given layout to the new layout
   * @param[in] layout
   * @param[in] newLayout
   * @return the memory barrier
   */
  vk::ImageMemoryBarrier CreateMemoryBarrier(vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;

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

  [[nodiscard]] Vulkan::MemoryImpl* GetMemory() const
  {
    return mMemory.get();
  }

private:
  Device&                            mDevice;
  vk::ImageCreateInfo                mCreateInfo;
  vk::Image                          mImage;
  vk::ImageLayout                    mImageLayout;
  vk::ImageAspectFlags               mAspectFlags;
  std::unique_ptr<MemoryImpl>        mMemory;
  bool                               mIsExternal;
  std::unique_ptr<::vma::Allocation> mVmaAllocation{nullptr};
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_IMAGE_IMPL_H
