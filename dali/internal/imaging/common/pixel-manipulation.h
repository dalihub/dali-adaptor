#ifndef DALI_INTERNAL_ADAPTOR_PIXEL_MANIPULATION_H
#define DALI_INTERNAL_ADAPTOR_PIXEL_MANIPULATION_H

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <dali/public-api/images/pixel.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

enum Channel
{
  LUMINANCE,
  RED,
  GREEN,
  BLUE,
  ALPHA,
  MAX_NUMBER_OF_CHANNELS
};

/**
 * Return true if the channel exists in the pixel format
 * @param[in] pixelFormat The pixelFormat
 * @param[in] channel The channel to test for
 * @return true if the channel exists
 */
bool HasChannel( Dali::Pixel::Format pixelFormat, Channel channel );


/**
 * Read a colour channel from the pixel with the given pixel format.
 * Returns zero if the format does not support the channel
 * @param[in] pixelData Location of the pixel
 * @param[in] pixelFormat The format of the pixel
 * @param[in] channel The channel to read
 * @return the channel value
 */
unsigned int ReadChannel( unsigned char* pixelData,
                          Dali::Pixel::Format pixelFormat,
                          Channel channel );

/**
 * Write a colour channel to the pixel with the given pixel format.
 * @param[in] pixelData Location of the pixel
 * @param[in] pixelFormat The format of the pixel
 * @param[in] channel The channel to write
 * @param[in] channelValue the value to write to the channel
 */
void WriteChannel( unsigned char* pixelData,
                   Dali::Pixel::Format pixelFormat,
                   Channel channel,
                   unsigned int channelValue );

/**
 * Convert the colors in the source pixel from their natural format to RGBA8888.
 * @param[in] srcBuffer The source buffer to read from
 * @param[in] srcOffset The offset of the pixel to convert
 * @param[in] srcFormat The pixel format of the source pixel
 * @param[in] destBuffer The destination buffer to write to
 * @param[in] destOffset The offset of the pixel to write
 */
void ConvertColorChannelsToRGBA8888(
  unsigned char* srcBuffer,  int srcOffset,  Dali::Pixel::Format srcFormat,
  unsigned char* destBuffer, int destOffset );

/**
 * Convert the alpha in the source pixel to A8.
 * @param[in] srcBuffer The source buffer to read from
 * @param[in] srcOffset The offset of the pixel to convert
 * @param[in] srcFormat The pixel format of the source pixel
 * @return the alpha value in the range 0-255
 */
int ConvertAlphaChannelToA8( unsigned char* srcPixel, int srcOffset, Dali::Pixel::Format srcFormat );


} // Adaptor
} // Internal
} // Dali


#endif // DALI_INTERNAL_ADAPTOR_PIXEL_MANIPULATION_H
