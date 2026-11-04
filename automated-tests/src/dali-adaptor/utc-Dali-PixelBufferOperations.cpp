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
 */

#include <dali-test-suite-utils.h>
#include <dali/dali.h>
#include <dali/devel-api/adaptor-framework/pixel-buffer-operations-devel.h>
#include <dali/devel-api/images/pixel-buffer-devel.h>
#include <dali/integration-api/pixel-buffer-integ.h>
#include <initializer_list>
using namespace Dali;

void utc_dali_pixel_buffer_operations_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_pixel_buffer_operations_cleanup(void)
{
  test_return_value = TET_PASS;
}

namespace
{
void Mask1stQuadrant(Dali::PixelBuffer maskData)
{
  int           width       = maskData.GetWidth();
  int           height      = maskData.GetHeight();
  Pixel::Format pixelFormat = maskData.GetPixelFormat();
  int           bpp         = Pixel::GetBytesPerPixel(pixelFormat);

  unsigned char* maskBuffer = maskData.GetBuffer();
  memset(maskBuffer, 0, width * height * bpp);
  int offset = 0;
  for(int x = 0; x < width; ++x)
  {
    for(int y = 0; y < height; ++y)
    {
      if(x >= width / 2 || y >= height / 2)
      {
        for(int b = 0; b < bpp; ++b)
        {
          maskBuffer[offset + b] = 0xff;
        }
      }
      offset += bpp;
    }
  }
}

void MaskCenterSquare(Dali::PixelBuffer maskData)
{
  int           width       = maskData.GetWidth();
  int           height      = maskData.GetHeight();
  Pixel::Format pixelFormat = maskData.GetPixelFormat();
  int           bpp         = Pixel::GetBytesPerPixel(pixelFormat);

  unsigned char* maskBuffer = maskData.GetBuffer();
  memset(maskBuffer, 0, width * height * bpp);
  int offset = 0;
  for(int y = 0; y < height; ++y)
  {
    for(int x = 0; x < width; ++x)
    {
      if(x >= width / 4 && x < 3 * width / 4 &&
         y >= height / 4 && y < 3 * height / 4)
      {
        for(int b = 0; b < bpp; ++b)
        {
          maskBuffer[offset + b] = 0xff;
        }
      }
      offset += bpp;
    }
  }
}

void AlternateQuadrants(Dali::PixelBuffer buffer)
{
  int           width       = buffer.GetWidth();
  int           height      = buffer.GetHeight();
  Pixel::Format pixelFormat = buffer.GetPixelFormat();
  int           bpp         = Pixel::GetBytesPerPixel(pixelFormat);
  int           stride      = width * bpp;

  unsigned char* pixels = buffer.GetBuffer();
  memset(pixels, 0, width * height * bpp);

  for(int x = 0; x < width; ++x)
  {
    for(int y = 0; y < height; ++y)
    {
      if((x < width / 2 && y >= height / 2) ||
         (x >= width / 2 && y < height / 2))
      {
        for(int b = 0; b < bpp; ++b)
        {
          pixels[y * stride + x * bpp + b] = 0xff;
        }
      }
    }
  }
}

void FillCheckerboard(Dali::PixelBuffer imageData)
{
  int           width       = imageData.GetWidth();
  int           height      = imageData.GetHeight();
  Pixel::Format pixelFormat = imageData.GetPixelFormat();
  int           bpp         = Pixel::GetBytesPerPixel(pixelFormat);

  unsigned char* imageBuffer = imageData.GetBuffer();
  memset(imageBuffer, 0, width * height * bpp);
  int offset = 0;
  for(int x = 0; x < width; ++x)
  {
    for(int y = 0; y < height; ++y)
    {
      // on even lines, odd pixels, or on odd lines, even pixels
      if((x % 2 && y % 2 == 0) || (x % 2 == 0 && y % 2))
      {
        switch(pixelFormat)
        {
          case Pixel::RGBA5551:
            imageBuffer[offset]     = 0xFF;
            imageBuffer[offset + 1] = 0xFF;
            break;
          case Pixel::RGBA4444:
            imageBuffer[offset]     = 0xFF;
            imageBuffer[offset + 1] = 0xFF;
            break;
          case Pixel::RGB565:
            imageBuffer[offset]     = 0xFF;
            imageBuffer[offset + 1] = 0xFF;
            break;
          case Pixel::RGB888:
            imageBuffer[offset]     = 0xFF;
            imageBuffer[offset + 1] = 0xFF;
            imageBuffer[offset + 2] = 0xFF;
            break;
          case Pixel::RGBA8888:
            imageBuffer[offset]     = 0xFF;
            imageBuffer[offset + 1] = 0xFF;
            imageBuffer[offset + 2] = 0xFF;
            imageBuffer[offset + 3] = 0xFF;
            break;
          default:
            break;
        }
      }
      offset += bpp;
    }
  }
}

int GetAlphaAt(Dali::PixelBuffer buffer, int x, int y)
{
  unsigned char* pixels = buffer.GetBuffer();
  int            bpp    = Pixel::GetBytesPerPixel(buffer.GetPixelFormat());
  int            stride = buffer.GetWidth() * bpp;
  int            byteOffset;
  int            bitMask;
  GetAlphaOffsetAndMask(buffer.GetPixelFormat(), byteOffset, bitMask);
  return int(pixels[stride * y + x * bpp + byteOffset]) & bitMask;
}

// Creates an L8 pixel buffer whose pixels hold 0, 1, 2, ... in row order
Dali::PixelBuffer CreateSequentialL8(uint32_t width, uint32_t height)
{
  Dali::PixelBuffer buffer = Dali::PixelBuffer::New(width, height, Pixel::L8);
  for(uint32_t i = 0u; i < width * height; ++i)
  {
    buffer.GetBuffer()[i] = static_cast<uint8_t>(i);
  }
  return buffer;
}

// Checks that a tightly packed L8 pixel buffer holds exactly the given pixels
void CheckL8Pixels(Dali::PixelBuffer buffer, std::initializer_list<uint8_t> expected, const char* location)
{
  DALI_TEST_EQUALS(buffer.GetStrideBytes(), buffer.GetWidth(), location);
  DALI_TEST_EQUALS(buffer.GetBufferSize(), static_cast<uint32_t>(expected.size()), location);

  const uint8_t* pixels = buffer.GetBuffer();
  for(uint8_t value : expected)
  {
    DALI_TEST_EQUALS(static_cast<uint32_t>(*pixels++), static_cast<uint32_t>(value), location);
  }
}

// Creates an RGB888 pixel buffer filled with the given color
Dali::PixelBuffer CreateFilledRGB888(uint32_t width, uint32_t height, uint8_t red, uint8_t green, uint8_t blue)
{
  Dali::PixelBuffer buffer = Dali::PixelBuffer::New(width, height, Pixel::RGB888);
  for(uint32_t i = 0u; i < width * height; ++i)
  {
    buffer.GetBuffer()[i * 3u]      = red;
    buffer.GetBuffer()[i * 3u + 1u] = green;
    buffer.GetBuffer()[i * 3u + 2u] = blue;
  }
  return buffer;
}
} // namespace

