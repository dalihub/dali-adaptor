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
 *
 */

// CLASS HEADER
#include <dali/internal/graphics/vulkan-impl/vulkan-sampler.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-graphics-controller.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-sampler-impl.h>

namespace Dali::Graphics::Vulkan
{
namespace
{
constexpr vk::Filter ConvertFilter(Dali::Graphics::SamplerFilter filter)
{
  switch(filter)
  {
    case Dali::Graphics::SamplerFilter::LINEAR:
      return vk::Filter::eLinear;
    case Dali::Graphics::SamplerFilter::NEAREST:
      return vk::Filter::eNearest;
  }
  return vk::Filter{};
}

constexpr vk::SamplerAddressMode ConvertAddressMode(Dali::Graphics::SamplerAddressMode mode)
{
  switch(mode)
  {
    case Dali::Graphics::SamplerAddressMode::CLAMP_TO_EDGE:
      return vk::SamplerAddressMode::eClampToEdge;
    case Dali::Graphics::SamplerAddressMode::CLAMP_TO_BORDER:
      return vk::SamplerAddressMode::eClampToBorder;
    case Dali::Graphics::SamplerAddressMode::MIRROR_CLAMP_TO_EDGE:
      return vk::SamplerAddressMode::eMirrorClampToEdge;
    case Dali::Graphics::SamplerAddressMode::MIRRORED_REPEAT:
      return vk::SamplerAddressMode::eMirroredRepeat;
    case Dali::Graphics::SamplerAddressMode::REPEAT:
      return vk::SamplerAddressMode::eRepeat;
  }
  return vk::SamplerAddressMode{};
}

constexpr vk::SamplerMipmapMode ConvertMipmapMode(Dali::Graphics::SamplerMipmapMode mode)
{
  switch(mode)
  {
    case Dali::Graphics::SamplerMipmapMode::NONE:
      return vk::SamplerMipmapMode::eNearest;
    case Dali::Graphics::SamplerMipmapMode::LINEAR:
      return vk::SamplerMipmapMode::eLinear;
    case Dali::Graphics::SamplerMipmapMode::NEAREST:
      return vk::SamplerMipmapMode::eNearest;
  }
  return vk::SamplerMipmapMode{};
}

} // namespace

Sampler::Sampler(const Graphics::SamplerCreateInfo& createInfo, VulkanGraphicsController& controller)
: SamplerResource(createInfo, controller)
{
}

Sampler::~Sampler() = default;

void Sampler::DestroyResource()
{
  if(mSamplerImpl)
  {
    mSamplerImpl->Destroy();
    mSamplerImpl = nullptr;
  }
}

bool Sampler::InitializeResource()
{
  vk::SamplerCreateInfo createInfo{};
  createInfo.setMinFilter(ConvertFilter(mCreateInfo.minFilter))
    .setMagFilter(ConvertFilter(mCreateInfo.magFilter))
    .setAddressModeU(ConvertAddressMode(mCreateInfo.addressModeU))
    .setAddressModeV(ConvertAddressMode(mCreateInfo.addressModeV))
    .setAddressModeW(ConvertAddressMode(mCreateInfo.addressModeW))
    .setMipmapMode(ConvertMipmapMode(mCreateInfo.mipMapMode))
    .setCompareEnable(vk::Bool32(mCreateInfo.compareEnable))
    .setUnnormalizedCoordinates(vk::Bool32(mCreateInfo.unnormalizeCoordinates))
    .setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
    .setAnisotropyEnable(vk::Bool32(mCreateInfo.anisotropyEnable))
    .setMaxAnisotropy(mCreateInfo.maxAnisotropy)
    .setMinLod(mCreateInfo.minLod)
    .setMaxLod(mCreateInfo.maxLod);

  mSamplerImpl = SamplerImpl::New(mController.GetGraphicsDevice(), createInfo);

  return true;
}

void Sampler::DiscardResource()
{
  GetController().DiscardResource(this);
}

} // namespace Dali::Graphics::Vulkan
