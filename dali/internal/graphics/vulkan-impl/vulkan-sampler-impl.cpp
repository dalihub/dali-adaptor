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

#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-sampler-impl.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{
SamplerImpl* SamplerImpl::New(Device& device, const vk::SamplerCreateInfo& samplerCreateInfo)
{
  auto sampler = new SamplerImpl(device, samplerCreateInfo);

  sampler->Initialize();

  return sampler;
}

SamplerImpl::SamplerImpl(Device& device, const vk::SamplerCreateInfo& samplerCreateInfo)
: mDevice(device),
  mCreateInfo(samplerCreateInfo)
{
}

SamplerImpl::~SamplerImpl() = default;

void SamplerImpl::Initialize()
{
  VkAssert(mDevice.GetLogicalDevice().createSampler(&mCreateInfo, &mDevice.GetAllocator("SAMPLER"), &mSampler));
}

vk::Sampler SamplerImpl::GetVkHandle() const
{
  return mSampler;
}

vk::SamplerCreateFlags SamplerImpl::GetCreateFlags() const
{
  return mCreateInfo.flags;
}

vk::Filter SamplerImpl::GetMinFilter() const
{
  return mCreateInfo.minFilter;
}

vk::Filter SamplerImpl::GetMagFilter() const
{
  return mCreateInfo.magFilter;
}

vk::SamplerMipmapMode SamplerImpl::GetMipMapMode() const
{
  return mCreateInfo.mipmapMode;
}

vk::SamplerAddressMode SamplerImpl::GetAddressModeU() const
{
  return mCreateInfo.addressModeU;
}

vk::SamplerAddressMode SamplerImpl::GetAddressModeV() const
{
  return mCreateInfo.addressModeV;
}

vk::SamplerAddressMode SamplerImpl::GetAddressModeW() const
{
  return mCreateInfo.addressModeW;
}

float SamplerImpl::GetMipLodBias() const
{
  return mCreateInfo.mipLodBias;
}

vk::Bool32 SamplerImpl::AnisotropyEnabled() const
{
  return mCreateInfo.anisotropyEnable;
}

float SamplerImpl::GetMaxAnisotropy() const
{
  return mCreateInfo.maxAnisotropy;
}

vk::Bool32 SamplerImpl::CompareEnabled() const
{
  return mCreateInfo.compareEnable;
}

vk::CompareOp SamplerImpl::GetCompareOperation() const
{
  return mCreateInfo.compareOp;
}

float SamplerImpl::GetMinLod() const
{
  return mCreateInfo.minLod;
}

float SamplerImpl::GetMaxLod() const
{
  return mCreateInfo.maxLod;
}

vk::BorderColor SamplerImpl::GetBorderColor() const
{
  return mCreateInfo.borderColor;
}

vk::Bool32 SamplerImpl::UsesUnnormalizedCoordinates() const
{
  return mCreateInfo.unnormalizedCoordinates;
}

} // namespace Dali::Graphics::Vulkan
