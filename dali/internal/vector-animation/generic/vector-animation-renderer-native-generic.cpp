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
#include <dali/internal/vector-animation/generic/vector-animation-renderer-native-generic.h>

// EXTERNAL INCLUDES
#include <cstring>
#include <dali/internal/vector-animation/common/vector-animation-renderer-event-manager.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/rendering/texture-set.h>
#include <chrono>

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

class VectorAnimationRendererNativeGeneric::RenderingDataImpl : public VectorAnimationRendererNative::RenderingData
{
public:
  std::vector<uint8_t> mBuffer;
};

VectorAnimationRendererNativeGeneric::VectorAnimationRendererNativeGeneric()
{
  Initialize();
}

VectorAnimationRendererNativeGeneric::~VectorAnimationRendererNativeGeneric()
{
  Finalize();
}

void VectorAnimationRendererNativeGeneric::OnFinalize()
{
  mRenderedTexture.Reset();
  mPreviousTextures.clear();
}


std::shared_ptr<VectorAnimationRendererNative::RenderingData> VectorAnimationRendererNativeGeneric::CreateRenderingData()
{
  return std::make_shared<RenderingDataImpl>();
}

void VectorAnimationRendererNativeGeneric::PrepareTarget(std::shared_ptr<RenderingData> renderingData)
{
  auto renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(renderingData);
  renderingDataImpl->mBuffer.resize(renderingDataImpl->mWidth * renderingDataImpl->mHeight * 4);
  renderingDataImpl->mTexture = Dali::Texture::New(Dali::TextureType::TEXTURE_2D, Dali::Pixel::RGBA8888, renderingDataImpl->mWidth, renderingDataImpl->mHeight);
}

bool VectorAnimationRendererNativeGeneric::IsTargetPrepared()
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl;
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(mPreparedRenderingData ? mPreparedRenderingData : mCurrentRenderingData);
    }
  }
  return (renderingDataImpl && !renderingDataImpl->mBuffer.empty());
}

Dali::Texture VectorAnimationRendererNativeGeneric::GetTargetTexture()
{
  return mRenderedTexture;
}

bool VectorAnimationRendererNativeGeneric::Render(uint32_t frameNumber)
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

    if(!mCanvas || !mAnimation || renderingDataImpl->mBuffer.empty())
    {
      return false;
    }

    if(mEnableFixedCache && (frameNumber < mDecodedBuffers.size()) && mDecodedBuffers[frameNumber].second)
    {
      const size_t bufferSize = mDecodedBuffers[frameNumber].first.size();
      memcpy(renderingDataImpl->mBuffer.data(), mDecodedBuffers[frameNumber].first.data(), bufferSize);
    }
    else
    {
      if(mTotalFrame > 0)
      {
        mAnimation->frame(static_cast<float>(frameNumber));
      }

      // Use CPU buffer for rendering
      // CPU RGBA8888 = memory [R,G,B,A] on LE -> tvg::ABGR8888
      uint32_t* targetBuffer = reinterpret_cast<uint32_t*>(renderingDataImpl->mBuffer.data());
      uint32_t  targetStride = renderingDataImpl->mWidth;
      uint32_t  targetWidth  = renderingDataImpl->mWidth;
      uint32_t  targetHeight = renderingDataImpl->mHeight;

      mCanvas->target(targetBuffer, targetStride, targetWidth, targetHeight, tvg::ColorSpace::ABGR8888);
      mCanvas->update();
      mCanvas->draw(true);
      mCanvas->sync();

      if(mEnableFixedCache && (frameNumber < mDecodedBuffers.size()))
      {
        mDecodedBuffers[frameNumber].first  = renderingDataImpl->mBuffer;
        mDecodedBuffers[frameNumber].second = true;
      }
    }

    if(!mResourceReadyTriggered)
    {
      if(renderingDataImpl->mTexture && mRenderedTexture != renderingDataImpl->mTexture)
      {
        mPreviousTextures.push_back(mRenderedTexture);
        mRenderedTexture = renderingDataImpl->mTexture;
      }

      mResourceReady          = true;
      mResourceReadyTriggered = true;
      VectorAnimationRendererEventManager::Get().TriggerEvent(*this);

      DALI_LOG_INFO(gVectorAnimationLogFilter, Debug::Verbose, "Resource ready [current = %d] [%p]\n", frameNumber, this);
    }
  }

  return true;
}

void VectorAnimationRendererNativeGeneric::OnNotify()
{
  std::shared_ptr<RenderingDataImpl> renderingDataImpl;
  {
    Dali::Mutex::ScopedLock lock(mRenderingDataMutex);
    if(DALI_LIKELY(!mFinalized))
    {
      renderingDataImpl = std::static_pointer_cast<RenderingDataImpl>(mCurrentRenderingData);
    }
  }

  if(renderingDataImpl && !renderingDataImpl->mBuffer.empty() && renderingDataImpl->mTexture)
  {
    uint32_t width  = renderingDataImpl->mWidth;
    uint32_t height = renderingDataImpl->mHeight;
    uint32_t stride = width * 4;

    // DALi's PixelData::FREE requires memory allocated with malloc
    uint8_t* newBuffer = static_cast<uint8_t*>(malloc(stride * height));
    if(newBuffer)
    {
      memcpy(newBuffer, renderingDataImpl->mBuffer.data(), stride * height);
      Dali::PixelData pixelData = Dali::PixelData::New(newBuffer, stride * height, width, height, Dali::Pixel::RGBA8888, Dali::PixelData::FREE);
      renderingDataImpl->mTexture.Upload(pixelData);
    }
  }

  mPreviousTextures.clear();
}

VectorAnimationRendererNative* VectorAnimationRendererNative::Create()
{
  return new VectorAnimationRendererNativeGeneric();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