int UtcDaliPixelBufferOperationsDevelApplyMask01(void)
{
  TestApplication application;

  unsigned int      width       = 10u;
  unsigned int      height      = 10u;
  Pixel::Format     pixelFormat = Pixel::L8;
  Dali::PixelBuffer maskData    = Dali::PixelBuffer::New(width, height, pixelFormat);

  Mask1stQuadrant(maskData);

  width       = 20u;
  height      = 20u;
  pixelFormat = Pixel::RGBA5551;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(width, height, pixelFormat);
  FillCheckerboard(imageData);

  DevelPixelBufferOperations::ApplyMask(imageData, maskData, 1.0f, false);

  // Test that the pixel format has been promoted to RGBA8888
  DALI_TEST_EQUALS(imageData.GetPixelFormat(), Pixel::RGBA8888, TEST_LOCATION);

  // Test that a pixel in the first quadrant has no alpha value
  unsigned char* buffer = imageData.GetBuffer();
  DALI_TEST_EQUALS(buffer[3], 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[7], 0x00u, TEST_LOCATION);

  // Test that an even pixel in the second quadrant has a full alpha value
  DALI_TEST_EQUALS(buffer[43], 0x00u, TEST_LOCATION);

  // Test that an odd pixel in the second quadrant has full alpha value
  DALI_TEST_EQUALS(buffer[47], 0xffu, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyMask02(void)
{
  TestApplication application;

  unsigned int      width       = 10u;
  unsigned int      height      = 10u;
  Pixel::Format     pixelFormat = Pixel::L8;
  Dali::PixelBuffer maskData    = Dali::PixelBuffer::New(width, height, pixelFormat);

  Mask1stQuadrant(maskData);

  width       = 20u;
  height      = 20u;
  pixelFormat = Pixel::RGBA4444;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(width, height, pixelFormat);
  FillCheckerboard(imageData);

  DevelPixelBufferOperations::ApplyMask(imageData, maskData, 1.0f, false);

  // Test that the pixel format has been promoted to RGBA8888
  DALI_TEST_EQUALS(imageData.GetPixelFormat(), Pixel::RGBA8888, TEST_LOCATION);

  // Test that a pixel in the first quadrant has no alpha value
  unsigned char* buffer = imageData.GetBuffer();
  DALI_TEST_EQUALS(buffer[3], 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[7], 0x00u, TEST_LOCATION);

  // Test that an even pixel in the second quadrant has no alpha value
  DALI_TEST_EQUALS(buffer[43], 0x00u, TEST_LOCATION);

  // Test that an odd pixel in the second quadrant has full alpha value
  DALI_TEST_EQUALS(buffer[47], 0xffu, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyMask03(void)
{
  TestApplication application;
  tet_infoline("Test application of alpha mask to smaller RGB565 image");

  unsigned int      width    = 20u;
  unsigned int      height   = 20u;
  Dali::PixelBuffer maskData = Dali::PixelBuffer::New(width, height, Pixel::L8);
  Mask1stQuadrant(maskData);

  width                       = 10u;
  height                      = 10u;
  Pixel::Format     format    = Pixel::RGB565;
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(width, height, format);
  FillCheckerboard(imageData);

  DevelPixelBufferOperations::ApplyMask(imageData, maskData, 1.0f, false);

  // Test that the pixel format has been promoted to RGBA8888
  DALI_TEST_EQUALS(imageData.GetPixelFormat(), Pixel::RGBA8888, TEST_LOCATION);

  // Test that a pixel in the first quadrant has no alpha value
  unsigned char* buffer = imageData.GetBuffer();
  DALI_TEST_EQUALS(buffer[3], 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[7], 0x00u, TEST_LOCATION);

  // Test that an odd pixel in the fourth quadrant has full alpha value
  DALI_TEST_EQUALS(buffer[(6 * 10 + 7) * 4 + 3], 0xffu, TEST_LOCATION);

  // Test that an even pixel in the fourth quadrant has full alpha value
  DALI_TEST_EQUALS(buffer[(6 * 10 + 8) * 4 + 3], 0xffu, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyMask04(void)
{
  TestApplication application;
  tet_infoline("Test application of alpha mask to larger RGBA8888 image");

  unsigned int      width    = 10u;
  unsigned int      height   = 10u;
  Dali::PixelBuffer maskData = Dali::PixelBuffer::New(width, height, Pixel::L8);
  Mask1stQuadrant(maskData);

  width                       = 20u;
  height                      = 20u;
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(width, height, Pixel::RGBA8888);
  FillCheckerboard(imageData);

  DevelPixelBufferOperations::ApplyMask(imageData, maskData, 1.0f, false);

  // Test that the pixel format has been promoted to RGBA8888
  DALI_TEST_EQUALS(imageData.GetPixelFormat(), Pixel::RGBA8888, TEST_LOCATION);

  // Test that a pixel in the first quadrant has no alpha value
  unsigned char* buffer = imageData.GetBuffer();
  DALI_TEST_EQUALS(buffer[3], 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[7], 0x00u, TEST_LOCATION);

  // Test that an even pixel in the second quadrant has no alpha value
  DALI_TEST_EQUALS(buffer[43], 0x00u, TEST_LOCATION);

  // Test that an odd pixel in the second quadrant has full alpha value
  DALI_TEST_EQUALS(buffer[47], 0xffu, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyMask05(void)
{
  TestApplication application;
  tet_infoline("Test application of alpha mask to smaller RGBA8888 image");

  unsigned int      width    = 20u;
  unsigned int      height   = 20u;
  Dali::PixelBuffer maskData = Dali::PixelBuffer::New(width, height, Pixel::RGBA8888);
  Mask1stQuadrant(maskData);

  width                       = 10u;
  height                      = 10u;
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(width, height, Pixel::RGBA8888);
  FillCheckerboard(imageData);

  DevelPixelBufferOperations::ApplyMask(imageData, maskData, 1.0f, false);

  // Test that the pixel format has been promoted to RGBA8888
  DALI_TEST_EQUALS(imageData.GetPixelFormat(), Pixel::RGBA8888, TEST_LOCATION);

  // Test that a pixel in the first quadrant has no alpha value
  unsigned char* buffer = imageData.GetBuffer();
  DALI_TEST_EQUALS(buffer[3], 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[7], 0x00u, TEST_LOCATION);

  // Test that an odd pixel in the second quadrant has full alpha value
  DALI_TEST_EQUALS(buffer[39], 0xffu, TEST_LOCATION);

  // Test that an even pixel in the second quadrant has no alpha value
  DALI_TEST_EQUALS(buffer[27], 0x00u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyMask06(void)
{
  TestApplication application;
  tet_infoline("Test application of alpha mask to same size RGBA8888 image");

  unsigned int      width    = 10u;
  unsigned int      height   = 10u;
  Dali::PixelBuffer maskData = Dali::PixelBuffer::New(width, height, Pixel::RGBA8888);
  Mask1stQuadrant(maskData);

  width                       = 10u;
  height                      = 10u;
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(width, height, Pixel::RGBA8888);
  FillCheckerboard(imageData);

  DevelPixelBufferOperations::ApplyMask(imageData, maskData, 1.0f, false);

  // Test that the pixel format has been promoted to RGBA8888
  DALI_TEST_EQUALS(imageData.GetPixelFormat(), Pixel::RGBA8888, TEST_LOCATION);

  // Test that a pixel in the first quadrant has no alpha value
  unsigned char* buffer = imageData.GetBuffer();
  DALI_TEST_EQUALS(buffer[3], 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[7], 0x00u, TEST_LOCATION);

  // Test that an odd pixel in the second quadrant has full alpha value
  DALI_TEST_EQUALS(buffer[39], 0xffu, TEST_LOCATION);

  // Test that an even pixel in the second quadrant has no alpha value
  DALI_TEST_EQUALS(buffer[27], 0x00u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyMask07(void)
{
  TestApplication application;
  tet_infoline("Test scaling of source image to match alpha mask");

  unsigned int      width    = 20u;
  unsigned int      height   = 20u;
  Dali::PixelBuffer maskData = Dali::PixelBuffer::New(width, height, Pixel::RGBA8888);
  MaskCenterSquare(maskData);

  // +----------+
  // |  XXXXXX  |
  // |  XXXXXX  |
  // |  XXXXXX  |
  // |  XXXXXX  |
  // *----------+

  width                       = 10u;
  height                      = 10u;
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(width, height, Pixel::RGBA8888);
  AlternateQuadrants(imageData);

  // +-----XXXXX+
  // |     XXXXX|
  // |     XXXXX|
  // |XXXXX     |
  // |XXXXX     |
  // *XXXXX-----+

  DevelPixelBufferOperations::ApplyMask(imageData, maskData, 2.0f, true);

  // +----------+
  // |     XXX  |
  // |     XXX  |
  // |  XXX     |
  // |  XXX     |
  // *----------+

  tet_infoline("Test that the image has been scaled to match the alpha mask");
  DALI_TEST_EQUALS(imageData.GetWidth(), 20, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 20, TEST_LOCATION);

  tet_infoline("Test that pixels in the outer eighths have no alpha\n");

  DALI_TEST_EQUALS(GetAlphaAt(imageData, 0, 0), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 9, 4), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 15, 4), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 19, 4), 0x00u, TEST_LOCATION);

  DALI_TEST_EQUALS(GetAlphaAt(imageData, 0, 19), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 8, 18), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 15, 17), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 19, 16), 0x00u, TEST_LOCATION);

  DALI_TEST_EQUALS(GetAlphaAt(imageData, 0, 1), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 1, 7), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 2, 10), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 3, 19), 0x00u, TEST_LOCATION);

  DALI_TEST_EQUALS(GetAlphaAt(imageData, 19, 1), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 18, 7), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 17, 10), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 16, 19), 0x00u, TEST_LOCATION);

  tet_infoline("Test that pixels in the center have full alpha\n");

  DALI_TEST_EQUALS(GetAlphaAt(imageData, 12, 8), 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 8, 12), 0xffu, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyMask08(void)
{
  TestApplication application;
  tet_infoline("Test scaling of source image to larger than the alpha mask");

  unsigned int      width    = 32u;
  unsigned int      height   = 20u;
  Dali::PixelBuffer maskData = Dali::PixelBuffer::New(width, height, Pixel::RGBA8888);
  AlternateQuadrants(maskData);

  // +-----XXXXX+
  // |     XXXXX|
  // |     XXXXX|
  // |XXXXX     |
  // |XXXXX     |
  // *XXXXX-----+

  width                       = 20u;
  height                      = 16u;
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(width, height, Pixel::RGBA8888);
  MaskCenterSquare(imageData);

  // +----------+
  // |  XXXXXX  |
  // |  XXXXXX  |
  // |  XXXXXX  |
  // |  XXXXXX  |
  // *----------+

  DevelPixelBufferOperations::ApplyMask(imageData, maskData, 4.0f, true);

  // +-----XXXXX+   quadrant
  // |     XXXXX|    1    2
  // |     XXXXX|
  // |XXXXX     |    4    3
  // |XXXXX     |
  // *XXXXX-----+

  tet_infoline("Test that the image has been scaled and cropped to match the alpha mask");
  DALI_TEST_EQUALS(imageData.GetWidth(), 32, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 20, TEST_LOCATION);

  tet_infoline("Test that the image has been resized (the center square should now fill the image)\n");
  tet_infoline("Test that the first quadrant has no alpha");
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 0, 0), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 5, 4), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 5, 8), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 14, 8), 0x00u, TEST_LOCATION);

  tet_infoline("Test that the second quadrant has alpha and data");
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 18, 0), 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 30, 1), 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 30, 8), 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 19, 8), 0xffu, TEST_LOCATION);

  tet_infoline("Test that the third quadrant has no alpha");
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 18, 12), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 31, 12), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 31, 19), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 18, 19), 0x00u, TEST_LOCATION);

  tet_infoline("Test that the fourth quadrant has alpha and data");
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 1, 12), 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 7, 12), 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 7, 19), 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 1, 19), 0xffu, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyMask09(void)
{
  TestApplication application;
  tet_infoline("Test scaling of large source image to larger than the alpha mask");

  unsigned int      width    = 32u;
  unsigned int      height   = 20u;
  Dali::PixelBuffer maskData = Dali::PixelBuffer::New(width, height, Pixel::RGBA8888);
  AlternateQuadrants(maskData);

  // +-----XXXXX+
  // |     XXXXX|
  // |     XXXXX|
  // |XXXXX     |
  // |XXXXX     |
  // *XXXXX-----+

  width                       = 40u;
  height                      = 50u;
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(width, height, Pixel::RGBA8888);
  MaskCenterSquare(imageData);

  // +----------+
  // |  XXXXXX  |
  // |  XXXXXX  |
  // |  XXXXXX  |
  // |  XXXXXX  |
  // *----------+

  DevelPixelBufferOperations::ApplyMask(imageData, maskData, 1.6f, true);

  // +-----XXXXX+   quadrant
  // |     XXXXX|    1    2
  // |     XXXXX|
  // |XXXXX     |    4    3
  // |XXXXX     |
  // *XXXXX-----+

  tet_infoline("Test that the image has been scaled and cropped to match the alpha mask");
  DALI_TEST_EQUALS(imageData.GetWidth(), 32, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 20, TEST_LOCATION);

  tet_infoline("Test that the image has been resized (the center square should now fill the image)\n");
  tet_infoline("Test that the first quadrant has no alpha");
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 0, 0), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 5, 4), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 5, 8), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 14, 8), 0x00u, TEST_LOCATION);

  tet_infoline("Test that the second quadrant has alpha and data");
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 18, 0), 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 30, 1), 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 30, 8), 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 19, 8), 0xffu, TEST_LOCATION);

  tet_infoline("Test that the third quadrant has no alpha");
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 18, 12), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 31, 12), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 31, 19), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 18, 19), 0x00u, TEST_LOCATION);

  tet_infoline("Test that the fourth quadrant has alpha and data");
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 1, 12), 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 7, 12), 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 7, 19), 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(GetAlphaAt(imageData, 1, 19), 0xffu, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsApplyGaussianBlur01(void)
{
  TestApplication application;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(10, 10, Pixel::RGBA8888);
  FillCheckerboard(imageData);

  DALI_TEST_EQUALS(imageData.GetWidth(), 10, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 10, TEST_LOCATION);

  unsigned char* buffer = imageData.GetBuffer();

  // Test that an even pixel in the odd row has full alpha value
  DALI_TEST_EQUALS(buffer[43], 0xffu, TEST_LOCATION);

  // Test that an even pixel in the even row has no alpha value
  DALI_TEST_EQUALS(buffer[55], 0x00u, TEST_LOCATION);

  DALI_TEST_EQUALS(true, PixelBufferOperations::ApplyGaussianBlur(imageData, 0.0f), TEST_LOCATION);

  // Test that the pixels' alpha values are not changed because there is no blur
  DALI_TEST_EQUALS(buffer[43], 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[55], 0x00u, TEST_LOCATION);

  DALI_TEST_EQUALS(false, PixelBufferOperations::ApplyGaussianBlur(imageData, -1.0f), TEST_LOCATION);

  // Test that the pixels' alpha values are not changed because there is no blur
  DALI_TEST_EQUALS(buffer[43], 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[55], 0x00u, TEST_LOCATION);

  DALI_TEST_EQUALS(true, PixelBufferOperations::ApplyGaussianBlur(imageData, 1.0f), TEST_LOCATION);

  // Test that the pixels' alpha values are changed after applying gaussian blur
  DALI_TEST_EQUALS(buffer[43], 0x7Au, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[55], 0x7Eu, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsApplyGaussianBlur02(void)
{
  TestApplication application;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(10, 10, Pixel::RGB888);
  FillCheckerboard(imageData);

  DALI_TEST_EQUALS(imageData.GetWidth(), 10, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 10, TEST_LOCATION);

  unsigned char* buffer = imageData.GetBuffer();

  // Test that an even pixel in the odd row has white value
  DALI_TEST_EQUALS(buffer[30], 0xffu, TEST_LOCATION);

  // Test that an even pixel in the even row has black value
  DALI_TEST_EQUALS(buffer[39], 0x00u, TEST_LOCATION);

  DALI_TEST_EQUALS(true, PixelBufferOperations::ApplyGaussianBlur(imageData, 0.0f), TEST_LOCATION);

  // Test that the pixels' red values are not changed because there is no blur
  DALI_TEST_EQUALS(buffer[30], 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[39], 0x00u, TEST_LOCATION);

  DALI_TEST_EQUALS(false, PixelBufferOperations::ApplyGaussianBlur(imageData, -1.0f), TEST_LOCATION);

  // Test that the pixels' red values are not changed because there is no blur
  DALI_TEST_EQUALS(buffer[30], 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[39], 0x00u, TEST_LOCATION);

  DALI_TEST_EQUALS(true, PixelBufferOperations::ApplyGaussianBlur(imageData, 1.0f), TEST_LOCATION);

  // Test that the pixels' red values are changed after applying gaussian blur
  DALI_TEST_EQUALS(buffer[30], 0x7Au, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[39], 0x7Eu, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsApplyGaussianBlur03(void)
{
  TestApplication application;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(10, 10, Pixel::RGB565);
  FillCheckerboard(imageData);

  DALI_TEST_EQUALS(imageData.GetWidth(), 10, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 10, TEST_LOCATION);

  unsigned char* buffer = imageData.GetBuffer();

  // Test that an even pixel in the odd row has white value
  DALI_TEST_EQUALS(buffer[20], 0xffu, TEST_LOCATION);

  // Test that an even pixel in the even row has black value
  DALI_TEST_EQUALS(buffer[26], 0x00u, TEST_LOCATION);

  DALI_TEST_EQUALS(false, PixelBufferOperations::ApplyGaussianBlur(imageData, 0.0f), TEST_LOCATION);

  // Test that the pixels' red values are not changed because not supported pixel format
  DALI_TEST_EQUALS(buffer[20], 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[26], 0x00u, TEST_LOCATION);

  DALI_TEST_EQUALS(false, PixelBufferOperations::ApplyGaussianBlur(imageData, -1.0f), TEST_LOCATION);

  // Test that the pixels' red values are not changed because not supported pixel format
  DALI_TEST_EQUALS(buffer[20], 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[26], 0x00u, TEST_LOCATION);

  DALI_TEST_EQUALS(false, PixelBufferOperations::ApplyGaussianBlur(imageData, 1.0f), TEST_LOCATION);

  // Test that the pixels' red values are not changed because not supported pixel format
  DALI_TEST_EQUALS(buffer[20], 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[26], 0x00u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelMultiplyColorByAlpha01(void)
{
  TestApplication application;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(10, 10, Pixel::RGBA8888);
  FillCheckerboard(imageData);

  DALI_TEST_EQUALS(imageData.GetWidth(), 10, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 10, TEST_LOCATION);

  uint8_t* buffer = imageData.GetBuffer();

  //Change 0x0 pixel's rgb value as non-zero forcibly.
  buffer[0] = 0xff;
  buffer[1] = 0xf0;
  buffer[2] = 0x0f;
  buffer[3] = 0x11;

  DALI_TEST_EQUALS(DevelPixelBuffer::IsAlphaPreMultiplied(imageData), false, TEST_LOCATION);

  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[0]), 0xff, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[1]), 0xf0, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[2]), 0x0f, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[3]), 0x11, TEST_LOCATION);

  // Apply alpha pre-multiplication
  DevelPixelBufferOperations::MultiplyColorByAlpha(imageData);

  DALI_TEST_EQUALS(DevelPixelBuffer::IsAlphaPreMultiplied(imageData), true, TEST_LOCATION);

  // Test that the buffer is still the same as the previous one.
  DALI_TEST_EQUALS(buffer, imageData.GetBuffer(), TEST_LOCATION);

  // Test alpha premultiplied
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[0]), 0x11, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[1]), 0x10, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[2]), 0x01, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[3]), 0x11, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelMultiplyColorByAlpha02(void)
{
  TestApplication application;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(10, 10, Pixel::RGB888);
  FillCheckerboard(imageData);

  DALI_TEST_EQUALS(imageData.GetWidth(), 10, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 10, TEST_LOCATION);

  uint8_t* buffer = imageData.GetBuffer();

  //Change 0x0 pixel's rgb value as non-zero forcibly.
  buffer[0] = 0xff;
  buffer[1] = 0xf0;
  buffer[2] = 0x0f;

  // Test that non-alpha channel item return false.
  DALI_TEST_EQUALS(DevelPixelBuffer::IsAlphaPreMultiplied(imageData), false, TEST_LOCATION);

  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[0]), 0xff, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[1]), 0xf0, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[2]), 0x0f, TEST_LOCATION);

  // Apply alpha pre-multiplication
  DevelPixelBufferOperations::MultiplyColorByAlpha(imageData);

  // Test that non-alpha channel item return true after call MultiplyColorByAlpha.
  DALI_TEST_EQUALS(DevelPixelBuffer::IsAlphaPreMultiplied(imageData), true, TEST_LOCATION);

  // Test that the buffer is still the same as the previous one.
  DALI_TEST_EQUALS(buffer, imageData.GetBuffer(), TEST_LOCATION);

  // Test MultiplyColorByAlpha have no effect
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[0]), 0xff, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[1]), 0xf0, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[2]), 0x0f, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelMultiplyColorByAlpha03(void)
{
  TestApplication application;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(10, 10, Pixel::COMPRESSED_RGBA_ASTC_4x4_KHR);

  DALI_TEST_EQUALS(imageData.GetWidth(), 10, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 10, TEST_LOCATION);

  // For now, we cannot create compressed pixel buffer directly.

  // Test that alpha channel item return false.
  DALI_TEST_EQUALS(DevelPixelBuffer::IsAlphaPreMultiplied(imageData), false, TEST_LOCATION);

  // Apply alpha pre-multiplication
  DevelPixelBufferOperations::MultiplyColorByAlpha(imageData);

  // Test compressed type have no effect.
  DALI_TEST_EQUALS(DevelPixelBuffer::IsAlphaPreMultiplied(imageData), false, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelMultiplyColorByAlpha04(void)
{
  TestApplication application;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(10, 10, Pixel::COMPRESSED_RGB8_ETC2);

  DALI_TEST_EQUALS(imageData.GetWidth(), 10, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 10, TEST_LOCATION);

  // For now, we cannot create compressed pixel buffer directly.

  // Test that non-alpha channel item return false.
  DALI_TEST_EQUALS(DevelPixelBuffer::IsAlphaPreMultiplied(imageData), false, TEST_LOCATION);

  // Apply alpha pre-multiplication
  DevelPixelBufferOperations::MultiplyColorByAlpha(imageData);

  // Test compressed type have no effect.
  DALI_TEST_EQUALS(DevelPixelBuffer::IsAlphaPreMultiplied(imageData), false, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyCenterCrop01(void)
{
  // SCALE_TO_FILL equivalent: wide source cropped to square
  TestApplication application;

  // 4:3 source (400x300), target 200x200 (1:1)
  // Expected: source is wider → crop sides → crop region 300x300 centered → resize 200x200
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(400, 300, Pixel::RGB888);

  DevelPixelBufferOperations::ApplyCenterCrop(imageData, 200, 200);

  DALI_TEST_EQUALS(imageData.GetWidth(), 200u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 200u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyCenterCrop02(void)
{
  // Tall source cropped to square
  TestApplication application;

  // 3:4 source (300x400), target 200x200 (1:1)
  // Expected: source is taller → crop top/bottom → crop region 300x300 centered → resize 200x200
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(300, 400, Pixel::RGB888);

  DevelPixelBufferOperations::ApplyCenterCrop(imageData, 200, 200);

  DALI_TEST_EQUALS(imageData.GetWidth(), 200u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 200u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyCenterCrop03(void)
{
  // Source already matches target aspect ratio: crop region equals source, only resize
  TestApplication application;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(400, 200, Pixel::RGB888);

  DevelPixelBufferOperations::ApplyCenterCrop(imageData, 200, 100);

  DALI_TEST_EQUALS(imageData.GetWidth(), 200u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 100u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyLetterbox01(void)
{
  // SHRINK_TO_FIT equivalent: wide source letterboxed into square
  TestApplication application;

  // 4:3 source (400x300), target 200x200
  // scale = min(200/400, 200/300) = 0.5 → scaled to 200x150 → pad 25px top and bottom
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(400, 300, Pixel::RGB888);

  DevelPixelBufferOperations::ApplyLetterbox(imageData, 200, 200);

  DALI_TEST_EQUALS(imageData.GetWidth(), 200u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 200u, TEST_LOCATION);

  // Top-left pixel (inside top padding) should be black
  const uint8_t* buffer = imageData.GetBuffer();
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[0]), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[1]), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[2]), 0x00u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyLetterbox02(void)
{
  // No upscaling: source smaller than target
  TestApplication application;

  // 100x100 source, target 200x200 → scale=2.0 >= 1.0 so no resize, just pad
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(100, 100, Pixel::RGB888);

  DevelPixelBufferOperations::ApplyLetterbox(imageData, 200, 200);

  DALI_TEST_EQUALS(imageData.GetWidth(), 200u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 200u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyLetterbox03(void)
{
  // Source already matches desired size: no-op
  TestApplication application;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(200, 200, Pixel::RGB888);

  DevelPixelBufferOperations::ApplyLetterbox(imageData, 200, 200);

  DALI_TEST_EQUALS(imageData.GetWidth(), 200u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 200u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyCenterCrop04(void)
{
  // Pixel content verification: left-half RED, right-half BLUE source → center crop to square
  // Source 400x200 (2:1), target 200x200 (1:1) → sourceAspect(2.0) > targetAspect(1.0)
  // cropWidth = 200*1.0 = 200, cropHeight = 200, offsetX = (400-200)/2 = 100
  // Result: cols 100-299 of source → left 100 cols of result are RED, right 100 cols are BLUE
  TestApplication application;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(400, 200, Pixel::RGB888);

  // Fill left half (cols 0-199) RED, right half (cols 200-399) BLUE
  uint8_t* buffer = imageData.GetBuffer();
  for(uint32_t row = 0u; row < 200u; ++row)
  {
    for(uint32_t col = 0u; col < 400u; ++col)
    {
      uint8_t* pixel = buffer + (row * 400u + col) * 3u;
      if(col < 200u)
      {
        pixel[0] = 0xFF;
        pixel[1] = 0x00;
        pixel[2] = 0x00; // RED
      }
      else
      {
        pixel[0] = 0x00;
        pixel[1] = 0x00;
        pixel[2] = 0xFF; // BLUE
      }
    }
  }

  DevelPixelBufferOperations::ApplyCenterCrop(imageData, 200, 200);

  DALI_TEST_EQUALS(imageData.GetWidth(), 200u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 200u, TEST_LOCATION);

  const uint8_t* result = imageData.GetBuffer();
  // Left side of result maps to source cols 100-199 → RED
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0]), 0xFFu, TEST_LOCATION); // R
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[1]), 0x00u, TEST_LOCATION); // G
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[2]), 0x00u, TEST_LOCATION); // B
  // Right side of result maps to source cols 200-299 → BLUE (pixel at col 100 = byte offset 100*3)
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[100 * 3 + 0]), 0x00u, TEST_LOCATION); // R
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[100 * 3 + 1]), 0x00u, TEST_LOCATION); // G
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[100 * 3 + 2]), 0xFFu, TEST_LOCATION); // B

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyCenterCrop05(void)
{
  // VISUAL_FITTING integration path: 200x200 source cropped to 200x100
  // Source 200x200 (1:1), target 200x100 (2:1) → sourceAspect(1.0) <= targetAspect(2.0)
  // cropWidth = 200, cropHeight = 200/2.0 = 100, offsetY = (200-100)/2 = 50
  // Result rows come from source rows 50-149: top half = RED (rows 50-99), bottom half = BLUE (rows 100-149)
  TestApplication application;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(200, 200, Pixel::RGB888);

  // Fill top half (rows 0-99) RED, bottom half (rows 100-199) BLUE
  uint8_t* buffer = imageData.GetBuffer();
  for(uint32_t row = 0u; row < 200u; ++row)
  {
    for(uint32_t col = 0u; col < 200u; ++col)
    {
      uint8_t* pixel = buffer + (row * 200u + col) * 3u;
      if(row < 100u)
      {
        pixel[0] = 0xFF;
        pixel[1] = 0x00;
        pixel[2] = 0x00; // RED
      }
      else
      {
        pixel[0] = 0x00;
        pixel[1] = 0x00;
        pixel[2] = 0xFF; // BLUE
      }
    }
  }

  DevelPixelBufferOperations::ApplyCenterCrop(imageData, 200, 100);

  DALI_TEST_EQUALS(imageData.GetWidth(), 200u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 100u, TEST_LOCATION);

  const uint8_t* result = imageData.GetBuffer();
  // Result row 0 → source row 50 → RED
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0]), 0xFFu, TEST_LOCATION); // R
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[1]), 0x00u, TEST_LOCATION); // G
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[2]), 0x00u, TEST_LOCATION); // B
  // Result row 50 → source row 100 → BLUE (row 50 byte offset = 50*200*3)
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[50u * 200u * 3u + 0]), 0x00u, TEST_LOCATION); // R
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[50u * 200u * 3u + 1]), 0x00u, TEST_LOCATION); // G
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[50u * 200u * 3u + 2]), 0xFFu, TEST_LOCATION); // B

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyLetterbox04(void)
{
  // Full padding verification: wide source → top/bottom black padding
  // Source 200x100 (2:1) all RED, target 200x200 (1:1)
  // scale = min(200/200, 200/100) = min(1.0, 2.0) = 1.0 → no resize (no upscaling)
  // offsetY = (200-100)/2 = 50 → rows 0-49 black, rows 50-149 RED, rows 150-199 black
  TestApplication application;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(200, 100, Pixel::RGB888);

  // Fill entire source RED
  uint8_t* buffer = imageData.GetBuffer();
  for(uint32_t i = 0u; i < 200u * 100u * 3u; i += 3u)
  {
    buffer[i + 0] = 0xFF;
    buffer[i + 1] = 0x00;
    buffer[i + 2] = 0x00; // RED
  }

  DevelPixelBufferOperations::ApplyLetterbox(imageData, 200, 200);

  DALI_TEST_EQUALS(imageData.GetWidth(), 200u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 200u, TEST_LOCATION);

  const uint8_t* result = imageData.GetBuffer();
  const uint32_t stride = 200u * 3u;

  // Row 0 (top padding) → black
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0 * stride + 0]), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0 * stride + 1]), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0 * stride + 2]), 0x00u, TEST_LOCATION);
  // Row 49 (last top padding row) → black
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[49u * stride + 0]), 0x00u, TEST_LOCATION);
  // Row 50 (first image row) → RED
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[50u * stride + 0]), 0xFFu, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[50u * stride + 1]), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[50u * stride + 2]), 0x00u, TEST_LOCATION);
  // Row 149 (last image row) → RED
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[149u * stride + 0]), 0xFFu, TEST_LOCATION);
  // Row 150 (first bottom padding row) → black
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[150u * stride + 0]), 0x00u, TEST_LOCATION);
  // Row 199 (last row, bottom padding) → black
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[199u * stride + 0]), 0x00u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelApplyLetterbox05(void)
{
  // VISUAL_FITTING integration path: 200x200 source letterboxed to 200x100
  // Source 200x200 (1:1) all GREEN, target 200x100 (2:1)
  // scale = min(200/200, 100/200) = 0.5 → resize to 100x100
  // offsetX = (200-100)/2 = 50 → cols 0-49 black, cols 50-149 GREEN, cols 150-199 black
  TestApplication application;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(200, 200, Pixel::RGB888);

  // Fill entire source GREEN
  uint8_t* buffer = imageData.GetBuffer();
  for(uint32_t i = 0u; i < 200u * 200u * 3u; i += 3u)
  {
    buffer[i + 0] = 0x00;
    buffer[i + 1] = 0xFF;
    buffer[i + 2] = 0x00; // GREEN
  }

  DevelPixelBufferOperations::ApplyLetterbox(imageData, 200, 100);

  DALI_TEST_EQUALS(imageData.GetWidth(), 200u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 100u, TEST_LOCATION);

  const uint8_t* result = imageData.GetBuffer();
  const uint32_t stride = 200u * 3u;

  // Col 0 of row 0 (left padding) → black
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0 * stride + 0 * 3 + 0]), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0 * stride + 0 * 3 + 1]), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0 * stride + 0 * 3 + 2]), 0x00u, TEST_LOCATION);
  // Col 49 of row 0 (last left padding col) → black
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0 * stride + 49u * 3u + 0]), 0x00u, TEST_LOCATION);
  // Col 50 of row 0 (first image col) → GREEN
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0 * stride + 50u * 3u + 0]), 0x00u, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0 * stride + 50u * 3u + 1]), 0xFFu, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0 * stride + 50u * 3u + 2]), 0x00u, TEST_LOCATION);
  // Col 149 of row 0 (last image col) → GREEN
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0 * stride + 149u * 3u + 1]), 0xFFu, TEST_LOCATION);
  // Col 150 of row 0 (first right padding col) → black
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0 * stride + 150u * 3u + 0]), 0x00u, TEST_LOCATION);
  // Col 199 of row 0 (last col, right padding) → black
  DALI_TEST_EQUALS(static_cast<uint32_t>(result[0 * stride + 199u * 3u + 0]), 0x00u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsResize01P(void)
{
  TestApplication   application;
  Dali::PixelBuffer pixbuf = Dali::PixelBuffer::New(20, 20, Pixel::RGBA8888);
  FillCheckerboard(pixbuf);

  // Downscale.
  DALI_TEST_EQUALS(PixelBufferOperations::Resize(pixbuf, 10, 10), true, TEST_LOCATION);
  DALI_TEST_EQUALS(pixbuf.GetWidth(), 10u, TEST_LOCATION);
  DALI_TEST_EQUALS(pixbuf.GetHeight(), 10u, TEST_LOCATION);
  DALI_TEST_EQUALS(pixbuf.GetStrideBytes(), 10u * 4u, TEST_LOCATION);
  DALI_TEST_EQUALS(pixbuf.GetBufferSize(), 10u * 10u * 4u, TEST_LOCATION);
  DALI_TEST_CHECK(pixbuf.GetBuffer() != nullptr);

  // Upscale.
  DALI_TEST_EQUALS(PixelBufferOperations::Resize(pixbuf, 30, 15), true, TEST_LOCATION);
  DALI_TEST_EQUALS(pixbuf.GetWidth(), 30u, TEST_LOCATION);
  DALI_TEST_EQUALS(pixbuf.GetHeight(), 15u, TEST_LOCATION);

  // Resizing to the current dimensions does nothing and succeeds.
  const uint8_t* buffer = pixbuf.GetBuffer();
  DALI_TEST_EQUALS(PixelBufferOperations::Resize(pixbuf, 30, 15), true, TEST_LOCATION);
  DALI_TEST_CHECK(pixbuf.GetBuffer() == buffer);

  END_TEST;
}

int UtcDaliPixelBufferOperationsResize01N(void)
{
  TestApplication application;

  // Zero dimensions cannot be allocated.
  Dali::PixelBuffer pixbuf = Dali::PixelBuffer::New(10, 10, Pixel::RGBA8888);
  DALI_TEST_EQUALS(PixelBufferOperations::Resize(pixbuf, 0, 0), false, TEST_LOCATION);
  DALI_TEST_EQUALS(pixbuf.GetWidth(), 10u, TEST_LOCATION);
  DALI_TEST_EQUALS(pixbuf.GetHeight(), 10u, TEST_LOCATION);

  // The resampler does not support this pixel format.
  Dali::PixelBuffer unsupported = Dali::PixelBuffer::New(10, 10, Pixel::RGB565);
  DALI_TEST_EQUALS(PixelBufferOperations::Resize(unsupported, 5, 5), false, TEST_LOCATION);
  DALI_TEST_EQUALS(unsupported.GetWidth(), 10u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDeprecatedMemberFunctions(void)
{
  TestApplication application;

  // The deprecated member functions route to the same implementations as the free functions.
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(10, 10, Pixel::RGBA8888);
  FillCheckerboard(imageData);

  unsigned char* buffer = imageData.GetBuffer();
  DALI_TEST_EQUALS(imageData.ApplyGaussianBlur(1.0f), true, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[43], 0x7Au, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[55], 0x7Eu, TEST_LOCATION);

  DALI_TEST_EQUALS(imageData.Resize(5, 5), true, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetWidth(), 5u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 5u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelCrop01P(void)
{
  TestApplication application;

  // 4x4 source:
  //  0  1  2  3
  //  4  5  6  7
  //  8  9 10 11
  // 12 13 14 15
  Dali::PixelBuffer imageData = CreateSequentialL8(4u, 4u);

  // A rectangle inside the buffer, copied row by row
  DevelPixelBufferOperations::Crop(imageData, 1u, 1u, 2u, 2u);

  DALI_TEST_EQUALS(imageData.GetWidth(), 2u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 2u, TEST_LOCATION);
  CheckL8Pixels(imageData, {5, 6, 9, 10}, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelCrop02P(void)
{
  TestApplication application;

  Dali::PixelBuffer imageData = CreateSequentialL8(4u, 4u);

  // Whole rows, copied at once
  DevelPixelBufferOperations::Crop(imageData, 0u, 1u, 4u, 2u);

  DALI_TEST_EQUALS(imageData.GetWidth(), 4u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 2u, TEST_LOCATION);
  CheckL8Pixels(imageData, {4, 5, 6, 7, 8, 9, 10, 11}, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelCrop03P(void)
{
  TestApplication application;

  Dali::PixelBuffer imageData = CreateSequentialL8(4u, 4u);
  const uint8_t*    buffer    = imageData.GetBuffer();

  // Cropping to the whole buffer does nothing, so the buffer is not replaced
  DevelPixelBufferOperations::Crop(imageData, 0u, 0u, 4u, 4u);

  DALI_TEST_CHECK(imageData.GetBuffer() == buffer);
  DALI_TEST_EQUALS(imageData.GetWidth(), 4u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 4u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelRotate01P(void)
{
  TestApplication application;

  // 3x2 source:
  // 0 1 2
  // 3 4 5
  Dali::PixelBuffer imageData = CreateSequentialL8(3u, 2u);

  // Counter clockwise
  DALI_TEST_EQUALS(DevelPixelBufferOperations::Rotate(imageData, Degree(90.0f)), true, TEST_LOCATION);

  DALI_TEST_EQUALS(imageData.GetWidth(), 2u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 3u, TEST_LOCATION);
  CheckL8Pixels(imageData, {2, 5, 1, 4, 0, 3}, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelRotate02P(void)
{
  TestApplication application;

  Dali::PixelBuffer imageData = CreateSequentialL8(3u, 2u);

  DALI_TEST_EQUALS(DevelPixelBufferOperations::Rotate(imageData, Degree(180.0f)), true, TEST_LOCATION);

  DALI_TEST_EQUALS(imageData.GetWidth(), 3u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 2u, TEST_LOCATION);
  CheckL8Pixels(imageData, {5, 4, 3, 2, 1, 0}, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelRotate03P(void)
{
  TestApplication application;

  Dali::PixelBuffer imageData = CreateSequentialL8(3u, 2u);

  DALI_TEST_EQUALS(DevelPixelBufferOperations::Rotate(imageData, Degree(270.0f)), true, TEST_LOCATION);

  DALI_TEST_EQUALS(imageData.GetWidth(), 2u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 3u, TEST_LOCATION);
  CheckL8Pixels(imageData, {3, 0, 4, 1, 5, 2}, TEST_LOCATION);

  // A negative angle is the same as its positive equivalent
  Dali::PixelBuffer negativeData = CreateSequentialL8(3u, 2u);

  DALI_TEST_EQUALS(DevelPixelBufferOperations::Rotate(negativeData, Degree(-90.0f)), true, TEST_LOCATION);

  DALI_TEST_EQUALS(negativeData.GetWidth(), 2u, TEST_LOCATION);
  DALI_TEST_EQUALS(negativeData.GetHeight(), 3u, TEST_LOCATION);
  CheckL8Pixels(negativeData, {3, 0, 4, 1, 5, 2}, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelRotate04P(void)
{
  TestApplication application;

  Dali::PixelBuffer imageData = CreateSequentialL8(3u, 2u);
  const uint8_t*    buffer    = imageData.GetBuffer();

  // Angles equivalent to 0 degrees do nothing and succeed
  for(float angle : {0.0f, 360.0f, -360.0f, 720.0f})
  {
    DALI_TEST_EQUALS(DevelPixelBufferOperations::Rotate(imageData, Degree(angle)), true, TEST_LOCATION);
    DALI_TEST_CHECK(imageData.GetBuffer() == buffer);
    DALI_TEST_EQUALS(imageData.GetWidth(), 3u, TEST_LOCATION);
    DALI_TEST_EQUALS(imageData.GetHeight(), 2u, TEST_LOCATION);
  }

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelRotate05P(void)
{
  TestApplication application;

  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(10u, 10u, Pixel::RGBA8888);
  FillCheckerboard(imageData);
  DevelPixelBufferOperations::MultiplyColorByAlpha(imageData);

  // An angle that is not a multiple of 90 degrees enlarges the buffer to fit the rotated image
  DALI_TEST_EQUALS(DevelPixelBufferOperations::Rotate(imageData, Degree(45.0f)), true, TEST_LOCATION);

  DALI_TEST_CHECK(imageData.GetWidth() > 10u);
  DALI_TEST_CHECK(imageData.GetHeight() > 10u);
  DALI_TEST_EQUALS(imageData.GetStrideBytes(), imageData.GetWidth() * 4u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetBufferSize(), imageData.GetStrideBytes() * imageData.GetHeight(), TEST_LOCATION);
  DALI_TEST_CHECK(imageData.GetBuffer() != nullptr);

  // The pre-multiplied flag is kept
  DALI_TEST_EQUALS(DevelPixelBuffer::IsAlphaPreMultiplied(imageData), true, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelRotate01N(void)
{
  TestApplication application;

  // The rotation does not support this pixel format
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(3u, 2u, Pixel::RGB565);
  const uint8_t*    buffer    = imageData.GetBuffer();

  DALI_TEST_EQUALS(DevelPixelBufferOperations::Rotate(imageData, Degree(90.0f)), false, TEST_LOCATION);

  DALI_TEST_CHECK(imageData.GetBuffer() == buffer);
  DALI_TEST_EQUALS(imageData.GetWidth(), 3u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 2u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelGetBrightness01P(void)
{
  TestApplication application;

  DALI_TEST_EQUALS(DevelPixelBufferOperations::GetBrightness(CreateFilledRGB888(4u, 4u, 0xff, 0xff, 0xff)), 255u, TEST_LOCATION);
  DALI_TEST_EQUALS(DevelPixelBufferOperations::GetBrightness(CreateFilledRGB888(4u, 4u, 0x00, 0x00, 0x00)), 0u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelGetBrightness02P(void)
{
  TestApplication application;

  // Each channel is weighted: (299 * R + 587 * G + 114 * B) / 1000
  DALI_TEST_EQUALS(DevelPixelBufferOperations::GetBrightness(CreateFilledRGB888(4u, 4u, 0xff, 0x00, 0x00)), 76u, TEST_LOCATION);
  DALI_TEST_EQUALS(DevelPixelBufferOperations::GetBrightness(CreateFilledRGB888(4u, 4u, 0x00, 0xff, 0x00)), 149u, TEST_LOCATION);
  DALI_TEST_EQUALS(DevelPixelBufferOperations::GetBrightness(CreateFilledRGB888(4u, 4u, 0x00, 0x00, 0xff)), 29u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelGetBrightness03P(void)
{
  TestApplication application;

  // Half of the pixels are white, and the alpha channel is ignored
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(10u, 10u, Pixel::RGBA8888);
  FillCheckerboard(imageData);

  DALI_TEST_EQUALS(DevelPixelBufferOperations::GetBrightness(imageData), 127u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelGetBrightness04P(void)
{
  TestApplication application;

  // Black pixels in rows padded with white bytes: the padding is ignored
  const uint32_t width       = 2u;
  const uint32_t height      = 2u;
  const uint32_t strideBytes = width * 3u + 2u;
  uint8_t*       buffer      = static_cast<uint8_t*>(malloc(strideBytes * height));
  memset(buffer, 0xff, strideBytes * height);
  for(uint32_t y = 0u; y < height; ++y)
  {
    memset(buffer + y * strideBytes, 0x00, width * 3u);
  }

  Dali::PixelBuffer imageData = Dali::Integration::NewPixelBuffer(buffer, strideBytes * height, width, height, strideBytes, Pixel::RGB888);

  DALI_TEST_EQUALS(DevelPixelBufferOperations::GetBrightness(imageData), 0u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferOperationsDevelGetBrightness01N(void)
{
  TestApplication application;

  // A compressed format has no buffer to read
  Dali::PixelBuffer imageData = Dali::PixelBuffer::New(4u, 4u, Pixel::COMPRESSED_RGB8_ETC2);

  DALI_TEST_EQUALS(DevelPixelBufferOperations::GetBrightness(imageData), 0u, TEST_LOCATION);

  END_TEST;
}
