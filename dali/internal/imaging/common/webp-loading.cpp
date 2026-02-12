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
#include <dali/internal/imaging/common/pixel-buffer-impl.h>
#include <dali/internal/system/common/system-error-print.h>

typedef uint8_t WebPByteType;

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

static constexpr int32_t INITIAL_INDEX               = -1;
static constexpr size_t  MAXIMUM_DOWNLOAD_IMAGE_SIZE = 50 * 1024 * 1024;
static constexpr int32_t WEBP_LOSSY                  = 1;
static constexpr int32_t WEBP_LOSSLESS               = 2;

} // namespace

struct WebPLoading::Impl
{
public:
  Impl(const std::string& url, bool isLocalResource)
  : mFile(nullptr),
    mUrl(url),
    mFrameCount(1u),
    mMutex(),
    mBuffer(nullptr),
    mBufferSize(0u),
    mImageSize(),
    mLoadSucceeded(false),
    mIsAnimatedImage(false),
    mIsLocalResource(isLocalResource)
  {
  }

  Impl(FILE* const fp)
  : mFile(fp),
    mUrl(),
    mFrameCount(1u),
    mMutex(),
    mBuffer(nullptr),
    mBufferSize(0u),
    mImageSize(),
    mLoadSucceeded(false),
    mIsAnimatedImage(false),
    mIsLocalResource(true)
  {
  }

  /// Worker thread and Event thread called. Mutex mMutex is locked
  bool LoadWebPInformation()
  {
    // Block to do not load this file again.
    if(DALI_UNLIKELY(mLoadSucceeded))
    {
      return mLoadSucceeded;
    }

#ifndef DALI_WEBP_AVAILABLE
    // If the system doesn't support webp, loading will be failed.
    mFrameCount    = 0u;
    mLoadSucceeded = false;
    return mLoadSucceeded;
#endif

    // mFrameCount will be 1 if the input image is non-animated image or animated image with single frame.
    if(DALI_LIKELY(ReadWebPInformation()))
    {
#ifdef DALI_WEBP_AVAILABLE
      WebPDataInit(&mWebPData);
      mWebPData.size  = mBufferSize;
      mWebPData.bytes = mBuffer;

      WebPDemuxer* demuxer = WebPDemux(&mWebPData);
      uint32_t     flags   = WebPDemuxGetI(demuxer, WEBP_FF_FORMAT_FLAGS);
      if(flags & ANIMATION_FLAG)
      {
        mIsAnimatedImage = true;
      }
      WebPDemuxDelete(demuxer);

      if(!mIsAnimatedImage)
      {
        int32_t imageWidth, imageHeight;
        if(WebPGetInfo(mBuffer, mBufferSize, &imageWidth, &imageHeight))
        {
          mImageSize = ImageDimensions(imageWidth, imageHeight);
        }
      }
#endif
#ifdef DALI_ANIMATED_WEBP_ENABLED
      if(mIsAnimatedImage)
      {
        WebPAnimDecoderOptions webPAnimDecoderOptions;
        WebPAnimDecoderOptionsInit(&webPAnimDecoderOptions);
        webPAnimDecoderOptions.color_mode = MODE_RGBA;
        mWebPAnimDecoder                  = WebPAnimDecoderNew(&mWebPData, &webPAnimDecoderOptions);
        WebPAnimDecoderGetInfo(mWebPAnimDecoder, &mWebPAnimInfo);
        mTimeStamp.assign(mWebPAnimInfo.frame_count, 0);
        mFrameCount = mWebPAnimInfo.frame_count;
        mImageSize  = ImageDimensions(mWebPAnimInfo.canvas_width, mWebPAnimInfo.canvas_height);
      }
#endif
      mLoadSucceeded = true;
    }
    else
    {
      mFrameCount    = 0u;
      mLoadSucceeded = false;
      DALI_LOG_ERROR("Image loading failed for: \"%s\".\n", mUrl.c_str());
    }

    return mLoadSucceeded;
  }

