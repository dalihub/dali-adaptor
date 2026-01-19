#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_TIZEN_VULKAN_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_TIZEN_VULKAN_H

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
 *
 */

// EXTERNAL INCLUDES
#include <dali/devel-api/common/map-wrapper.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/images/native-image-interface.h>
#include <tbm_surface.h>
#include <tbm_surface_queue.h>
#include <atomic>
#include <chrono>
#include <unordered_map>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-source-queue-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * @brief Surface reference management interface for external consumers
 *
 * This interface allows external components (like Vulkan textures) to manage
 * surface lifetimes. The NativeImageSourceQueue uses this to delay surface
 * recycling until all references are released.
 */
class NativeImageSurfaceReference
{
public:
  virtual ~NativeImageSurfaceReference() = default;

  /**
   * @brief Called when a surface reference is acquired by an external consumer
   * @param surface The TBM surface handle
   */
  virtual void AcquireSurfaceReference(void* surface) = 0;

  /**
   * @brief Called when a surface reference is released by an external consumer
   * @param surface The TBM surface handle
   */
  virtual void ReleaseSurfaceReference(void* surface) = 0;
};

/**
 * Dali internal NativeImageSourceQueue.
 */
class NativeImageSourceQueueTizenVulkan : public Internal::Adaptor::NativeImageSourceQueue,
                                          public NativeImageSurfaceReference
{
public:
  /**
   * Create a new NativeImageSourceQueueTizenVulkan internally.
   * Depending on hardware the width and height may have to be a power of two.
   * @param[in] queueCount The number of queue of the image. If it is 0, will use default.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colorFormat The color format of the image.
   * @param[in] nativeImageSourceQueue contains tbm_surface_queue_h or is empty
   * @return A smart-pointer to a newly allocated image.
   */
  static NativeImageSourceQueueTizenVulkan* New(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorFormat colorFormat, Any nativeImageSourceQueue);

  /**
   * @copydoc Dali::NativeImageSourceQueue::GetNativeImageSourceQueue()
   */
  Any GetNativeImageSourceQueue() const override;

  /**
   * @copydoc Dali::NativeImageSourceQueue::SetSize
   */
  void SetSize(uint32_t width, uint32_t height) override;

  /**
   * @copydoc Dali::NativeImageSourceQueue::IgnoreSourceImage
   */
  void IgnoreSourceImage() override;

  /**
   * @copydoc Dali::NativeImageSourceQueue::CanDequeueBuffer
   */
  bool CanDequeueBuffer() override;

  /**
   * @copydoc Dali::NativeImageSourceQueue::DequeueBuffer
   */
  uint8_t* DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride, Dali::NativeImageSourceQueue::BufferAccessType type) override;

  /**
   * @copydoc Dali::NativeImageSourceQueue::EnqueueBuffer
   */
  bool EnqueueBuffer(uint8_t* buffer) override;

  /**
   * @copydoc Dali::NativeImageSourceQueue::CancelDequeuedBuffer
   */
  void CancelDequeuedBuffer(uint8_t* buffer) override;

  /**
   * @copydoc Dali::NativeImageSourceQueue::EnqueueBuffer
   */
  void FreeReleasedBuffers() override;

  /**
   * @copydoc Dali::NativeImageSourceQueue::SetQueueUsageHint
   */
  void SetQueueUsageHint(Dali::NativeImageSourceQueue::QueueUsageType type) override
  {
  }

  /**
   * destructor
   */
  ~NativeImageSourceQueueTizenVulkan() override;

  /**
   * @copydoc Dali::NativeImageInterface::CreateResource
   */
  bool CreateResource() override;

  /**
   * @copydoc Dali::NativeImageInterface::DestroyResource()
   */
  void DestroyResource() override;

  /**
   * @copydoc Dali::NativeImageInterface::TargetTexture()
   */
  uint32_t TargetTexture() override;

  /**
   * @copydoc Dali::NativeImageInterface::PrepareTexture()
   */
  Dali::NativeImageInterface::PrepareTextureResult PrepareTexture() override;

  /**
   * @copydoc Dali::NativeImageSourceQueue::GetQueueCount()
   */
  uint32_t GetQueueCount() const override
  {
    return mQueueCount;
  }

  /**
   * @copydoc Dali::NativeImageInterface::GetWidth()
   */
  uint32_t GetWidth() const override
  {
    return mWidth;
  }

  /**
   * @copydoc Dali::NativeImageInterface::GetHeight()
   */
  uint32_t GetHeight() const override
  {
    return mHeight;
  }

  /**
   * @copydoc Dali::NativeImageInterface::RequiresBlending()
   */
  bool RequiresBlending() const override
  {
    return mBlendingRequired;
  }

  /**
   * @copydoc Dali::NativeImageInterface::ApplyNativeFragmentShader()
   */
  bool ApplyNativeFragmentShader(std::string& shader, int mask) override;

  /**
   * @copydoc Dali::NativeImageInterface::GetCustomSamplerTypename()
   */
  const char* GetCustomSamplerTypename() const override;

  /**
   * @copydoc Dali::NativeImageInterface::GetTextureTarget()
   */
  int GetTextureTarget() const override;

  /**
   * @copydoc Dali::NativeImageInterface::GetNativeImageHandle()
   */
  Any GetNativeImageHandle() const override;

  /**
   * @copydoc Dali::NativeImageInterface::SourceChanged()
   */
  bool SourceChanged() const override;

  /**
   * @copydoc Dali::NativeImageInterface::GetUpdatedArea()
   */
  Rect<uint32_t> GetUpdatedArea() override
  {
    return Rect<uint32_t>{0, 0, mWidth, mHeight};
  }

  /**
   * @copydoc Dali::NativeImageInterface::PostRender()
   */
  void PostRender() override
  {
  }

  /**
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  NativeImageInterface::Extension* GetNativeImageInterfaceExtension() override
  {
    return reinterpret_cast<NativeImageInterface::Extension*>(static_cast<NativeImageSurfaceReference*>(this));
  }

  // NativeImageSurfaceReference implementation

  /**
   * @copydoc NativeImageSurfaceReference::AcquireSurfaceReference()
   */
  void AcquireSurfaceReference(void* surface) override;

  /**
   * @copydoc NativeImageSurfaceReference::ReleaseSurfaceReference()
   */
  void ReleaseSurfaceReference(void* surface) override;

