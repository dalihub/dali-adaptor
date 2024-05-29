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
#include <dali/internal/graphics/vulkan-impl/vulkan-command-pool-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-fence-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-image-impl.h>
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

  mSurfaceFBIDMap.clear();

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

Graphics::FramebufferId Device::CreateSurface(
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

  // map surface to FramebufferImpl Id
  auto fbid = ++mBaseFramebufferId;

  mSurfaceFBIDMap[ fbid ] = SwapchainSurfacePair{ nullptr, surface };

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

  return fbid;
}

void Device::DestroySurface( Dali::Graphics::FramebufferId framebufferId )
{
  if( auto surface = GetSurface( framebufferId ) )
  {
    DeviceWaitIdle();
    auto swapchain = GetSwapchainForFramebuffer( framebufferId );
    swapchain->Destroy();
    surface->Destroy();
  }
}

Swapchain* Device::CreateSwapchainForSurface( SurfaceImpl* surface )
{
  DALI_ASSERT_DEBUG(surface && "Surface ptr must be allocated");

  auto surfaceCapabilities = surface->GetCapabilities();

  //TODO: propagate the format and presentation mode to higher layers to allow for more control?
  Swapchain* swapchain = CreateSwapchain( surface,
                                          vk::Format::eB8G8R8A8Unorm,
                                          vk::PresentModeKHR::eFifo,
                                          surfaceCapabilities.minImageCount,
                                          nullptr );

  // store swapchain in the correct pair
  for( auto&& val : mSurfaceFBIDMap )
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
  for( auto&& val : mSurfaceFBIDMap )
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
  // obtain supported image format
  auto supportedFormats = VkAssert( mPhysicalDevice.getSurfaceFormatsKHR( surface->GetVkHandle() ) );

  vk::Format swapchainImageFormat{};
  vk::ColorSpaceKHR swapchainColorSpace{};

  // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
  // there is no preferred format, so we assume vk::Format::eB8G8R8A8Unorm
  if( supportedFormats.size() == 1 && supportedFormats[0].format == vk::Format::eUndefined )
  {
    swapchainColorSpace = supportedFormats[0].colorSpace;
    swapchainImageFormat = vk::Format::eB8G8R8A8Unorm;
  }
  else // Try to find the requested format in the list
  {
    auto found = std::find_if( supportedFormats.begin(),
                               supportedFormats.end(),
                               [ & ]( vk::SurfaceFormatKHR supportedFormat ) {
                                 return requestedFormat == supportedFormat.format;
                               } );

    // If found assign it.
    if( found != supportedFormats.end() )
    {
      auto surfaceFormat = *found;
      swapchainColorSpace = surfaceFormat.colorSpace;
      swapchainImageFormat = surfaceFormat.format;
    }
    else // Requested format not found...attempt to use the first one on the list
    {
      auto surfaceFormat = supportedFormats[0];
      swapchainColorSpace = surfaceFormat.colorSpace;
      swapchainImageFormat = surfaceFormat.format;
    }
  }

  assert( swapchainImageFormat != vk::Format::eUndefined && "Could not find a supported swap chain image format." );

  // Get the surface capabilities to determine some settings of the swap chain
  auto surfaceCapabilities = surface->GetCapabilities();

  // Determine the swap chain extent
  auto swapchainExtent = surfaceCapabilities.currentExtent;

  // Find a supported composite alpha format (not all devices support alpha opaque)
  auto compositeAlpha = vk::CompositeAlphaFlagBitsKHR{};

  // Simply select the first composite alpha format available
  auto compositeAlphaFlags = std::vector< vk::CompositeAlphaFlagBitsKHR >{
          vk::CompositeAlphaFlagBitsKHR::eOpaque,
          vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
          vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
          vk::CompositeAlphaFlagBitsKHR::eInherit
  };

  for( const auto& compositeAlphaFlag : compositeAlphaFlags )
  {

    if( surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag )
    {
      compositeAlpha = compositeAlphaFlag;
      break;
    }
  }

  // Determine the number of images
  if (surfaceCapabilities.minImageCount > 0 &&
      bufferCount > surfaceCapabilities.minImageCount )
  {
      bufferCount = surfaceCapabilities.minImageCount;
  }

  // Find the transformation of the surface
  vk::SurfaceTransformFlagBitsKHR preTransform;
  if( surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity )
  {
    // We prefer a non-rotated transform
    preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
  }
  else
  {
    preTransform = surfaceCapabilities.currentTransform;
  }


  // Check if the requested present mode is supported
  auto presentModes = mPhysicalDevice.getSurfacePresentModesKHR( surface->GetVkHandle() ).value;

  auto found = std::find_if( presentModes.begin(),
                             presentModes.end(),
                             [ & ]( vk::PresentModeKHR mode ) {
                               return presentMode == mode;
                             } );

  if( found == presentModes.end() )
  {
    // Requested present mode not supported. Default to FIFO. FIFO is always supported as per spec.
    presentMode = vk::PresentModeKHR::eFifo;
  }

  // Creation settings have been determined. Fill in the create info struct.
  auto swapChainCreateInfo = vk::SwapchainCreateInfoKHR{}.setSurface( surface->GetVkHandle() )
                                                         .setPreTransform( preTransform )
                                                         .setPresentMode( presentMode )
                                                         .setOldSwapchain( oldSwapchain ? oldSwapchain->GetVkHandle()
                                                                                        : vk::SwapchainKHR{} )
                                                         .setMinImageCount( bufferCount )
                                                         .setImageUsage( vk::ImageUsageFlagBits::eColorAttachment )
                                                         .setImageSharingMode( vk::SharingMode::eExclusive )
                                                         .setImageArrayLayers( 1 )
                                                         .setImageColorSpace( swapchainColorSpace )
                                                         .setImageFormat( swapchainImageFormat )
                                                         .setImageExtent( swapchainExtent )
                                                         .setCompositeAlpha( compositeAlpha )
                                                         .setClipped( static_cast<vk::Bool32>(true) )
                                                         .setQueueFamilyIndexCount( 0 )
                                                         .setPQueueFamilyIndices( nullptr );


  // Create the swap chain
  auto swapChainVkHandle = VkAssert( mLogicalDevice.createSwapchainKHR( swapChainCreateInfo, mAllocator.get() ) );

  if( oldSwapchain )
  {
    for( auto&& i : mSurfaceFBIDMap )
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

  // pull images and create Framebuffers
  auto images = VkAssert( mLogicalDevice.getSwapchainImagesKHR( swapChainVkHandle ) );

  // number of images must match requested buffering mode
  if( images.size() < surfaceCapabilities.minImageCount )
  {
    DALI_LOG_STREAM( gVulkanFilter,
                     Debug::General,
                     "Swapchain creation failed: Swapchain images are less than the requested amount" );
    mLogicalDevice.destroySwapchainKHR( swapChainVkHandle );
    return nullptr;
  }

  auto framebuffers = std::vector<FramebufferImpl* >{};
  framebuffers.reserve( images.size() );

  auto clearColor = vk::ClearColorValue{}.setFloat32( { 0.0f, 0.0f, 0.0f, 0.0f } );

  //
  // CREATE FRAMEBUFFERS
  //

  for( auto&& image : images )
  {
    auto colorImageView = CreateImageView( CreateImageFromExternal( image, swapchainImageFormat, swapchainExtent ) );

    // A new color attachment for each framebuffer
    auto colorAttachment = FramebufferAttachment::NewColorAttachment( colorImageView,
                                                                      clearColor,
                                                                      true ); // presentable

    framebuffers.push_back( CreateFramebuffer( { colorAttachment },
                                               nullptr,
                                               swapchainExtent.width,
                                               swapchainExtent.height ) );
  }

  return new Swapchain( *this, GetPresentQueue(), surface, std::move(framebuffers),
                       swapChainCreateInfo, swapChainVkHandle );
}

vk::Result Device::Present( Queue& queue, vk::PresentInfoKHR presentInfo )
{
  auto lock( queue.Lock() );
  return queue.mQueue.presentKHR( &presentInfo );
}

vk::Result Device::QueueWaitIdle( Queue& queue )
{
  auto lock( queue.Lock() );
  return queue.mQueue.waitIdle();
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
                                       vk::RenderPass externalRenderPass )
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
  auto isRenderPassExternal = externalRenderPass != vk::RenderPass{};

  // This vector stores the attachments (vk::ImageViews)
  auto attachments = std::vector< vk::ImageView >{};

  // If no external render pass was provided, create one internally
  if( !isRenderPassExternal )
  {
    renderPass = CreateCompatibleRenderPass( colorAttachments, depthAttachment, attachments );
  }

  // Finally create the framebuffer
  auto framebufferCreateInfo = vk::FramebufferCreateInfo{}.setRenderPass( renderPass )
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
                             renderPass,
                             width,
                             height,
                             isRenderPassExternal );
}