  /// Worker thread and Event thread called. Mutex mMutex is locked
  bool ReadWebPInformation()
  {
    mBufferSize = 0;

    FILE*                                           fp = mFile;
    std::unique_ptr<Internal::Platform::FileReader> fileReader;
    Dali::Vector<uint8_t>                           dataBuffer;
    if(fp == nullptr)
    {
      if(mIsLocalResource)
      {
        fileReader = std::make_unique<Internal::Platform::FileReader>(mUrl);
      }
      else
      {
        size_t dataSize;
        if(DALI_LIKELY(TizenPlatform::Network::DownloadRemoteFileIntoMemory(mUrl, dataBuffer, dataSize, MAXIMUM_DOWNLOAD_IMAGE_SIZE)))
        {
          mBufferSize = dataBuffer.Size();
          if(DALI_LIKELY(mBufferSize > 0U))
          {
            // Open a file handle on the memory buffer:
            fileReader = std::make_unique<Internal::Platform::FileReader>(dataBuffer, mBufferSize);
          }
          else
          {
            DALI_LOG_ERROR("Error download empty buffer!\n");
          }
        }
        else
        {
          DALI_LOG_ERROR("Error download failed!\n");
        }
      }

      if(DALI_LIKELY(fileReader))
      {
        fp = fileReader->GetFile();
      }
    }

    if(DALI_LIKELY(fp != nullptr))
    {
      if(mBufferSize == 0)
      {
        if(DALI_UNLIKELY(fseek(fp, 0, SEEK_END) <= -1))
        {
          DALI_LOG_ERROR("Error seeking within file\n");
          DALI_PRINT_SYSTEM_ERROR_LOG();
          return false;
        }
        long positionIndicator = ftell(fp);

        if(positionIndicator > -1L)
        {
          mBufferSize = static_cast<uint32_t>(positionIndicator);
        }

        if(DALI_UNLIKELY(mBufferSize == 0u))
        {
          DALI_LOG_ERROR("Error: filesize is 0!\n");
          DALI_PRINT_SYSTEM_ERROR_LOG();
          return false;
        }
      }

      if(DALI_LIKELY(!fseek(fp, 0, SEEK_SET)))
      {
        mBuffer = reinterpret_cast<WebPByteType*>(malloc(sizeof(WebPByteType) * mBufferSize));
        if(DALI_UNLIKELY(!mBuffer))
        {
          DALI_LOG_ERROR("malloc is failed. request malloc size : %zu\n", sizeof(WebPByteType) * mBufferSize);
          return false;
        }
        if(DALI_UNLIKELY(fread(mBuffer, sizeof(WebPByteType), mBufferSize, fp) != mBufferSize))
        {
          DALI_LOG_ERROR("Error read file\n");
          DALI_PRINT_SYSTEM_ERROR_LOG();
          return false;
        }
        return true;
      }
      else
      {
        DALI_LOG_ERROR("Error seeking within file\n");
        DALI_PRINT_SYSTEM_ERROR_LOG();
      }
    }
    else
    {
      DALI_LOG_ERROR("Error reading file\n");
    }
    return false;
  }

  /// Worker thread called, and Event thread called when destruction. Mutex mMutex is locked
  void ReleaseResource()
  {
#ifdef DALI_WEBP_AVAILABLE
    mWebPData.bytes = nullptr;
    WebPDataInit(&mWebPData);
#endif
#ifdef DALI_ANIMATED_WEBP_ENABLED
    if(mIsAnimatedImage)
    {
      if(mWebPAnimDecoder != nullptr)
      {
        WebPAnimDecoderDelete(mWebPAnimDecoder);
        mWebPAnimDecoder = nullptr;
      }
    }
#endif
    if(mBuffer != nullptr)
    {
      free((void*)mBuffer);
      mBuffer = nullptr;
    }

    // Make to load this file again.
    mLoadSucceeded = false;
  }

