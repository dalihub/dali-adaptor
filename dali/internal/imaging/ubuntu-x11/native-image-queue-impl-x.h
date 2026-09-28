#ifndef DALI_INTERNAL_NATIVE_IMAGE_QUEUE_IMPL_X_H
#define DALI_INTERNAL_NATIVE_IMAGE_QUEUE_IMPL_X_H

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
#include <cstdint>
#include <memory>
#include <vector>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/native-image-queue-impl.h>
#include <dali/internal/imaging/ubuntu-x11/native-image-queue-producer-x.h>

struct gbm_bo;
struct gbm_device;

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
class EglGraphics;
class EglImplementation;

/**
 * Dali internal NativeImageQueue.
 *
 * No EGL here will accept a buffer queue as a native window, or turn one into an
 * EGLImage. What it does have is dma_buf, so the queue is a ring of CPU-mappable
 * dma_bufs allocated through GBM. The producer writes a frame into the mapping
 * (see NativeImageQueueProducerX) and the consumer imports the same dma_buf as
 * an EGLImage, which lands on GL_TEXTURE_EXTERNAL_OES - the target a native
 * image queue is expected to use, so the shader DALi generates for it is the
 * same one it generates on a device.
 *
 * Requires a DRM render node and EGL_EXT_image_dma_buf_import. Where either is
 * missing the queue reports itself unsupported rather than substituting a
 * mechanism that would behave differently from a device.
 */
class NativeImageQueueX : public Internal::Adaptor::NativeImageQueue,
                          public Internal::Adaptor::NativeImageQueueProducerX
{
public:
  /**
   * Create a new NativeImageQueueX internally.
   * Depending on hardware the width and height may have to be a power of two.
   * @param[in] queueCount The number of queue of the image. If it is 0, will use default.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colorFormat The color format of the image.
   * @param[in] nativeImageQueue Must be empty; there is no platform queue type to adopt here
   * @return A smart-pointer to a newly allocated image.
   */
  static NativeImageQueueX* New(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageQueue::ColorFormat colorFormat, Any nativeImageQueue);

  /**
   * @copydoc Dali::NativeImageQueue::GetNativeImageQueue()
   */
  Any GetNativeImageQueue() const override;

  /**
   * @copydoc Dali::NativeImageQueue::SetSize
   */
  void SetSize(uint32_t width, uint32_t height) override;

  /**
   * @copydoc Dali::NativeImageQueue::IgnoreSourceImage
   */
  void IgnoreSourceImage() override;

  /**
   * @copydoc Dali::NativeImageQueue::CanDequeueBuffer
   */
  bool CanDequeueBuffer() override;

  /**
   * @copydoc Dali::NativeImageQueue::DequeueBuffer
   */
  uint8_t* DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride, Dali::NativeImageQueue::BufferAccessType type) override;

  /**
   * @copydoc Dali::NativeImageQueue::EnqueueBuffer
   */
  bool EnqueueBuffer(uint8_t* buffer) override;

  /**
   * @copydoc Dali::NativeImageQueue::CancelDequeuedBuffer
   */
  void CancelDequeuedBuffer(uint8_t* buffer) override;

  /**
   * @copydoc Dali::NativeImageQueue::EnqueueBuffer
   */
  void FreeReleasedBuffers() override;

  /**
   * @copydoc Dali::NativeImageQueue::SetQueueUsageHint
   */
  void SetQueueUsageHint(Dali::NativeImageQueue::QueueUsageType type) override
  {
  }

  /**
   * destructor
   */
  ~NativeImageQueueX() override;

  /**
   * @copydoc Dali::NativeImageInterface::CreateResource()
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
   * @copydoc Dali::NativeImageQueue::GetQueueCount
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
  bool RequiresBlending() const override;

  /**
   * @copydoc Dali::NativeImageInterface::GetTextureTarget()
   */
  int GetTextureTarget() const override;

  /**
   * @copydoc Dali::NativeImageInterface::ApplyNativeFragmentShader()
   */
  bool ApplyNativeFragmentShader(std::string& shader, int mask) override;

  /**
   * @copydoc Dali::NativeImageInterface::GetCustomSamplerTypename()
   */
  const char* GetCustomSamplerTypename() const override;

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

public: // From NativeImageQueueProducerX
  /**
   * @copydoc Dali::Internal::Adaptor::NativeImageQueueProducerX::CanDequeue()
   */
  bool CanDequeue() override;

  /**
   * @copydoc Dali::Internal::Adaptor::NativeImageQueueProducerX::Dequeue()
   */
  Buffer Dequeue() override;

  /**
   * @copydoc Dali::Internal::Adaptor::NativeImageQueueProducerX::Enqueue()
   */
  void Enqueue(const Buffer& buffer) override;

  /**
   * @copydoc Dali::Internal::Adaptor::NativeImageQueueProducerX::CancelDequeued()
   */
  void CancelDequeued(const Buffer& buffer) override;

private:
  /**
   * Private constructor; @see NativeImageQueue::New()
   * @param[in] queueCount The number of queue of the image. If it is 0, will use default.
   * @param[in] width The width of the image.
   * @param[in] height The height of the image.
   * @param[in] colorFormat The color format of the image.
   * @param[in] nativeImageQueue Must be empty; there is no platform queue type to adopt here
   */
  NativeImageQueueX(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageQueue::ColorFormat colorFormat, Any nativeImageQueue);

  NativeImageQueueX(const NativeImageQueueX&)            = delete;
  NativeImageQueueX& operator=(const NativeImageQueueX&) = delete;

