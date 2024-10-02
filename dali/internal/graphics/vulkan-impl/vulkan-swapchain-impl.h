#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_SWAPCHAIN_IMPL_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_SWAPCHAIN_IMPL_H

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
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

namespace Dali::Graphics::Vulkan
{
class CommandBufferImpl;
class Device;
class FenceImpl;
class SurfaceImpl;
class Queue;
class SwapchainBuffer;

/**
 * Creates swapchain for given surface and queue
 */
class Swapchain
{
public:
  static Swapchain* NewSwapchain(
    Device&            device,
    Queue&             presentationQueue,
    vk::SwapchainKHR   oldSwapchain,
    SurfaceImpl*       surface,
    vk::Format         requestedFormat,
    vk::PresentModeKHR presentMode,
    uint32_t           bufferCount);

  Swapchain(Device& graphicsDevice, Queue& presentationQueue);

  ~Swapchain();

  Swapchain(const Swapchain&)            = delete;
  Swapchain& operator=(const Swapchain&) = delete;

  void Destroy();

  /**
   * Automatically create framebuffers (generating compatible render passes)
   */
  void CreateFramebuffers();

  [[nodiscard]] vk::SwapchainKHR GetVkHandle() const;

  void SetVkHandle(vk::SwapchainKHR swapchainKhr)
  {
    mSwapchainKHR = swapchainKhr;
  }

  /**
   * Returns current framebuffer ( the one which is rendering to )
   * @return
   */
  [[nodiscard]] FramebufferImpl* GetCurrentFramebuffer() const;

  /**
   * Returns any framebuffer from the queue
   * @param index
   * @return
   */
  [[nodiscard]] FramebufferImpl* GetFramebuffer(uint32_t index) const;

  /**
   * This function acquires next framebuffer
   * @todo we should rather use round robin method
   * @return
   */
  FramebufferImpl* AcquireNextFramebuffer(bool shouldCollectGarbageNow = true);

  /**
   * Submits the given command buffer to the swapchain queue
   */
  void Submit(CommandBufferImpl* commandBuffer);

  /**
   * Presents using default present queue, asynchronously
   */
  void Present();

  /**
   * Returns true when swapchain expired
   * @return
   */
  [[nodiscard]] bool IsValid() const;

  void Invalidate();

  [[nodiscard]] Queue* GetQueue() const
  {
    return mQueue;
  }

  /**
   * Enables depth/stencil buffer for swapchain ( off by default )
   *
   * @param[in] depthStencilFormat valid depth/stencil pixel format
   */
  void SetDepthStencil(vk::Format depthStencilFormat);

  /**
   * Returns number of allocated swapchain images
   * @return Number of swapchain images
   */
  [[nodiscard]] uint32_t GetImageCount() const;

private:
  void CreateVkSwapchain(
    vk::SwapchainKHR   oldSwapchain,
    SurfaceImpl*       surface,
    vk::Format         requestedFormat,
    vk::PresentModeKHR presentMode,
    uint32_t           bufferCount);

private:
  Device&      mGraphicsDevice;
  Queue*       mQueue;
  SurfaceImpl* mSurface{};

  uint32_t mSwapchainImageIndex{}; ///< Swapchain image index returned by vkAcquireNextImageKHR

  vk::SwapchainKHR           mSwapchainKHR;
  vk::SwapchainCreateInfoKHR mSwapchainCreateInfoKHR{};

  /**
   * FramebufferImpl object associated with the buffer
   */
  using OwnedFramebuffer = std::unique_ptr<FramebufferImpl, void (*)(FramebufferImpl*)>;
  std::vector<OwnedFramebuffer> mFramebuffers;

  /**
   * Array of swapchain buffers
   */
  std::vector<std::unique_ptr<SwapchainBuffer>> mSwapchainBuffers;
  uint32_t                                      mFrameCounter{0u}; ///< Current frame number

  bool mIsValid; // indicates whether the swapchain is still valid or requires to be recreated
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_SWAPCHAIN_IMPL_H
