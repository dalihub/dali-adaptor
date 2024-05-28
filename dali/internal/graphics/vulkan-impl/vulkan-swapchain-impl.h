#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_SWAPCHAIN_IMPL_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_SWAPCHAIN_IMPL_H

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

// INTERNAL INCLUDES
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

namespace Dali::Graphics::Vulkan
{
class Device;
class SurfaceImpl;
class Queue;
class SwapchainBuffer;

/**
 * Creates swapchain for given surface and queue
 */
class Swapchain : public VkManaged
{
public:
  ~Swapchain() override;
  Swapchain( const Swapchain& ) = delete;
  Swapchain& operator=( const Swapchain& ) = delete;

  Swapchain(Device& graphicsDevice,
            Queue& presentationQueue,
            SurfaceImpl* surface,
            std::vector< Framebuffer* >&& framebuffers,
            vk::SwapchainCreateInfoKHR createInfo,
            vk::SwapchainKHR vkHandle );

  [[nodiscard]] vk::SwapchainKHR GetVkHandle() const;
  void SetVkHandle(vk::SwapchainKHR swapchainKhr)
  {
    mSwapchainKHR = swapchainKhr;
  }

#if 0
  /**
   * Allocate command buffers for each render pass.
   * @param[in] renderPassCount The number of render passes to allocate for (includes main renderpass)
   */
  void AllocateCommandBuffers( size_t renderPassCount );

  /**
   * Returns current framebuffer ( the one which is rendering to )
   * @return
   */
  FramebufferImpl* GetCurrentFramebuffer() const;

  /**
   * Returns any framebuffer from the queue
   * @param index
   * @return
   */
  FramebufferImpl* GetFramebuffer( uint32_t index ) const;

  /**
   * This function acquires next framebuffer
   * @todo we should rather use round robin method
   * @return
   */
  FramebufferImpl* AcquireNextFramebuffer( bool shouldCollectGarbageNow = true );

  /**
   * Return the primary command buffer associated with the swapchain
   */
  CommandBuffer* GetLastCommandBuffer();

  /**
   * Returns the primary command buffers associated with each render pass
   * being recorded
   * @return mutable vector of command buffers
   */
  std::vector<CommandBuffer*>& GetCommandBuffers() const;

  /**
   * Presents using default present queue, asynchronously
   */
  void Present();

  /**
   * Presents using default queue, synchronized with supplied semaphores
   * @param waitSemaphores
   */
  void Present( std::vector< vk::Semaphore > waitSemaphores );

  bool OnDestroy() override;

  /**
   * Returns true when swapchain expired
   * @return
   */
  bool IsValid() const;

  void Invalidate();

  /**
   * Enables depth/stencil buffer for swapchain ( off by default )
   *
   * @param[in] depthStencilFormat valid depth/stencil pixel format
   */
  void SetDepthStencil( vk::Format depthStencilFormat );

  /**
   * Resets all existing command buffers for all swapchain images
   * To run it safe it must be sure the GPU is not using any of the
   * command buffers.
   */
  void ResetAllCommandBuffers();

  /**
   * Returns number of allocated swapchain images
   * @return Number of swapchain images
   */
  uint32_t GetImageCount() const;
#endif

private:
  Device* mGraphicsDevice;
  Queue* mQueue;
  SurfaceImpl* mSurface;

  uint32_t mSwapchainImageIndex; ///< Swapchain image index returned by vkAcquireNextImageKHR

  vk::SwapchainKHR mSwapchainKHR;
  vk::SwapchainCreateInfoKHR mSwapchainCreateInfoKHR;
  /*
   * FramebufferImpl object associated with the buffer
   */
  std::vector<Framebuffer*> mFramebuffers;

#if 0
  /**
   * Array of swapchain buffers
   */
  std::vector<std::unique_ptr<SwapchainBuffer>> mSwapchainBuffers;

  Fence* mBetweenRenderPassFence;

  uint32_t mFrameCounter { 0u }; ///< Current frame number
#endif

  bool mIsValid; // indicates whether the swapchain is still valid or requires to be recreated
};

} // namespace Dali::Graphics::Vulkan

#endif //DALI_INTERNAL_GRAPHICS_VULKAN_SWAPCHAIN_IMPL_H
