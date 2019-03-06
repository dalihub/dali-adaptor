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

  void CopyTexture( const Dali::Graphics::Texture &srcTexture, Dali::Graphics::Rect2D srcRegion, Dali::Graphics::Offset2D dstOffset, uint32_t layer, uint32_t level, Dali::Graphics::TextureDetails::UpdateMode updateMode ) override;

  void CopyBuffer( const Dali::Graphics::Buffer &srcBuffer, Dali::Graphics::Extent2D srcExtent, Dali::Graphics::Offset2D dstOffset, uint32_t layer, uint32_t level, Dali::Graphics::TextureDetails::UpdateMode updateMode) override;

  vk::Format ConvertApiToVk( Dali::Graphics::Format format );

  vk::ComponentMapping GetVkComponentMapping( Dali::Graphics::Format format );

protected:

  void CreateSampler();
  void CreateImageView();
  bool InitialiseTexture();

  void SetFormatAndUsage();

protected:

  VulkanAPI::TextureFactory& mTextureFactory;
  VulkanAPI::Controller& mController;
  Vulkan::Graphics& mGraphics;

  Vulkan::RefCountedImage       mImage;
  Vulkan::RefCountedImageView   mImageView;
  Vulkan::RefCountedSampler     mSampler;

  uint32_t    mWidth {};
  uint32_t    mHeight {};
  vk::Format  mFormat {};
  vk::ImageUsageFlags mUsage {};
  vk::ImageLayout mLayout {};
  vk::ComponentMapping mComponentMapping{};

  bool mDisableStagingBuffer { false };
};

} // namespace VulkanAPI
} // namespace Graphics
} // namespace Dali
#endif // DALI_GRAPHICS_VULKAN_API_TEXTURE_H
