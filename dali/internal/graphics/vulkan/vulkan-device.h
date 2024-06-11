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
#include <dali/internal/graphics/vulkan-impl/vulkan-surface-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-swapchain-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-queue-impl.h>
#include <dali/graphics-api/graphics-types.h>

#include <thread>
#include <mutex>
#include <map>
#include <functional>

namespace Dali::Graphics::Vulkan
{
class RenderPassImpl;

using CommandPoolMap = std::unordered_map< std::thread::id, CommandPool* >;

struct SwapchainSurfacePair
{
  Swapchain* swapchain;
  SurfaceImpl* surface;
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

  Graphics::SurfaceId CreateSurface( Dali::Graphics::SurfaceFactory& surfaceFactory,
                                     const Dali::Graphics::GraphicsCreateInfo& createInfo );

  void DestroySurface( Dali::Graphics::SurfaceId surfaceId );

  Swapchain* CreateSwapchainForSurface( SurfaceImpl* surface );

  Swapchain* ReplaceSwapchainForSurface( SurfaceImpl* surface, Swapchain*&& oldSwapchain);

  Swapchain* CreateSwapchain( SurfaceImpl* surface, vk::Format requestedFormat,
                              vk::PresentModeKHR presentMode,
                              uint32_t bufferCount,
                              Swapchain*&& oldSwapchain );

  vk::Result Submit(Queue& queue, const std::vector< SubmissionData >& submissionData, Fence* fence = nullptr);
  vk::Result Present( Queue& queue, vk::PresentInfoKHR presentInfo );
  vk::Result QueueWaitIdle( Queue& queue );
  vk::Result DeviceWaitIdle();


public: // Getters
  SurfaceImpl* GetSurface( Graphics::SurfaceId surfaceId );

  Swapchain* GetSwapchainForSurface( SurfaceImpl* surface );

  Swapchain* GetSwapchainForSurfaceId( Graphics::SurfaceId surfaceId );

  vk::Device GetLogicalDevice() const;

  vk::PhysicalDevice GetPhysicalDevice() const;

  vk::Instance GetInstance() const;

  const vk::AllocationCallbacks& GetAllocator( const char* tag  = nullptr );

  Queue& GetGraphicsQueue( uint32_t index = 0u ) const;

  Queue& GetTransferQueue( uint32_t index = 0u ) const;

  Queue& GetComputeQueue( uint32_t index = 0u ) const;

  Queue& GetPresentQueue() const;

  Platform GetDefaultPlatform() const;

  CommandPool* GetCommandPool( std::thread::id threadid);

  void SurfaceResized( unsigned int width, unsigned int height );
  bool IsSurfaceResized() const
  {
    return mSurfaceResized;
  }

  void DiscardResource( std::function< void() > deleter );

  Fence* CreateFence(const vk::FenceCreateInfo& fenceCreateInfo);

  FramebufferImpl* CreateFramebuffer(const std::vector< FramebufferAttachment* >& colorAttachments,
                                     FramebufferAttachment* depthAttachment,
                                     uint32_t width,
                                     uint32_t height,
                                     RenderPassImpl* externalRenderPass = nullptr);

  Image* CreateImageFromExternal( vk::Image externalImage, vk::Format imageFormat, vk::Extent2D extent );

  ImageView* CreateImageView(const vk::ImageViewCreateFlags& flags,
                             const Image& image,
                             vk::ImageViewType viewType,
                             vk::Format format,
                             vk::ComponentMapping components,
                             vk::ImageSubresourceRange subresourceRange,
                             void* pNext = nullptr);

  ImageView* CreateImageView( Image* image );

  vk::Result WaitForFence( Fence* fence, uint32_t timeout = std::numeric_limits< uint32_t >::max() );

  uint32_t GetCurrentBufferIndex() const;

  uint32_t SwapBuffers();


private: // Methods

  void CreateInstance( const std::vector< const char* >& extensions,
                       const std::vector< const char* >& validationLayers );

  void DestroyInstance();

  void PreparePhysicalDevice();

  void GetPhysicalDeviceProperties();

  void GetQueueFamilyProperties();

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

  CommandPoolMap mCommandPools;

  std::unordered_map< Graphics::SurfaceId, SwapchainSurfacePair > mSurfaceMap;
  bool mSurfaceResized{false};
  Graphics::SurfaceId mBaseSurfaceId{0u};

  Platform mPlatform{Platform::UNDEFINED};
  uint32_t mCurrentBufferIndex{0u};
  std::mutex mMutex;

  bool mHasDepth { false };
  bool mHasStencil { false };

};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_DEVICE_H
