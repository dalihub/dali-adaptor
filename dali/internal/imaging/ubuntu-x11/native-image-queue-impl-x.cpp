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

// CLASS HEADER
#include <dali/internal/imaging/ubuntu-x11/native-image-queue-impl-x.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/integration-api/gl-defines.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/graphics/linux/egl-image-extensions-linux-dma-buf.h>

#ifdef DALI_USE_GBM
#include <fcntl.h>
#include <gbm.h>
#include <unistd.h>
#include <string>
#endif

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
namespace
{
constexpr uint32_t DEFAULT_QUEUE_SIZE = 3u;

const char* const SAMPLER_TYPE = "samplerExternalOES";

#ifdef DALI_USE_GBM
/**
 * Maps a queue colour format onto the DRM fourcc with the same byte order.
 *
 * DRM fourccs name their channels in little-endian packing order, so the byte
 * order they describe is the reverse of the name: GBM_FORMAT_ABGR8888 is R,G,B,A
 * in memory. Dali::NativeImageQueue::ColorFormat is documented as following
 * pixel byte order, so the two line up as below.
 *
 * Getting this pairing right is what keeps the producer's readback a per-row
 * copy: glReadPixels(GL_RGBA) writes R,G,B,A, which needs no channel swizzling
 * to land in an ABGR8888 buffer.
 */
uint32_t ToFourcc(Dali::NativeImageQueue::ColorFormat colorFormat)
{
  switch(colorFormat)
  {
    case Dali::NativeImageQueue::ColorFormat::BGRA8888:
    {
      return GBM_FORMAT_ARGB8888;
    }
    case Dali::NativeImageQueue::ColorFormat::BGRX8888:
    case Dali::NativeImageQueue::ColorFormat::BGR888:
    {
      return GBM_FORMAT_XRGB8888;
    }
    case Dali::NativeImageQueue::ColorFormat::RGBA8888:
    {
      return GBM_FORMAT_ABGR8888;
    }
    case Dali::NativeImageQueue::ColorFormat::RGBX8888:
    case Dali::NativeImageQueue::ColorFormat::RGB888:
    default:
    {
      return GBM_FORMAT_XBGR8888;
    }
  }
}
#endif

bool HasAlpha(Dali::NativeImageQueue::ColorFormat colorFormat)
{
  return colorFormat == Dali::NativeImageQueue::ColorFormat::BGRA8888 ||
         colorFormat == Dali::NativeImageQueue::ColorFormat::RGBA8888;
}

bool HasBgraOrder(Dali::NativeImageQueue::ColorFormat colorFormat)
{
  return colorFormat == Dali::NativeImageQueue::ColorFormat::BGR888 ||
         colorFormat == Dali::NativeImageQueue::ColorFormat::BGRA8888 ||
         colorFormat == Dali::NativeImageQueue::ColorFormat::BGRX8888;
}

} // namespace

NativeImageQueueX* NativeImageQueueX::New(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageQueue::ColorFormat colorFormat, Any nativeImageQueue)
{
  NativeImageQueueX* image = new NativeImageQueueX(queueCount, width, height, colorFormat, nativeImageQueue);
  return image;
}

NativeImageQueueX::NativeImageQueueX(uint32_t queueCount, uint32_t width, uint32_t height, Dali::NativeImageQueue::ColorFormat colorFormat, Any nativeImageQueue)
: mQueueCount(queueCount == 0u ? DEFAULT_QUEUE_SIZE : queueCount),
  mWidth(width),
  mHeight(height),
  mColorFormat(colorFormat)
{
#ifdef DALI_USE_GBM
  if(!nativeImageQueue.Empty())
  {
    // Wrapping a queue the caller already owns has no meaning here: there is no
    // platform queue type to wrap, only this class's own dma_buf ring.
    DALI_LOG_ERROR("NativeImageQueueX: cannot adopt an existing native image queue\n");
    return;
  }

  mFourcc = ToFourcc(colorFormat);

  // Taken here rather than in CreateResource(), which runs on the render thread.
  if(Dali::Adaptor::IsAvailable())
  {
    auto graphics      = &(Adaptor::GetImplementation(Adaptor::Get()).GetGraphicsInterface());
    mEglGraphics       = static_cast<EglGraphics*>(graphics);
    mEglImplementation = &mEglGraphics->GetEglImplementation();
  }
#else
  DALI_LOG_ERROR("NativeImageQueueX: built without GBM, offscreen rendering is unavailable\n");
#endif
}