  // Moveable but not copyable

  Impl(const Impl&)                = delete;
  Impl& operator=(const Impl&)     = delete;
  Impl(Impl&&) noexcept            = default;
  Impl& operator=(Impl&&) noexcept = default;

  ~Impl()
  {
    Mutex::ScopedLock lock(mMutex);
    ReleaseResource();
  }

  FILE*                 mFile;
  std::string           mUrl;
  std::vector<uint32_t> mTimeStamp;
  int32_t               mLatestLoadedFrame{INITIAL_INDEX};
  uint32_t              mFrameCount;
  Mutex                 mMutex;

  // For the case the system doesn't support DALI_ANIMATED_WEBP_ENABLED
  unsigned char*  mBuffer;
  uint32_t        mBufferSize;
  ImageDimensions mImageSize;
  bool            mLoadSucceeded; ///< Should be changed under mMutex
  bool            mIsAnimatedImage;
  bool            mIsLocalResource;

#ifdef DALI_WEBP_AVAILABLE
  WebPData mWebPData{0};
#endif

#ifdef DALI_ANIMATED_WEBP_ENABLED
  WebPAnimDecoder*         mWebPAnimDecoder{nullptr};
  WebPAnimInfo             mWebPAnimInfo{0};
  Dali::Devel::PixelBuffer mPreLoadedFrame{};
#endif
};

AnimatedImageLoadingPtr WebPLoading::New(const std::string& url, bool isLocalResource)
{
#ifndef DALI_ANIMATED_WEBP_ENABLED
  DALI_LOG_ERROR("The system does not support Animated WebP format.\n");
#endif
  return AnimatedImageLoadingPtr(new WebPLoading(url, isLocalResource));
}

AnimatedImageLoadingPtr WebPLoading::New(FILE* const fp)
{
#ifndef DALI_ANIMATED_WEBP_ENABLED
  DALI_LOG_ERROR("The system does not support Animated WebP format.\n");
#endif
  return AnimatedImageLoadingPtr(new WebPLoading(fp));
}

WebPLoading::WebPLoading(const std::string& url, bool isLocalResource)
: mImpl(new WebPLoading::Impl(url, isLocalResource))
{
}

WebPLoading::WebPLoading(FILE* const fp)
: mImpl(new WebPLoading::Impl(fp))
{
}

WebPLoading::~WebPLoading()
{
  delete mImpl;
}

