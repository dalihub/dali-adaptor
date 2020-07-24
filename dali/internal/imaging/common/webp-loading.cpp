/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#define DALI_WEBP_ENABLED 1
#endif

#endif
#include <dali/integration-api/debug.h>
#include <dali/public-api/images/pixel-data.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <dali/internal/imaging/common/file-download.h>
#include <dali/internal/system/common/file-reader.h>

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
Debug::Filter *gWebPLoadingLogFilter = Debug::Filter::New( Debug::NoLogging, false, "LOG_GIF_LOADING" );
#endif

constexpr size_t MAXIMUM_DOWNLOAD_IMAGE_SIZE  = 50 * 1024 * 1024;

}

struct WebPLoading::Impl
{
public:
  Impl( const std::string& url, bool isLocalResource )
  : mUrl( url )
  {
#ifdef DALI_WEBP_ENABLED
    if( ReadWebPInformation( isLocalResource ) )
    {
      WebPAnimDecoderOptions webPAnimDecoderOptions;
      WebPAnimDecoderOptionsInit( &webPAnimDecoderOptions );
      webPAnimDecoderOptions.color_mode = MODE_RGBA;
      mWebPAnimDecoder = WebPAnimDecoderNew( &mWebPData, &webPAnimDecoderOptions );
      WebPAnimDecoderGetInfo( mWebPAnimDecoder, &mWebPAnimInfo );
      mTimeStamp.assign( mWebPAnimInfo.frame_count, 0 );
    }
#endif
  }

  bool ReadWebPInformation( bool isLocalResource )
  {
#ifdef DALI_WEBP_ENABLED
    WebPDataInit( &mWebPData );
    if( isLocalResource )
    {
      Internal::Platform::FileReader fileReader( mUrl );
      FILE *fp = fileReader.GetFile();
      if( fp == NULL )
      {
        return false;
      }

      if( fseek( fp, 0, SEEK_END ) <= -1 )
      {
        return false;
      }

      mWebPData.size = ftell( fp );
      if( ( ! fseek( fp, 0, SEEK_SET ) ) )
      {
        unsigned char *WebPDataBuffer;
        WebPDataBuffer = reinterpret_cast<WebPByteType*>( malloc(sizeof( WebPByteType ) * mWebPData.size ) );
        mWebPData.size = fread( WebPDataBuffer, sizeof( WebPByteType ), mWebPData.size, fp );
        mWebPData.bytes = WebPDataBuffer;
      }
      else
      {
        return false;
      }
    }
    else
    {
      // remote file
      bool succeeded;
      Dali::Vector<uint8_t> dataBuffer;
      size_t dataSize;

      succeeded = TizenPlatform::Network::DownloadRemoteFileIntoMemory( mUrl, dataBuffer, dataSize, MAXIMUM_DOWNLOAD_IMAGE_SIZE );
      if( succeeded )
      {
        size_t blobSize = dataBuffer.Size();
        if( blobSize > 0U )
        {
          // Open a file handle on the memory buffer:
          Dali::Internal::Platform::FileReader fileReader( dataBuffer, blobSize );
          FILE * const fp = fileReader.GetFile();
          if ( NULL != fp )
          {
            if( ( ! fseek( fp, 0, SEEK_SET ) ) )
            {
              unsigned char *WebPDataBuffer;
              WebPDataBuffer = reinterpret_cast<WebPByteType*>( malloc(sizeof( WebPByteType ) * blobSize ) );
              mWebPData.size = fread( WebPDataBuffer, sizeof( WebPByteType ), mWebPData.size, fp );
              mWebPData.bytes = WebPDataBuffer;
            }
            else
            {
              DALI_LOG_ERROR( "Error seeking within file\n" );
            }
          }
          else
          {
            DALI_LOG_ERROR( "Error reading file\n" );
          }
        }
      }
    }
    return true;
#else
    return false;
#endif
  }

  // Moveable but not copyable

  Impl( const Impl& ) = delete;
  Impl& operator=( const Impl& ) = delete;
  Impl( Impl&& ) = default;
  Impl& operator=( Impl&& ) = default;

  ~Impl()
  {
#ifdef DALI_WEBP_ENABLED
    if( &mWebPData != NULL )
    {
      free( (void*)mWebPData.bytes );
      mWebPData.bytes = nullptr;
      WebPDataInit( &mWebPData );
    }
    if( mWebPAnimDecoder )
    {
      WebPAnimDecoderDelete(mWebPAnimDecoder);
    }
#endif
  }

  std::string mUrl;
  std::vector<uint32_t> mTimeStamp;
  uint32_t mLoadingFrame{0};

#ifdef DALI_WEBP_ENABLED
  WebPData mWebPData{0};
  WebPAnimDecoder* mWebPAnimDecoder{nullptr};
  WebPAnimInfo mWebPAnimInfo{0};
#endif
};

AnimatedImageLoadingPtr WebPLoading::New( const std::string &url, bool isLocalResource )
{
#ifndef DALI_WEBP_ENABLED
  DALI_LOG_ERROR( "The system does not support Animated WebP format.\n" );
#endif
  return AnimatedImageLoadingPtr( new WebPLoading( url, isLocalResource ) );
}

WebPLoading::WebPLoading( const std::string &url, bool isLocalResource )
: mImpl( new WebPLoading::Impl( url, isLocalResource ) )
{
}

