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

#include <dali/graphics/vulkan/api/vulkan-api-sampler-factory.h>
#include <dali/graphics/vulkan/api/vulkan-api-sampler.h>

namespace Dali
{
namespace Graphics
{
namespace VulkanAPI
{

Dali::Graphics::SamplerFactory& SamplerFactory::SetAddressModeU( Dali::Graphics::SamplerAddressMode mode )
{
  mAddressModeU = mode;
  return *this;
}

Dali::Graphics::SamplerFactory& SamplerFactory::SetAddressModeV( Dali::Graphics::SamplerAddressMode mode )
{
  mAddressModeV = mode;
  return *this;
}

Dali::Graphics::SamplerFactory& SamplerFactory::SetAddressModeW( Dali::Graphics::SamplerAddressMode mode )
{
  mAddressModeW = mode;
  return *this;
}

Dali::Graphics::SamplerFactory& SamplerFactory::SetMinFilter( Dali::Graphics::SamplerFilter filter )
{
  mMinFilter = filter;
  return *this;
}

Dali::Graphics::SamplerFactory& SamplerFactory::SetMagFilter( Dali::Graphics::SamplerFilter filter )
{
  mMagFilter = filter;
  return *this;
}

Dali::Graphics::SamplerFactory& SamplerFactory::SetMipmapMode( Dali::Graphics::SamplerMipmapMode mipmapMode )
{
  mMipmapMode = mipmapMode;
  return *this;
}

Dali::Graphics::SamplerFactory& SamplerFactory::SetAnisotropyEnable( bool anisotropyEnable )
{
  mAnisotropyEnable = anisotropyEnable;
  return *this;
}

Dali::Graphics::SamplerFactory& SamplerFactory::SetMaxAnisotropy( float maxAnisotropy )
{
  mMaxAnisotropy = maxAnisotropy;
  return *this;
}

Dali::Graphics::SamplerFactory& SamplerFactory::SetMinLod( float minLod )
{
  mMinLod = minLod;
  return *this;
}

Dali::Graphics::SamplerFactory& SamplerFactory::SetMaxLod( float maxLod )
{
  mMaxLod = maxLod;
  return *this;
}

Dali::Graphics::SamplerFactory& SamplerFactory::SetUnnormalizeCoordinates( bool unnormalizedCoordinates )
{
  mUnnormalizedCoordinates = unnormalizedCoordinates;
  return *this;
}

Dali::Graphics::SamplerFactory& SamplerFactory::SetCompareEnable( bool compareEnable )
{
  mCompareEnable = compareEnable;
  return *this;
}

Dali::Graphics::SamplerFactory& SamplerFactory::SetCompareOp( Dali::Graphics::CompareOp compareOp )
{
  mCompareOp = compareOp;
  return *this;
}

SamplerFactory& SamplerFactory::Reset()
{
  mAddressModeU = Dali::Graphics::SamplerAddressMode{};
  mAddressModeV = Dali::Graphics::SamplerAddressMode{};
  mAddressModeW = Dali::Graphics::SamplerAddressMode{};
  mMinFilter = Dali::Graphics::SamplerFilter{};
  mMagFilter = Dali::Graphics::SamplerFilter{};
  mMipmapMode = Dali::Graphics::SamplerMipmapMode{};
  mCompareOp = Dali::Graphics::CompareOp{};
  mMaxAnisotropy = 1.0f; // value must be 1.0f as default if anisotropy feature isn't enabled
  mMinLod = 0.0f;
  mMaxLod = 0.0f;
  mCompareEnable = false;
  mAnisotropyEnable = false;
  mUnnormalizedCoordinates = false;
  return *this;
}

std::unique_ptr< Graphics::Sampler > SamplerFactory::Create() const
{
  return std::unique_ptr<VulkanAPI::Sampler>( new VulkanAPI::Sampler( mController, *this ) );
}

} // namespace API
} // namespace Graphics
} // namespace Dali
