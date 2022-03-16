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

#include <dali/internal/imaging/common/loader-webp.h>

// EXTERNAL INCLUDES
#ifdef DALI_WEBP_AVAILABLE
#include <webp/decode.h>
#include <webp/demux.h>

#if WEBP_DEMUX_ABI_VERSION > 0x0101
#define DALI_ANIMATED_WEBP_ENABLED 1
#endif
#endif
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/integration-api/debug.h>
#include <cstring>
#include <memory>

typedef unsigned char WebPByteType;

namespace Dali
{
namespace TizenPlatform
{
#ifdef DALI_ANIMATED_WEBP_ENABLED
bool ReadWebPInformation(FILE* const fp, WebPData& webPData)
{
  if(DALI_UNLIKELY(fp == NULL))
  {
    return false;
  }

  if(DALI_UNLIKELY(fseek(fp, 0, SEEK_END) <= -1))
  {
    return false;
  }
  WebPDataInit(&webPData);
  webPData.size = ftell(fp);

  if(DALI_LIKELY(!fseek(fp, 0, SEEK_SET)))
  {
    unsigned char* WebPDataBuffer;
    WebPDataBuffer = reinterpret_cast<WebPByteType*>(malloc(sizeof(WebPByteType) * webPData.size));
    webPData.size  = fread(WebPDataBuffer, sizeof(WebPByteType), webPData.size, fp);
    webPData.bytes = WebPDataBuffer;
  }
  else
  {
    return false;
  }
  return true;
}

void ReleaseResource(WebPData& webPData, WebPAnimDecoder* webPAnimDecoder)
{
  free((void*)webPData.bytes);
  webPData.bytes = nullptr;
  WebPDataInit(&webPData);
  if(webPAnimDecoder)
  {
    WebPAnimDecoderDelete(webPAnimDecoder);
  }
}

#endif

bool LoadWebpHeader(const Dali::ImageLoader::Input& input, unsigned int& width, unsigned int& height)
{
  FILE* const fp = input.file;
  if(DALI_UNLIKELY(fp == NULL))
  {
    return false;
  }

  if(DALI_UNLIKELY(fseek(fp, 0, SEEK_END) <= -1))
  {
    return false;
  }

  // If the image is non-animated webp
#ifdef DALI_WEBP_AVAILABLE
  size_t webPSize = ftell(fp);
  if(DALI_LIKELY(!fseek(fp, 0, SEEK_SET)))
  {
    std::vector<uint8_t> encodedImage;
    encodedImage.resize(webPSize);
    size_t readCount = fread(&encodedImage[0], sizeof(uint8_t), encodedImage.size(), fp);
    if(DALI_UNLIKELY(readCount != encodedImage.size()))
    {
      return false;
    }
    int32_t imageWidth, imageHeight;
    if(WebPGetInfo(&encodedImage[0], encodedImage.size(), &imageWidth, &imageHeight))
    {
      width  = static_cast<uint32_t>(imageWidth);
      height = static_cast<uint32_t>(imageHeight);
      return true;
    }
  }
#endif

  // If the image is animated webp
#ifdef DALI_ANIMATED_WEBP_ENABLED
  WebPData         webPData;
  WebPAnimDecoder* webPAnimDecoder = nullptr;
  WebPAnimInfo     webPAnimInfo;
  if(ReadWebPInformation(fp, webPData))
  {
    WebPAnimDecoderOptions webPAnimDecoderOptions;
    WebPAnimDecoderOptionsInit(&webPAnimDecoderOptions);
    webPAnimDecoderOptions.color_mode = MODE_RGBA;
    webPAnimDecoder                   = WebPAnimDecoderNew(&webPData, &webPAnimDecoderOptions);
    if(webPAnimDecoder != nullptr)
    {
      WebPAnimDecoderGetInfo(webPAnimDecoder, &webPAnimInfo);
      width  = webPAnimInfo.canvas_width;
      height = webPAnimInfo.canvas_height;
      ReleaseResource(webPData, webPAnimDecoder);
      return true;
    }
  }
  ReleaseResource(webPData, webPAnimDecoder);
#endif
  DALI_LOG_ERROR("WebP file open failed.\n");
  return false;
}

bool LoadBitmapFromWebp(const Dali::ImageLoader::Input& input, Dali::Devel::PixelBuffer& bitmap)
{
  FILE* const fp = input.file;
  if(DALI_UNLIKELY(fp == NULL))
  {
    return false;
  }

  if(DALI_UNLIKELY(fseek(fp, 0, SEEK_END) <= -1))
  {
    return false;
  }

  // If the image is non-animated webp
#ifdef DALI_WEBP_AVAILABLE
  size_t webPSize = ftell(fp);
  if(DALI_LIKELY(!fseek(fp, 0, SEEK_SET)))
  {
    std::vector<uint8_t> encodedImage;
    encodedImage.resize(webPSize);
    size_t readCount = fread(&encodedImage[0], sizeof(uint8_t), encodedImage.size(), fp);
    if(DALI_UNLIKELY(readCount != encodedImage.size()))
    {
      DALI_LOG_ERROR("WebP image loading failed.\n");
      return false;
    }

    int32_t width, height;
    if(DALI_UNLIKELY(!WebPGetInfo(&encodedImage[0], encodedImage.size(), &width, &height)))
    {
      DALI_LOG_ERROR("Cannot retrieve WebP image size information.\n");
      return false;
    }

    WebPBitstreamFeatures features;
    if(DALI_UNLIKELY(VP8_STATUS_NOT_ENOUGH_DATA == WebPGetFeatures(&encodedImage[0], encodedImage.size(), &features)))
    {
      DALI_LOG_ERROR("Cannot retrieve WebP image features.\n");
      return false;
    }

    uint32_t      channelNumber = (features.has_alpha) ? 4 : 3;
    Pixel::Format pixelFormat   = (channelNumber == 4) ? Pixel::RGBA8888 : Pixel::RGB888;
    bitmap                      = Dali::Devel::PixelBuffer::New(width, height, pixelFormat);
    uint8_t* frameBuffer        = nullptr;
    if(channelNumber == 4)
    {
      frameBuffer = WebPDecodeRGBA(&encodedImage[0], encodedImage.size(), &width, &height);
    }
    else
    {
      frameBuffer = WebPDecodeRGB(&encodedImage[0], encodedImage.size(), &width, &height);
    }

    if(frameBuffer != nullptr)
    {
      const int32_t bufferSize = width * height * sizeof(uint8_t) * channelNumber;
      memcpy(bitmap.GetBuffer(), frameBuffer, bufferSize);
      free((void*)frameBuffer);
      return true;
    }
  }
#endif

  // If the image is animated webp
#ifdef DALI_ANIMATED_WEBP_ENABLED
  WebPData         webPData;
  WebPAnimDecoder* webPAnimDecoder = nullptr;
  WebPAnimInfo     webPAnimInfo;
  if(ReadWebPInformation(fp, webPData))
  {
    WebPAnimDecoderOptions webPAnimDecoderOptions;
    WebPAnimDecoderOptionsInit(&webPAnimDecoderOptions);
    webPAnimDecoderOptions.color_mode = MODE_RGBA;
    webPAnimDecoder                   = WebPAnimDecoderNew(&webPData, &webPAnimDecoderOptions);
    if(webPAnimDecoder != nullptr)
    {
      uint8_t* frameBuffer;
      int      timestamp;
      WebPAnimDecoderGetInfo(webPAnimDecoder, &webPAnimInfo);
      WebPAnimDecoderReset(webPAnimDecoder);
      WebPAnimDecoderGetNext(webPAnimDecoder, &frameBuffer, &timestamp);

      bitmap                   = Dali::Devel::PixelBuffer::New(webPAnimInfo.canvas_width, webPAnimInfo.canvas_height, Dali::Pixel::RGBA8888);
      const int32_t bufferSize = webPAnimInfo.canvas_width * webPAnimInfo.canvas_height * sizeof(uint32_t);
      memcpy(bitmap.GetBuffer(), frameBuffer, bufferSize);
      ReleaseResource(webPData, webPAnimDecoder);
      return true;
    }
  }
  ReleaseResource(webPData, webPAnimDecoder);
#endif

  DALI_LOG_ERROR("WebP image loading failed.\n");
  return false;
}

} // namespace TizenPlatform

} // namespace Dali
