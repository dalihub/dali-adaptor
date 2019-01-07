#ifndef DALI_GRAPHICS_VULKAN_API_TEXTURE_FACTORY_H
#define DALI_GRAPHICS_VULKAN_API_TEXTURE_FACTORY_H

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

  Graphics::TextureFactory& SetType( Dali::Graphics::TextureDetails::Type type ) override;

  Graphics::TextureFactory& SetSize( const Dali::Graphics::RectSize& size ) override;

  Graphics::TextureFactory& SetFormat( Dali::Graphics::TextureDetails::Format format ) override;

  Graphics::TextureFactory& SetMipMapFlag( Dali::Graphics::TextureDetails::MipMapFlag mipMSapFlag ) override;

  Graphics::TextureFactory& SetUsage( Dali::Graphics::TextureDetails::Usage usage ) override;

  Graphics::TextureFactory& SetData( void* pData ) override;

  Graphics::TextureFactory& SetDataSize( uint32_t dataSizeInBytes ) override;

  // not copyable
  TextureFactory( const TextureFactory& ) = delete;

  TextureFactory& operator=( const TextureFactory& ) = delete;

  ~TextureFactory() override;

  std::unique_ptr< Graphics::Texture > Create() const override;

  /** Internal interface */
  const Dali::Graphics::TextureDetails::Type& GetType() const;

  const Dali::Graphics::RectSize& GetSize() const;

  const Dali::Graphics::TextureDetails::Format& GetFormat() const;

  const Dali::Graphics::TextureDetails::Usage& GetUsage() const;

  const Dali::Graphics::TextureDetails::MipMapFlag& GetMipMapFlag() const;

  const void* GetData() const;

  uint32_t GetDataSize() const;

  Vulkan::Graphics& GetGraphics() const;

  VulkanAPI::Controller& GetController() const;

protected:
  /// @brief default constructor
  TextureFactory() = default;

  // derived types should not be moved direcly to prevent slicing
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
