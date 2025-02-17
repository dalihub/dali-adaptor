#ifndef DALI_GRAPHICS_VULKAN_DEVICE_H
#define DALI_GRAPHICS_VULKAN_DEVICE_H

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

#ifndef VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_EXCEPTIONS
#endif

// INTERNAL INCLUDES
#include <dali/graphics-api/graphics-types.h>
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/internal/graphics/common/surface-factory.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-queue-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-surface-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-swapchain-impl.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

#include <functional>
#include <map>
#include <mutex>
#include <thread>

namespace Dali::Graphics::Vulkan
{
class RenderPassImpl;
class CommandPool;
using CommandPoolMap = std::unordered_map<std::thread::id, CommandPool*>;

struct SwapchainSurfacePair
{
  Swapchain*   swapchain;
  SurfaceImpl* surface;
};

class Device
{
public:
  Device();

  Device(const Device&) = delete;

  Device& operator=(const Device&) = delete;

  ~Device();

public: // Create methods
  void Create();

  void CreateDevice(SurfaceImpl* surface);

  Graphics::SurfaceId CreateSurface(Dali::Graphics::SurfaceFactory&           surfaceFactory,
                                    const Dali::Graphics::GraphicsCreateInfo& createInfo);

  void DestroySurface(Dali::Graphics::SurfaceId surfaceId);

  void CreateSwapchainForSurface(Dali::Graphics::SurfaceId surfaceId);

  Swapchain* ReplaceSwapchainForSurface(Dali::Graphics::SurfaceId surfaceId, Swapchain*&& oldSwapchain);

  /**
   * Ensures that the next available image is retrieved for drawing onto.
   * Should only call this method if there is something to present, as this
   * sets up a fence, and will cause a stall if nothing waits on it.
   * @param surfaceId The id of the surface to get the next image for
   */
  void AcquireNextImage(SurfaceId surfaceId);

  vk::Result Present(Queue& queue, vk::PresentInfoKHR& presentInfo);
  vk::Result QueueWaitIdle(Queue& queue);
  vk::Result DeviceWaitIdle();

public: // Getters
  SurfaceImpl* GetSurface(Graphics::SurfaceId surfaceId);

  Swapchain* GetSwapchainForSurface(SurfaceImpl* surface);

  Swapchain* GetSwapchainForSurfaceId(Graphics::SurfaceId surfaceId);

  vk::Device GetLogicalDevice() const;

  vk::PhysicalDevice GetPhysicalDevice() const;

  vk::Instance GetInstance() const;

  const vk::AllocationCallbacks& GetAllocator(const char* tag = nullptr);

  Queue& GetGraphicsQueue(uint32_t index = 0u) const;

  Queue& GetTransferQueue(uint32_t index = 0u) const;

  Queue& GetComputeQueue(uint32_t index = 0u) const;

  Queue& GetPresentQueue() const;

  Platform GetDefaultPlatform() const;

  CommandPool* GetCommandPool(std::thread::id threadid);

  void SurfaceResized(unsigned int width, unsigned int height);
  bool IsSurfaceResized() const
  {
    return mSurfaceResized;
  }

  void DiscardResource(std::function<void()> deleter);

  Image* CreateImageFromExternal(vk::Image externalImage, vk::Format imageFormat, vk::Extent2D extent);

  uint32_t GetCurrentBufferIndex() const;

  uint32_t GetBufferCount() const;

  uint32_t SwapBuffers();

  const vk::PhysicalDeviceMemoryProperties& GetMemoryProperties() const
  {
    return mPhysicalDeviceMemoryProperties;
  }

  static uint32_t GetMemoryIndex(
    const vk::PhysicalDeviceMemoryProperties& memoryProperties,
    uint32_t                                  memoryTypeBits,
    vk::MemoryPropertyFlags                   properties);

  const vk::PhysicalDeviceProperties& GetPhysicalDeviceProperties() const;

private: // Methods
  void CreateInstance(const std::vector<const char*>& extensions,
                      const std::vector<const char*>& validationLayers);

  void DestroyInstance();

  void PreparePhysicalDevice(SurfaceImpl* surface);

  void InitializePhysicalDeviceProperties();

  void GetQueueFamilyProperties();

  std::vector<vk::DeviceQueueCreateInfo> GetQueueCreateInfos();

  std::vector<const char*> PrepareDefaultInstanceExtensions();

  void ReleaseCommandPools();

  Swapchain* CreateSwapchain(SurfaceImpl* surface, vk::Format requestedFormat, vk::PresentModeKHR presentMode, Swapchain*&& oldSwapchain);

private: // Members
  vk::PhysicalDevice mPhysicalDevice;
  vk::Device         mLogicalDevice;
  vk::Instance       mInstance;

  vk::PhysicalDeviceProperties             mPhysicalDeviceProperties;
  vk::PhysicalDeviceMemoryProperties       mPhysicalDeviceMemoryProperties;
  vk::PhysicalDeviceFeatures               mPhysicalDeviceFeatures;
  std::unique_ptr<vk::AllocationCallbacks> mAllocator{nullptr};

  std::vector<vk::QueueFamilyProperties> mQueueFamilyProperties;

  // Sets of queues
  std::vector<std::unique_ptr<Queue>> mAllQueues;
  std::vector<Queue*>                 mGraphicsQueues;
  std::vector<Queue*>                 mTransferQueues;
  std::vector<Queue*>                 mComputeQueues;

  CommandPoolMap mCommandPools;

  std::unordered_map<Graphics::SurfaceId, SwapchainSurfacePair> mSurfaceMap;
  bool                                                          mSurfaceResized{false};
  Graphics::SurfaceId                                           mBaseSurfaceId{0u};

  Platform   mPlatform{Platform::UNDEFINED};
  uint32_t   mCurrentBufferIndex{0u};
  uint32_t   mBufferCount{2};
  std::mutex mMutex;

  bool mHasDepth{false};
  bool mHasStencil{false};
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_GRAPHICS_VULKAN_DEVICE_H
