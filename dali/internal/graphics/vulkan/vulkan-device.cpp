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
#include <dali/internal/graphics/vulkan/vulkan-device.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan/vulkan-surface-factory.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-buffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-command-pool-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-fence-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-render-pass-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-view-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-surface-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-swapchain-impl.h>
#include <dali/integration-api/debug.h>

#ifndef VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#define VK_KHR_XLIB_SURFACE_EXTENSION_NAME "VK_KHR_xlib_surface"
#endif

#ifndef VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
#define VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME "VK_KHR_wayland_surface"
#endif

#ifndef VK_KHR_XCB_SURFACE_EXTENSION_NAME
#define VK_KHR_XCB_SURFACE_EXTENSION_NAME "VK_KHR_xcb_surface"
#endif

#include <iostream>
#include <utility>

#if defined(DEBUG_ENABLED)
Debug::Filter* gVulkanFilter = Debug::Filter::New(Debug::Concise, false, "LOG_VULKAN");
#endif

namespace Dali::Graphics::Vulkan
{
class RenderPassImpl;

auto reqLayers = std::vector< const char* >{
        //"VK_LAYER_LUNARG_screenshot",           // screenshot
        //"VK_LAYER_LUNARG_parameter_validation", // parameter
        //"VK_LAYER_LUNARG_vktrace",              // vktrace ( requires vktrace connection )
        //"VK_LAYER_LUNARG_monitor",             // monitor
        //"VK_LAYER_LUNARG_swapchain",           // swapchain
        //"VK_LAYER_GOOGLE_threading",           // threading
        //"VK_LAYER_LUNARG_api_dump",            // api
        //"VK_LAYER_LUNARG_object_tracker",      // objects
        //"VK_LAYER_LUNARG_core_validation",     // core
        //"VK_LAYER_GOOGLE_unique_objects",      // unique objects
        //"VK_LAYER_LUNARG_standard_validation", // standard
        // Don't add VK_LAYER_RENDERDOC_Capture, set ENABLE_VULKAN_RENDERDOC_CAPTURE=1 environment variable
};

Device::Device()
{
  mAllocator.reset(nullptr); // Don't provide any callbacks unless we're debugging.
}

Device::~Device()
{
  // Wait for everything to finish on the GPU
  DeviceWaitIdle();

  mSurfaceMap.clear();

  DALI_LOG_STREAM( gVulkanFilter, Debug::General, "DESTROYING GRAPHICS CONTEXT--------------------------------\n" );

  SwapBuffers();

  // We are done with all resources (technically... . If not we will get a ton of validation layer errors)
  // Kill the Vulkan logical device
  mLogicalDevice.destroy( mAllocator.get() );

  // Kill the Vulkan instance
  DestroyInstance();
}

// Create methods -----------------------------------------------------------------------------------------------
void Device::Create()
{
  auto extensions = PrepareDefaultInstanceExtensions();
  auto layers = vk::enumerateInstanceLayerProperties();

  std::vector< const char* > validationLayers;
  for( auto&& reqLayer : reqLayers )
  {
    for( auto&& prop : layers.value )
    {
      DALI_LOG_STREAM( gVulkanFilter, Debug::General, prop.layerName );
      if( std::string( prop.layerName ) == reqLayer )
      {
        validationLayers.push_back( reqLayer );
      }
    }
  }

  CreateInstance( extensions, validationLayers );
  PreparePhysicalDevice();
}

void Device::CreateDevice()
{
  auto queueInfos = GetQueueCreateInfos();
  {
    auto maxQueueCountPerFamily = 0u;
    for( auto&& info : queueInfos )
    {
      maxQueueCountPerFamily = std::max( info.queueCount, maxQueueCountPerFamily );
    }

    auto priorities = std::vector< float >( maxQueueCountPerFamily );
    std::fill( priorities.begin(), priorities.end(), 1.0f );

    for( auto& info : queueInfos )
    {
      info.setPQueuePriorities( priorities.data() );
    }

    std::vector< const char* > extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    /**
     * @todo Check these exist before using them for native image:
     * VK_KHR_SWAPCHAIN_EXTENSION_NAME
     * VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME
     * VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME
     * VK_KHR_BIND_MEMORY_2_EXTENSION_NAME
     * VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME
     * VK_KHR_MAINTENANCE1_EXTENSION_NAME
     * VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME
     * VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME
     */

    vk::PhysicalDeviceFeatures featuresToEnable{};

    if( mPhysicalDeviceFeatures.fillModeNonSolid )
    {
      featuresToEnable.fillModeNonSolid = VK_TRUE;
    }

    if( mPhysicalDeviceFeatures.textureCompressionASTC_LDR )
    {
      featuresToEnable.textureCompressionASTC_LDR = VK_TRUE;
    }

    if( mPhysicalDeviceFeatures.textureCompressionETC2 )
    {
      featuresToEnable.textureCompressionETC2 = VK_TRUE;
    }

    auto info = vk::DeviceCreateInfo{};
    info.setEnabledExtensionCount( U32( extensions.size() ) )
        .setPpEnabledExtensionNames( extensions.data() )
        .setPEnabledFeatures( &featuresToEnable  )
        .setPQueueCreateInfos( queueInfos.data() )
        .setQueueCreateInfoCount( U32( queueInfos.size() ) );

    mLogicalDevice = VkAssert( mPhysicalDevice.createDevice( info, *mAllocator ) );
  }

  // create Queue objects
  for( auto& queueInfo : queueInfos )
  {
    for( auto i = 0u; i < queueInfo.queueCount; ++i )
    {
      auto queue = mLogicalDevice.getQueue( queueInfo.queueFamilyIndex, i );

      // based on family push queue instance into right array
      auto flags = mQueueFamilyProperties[queueInfo.queueFamilyIndex].queueFlags;
      auto queueWrapper = std::unique_ptr< Queue >( new Queue(queue, queueInfo.queueFamilyIndex, i, flags ) );

      if( flags & vk::QueueFlagBits::eGraphics )
      {
        mGraphicsQueues.emplace_back( queueWrapper.get() );
      }
      if( flags & vk::QueueFlagBits::eTransfer )
      {
        mTransferQueues.emplace_back( queueWrapper.get() );
      }
      if( flags & vk::QueueFlagBits::eCompute )
      {
        mComputeQueues.emplace_back( queueWrapper.get() );
      }
      mAllQueues.emplace_back( std::move( queueWrapper ));
      // todo: present queue
    }
  }

  // if( !mVulkanPipelineCache )
  // {
  //   mVulkanPipelineCache = mLogicalDevice.createPipelineCache( vk::PipelineCacheCreateInfo{}, GetAllocator() ).value;
  // }
}

Graphics::SurfaceId Device::CreateSurface(
  Dali::Graphics::SurfaceFactory& surfaceFactory,
  const Dali::Graphics::GraphicsCreateInfo& createInfo )
{
  auto vulkanSurfaceFactory = dynamic_cast<Dali::Graphics::Vulkan::SurfaceFactory*>( &surfaceFactory );

  if( !vulkanSurfaceFactory )
  {
    return -1; // fail
  }

  // create surface from the factory
  auto* surface = new SurfaceImpl( *this, vulkanSurfaceFactory->Create( mInstance,
                                                                        mAllocator.get(),
                                                                        mPhysicalDevice ));
  if( !surface->GetVkHandle() )
  {
    return -1;
  }

  VkBool32 supported( VK_FALSE );
  for( auto i = 0u; i < mQueueFamilyProperties.size(); ++i )
  {
    auto result = mPhysicalDevice.getSurfaceSupportKHR( i, surface->GetVkHandle(), &supported );
    if( result == vk::Result::eSuccess && supported )
    {
      break;
    }
  }

  assert( supported && "There is no queue family supporting presentation!");

  surface->GetCapabilities() = VkAssert( mPhysicalDevice.getSurfaceCapabilitiesKHR( surface->GetVkHandle() ) );

  // If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
  if( surface->GetCapabilities().currentExtent.width == std::numeric_limits< uint32_t >::max() )
  {
    surface->GetCapabilities().currentExtent.width = std::max( surface->GetCapabilities().minImageExtent.width,
                                                               std::min( surface->GetCapabilities().maxImageExtent.width, createInfo.surfaceWidth ) );

    surface->GetCapabilities().currentExtent.height = std::max( surface->GetCapabilities().minImageExtent.height,
                                                                std::min( surface->GetCapabilities().maxImageExtent.height, createInfo.surfaceHeight ) );

  }

  mSurfaceResized = false;

  // map surface to SurfaceId
  auto surfaceId = ++mBaseSurfaceId;

  mSurfaceMap[ surfaceId ] = SwapchainSurfacePair{ nullptr, surface };

  if( createInfo.depthStencilMode == Dali::Graphics::DepthStencilMode::DEPTH_OPTIMAL ||
      createInfo.depthStencilMode == Dali::Graphics::DepthStencilMode::DEPTH_STENCIL_OPTIMAL )
  {
    mHasDepth = true;
  }
  else
  {
    mHasDepth = false;
  }

  if( createInfo.depthStencilMode == Dali::Graphics::DepthStencilMode::DEPTH_STENCIL_OPTIMAL )
  {
    mHasStencil = true;
  }

  return surfaceId;
}

void Device::DestroySurface( Dali::Graphics::SurfaceId surfaceId )
{
  if( auto surface = GetSurface( surfaceId ) )
  {
    DeviceWaitIdle();
    auto swapchain = GetSwapchainForSurfaceId( surfaceId );
    swapchain->Destroy();
    surface->Destroy();
  }
}

Swapchain* Device::CreateSwapchainForSurface( SurfaceImpl* surface )
{
  DALI_ASSERT_DEBUG(surface && "Surface ptr must be allocated");

  auto surfaceCapabilities = surface->GetCapabilities();

  //TODO: propagate the format and presentation mode to higher layers to allow for more control?
  // @todo - for instance, Graphics::RenderTargetCreateInfo?!
  Swapchain* swapchain = CreateSwapchain( surface,
                                          vk::Format::eB8G8R8A8Unorm,
                                          vk::PresentModeKHR::eFifo,
                                          surfaceCapabilities.minImageCount,
                                          nullptr );

  // store swapchain in the correct pair
  for( auto&& val : mSurfaceMap )
  {
    if( val.second.surface == surface )
    {
      val.second.swapchain = swapchain;
      break;
    }
  }

  return swapchain;
}


Swapchain* Device::ReplaceSwapchainForSurface( SurfaceImpl* surface, Swapchain*&& oldSwapchain )
{
  auto surfaceCapabilities = surface->GetCapabilities();

  mSurfaceResized = false;

  auto swapchain = CreateSwapchain( surface,
                                    vk::Format::eB8G8R8A8Unorm,
                                    vk::PresentModeKHR::eFifo,
                                    surfaceCapabilities.minImageCount,
                                    std::move(oldSwapchain) );

  // store swapchain in the correct pair
  for( auto&& val : mSurfaceMap )
  {
    if( val.second.surface == surface )
    {
      val.second.swapchain = swapchain;
      break;
    }
  }

  return swapchain;
}

Swapchain* Device::CreateSwapchain( SurfaceImpl* surface,
                                    vk::Format requestedFormat,
                                    vk::PresentModeKHR presentMode,
                                    uint32_t bufferCount,
                                    Swapchain*&& oldSwapchain )
{
  auto newSwapchain = Swapchain::NewSwapchain( *this, GetPresentQueue(),
                                              oldSwapchain?oldSwapchain->GetVkHandle():nullptr,
                                              surface, requestedFormat, presentMode, bufferCount);

  if( oldSwapchain )
  {
    for( auto&& i : mSurfaceMap )
    {
      if( i.second.swapchain == oldSwapchain )
      {
        i.second.swapchain = nullptr;
        break;
      }
    }
  }

  if( oldSwapchain )
  {
    // prevent destroying the swapchain as it is handled automatically
    // during replacing the swapchain
    auto khr = oldSwapchain->GetVkHandle();
    oldSwapchain->SetVkHandle(nullptr);
    oldSwapchain->Release();

    mLogicalDevice.destroySwapchainKHR( khr, *mAllocator );
  }

  // @todo: Only create framebuffers if no "external" render passes.
  newSwapchain->CreateFramebuffers(); // Note, this may destroy vk swapchain if invalid.
  return newSwapchain;
}

vk::Result Device::Present( Queue& queue, vk::PresentInfoKHR presentInfo )
{
  auto lock( queue.Lock() );
  return queue.Present( presentInfo );
}

vk::Result Device::QueueWaitIdle( Queue& queue )
{
  auto lock( queue.Lock() );
  return queue.WaitIdle();
}

vk::Result Device::DeviceWaitIdle()
{
  return mLogicalDevice.waitIdle();
}

const vk::AllocationCallbacks& Device::GetAllocator( const char* tag )
{
  if( mAllocator )
  {
    //mAllocator->setPUserData( CreateMemoryAllocationTag( tag ) );
  }
  return *mAllocator;
}

Queue& Device::GetGraphicsQueue( uint32_t index ) const
{
  return *mGraphicsQueues[index];
}

Queue& Device::GetTransferQueue( uint32_t index ) const
{
  return *mTransferQueues[index];
}

Queue& Device::GetComputeQueue( uint32_t index ) const
{
  return *mComputeQueues[index];
}

Queue& Device::GetPresentQueue() const
{
  return GetGraphicsQueue(0);
}

void Device::DiscardResource( std::function< void() > deleter )
{
  //std::lock_guard< std::mutex > lock( mMutex );
  //mDiscardQueue[mCurrentBufferIndex].push_back( std::move( deleter ) );
}

Fence* Device::CreateFence( const vk::FenceCreateInfo& fenceCreateInfo )
{
  vk::Fence vkFence;
  VkAssert( mLogicalDevice.createFence( &fenceCreateInfo, mAllocator.get(), &vkFence ) );

  return new Fence(*this, vkFence);
}

FramebufferImpl* Device::CreateFramebuffer(const std::vector< FramebufferAttachment* >& colorAttachments,
                                           FramebufferAttachment* depthAttachment,
                                           uint32_t width,
                                           uint32_t height,
                                           RenderPassImpl* externalRenderPass )
{
  assert( ( !colorAttachments.empty() || depthAttachment )
          && "Cannot create framebuffer. Please provide at least one attachment" );

  auto colorAttachmentsValid = true;
  for( auto& attachment : colorAttachments )
  {
    if( !attachment->IsValid() )
    {
      colorAttachmentsValid = false;
      break;
    }
  }

  assert( colorAttachmentsValid && "Invalid color attachment! The attachment has no ImageView" );

  // Flag that indicates if the framebuffer has a depth attachment
  auto hasDepth = false;
  if( depthAttachment )
  {
    hasDepth = depthAttachment->IsValid();
    assert( hasDepth && "Invalid depth attachment! The attachment has no ImageView" );
  }

  auto renderPass = externalRenderPass;

  // Flag that indicates if the render pass is externally provided
  auto isRenderPassExternal = (externalRenderPass != nullptr);

  // This vector stores the attachments (vk::ImageViews)
  auto attachments = std::vector< vk::ImageView >{};

  // If no external render pass was provided, create one internally
  if( !isRenderPassExternal )
  {
    renderPass = RenderPassImpl::NewRenderPass( *this, colorAttachments, depthAttachment );
    attachments = renderPass->GetAttachments();
  }

  // Finally create the framebuffer
  auto framebufferCreateInfo = vk::FramebufferCreateInfo{}.setRenderPass( renderPass->GetVkHandle() )
                                                          .setPAttachments( attachments.data() )
                                                          .setLayers( 1 )
                                                          .setWidth( width )
                                                          .setHeight( height )
                                                          .setAttachmentCount( U32( attachments.size() ) );

  auto framebuffer = VkAssert( mLogicalDevice.createFramebuffer( framebufferCreateInfo, mAllocator.get() ) );

  return new FramebufferImpl( *this,
                             colorAttachments,
                             depthAttachment,
                             framebuffer,
                             renderPass->GetVkHandle(),
                             width,
                             height,
                             isRenderPassExternal );
}



Image* Device::CreateImageFromExternal( vk::Image externalImage, vk::Format imageFormat, vk::Extent2D extent )
{
  auto imageCreateInfo = vk::ImageCreateInfo{}
          .setFormat( imageFormat )
          .setSamples( vk::SampleCountFlagBits::e1 )
          .setInitialLayout( vk::ImageLayout::eUndefined )
          .setSharingMode( vk::SharingMode::eExclusive )
          .setUsage( vk::ImageUsageFlagBits::eColorAttachment )
          .setExtent( { extent.width, extent.height, 1 } )
          .setArrayLayers( 1 )
          .setImageType( vk::ImageType::e2D )
          .setTiling( vk::ImageTiling::eOptimal )
          .setMipLevels( 1 );

  return new Image( *this, imageCreateInfo, externalImage );

  return nullptr;
}

ImageView* Device::CreateImageView(const vk::ImageViewCreateFlags& flags,
                                   const Image& image,
                                   vk::ImageViewType viewType,
                                   vk::Format format,
                                   vk::ComponentMapping components,
                                   vk::ImageSubresourceRange subresourceRange,
                                   void* pNext )
{
  auto imageViewCreateInfo = vk::ImageViewCreateInfo{}
          .setPNext( pNext )
          .setFlags( flags )
          .setImage( image.GetVkHandle() )
          .setViewType( viewType )
          .setFormat( format )
          .setComponents( components )
          .setSubresourceRange( std::move( subresourceRange ) );

  auto imageView = new ImageView( *this, &image, imageViewCreateInfo );

  VkAssert( mLogicalDevice.createImageView( &imageViewCreateInfo, &GetAllocator("IMAGEVIEW"), &imageView->mImageView ) );

  return imageView;
}

ImageView* Device::CreateImageView( Image* image )
{
  vk::ComponentMapping componentsMapping = { vk::ComponentSwizzle::eR,
                                             vk::ComponentSwizzle::eG,
                                             vk::ComponentSwizzle::eB,
                                             vk::ComponentSwizzle::eA };


  auto subresourceRange = vk::ImageSubresourceRange{}
          .setAspectMask( image->GetAspectFlags() )
          .setBaseArrayLayer( 0 )
          .setBaseMipLevel( 0 )
          .setLevelCount( image->GetMipLevelCount() )
          .setLayerCount( image->GetLayerCount() );

  auto imageView = CreateImageView({},
                                   *image,
                                   vk::ImageViewType::e2D,
                                   image->GetFormat(),
                                   componentsMapping,
                                   subresourceRange );

  return imageView;
}

vk::Result Device::WaitForFence(Fence* fence, uint32_t timeout)
{
  auto f = fence->GetVkHandle();
  return mLogicalDevice.waitForFences( 1, &f, VK_TRUE, timeout );
}

// -------------------------------------------------------------------------------------------------------
// Getters------------------------------------------------------------------------------------------------
SurfaceImpl* Device::GetSurface( Graphics::SurfaceId surfaceId )
{
  // Note, surface ID == 0 means main window
  if( surfaceId == 0 )
  {
    return mSurfaceMap.begin()->second.surface;
  }
  return mSurfaceMap[surfaceId].surface;
}

Swapchain* Device::GetSwapchainForSurface( SurfaceImpl* surface )
{
  for( auto&& val : mSurfaceMap )
  {
    if( val.second.surface == surface )
    {
      return val.second.swapchain;
    }
  }
  return nullptr;
}

Swapchain* Device::GetSwapchainForSurfaceId( Graphics::SurfaceId surfaceId )
{
  if( surfaceId == 0 )
  {
    return mSurfaceMap.begin()
                          ->second
                          .swapchain;
  }
  return mSurfaceMap[surfaceId].swapchain;
}

vk::Device Device::GetLogicalDevice() const
{
  return mLogicalDevice;
}

vk::PhysicalDevice Device::GetPhysicalDevice() const
{
  return mPhysicalDevice;
}

vk::Instance Device::GetInstance() const
{
  return mInstance;
}

Platform Device::GetDefaultPlatform() const
{
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
  mPlatform = Platform::WAYLAND;
#elif VK_USE_PLATFORM_XCB_KHR
  mPlatform = Platform::XCB;
#elif VK_USE_PLATFORM_XLIB_KHR
  mPlatform =  Platform::XLIB;
#else
  return mPlatform;
#endif
}

CommandPool* Device::GetCommandPool( std::thread::id threadId)
{
  CommandPool* commandPool=nullptr;
  {
    std::lock_guard<std::mutex> lock{mMutex};
    commandPool = mCommandPools.find(threadId) == mCommandPools.end()?nullptr:mCommandPools[threadId];
  }
  if(!commandPool)
  {
    vk::CommandPoolCreateInfo createInfo{};
    createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    commandPool = CommandPool::New(*this, createInfo);
    {
      std::lock_guard<std::mutex> lock{mMutex};
      mCommandPools[std::this_thread::get_id()] = commandPool;
    }
  }
  return commandPool;
}

void Device::SurfaceResized( unsigned int width, unsigned int height )
{
  // Get main window's surface
  // At first, check to empty about mSurfaceMap
  if ( !mSurfaceMap.empty() )
  {
    auto surface = mSurfaceMap.begin()->second.surface;
    if (surface)
    {
      auto surfaceCapabilities = surface->GetCapabilities();
      if ( surfaceCapabilities.currentExtent.width != width
         || surfaceCapabilities.currentExtent.height != height )
      {
        surface->UpdateSize( width, height );
        mSurfaceResized = true;
      }
    }
  }
}

uint32_t Device::SwapBuffers()
{
  mCurrentBufferIndex = (mCurrentBufferIndex+1)&1;
  return mCurrentBufferIndex;
}

uint32_t Device::GetCurrentBufferIndex() const
{
  return mCurrentBufferIndex;
}

void Device::CreateInstance( const std::vector< const char* >& extensions,
                               const std::vector< const char* >& validationLayers )
{
  auto info = vk::InstanceCreateInfo{};

  info.setEnabledExtensionCount( U32( extensions.size() ) )
      .setPpEnabledExtensionNames( extensions.data() )
      .setEnabledLayerCount( U32( validationLayers.size() ) )
      .setPpEnabledLayerNames( validationLayers.data() );

  const char* log_level = std::getenv( "LOG_VULKAN" );
  int intValue = log_level ? std::atoi(log_level) : 0;
  if ( !intValue )
  {
    info.setEnabledLayerCount( 0 );
  }

  mInstance = VkAssert( vk::createInstance( info, *mAllocator ) );
}

void Device::DestroyInstance()
{
  if( mInstance )
  {
    mInstance.destroy( *mAllocator );
    mInstance = nullptr;
  }
}

void Device::PreparePhysicalDevice()
{
  auto devices = VkAssert( mInstance.enumeratePhysicalDevices() );
  assert( !devices.empty() && "No Vulkan supported device found!" );

  // if only one, pick first
  mPhysicalDevice = nullptr;
  if( devices.size() == 1 )
  {
      mPhysicalDevice = devices[0];
  }
  else // otherwise look for one which is a graphics device
  {
    for( auto& device : devices )
    {
      auto properties = device.getProperties();
      if( properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu ||
          properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu )
      {
        auto queueFamilyProperties = device.getQueueFamilyProperties();
        for(const auto& queueFamily : queueFamilyProperties)
        {
          if(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
          {
            mPhysicalDevice = device;
            break;
          }
        }
      }
    }
  }

  assert( mPhysicalDevice && "No suitable Physical Device found!" );

  GetPhysicalDeviceProperties();
  GetQueueFamilyProperties();
}

void Device::GetPhysicalDeviceProperties()
{
  mPhysicalDeviceProperties =  mPhysicalDevice.getProperties();
  mPhysicalDeviceMemoryProperties = mPhysicalDevice.getMemoryProperties();
  mPhysicalDeviceFeatures = mPhysicalDevice.getFeatures();
}

void Device::GetQueueFamilyProperties()
{
  mQueueFamilyProperties = mPhysicalDevice.getQueueFamilyProperties();
}

std::vector< vk::DeviceQueueCreateInfo > Device::GetQueueCreateInfos()
{
  std::vector< vk::DeviceQueueCreateInfo > queueInfos{};

  constexpr uint8_t MAX_QUEUE_TYPES = 3;


  // find suitable family for each type of queue
  auto familyIndexTypes = std::array< uint32_t, MAX_QUEUE_TYPES >{};
  familyIndexTypes.fill( std::numeric_limits< uint32_t >::max() );

  // Device
  auto& graphicsFamily = familyIndexTypes[0];

  // Transfer
  auto& transferFamily = familyIndexTypes[1];

  // Present
  auto& presentFamily = familyIndexTypes[2];

  auto queueFamilyIndex = 0u;
  for( auto& prop : mQueueFamilyProperties )
  {
    if( ( prop.queueFlags & vk::QueueFlagBits::eGraphics ) && graphicsFamily == -1u )
    {
      graphicsFamily = queueFamilyIndex;
      presentFamily = queueFamilyIndex;
    }
    if( ( prop.queueFlags & vk::QueueFlagBits::eTransfer ) && transferFamily == -1u )
    {
      transferFamily = queueFamilyIndex;
    }
    ++queueFamilyIndex;
  }

  assert( graphicsFamily != std::numeric_limits< uint32_t >::max()
          && "No queue family that supports graphics operations!" );
  assert( transferFamily != std::numeric_limits< uint32_t >::max()
          && "No queue family that supports transfer operations!" );

  // todo: we may require that the family must be same for all type of operations, it makes
  // easier to handle synchronisation related issues.

  // sort queues
  std::sort( familyIndexTypes.begin(), familyIndexTypes.end() );

  // allocate all queues from graphics family
  auto prevQueueFamilyIndex = std::numeric_limits< uint32_t >::max();

  for( const auto& familyIndex : familyIndexTypes )
  {
    if( prevQueueFamilyIndex == familyIndex )
    {
      continue;
    }

    auto& queueCount = mQueueFamilyProperties[familyIndex].queueCount;

    // fill queue create info for the family.
    // note the priorities are not being set as local pointer will out of scope, this
    // will be fixed by the caller function
    auto info = vk::DeviceQueueCreateInfo{}
            .setPQueuePriorities( nullptr )
            .setQueueCount( queueCount )
            .setQueueFamilyIndex( familyIndex );
    queueInfos.push_back( info );
    prevQueueFamilyIndex = familyIndex;
  }

  return queueInfos;
}

std::vector< const char* > Device::PrepareDefaultInstanceExtensions()
{
  auto extensions = vk::enumerateInstanceExtensionProperties();

  std::string extensionName;

  bool xlibAvailable{ false };
  bool xcbAvailable{ false };
  bool waylandAvailable{ false };

  for( auto&& ext : extensions.value )
  {
    extensionName = std::string(ext.extensionName);
    if( extensionName == VK_KHR_XCB_SURFACE_EXTENSION_NAME )
    {
      xcbAvailable = true;
    }
    else if( extensionName == VK_KHR_XLIB_SURFACE_EXTENSION_NAME )
    {
      xlibAvailable = true;
    }
    else if( extensionName == VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME )
    {
      waylandAvailable = true;
    }
  }

  std::vector< const char* > retval{};

  // depending on the platform validate extensions
  auto platform = GetDefaultPlatform();

  if( platform != Platform::UNDEFINED )
  {
    if( platform == Platform::XCB && xcbAvailable )
    {
      retval.push_back( VK_KHR_XCB_SURFACE_EXTENSION_NAME );
    }
    else if( platform == Platform::XLIB && xlibAvailable )
    {
      retval.push_back( VK_KHR_XLIB_SURFACE_EXTENSION_NAME );
    }
    else if( platform == Platform::WAYLAND && waylandAvailable )
    {
      retval.push_back( VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME );
      /* For native image, check these exist first:
       *  VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
       *  VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME
       */
    }
  }
  else // try to determine the platform based on available extensions
  {
    if( xcbAvailable )
    {
      mPlatform = Platform::XCB;
      retval.push_back( VK_KHR_XCB_SURFACE_EXTENSION_NAME );
    }
    else if( xlibAvailable )
    {
      mPlatform = Platform::XLIB;
      retval.push_back( VK_KHR_XLIB_SURFACE_EXTENSION_NAME );
    }
    else if( waylandAvailable )
    {
      mPlatform = Platform::WAYLAND;
      retval.push_back( VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME );
      /* For native image, check these exist first:
       *  VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
       *  VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME
       */
    }
    else
    {
      // can't determine the platform!
      mPlatform = Platform::UNDEFINED;
    }
  }

  // other essential extensions
  retval.push_back( VK_KHR_SURFACE_EXTENSION_NAME );
  retval.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );

  return retval;
}

vk::Result Device::Submit(Queue& queue, const std::vector<SubmissionData>& submissionData, Fence* fence)
{
  auto lock( queue.Lock() );

  auto submitInfos = std::vector< vk::SubmitInfo >{};
  submitInfos.reserve( submissionData.size() );
  auto commandBufferHandles = std::vector< vk::CommandBuffer >{};

  // prepare memory
  auto bufferSize = 0u;
  for( auto& data : submissionData )
  {
    bufferSize += uint32_t( data.commandBuffers.size() );
  }
  commandBufferHandles.reserve( bufferSize );

  // Transform SubmissionData to vk::SubmitInfo
  for( const auto& subData : submissionData )
  {
    auto currentBufferIndex = commandBufferHandles.size();

    //Extract the command buffer handles
    std::transform( subData.commandBuffers.cbegin(),
                   subData.commandBuffers.cend(),
                   std::back_inserter( commandBufferHandles ),
                   [ & ]( CommandBufferImpl* entry ) {
                     return entry->GetVkHandle();
                   } );

    auto retval = vk::SubmitInfo().setWaitSemaphoreCount( U32( subData.waitSemaphores.size() ) )
                    .setPWaitSemaphores( subData.waitSemaphores.data() )
                    .setPWaitDstStageMask( &subData.waitDestinationStageMask )
                    .setCommandBufferCount( U32( subData.commandBuffers.size() ) )
                    .setPCommandBuffers( &commandBufferHandles[currentBufferIndex] )
                    .setSignalSemaphoreCount( U32( subData.signalSemaphores.size() ) )
                    .setPSignalSemaphores( subData.signalSemaphores.data() );

    submitInfos.push_back( retval );
  }

  return VkAssert( queue.Submit( submitInfos, fence) );


}

} // namespace Dali::Graphics::Vulkan
