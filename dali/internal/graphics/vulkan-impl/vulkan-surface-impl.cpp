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

// CLASS HEADER
#include <dali/internal/graphics/vulkan-impl/vulkan-surface-impl.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/integration-api/debug.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{

SurfaceImpl::SurfaceImpl( Device& device, vk::SurfaceKHR surfaceKhr)
: mGraphicsDevice( &device ),
  mSurface(surfaceKhr)
{
}

SurfaceImpl::~SurfaceImpl() = default;

vk::SurfaceKHR SurfaceImpl::GetVkHandle() const
{
  return mSurface;
}

const vk::SurfaceCapabilitiesKHR& SurfaceImpl::GetCapabilities() const
{
  return mCapabilities;
}

vk::SurfaceCapabilitiesKHR& SurfaceImpl::GetCapabilities()
{
  return mCapabilities;
}

void SurfaceImpl::UpdateSize( unsigned int width, unsigned int height )
{
  mCapabilities.currentExtent.width = width;
  mCapabilities.currentExtent.height = height;
}

bool SurfaceImpl::OnDestroy()
{
  if( mSurface )
  {
    auto instance = mGraphicsDevice->GetInstance();
    auto surface = mSurface;
    auto allocator = &mGraphicsDevice->GetAllocator();

    mGraphicsDevice->DiscardResource( [ instance, surface, allocator ]() {
      DALI_LOG_INFO( gVulkanFilter, Debug::General, "Invoking deleter function: surface->%p\n",
                     static_cast< VkSurfaceKHR >( surface ) )
      instance.destroySurfaceKHR( surface, allocator );
    } );

    mSurface = nullptr;
  }
  return false;
}

} // namespace Dali::Graphics::Vulkan


