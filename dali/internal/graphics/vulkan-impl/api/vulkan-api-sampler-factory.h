#ifndef DALI_VULKAN_API_SAMPLER_FACTORY_H
#define DALI_VULKAN_API_SAMPLER_FACTORY_H

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

#include <dali/graphics-api/graphics-api-sampler-factory.h>
#include "vulkan-api-controller.h"

namespace Dali
{
namespace Graphics
{
namespace VulkanAPI
{

/**
 * @brief Interface class for ShaderFactory types in the graphics API.
 */
class SamplerFactory : public Dali::Graphics::SamplerFactory
{
public:

  SamplerFactory( Controller& controller )
  : mController( controller )
  {

  }

  Dali::Graphics::SamplerFactory& SetAddressModeU( Dali::Graphics::SamplerAddressMode mode ) override;

  Dali::Graphics::SamplerFactory& SetAddressModeV( Dali::Graphics::SamplerAddressMode mode ) override;

  Dali::Graphics::SamplerFactory& SetAddressModeW( Dali::Graphics::SamplerAddressMode mode ) override;

  Dali::Graphics::SamplerFactory& SetMinFilter( Dali::Graphics::SamplerFilter filter ) override;

  Dali::Graphics::SamplerFactory& SetMagFilter( Dali::Graphics::SamplerFilter filter ) override;

  Dali::Graphics::SamplerFactory& SetMipmapMode( Dali::Graphics::SamplerMipmapMode mipmapMode ) override;

  Dali::Graphics::SamplerFactory& SetAnisotropyEnable( bool anisotropyEnable ) override;

  Dali::Graphics::SamplerFactory& SetMaxAnisotropy( float maxAnisotropy ) override;

  Dali::Graphics::SamplerFactory& SetMinLod( float minLod ) override;

  Dali::Graphics::SamplerFactory& SetMaxLod( float maxLod ) override;

  Dali::Graphics::SamplerFactory& SetUnnormalizeCoordinates( bool unnormalizedCoordinates ) override;

  Dali::Graphics::SamplerFactory& SetCompareEnable( bool compareEnable ) override;

  Dali::Graphics::SamplerFactory& SetCompareOp( Dali::Graphics::CompareOp compareOp ) override;

  // not copyable
  SamplerFactory(const Dali::Graphics::SamplerFactory&) = delete;
  SamplerFactory& operator=(const SamplerFactory&) = delete;

  ~SamplerFactory() = default;

  PointerType Create() const override;

protected:

  /// @brief default constructor
  SamplerFactory() = default;

  // derived types should not be moved directly to prevent slicing
  SamplerFactory(SamplerFactory&&) = default;
  SamplerFactory& operator=(SamplerFactory&&) = default;

public:

  SamplerFactory& Reset();

  VulkanAPI::Controller& mController;

  Dali::Graphics::SamplerAddressMode mAddressModeU {};
  Dali::Graphics::SamplerAddressMode mAddressModeW {};
  Dali::Graphics::SamplerAddressMode mAddressModeV {};
  Dali::Graphics::SamplerFilter      mMinFilter {};
  Dali::Graphics::SamplerFilter      mMagFilter {};
  Dali::Graphics::SamplerMipmapMode  mMipmapMode {};
  Dali::Graphics::CompareOp          mCompareOp {};
  float                   mMaxAnisotropy { 0.0f };
  float                   mMinLod { 0.0f };
  float                   mMaxLod { 0.0f };
  bool                    mCompareEnable {};
  bool                    mAnisotropyEnable { false };
  bool                    mUnnormalizedCoordinates { false };


};

} // namespace API
} // namespace Graphics
} // namespace Dali

#endif // DALI_VULKAN_API_SAMPLER_FACTORY_H