NativeImageQueueX::~NativeImageQueueX()
{
#ifdef DALI_USE_GBM
  // Releasing the slots releases their buffers; their EGLImages went earlier, in
  // DestroyResource(). The GBM device has to outlive them all, hence the order.
  mSlots.clear();
  mRetiredSlots.clear();
  mConsumeSlot.reset();

  if(mGbmDevice)
  {
    gbm_device_destroy(mGbmDevice);
    mGbmDevice = nullptr;
  }

  if(mDrmFd >= 0)
  {
    ::close(mDrmFd);
    mDrmFd = -1;
  }
#endif
}

// ---------------------------------------------------------------------------
// Ring management
// ---------------------------------------------------------------------------

NativeImageQueueX::Slot::~Slot()
{
#ifdef DALI_USE_GBM
  if(map)
  {
    gbm_bo_unmap(bo, mapData);
  }

  if(fd >= 0)
  {
    ::close(fd);
  }

  if(bo)
  {
    gbm_bo_destroy(bo);
  }
#endif
}

bool NativeImageQueueX::EnsureRing()
{
#ifdef DALI_USE_GBM
  if(mRingReady)
  {
    return true;
  }

  if(mRingUnavailable)
  {
    return false;
  }

  // Resolved when DALi's graphics came up, so this is a read rather than an EGL
  // call and is safe from whichever thread arrives first. Empty means graphics
  // is not up yet, which is worth another look on the next frame.
  const std::string renderNode = EglImageExtensionsLinuxDmaBuf::GetRenderNodePath();
  if(renderNode.empty())
  {
    return false;
  }

  // Past this point a failure is a property of the machine, so do not retry.
  mRingUnavailable = true;

  mDrmFd = ::open(renderNode.c_str(), O_RDWR | O_CLOEXEC);
  if(mDrmFd < 0)
  {
    DALI_LOG_ERROR("NativeImageQueueX: could not open %s, offscreen rendering is unavailable\n", renderNode.c_str());
    return false;
  }

  mGbmDevice = gbm_create_device(mDrmFd);
  if(!mGbmDevice)
  {
    DALI_LOG_ERROR("NativeImageQueueX: gbm_create_device failed for %s\n", renderNode.c_str());
    ::close(mDrmFd);
    mDrmFd = -1;
    return false;
  }

  if(!CreateSlots(mWidth, mHeight))
  {
    DALI_LOG_ERROR("NativeImageQueueX: could not allocate a %ux%u dma_buf ring\n", mWidth, mHeight);
    return false;
  }

  mRingUnavailable = false;
  mRingReady       = true;
  return true;
#else
  return false;
#endif
}

bool NativeImageQueueX::CreateSlots(uint32_t width, uint32_t height)
{
#ifdef DALI_USE_GBM
  if(!mGbmDevice || width == 0u || height == 0u)
  {
    return false;
  }

  std::vector<SlotPtr> slots;
  slots.reserve(mQueueCount);

  for(uint32_t i = 0u; i < mQueueCount; ++i)
  {
    SlotPtr slot = std::make_shared<Slot>();

    // GBM_BO_USE_LINEAR is what makes the buffer CPU mappable with a predictable
    // row layout. GBM_BO_USE_RENDERING is deliberately not requested: it makes
    // allocation fail on some drivers, and nothing renders into this buffer
    // through GL - the producer writes it with the CPU.
    slot->bo = gbm_bo_create(mGbmDevice, width, height, mFourcc, GBM_BO_USE_LINEAR);
    if(!slot->bo)
    {
      DALI_LOG_ERROR("NativeImageQueueX: gbm_bo_create failed for %ux%u fourcc[0x%x]\n", width, height, mFourcc);
      break;
    }

    slot->fd = gbm_bo_get_fd(slot->bo);
    if(slot->fd < 0)
    {
      DALI_LOG_ERROR("NativeImageQueueX: gbm_bo_get_fd failed\n");
      break;
    }

    // Mapped once and kept mapped: mapping and unmapping around every frame
    // costs roughly twice as much as the copy itself.
    uint32_t mapStride = 0u;
    void*    map       = gbm_bo_map(slot->bo, 0, 0, width, height, GBM_BO_TRANSFER_WRITE, &mapStride, &slot->mapData);
    if(!map)
    {
      DALI_LOG_ERROR("NativeImageQueueX: gbm_bo_map failed\n");
      break;
    }

    slot->map = static_cast<uint8_t*>(map);
    // Two strides, and they are not interchangeable: the producer writes through
    // the mapping, while the EGLImage import describes the dma_buf itself.
    slot->mapStride    = mapStride;
    slot->bufferStride = gbm_bo_get_stride(slot->bo);
    slot->width        = width;
    slot->height       = height;
    slot->state        = SlotState::FREE;

    slots.push_back(std::move(slot));
  }

  if(slots.size() != mQueueCount)
  {
    // Whatever was built is released as the vector goes out of scope.
    return false;
  }

  mSlots = std::move(slots);
  return true;
#else
  return false;
#endif
}

