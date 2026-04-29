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
#include <dali/internal/vector-animation/tizen/vector-animation-renderer-native-tizen.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/native-image-queue.h>
#include <dali/internal/vector-animation/common/vector-animation-renderer-event-manager.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/rendering/texture-set.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gVectorAnimationLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VECTOR_ANIMATION");
#endif
} // unnamed namespace
class VectorAnimationRendererNativeTizen::RenderingDataImpl : public VectorAnimationRendererNative::RenderingData
{
public:
  Dali::NativeImageQueuePtr mTargetSurface;
};

VectorAnimationRendererNativeTizen::VectorAnimationRendererNativeTizen()
{
  Initialize();
}

VectorAnimationRendererNativeTizen::~VectorAnimationRendererNativeTizen()
{
  Finalize();
}

void VectorAnimationRendererNativeTizen::OnFinalize()
{
  mRenderedTexture.Reset();
  mPreviousTextures.clear();
}

void VectorAnimationRendererNativeTizen::OnNotify()
{
  mPreviousTextures.clear();
}

std::shared_ptr<VectorAnimationRendererNative::RenderingData> VectorAnimationRendererNativeTizen::CreateRenderingData()
{
  return std::make_shared<RenderingDataImpl>();
}

void VectorAnimationRendererNativeTizen::PrepareTarget(std::shared_ptr<RenderingData> renderingData)
{
  auto renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(renderingData);
  renderingDataImpl->mTargetSurface = Dali::NativeImageQueue::New(
    renderingDataImpl->mWidth, renderingDataImpl->mHeight,
    Dali::NativeImageQueue::ColorFormat::BGRA8888);

  if(renderingDataImpl->mTargetSurface)
  {
    renderingDataImpl->mTexture = Dali::Texture::New(*renderingDataImpl->mTargetSurface);
    renderingDataImpl->mTargetSurface->SetQueueUsageHint(Dali::NativeImageQueue::QueueUsageType::ENQUEUE_DEQUEUE);
  }
}

bool VectorAnimationRendererNativeTizen::IsTargetPrepared()
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl;
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(mPreparedRenderingData ? mPreparedRenderingData : mCurrentRenderingData);
    }
  }
  return (renderingDataImpl && renderingDataImpl->mTargetSurface);
}

Dali::Texture VectorAnimationRendererNativeTizen::GetTargetTexture()
{
  return mRenderedTexture;
}

bool VectorAnimationRendererNativeTizen::Render(uint32_t frameNumber)
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl;

  bool resourceChanged = false;
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      if(mPreparedRenderingData)
      {
        mPreviousRenderingData.push_back(mCurrentRenderingData);
        mCurrentRenderingData = std::move(mPreparedRenderingData);
        mPreparedRenderingData.reset();
        resourceChanged = true;
      }
      renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(mCurrentRenderingData);
    }
  }

  if(!renderingDataImpl)
  {
    return false;
  }

  {
    Dali::Mutex::ScopedLock lock(mMutex);

    UpdateSizeIfNeeded();

    if(DALI_UNLIKELY(mFinalized))
    {
      return false;
    }

    if(resourceChanged)
    {
      mResourceReady = false;
    }

    if(mEnableFixedCache)
    {
      if(resourceChanged || mDecodedBuffers.size() < mTotalFrame)
      {
        mDecodedBuffers.clear();
        mDecodedBuffers.resize(mTotalFrame, std::make_pair(std::vector<uint8_t>(), false));
      }
    }

    if(!mCanvas || !mAnimation || !renderingDataImpl->mTargetSurface)
    {
      return false;
    }

    // Try to dequeue TBM buffer. If unavailable, skip this frame (like rlottie plugin).
    if(!renderingDataImpl->mTargetSurface->CanDequeueBuffer())
      {
        renderingDataImpl->mTargetSurface->IgnoreSourceImage();

        if(!renderingDataImpl->mTargetSurface->CanDequeueBuffer())
        {
          return false;
        }
      }

    Dali::NativeImageQueue::BufferAccessType type;
    if(mEnableFixedCache && (frameNumber < mDecodedBuffers.size()) && (!mDecodedBuffers[frameNumber].second))
    {
      type = Dali::NativeImageQueue::BufferAccessType::READ | Dali::NativeImageQueue::BufferAccessType::WRITE;
    }
    else
    {
      type = Dali::NativeImageQueue::BufferAccessType::WRITE;
    }

    uint32_t width, height, stride;
    uint8_t* buffer = renderingDataImpl->mTargetSurface->DequeueBuffer(width, height, stride, type);
    if(!buffer)
    {
      DALI_LOG_ERROR("DequeueBuffer failed [%p]\n", this);
      return false;
    }

    if(width != renderingDataImpl->mWidth || height != renderingDataImpl->mHeight)
    {
      DALI_LOG_ERROR("VectorAnimationRendererTizen::Render: Invalid buffer! [%d, %d, %p] [%p]\n", width, height, buffer, this);
      renderingDataImpl->mTargetSurface->CancelDequeuedBuffer(buffer);
      return false;
    }

    // Check frame cache: if cached, memcpy directly without ThorVG rendering
    if(mEnableFixedCache && (frameNumber < mDecodedBuffers.size()) && mDecodedBuffers[frameNumber].second)
    {
      const size_t bufferSize = mDecodedBuffers[frameNumber].first.size();
      memcpy(buffer, mDecodedBuffers[frameNumber].first.data(), bufferSize);
    }
    else
    {
      // Render with ThorVG
      if(mTotalFrame > 0)
      {
        mAnimation->frame(static_cast<float>(frameNumber));
      }

      // NativeQueue BGRA8888 = TBM_FORMAT_ARGB8888 -> tvg::ARGB8888
      uint32_t* targetBuffer = reinterpret_cast<uint32_t*>(buffer);
      uint32_t  targetStride = stride / 4;
      mCanvas->target(targetBuffer, targetStride, width, height, tvg::ColorSpace::ARGB8888);
      mCanvas->update();
      mCanvas->draw(true);
      mCanvas->sync();

      // Store in frame cache if enabled
      if(mEnableFixedCache && (frameNumber < mDecodedBuffers.size()))
      {
        const size_t bufferSize = height * static_cast<size_t>(stride);
        mDecodedBuffers[frameNumber].first.assign(buffer, buffer + bufferSize);
        mDecodedBuffers[frameNumber].second = true;
      }
    }

    renderingDataImpl->mTargetSurface->EnqueueBuffer(buffer);

    if(!mResourceReadyTriggered)
    {
      if(renderingDataImpl->mTexture && mRenderedTexture != renderingDataImpl->mTexture)
      {
        mPreviousTextures.push_back(mRenderedTexture);
        mRenderedTexture = std::move(renderingDataImpl->mTexture);
      }

      // Don't need to keep reference of texture.
      renderingDataImpl->mTexture.Reset();

      mResourceReady          = true;
      mResourceReadyTriggered = true;

      VectorAnimationRendererEventManager::Get().TriggerEvent(*this);

      DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "Resource ready [current = %d] [%p]\n", frameNumber, this);
    }
  }
  return true;
}

void VectorAnimationRendererNativeTizen::FreeReleasedBuffers()
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl;
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(mCurrentRenderingData);
    }
  }

  if(renderingDataImpl && renderingDataImpl->mTargetSurface)
  {
    renderingDataImpl->mTargetSurface->FreeReleasedBuffers();
  }
}

VectorAnimationRendererNative* VectorAnimationRendererNative::Create()
{
  return new VectorAnimationRendererNativeTizen();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
