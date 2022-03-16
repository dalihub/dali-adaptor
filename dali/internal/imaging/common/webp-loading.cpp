/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/internal/imaging/common/webp-loading.h>

// EXTERNAL INCLUDES
#ifdef DALI_WEBP_AVAILABLE
#include <webp/decode.h>
#include <webp/demux.h>

#if WEBP_DEMUX_ABI_VERSION > 0x0101
#define DALI_ANIMATED_WEBP_ENABLED 1
#endif

#endif
#include <dali/integration-api/debug.h>
#include <dali/public-api/images/pixel-data.h>

#include <dali/devel-api/threading/mutex.h>
#include <dali/internal/imaging/common/file-download.h>
#include <dali/internal/system/common/file-reader.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/image-loading.h>

typedef unsigned char WebPByteType;

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gWebPLoadingLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_GIF_LOADING");
#endif

constexpr size_t MAXIMUM_DOWNLOAD_IMAGE_SIZE = 50 * 1024 * 1024;

} // namespace

struct WebPLoading::Impl
{
public:
  Impl(const std::string& url, bool isLocalResource)
  : mUrl(url),
    mFrameCount(1u),
    mMutex(),
    mBuffer(nullptr),
    mBufferSize(0u),
    mImageSize(),
    mLoadSucceeded(true)
  {
    // mFrameCount will be 1 if the input image is non-animated image or animated image with single frame.
    if(DALI_LIKELY(ReadWebPInformation(isLocalResource)))
    {
#ifdef DALI_ANIMATED_WEBP_ENABLED
      WebPDataInit(&mWebPData);
      mWebPData.size  = mBufferSize;
      mWebPData.bytes = mBuffer;
      WebPAnimDecoderOptions webPAnimDecoderOptions;
      WebPAnimDecoderOptionsInit(&webPAnimDecoderOptions);
      webPAnimDecoderOptions.color_mode = MODE_RGBA;
      mWebPAnimDecoder                  = WebPAnimDecoderNew(&mWebPData, &webPAnimDecoderOptions);
      WebPAnimDecoderGetInfo(mWebPAnimDecoder, &mWebPAnimInfo);
      mTimeStamp.assign(mWebPAnimInfo.frame_count, 0);
      mFrameCount = mWebPAnimInfo.frame_count;
      mImageSize  = ImageDimensions(mWebPAnimInfo.canvas_width, mWebPAnimInfo.canvas_height);
#elif DALI_WEBP_AVAILABLE
      int32_t imageWidth, imageHeight;
      if(WebPGetInfo(mBuffer, mBufferSize, &imageWidth, &imageHeight))
      {
        mImageSize = ImageDimensions(imageWidth, imageHeight);
      }
#endif
#ifndef DALI_WEBP_AVAILABLE
      // If the system doesn't support webp, loading will be failed.
      mFrameCount    = 0u;
      mLoadSucceeded = false;
#endif
    }
    else
    {
      mFrameCount    = 0u;
      mLoadSucceeded = false;
      DALI_LOG_ERROR("Image loading failed for: \"%s\".\n", mUrl.c_str());
    }
  }

  bool ReadWebPInformation(bool isLocalResource)
  {
    FILE* fp = nullptr;
    if(isLocalResource)
    {
      Internal::Platform::FileReader fileReader(mUrl);
      fp = fileReader.GetFile();
      if(DALI_UNLIKELY(fp == nullptr))
      {
        return false;
      }

      if(DALI_UNLIKELY(fseek(fp, 0, SEEK_END) <= -1))
      {
        return false;
      }

      mBufferSize = ftell(fp);
      if(DALI_LIKELY(!fseek(fp, 0, SEEK_SET)))
      {
        mBuffer     = reinterpret_cast<WebPByteType*>(malloc(sizeof(WebPByteType) * mBufferSize));
        mBufferSize = fread(mBuffer, sizeof(WebPByteType), mBufferSize, fp);
        return true;
      }
    }
    else
    {
      // remote file
      bool                  succeeded;
      Dali::Vector<uint8_t> dataBuffer;
      size_t                dataSize;

      succeeded = TizenPlatform::Network::DownloadRemoteFileIntoMemory(mUrl, dataBuffer, dataSize, MAXIMUM_DOWNLOAD_IMAGE_SIZE);
      if(DALI_LIKELY(succeeded))
      {
        mBufferSize = dataBuffer.Size();
        if(DALI_LIKELY(mBufferSize > 0U))
        {
          // Open a file handle on the memory buffer:
          Internal::Platform::FileReader fileReader(dataBuffer, mBufferSize);
          fp = fileReader.GetFile();
          if(DALI_LIKELY(fp != nullptr))
          {
            if(DALI_LIKELY(!fseek(fp, 0, SEEK_SET)))
            {
              mBuffer     = reinterpret_cast<WebPByteType*>(malloc(sizeof(WebPByteType) * mBufferSize));
              mBufferSize = fread(mBuffer, sizeof(WebPByteType), mBufferSize, fp);
              return true;
            }
          }
        }
      }
    }
    return false;
  }

