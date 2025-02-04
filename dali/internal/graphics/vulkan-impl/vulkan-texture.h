#ifndef DALI_GRAPHICS_VULKAN_TEXTURE_IMPL_H
#define DALI_GRAPHICS_VULKAN_TEXTURE_IMPL_H

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

#include <dali/graphics-api/graphics-texture-create-info.h>
#include <dali/graphics-api/graphics-texture.h>
#include <dali/graphics-api/graphics-types.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-resource.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/internal/graphics/vulkan/vulkan-hpp-wrapper.h>

namespace Dali::Graphics::Vulkan
{
class Device;
class Memory;
class Image;
class ImageView;
class SamplerImpl;

using TextureResource = Resource<Graphics::Texture, Graphics::TextureCreateInfo>;

class Texture : public TextureResource
{
public:
  static Texture* New(Device& graphicsDevice);

  Texture(const Graphics::TextureCreateInfo& createInfo, VulkanGraphicsController& controller);
  ~Texture();

  /**
   * @copydoc Graphics::Vulkan::Resource::InitializeResource();
   */
  bool InitializeResource() override;

  /**
   * @copydoc Graphics::Vulkan::Resource::DiscardResource();
   */
  void DiscardResource() override;
  /**
   * @copydoc Graphics::Vulkan::Resource::DestroyResource();
   */
  void DestroyResource() override;

  /**
   * @copydoc Graphics::Vulkan::Resource::GetAllocationCallbacks()
   */
  [[nodiscard]] const Graphics::AllocationCallbacks* GetAllocationCallbacks() const override
  {
    return mCreateInfo.allocationCallbacks;
  }

  /**
   * @copydoc Graphics::Vulkan::Resource::InvokeDeleter()
   * Only intended for use by discard queue.
   */
  void InvokeDeleter() override
  {
    this->~Texture();
  }

  bool IsSamplerImmutable() const;

  Image* GetImage() const;

  ImageView* GetImageView() const;

  /**
   * Create a new image view onto the image.
   */
  std::unique_ptr<ImageView> CreateImageView();

  SamplerImpl* GetSampler() const;

  // @todo Temporary. We need to use the newer APIs
  enum UpdateMode
  {
    UNDEFINED,
    DEFERRED,
    IMMEDIATE
  };

  void SetFormatAndUsage();

  Dali::Graphics::MemoryRequirements GetMemoryRequirements() const;

  bool InitializeTexture();

  /**
   * Returns structure with texture properties
   * @return The reference to immutable TextureProperties object
   */
  const Dali::Graphics::TextureProperties& GetProperties();

  /**
   * Initialises resources like memory, image view and samplers for previously
   * initialised image object. Used when lazy allocation is needed.
   */
  void InitializeImageView();

  /**
   * Tries to convert pixel data to the compatible format. As result it returns new buffer.
   * @param pData source data
   * @param sizeInBytes size of source data in bytes
   * @param width width in pixels
   * @param height height in pixels
   * @param outputBuffer reference to an output buffer
   * @return True if conversion was successful
   */
  bool TryConvertPixelData(const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, std::vector<uint8_t>& outputBuffer);

  /**
   * Tries to convert pixel data to the compatible format. The result is written into the specified memory area.
   * The memory must be allocated and large enough to accomodate output data.
   * @param pData  source data
   * @param sizeInBytes  size of source data in bytes
   * @param width width in pixels
   * @param height height in pixels
   * @param pOutputBuffer pointer to a valid output buffer
   * @return True if conversion was successful
   */
  bool TryConvertPixelData(const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, void* pOutputBuffer);

  /**
   * @param[in] buffer The buffer to copy from
   * @param[in] bufferOffset The start of the texture within the buffer
   * @param[in] extent2D The size of the data
   * @param[in] textureOffset2D
   * @param[in] layer
   * @param[in] level
   * @param[in] flags
   */
  void CopyBuffer(
    const Dali::Graphics::Buffer&      buffer,
    uint32_t                           bufferOffset,
    Dali::Graphics::Extent2D           extent2D,
    Dali::Graphics::Offset2D           textureOffset2D,
    uint32_t                           layer,
    uint32_t                           level,
    Dali::Graphics::TextureUpdateFlags flags);

  /**
   * Direct copy memory to memory, used when linear tiling is enabled. This function
   * doesn't check if data is valid and doesn't perform format conversion.
   * @param info
   * @param sourceInfo
   * @param keepMapped if true, the memory stays mapped after the call
   */
  void CopyMemoryDirect(
    const Dali::Graphics::TextureUpdateInfo&       info,
    const Dali::Graphics::TextureUpdateSourceInfo& sourceInfo,
    bool                                           keepMapped);

private:
  /**
   * Validates initial format
   * @return if valid, returns existing format
   *         if possible conversion, returns new converted format
   *         if not supported returns vk::Format::eUndefined
   */
  vk::Format ValidateFormat(vk::Format sourceFormat);

  vk::Format ConvertApiToVk(Dali::Graphics::Format format);

  vk::ComponentMapping GetVkComponentMapping(Dali::Graphics::Format format);

private:
  Vulkan::Device& mDevice;

  Image*       mImage;
  ImageView*   mImageView;
  SamplerImpl* mSampler;

  uint32_t             mWidth;
  uint32_t             mHeight;
  vk::Format           mFormat;
  vk::Format           mConvertFromFormat{vk::Format::eUndefined};
  vk::ImageUsageFlags  mUsage;
  vk::ImageLayout      mLayout;
  vk::ComponentMapping mComponentMapping{};

  bool                          mDisableStagingBuffer{false};
  Dali::Graphics::TextureTiling mTiling{Dali::Graphics::TextureTiling::OPTIMAL};

  std::unique_ptr<Dali::Graphics::TextureProperties> mProperties;
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_TEXTURE_IMPL_H
