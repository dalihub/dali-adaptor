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

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-fence-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-view-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-surface-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-swapchain-impl.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

#include <dali/integration-api/debug.h>
#include <dali/public-api/common/dali-common.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{
/**
 * SwapchainBuffer stores all per-buffer data
 */
struct SwapchainBuffer
{
  SwapchainBuffer(Device& _graphicsDevice);

  ~SwapchainBuffer();

  /**
   * Semaphore signalled on acquire next image
   */
  vk::Semaphore acquireNextImageSemaphore;

  /**
   * Semaphore signalled on complete commands submission
   */
  vk::Semaphore submitSemaphore;

  std::unique_ptr<FenceImpl> endOfFrameFence;

  Device& graphicsDevice;
  bool    submitted{false};
};

SwapchainBuffer::SwapchainBuffer(Device& graphicsDevice_)
: graphicsDevice(graphicsDevice_)
{
  acquireNextImageSemaphore = graphicsDevice.GetLogicalDevice().createSemaphore({}, graphicsDevice.GetAllocator()).value;
  submitSemaphore           = graphicsDevice.GetLogicalDevice().createSemaphore({}, graphicsDevice.GetAllocator()).value;

  endOfFrameFence = FenceImpl::New(graphicsDevice, {});
}

SwapchainBuffer::~SwapchainBuffer()
{
  // swapchain dies so make sure semaphore are not in use anymore
  auto result = graphicsDevice.GetLogicalDevice().waitIdle();
  VkTest(result, vk::Result::eSuccess);
  graphicsDevice.GetLogicalDevice().destroySemaphore(acquireNextImageSemaphore, graphicsDevice.GetAllocator());
  graphicsDevice.GetLogicalDevice().destroySemaphore(submitSemaphore, graphicsDevice.GetAllocator());
}

Swapchain* Swapchain::NewSwapchain(
  Device&            device,
  Queue&             presentationQueue,
  vk::SwapchainKHR   oldSwapchain,
  SurfaceImpl*       surface,
  vk::Format         requestedFormat,
  vk::PresentModeKHR presentMode,
  uint32_t&          bufferCount)
{
  auto swapchain = new Swapchain(device, presentationQueue);
  swapchain->CreateVkSwapchain(oldSwapchain, surface, requestedFormat, presentMode, bufferCount);
  return swapchain;
}

Swapchain::Swapchain(Device& graphicsDevice, Queue& presentationQueue)
: mGraphicsDevice(graphicsDevice),
  mQueue(&presentationQueue),
  mSwapchainKHR(nullptr),
  mIsValid(false)
{
}

Swapchain::~Swapchain()
{
}

