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
 */

// CLASS HEADER
#include <dali/internal/imaging/common/pixel-manipulation.h>

// INTERNAL HEADERS
#include <dali/integration-api/debug.h>
#include <dali/public-api/images/pixel.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
// clang-format off
/**
 * @brief Pre-defined offset tables for each Channel.
 * If invalid channel, return -1. else, return the offset bytes.
 */
//                                                                           | LUMINANCE | RED | GREEN | BLUE | ALPHA | DEPTH | STENCIL |
constexpr std::int8_t ALPHA_ONLY_OFFSET_TABLE[MAX_NUMBER_OF_CHANNELS]      = {        -1 ,  -1 ,    -1 ,   -1 ,     0 ,    -1 ,      -1 };
constexpr std::int8_t LUMINANCE_ONLY_OFFSET_TABLE[MAX_NUMBER_OF_CHANNELS]  = {         0 ,  -1 ,    -1 ,   -1 ,    -1 ,    -1 ,      -1 };
constexpr std::int8_t LUMINANCE_ALPHA_OFFSET_TABLE[MAX_NUMBER_OF_CHANNELS] = {         0 ,  -1 ,    -1 ,   -1 ,     1 ,    -1 ,      -1 };
constexpr std::int8_t RGB_OFFSET_TABLE[MAX_NUMBER_OF_CHANNELS]             = {        -1 ,   0 ,     1 ,    2 ,    -1 ,    -1 ,      -1 };
constexpr std::int8_t BGR_OFFSET_TABLE[MAX_NUMBER_OF_CHANNELS]             = {        -1 ,   2 ,     1 ,    0 ,    -1 ,    -1 ,      -1 };
constexpr std::int8_t RGBA_OFFSET_TABLE[MAX_NUMBER_OF_CHANNELS]            = {        -1 ,   0 ,     1 ,    2 ,     3 ,    -1 ,      -1 };
constexpr std::int8_t BGRA_OFFSET_TABLE[MAX_NUMBER_OF_CHANNELS]            = {        -1 ,   2 ,     1 ,    0 ,     3 ,    -1 ,      -1 };
// clang-format on

/**
 * @brief Template to Read from a buffer with pixel formats that have one byte per channel.
 *
 * @tparam NumberOfChannels The number of channels to check
 * @param pixelData The pixel data to retrieve the value from
 * @param channel The channel we're after
 * @param offsetTable The array of offset bytes for each channels in the pixel format
 * @return The value of the required channel
 */
unsigned int ReadChannelTable(unsigned char* pixelData, Channel channel, const std::int8_t (&offsetTable)[MAX_NUMBER_OF_CHANNELS])
{
  auto retVal = 0u;
  if(offsetTable[channel] >= 0)
  {
    retVal = static_cast<unsigned int>(*(pixelData + offsetTable[channel]));
  }
  return retVal;
}

/**
 * @brief Template to Write to a buffer with pixel formats that have one byte per channel.
 *
 * @tparam NumberOfChannels The number of channels to check
 * @param pixelData The pixel data to write the value to
 * @param channel The channel we're after
 * @param channelValue The value of the channel to set
 * @param offsetTable The array of offset bytes for each channels in the pixel format
 */
void WriteChannelTable(unsigned char* pixelData, Channel channel, unsigned int channelValue, const std::int8_t (&offsetTable)[MAX_NUMBER_OF_CHANNELS])
{
  if(offsetTable[channel] >= 0)
  {
    *(pixelData + offsetTable[channel]) = static_cast<unsigned char>(channelValue & 0xFF);
  }
}

/**
 * @brief Reads from buffers with a pixel format of 565.
 *
 * @param pixelData The pixel data to read from
 * @param channel The channel we're after
 * @param one The first channel of the pixel format
 * @param two The second channel of the pixel format
 * @param three The third channel of the pixel format
 * @return The value of the required channel
 */
unsigned int ReadChannel565(unsigned char* pixelData, Channel channel, Channel one, Channel two, Channel three)
{
  if(channel == one)
  {
    return (static_cast<unsigned int>(*pixelData) & 0xF8) >> 3;
  }
  else if(channel == two)
  {
    return ((static_cast<unsigned int>(*pixelData) & 0x07) << 3) |
           ((static_cast<unsigned int>(*(pixelData + 1)) & 0xE0) >> 5);
  }
  else if(channel == three)
  {
    return static_cast<unsigned int>(*(pixelData + 1)) & 0x1F;
  }
  return 0u;
}

