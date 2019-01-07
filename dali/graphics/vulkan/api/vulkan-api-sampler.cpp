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

#include <dali/graphics/vulkan/api/vulkan-api-sampler.h>
#include <dali/graphics/vulkan/api/vulkan-api-sampler-factory.h>
#include <dali/graphics/vulkan/api/vulkan-api-controller.h>
#include <dali/graphics/vulkan/vulkan-graphics.h>

namespace Dali
{
namespace Graphics
{
namespace VulkanAPI
{
namespace
{
constexpr vk::Filter ConvertFilter( Dali::Graphics::SamplerFilter filter )
{
  switch(filter)
  {
    case Dali::Graphics::SamplerFilter::LINEAR: return vk::Filter::eLinear;
    case Dali::Graphics::SamplerFilter::NEAREST: return vk::Filter::eNearest;
  }
  return vk::Filter{};
}

constexpr vk::SamplerAddressMode ConvertAddressMode( Dali::Graphics::SamplerAddressMode mode )
{
  switch(mode)
  {
    case Dali::Graphics::SamplerAddressMode::CLAMP_TO_EDGE: return vk::SamplerAddressMode::eClampToEdge;
    case Dali::Graphics::SamplerAddressMode::CLAMP_TO_BORDER: return vk::SamplerAddressMode::eClampToBorder;
    case Dali::Graphics::SamplerAddressMode::MIRROR_CLAMP_TO_EDGE: return vk::SamplerAddressMode::eMirrorClampToEdge;
    case Dali::Graphics::SamplerAddressMode::MIRRORED_REPEAT: return vk::SamplerAddressMode::eMirroredRepeat;
    case Dali::Graphics::SamplerAddressMode::REPEAT: return vk::SamplerAddressMode::eRepeat;
  }
  return vk::SamplerAddressMode{};
}

constexpr vk::SamplerMipmapMode ConvertMipmapMode( Dali::Graphics::SamplerMipmapMode mode )
{
  switch(mode)
  {
    case Dali::Graphics::SamplerMipmapMode::LINEAR: return vk::SamplerMipmapMode::eLinear;
    case Dali::Graphics::SamplerMipmapMode::NEAREST: return vk::SamplerMipmapMode::eNearest;
  }
  return vk::SamplerMipmapMode{};
}

}

Sampler::Sampler( Controller& controller, const SamplerFactory& factory )
: mController( controller )
{
  mSampler = controller.GetGraphics().CreateSampler(
                         vk::SamplerCreateInfo()
                           .setMinFilter( ConvertFilter( factory.mMinFilter ) )
                           .setMagFilter( ConvertFilter( factory.mMagFilter ) )
                           .setAddressModeU( ConvertAddressMode( factory.mAddressModeU ) )
                           .setAddressModeV( ConvertAddressMode( factory.mAddressModeV ) )
                           .setAddressModeW( ConvertAddressMode( factory.mAddressModeW ) )
                           .setMipmapMode( ConvertMipmapMode( factory.mMipmapMode ) )
                           .setCompareEnable( vk::Bool32(factory.mCompareEnable) )
                           .setUnnormalizedCoordinates( vk::Bool32(factory.mUnnormalizedCoordinates) )
                           .setBorderColor( vk::BorderColor::eFloatOpaqueBlack )
                           .setAnisotropyEnable( vk::Bool32(factory.mAnisotropyEnable) )
                           .setMaxAnisotropy( factory.mMaxAnisotropy )
                           .setMinLod( factory.mMinLod )
                           .setMaxLod( factory.mMaxLod )
                       );
}

} // namespace VulkanAPI
} // namespace Graphics
} // namespace Dali