void Swapchain::CreateVkSwapchain(
  vk::SwapchainKHR   oldSwapchain,
  SurfaceImpl*       surface,
  vk::Format         requestedFormat,
  vk::PresentModeKHR presentMode,
  uint32_t&          bufferCount)
{
  mSurface = surface;
  vk::Format        swapchainImageFormat{};
  vk::ColorSpaceKHR swapchainColorSpace{};
  surface->GetSupportedFormats(requestedFormat, swapchainImageFormat, swapchainColorSpace);

  // Get the surface capabilities to determine some settings of the swap chain
  auto surfaceCapabilities = surface->GetCapabilities();

  // Determine the swap chain extent
  auto swapchainExtent = surfaceCapabilities.currentExtent;

  // Find a supported composite alpha format (not all devices support alpha opaque)
  auto compositeAlpha = vk::CompositeAlphaFlagBitsKHR{};

  // Simply select the first composite alpha format available
  auto compositeAlphaFlags = std::vector<vk::CompositeAlphaFlagBitsKHR>{
    vk::CompositeAlphaFlagBitsKHR::eOpaque,
    vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
    vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
    vk::CompositeAlphaFlagBitsKHR::eInherit};

  for(const auto& compositeAlphaFlag : compositeAlphaFlags)
  {
    if(surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag)
    {
      compositeAlpha = compositeAlphaFlag;
      break;
    }
  }

  // Determine the number of images
  if(surfaceCapabilities.minImageCount > 0)
  {
    bufferCount = surfaceCapabilities.minImageCount;
  }
  mBufferCount = bufferCount;

  // Find the transformation of the surface
  vk::SurfaceTransformFlagBitsKHR preTransform;
  if(surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
  {
    // We prefer a non-rotated transform
    preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
  }
  else
  {
    preTransform = surfaceCapabilities.currentTransform;
  }

  auto presentModes = surface->GetSurfacePresentModes();
  auto found        = std::find_if(presentModes.begin(),
                                   presentModes.end(),
                                   [&](vk::PresentModeKHR mode)
         {
    return presentMode == mode;
  });

  if(found == presentModes.end())
  {
    // Requested present mode not supported. Default to FIFO. FIFO is always supported as per spec.
    presentMode = vk::PresentModeKHR::eFifo;
  }

  // Creation settings have been determined. Fill in the create info struct.
  mSwapchainCreateInfoKHR
    .setSurface(surface->GetVkHandle())
    .setPreTransform(preTransform)
    .setPresentMode(presentMode)
    .setOldSwapchain(oldSwapchain)
    .setMinImageCount(bufferCount)
    .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
    .setImageSharingMode(vk::SharingMode::eExclusive)
    .setImageArrayLayers(1)
    .setImageColorSpace(swapchainColorSpace)
    .setImageFormat(swapchainImageFormat)
    .setImageExtent(swapchainExtent)
    .setCompositeAlpha(compositeAlpha)
    .setClipped(static_cast<vk::Bool32>(true))
    .setQueueFamilyIndexCount(0)
    .setPQueueFamilyIndices(nullptr);

  // Create the swap chain
  mSwapchainKHR = VkAssert(mGraphicsDevice.GetLogicalDevice().createSwapchainKHR(mSwapchainCreateInfoKHR, mGraphicsDevice.GetAllocator()));
}

void Swapchain::Destroy()
{
  if(mDepthStencilBuffer)
  {
    mDepthStencilBuffer->Destroy();
  }

  mFramebuffers.clear();
  mSwapchainBuffers.clear();
  mSwapchainImages.clear();
  mImageViews.clear();

  DALI_LOG_INFO(gVulkanFilter, Debug::General, "Destroying SwapChain: %p\n", static_cast<VkSwapchainKHR>(mSwapchainKHR));

  if(mSwapchainKHR)
  {
    auto device    = mGraphicsDevice.GetLogicalDevice();
    auto allocator = &mGraphicsDevice.GetAllocator();

    device.destroySwapchainKHR(mSwapchainKHR, allocator);
    mSwapchainKHR = nullptr;
  }
}

void Swapchain::CreateFramebuffers(FramebufferAttachmentHandle depthAttachment)
{
  DALI_ASSERT_ALWAYS(mSwapchainKHR && "Needs a swapchain before creating framebuffers");

  // pull images and create Framebuffers
  auto images              = VkAssert(mGraphicsDevice.GetLogicalDevice().getSwapchainImagesKHR(mSwapchainKHR));
  auto surfaceCapabilities = mSurface->GetCapabilities();

  // number of images must match requested buffering mode
  if(images.size() < surfaceCapabilities.minImageCount)
  {
    DALI_LOG_STREAM(gVulkanFilter,
                    Debug::General,
                    "Swapchain creation failed: Swapchain images are less than the requested amount");
    mGraphicsDevice.GetLogicalDevice().destroySwapchainKHR(mSwapchainKHR);
    mSwapchainKHR = nullptr;
    return;
  }

  mFramebuffers.clear();
  mFramebuffers.reserve(images.size());
  mSwapchainImages.clear();
  mSwapchainImages.reserve(images.size());

  auto clearColor = vk::ClearColorValue{}.setFloat32({1.0f, 0.0f, 1.0f, 1.0f});

  DALI_LOG_INFO(gVulkanFilter, Debug::General, "Num Images: %u\n", images.size());

  //
  // CREATE FRAMEBUFFERS
  //
  RenderPassHandle compatibleRenderPass{};
  for(auto&& image : images)
  {
    auto colorImage = mGraphicsDevice.CreateImageFromExternal(image,
                                                              mSwapchainCreateInfoKHR.imageFormat,
                                                              mSwapchainCreateInfoKHR.imageExtent);
    mSwapchainImages.emplace_back(colorImage);
    mImageViews.emplace_back(ImageView::NewFromImage(mGraphicsDevice, *colorImage));

    // A new color attachment for each framebuffer
    SharedAttachments attachments;
    attachments.emplace_back(FramebufferAttachment::NewColorAttachment(mImageViews.back().get(),
                                                                       clearColor,
                                                                       nullptr,
                                                                       true));

    std::unique_ptr<FramebufferImpl, void (*)(FramebufferImpl*)> framebuffer(
      FramebufferImpl::New(mGraphicsDevice,
                           compatibleRenderPass,
                           attachments,
                           depthAttachment,
                           mSwapchainCreateInfoKHR.imageExtent.width,
                           mSwapchainCreateInfoKHR.imageExtent.height),
      [](FramebufferImpl* framebuffer1)
    {
      framebuffer1->Destroy();
      delete framebuffer1;
    });
    mFramebuffers.push_back(std::move(framebuffer));

    if(!compatibleRenderPass)
    {
      // use common renderpass for all framebuffers.
      compatibleRenderPass = mFramebuffers.back()->GetRenderPass(0);
    }
  }
  mIsValid = true;
}

vk::SwapchainKHR Swapchain::GetVkHandle() const
{
  return mSwapchainKHR;
}

FramebufferImpl* Swapchain::GetCurrentFramebuffer() const
{
  return GetFramebuffer(mSwapchainImageIndex);
}

FramebufferImpl* Swapchain::GetFramebuffer(uint32_t index) const
{
  return mFramebuffers[index].get();
}

FramebufferImpl* Swapchain::AcquireNextFramebuffer(bool shouldCollectGarbageNow)
{
  // prevent from using invalid swapchain
  if(!mIsValid)
  {
    DALI_LOG_INFO(gVulkanFilter, Debug::General, "Attempt to acquire from invalid/expired swapchain: %p\n", static_cast<VkSwapchainKHR>(mSwapchainKHR));
    return nullptr;
  }

  const auto& device = mGraphicsDevice.GetLogicalDevice();

  // on swapchain first create sync primitives if not created yet
  if(mSwapchainBuffers.empty())
  {
    mSwapchainBuffers.resize(mBufferCount);
    for(auto& buffer : mSwapchainBuffers)
    {
      buffer.reset(new SwapchainBuffer(mGraphicsDevice));
    }
  }

  DALI_LOG_INFO(gVulkanFilter, Debug::General, "Swapchain Image Index ( BEFORE Acquire ) = %d", int(mSwapchainImageIndex));

  constexpr auto TIMEOUT = 1'000'000'000; //'

  auto& swapchainBuffer = mSwapchainBuffers[GetCurrentBufferIndex()];

  // First frames don't need waiting as they haven't been submitted
  // yet. Note, that waiting on the fence without resetting it may
  // cause a stall ( nvidia, ubuntu )
  if(mFrameCounter >= mSwapchainBuffers.size())
  {
    vk::Result status = swapchainBuffer->endOfFrameFence->GetStatus();
    if(status == vk::Result::eNotReady)
    {
      swapchainBuffer->endOfFrameFence->Wait();
      swapchainBuffer->endOfFrameFence->Reset();
    }
  }

  auto result = device.acquireNextImageKHR(mSwapchainKHR,
                                           TIMEOUT,
                                           swapchainBuffer->acquireNextImageSemaphore,
                                           nullptr,
                                           &mSwapchainImageIndex);

  DALI_LOG_INFO(gVulkanFilter, Debug::General, "Swapchain Image Index ( AFTER Acquire ) = %d", int(mSwapchainImageIndex));

  DALI_LOG_INFO(gVulkanFilter, Debug::General, "acquireNextImageKHR result %s\n", vk::to_string(result).c_str());

  // swapchain either not optimal or expired, returning nullptr and
  // setting validity to false
  if(result != vk::Result::eSuccess)
  {
    if(result == vk::Result::eErrorOutOfDateKHR)
    {
      mIsValid = false;
      return nullptr;
    }
    else if(result == vk::Result::eTimeout || result == vk::Result::eNotReady)
    {
      // TODO: this isn't error, swapchain may be still valid.
      // we may think of looping it or discarding old swapchain.
      // At this point, we ignore those return codes.
    }
    else // Only real error case
    {
      mIsValid = false;
      DALI_LOG_DEBUG_INFO("Swapchain::AcquireNextFramebuffer() failed with result %s\n", vk::to_string(result).c_str());
      DALI_ASSERT_ALWAYS(true && "AcquireNextImageKHR failed with error, cannot continue.");
    }
  }

  return mFramebuffers[mSwapchainImageIndex].get();
}

Queue* Swapchain::GetQueue()
{
  return mQueue;
}

FenceImpl* Swapchain::GetEndOfFrameFence()
{
  auto& swapchainBuffer = mSwapchainBuffers[GetCurrentBufferIndex()];
  return swapchainBuffer->endOfFrameFence.get();
}

void Swapchain::UpdateSubmissionData(std::vector<SubmissionData>& submissionData)
{
  auto& swapchainBuffer = mSwapchainBuffers[GetCurrentBufferIndex()];

  swapchainBuffer->endOfFrameFence->Reset();
  swapchainBuffer->submitted = true;

  submissionData.front().waitSemaphores.emplace_back(swapchainBuffer->acquireNextImageSemaphore);
  submissionData.front().waitDestinationStageMask.emplace_back(vk::PipelineStageFlagBits::eFragmentShader);
  submissionData.back().signalSemaphores.emplace_back(swapchainBuffer->submitSemaphore);
}

uint32_t Swapchain::GetCurrentBufferIndex() const
{
  return mSwapchainBuffers.empty() ? 0 : mFrameCounter % mBufferCount;
}

bool Swapchain::Present()
{
  bool presented = false;
  DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "Vulkan::Swapchain::Present() valid:%s SwapchainBuffer count:%u\n", mIsValid ? "True" : "False", mSwapchainBuffers.size());

  // prevent from using invalid swapchain
  if(!mIsValid || mSwapchainBuffers.empty())
  {
    return presented;
  }

  auto& swapchainBuffer = mSwapchainBuffers[GetCurrentBufferIndex()];

  DALI_LOG_INFO(gVulkanFilter, Debug::Verbose, "Vulkan::Swapchain::Present() work submitted:%s\n", swapchainBuffer->submitted ? "True" : "False");

  // Only present if we've submitted work
  if(swapchainBuffer->submitted)
  {
    vk::PresentInfoKHR presentInfo{};
    vk::Result         result;

    presentInfo.setPImageIndices(&mSwapchainImageIndex)
      .setPResults(&result)
      .setPSwapchains(&mSwapchainKHR)
      .setSwapchainCount(1)
      .setPWaitSemaphores(&swapchainBuffer->submitSemaphore)
      .setWaitSemaphoreCount(1);

    vk::Result presentResult = mGraphicsDevice.Present(*mQueue, presentInfo);
    presented                = true;

    // handle error
    if((presentResult != vk::Result::eSuccess || presentInfo.pResults[0] != vk::Result::eSuccess) && result != vk::Result::eSuboptimalKHR)
    {
      // invalidate swapchain
      if(result == vk::Result::eErrorOutOfDateKHR)
      {
        mIsValid = false;
      }
      else
      {
        mIsValid = false;
        DALI_LOG_DEBUG_INFO("Vulkan::Swapchain::Present() failed. presentResult:%s\n", vk::to_string(presentResult).c_str());
        DALI_ASSERT_ALWAYS(mIsValid && "Present failed. Swapchain invalidated");
      }
    }
  }
  swapchainBuffer->submitted = false;
  mFrameCounter++;
  return presented;
}

