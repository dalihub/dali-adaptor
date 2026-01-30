/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include <dali/integration-api/debug.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-memory-impl.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>
#include <dali/internal/graphics/vulkan/vulkan-memory-allocation.h>
#include <algorithm>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali
{
namespace Graphics
{
namespace Vulkan
{
Image* Image::New(Device& graphicsDevice, const vk::ImageCreateInfo& createInfo, vk::MemoryPropertyFlags memoryProperties)
{
  auto image = new Image(graphicsDevice, createInfo, nullptr);
  image->Initialize(memoryProperties);
  return image;
}

Image::Image(Device& graphicsDevice, const vk::ImageCreateInfo& createInfo, vk::Image externalImage)
: mDevice(graphicsDevice),
  mCreateInfo(createInfo),
  mImage(externalImage),
  mImageLayout(mCreateInfo.initialLayout),
  mIsExternal(static_cast<bool>(externalImage))
{
  auto depthStencilFormats = std::vector<vk::Format>{
    vk::Format::eD32Sfloat,
    vk::Format::eD16Unorm,
    vk::Format::eD32SfloatS8Uint,
    vk::Format::eD24UnormS8Uint,
    vk::Format::eD16UnormS8Uint,
    vk::Format::eS8Uint,
  };

  auto hasDepth = std::find(depthStencilFormats.begin(), depthStencilFormats.end(), createInfo.format);

  if(hasDepth != depthStencilFormats.end())
  {
    auto format = *hasDepth;

    if(format == vk::Format::eD32Sfloat || format == vk::Format::eD16Unorm)
    {
      mAspectFlags = vk::ImageAspectFlagBits::eDepth;
    }
    else if(format == vk::Format::eS8Uint)
    {
      mAspectFlags = vk::ImageAspectFlagBits::eStencil;
    }
    else
    {
      mAspectFlags = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
    }
  }
  else
  {
    mAspectFlags = vk::ImageAspectFlagBits::eColor;
  }
}

Image::~Image()
{
  Destroy();
}

void Image::Initialize(vk::MemoryPropertyFlags memoryProperties)
{
  ::vma::Allocator* vmaAllocator = mDevice.GetVulkanMemoryAllocator();

  if(vmaAllocator)
  {
    auto vmaAllocInfo = ::vma::AllocationCreateInfo{}
                          .setPreferredFlags(memoryProperties)
                          .setUsage(::vma::MemoryUsage::eAuto);

    // If the image is an attachment, prefer dedicated memory
    constexpr vk::ImageUsageFlags attachmentOnlyFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eDepthStencilAttachment;
    if(mCreateInfo.usage & attachmentOnlyFlags || mCreateInfo.tiling == vk::ImageTiling::eOptimal)
    {
      vmaAllocInfo.setFlags(::vma::AllocationCreateFlagBits::eDedicatedMemory);
    }
    else
    {
      vmaAllocInfo.setFlags(::vma::AllocationCreateFlagBits::eHostAccessSequentialWrite);
    }

    mVmaAllocation = std::make_unique<::vma::Allocation>();
    // This creates the image, allocates appropriate memory for it, and binds the buffer with the memory.
    ::vma::AllocationInfo allocationInfo;
    VkAssert(vmaAllocator->createImage(&mCreateInfo, &vmaAllocInfo, &mImage, mVmaAllocation.get(), &allocationInfo));

    // Wrap the allocated memory so that we can map and unmap it.
    mMemory = std::make_unique<MemoryImpl>(mDevice, memoryProperties, mVmaAllocation.get());
  }
}

void Image::Destroy()
{
  DALI_LOG_INFO(gVulkanFilter, Debug::General, "Destroying image: %p\n", static_cast<VkImage>(mImage));
  if(mImage && !mIsExternal)
  {
    auto vmaAllocator = mDevice.GetVulkanMemoryAllocator();
    if(vmaAllocator && mVmaAllocation)
    {
      // This destroys the image and frees the allocated memory.
      vmaAllocator->destroyImage(mImage, *mVmaAllocation);
      mVmaAllocation.reset(nullptr);
    }
  }

  mImage = nullptr;
  mMemory.reset();
}

vk::Image Image::GetVkHandle() const
{
  return mImage;
}

vk::ImageLayout Image::GetImageLayout() const
{
  return mImageLayout;
}

void Image::SetImageLayout(vk::ImageLayout imageLayout)
{
  mImageLayout = imageLayout;
}

vk::ImageMemoryBarrier Image::CreateMemoryBarrier(vk::ImageLayout newLayout) const
{
  return CreateMemoryBarrier(mImageLayout, newLayout);
}

vk::ImageMemoryBarrier Image::CreateMemoryBarrier(vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const
{
  auto subResourceRange = vk::ImageSubresourceRange{}
                            .setBaseMipLevel(0)
                            .setLevelCount(1)
                            .setBaseArrayLayer(0)
                            .setLayerCount(1) // @todo is this right? Probably not.
                            .setAspectMask(GetAspectFlags());

  return CreateMemoryBarrier(oldLayout, newLayout, subResourceRange);
}

vk::ImageMemoryBarrier Image::CreateMemoryBarrier(vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageSubresourceRange subResourceRange) const
{
  // This function assumes that all images have 1 mip level and 1 layer
  // Should expand to handle any level/layer
  auto barrier = vk::ImageMemoryBarrier{}
                   .setOldLayout(oldLayout)
                   .setNewLayout(newLayout)
                   .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                   .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                   .setImage(GetVkHandle())
                   .setSubresourceRange(subResourceRange);

  // The srcAccessMask of the image memory barrier shows which operation
  // must be completed using the old layout, before the transition to the
  // new one happens.
  switch(oldLayout)
  {
    case vk::ImageLayout::eUndefined:
      barrier.srcAccessMask = vk::AccessFlags{};
      break;
    case vk::ImageLayout::ePreinitialized:
      barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
      break;
    case vk::ImageLayout::eColorAttachmentOptimal:
      barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
      break;
    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
      barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
      break;
    case vk::ImageLayout::eTransferSrcOptimal:
      barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
      break;
    case vk::ImageLayout::eTransferDstOptimal:
      barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
      break;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
      barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
      break;
    default:
      assert(false && "Image layout transition failed: Initial layout not supported.");
  }

  // Destination access mask controls the dependency for the new image layout
  switch(newLayout)
  {
    case vk::ImageLayout::eTransferDstOptimal:
      barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
      break;
    case vk::ImageLayout::eTransferSrcOptimal:
      barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
      break;
    case vk::ImageLayout::eColorAttachmentOptimal:
      barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
      break;
    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
      barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
      break;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
      if(barrier.srcAccessMask == vk::AccessFlags{})
      {
        barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
      }

      barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
      break;
    default:
      assert(false && "Image layout transition failed: Target layout not supported.");
  }

  return barrier;
}

uint32_t Image::GetWidth() const
{
  return mCreateInfo.extent.width;
}

uint32_t Image::GetHeight() const
{
  return mCreateInfo.extent.height;
}

uint32_t Image::GetLayerCount() const
{
  return mCreateInfo.arrayLayers;
}

uint32_t Image::GetMipLevelCount() const
{
  return mCreateInfo.mipLevels;
}

vk::Format Image::GetFormat() const
{
  return mCreateInfo.format;
}

vk::ImageType Image::GetImageType() const
{
  return mCreateInfo.imageType;
}

vk::ImageTiling Image::GetImageTiling() const
{
  return mCreateInfo.tiling;
}

vk::ImageAspectFlags Image::GetAspectFlags() const
{
  return mAspectFlags;
}

vk::ImageUsageFlags Image::GetUsageFlags() const
{
  return mCreateInfo.usage;
}

vk::SampleCountFlagBits Image::GetSampleCount() const
{
  return mCreateInfo.samples;
}

} // namespace Vulkan

} // namespace Graphics

} // namespace Dali
