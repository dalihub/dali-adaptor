#ifndef DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_TIZEN_H
#define DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_TIZEN_H

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
#include <dali/devel-api/threading/mutex.h>
#include <tbm_surface.h>
#include <tbm_surface_queue.h>
#include <unordered_map>
#include <utility>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles-impl/gles-sync-pool.h>
#include <dali/internal/imaging/common/native-image-source-queue-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class EglGraphics;
class EglImageExtensions;

/**
 * Dali internal NativeImageSource.
 */
class NativeImageSourceQueueTizen : public Internal::Adaptor::NativeImageSourceQueue
{
public:
  /**
   * Create a new NativeImageSourceQueueTizen internally.
   * Depending on hardware the width and height may have to be a power of two.
   * @param[in] queueCount The number of queue of the image. If it is 0, will use default.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colorFormat The color format of the image.
   * @param[in] nativeImageSourceQueue contains tbm_surface_queue_h or is empty
   * @return A smart-pointer to a newly allocated image.
   */
  static NativeImageSourceQueueTizen* New(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageSourceQueue::ColorFormat colorFormat, Any nativeImageSourceQueue);

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
  void SetQueueUsageHint(Dali::NativeImageSourceQueue::QueueUsageType type) override;

  /**
   * destructor
   */
  ~NativeImageSourceQueueTizen() override;

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
  void PostRender() override;

  /**
   * @copydoc Dali::NativeImageInterface::GetExtension()
   */
  NativeImageInterface::Extension* GetNativeImageInterfaceExtension() override
  {
    return nullptr;
  }

private:
  /**
   * Private constructor; @see NativeImageSourceQueue::New()
   * @param[in] queueCount The number of queue of the image. If it is 0, will use default.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] nativeImageSourceQueue contains tbm_surface_queue_h or is empty
   */
  NativeImageSourceQueueTizen(uint32_t queueCount, uint32_t width, uint32_t height, Any nativeImageSourceQueue);

  void Initialize(Dali::NativeImageSourceQueue::ColorFormat colorFormat);

  void ResetEglImageList(bool releaseConsumeSurface);

  tbm_surface_queue_h GetSurfaceFromAny(Any source) const;

  bool CheckBlending(int format);

  /**
   * @brief Creates a synchronization object for GPU-CPU synchronization.
   *
   * This method creates a sync object that can be used to synchronize between
   * GPU and CPU operations. The sync object ensures that GPU operations have
   * completed before CPU operations proceed.
   * Note: The created sync object should be used by implementing classes to provide
   * wait functionality that ensures proper synchronization between GPU and CPU.
   *
   * @return true if the sync object was created successfully, false otherwise
   */
  bool CreateSyncObject();

private:
  enum class ImageState : uint8_t
  {
    INITIALIZED,
    NOT_CHANGED,
    CHANGED
  };

  using SurfaceEglContainer    = std::unordered_map<tbm_surface_h, void*>;
  using BufferSurfaceContainer = std::unordered_map<uint8_t*, tbm_surface_h>;
  using SyncObjectContainer    = std::unordered_map<tbm_surface_h, std::shared_ptr<Graphics::GLES::SyncPool::SharedSyncObject>>;

  Dali::Mutex            mMutex;              ///< Mutex
  uint32_t               mQueueCount;         ///< queue count
  uint32_t               mWidth;              ///< image width
  uint32_t               mHeight;             ///< image height
  tbm_surface_queue_h    mTbmQueue;           ///< Tbm surface queue handle
  tbm_surface_h          mConsumeSurface;     ///< The current tbm surface
  tbm_surface_h          mOldSurface;         ///< The old surface to be released
  SurfaceEglContainer    mEglImages;          ///< EGL Image map
  BufferSurfaceContainer mBuffers;            ///< Buffer map
  SyncObjectContainer    mSyncObjects;        ///< Sync object map
  EglGraphics*           mEglGraphics;        ///< EGL Graphics
  EglImageExtensions*    mEglImageExtensions; ///< The EGL Image Extensions

  Dali::Graphics::GLES::SyncPool* mSyncPool; ///< Sync Pool

  ImageState mImageState;         ///< Image state
  bool       mOwnTbmQueue;        ///< Whether we created tbm queue
  bool       mBlendingRequired;   ///< Whether blending is required
  bool       mIsResized;          ///< Whether the size has changed
  bool       mFreeRequest;        ///< Whether it is requested to free the released buffers
  bool       mNeedSync;           ///< Whether we need to create the egl sync object
  bool       mWaitInWorkerThread; ///< Whether we can wait for the sync to be signaled in the worker thread
  bool       mRendered;           ///< Whether this texture is rendered in this frame
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_NATIVE_IMAGE_SOURCE_QUEUE_IMPL_TIZEN_H
