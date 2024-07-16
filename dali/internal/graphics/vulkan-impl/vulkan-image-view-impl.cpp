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

#include <dali/internal/graphics/vulkan-impl/vulkan-image-view-impl.h>

#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/integration-api/debug.h>

#include <utility>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{

ImageView::ImageView( Device& graphicsDevice, const Image* image, vk::ImageViewCreateInfo createInfo )
: mGraphicsDevice( &graphicsDevice ),
  mImage( image ),
  mCreateInfo( std::move( createInfo ) ),
  mImageView( nullptr )
{
}

ImageView::~ImageView() = default;

vk::ImageView ImageView::GetVkHandle() const
{
  return mImageView;
}

const Image* ImageView::GetImage() const
{
  return mImage;
}

uint32_t ImageView::GetLayerCount() const
{
  return mImage->GetLayerCount();
}

uint32_t ImageView::GetMipLevelCount() const
{
  return mImage->GetMipLevelCount();
}

vk::ImageAspectFlags ImageView::GetImageAspectMask() const
{
  return vk::ImageAspectFlags();
}

const ImageView& ImageView::ConstRef()
{
  return *this;
}

ImageView& ImageView::Ref()
{
  return *this;
}

bool ImageView::OnDestroy()
{
  auto device = mGraphicsDevice->GetLogicalDevice();
  auto imageView = mImageView;
  auto allocator = &mGraphicsDevice->GetAllocator();

  mGraphicsDevice->DiscardResource( [ device, imageView, allocator ]() {
    DALI_LOG_INFO( gVulkanFilter, Debug::General, "Invoking deleter function: image view->%p\n",
                   static_cast< VkImageView >(imageView) )
    device.destroyImageView( imageView, allocator );
  } );

  return VkManaged::OnDestroy();
}

} // namespace Vulkan
} // namespace Graphics
} // namespace Dali