private:
  /**
   * Private constructor; @see NativeImageSourceQueue::New()
   * @param[in] queueCount The number of queue of the image. If it is 0, will use default.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] nativeImageSourceQueue contains tbm_surface_queue_h or is empty
   */
  NativeImageSourceQueueTizenVulkan(uint32_t queueCount, uint32_t width, uint32_t height, Any nativeImageSourceQueue);

  void Initialize(Dali::NativeImageSourceQueue::ColorFormat colorFormat);

  void ResetSurfaceList(bool releaseConsumeSurface);

  tbm_surface_queue_h GetSurfaceFromAny(Any source) const;

  bool CheckBlending(int format);

private:
  struct SurfaceReferenceData
  {
    std::atomic<int>                      refCount{0};
    std::chrono::steady_clock::time_point lastUsed;
    tbm_surface_h                         surface{nullptr};

    SurfaceReferenceData(tbm_surface_h surf)
    : lastUsed(std::chrono::steady_clock::now()),
      surface(surf)
    {
    }
  };

  struct PendingRelease
  {
    tbm_surface_h                         surface;
    std::chrono::steady_clock::time_point timestamp;
  };

  /**
   * @brief Schedule a surface for delayed release
   * @param surface The surface to be released when safe
   */
  void ScheduleSurfaceForDelayedRelease(tbm_surface_h surface);

  /**
   * @brief Clean up surfaces that are no longer referenced
   */
  void CleanupPendingReleases();

  /**
   * @brief Get or create surface reference entry
   * @param surface The TBM surface handle
   * @return Reference to surface reference data
   */
  struct SurfaceReferenceData& GetOrCreateSurfaceRef(tbm_surface_h surface);

private:
  using SurfaceContainer       = std::vector<tbm_surface_h>;
  using BufferSurfaceContainer = std::unordered_map<uint8_t*, tbm_surface_h>;

  Dali::Mutex            mMutex;            ///< Mutex
  uint32_t               mQueueCount;       ///< queue count
  uint32_t               mWidth;            ///< image width
  uint32_t               mHeight;           ///< image height
  tbm_surface_queue_h    mTbmQueue;         ///< Tbm surface queue handle
  tbm_surface_h          mConsumeSurface;   ///< The current tbm surface
  SurfaceContainer       mSurfaces;         ///< Surface container
  BufferSurfaceContainer mBuffers;          ///< Buffer map
  bool                   mOwnTbmQueue;      ///< Whether we created tbm queue
  bool                   mBlendingRequired; ///< Whether blending is required
  bool                   mIsResized;        ///< Whether the size has changed
  bool                   mFreeRequest;      ///< Whether it is requested to free the released buffers

  // Surface reference management
  std::unordered_map<tbm_surface_h, std::unique_ptr<SurfaceReferenceData>> mSurfaceRefs;
  std::vector<PendingRelease>                                              mPendingRelease;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_TIZEN_VULKAN_H
