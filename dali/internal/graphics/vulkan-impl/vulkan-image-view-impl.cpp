/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

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
ImageView* ImageView::NewFromImage(Device& device, const Image& image)
{
  vk::ComponentMapping componentsMapping = {vk::ComponentSwizzle::eR,
                                            vk::ComponentSwizzle::eG,
                                            vk::ComponentSwizzle::eB,
                                            vk::ComponentSwizzle::eA};

  return NewFromImage(device, image, componentsMapping);
}

ImageView* ImageView::NewFromImage(
  Device&                     device,
  const Image&                image,
  const vk::ComponentMapping& componentMapping)
{
  auto subresourceRange = vk::ImageSubresourceRange{}
                            .setAspectMask(image.GetAspectFlags())
                            .setBaseArrayLayer(0)
                            .setBaseMipLevel(0)
                            .setLevelCount(image.GetMipLevelCount())
                            .setLayerCount(image.GetLayerCount());

  vk::ImageViewType viewType = vk::ImageViewType::e2D;
  if(image.GetCreateInfo().flags & vk::ImageCreateFlagBits::eCubeCompatible)
  {
    viewType = vk::ImageViewType::eCube;
  }
  auto imageView = New(device,
                       image,
                       {},
                       viewType,
                       image.GetFormat(),
                       componentMapping,
                       subresourceRange,
                       nullptr);

  return imageView;
}

ImageView* ImageView::New(Device&                         device,
                          const Image&                    image,
                          const vk::ImageViewCreateFlags& flags,
                          vk::ImageViewType               viewType,
                          vk::Format                      format,
                          vk::ComponentMapping            components,
                          vk::ImageSubresourceRange       subresourceRange,
                          void*                           pNext)
{
  auto imageViewCreateInfo = vk::ImageViewCreateInfo{}
                               .setPNext(pNext)
                               .setFlags(flags)
                               .setImage(image.GetVkHandle())
                               .setViewType(viewType)
                               .setFormat(format)
                               .setComponents(components)
                               .setSubresourceRange(std::move(subresourceRange));

  return New(device, image, imageViewCreateInfo);
}

ImageView* ImageView::New(Device& device, const Image& image, const vk::ImageViewCreateInfo& createInfo)
{
  auto imageView = new ImageView(device, image, createInfo);

  VkAssert(device.GetLogicalDevice().createImageView(&createInfo, &device.GetAllocator("IMAGEVIEW"), &imageView->mImageView));

  return imageView;
}

ImageView::ImageView(Device& graphicsDevice, const Image& image, vk::ImageViewCreateInfo createInfo)
: mDevice(graphicsDevice),
  mImage(image),
  mCreateInfo(std::move(createInfo)),
  mImageView(nullptr)
{
}

ImageView::~ImageView()
{
  Destroy();
}

void ImageView::Destroy()
{
  DALI_LOG_INFO(gVulkanFilter, Debug::General, "Destroying ImageView: %p\n", static_cast<VkImageView>(mImageView));
  auto device = mDevice.GetLogicalDevice();
  device.destroyImageView(mImageView, mDevice.GetAllocator());
}

vk::ImageView ImageView::GetVkHandle() const
{
  return mImageView;
}

const Image* ImageView::GetImage() const
{
  return &mImage;
}

uint32_t ImageView::GetLayerCount() const
{
  return mImage.GetLayerCount();
}

uint32_t ImageView::GetMipLevelCount() const
{
  return mImage.GetMipLevelCount();
}

vk::ImageAspectFlags ImageView::GetImageAspectMask() const
{
  return vk::ImageAspectFlags();
}

} // namespace Vulkan
} // namespace Graphics
} // namespace Dali
