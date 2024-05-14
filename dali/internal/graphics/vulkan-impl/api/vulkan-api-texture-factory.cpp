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

#include <dali/graphics/vulkan/api/vulkan-api-texture-factory.h>
#include <dali/graphics/vulkan/api/vulkan-api-texture.h>
#include <dali/graphics/vulkan/api/vulkan-api-native-texture.h>
#include <dali/graphics/vulkan/api/vulkan-api-controller.h>
#include <dali/graphics/vulkan/vulkan-graphics.h>

namespace Dali
{
namespace Graphics
{
namespace VulkanAPI
{

struct TextureFactory::Impl
{
  Impl( Dali::Graphics::TextureFactory& api, VulkanAPI::Controller& controller )
    : mApi( api ),
      mController( controller ),
      mGraphics( controller.GetGraphics() )
  {
  }

  // Explicit copy constructor used when cloning into new object
  Impl( const Impl& rhs )
  : mApi( rhs.mApi ),
    mController( rhs.mController ),
    mGraphics( rhs.mGraphics ),
    mType( rhs.mType ),
    mUsage( rhs.mUsage ),
    mSize( rhs.mSize ),
    mFormat( rhs.mFormat ),
    mMipmapFlags( rhs.mMipmapFlags ),
    mTiling( rhs.mTiling ),
    mData( rhs.mData ),
    mDataSizeInBytes( rhs.mDataSizeInBytes ),
    mNativeImageInterface( rhs.mNativeImageInterface )
  {
  }

  ~Impl() = default;

  std::unique_ptr< Graphics::Texture > Create()
  {
    auto retval = mNativeImageInterface ?
                  VulkanAPI::MakeUniqueVulkanNativeTexture( static_cast<Graphics::TextureFactory&>(mApi) ) :
                  std::make_unique< VulkanAPI::Texture >( static_cast<Graphics::TextureFactory&>(mApi) );

    if( retval->Initialize() )
    {
      return retval;
    }

    return nullptr;
  }

  Dali::Graphics::TextureFactory& mApi;
  VulkanAPI::Controller& mController;
  Vulkan::Graphics& mGraphics;

  Dali::Graphics::TextureDetails::Type mType;
  Dali::Graphics::TextureDetails::Usage mUsage;
  Dali::Graphics::Extent2D mSize;
  Dali::Graphics::Format mFormat;
  Dali::Graphics::TextureDetails::MipMapFlag mMipmapFlags;
  Dali::Graphics::TextureTiling mTiling;
  void* mData;
  uint32_t mDataSizeInBytes;
  Dali::NativeImageInterfacePtr mNativeImageInterface;
};

TextureFactory::TextureFactory( VulkanAPI::Controller& controller )
{
  mImpl = std::make_unique< Impl >( *this, controller );
}

TextureFactory::~TextureFactory() = default;

Graphics::TextureFactory& TextureFactory::SetType( Dali::Graphics::TextureDetails::Type type )
{
  mImpl->mType = type;
  return *this;
}

Graphics::TextureFactory& TextureFactory::SetSize( const Dali::Graphics::Extent2D& size )
{
  mImpl->mSize = size;
  return *this;
}

Graphics::TextureFactory& TextureFactory::SetUsage( Dali::Graphics::TextureDetails::Usage usage )
{
  mImpl->mUsage = usage;
  return *this;
}

Graphics::TextureFactory& TextureFactory::SetFormat( Dali::Graphics::TextureDetails::Format format )
{
  mImpl->mFormat = format;
  return *this;
}

Graphics::TextureFactory& TextureFactory::SetMipMapFlag( Dali::Graphics::TextureDetails::MipMapFlag mipMSapFlag )
{
  mImpl->mMipmapFlags = mipMSapFlag;
  return *this;
}

Graphics::TextureFactory& TextureFactory::SetData( void* pData )
{
  mImpl->mData = pData;
  return *this;
}

Graphics::TextureFactory& TextureFactory::SetNativeImage(  NativeImageInterfacePtr nativeImageInterface )
{
  mImpl->mNativeImageInterface = nativeImageInterface;
  return *this;
}

Graphics::TextureFactory& TextureFactory::SetDataSize( uint32_t dataSizeInBytes )
{
  mImpl->mDataSizeInBytes = dataSizeInBytes;
  return *this;
}

Graphics::TextureFactory& TextureFactory::SetTiling( Dali::Graphics::TextureTiling tiling )
{
  mImpl->mTiling = tiling;
  return *this;
}

std::unique_ptr< Graphics::Texture > TextureFactory::Create() const
{
  return mImpl->Create();
}

const Dali::Graphics::TextureDetails::Type& TextureFactory::GetType() const
{
  return mImpl->mType;
}

const Dali::Graphics::TextureDetails::Usage& TextureFactory::GetUsage() const
{
  return mImpl->mUsage;
}

const Dali::Graphics::Extent2D& TextureFactory::GetSize() const
{
  return mImpl->mSize;
}

const Dali::Graphics::TextureDetails::Format& TextureFactory::GetFormat() const
{
  return mImpl->mFormat;
}

const Dali::Graphics::TextureDetails::MipMapFlag& TextureFactory::GetMipMapFlag() const
{
  return mImpl->mMipmapFlags;
}

const void* TextureFactory::GetData() const
{
  return mImpl->mData;
}

const NativeImageInterfacePtr TextureFactory::GetNativeImage() const
{
  return mImpl->mNativeImageInterface;
}

uint32_t TextureFactory::GetDataSize() const
{
  return mImpl->mDataSizeInBytes;
}

Dali::Graphics::TextureTiling TextureFactory::GetTiling() const
{
  return mImpl->mTiling;
}

Vulkan::Graphics& TextureFactory::GetGraphics() const
{
  return mImpl->mGraphics;
}

VulkanAPI::Controller& TextureFactory::GetController() const
{
  return mImpl->mController;
}

std::unique_ptr<TextureFactory> TextureFactory::Clone()
{
  auto newFactory = std::unique_ptr<TextureFactory>( new TextureFactory() );
  newFactory->mImpl.reset( new Impl( *mImpl.get() ) );
  return newFactory;
}

}
}
}