void NativeImageQueueX::DestroySlotImage(Slot& slot)
{
  if(slot.eglImage && mEglImplementation)
  {
    EglImageExtensionsLinuxDmaBuf::DestroyImage(*mEglImplementation, slot.eglImage);
  }
  slot.eglImage = nullptr;
}

void NativeImageQueueX::DestroyRetiredSlots()
{
  // A slot the producer still has checked out must outlive this: it writes into
  // the mapping with the mutex released, so it would be writing into freed
  // memory. Leave those for a later call, once the producer has handed them back.
  for(auto iter = mRetiredSlots.begin(); iter != mRetiredSlots.end();)
  {
    if((*iter)->state == SlotState::DEQUEUED)
    {
      ++iter;
      continue;
    }

    DestroySlotImage(**iter);
    iter = mRetiredSlots.erase(iter); ///< Drops the last reference, freeing the buffer
  }
}

NativeImageQueueX::SlotPtr NativeImageQueueX::FindSlot(const uint8_t* data)
{
  for(auto&& slot : mSlots)
  {
    if(slot->map == data)
    {
      return slot;
    }
  }
  return nullptr;
}

NativeImageQueueX::SlotPtr NativeImageQueueX::FindRetiredSlot(const uint8_t* data)
{
  for(auto&& slot : mRetiredSlots)
  {
    if(slot->map == data)
    {
      return slot;
    }
  }
  return nullptr;
}

// ---------------------------------------------------------------------------
// NativeImageQueue
// ---------------------------------------------------------------------------

Any NativeImageQueueX::GetNativeImageQueue() const
{
#ifndef DALI_USE_GBM
  return Any();
#else
  // The producer channel. Where EGL drives the queue itself this call yields the
  // platform queue object; there is none here, so the surface is handed the
  // interface it must drive by hand instead.
  return Any(static_cast<NativeImageQueueProducerX*>(const_cast<NativeImageQueueX*>(this)));
#endif
}

void NativeImageQueueX::SetSize(uint32_t width, uint32_t height)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(mWidth == width && mHeight == height)
  {
    return;
  }

  mWidth  = width;
  mHeight = height;

  if(!mRingReady)
  {
    // Nothing to reallocate; EnsureRing() will build the ring at this size.
    return;
  }

  // The ring cannot be resized in place, so retire it and build a new one. The
  // slot the consumer is sampling is deliberately not retired here: it is still
  // bound to a live texture, and mConsumeSlot keeps it alive until PrepareTexture
  // moves off it. Retired slots are freed on the render thread, the only thread
  // allowed to destroy their EGLImages.
  for(auto&& slot : mSlots)
  {
    if(slot != mConsumeSlot)
    {
      mRetiredSlots.push_back(slot);
    }
  }
  mSlots.clear();

  if(!CreateSlots(width, height))
  {
    DALI_LOG_ERROR("NativeImageQueueX: could not reallocate the ring at %ux%u\n", width, height);
    mRingReady       = false;
    mRingUnavailable = true;
  }
}

void NativeImageQueueX::IgnoreSourceImage()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  // Drops a single frame, the oldest waiting one, leaving anything newer to be
  // acquired as usual.
  SlotPtr oldest;
  for(auto&& slot : mSlots)
  {
    if(slot->state == SlotState::READY && (!oldest || slot->serial < oldest->serial))
    {
      oldest = slot;
    }
  }

  if(oldest)
  {
    oldest->state = SlotState::FREE;
  }
}

bool NativeImageQueueX::CanDequeueBuffer()
{
  return CanDequeue();
}

