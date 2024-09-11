#ifndef DALI_GRAPHICS_VULKAN_FENCE_IMPL_H
#define DALI_GRAPHICS_VULKAN_FENCE_IMPL_H

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

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

// EXTERNAL INCLUDES
#include <limits>

namespace Dali::Graphics::Vulkan
{
class Device;

/**
 * Synchronization primitive
 */
class FenceImpl
{
public:
  static FenceImpl* New(Device& graphicsDevice, const vk::FenceCreateInfo& fenceCreateInfo);

  FenceImpl(Device& graphicsDevice);

  void Initialize(const vk::FenceCreateInfo& fenceCreateInfo);

  ~FenceImpl();

  vk::Fence GetVkHandle() const;

  void Reset();

  void Wait(uint32_t timeout = std::numeric_limits<uint32_t>::max());

  vk::Result GetStatus();

private:
  void Destroy();

  Device*   mGraphicsDevice;
  vk::Fence mFence;
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_FENCE_IMPL_H
