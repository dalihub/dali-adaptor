#ifndef DALI_GRAPHICS_VULKAN_DEVICE_H
#define DALI_GRAPHICS_VULKAN_DEVICE_H

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

#ifndef VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_EXCEPTIONS
#endif

// INTERNAL INCLUDES
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/internal/graphics/common/surface-factory.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-queue.h>

#include <thread>
#include <mutex>
#include <map>
#include <functional>


namespace Dali
{
namespace Graphics
{
namespace Vulkan
{

// Consider using Graphics::UniquePtr<Vulkan::Swapchain>& ?
// where Vulkan::Swapchain is derived from Graphics::Resource<Graphics::Swapchain, Graphics::SwapchainCreateInfo>...
// Which is new!

struct SwapchainSurfacePair
{
  Swapchain* swapchain;
  Surface* surface;
};

class Device
{
public:
  Device();

  Device( const Device& ) = delete;

  Device& operator=( const Device& ) = delete;

  ~Device();

public: // Create methods

  void Create();

  void CreateDevice();

  FBID CreateSurface( Dali::Graphics::SurfaceFactory& surfaceFactory,
                      const Dali::Graphics::GraphicsCreateInfo& createInfo );

  void DestroySurface( Dali::Graphics::FBID framebufferId );

  Swapchain* CreateSwapchainForSurface( Surface* surface );

  Swapchain* ReplaceSwapchainForSurface( Surface* surface, Swapchain*&& oldSwapchain);

  Swapchain* CreateSwapchain( Surface* surface, vk::Format requestedFormat,
                              vk::PresentModeKHR presentMode,
                              uint32_t bufferCount,
                              Swapchain*&& oldSwapchain );

  vk::Result Present( Queue& queue, vk::PresentInfoKHR presentInfo );
  vk::Result QueueWaitIdle( Queue& queue );
  vk::Result DeviceWaitIdle();


public: // Getters
  Surface* GetSurface( FBID surfaceId );

  Swapchain* GetSwapchainForSurface( Surface* surface );

  Swapchain* GetSwapchainForFBID( FBID surfaceId );

  vk::Device GetDevice() const;

  vk::PhysicalDevice GetPhysicalDevice() const;

  vk::Instance GetInstance() const;

  const vk::AllocationCallbacks& GetAllocator( const char* tag  = nullptr );

  Queue& GetGraphicsQueue( uint32_t index = 0u ) const;

  Queue& GetTransferQueue( uint32_t index = 0u ) const;

  Queue& GetComputeQueue( uint32_t index = 0u ) const;

  Queue& GetPresentQueue() const;

  Platform GetDefaultPlatform() const;

  void SurfaceResized( unsigned int width, unsigned int height );
  bool IsSurfaceResized() const
  {
    return mSurfaceResized;
  }

  void DiscardResource( std::function< void() > deleter );

  Framebuffer* CreateFramebuffer(const std::vector< FramebufferAttachment* >& colorAttachments,
                                 FramebufferAttachment* depthAttachment,
                                 uint32_t width,
                                 uint32_t height,
                                 vk::RenderPass externalRenderPass = nullptr);

  vk::RenderPass CreateCompatibleRenderPass(const std::vector< FramebufferAttachment* >& colorAttachments,
                                            FramebufferAttachment* depthAttachment,
                                            std::vector<vk::ImageView>& attachments);

  Image* CreateImageFromExternal( vk::Image externalImage, vk::Format imageFormat, vk::Extent2D extent );

  ImageView* CreateImageView(const vk::ImageViewCreateFlags& flags,
                             const Image& image,
                             vk::ImageViewType viewType,
                             vk::Format format,
                             vk::ComponentMapping components,
                             vk::ImageSubresourceRange subresourceRange,
                             void* pNext = nullptr);

  ImageView* CreateImageView( Image* image );

private: // Methods

  void CreateInstance( const std::vector< const char* >& extensions,
                       const std::vector< const char* >& validationLayers );

  void DestroyInstance();

  void PreparePhysicalDevice();

  void GetPhysicalDeviceProperties();

  void GetQueueFamilyProperties();

  uint32_t SwapBuffers();

  uint32_t GetCurrentBufferIndex() const;

  std::vector< vk::DeviceQueueCreateInfo > GetQueueCreateInfos();

  std::vector< const char* > PrepareDefaultInstanceExtensions();

private: // Members
  vk::PhysicalDevice mPhysicalDevice;
  vk::Device mLogicalDevice;
  vk::Instance mInstance;

  vk::PhysicalDeviceProperties mPhysicalDeviceProperties;
  vk::PhysicalDeviceMemoryProperties mPhysicalDeviceMemoryProperties;
  vk::PhysicalDeviceFeatures mPhysicalDeviceFeatures;
  std::unique_ptr< vk::AllocationCallbacks > mAllocator{ nullptr };

  std::vector< vk::QueueFamilyProperties > mQueueFamilyProperties;

  // Sets of queues
  std::vector< std::unique_ptr< Queue > > mAllQueues;
  std::vector< Queue* > mGraphicsQueues;
  std::vector< Queue* > mTransferQueues;
  std::vector< Queue* > mComputeQueues;

  std::unordered_map< FBID, SwapchainSurfacePair > mSurfaceFBIDMap;
  bool mSurfaceResized;
  FBID mBaseFBID{0u};

  Platform mPlatform{Platform::UNDEFINED};
  uint32_t mCurrentBufferIndex{0u};
  std::mutex mMutex;

  bool mHasDepth { false };
  bool mHasStencil { false };

};

} // namespace Vulkan
} // namespace Graphics
} // namespace Dali

#endif // DALI_GRAPHICS_VULKAN_DEVICE_H