/**
 * @brief Writes to the buffer with a pixel format of 565.
 *
 * @param pixelData The pixel data to write to
 * @param channel The channel we're after
 * @param channelValue The value to write
 * @param one The first channel of the pixel format
 * @param two The second channel of the pixel format
 * @param three The third channel of the pixel format
 */
void WriteChannel565(unsigned char* pixelData, Channel channel, unsigned int channelValue, Channel one, Channel two, Channel three)
{
  if(channel == one)
  {
    *pixelData &= static_cast<unsigned char>(~0xF8);
    *pixelData |= static_cast<unsigned char>((channelValue << 3) & 0xF8);
  }
  else if(channel == two)
  {
    *pixelData &= static_cast<unsigned char>(~0x07);
    *pixelData |= static_cast<unsigned char>((channelValue >> 3) & 0x07);

    *(pixelData + 1) &= static_cast<unsigned char>(~0xE0);
    *(pixelData + 1) |= static_cast<unsigned char>((channelValue << 5) & 0xE0);
  }
  else if(channel == three)
  {
    *(pixelData + 1) &= static_cast<unsigned char>(~0x1F);
    *(pixelData + 1) |= static_cast<unsigned char>(channelValue & 0x1F);
  }
}

/**
 * @brief Reads from buffers with a pixel format of 4444.
 *
 * @param pixelData The pixel data to read from
 * @param channel The channel we're after
 * @param one The first channel of the pixel format
 * @param two The second channel of the pixel format
 * @param three The third channel of the pixel format
 * @param four The fourth channel of the pixel format
 * @return
 */
unsigned int ReadChannel4444(unsigned char* pixelData, Channel channel, Channel one, Channel two, Channel three, Channel four)
{
  if(channel == one)
  {
    return (static_cast<unsigned int>(*pixelData) & 0xF0) >> 4;
  }
  else if(channel == two)
  {
    return (static_cast<unsigned int>(*pixelData) & 0x0F);
  }
  else if(channel == three)
  {
    return (static_cast<unsigned int>(*(pixelData + 1)) & 0xF0) >> 4;
  }
  else if(channel == four)
  {
    return (static_cast<unsigned int>(*(pixelData + 1)) & 0x0F);
  }
  return 0u;
}

/**
 * @brief Writes to the buffer with a pixel format of 565.
 *
 * @param pixelData The pixel data to write to
 * @param channel The channel we're after
 * @param channelValue The value to write
 * @param one The first channel of the pixel format
 * @param two The second channel of the pixel format
 * @param three The third channel of the pixel format
 * @param four The fourth channel of the pixel format
 */
void WriteChannel4444(unsigned char* pixelData, Channel channel, unsigned int channelValue, Channel one, Channel two, Channel three, Channel four)
{
  if(channel == one)
  {
    *pixelData &= static_cast<unsigned char>(~0xF0);
    *pixelData |= static_cast<unsigned char>((channelValue << 4) & 0xF0);
  }
  else if(channel == two)
  {
    *pixelData &= static_cast<unsigned char>(~0x0F);
    *pixelData |= static_cast<unsigned char>(channelValue & 0x0F);
  }
  else if(channel == three)
  {
    *(pixelData + 1) &= static_cast<unsigned char>(~0xF0);
    *(pixelData + 1) |= static_cast<unsigned char>((channelValue << 4) & 0xF0);
  }
  else if(channel == four)
  {
    *(pixelData + 1) &= static_cast<unsigned char>(~0x0F);
    *(pixelData + 1) |= static_cast<unsigned char>(channelValue & 0x0F);
  }
}

/**
 * @brief Reads from buffers with a pixel format of 5551.
 *
 * @param pixelData The pixel data to read from
 * @param channel The channel we're after
 * @param one The first channel of the pixel format
 * @param two The second channel of the pixel format
 * @param three The third channel of the pixel format
 * @param four The fourth channel of the pixel format
 * @return
 */
unsigned int ReadChannel5551(unsigned char* pixelData, Channel channel, Channel one, Channel two, Channel three, Channel four)
{
  if(channel == one)
  {
    return (static_cast<unsigned int>(*pixelData) & 0xF8) >> 3;
  }
  else if(channel == two)
  {
    return ((static_cast<unsigned int>(*pixelData) & 0x07) << 2) |
           ((static_cast<unsigned int>(*(pixelData + 1)) & 0xC0) >> 6);
  }
  else if(channel == three)
  {
    return (static_cast<unsigned int>(*(pixelData + 1)) & 0x3E) >> 1;
  }
  else if(channel == four)
  {
    return static_cast<unsigned int>(*(pixelData + 1)) & 0x01;
  }
  return 0u;
}