  void ReleaseResource()
  {
#ifdef DALI_ANIMATED_WEBP_ENABLED
    if(&mWebPData != nullptr)
    {
      mWebPData.bytes = nullptr;
      WebPDataInit(&mWebPData);
    }
    if(mWebPAnimDecoder != nullptr)
    {
      WebPAnimDecoderDelete(mWebPAnimDecoder);
      mWebPAnimDecoder = nullptr;
    }
#endif
    if(mBuffer != nullptr)
    {
      free((void*)mBuffer);
      mBuffer = nullptr;
    }
  }

  // Moveable but not copyable

  Impl(const Impl&) = delete;
  Impl& operator=(const Impl&) = delete;
  Impl(Impl&&)                 = default;
  Impl& operator=(Impl&&) = default;

  ~Impl()
  {
    ReleaseResource();
  }

  std::string           mUrl;
  std::vector<uint32_t> mTimeStamp;
  uint32_t              mLoadingFrame{0};
  uint32_t              mFrameCount;
  Mutex                 mMutex;
  // For the case the system doesn't support DALI_ANIMATED_WEBP_ENABLED
  unsigned char*  mBuffer;
  uint32_t        mBufferSize;
  ImageDimensions mImageSize;
  bool            mLoadSucceeded;

#ifdef DALI_ANIMATED_WEBP_ENABLED
  WebPData         mWebPData{0};
  WebPAnimDecoder* mWebPAnimDecoder{nullptr};
  WebPAnimInfo     mWebPAnimInfo{0};
#endif
};

AnimatedImageLoadingPtr WebPLoading::New(const std::string& url, bool isLocalResource)
{
#ifndef DALI_ANIMATED_WEBP_ENABLED
  DALI_LOG_ERROR("The system does not support Animated WebP format.\n");
#endif
  return AnimatedImageLoadingPtr(new WebPLoading(url, isLocalResource));
}

WebPLoading::WebPLoading(const std::string& url, bool isLocalResource)
: mImpl(new WebPLoading::Impl(url, isLocalResource))
{
}

WebPLoading::~WebPLoading()
{
  delete mImpl;
}

bool WebPLoading::LoadNextNFrames(uint32_t frameStartIndex, int count, std::vector<Dali::PixelData>& pixelData)
{
  for(int i = 0; i < count; ++i)
  {
    Dali::Devel::PixelBuffer pixelBuffer = LoadFrame((frameStartIndex + i) % mImpl->mFrameCount);
    Dali::PixelData          imageData   = Devel::PixelBuffer::Convert(pixelBuffer);
    pixelData.push_back(imageData);
  }
  if(DALI_UNLIKELY(pixelData.size() != static_cast<uint32_t>(count)))
  {
    return false;
  }
  return true;
}

