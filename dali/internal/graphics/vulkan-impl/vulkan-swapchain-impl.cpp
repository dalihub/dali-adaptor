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
#include <dali/internal/graphics/vulkan-impl/vulkan-fence-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-surface-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-swapchain-impl.h>
#include <dali/internal/graphics/vulkan/vulkan-device.h>

#if 0
#include <dali/internal/graphics/vulkan-impl/vulkan-debug.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-view.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-queue.h>
#endif

#include <dali/integration-api/debug.h>

#if defined(DEBUG_ENABLED)
extern Debug::Filter* gVulkanFilter;
#endif

namespace Dali::Graphics::Vulkan
{
namespace
{
const auto MAX_SWAPCHAIN_RESOURCE_BUFFERS = 2u;
}

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

  Fence* endOfFrameFence;

  Device& graphicsDevice;
};

SwapchainBuffer::SwapchainBuffer(Device& graphicsDevice_)
: graphicsDevice(graphicsDevice_)
{
  acquireNextImageSemaphore = graphicsDevice.GetLogicalDevice().createSemaphore({}, graphicsDevice.GetAllocator()).value;
  submitSemaphore           = graphicsDevice.GetLogicalDevice().createSemaphore({}, graphicsDevice.GetAllocator()).value;

  endOfFrameFence = graphicsDevice.CreateFence({});
}

SwapchainBuffer::~SwapchainBuffer()
{
  // swapchain dies so make sure semaphore are not in use anymore
  auto result = graphicsDevice.GetLogicalDevice().waitIdle();
  assert(result == vk::Result::eSuccess);
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
  uint32_t           bufferCount)
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

Swapchain::~Swapchain() = default;

void Swapchain::CreateVkSwapchain(
  vk::SwapchainKHR   oldSwapchain,
  SurfaceImpl*       surface,
  vk::Format         requestedFormat,
  vk::PresentModeKHR presentMode,
  uint32_t           bufferCount)
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
  if(surfaceCapabilities.minImageCount > 0 &&
     bufferCount > surfaceCapabilities.minImageCount)
  {
    bufferCount = surfaceCapabilities.minImageCount;
  }

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

