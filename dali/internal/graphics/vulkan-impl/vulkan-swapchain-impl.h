#ifndef DALI_INTERNAL_GRAPHICS_VULKAN_SWAPCHAIN_IMPL_H
#define DALI_INTERNAL_GRAPHICS_VULKAN_SWAPCHAIN_IMPL_H

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
#include <dali/internal/graphics/vulkan-impl/vulkan-framebuffer-attachment.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-types.h>

namespace Dali::Graphics::Vulkan
{
class CommandBufferImpl;
class Device;
class FenceImpl;
class SurfaceImpl;
class Queue;
class SubmissionData;
struct SwapchainBuffer;

/**
 * Creates swapchain for given surface and queue
 */
class Swapchain
{
public:
  /**
   * @brief Create a new swapchain for the given surface.
   *
   * @param device The vulkan device
   * @param presentationQueue The queue to use for presenting the swapchain
   * @param oldSwapchain Any old swapchain we're recyclying
   * @param surface The surface to create the swapchain images for
   * @param requestedFormat The desired image format
   * @param presentMode Usually eFifo or eMailbox
   * @param[out] bufferCount Number of available swapchain buffers
   * @return A new swapchain
   */
  static Swapchain* NewSwapchain(
    Device&            device,
    Queue&             presentationQueue,
    vk::SwapchainKHR   oldSwapchain,
    SurfaceImpl*       surface,
    vk::Format         requestedFormat,
    vk::PresentModeKHR presentMode,
    uint32_t&          bufferCount);

  Swapchain(Device& graphicsDevice, Queue& presentationQueue);

  ~Swapchain();

  Swapchain(const Swapchain&)            = delete;
  Swapchain& operator=(const Swapchain&) = delete;

  void Destroy();

  /**
   * Automatically create framebuffers (generating compatible render passes)
   *
   * @param[in] depthAttachment Optional depth attachment.
   */
  void CreateFramebuffers(FramebufferAttachmentHandle depthAttachment);

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
  [[nodiscard]] FramebufferImpl* AcquireNextFramebuffer(bool shouldCollectGarbageNow = true);

  /**
   * Submits the given command buffer to the swapchain queue
   */
  void Submit(CommandBufferImpl* commandBuffer, const std::vector<vk::Semaphore>& depends);

  void UpdateSubmissionData(std::vector<SubmissionData>& submissionData);

  Queue* GetQueue();

  FenceImpl* GetEndOfFrameFence();

  /**
   * Get the end-of-frame fence for a specific buffer index
   * @param bufferIndex The buffer index to get the fence for
   * @return Pointer to the fence, or nullptr if index is invalid
   */
  FenceImpl* GetEndOfFrameFence(uint32_t bufferIndex);

  /**
   * Presents using default present queue, asynchronously
   * @return true if something was presented to the surface, regardless of error
   */
  bool Present();

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
   * Get the current depth/stencil format, if any.
   * @return The current depth stencil format.
   */
  vk::Format GetDepthStencilFormat()
  {
    return mDepthStencilFormat;
  }

  /**
   * Returns surface associated with swapchain
   * @return Pointer to surface
   */
  [[nodiscard]] SurfaceImpl* GetSurface() const
  {
    return mSurface;
  }

  [[nodiscard]] uint32_t GetCurrentBufferIndex() const;

  [[nodiscard]] uint32_t GetBufferCount() const
  {
    return mBufferCount;
  }

private:
  void CreateVkSwapchain(
    vk::SwapchainKHR   oldSwapchain,
    SurfaceImpl*       surface,
    vk::Format         requestedFormat,
    vk::PresentModeKHR presentMode,
    uint32_t&          bufferCount);

private:
  Device&      mGraphicsDevice;
  Queue*       mQueue;
  SurfaceImpl* mSurface{};

  vk::SwapchainKHR           mSwapchainKHR;
  vk::SwapchainCreateInfoKHR mSwapchainCreateInfoKHR{};

  /**
   * FramebufferImpl object associated with the buffer
   */
  using OwnedFramebuffer = std::unique_ptr<FramebufferImpl, void (*)(FramebufferImpl*)>;
  std::vector<OwnedFramebuffer>           mFramebuffers;
  std::vector<std::unique_ptr<Image>>     mSwapchainImages;
  std::vector<std::unique_ptr<ImageView>> mImageViews;
  std::unique_ptr<ImageView>              mDepthStencilImageView;
  std::unique_ptr<Image>                  mDepthStencilBuffer;

  /**
   * Array of swapchain buffers
   */
  std::vector<std::unique_ptr<SwapchainBuffer>> mSwapchainBuffers;
  uint32_t                                      mBufferCount{2u};       ///< Minimum Number of swapchain buffers
  uint32_t                                      mFrameCounter{0u};      ///< Current frame number
  uint32_t                                      mSwapchainImageIndex{}; ///< Swapchain image index returned by vkAcquireNextImageKHR

  vk::Format mDepthStencilFormat{vk::Format::eUndefined};

  bool mIsValid; // indicates whether the swapchain is still valid or requires to be recreated
};

} // namespace Dali::Graphics::Vulkan

#endif // DALI_INTERNAL_GRAPHICS_VULKAN_SWAPCHAIN_IMPL_H