bool Swapchain::IsValid() const
{
  return mIsValid;
}

void Swapchain::Invalidate()
{
  mIsValid = false;
}

void Swapchain::SetDepthStencil(vk::Format depthStencilFormat)
{
  FramebufferAttachmentHandle depthAttachment;
  auto                        swapchainExtent = mSwapchainCreateInfoKHR.imageExtent;
  if(depthStencilFormat != mDepthStencilFormat)
  {
    if(depthStencilFormat != vk::Format::eUndefined)
    {
      mDepthStencilFormat = depthStencilFormat;
      // Create depth/stencil image
      auto imageCreateInfo = vk::ImageCreateInfo{}
                               .setFormat(depthStencilFormat)
                               .setMipLevels(1)
                               .setTiling(vk::ImageTiling::eOptimal)
                               .setImageType(vk::ImageType::e2D)
                               .setArrayLayers(1)
                               .setExtent({swapchainExtent.width, swapchainExtent.height, 1})
                               .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
                               .setSharingMode(vk::SharingMode::eExclusive)
                               .setInitialLayout(vk::ImageLayout::eUndefined)
                               .setSamples(vk::SampleCountFlagBits::e1);

      mDepthStencilBuffer.reset(Image::New(mGraphicsDevice, imageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal));

      // create the depth stencil ImageView to be used within framebuffer
      mDepthStencilImageView.reset(ImageView::NewFromImage(mGraphicsDevice, *mDepthStencilBuffer.get()));
      auto depthClearValue = vk::ClearDepthStencilValue{}.setDepth(0.0).setStencil(STENCIL_DEFAULT_CLEAR_VALUE);

      // A single depth attachment for the swapchain.
      depthAttachment = FramebufferAttachmentHandle(FramebufferAttachment::NewDepthAttachment(mDepthStencilImageView.get(), depthClearValue, nullptr));
    }

    // Before replacing framebuffers in the swapchain, wait until all is done
    mGraphicsDevice.DeviceWaitIdle();

    CreateFramebuffers(depthAttachment);
  }
}

} // namespace Dali::Graphics::Vulkan
