#pragma once

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

#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/internal/graphics/vulkan/vulkan-hpp-wrapper.h>

namespace Dali::Graphics::Vulkan
{
class Device;

/**
 * Wrapper for vk::Sampler
 */
class SamplerImpl
{
public:
  static SamplerImpl* New(Device& device, const vk::SamplerCreateInfo& createInfo);
  SamplerImpl(Device& device, const vk::SamplerCreateInfo& createInfo);

  ~SamplerImpl();

  void Initialize();

  /**
   * Returns VkSampler object
   * @return
   */
  vk::Sampler GetVkHandle() const;

  vk::SamplerCreateFlags GetCreateFlags() const;

  vk::Filter GetMinFilter() const;

  vk::Filter GetMagFilter() const;

  vk::SamplerMipmapMode GetMipMapMode() const;

  vk::SamplerAddressMode GetAddressModeU() const;

  vk::SamplerAddressMode GetAddressModeV() const;

  vk::SamplerAddressMode GetAddressModeW() const;

  float GetMipLodBias() const;

  vk::Bool32 AnisotropyEnabled() const;

  float GetMaxAnisotropy() const;

  vk::Bool32 CompareEnabled() const;

  vk::CompareOp GetCompareOperation() const;

  float GetMinLod() const;

  float GetMaxLod() const;

  vk::BorderColor GetBorderColor() const;

  vk::Bool32 UsesUnnormalizedCoordinates() const;

private:
  Device&               mDevice;
  vk::SamplerCreateInfo mCreateInfo;
  vk::Sampler           mSampler;
};

} // namespace Dali::Graphics::Vulkan
