#ifndef DALI_GRAPHICS_VULKAN_API_TEXTURE_FACTORY_H
#define DALI_GRAPHICS_VULKAN_API_TEXTURE_FACTORY_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

#include <dali/graphics-api/graphics-api-texture-factory.h>
#include <dali/graphics-api/graphics-api-texture-details.h>

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
class Controller;
class TextureFactory : public Dali::Graphics::TextureFactory
{
public:

  explicit TextureFactory( VulkanAPI::Controller& controller );

  Graphics::TextureFactory& SetType( Graphics::TextureDetails::Type type ) override;

  Graphics::TextureFactory& SetSize( const Graphics::Extent2D& size ) override;

  Graphics::TextureFactory& SetFormat( Graphics::Format format ) override;

  Graphics::TextureFactory& SetMipMapFlag( Graphics::TextureDetails::MipMapFlag mipMSapFlag ) override;

  Graphics::TextureFactory& SetUsage( Graphics::TextureDetails::Usage usage ) override;

  Graphics::TextureFactory& SetData( void* pData ) override;

  Graphics::TextureFactory& SetDataSize( uint32_t dataSizeInBytes ) override;

  Graphics::TextureFactory& SetNativeImage( Dali::NativeImageInterfacePtr nativeImageInterface ) override;

  Graphics::TextureFactory& SetTiling( Graphics::TextureTiling tiling ) override;

  // not copyable
  TextureFactory( const TextureFactory& ) = delete;

  TextureFactory& operator=( const TextureFactory& ) = delete;

  ~TextureFactory() override;

  std::unique_ptr< Graphics::Texture > Create() const override;

  /** Internal interface */
  const TextureDetails::Type& GetType() const;

  const Graphics::Extent2D& GetSize() const;

  const Format& GetFormat() const;

  const TextureDetails::Usage& GetUsage() const;

  const TextureDetails::MipMapFlag& GetMipMapFlag() const;

  Graphics::TextureTiling GetTiling() const;

  const void* GetData() const;

  const Dali::NativeImageInterfacePtr GetNativeImage() const;

  uint32_t GetDataSize() const;

  Vulkan::Graphics& GetGraphics() const;

  VulkanAPI::Controller& GetController() const;

  /**
   * Clones this texture factory into the new object
   * @return copy of texture factory
   */
  std::unique_ptr<TextureFactory> Clone();

protected:
  /// @brief default constructor
  TextureFactory() = default;

  // derived types should not be moved directly to prevent slicing
  TextureFactory( TextureFactory&& ) = default;

  TextureFactory& operator=( TextureFactory&& ) = default;

private:

  struct Impl;
  std::unique_ptr< Impl > mImpl;
};

}
}
}

#endif //DALI_GRAPHICS_VULKAN_API_TEXTURE_FACTORY_H