uint8_t* NativeImageQueueX::DequeueBuffer(uint32_t& width, uint32_t& height, uint32_t& stride, Dali::NativeImageQueue::BufferAccessType type)
{
  Buffer buffer = Dequeue();
  if(!buffer.IsValid())
  {
    return nullptr;
  }

  width  = buffer.width;
  height = buffer.height;
  stride = buffer.stride;
  return buffer.data;
}

bool NativeImageQueueX::EnqueueBuffer(uint8_t* buffer)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  SlotPtr slot = FindSlot(buffer);
  if(!slot)
  {
    // A resize took this slot out of the ring while the producer was writing
    // into it. The frame is the wrong size now, so drop it and let the slot be
    // released; marking it not-dequeued is what allows that.
    slot = FindRetiredSlot(buffer);
    if(slot && slot->state == SlotState::DEQUEUED)
    {
      slot->state = SlotState::FREE;
    }
    return false;
  }

  if(slot->state != SlotState::DEQUEUED)
  {
    return false;
  }

  slot->state  = SlotState::READY;
  slot->serial = ++mEnqueueSerial;
  return true;
}

void NativeImageQueueX::CancelDequeuedBuffer(uint8_t* buffer)
{
  Dali::Mutex::ScopedLock lock(mMutex);

  SlotPtr slot = FindSlot(buffer);
  if(!slot)
  {
    // Retired by a resize while checked out; releasing it is all that is left.
    slot = FindRetiredSlot(buffer);
  }

  if(slot && slot->state == SlotState::DEQUEUED)
  {
    slot->state = SlotState::FREE;
  }
}

void NativeImageQueueX::FreeReleasedBuffers()
{
  // Buffers are allocated once per ring and reused, so there is nothing to free
  // until the ring itself is replaced.
}

// ---------------------------------------------------------------------------
// NativeImageQueueProducerX
// ---------------------------------------------------------------------------

bool NativeImageQueueX::CanDequeue()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(!EnsureRing())
  {
    return false;
  }

  for(auto&& slot : mSlots)
  {
    if(slot->state == SlotState::FREE)
    {
      return true;
    }
  }
  return false;
}

NativeImageQueueX::Buffer NativeImageQueueX::Dequeue()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(!EnsureRing())
  {
    return Buffer{};
  }

  for(uint32_t i = 0u; i < mSlots.size(); ++i)
  {
    SlotPtr& slot = mSlots[i];
    if(slot->state != SlotState::FREE)
    {
      continue;
    }

    slot->state = SlotState::DEQUEUED;

    Buffer buffer;
    buffer.data         = slot->map;
    buffer.stride       = slot->mapStride;
    buffer.width        = slot->width;
    buffer.height       = slot->height;
    buffer.index        = static_cast<int32_t>(i);
    buffer.channelOrder = HasBgraOrder(mColorFormat) ? ChannelOrder::BGRA : ChannelOrder::RGBA;
    return buffer;
  }

  return Buffer{};
}

void NativeImageQueueX::Enqueue(const Buffer& buffer)
{
  EnqueueBuffer(buffer.data);
}

void NativeImageQueueX::CancelDequeued(const Buffer& buffer)
{
  CancelDequeuedBuffer(buffer.data);
}

// ---------------------------------------------------------------------------
// NativeImageInterface - consumer side, render thread
// ---------------------------------------------------------------------------

bool NativeImageQueueX::CreateResource()
{
  if(!mEglImplementation || !EglImageExtensionsLinuxDmaBuf::IsSupported(*mEglImplementation))
  {
    return false;
  }

  Dali::Mutex::ScopedLock lock(mMutex);
  return EnsureRing();
}

void NativeImageQueueX::DestroyResource()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  for(auto&& slot : mSlots)
  {
    DestroySlotImage(*slot);
  }

  if(mConsumeSlot)
  {
    DestroySlotImage(*mConsumeSlot);
    mConsumeSlot.reset();
  }

  DestroyRetiredSlots();

  mImageState = ImageState::INITIALIZED;
}