private:
  /// Where a buffer sits between the producer and the consumer.
  enum class SlotState : uint8_t
  {
    FREE,     ///< Nothing holds it; the producer may take it
    DEQUEUED, ///< The producer is writing into it
    READY,    ///< Written and waiting to be acquired
    CONSUMED  ///< The consumer is sampling it
  };

  /// What TargetTexture() is driven off.
  enum class ImageState : uint8_t
  {
    INITIALIZED,
    NOT_CHANGED,
    CHANGED
  };

  /**
   * One ring buffer: a dma_buf, its persistent CPU mapping, and its EGLImage.
   *
   * The destructor releases the buffer, its descriptor and its mapping, none of
   * which care which thread they are freed on. The EGLImage does - only the
   * render thread may destroy one - so it is released explicitly beforehand, by
   * DestroySlotImage().
   */
  struct Slot
  {
    Slot() = default;
    ~Slot();

    Slot(const Slot&)            = delete;
    Slot& operator=(const Slot&) = delete;

    gbm_bo*   bo{nullptr};
    int       fd{-1};           ///< dma_buf descriptor, owned here
    uint8_t*  map{nullptr};     ///< Persistent mapping. Re-mapping per frame costs about twice as much.
    void*     mapData{nullptr}; ///< GBM's opaque mapping cookie
    void*     eglImage{nullptr};
    uint32_t  mapStride{0};    ///< Bytes per row of the CPU mapping, for the producer
    uint32_t  bufferStride{0}; ///< Bytes per row of the dma_buf, for the EGLImage import
    uint32_t  width{0};
    uint32_t  height{0};
    SlotState state{SlotState::FREE};
    uint64_t  serial{0}; ///< Enqueue order, so the consumer can find the newest
  };

  /// Held by shared_ptr so the slot the consumer is sampling survives a resize
  /// that replaces the whole ring.
  using SlotPtr = std::shared_ptr<Slot>;

  /**
   * @brief Brings up the GBM device and the ring, once.
   *
   * Deferred rather than done in the constructor because it needs the render
   * node EGL renders with, and EGL is initialized on the render thread - which
   * is not ordered against this object being built. Whichever of the producer
   * and the consumer gets here first once that has happened does the work.
   *
   * Not retried after a failure: nothing it needs becomes available later.
   *
   * @return True if the ring is ready to use
   *
   * @note Called under mMutex.
   */
  bool EnsureRing();

  /**
   * @brief Allocates a fresh ring at the given size.
   *
   * @return True if every slot could be allocated and mapped
   *
   * @note Needs exclusive access to the ring, so callers hold mMutex.
   */
  bool CreateSlots(uint32_t width, uint32_t height);

  /**
   * @brief Finds the ring slot a producer buffer belongs to. Called under mMutex.
   *
   * Matched on the mapping address rather than the index, so a buffer dequeued
   * before a resize is simply not found instead of aliasing a new slot.
   *
   * @return The slot, or nullptr if it is no longer part of the ring
   */
  SlotPtr FindSlot(const uint8_t* data);

  /**
   * @brief Finds a retired slot by its mapping address. Called under mMutex.
   *
   * A resize can retire a slot the producer is still writing into, and that slot
   * has to be found again when the producer hands it back.
   *
   * @return The slot, or nullptr if no retired slot has that mapping
   */
  SlotPtr FindRetiredSlot(const uint8_t* data);

  /**
   * @brief Releases a slot's EGLImage, if it has one.
   *
   * Everything else the slot owns goes with ~Slot().
   *
   * @note Render thread only: destroying an EGLImage needs the graphics side.
   */
  void DestroySlotImage(Slot& slot);

  /**
   * @brief Releases the slots that a resize or a release took out of the ring.
   *
   * Slots the producer still holds are left alone: it writes into the mapping
   * without the mutex held, so freeing one now would pull the memory out from
   * under it. They are released on a later call, once handed back.
   *
   * @note Render thread only, and called under mMutex.
   */
  void DestroyRetiredSlots();

private:
  uint32_t                            mQueueCount; ///< queue count
  uint32_t                            mWidth;      ///< image width
  uint32_t                            mHeight;     ///< image height
  Dali::NativeImageQueue::ColorFormat mColorFormat;
  uint32_t                            mFourcc{0}; ///< DRM fourcc matching mColorFormat

  mutable Dali::Mutex mMutex; ///< Guards the ring against concurrent producer access

  int         mDrmFd{-1};
  gbm_device* mGbmDevice{nullptr};

  /// The three hold disjoint sets of slots, which is what lets a slot be freed
  /// as soon as it appears in mRetiredSlots and nowhere else:
  ///
  ///  - mSlots is the ring, and the only place the producer dequeues from. A
  ///    resize replaces it wholesale.
  ///  - mConsumeSlot is the slot the consumer samples. Normally it is also in
  ///    mSlots; a resize drops it from the ring but not from here, because its
  ///    EGLImage is still bound to a texture.
  ///  - mRetiredSlots is what is left over: out of the ring, not being sampled,
  ///    and waiting only for the producer to hand it back if it still holds it.
  std::vector<SlotPtr> mSlots;
  SlotPtr              mConsumeSlot;
  std::vector<SlotPtr> mRetiredSlots;

  EglGraphics*       mEglGraphics{nullptr};
  EglImplementation* mEglImplementation{nullptr}; ///< Passed to EglImageExtensionsLinuxDmaBuf, which holds no instance

  uint64_t   mEnqueueSerial{0};
  ImageState mImageState{ImageState::INITIALIZED};
  bool       mRingReady{false};       ///< Whether EnsureRing() has succeeded
  bool       mRingUnavailable{false}; ///< Whether it has failed, so it is not retried
};

} // namespace Adaptor

} // namespace Internal

} //namespace DALI_NAMESPACE

#endif // DALI_INTERNAL_NATIVE_IMAGE_QUEUE_IMPL_X_H