Dali::Devel::PixelBuffer WebPLoading::LoadFrame(uint32_t frameIndex)
{
  Dali::Devel::PixelBuffer pixelBuffer;

  // WebPDecodeRGBA is faster than to use demux API for loading non-animated image.
  // If frame count is 1, use WebPDecodeRGBA api.
#ifdef DALI_WEBP_AVAILABLE
  if(mImpl->mFrameCount == 1)
  {
    int32_t width, height;
    if(DALI_UNLIKELY(!WebPGetInfo(mImpl->mBuffer, mImpl->mBufferSize, &width, &height)))
    {
      return pixelBuffer;
    }

    WebPBitstreamFeatures features;
    if(DALI_UNLIKELY(VP8_STATUS_NOT_ENOUGH_DATA == WebPGetFeatures(mImpl->mBuffer, mImpl->mBufferSize, &features)))
    {
      return pixelBuffer;
    }

    uint32_t      channelNumber = (features.has_alpha) ? 4 : 3;
    Pixel::Format pixelFormat   = (channelNumber == 4) ? Pixel::RGBA8888 : Pixel::RGB888;
    pixelBuffer                 = Dali::Devel::PixelBuffer::New(width, height, pixelFormat);
    uint8_t* frameBuffer        = nullptr;
    if(channelNumber == 4)
    {
      frameBuffer = WebPDecodeRGBA(mImpl->mBuffer, mImpl->mBufferSize, &width, &height);
    }
    else
    {
      frameBuffer = WebPDecodeRGB(mImpl->mBuffer, mImpl->mBufferSize, &width, &height);
    }

    if(frameBuffer != nullptr)
    {
      const int32_t imageBufferSize = width * height * sizeof(uint8_t) * channelNumber;
      memcpy(pixelBuffer.GetBuffer(), frameBuffer, imageBufferSize);
      free((void*)frameBuffer);
    }
    mImpl->ReleaseResource();
    return pixelBuffer;
  }
#endif

#ifdef DALI_ANIMATED_WEBP_ENABLED
  Mutex::ScopedLock lock(mImpl->mMutex);
  if(DALI_UNLIKELY(frameIndex >= mImpl->mWebPAnimInfo.frame_count || !mImpl->mLoadSucceeded))
  {
    return pixelBuffer;
  }

  DALI_LOG_INFO(gWebPLoadingLogFilter, Debug::Concise, "LoadFrame( frameIndex:%d )\n", frameIndex);

  if(mImpl->mLoadingFrame > frameIndex)
  {
    mImpl->mLoadingFrame = 0;
    WebPAnimDecoderReset(mImpl->mWebPAnimDecoder);
  }

  for(; mImpl->mLoadingFrame < frameIndex; ++mImpl->mLoadingFrame)
  {
    uint8_t* frameBuffer;
    int      timestamp;
    WebPAnimDecoderGetNext(mImpl->mWebPAnimDecoder, &frameBuffer, &timestamp);
    mImpl->mTimeStamp[mImpl->mLoadingFrame] = timestamp;
  }

  const int bufferSize = mImpl->mWebPAnimInfo.canvas_width * mImpl->mWebPAnimInfo.canvas_height * sizeof(uint32_t);
  uint8_t*  frameBuffer;
  int       timestamp;
  WebPAnimDecoderGetNext(mImpl->mWebPAnimDecoder, &frameBuffer, &timestamp);

  pixelBuffer = Dali::Devel::PixelBuffer::New(mImpl->mWebPAnimInfo.canvas_width, mImpl->mWebPAnimInfo.canvas_height, Dali::Pixel::RGBA8888);
  memcpy(pixelBuffer.GetBuffer(), frameBuffer, bufferSize);
  mImpl->mTimeStamp[mImpl->mLoadingFrame] = timestamp;

  mImpl->mLoadingFrame++;
  if(mImpl->mLoadingFrame >= mImpl->mWebPAnimInfo.frame_count)
  {
    mImpl->mLoadingFrame = 0;
    WebPAnimDecoderReset(mImpl->mWebPAnimDecoder);
  }
#endif
  return pixelBuffer;
}

ImageDimensions WebPLoading::GetImageSize() const
{
  return mImpl->mImageSize;
}

uint32_t WebPLoading::GetImageCount() const
{
  return mImpl->mFrameCount;
}

uint32_t WebPLoading::GetFrameInterval(uint32_t frameIndex) const
{
  // If frameIndex is above the value of ImageCount or current frame is not loading yet, return 0u.
  if(frameIndex >= GetImageCount() || (frameIndex > 0 && mImpl->mTimeStamp[frameIndex - 1] > mImpl->mTimeStamp[frameIndex]))
  {
    return 0u;
  }
  else
  {
    if(frameIndex > 0)
    {
      return mImpl->mTimeStamp[frameIndex] - mImpl->mTimeStamp[frameIndex - 1];
    }
    return mImpl->mTimeStamp[frameIndex];
  }
}

std::string WebPLoading::GetUrl() const
{
  return mImpl->mUrl;
}

bool WebPLoading::HasLoadingSucceeded() const
{
  return mImpl->mLoadSucceeded;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