/**
 * @brief Writes to the buffer with a pixel format of 5551.
 *
 * @param pixelData The pixel data to write to
 * @param channel The channel we're after
 * @param channelValue The value to write
 * @param one The first channel of the pixel format
 * @param two The second channel of the pixel format
 * @param three The third channel of the pixel format
 * @param four The fourth channel of the pixel format
 */
void WriteChannel5551(unsigned char* pixelData, Channel channel, unsigned int channelValue, Channel one, Channel two, Channel three, Channel four)
{
  // 11111222 22333334
  //    F8  7 C0  3E 1
  if(channel == one)
  {
    *pixelData &= static_cast<unsigned char>(~0xF8);
    *pixelData |= static_cast<unsigned char>((channelValue << 3) & 0xF8);
  }
  else if(channel == two)
  {
    *pixelData &= static_cast<unsigned char>(~0x07);
    *pixelData |= static_cast<unsigned char>((channelValue >> 2) & 0x07);

    *(pixelData + 1) &= static_cast<unsigned char>(~0xC0);
    *(pixelData + 1) |= static_cast<unsigned char>((channelValue << 6) & 0xC0);
  }
  else if(channel == three)
  {
    *(pixelData + 1) &= static_cast<unsigned char>(~0x3E);
    *(pixelData + 1) |= static_cast<unsigned char>((channelValue << 1) & 0x3E);
  }
  else if(channel == four)
  {
    *(pixelData + 1) &= static_cast<unsigned char>(~0x01);
    *(pixelData + 1) |= static_cast<unsigned char>(channelValue & 0x01);
  }
}

} // unnamed namespace

struct Location
{
  unsigned int bitShift;
  unsigned int bitMask;
  bool         available;
};

struct Locations
{
  Location luminance;
  Location alpha;
  Location red;
  Location green;
  Location blue;
};

bool HasChannel(Dali::Pixel::Format pixelFormat, Channel channel)
{
  switch(pixelFormat)
  {
    case Dali::Pixel::A8:
    {
      return (channel == ALPHA);
    }
    case Dali::Pixel::L8:
    {
      return (channel == LUMINANCE);
    }
    case Dali::Pixel::LA88:
    {
      return (channel == LUMINANCE || channel == ALPHA);
    }
    case Dali::Pixel::RGB565:
    case Dali::Pixel::BGR565:
    case Dali::Pixel::RGB888:
    case Dali::Pixel::RGB8888:
    case Dali::Pixel::BGR8888:
    case Dali::Pixel::RGB16F:
    case Dali::Pixel::RGB32F:
    case Dali::Pixel::R11G11B10F:
    {
      return (channel == RED || channel == GREEN || channel == BLUE);
    }

    case Dali::Pixel::RGBA8888:
    case Dali::Pixel::BGRA8888:
    case Dali::Pixel::RGBA4444:
    case Dali::Pixel::BGRA4444:
    case Dali::Pixel::RGBA5551:
    case Dali::Pixel::BGRA5551:
    {
      return (channel == RED || channel == GREEN || channel == BLUE || channel == ALPHA);
    }

    case Dali::Pixel::DEPTH_UNSIGNED_INT:
    case Dali::Pixel::DEPTH_FLOAT:
    {
      return (channel == DEPTH);
    }

    case Dali::Pixel::DEPTH_STENCIL:
    {
      return (channel == DEPTH || channel == STENCIL);
    }

    case Dali::Pixel::INVALID:
    case Dali::Pixel::COMPRESSED_R11_EAC:
    case Dali::Pixel::COMPRESSED_SIGNED_R11_EAC:
    case Dali::Pixel::COMPRESSED_RG11_EAC:
    case Dali::Pixel::COMPRESSED_SIGNED_RG11_EAC:
    case Dali::Pixel::COMPRESSED_RGB8_ETC2:
    case Dali::Pixel::COMPRESSED_SRGB8_ETC2:
    case Dali::Pixel::COMPRESSED_RGB8_ETC1:
    case Dali::Pixel::COMPRESSED_RGB_PVRTC_4BPPV1:
    case Dali::Pixel::COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
    case Dali::Pixel::COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
    case Dali::Pixel::COMPRESSED_RGBA8_ETC2_EAC:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
    case Dali::Pixel::COMPRESSED_RGBA_ASTC_4x4_KHR:
    case Dali::Pixel::COMPRESSED_RGBA_ASTC_5x4_KHR:
    case Dali::Pixel::COMPRESSED_RGBA_ASTC_5x5_KHR:
    case Dali::Pixel::COMPRESSED_RGBA_ASTC_6x5_KHR:
    case Dali::Pixel::COMPRESSED_RGBA_ASTC_6x6_KHR:
    case Dali::Pixel::COMPRESSED_RGBA_ASTC_8x5_KHR:
    case Dali::Pixel::COMPRESSED_RGBA_ASTC_8x6_KHR:
    case Dali::Pixel::COMPRESSED_RGBA_ASTC_8x8_KHR:
    case Dali::Pixel::COMPRESSED_RGBA_ASTC_10x5_KHR:
    case Dali::Pixel::COMPRESSED_RGBA_ASTC_10x6_KHR:
    case Dali::Pixel::COMPRESSED_RGBA_ASTC_10x8_KHR:
    case Dali::Pixel::COMPRESSED_RGBA_ASTC_10x10_KHR:
    case Dali::Pixel::COMPRESSED_RGBA_ASTC_12x10_KHR:
    case Dali::Pixel::COMPRESSED_RGBA_ASTC_12x12_KHR:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
    case Dali::Pixel::COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
    {
      DALI_LOG_ERROR("Pixel formats for compressed images are not compatible with simple channels.\n");
      break;
    }
  }

  return false;
}