vk::RenderPass Device::CreateCompatibleRenderPass(
  const std::vector< FramebufferAttachment* >& colorAttachments,
  FramebufferAttachment* depthAttachment,
  std::vector<vk::ImageView>& attachments)
{
  auto hasDepth = false;
  if( depthAttachment )
  {
    hasDepth = depthAttachment->IsValid();
    assert( hasDepth && "Invalid depth attachment! The attachment has no ImageView" );
  }

  // The total number of attachments
  auto totalAttachmentCount = hasDepth ? colorAttachments.size() + 1 : colorAttachments.size();

  attachments.clear();
  attachments.reserve( totalAttachmentCount );

  // This vector stores the attachment references
  auto colorAttachmentReferences = std::vector< vk::AttachmentReference >{};
  colorAttachmentReferences.reserve( colorAttachments.size() );

  // This vector stores the attachment descriptions
  auto attachmentDescriptions = std::vector< vk::AttachmentDescription >{};
  attachmentDescriptions.reserve( totalAttachmentCount );

  // For each color attachment...
  for( auto i = 0u; i < colorAttachments.size(); ++i )
  {
    // Get the image layout
    auto imageLayout = colorAttachments[i]->GetImageView()->GetImage()->GetImageLayout();

    // If the layout is undefined...
    if( imageLayout == vk::ImageLayout::eUndefined )
    {
      // Set it to color attachment optimal
      imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    }

    // Any other case should be invalid
    assert( imageLayout == vk::ImageLayout::eColorAttachmentOptimal );

    // Add a reference and a descriptions and image views to their respective vectors
    colorAttachmentReferences.push_back( vk::AttachmentReference{}.setLayout( imageLayout )
                                                                  .setAttachment( U32( i ) ) );

    attachmentDescriptions.push_back( colorAttachments[i]->GetDescription() );

    attachments.push_back( colorAttachments[i]->GetImageView()->GetVkHandle() );
  }


  // Follow the exact same procedure as color attachments
  auto depthAttachmentReference = vk::AttachmentReference{};
  if( hasDepth )
  {
    auto imageLayout = depthAttachment->GetImageView()->GetImage()->GetImageLayout();

    if( imageLayout == vk::ImageLayout::eUndefined )
    {
      imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }

    assert( imageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal );

    depthAttachmentReference.setLayout( imageLayout );
    depthAttachmentReference.setAttachment( U32( colorAttachmentReferences.size() ) );

    attachmentDescriptions.push_back( depthAttachment->GetDescription() );

    attachments.push_back( depthAttachment->GetImageView()->GetVkHandle() );
  }

  // Creating a single subpass per framebuffer
  auto subpassDesc = vk::SubpassDescription{};
  subpassDesc.setPipelineBindPoint( vk::PipelineBindPoint::eGraphics );
  subpassDesc.setColorAttachmentCount( U32( colorAttachments.size()));
  if( hasDepth )
  {
    subpassDesc.setPDepthStencilAttachment( &depthAttachmentReference );
  }
  subpassDesc.setPColorAttachments( colorAttachmentReferences.data() );

  // Creating 2 subpass dependencies using VK_SUBPASS_EXTERNAL to leverage the implicit image layout
  // transitions provided by the driver
  std::array< vk::SubpassDependency, 2 > subpassDependencies{

    vk::SubpassDependency{}.setSrcSubpass( VK_SUBPASS_EXTERNAL )
                           .setDstSubpass( 0 )
                           .setSrcStageMask( vk::PipelineStageFlagBits::eBottomOfPipe )
                           .setDstStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput )
                           .setSrcAccessMask( vk::AccessFlagBits::eMemoryRead )
                           .setDstAccessMask( vk::AccessFlagBits::eColorAttachmentRead |
                                              vk::AccessFlagBits::eColorAttachmentWrite )
                           .setDependencyFlags( vk::DependencyFlagBits::eByRegion ),

    vk::SubpassDependency{}.setSrcSubpass( 0 )
                           .setDstSubpass( VK_SUBPASS_EXTERNAL )
                           .setSrcStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput )
                           .setDstStageMask( vk::PipelineStageFlagBits::eBottomOfPipe )
                           .setSrcAccessMask( vk::AccessFlagBits::eColorAttachmentRead |
                                              vk::AccessFlagBits::eColorAttachmentWrite )
                           .setDstAccessMask( vk::AccessFlagBits::eMemoryRead )
                           .setDependencyFlags( vk::DependencyFlagBits::eByRegion )

  };


  // Create the render pass
  auto renderPassCreateInfo = vk::RenderPassCreateInfo{}.setAttachmentCount( U32( attachmentDescriptions.size() ) )
                                                        .setPAttachments( attachmentDescriptions.data() )
                                                        .setPSubpasses( &subpassDesc )
                                                        .setSubpassCount( 1 )
                                                        .setPDependencies( subpassDependencies.data() );


  return VkAssert( mLogicalDevice.createRenderPass( renderPassCreateInfo, mAllocator.get() ) );
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
SurfaceImpl* Device::GetSurface( Graphics::FramebufferId surfaceId )
{
  // TODO: FBID == 0 means default framebuffer, but there should be no
  // such thing as default framebuffer.
  if( surfaceId == 0 )
  {
    return mSurfaceFBIDMap.begin()->second.surface;
  }
  return mSurfaceFBIDMap[surfaceId].surface;
}

Swapchain* Device::GetSwapchainForSurface( SurfaceImpl* surface )
{
  for( auto&& val : mSurfaceFBIDMap )
  {
    if( val.second.surface == surface )
    {
      return val.second.swapchain;
    }
  }
  return nullptr;
}

Swapchain* Device::GetSwapchainForFramebuffer( Graphics::FramebufferId surfaceId )
{
  if( surfaceId == 0 )
  {
    return mSurfaceFBIDMap.begin()
                          ->second
                          .swapchain;
  }
  return mSurfaceFBIDMap[surfaceId].swapchain;
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
  // Get surface with FBID "0"
  // At first, check to empty about mSurfaceFBIDMap
  if ( !mSurfaceFBIDMap.empty() )
  {
    auto surface = mSurfaceFBIDMap.begin()->second.surface;
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

} // namespace Dali::Graphics::Vulkan