Dali::Devel::PixelBuffer WebPLoading::LoadFrame(uint32_t frameIndex, ImageDimensions desiredSize)
{
  Dali::Devel::PixelBuffer pixelBuffer;

  // If WebP file is still not loaded, Load the information.
  {
    Mutex::ScopedLock lock(mImpl->mMutex);
    if(DALI_UNLIKELY(!mImpl->mLoadSucceeded))
    {
      if(DALI_UNLIKELY(!mImpl->LoadWebPInformation()))
      {
        mImpl->ReleaseResource();
        return pixelBuffer;
      }
    }
  }

  // WebPDecodeRGBA is faster than to use demux API for loading non-animated image.
  // If frame count is 1, use WebPDecodeRGBA api.
#ifdef DALI_WEBP_AVAILABLE
  if(!mImpl->mIsAnimatedImage)
  {
    int32_t width, height;
    if(DALI_LIKELY(WebPGetInfo(mImpl->mBuffer, mImpl->mBufferSize, &width, &height)))
    {
      WebPBitstreamFeatures features;
      if(DALI_LIKELY(VP8_STATUS_NOT_ENOUGH_DATA != WebPGetFeatures(mImpl->mBuffer, mImpl->mBufferSize, &features)))
      {
        uint32_t channelNumber = (features.has_alpha) ? 4 : 3;
        uint8_t* frameBuffer   = nullptr;

        if(desiredSize.GetWidth() > 0 && desiredSize.GetHeight() > 0)
        {
          const int desiredWidth  = desiredSize.GetWidth();
          const int desiredHeight = desiredSize.GetHeight();

          WebPDecoderConfig config;
          if(!DALI_LIKELY(WebPInitDecoderConfig(&config)))
          {
            DALI_LOG_ERROR("WebPInitDecoderConfig is failed");
            return pixelBuffer;
          }

          // Apply config for scaling
          config.options.use_scaling   = 1;
          config.options.scaled_width  = desiredWidth;
          config.options.scaled_height = desiredHeight;

          if(channelNumber == 4)
          {
            config.output.colorspace = MODE_RGBA;
          }
          else
          {
            config.output.colorspace = MODE_RGB;
          }

          if(WebPDecode(mImpl->mBuffer, mImpl->mBufferSize, &config) == VP8_STATUS_OK)
          {
            frameBuffer = config.output.u.RGBA.rgba;
          }
          else
          {
            DALI_LOG_ERROR("Webp Decoding with scaled size is failed \n");
          }

          if(frameBuffer != nullptr)
          {
            Pixel::Format pixelFormat = (channelNumber == 4) ? Pixel::RGBA8888 : Pixel::RGB888;
            int32_t       bufferSize  = desiredWidth * desiredHeight * Dali::Pixel::GetBytesPerPixel(pixelFormat);
            pixelBuffer               = Dali::Devel::PixelBuffer::New(desiredWidth, desiredHeight, pixelFormat);
            memcpy(pixelBuffer.GetBuffer(), frameBuffer, bufferSize);
          }

          WebPFreeDecBuffer(&config.output);
        }
        else
        {
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
            Pixel::Format  pixelFormat   = (channelNumber == 4) ? Pixel::RGBA8888 : Pixel::RGB888;
            const uint32_t bytesPerPixel = Dali::Pixel::GetBytesPerPixel(pixelFormat);
            int32_t        bufferSize    = width * height * bytesPerPixel;

            Internal::Adaptor::PixelBufferPtr internal = Internal::Adaptor::PixelBuffer::New(frameBuffer, bufferSize, width, height, width * bytesPerPixel, pixelFormat);

            pixelBuffer = Devel::PixelBuffer(internal.Get());
          }
        }
      }
    }
    // The single frame resource should be released after loading.
    {
      Mutex::ScopedLock lock(mImpl->mMutex);
      mImpl->ReleaseResource();
    }
  }
#endif

#ifdef DALI_ANIMATED_WEBP_ENABLED
  if(mImpl->mIsAnimatedImage && mImpl->mBuffer != nullptr)
  {
    Mutex::ScopedLock lock(mImpl->mMutex);
    if(DALI_LIKELY(frameIndex < mImpl->mWebPAnimInfo.frame_count && mImpl->mLoadSucceeded))
    {
      DALI_LOG_INFO(gWebPLoadingLogFilter, Debug::Concise, "LoadFrame( frameIndex:%d )\n", frameIndex);

      if(mImpl->mPreLoadedFrame && mImpl->mLatestLoadedFrame == static_cast<int32_t>(frameIndex))
      {
        pixelBuffer = mImpl->mPreLoadedFrame;
      }
      else
      {
        pixelBuffer = DecodeFrame(frameIndex);
      }
      mImpl->mPreLoadedFrame.Reset();

      // If time stamp of next frame is unknown, load a frame more to know it.
      if(frameIndex + 1 < mImpl->mWebPAnimInfo.frame_count && mImpl->mTimeStamp[frameIndex + 1] == 0u)
      {
        mImpl->mPreLoadedFrame = DecodeFrame(frameIndex + 1);
      }
    }
    else
    {
      mImpl->ReleaseResource();
    }
  }
#endif
  return pixelBuffer;
}

