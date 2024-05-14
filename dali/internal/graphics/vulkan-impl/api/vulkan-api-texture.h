#ifndef DALI_GRAPHICS_VULKAN_API_TEXTURE_H
#define DALI_GRAPHICS_VULKAN_API_TEXTURE_H

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

// INTERNAL INCLUDES
#include <dali/graphics-api/graphics-api-texture-factory.h>
#include <dali/graphics-api/graphics-api-texture.h>
#include <dali/graphics/vulkan/internal/vulkan-types.h>

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{
class Graphics;
}
namespace VulkanAPI
{



class TextureFactory;
class Controller;

/**
 * Implementation of Texture class for Vulkan backend
 */
class Texture : public Dali::Graphics::Texture
{
public:

  Texture() = delete;

  explicit Texture( Dali::Graphics::TextureFactory& factory );

  ~Texture() override;

  virtual bool Initialize();

  virtual bool IsSamplerImmutable() const;

  Vulkan::RefCountedImage GetImageRef() const;

  Vulkan::RefCountedImageView GetImageViewRef() const;

  Vulkan::RefCountedSampler GetSamplerRef() const;

  void CopyMemory( const void *srcMemory, uint32_t maxDataSize, Dali::Graphics::Extent2D srcExtent, Dali::Graphics::Offset2D dstOffset, uint32_t layer, uint32_t level, Dali::Graphics::TextureDetails::UpdateMode updateMode ) override;

  void CopyTexture( const Graphics::Texture &srcTexture, Dali::Graphics::Rect2D srcRegion, Dali::Graphics::Offset2D dstOffset, uint32_t layer, uint32_t level, Dali::Graphics::TextureDetails::UpdateMode updateMode ) override;

  void CopyBuffer(const Dali::Graphics::Buffer& buffer,
                  uint32_t bufferOffset,
                  Dali::Graphics::Extent2D extent2D,
                  Dali::Graphics::Offset2D textureOffset2D,
                  uint32_t layer,
                  uint32_t level,
                  Dali::Graphics::TextureUpdateFlags flags ) override;

  Dali::Graphics::MemoryRequirements GetMemoryRequirements() const override;

  /**
   * Returns structure with texture properties
   * @return The reference to immutable TextureProperties object
   */
  const Dali::Graphics::TextureProperties& GetProperties() override;

  /**
   * Initialises resources like memory, image view and samplers for previously
   * initialised image object. Used when lazy allocation is needed.
   */
  void InitialiseResources();

  /**
   * Tries to convert pixel data to the compatible format. As result it returns new buffer.
   * @param pData source data
   * @param sizeInBytes size of source data in bytes
   * @param width width in pixels
   * @param height height in pixels
   * @param outputBuffer reference to an output buffer
   * @return True if conversion was successful
   */
  bool TryConvertPixelData( const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, std::vector<uint8_t>& outputBuffer );

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
  bool TryConvertPixelData( const void* pData, uint32_t sizeInBytes, uint32_t width, uint32_t height, void* pOutputBuffer );

  /**
   * Direct copy memory to memory, used when linear tiling is enabled. This function
   * doesn't check if data is valid and doesn't perform format conversion.
   * @param info
   * @param sourceInfo
   * @param keepMapped if true, the memory stays mapped after the call
   */
  void CopyMemoryDirect( const Dali::Graphics::TextureUpdateInfo& info, const Dali::Graphics::TextureUpdateSourceInfo& sourceInfo, bool keepMapped );

private:
  /**
   * Validates initial format
   * @return if valid, returns existing format
   *         if possible conversion, returns new converted format
   *         if not supported returns vk::Format::eUndefined
   */
  vk::Format ValidateFormat( vk::Format sourceFormat );

  vk::Format ConvertApiToVk( Dali::Graphics::Format format );

  vk::ComponentMapping GetVkComponentMapping( Dali::Graphics::Format format );

protected:

  void CreateSampler();
  void CreateImageView();
  bool InitialiseTexture();

  void SetFormatAndUsage();

protected:

  std::unique_ptr<VulkanAPI::TextureFactory> mTextureFactory;
  VulkanAPI::Controller& mController;
  Vulkan::Graphics& mGraphics;

  Vulkan::RefCountedImage       mImage;
  Vulkan::RefCountedImageView   mImageView;
  Vulkan::RefCountedSampler     mSampler;

  uint32_t    mWidth;
  uint32_t    mHeight;
  vk::Format  mFormat;
  vk::Format  mConvertFromFormat { vk::Format::eUndefined };
  vk::ImageUsageFlags mUsage;
  vk::ImageLayout mLayout;
  vk::ComponentMapping mComponentMapping{};

  bool mDisableStagingBuffer { false };
  Dali::Graphics::TextureTiling  mTiling { Dali::Graphics::TextureTiling::OPTIMAL };
  std::unique_ptr<Dali::Graphics::TextureProperties> mProperties;
};

} // namespace VulkanAPI
} // namespace Graphics
} // namespace Dali
#endif // DALI_GRAPHICS_VULKAN_API_TEXTURE_H
