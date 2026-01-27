#pragma once

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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
 */
#include <dali/devel-api/threading/thread-pool.h>
#include <dali/graphics-api/graphics-types.h>
#include <dali/public-api/common/vector-wrapper.h>

#include <dali/internal/graphics/vulkan-impl/vulkan-buffer.h>
#include <dali/internal/graphics/vulkan-impl/vulkan-resource-transfer-request.h>

namespace Dali::Graphics::Vulkan
{
class ResourceTransfer
{
public:
  ResourceTransfer(VulkanGraphicsController& mGraphicsController);
  ~ResourceTransfer();

  void Initialize();

  void UpdateTextures(const std::vector<Dali::Graphics::TextureUpdateInfo>&       updateInfoList,
                      const std::vector<Dali::Graphics::TextureUpdateSourceInfo>& sourceList);

  void UpdateSerially(
    const std::vector<Dali::Graphics::TextureUpdateInfo>&       updateInfoList,
    const std::vector<Dali::Graphics::TextureUpdateSourceInfo>& sourceList);
  void UpdateWithFutures(
    const std::vector<Dali::Graphics::TextureUpdateInfo>&       updateInfoList,
    const std::vector<Dali::Graphics::TextureUpdateSourceInfo>& sourceList);

  /**
   * If there are any outstanding resource transfers, wait for them.
   */
  void WaitOnResourceTransferFutures();

private:
  Dali::SharedFuture InitializeTextureStagingBuffer(uint32_t size, bool useWorkerThread);
  void               MapTextureStagingBuffer();
  void               UnmapTextureStagingBuffer();

  static void CopyBuffer(
    ResourceTransfer&                  resourceTransfer,
    Texture&                           destTexture,
    const Dali::Graphics::Buffer&      gfxBuffer,
    uint32_t                           bufferOffset,
    Dali::Graphics::Extent2D           extent2D,
    Dali::Graphics::Offset2D           textureOffset2D,
    uint32_t                           layer,
    uint32_t                           level,
    Dali::Graphics::TextureUpdateFlags flags);

  // Build command buffer for this copy & layout change, submit immediately,
  // and use Fence to wait for it to finish.
  static void CopyBufferAndTransition(
    ResourceTransfer&                  resourceTransfer,
    Texture&                           destTexture,
    const Dali::Graphics::Buffer&      gfxBuffer,
    uint32_t                           bufferOffset,
    Dali::Graphics::Extent2D           extent2D,
    Dali::Graphics::Offset2D           textureOffset2D,
    uint32_t                           layer,
    uint32_t                           level,
    Dali::Graphics::TextureUpdateFlags flags);

  /**
   * Direct copy memory to memory, used when linear tiling is enabled. This function
   * doesn't check if data is valid and doesn't perform format conversion.
   * @param destTexture The destination texture
   * @param info
   * @param sourceInfo
   * @param keepMapped if true, the memory stays mapped after the call
   */
  static void CopyMemoryDirect(
    ResourceTransfer&                              resourceTransfer,
    Texture&                                       destTexture,
    const Dali::Graphics::TextureUpdateInfo&       info,
    const Dali::Graphics::TextureUpdateSourceInfo& sourceInfo,
    bool                                           keepMapped);

  void CreateTransferFutures();
  void ScheduleResourceTransfer(ResourceTransferRequest&& transferRequest);
  void ProcessResourceTransferRequests(bool immediateOnly = false);

  Device& GetDevice()
  {
    return mGraphicsController.GetGraphicsDevice();
  }

  VulkanGraphicsController& GetController()
  {
    return mGraphicsController;
  }

private:
  VulkanGraphicsController& mGraphicsController;

  // used for texture<->buffer<->memory transfers
  std::vector<ResourceTransferRequest>  mResourceTransferRequests;
  std::recursive_mutex                  mResourceTransferMutex{};
  ThreadPool                            mThreadPool;
  std::unique_ptr<Vulkan::Buffer>       mTextureStagingBuffer{};
  Dali::SharedFuture                    mTextureStagingBufferFuture{};
  Graphics::UniquePtr<Graphics::Memory> mTextureStagingBufferMappedMemory{nullptr};
  void*                                 mTextureStagingBufferMappedPtr{nullptr};

  std::vector<std::shared_ptr<Future<void> > > mTransferFutures;
};

} // namespace Dali::Graphics::Vulkan