unsigned int ReadChannel(unsigned char*      pixelData,
                         Dali::Pixel::Format pixelFormat,
                         Channel             channel)
{
  switch(pixelFormat)
  {
    case Dali::Pixel::A8:
    {
      return ReadChannelTable(pixelData, channel, ALPHA_ONLY_OFFSET_TABLE);
    }
    case Dali::Pixel::L8:
    {
      return ReadChannelTable(pixelData, channel, LUMINANCE_ONLY_OFFSET_TABLE);
    }
    case Dali::Pixel::LA88:
    {
      return ReadChannelTable(pixelData, channel, LUMINANCE_ALPHA_OFFSET_TABLE);
    }
    case Dali::Pixel::RGB565:
    {
      return ReadChannel565(pixelData, channel, RED, GREEN, BLUE);
    }

    case Dali::Pixel::BGR565:
    {
      return ReadChannel565(pixelData, channel, BLUE, GREEN, RED);
    }

    case Dali::Pixel::RGB888:
    case Dali::Pixel::RGB8888:
    {
      return ReadChannelTable(pixelData, channel, RGB_OFFSET_TABLE);
    }

    case Dali::Pixel::BGR8888:
    {
      return ReadChannelTable(pixelData, channel, BGR_OFFSET_TABLE);
    }

    case Dali::Pixel::RGBA8888:
    {
      return ReadChannelTable(pixelData, channel, RGBA_OFFSET_TABLE);
    }

    case Dali::Pixel::BGRA8888:
    {
      return ReadChannelTable(pixelData, channel, BGRA_OFFSET_TABLE);
    }

    case Dali::Pixel::RGBA4444:
    {
      return ReadChannel4444(pixelData, channel, RED, GREEN, BLUE, ALPHA);
    }

    case Dali::Pixel::BGRA4444:
    {
      return ReadChannel4444(pixelData, channel, BLUE, GREEN, RED, ALPHA);
    }

    case Dali::Pixel::RGBA5551:
    {
      return ReadChannel5551(pixelData, channel, RED, GREEN, BLUE, ALPHA);
    }

    case Dali::Pixel::BGRA5551:
    {
      return ReadChannel5551(pixelData, channel, BLUE, GREEN, RED, ALPHA);
    }

    case Dali::Pixel::DEPTH_UNSIGNED_INT:
    case Dali::Pixel::DEPTH_FLOAT:
    case Dali::Pixel::DEPTH_STENCIL:
    {
      return 0u;
    }

    default:
    {
      return 0u;
    }
  }
}