bool WebPLoading::LoadFramePlanes(uint32_t frameIndex, std::vector<Dali::Devel::PixelBuffer>& pixelBuffers, ImageDimensions size)
{
  {
    Mutex::ScopedLock lock(mImpl->mMutex);
    if(DALI_UNLIKELY(!mImpl->mLoadSucceeded))
    {
      if(DALI_UNLIKELY(!mImpl->LoadWebPInformation()))
      {
        mImpl->ReleaseResource();
        return false;
      }
    }
  }

#ifdef DALI_WEBP_AVAILABLE
  if(!mImpl->mIsAnimatedImage)
  {
    WebPDecoderConfig config;
    if(DALI_UNLIKELY(!WebPInitDecoderConfig(&config)))
    {
      DALI_LOG_ERROR("WebPInitDecoderConfig failed\n");
      return false;
    }

    if(WebPGetFeatures(mImpl->mBuffer, mImpl->mBufferSize, &config.input) != VP8_STATUS_OK)
    {
      return false;
    }

    if(config.input.format == WEBP_LOSSLESS)
    {
      // Fallback for RGB(A) format
      return false;
    }

    int width  = config.input.width;
    int height = config.input.height;

    if(size.GetWidth() > 0 && size.GetHeight() > 0)
    {
      config.options.use_scaling   = 1;
      config.options.scaled_width  = size.GetWidth();
      config.options.scaled_height = size.GetHeight();
      width                        = size.GetWidth();
      height                       = size.GetHeight();
    }

    config.output.colorspace         = (!config.input.has_alpha) ? MODE_YUV : MODE_YUVA;
    config.output.is_external_memory = 1;

    int uvWidth  = (width + 1) / 2;
    int uvHeight = (height + 1) / 2;

    size_t ySize  = width * height;
    size_t uvSize = uvWidth * uvHeight;

    Dali::Devel::PixelBuffer yBuffer = Dali::Devel::PixelBuffer::New(width, height, Dali::Pixel::L8);
    Dali::Devel::PixelBuffer uBuffer = Dali::Devel::PixelBuffer::New(uvWidth, uvHeight, Dali::Pixel::CHROMINANCE_U); // U Plane
    Dali::Devel::PixelBuffer vBuffer = Dali::Devel::PixelBuffer::New(uvWidth, uvHeight, Dali::Pixel::CHROMINANCE_V); // V Plane

    Dali::Devel::PixelBuffer aBuffer;
    if(config.input.has_alpha)
    {
      aBuffer = Dali::Devel::PixelBuffer::New(width, height, Dali::Pixel::A8);
    }

    config.output.u.YUVA.y = yBuffer.GetBuffer();
    config.output.u.YUVA.u = uBuffer.GetBuffer();
    config.output.u.YUVA.v = vBuffer.GetBuffer();
    if(aBuffer)
    {
      config.output.u.YUVA.a = aBuffer.GetBuffer();
    }

    config.output.u.YUVA.y_stride = width;
    config.output.u.YUVA.u_stride = uvWidth;
    config.output.u.YUVA.v_stride = uvWidth;
    if(aBuffer)
    {
      config.output.u.YUVA.a_stride = width;
    }

    config.output.u.YUVA.y_size = ySize;
    config.output.u.YUVA.u_size = uvSize;
    config.output.u.YUVA.v_size = uvSize;
    if(aBuffer)
    {
      config.output.u.YUVA.a_size = ySize;
    }

    VP8StatusCode status = WebPDecode(mImpl->mBuffer, mImpl->mBufferSize, &config);

    bool success = false;
    if(status == VP8_STATUS_OK)
    {
      // Black correction.
      uint8_t* yData = yBuffer.GetBuffer();
      for(uint32_t i = 0; i < ySize; ++i)
      {
        int32_t y = static_cast<int32_t>(yData[i]);
        y     = std::max(0, y - 16);
        y     = (y * 298 + 128) >> 8;
        y     = std::min(255, y);

        yData[i] = static_cast<uint8_t>(y);
      }

      pixelBuffers.push_back(yBuffer);
      pixelBuffers.push_back(uBuffer);
      pixelBuffers.push_back(vBuffer);
      if(aBuffer)
      {
        pixelBuffers.push_back(aBuffer);
      }
      success = true;
    }
    else
    {
      DALI_LOG_ERROR("WebP YUV Decoding failed with status: %d\n", status);
    }

    WebPFreeDecBuffer(&config.output);

    {
      Mutex::ScopedLock lock(mImpl->mMutex);
      mImpl->ReleaseResource();
    }

    return success;
  }
#endif

  return false;
}