WebPLoading::~WebPLoading()
{
  delete mImpl;
}

bool WebPLoading::LoadNextNFrames( uint32_t frameStartIndex, int count, std::vector<Dali::PixelData> &pixelData )
{
#ifdef DALI_WEBP_ENABLED
  if( frameStartIndex  >= mImpl->mWebPAnimInfo.frame_count )
  {
    return false;
  }

  DALI_LOG_INFO( gWebPLoadingLogFilter, Debug::Concise, "LoadNextNFrames( frameStartIndex:%d, count:%d )\n", frameStartIndex, count );

  if( mImpl->mLoadingFrame > frameStartIndex  )
  {
    mImpl->mLoadingFrame = 0;
    WebPAnimDecoderReset( mImpl->mWebPAnimDecoder );
  }

  for( ; mImpl->mLoadingFrame < frameStartIndex ; ++mImpl->mLoadingFrame )
  {
    uint8_t* frameBuffer;
    int timestamp;
    WebPAnimDecoderGetNext( mImpl->mWebPAnimDecoder, &frameBuffer, &timestamp );
    mImpl->mTimeStamp[mImpl->mLoadingFrame] = timestamp;
  }

  for( int i = 0; i < count; ++i )
  {
    const int bufferSize = mImpl->mWebPAnimInfo.canvas_width * mImpl->mWebPAnimInfo.canvas_height * sizeof( uint32_t );
    uint8_t* frameBuffer;
    int timestamp;
    WebPAnimDecoderGetNext( mImpl->mWebPAnimDecoder, &frameBuffer, &timestamp );

    auto pixelBuffer = new uint8_t[ bufferSize ];
    memcpy( pixelBuffer, frameBuffer, bufferSize );
    mImpl->mTimeStamp[mImpl->mLoadingFrame] = timestamp;

    if( pixelBuffer )
    {
      pixelData.push_back( Dali::PixelData::New( pixelBuffer, bufferSize,
                                                 mImpl->mWebPAnimInfo.canvas_width, mImpl->mWebPAnimInfo.canvas_height,
                                                 Dali::Pixel::RGBA8888, Dali::PixelData::DELETE_ARRAY) );
    }

    mImpl->mLoadingFrame++;
    if( mImpl->mLoadingFrame >= mImpl->mWebPAnimInfo.frame_count )
    {
      mImpl->mLoadingFrame = 0;
      WebPAnimDecoderReset( mImpl->mWebPAnimDecoder );
    }
  }

  return true;
#else
  return false;
#endif
}

Dali::Devel::PixelBuffer WebPLoading::LoadFrame( uint32_t frameIndex )
{
  Dali::Devel::PixelBuffer pixelBuffer;
#ifdef DALI_WEBP_ENABLED
  if( frameIndex  >= mImpl->mWebPAnimInfo.frame_count )
  {
    return pixelBuffer;
  }

  DALI_LOG_INFO( gWebPLoadingLogFilter, Debug::Concise, "LoadNextNFrames( frameIndex:%d )\n", frameIndex );

  if( mImpl->mLoadingFrame > frameIndex  )
  {
    mImpl->mLoadingFrame = 0;
    WebPAnimDecoderReset( mImpl->mWebPAnimDecoder );
  }

  for( ; mImpl->mLoadingFrame < frameIndex ; ++mImpl->mLoadingFrame )
  {
    uint8_t* frameBuffer;
    int timestamp;
    WebPAnimDecoderGetNext( mImpl->mWebPAnimDecoder, &frameBuffer, &timestamp );
    mImpl->mTimeStamp[mImpl->mLoadingFrame] = timestamp;
  }

  const int bufferSize = mImpl->mWebPAnimInfo.canvas_width * mImpl->mWebPAnimInfo.canvas_height * sizeof( uint32_t );
  uint8_t* frameBuffer;
  int timestamp;
  WebPAnimDecoderGetNext( mImpl->mWebPAnimDecoder, &frameBuffer, &timestamp );

  pixelBuffer = Dali::Devel::PixelBuffer::New( mImpl->mWebPAnimInfo.canvas_width, mImpl->mWebPAnimInfo.canvas_height, Dali::Pixel::RGBA8888 );
  memcpy( pixelBuffer.GetBuffer(), frameBuffer, bufferSize );
  mImpl->mTimeStamp[mImpl->mLoadingFrame] = timestamp;

  mImpl->mLoadingFrame++;
  if( mImpl->mLoadingFrame >= mImpl->mWebPAnimInfo.frame_count )
  {
    mImpl->mLoadingFrame = 0;
    WebPAnimDecoderReset( mImpl->mWebPAnimDecoder );
  }
#endif
  return pixelBuffer;
}

ImageDimensions WebPLoading::GetImageSize() const
{
#ifdef DALI_WEBP_ENABLED
  return ImageDimensions( mImpl->mWebPAnimInfo.canvas_width, mImpl->mWebPAnimInfo.canvas_height );
#else
  return ImageDimensions();
#endif
}

uint32_t WebPLoading::GetImageCount() const
{
#ifdef DALI_WEBP_ENABLED
  return mImpl->mWebPAnimInfo.frame_count;
#else
  return 0u;
#endif
}

uint32_t WebPLoading::GetFrameInterval( uint32_t frameIndex ) const
{
  if( frameIndex >= GetImageCount() )
  {
    return 0u;
  }
  else
  {
    if( frameIndex > 0 )
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

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