Dali::NativeImageInterface::PrepareTextureResult NativeImageQueueX::PrepareTexture()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(!mRingReady)
  {
    return Dali::NativeImageInterface::PrepareTextureResult::NOT_INITIALIZED_IMAGE;
  }

  // The render thread is the only place the retired slots may be released.
  DestroyRetiredSlots();

  // Take the newest frame the producer has published, freeing the older ones so
  // the producer is not starved when it runs faster than the UI.
  SlotPtr newest;
  for(auto&& slot : mSlots)
  {
    if(slot->state != SlotState::READY)
    {
      continue;
    }

    if(!newest || slot->serial > newest->serial)
    {
      if(newest)
      {
        newest->state = SlotState::FREE;
      }
      newest = slot;
    }
    else
    {
      slot->state = SlotState::FREE;
    }
  }

  if(!newest)
  {
    mImageState = ImageState::NOT_CHANGED;
    return mConsumeSlot ? Dali::NativeImageInterface::PrepareTextureResult::NO_ERROR
                        : Dali::NativeImageInterface::PrepareTextureResult::NOT_INITIALIZED_IMAGE;
  }

  if(mConsumeSlot && mConsumeSlot != newest)
  {
    // Whatever we were sampling goes back to the producer, unless a resize took
    // it out of the ring, in which case it is retired for release below.
    mConsumeSlot->state = SlotState::FREE;
    if(!FindSlot(mConsumeSlot->map))
    {
      // A resize took it out of the ring, so it is ours to release rather than
      // the producer's to reuse.
      mRetiredSlots.push_back(mConsumeSlot);
    }
  }

  newest->state = SlotState::CONSUMED;
  mConsumeSlot  = newest;
  mImageState   = ImageState::CHANGED;

  return Dali::NativeImageInterface::PrepareTextureResult::IMAGE_CHANGED;
}

uint32_t NativeImageQueueX::TargetTexture()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  if(!mEglImplementation)
  {
    return 1u; // error
  }

  // Called once while the texture is created, before any frame has been
  // acquired, and then once per frame. Nothing to attach in the former case.
  if(mImageState != ImageState::CHANGED || !mConsumeSlot)
  {
    return 0u;
  }

  if(!mConsumeSlot->eglImage)
  {
    DmaBufDescriptor descriptor;
    descriptor.fd     = mConsumeSlot->fd;
    descriptor.width  = mConsumeSlot->width;
    descriptor.height = mConsumeSlot->height;
    descriptor.fourcc = mFourcc;
    // The importer wants the dma_buf's own pitch, not the mapping's.
    descriptor.stride = mConsumeSlot->bufferStride;
    descriptor.offset = 0u;

    mConsumeSlot->eglImage = EglImageExtensionsLinuxDmaBuf::CreateImage(*mEglImplementation, descriptor);
    if(!mConsumeSlot->eglImage)
    {
      // Nothing to attach, so the previous frame stays on screen. Reporting an
      // error here would have DALi destroy a texture that is otherwise fine.
      DALI_LOG_ERROR("NativeImageQueueX: could not import the dma_buf as an EGLImage\n");
      return 0u;
    }
  }

  // Must match GetTextureTarget(): the caller has GL_TEXTURE_EXTERNAL_OES bound.
  EglImageExtensionsLinuxDmaBuf::TargetExternalTexture(mConsumeSlot->eglImage);

  return 0u;
}

void NativeImageQueueX::PostRender()
{
  Dali::Mutex::ScopedLock lock(mMutex);

  // Nothing to synchronise: the producer finishes its GPU work before it
  // enqueues, so a published buffer is already complete.
  mImageState = ImageState::INITIALIZED;
}

// ---------------------------------------------------------------------------
// NativeImageInterface - description
// ---------------------------------------------------------------------------

bool NativeImageQueueX::RequiresBlending() const
{
  return HasAlpha(mColorFormat);
}

bool NativeImageQueueX::ApplyNativeFragmentShader(std::string& shader, int mask)
{
  if(!mEglGraphics)
  {
    return false;
  }
  return mEglGraphics->ApplyNativeFragmentShader(shader, SAMPLER_TYPE, mask);
}

const char* NativeImageQueueX::GetCustomSamplerTypename() const
{
  return SAMPLER_TYPE;
}

int NativeImageQueueX::GetTextureTarget() const
{
  return GL_TEXTURE_EXTERNAL_OES;
}

Any NativeImageQueueX::GetNativeImageHandle() const
{
  return nullptr;
}

bool NativeImageQueueX::SourceChanged() const
{
  return true;
}

} // namespace Adaptor

} // namespace Internal

} //namespace DALI_NAMESPACE