Dali::Devel::PixelBuffer WebPLoading::DecodeFrame(uint32_t frameIndex)
{
  Dali::Devel::PixelBuffer pixelBuffer;
#ifdef DALI_ANIMATED_WEBP_ENABLED
  if(mImpl->mLatestLoadedFrame >= static_cast<int32_t>(frameIndex))
  {
    mImpl->mLatestLoadedFrame = INITIAL_INDEX;
    WebPAnimDecoderReset(mImpl->mWebPAnimDecoder);
  }

  uint8_t* frameBuffer = nullptr;
  int32_t  timestamp   = 0u;
  for(; mImpl->mLatestLoadedFrame < static_cast<int32_t>(frameIndex);)
  {
    WebPAnimDecoderGetNext(mImpl->mWebPAnimDecoder, &frameBuffer, &timestamp);
    mImpl->mTimeStamp[++mImpl->mLatestLoadedFrame] = timestamp;
  }

  if(frameBuffer != nullptr)
  {
    const int bufferSize = mImpl->mWebPAnimInfo.canvas_width * mImpl->mWebPAnimInfo.canvas_height * sizeof(uint32_t);
    pixelBuffer          = Dali::Devel::PixelBuffer::New(mImpl->mWebPAnimInfo.canvas_width, mImpl->mWebPAnimInfo.canvas_height, Dali::Pixel::RGBA8888);
    memcpy(pixelBuffer.GetBuffer(), frameBuffer, bufferSize);
  }

#endif
  return pixelBuffer;
}

ImageDimensions WebPLoading::GetImageSize() const
{
  if(mImpl->mImageSize == ImageDimensions())
  {
    Mutex::ScopedLock lock(mImpl->mMutex);
    if(DALI_UNLIKELY(!mImpl->mLoadSucceeded))
    {
      mImpl->LoadWebPInformation();
    }
  }
  return mImpl->mImageSize;
}

uint32_t WebPLoading::GetImageCount() const
{
  if(mImpl->mFrameCount == 0u)
  {
    Mutex::ScopedLock lock(mImpl->mMutex);
    if(DALI_UNLIKELY(!mImpl->mLoadSucceeded))
    {
      mImpl->LoadWebPInformation();
    }
  }
  return mImpl->mFrameCount;
}

uint32_t WebPLoading::GetFrameInterval(uint32_t frameIndex) const
{
  if(DALI_UNLIKELY(!mImpl->mLoadSucceeded))
  {
    DALI_LOG_ERROR("WebP file is still not loaded, this frame interval could not be correct value.\n");
  }
  if(frameIndex >= GetImageCount())
  {
    DALI_LOG_ERROR("Input frameIndex exceeded frame count of the WebP.");
    return 0u;
  }
  else
  {
    int32_t interval = 0u;
    if(mImpl->mFrameCount == 1u)
    {
      return 0u;
    }
    else if(frameIndex + 1 == mImpl->mFrameCount)
    {
      // For the interval between last frame and first frame, use last interval again.
      interval = mImpl->mTimeStamp[frameIndex] - mImpl->mTimeStamp[frameIndex - 1];
    }
    else
    {
      interval = mImpl->mTimeStamp[frameIndex + 1] - mImpl->mTimeStamp[frameIndex];
    }

    if(DALI_UNLIKELY(interval < 0))
    {
      DALI_LOG_ERROR("This interval value is not correct, because the frame still hasn't ever been decoded.");
      return 0u;
    }
    return static_cast<uint32_t>(interval);
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