void WriteChannel(unsigned char*      pixelData,
                  Dali::Pixel::Format pixelFormat,
                  Channel             channel,
                  unsigned int        channelValue)
{
  switch(pixelFormat)
  {
    case Dali::Pixel::A8:
    {
      WriteChannelTable(pixelData, channel, channelValue, ALPHA_ONLY_OFFSET_TABLE);
      break;
    }
    case Dali::Pixel::L8:
    {
      WriteChannelTable(pixelData, channel, channelValue, LUMINANCE_ONLY_OFFSET_TABLE);
      break;
    }
    case Dali::Pixel::LA88:
    {
      WriteChannelTable(pixelData, channel, channelValue, LUMINANCE_ALPHA_OFFSET_TABLE);
      break;
    }
    case Dali::Pixel::RGB565:
    {
      WriteChannel565(pixelData, channel, channelValue, RED, GREEN, BLUE);
      break;
    }

    case Dali::Pixel::BGR565:
    {
      WriteChannel565(pixelData, channel, channelValue, BLUE, GREEN, RED);
      break;
    }

    case Dali::Pixel::RGB888:
    case Dali::Pixel::RGB8888:
    {
      WriteChannelTable(pixelData, channel, channelValue, RGB_OFFSET_TABLE);
      break;
    }

    case Dali::Pixel::BGR8888:
    {
      WriteChannelTable(pixelData, channel, channelValue, BGR_OFFSET_TABLE);
      break;
    }

    case Dali::Pixel::RGBA8888:
    {
      WriteChannelTable(pixelData, channel, channelValue, RGBA_OFFSET_TABLE);
      break;
    }

    case Dali::Pixel::BGRA8888:
    {
      WriteChannelTable(pixelData, channel, channelValue, BGRA_OFFSET_TABLE);
      break;
    }

    case Dali::Pixel::RGBA4444:
    {
      WriteChannel4444(pixelData, channel, channelValue, RED, GREEN, BLUE, ALPHA);
      break;
    }

    case Dali::Pixel::BGRA4444:
    {
      WriteChannel4444(pixelData, channel, channelValue, BLUE, GREEN, RED, ALPHA);
      break;
    }

    case Dali::Pixel::RGBA5551:
    {
      WriteChannel5551(pixelData, channel, channelValue, RED, GREEN, BLUE, ALPHA);
      break;
    }

    case Dali::Pixel::BGRA5551:
    {
      WriteChannel5551(pixelData, channel, channelValue, BLUE, GREEN, RED, ALPHA);
      break;
    }

    case Dali::Pixel::DEPTH_UNSIGNED_INT:
    case Dali::Pixel::DEPTH_FLOAT:
    case Dali::Pixel::DEPTH_STENCIL:
    {
      break;
    }

    default:
      break;
  }
}

void ConvertColorChannelsToRGBA8888(
  unsigned char* srcPixel, int srcOffset, Dali::Pixel::Format srcFormat, unsigned char* destPixel, int destOffset)
{
  int red   = ReadChannel(srcPixel + srcOffset, srcFormat, RED);
  int green = ReadChannel(srcPixel + srcOffset, srcFormat, GREEN);
  int blue  = ReadChannel(srcPixel + srcOffset, srcFormat, BLUE);
  switch(srcFormat)
  {
    case Dali::Pixel::RGB565:
    case Dali::Pixel::BGR565:
    {
      red   = (red << 3) | (red & 0x07);
      green = (green << 2) | (green & 0x03);
      blue  = (blue << 3) | (blue & 0x07);
      break;
    }
    case Dali::Pixel::RGBA4444:
    case Dali::Pixel::BGRA4444:
    {
      red   = (red << 4) | (red & 0x0F);
      green = (green << 4) | (green & 0x0F);
      blue  = (blue << 4) | (blue & 0x0F);
      break;
    }
    case Dali::Pixel::RGBA5551:
    case Dali::Pixel::BGRA5551:
    {
      red   = (red << 3) | (red & 0x07);
      green = (green << 3) | (green & 0x07);
      blue  = (blue << 3) | (blue & 0x07);
      break;
    }
    default:
      break;
  }
  WriteChannel(destPixel + destOffset, Dali::Pixel::RGBA8888, RED, red);
  WriteChannel(destPixel + destOffset, Dali::Pixel::RGBA8888, GREEN, green);
  WriteChannel(destPixel + destOffset, Dali::Pixel::RGBA8888, BLUE, blue);
}

int ConvertAlphaChannelToA8(unsigned char* srcPixel, int srcOffset, Dali::Pixel::Format srcFormat)
{
  int alpha     = ReadChannel(srcPixel + srcOffset, srcFormat, ALPHA);
  int destAlpha = alpha;
  switch(srcFormat)
  {
    case Pixel::RGBA5551:
    case Pixel::BGRA5551:
    {
      destAlpha = (alpha == 0) ? 0 : 255;
      break;
    }
    case Pixel::RGBA4444:
    case Pixel::BGRA4444:
    {
      destAlpha = (alpha << 4) | (alpha & 0x0F);
      break;
    }
    default:
      break;
  }
  return destAlpha;
}

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