void Swapchain::CreateFramebuffers()
{
  assert(mSwapchainKHR && "Needs a swapchain before creating framebuffers");

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

  auto clearColor = vk::ClearColorValue{}.setFloat32({1.0f, 0.0f, 1.0f, 1.0f});

  //
  // CREATE FRAMEBUFFERS
  //
  for(auto&& image : images)
  {
    auto colorImage = mGraphicsDevice.CreateImageFromExternal(image,
                                                              mSwapchainCreateInfoKHR.imageFormat,
                                                              mSwapchainCreateInfoKHR.imageExtent);

    auto colorImageView = mGraphicsDevice.CreateImageView(colorImage);

    // A new color attachment for each framebuffer
    auto colorAttachment = FramebufferAttachment::NewColorAttachment(colorImageView,
                                                                     clearColor,
                                                                     true); // presentable

    mFramebuffers.push_back(FramebufferImpl::New(mGraphicsDevice,
                                                 nullptr,
                                                 {colorAttachment},
                                                 nullptr,
                                                 mSwapchainCreateInfoKHR.imageExtent.width,
                                                 mSwapchainCreateInfoKHR.imageExtent.height));
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
  return mFramebuffers[index];
}

FramebufferImpl* Swapchain::AcquireNextFramebuffer(bool shouldCollectGarbageNow)
{
  // prevent from using invalid swapchain
  if(!mIsValid)
  {
    DALI_LOG_INFO(gVulkanFilter, Debug::General, "Attempt to present invalid/expired swapchain: %p\n", static_cast<VkSwapchainKHR>(mSwapchainKHR));
    return nullptr;
  }

  const auto& device = mGraphicsDevice.GetLogicalDevice();

  // on swapchain first create sync primitives if not created yet
  if(mSwapchainBuffers.empty())
  {
    mSwapchainBuffers.resize(MAX_SWAPCHAIN_RESOURCE_BUFFERS);
    for(auto& buffer : mSwapchainBuffers)
    {
      buffer.reset(new SwapchainBuffer(mGraphicsDevice));
    }
  }

  DALI_LOG_INFO(gVulkanFilter, Debug::General, "Swapchain Image Index ( BEFORE Acquire ) = %d", int(mSwapchainImageIndex));

  auto& swapchainBuffer = mSwapchainBuffers[mGraphicsDevice.GetCurrentBufferIndex()];
  auto  result          = device.acquireNextImageKHR(mSwapchainKHR,
                                           std::numeric_limits<uint64_t>::max(),
                                           swapchainBuffer->acquireNextImageSemaphore,
                                           nullptr,
                                           &mSwapchainImageIndex);

  DALI_LOG_INFO(gVulkanFilter, Debug::General, "Swapchain Image Index ( AFTER Acquire ) = %d", int(mSwapchainImageIndex));

  // swapchain either not optimal or expired, returning nullptr and
  // setting validity to false
  if(result != vk::Result::eSuccess)
  {
    mIsValid = false;
    if(result == vk::Result::eErrorOutOfDateKHR)
    {
      return nullptr;
    }
    else
    {
      assert(mIsValid);
    }
  }

  // First frames don't need waiting as they haven't been submitted
  // yet. Note, that waiting on the fence without resetting it may
  // cause a stall ( nvidia, ubuntu )
  if(mFrameCounter >= mSwapchainBuffers.size())
  {
    vk::Result result = swapchainBuffer->endOfFrameFence->GetStatus();
    if(result == vk::Result::eNotReady)
    {
      swapchainBuffer->endOfFrameFence->Wait();
      swapchainBuffer->endOfFrameFence->Reset();
    }
  }
  else
  {
    mGraphicsDevice.DeviceWaitIdle();
  }
  // mGraphicsDevice.CollectGarbage();

  return mFramebuffers[mSwapchainImageIndex];
}

void Swapchain::Submit(CommandBufferImpl* commandBuffer)
{
  auto& swapchainBuffer = mSwapchainBuffers[mGraphicsDevice.GetCurrentBufferIndex()];

  swapchainBuffer->endOfFrameFence->Reset();

  mGraphicsDevice.Submit(*mQueue,
                         {Vulkan::SubmissionData{
                           {swapchainBuffer->acquireNextImageSemaphore},
                           {},
                           {commandBuffer},
                           {swapchainBuffer->submitSemaphore}}},
                         nullptr /*swapchainBuffer->inFlightFence*/); // This fence could guard resetting the cmd buffer
}

void Swapchain::Present()
{
  // prevent from using invalid swapchain
  if(!mIsValid)
  {
    return;
  }

  auto&              swapchainBuffer = mSwapchainBuffers[mGraphicsDevice.GetCurrentBufferIndex()];
  vk::PresentInfoKHR presentInfo{};
  vk::Result         result;
  presentInfo.setPImageIndices(&mSwapchainImageIndex)
    .setPResults(&result)
    .setPSwapchains(&mSwapchainKHR)
    .setSwapchainCount(1)
    .setPWaitSemaphores(&swapchainBuffer->submitSemaphore)
    .setWaitSemaphoreCount(1);

  mGraphicsDevice.Present(*mQueue, presentInfo);

  // handle error
  if(presentInfo.pResults[0] != vk::Result::eSuccess)
  {
    // invalidate swapchain
    if(result == vk::Result::eErrorOutOfDateKHR)
    {
      mIsValid = false;
    }
    else
    {
      mIsValid = false;
      assert(mIsValid);
    }
  }

  mFrameCounter++;
}

bool Swapchain::OnDestroy()
{
  if(mSwapchainKHR)
  {
    auto device    = mGraphicsDevice.GetLogicalDevice();
    auto swapchain = mSwapchainKHR;
    auto allocator = &mGraphicsDevice.GetAllocator();

    mGraphicsDevice.DiscardResource([device, swapchain, allocator]()
                                    {
      DALI_LOG_INFO(gVulkanFilter, Debug::General, "Invoking deleter function: swap chain->%p\n", static_cast<VkSwapchainKHR>(swapchain))
      device.destroySwapchainKHR(swapchain, allocator); });

    mSwapchainKHR = nullptr;
  }
  return false;
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
#if 0
  RefCountedFramebufferAttachment depthAttachment;
  auto swapchainExtent = mSwapchainCreateInfoKHR.imageExtent;

  if( depthStencilFormat != vk::Format::eUndefined )
  {
    // Create depth/stencil image
    auto imageCreateInfo = vk::ImageCreateInfo{}
      .setFormat( depthStencilFormat )
      .setMipLevels( 1 )
      .setTiling( vk::ImageTiling::eOptimal )
      .setImageType( vk::ImageType::e2D )
      .setArrayLayers( 1 )
      .setExtent( { swapchainExtent.width, swapchainExtent.height, 1 } )
      .setUsage( vk::ImageUsageFlagBits::eDepthStencilAttachment )
      .setSharingMode( vk::SharingMode::eExclusive )
      .setInitialLayout( vk::ImageLayout::eUndefined )
      .setSamples( vk::SampleCountFlagBits::e1 );

    auto dsRefCountedImage = mGraphicsDevice->CreateImage( imageCreateInfo );

    auto memory = mGraphicsDevice->AllocateMemory( dsRefCountedImage, vk::MemoryPropertyFlagBits::eDeviceLocal );

    mGraphics->BindImageMemory( dsRefCountedImage, std::move(memory), 0 );

    // create the depth stencil ImageView to be used within framebuffer
    auto depthStencilImageView = mGraphics->CreateImageView( dsRefCountedImage );
    auto depthClearValue = vk::ClearDepthStencilValue{}.setDepth( 0.0 )
                                                       .setStencil( STENCIL_DEFAULT_CLEAR_VALUE );

    // A single depth attachment for the swapchain.
    depthAttachment = FramebufferAttachment::NewDepthAttachment( depthStencilImageView, depthClearValue );
  }

  // Get images
  auto images = VkAssert( mGraphics->GetDevice().getSwapchainImagesKHR( mSwapchainKHR ) );

  // allocate framebuffers
  auto framebuffers = std::vector< RefCountedFramebuffer >{};
  framebuffers.reserve( images.size() );

  auto clearColor = vk::ClearColorValue{}.setFloat32( { 0.0f, 0.0f, 0.0f, 1.0f } );

  for( auto&& image : images )
  {

    auto colorImageView = mGraphics->CreateImageView( mGraphics->CreateImageFromExternal( image, mSwapchainCreateInfoKHR.imageFormat, swapchainExtent ) );

    // A new color attachment for each framebuffer
    auto colorAttachment = FramebufferAttachment::NewColorAttachment( colorImageView,
                                                                      clearColor,
                                                                      true );//presentable

    framebuffers.push_back( mGraphics->CreateFramebuffer( { colorAttachment },
                                               depthAttachment,
                                               swapchainExtent.width,
                                               swapchainExtent.height ) );
  }

  // Before replacing framebuffers in the swapchain, wait until all is done
  mGraphics->DeviceWaitIdle();

  mFramebuffers = std::move( framebuffers );
#endif
}

uint32_t Swapchain::GetImageCount() const
{
  return uint32_t(mFramebuffers.size());
}

} // namespace Dali::Graphics::Vulkan
