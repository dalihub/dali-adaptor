/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include "mesh-builder.h"
using namespace Dali;

void utc_dali_pixelbuffer_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_pixelbuffer_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliPixelBufferCreatePixelData(void)
{
  TestApplication application;

  unsigned int       width     = 20u;
  unsigned int       height    = 20u;
  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(width, height, Pixel::RGB888);

  PixelData pixelData = imageData.CreatePixelData();

  DALI_TEST_EQUALS(true, (bool)pixelData, TEST_LOCATION);

  END_TEST;
}

namespace
{
void Mask1stQuadrant(Devel::PixelBuffer maskData)
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

void MaskCenterSquare(Devel::PixelBuffer maskData)
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

void AlternateQuadrants(Devel::PixelBuffer buffer)
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

void FillCheckerboard(Devel::PixelBuffer imageData)
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

int GetAlphaAt(Devel::PixelBuffer buffer, int x, int y)
{
  unsigned char* pixels = buffer.GetBuffer();
  int            bpp    = Pixel::GetBytesPerPixel(buffer.GetPixelFormat());
  int            stride = buffer.GetWidth() * bpp;
  int            byteOffset;
  int            bitMask;
  GetAlphaOffsetAndMask(buffer.GetPixelFormat(), byteOffset, bitMask);
  return int(pixels[stride * y + x * bpp + byteOffset]) & bitMask;
}
} // namespace

int UtcDaliPixelBufferNew01P(void)
{
  TestApplication    application;
  Devel::PixelBuffer pixbuf = Devel::PixelBuffer::New(10, 10, Pixel::RGBA8888);
  DALI_TEST_CHECK(pixbuf);
  DALI_TEST_CHECK(pixbuf.GetBuffer() != NULL);
  END_TEST;
}

int UtcDaliPixelBufferConstructor01P(void)
{
  TestApplication    application;
  Devel::PixelBuffer pixbuf = Devel::PixelBuffer::New(10, 10, Pixel::RGBA8888);

  Devel::PixelBuffer copiedBuf = pixbuf;
  DALI_TEST_CHECK(pixbuf);
  DALI_TEST_CHECK(copiedBuf);
  DALI_TEST_CHECK(pixbuf.GetBuffer() != NULL);
  DALI_TEST_CHECK(copiedBuf.GetBuffer() != NULL);

  Devel::PixelBuffer movedBuf = std::move(pixbuf);
  DALI_TEST_CHECK(!pixbuf);
  DALI_TEST_CHECK(movedBuf);
  DALI_TEST_CHECK(movedBuf.GetBuffer() != NULL);
  END_TEST;
}

int UtcDaliPixelBufferAssign01P(void)
{
  TestApplication    application;
  Devel::PixelBuffer pixbuf = Devel::PixelBuffer::New(10, 10, Pixel::RGBA8888);

  Devel::PixelBuffer copiedBuf;
  copiedBuf = pixbuf;
  DALI_TEST_CHECK(pixbuf);
  DALI_TEST_CHECK(copiedBuf);
  DALI_TEST_CHECK(pixbuf.GetBuffer() != NULL);
  DALI_TEST_CHECK(copiedBuf.GetBuffer() != NULL);

  Devel::PixelBuffer movedBuf;
  DALI_TEST_CHECK(!movedBuf);
  movedBuf = std::move(pixbuf);
  DALI_TEST_CHECK(!pixbuf);
  DALI_TEST_CHECK(movedBuf);
  DALI_TEST_CHECK(movedBuf.GetBuffer() != NULL);
  END_TEST;
}

int UtcDaliPixelBufferNew01N(void)
{
  TestApplication    application;
  Devel::PixelBuffer pixbuf = Devel::PixelBuffer::New(0, 0, Pixel::RGBA8888);
  DALI_TEST_CHECK(pixbuf);
  DALI_TEST_CHECK(pixbuf.GetBuffer() == NULL);
  END_TEST;
}

int UtcDaliPixelBufferConvert01(void)
{
  TestApplication    application;
  TestGlAbstraction& gl           = application.GetGlAbstraction();
  TraceCallStack&    textureTrace = gl.GetTextureTrace();
  textureTrace.Enable(true);

  Devel::PixelBuffer pixbuf = Devel::PixelBuffer::New(10, 10, Pixel::RGB565);
  FillCheckerboard(pixbuf);

  {
    Devel::PixelBuffer pixbufPrime = pixbuf; // store a second handle to the data

    Dali::PixelData pixelData = Devel::PixelBuffer::Convert(pixbuf);
    DALI_TEST_CHECK(!pixbuf);

    // check the buffer in the second handle is empty
    DALI_TEST_CHECK(pixbufPrime.GetBuffer() == NULL);

    DALI_TEST_CHECK(pixelData);
    DALI_TEST_EQUALS(pixelData.GetWidth(), 10, TEST_LOCATION);
    DALI_TEST_EQUALS(pixelData.GetHeight(), 10, TEST_LOCATION);
    DALI_TEST_EQUALS(pixelData.GetStride(), 10, TEST_LOCATION);
    DALI_TEST_EQUALS(pixelData.GetPixelFormat(), Pixel::RGB565, TEST_LOCATION);

    // Try drawing it
    Texture t = Texture::New(TextureType::TEXTURE_2D, Pixel::RGB565, 10, 10);
    t.Upload(pixelData);
    TextureSet ts = TextureSet::New();
    ts.SetTexture(0, t);
    Geometry g = CreateQuadGeometry();
    Shader   s = Shader::New("v", "f");
    Renderer r = Renderer::New(g, s);
    r.SetTextures(ts);
    Actor a = Actor::New();
    a.AddRenderer(r);
    a.SetProperty(Actor::Property::SIZE, Vector2(10, 10));
    a.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
    application.GetScene().Add(a);

    application.SendNotification();
    application.Render();
    DALI_TEST_EQUALS(textureTrace.FindMethod("BindTexture"), true, TEST_LOCATION);

    // Let secondary scope destroy pixbufPrime
  }

  END_TEST;
}

int UtcDaliPixelBufferConvert02(void)
{
  TestApplication    application;
  TestGlAbstraction& gl           = application.GetGlAbstraction();
  TraceCallStack&    textureTrace = gl.GetTextureTrace();
  textureTrace.Enable(true);

  Devel::PixelBuffer pixbuf = Devel::PixelBuffer::New(10, 10, Pixel::RGB565);
  FillCheckerboard(pixbuf);

  {
    Devel::PixelBuffer pixbufPrime = pixbuf; // store a second handle to the data

    Dali::PixelData pixelData = Devel::PixelBuffer::Convert(pixbuf, true);
    DALI_TEST_CHECK(!pixbuf);

    // check the buffer in the second handle is empty
    DALI_TEST_CHECK(pixbufPrime.GetBuffer() == NULL);

    DALI_TEST_CHECK(pixelData);
    DALI_TEST_EQUALS(pixelData.GetWidth(), 10, TEST_LOCATION);
    DALI_TEST_EQUALS(pixelData.GetHeight(), 10, TEST_LOCATION);
    DALI_TEST_EQUALS(pixelData.GetStride(), 10, TEST_LOCATION);
    DALI_TEST_EQUALS(pixelData.GetPixelFormat(), Pixel::RGB565, TEST_LOCATION);

    // Try drawing it
    Texture t = Texture::New(TextureType::TEXTURE_2D, Pixel::RGB565, 10, 10);
    t.Upload(pixelData);
    TextureSet ts = TextureSet::New();
    ts.SetTexture(0, t);
    Geometry g = CreateQuadGeometry();
    Shader   s = Shader::New("v", "f");
    Renderer r = Renderer::New(g, s);
    r.SetTextures(ts);
    Actor a = Actor::New();
    a.AddRenderer(r);
    a.SetProperty(Actor::Property::SIZE, Vector2(10, 10));
    a.SetProperty(Actor::Property::PARENT_ORIGIN, ParentOrigin::CENTER);
    application.GetScene().Add(a);

    application.SendNotification();
    application.Render();
    DALI_TEST_EQUALS(textureTrace.FindMethod("BindTexture"), true, TEST_LOCATION);

    // Let secondary scope destroy pixbufPrime
  }

  END_TEST;
}

int UtcDaliPixelBufferGetWidth(void)
{
  TestApplication    application;
  Devel::PixelBuffer pixbuf = Devel::PixelBuffer::New(10, 10, Pixel::RGB565);
  FillCheckerboard(pixbuf);

  DALI_TEST_EQUALS(pixbuf.GetWidth(), 10, TEST_LOCATION);
  DALI_TEST_EQUALS(pixbuf.GetStride(), 10, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferGetHeight(void)
{
  TestApplication    application;
  Devel::PixelBuffer pixbuf = Devel::PixelBuffer::New(10, 10, Pixel::RGB565);
  FillCheckerboard(pixbuf);

  DALI_TEST_EQUALS(pixbuf.GetHeight(), 10, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferGetPixelFormat(void)
{
  TestApplication    application;
  Devel::PixelBuffer pixbuf = Devel::PixelBuffer::New(10, 10, Pixel::RGB565);
  FillCheckerboard(pixbuf);

  DALI_TEST_EQUALS(pixbuf.GetPixelFormat(), Pixel::RGB565, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferMask01(void)
{
  TestApplication application;

  unsigned int       width       = 10u;
  unsigned int       height      = 10u;
  Pixel::Format      pixelFormat = Pixel::L8;
  Devel::PixelBuffer maskData    = Devel::PixelBuffer::New(width, height, pixelFormat);

  Mask1stQuadrant(maskData);

  width       = 20u;
  height      = 20u;
  pixelFormat = Pixel::RGBA5551;

  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(width, height, pixelFormat);
  FillCheckerboard(imageData);

  imageData.ApplyMask(maskData, 1.0f, false);

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

int UtcDaliPixelBufferMask02(void)
{
  TestApplication application;

  unsigned int       width       = 10u;
  unsigned int       height      = 10u;
  Pixel::Format      pixelFormat = Pixel::L8;
  Devel::PixelBuffer maskData    = Devel::PixelBuffer::New(width, height, pixelFormat);

  Mask1stQuadrant(maskData);

  width       = 20u;
  height      = 20u;
  pixelFormat = Pixel::RGBA4444;

  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(width, height, pixelFormat);
  FillCheckerboard(imageData);

  imageData.ApplyMask(maskData, 1.0f, false);

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

int UtcDaliPixelBufferMask03(void)
{
  TestApplication application;
  tet_infoline("Test application of alpha mask to smaller RGB565 image");

  unsigned int       width    = 20u;
  unsigned int       height   = 20u;
  Devel::PixelBuffer maskData = Devel::PixelBuffer::New(width, height, Pixel::L8);
  Mask1stQuadrant(maskData);

  width                        = 10u;
  height                       = 10u;
  Pixel::Format      format    = Pixel::RGB565;
  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(width, height, format);
  FillCheckerboard(imageData);

  imageData.ApplyMask(maskData, 1.0f, false);

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

int UtcDaliPixelBufferMask04(void)
{
  TestApplication application;
  tet_infoline("Test application of alpha mask to larger RGBA8888 image");

  unsigned int       width    = 10u;
  unsigned int       height   = 10u;
  Devel::PixelBuffer maskData = Devel::PixelBuffer::New(width, height, Pixel::L8);
  Mask1stQuadrant(maskData);

  width                        = 20u;
  height                       = 20u;
  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(width, height, Pixel::RGBA8888);
  FillCheckerboard(imageData);

  imageData.ApplyMask(maskData, 1.0f, false);

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

int UtcDaliPixelBufferMask05(void)
{
  TestApplication application;
  tet_infoline("Test application of alpha mask to smaller RGBA8888 image");

  unsigned int       width    = 20u;
  unsigned int       height   = 20u;
  Devel::PixelBuffer maskData = Devel::PixelBuffer::New(width, height, Pixel::RGBA8888);
  Mask1stQuadrant(maskData);

  width                        = 10u;
  height                       = 10u;
  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(width, height, Pixel::RGBA8888);
  FillCheckerboard(imageData);

  imageData.ApplyMask(maskData, 1.0f, false);

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

int UtcDaliPixelBufferMask06(void)
{
  TestApplication application;
  tet_infoline("Test application of alpha mask to same size RGBA8888 image");

  unsigned int       width    = 10u;
  unsigned int       height   = 10u;
  Devel::PixelBuffer maskData = Devel::PixelBuffer::New(width, height, Pixel::RGBA8888);
  Mask1stQuadrant(maskData);

  width                        = 10u;
  height                       = 10u;
  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(width, height, Pixel::RGBA8888);
  FillCheckerboard(imageData);

  imageData.ApplyMask(maskData, 1.0f, false);

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

int UtcDaliPixelBufferMask07(void)
{
  TestApplication application;
  tet_infoline("Test scaling of source image to match alpha mask");

  unsigned int       width    = 20u;
  unsigned int       height   = 20u;
  Devel::PixelBuffer maskData = Devel::PixelBuffer::New(width, height, Pixel::RGBA8888);
  MaskCenterSquare(maskData);

  // +----------+
  // |  XXXXXX  |
  // |  XXXXXX  |
  // |  XXXXXX  |
  // |  XXXXXX  |
  // *----------+

  width                        = 10u;
  height                       = 10u;
  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(width, height, Pixel::RGBA8888);
  AlternateQuadrants(imageData);

  // +-----XXXXX+
  // |     XXXXX|
  // |     XXXXX|
  // |XXXXX     |
  // |XXXXX     |
  // *XXXXX-----+

  imageData.ApplyMask(maskData, 2.0f, true);

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

int UtcDaliPixelBufferMask08(void)
{
  TestApplication application;
  tet_infoline("Test scaling of source image to larger than the alpha mask");

  unsigned int       width    = 32u;
  unsigned int       height   = 20u;
  Devel::PixelBuffer maskData = Devel::PixelBuffer::New(width, height, Pixel::RGBA8888);
  AlternateQuadrants(maskData);

  // +-----XXXXX+
  // |     XXXXX|
  // |     XXXXX|
  // |XXXXX     |
  // |XXXXX     |
  // *XXXXX-----+

  width                        = 20u;
  height                       = 16u;
  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(width, height, Pixel::RGBA8888);
  MaskCenterSquare(imageData);

  // +----------+
  // |  XXXXXX  |
  // |  XXXXXX  |
  // |  XXXXXX  |
  // |  XXXXXX  |
  // *----------+

  imageData.ApplyMask(maskData, 4.0f, true);

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

int UtcDaliPixelBufferMask09(void)
{
  TestApplication application;
  tet_infoline("Test scaling of large source image to larger than the alpha mask");

  unsigned int       width    = 32u;
  unsigned int       height   = 20u;
  Devel::PixelBuffer maskData = Devel::PixelBuffer::New(width, height, Pixel::RGBA8888);
  AlternateQuadrants(maskData);

  // +-----XXXXX+
  // |     XXXXX|
  // |     XXXXX|
  // |XXXXX     |
  // |XXXXX     |
  // *XXXXX-----+

  width                        = 40u;
  height                       = 50u;
  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(width, height, Pixel::RGBA8888);
  MaskCenterSquare(imageData);

  // +----------+
  // |  XXXXXX  |
  // |  XXXXXX  |
  // |  XXXXXX  |
  // |  XXXXXX  |
  // *----------+

  imageData.ApplyMask(maskData, 1.6f, true);

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

int UtcDaliPixelBufferGaussianBlur(void)
{
  TestApplication application;

  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(10, 10, Pixel::RGBA8888);
  FillCheckerboard(imageData);

  DALI_TEST_EQUALS(imageData.GetWidth(), 10, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 10, TEST_LOCATION);

  unsigned char* buffer = imageData.GetBuffer();

  // Test that an even pixel in the odd row has full alpha value
  DALI_TEST_EQUALS(buffer[43], 0xffu, TEST_LOCATION);

  // Test that an even pixel in the even row has no alpha value
  DALI_TEST_EQUALS(buffer[55], 0x00u, TEST_LOCATION);

  imageData.ApplyGaussianBlur(0.0f);

  // Test that the pixels' alpha values are not changed because there is no blur
  DALI_TEST_EQUALS(buffer[43], 0xffu, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[55], 0x00u, TEST_LOCATION);

  imageData.ApplyGaussianBlur(1.0f);

  // Test that the pixels' alpha values are changed after applying gaussian blur
  DALI_TEST_EQUALS(buffer[43], 0x7Au, TEST_LOCATION);
  DALI_TEST_EQUALS(buffer[55], 0x7Eu, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferMultiplyColorByAlpha01(void)
{
  TestApplication application;

  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(10, 10, Pixel::RGBA8888);
  FillCheckerboard(imageData);

  DALI_TEST_EQUALS(imageData.GetWidth(), 10, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 10, TEST_LOCATION);

  uint8_t* buffer = imageData.GetBuffer();

  //Change 0x0 pixel's rgb value as non-zero forcibly.
  buffer[0] = 0xff;
  buffer[1] = 0xf0;
  buffer[2] = 0x0f;
  buffer[3] = 0x11;

  DALI_TEST_EQUALS(imageData.IsAlphaPreMultiplied(), false, TEST_LOCATION);

  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[0]), 0xff, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[1]), 0xf0, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[2]), 0x0f, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[3]), 0x11, TEST_LOCATION);

  // Apply alpha pre-multiplication
  imageData.MultiplyColorByAlpha();

  DALI_TEST_EQUALS(imageData.IsAlphaPreMultiplied(), true, TEST_LOCATION);

  // Test that the buffer is still the same as the previous one.
  DALI_TEST_EQUALS(buffer, imageData.GetBuffer(), TEST_LOCATION);

  // Test alpha premultiplied
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[0]), 0x11, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[1]), 0x10, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[2]), 0x01, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[3]), 0x11, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferMultiplyColorByAlpha02(void)
{
  TestApplication application;

  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(10, 10, Pixel::RGB888);
  FillCheckerboard(imageData);

  DALI_TEST_EQUALS(imageData.GetWidth(), 10, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 10, TEST_LOCATION);

  uint8_t* buffer = imageData.GetBuffer();

  //Change 0x0 pixel's rgb value as non-zero forcibly.
  buffer[0] = 0xff;
  buffer[1] = 0xf0;
  buffer[2] = 0x0f;

  // Test that non-alpha channel item return false.
  DALI_TEST_EQUALS(imageData.IsAlphaPreMultiplied(), false, TEST_LOCATION);

  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[0]), 0xff, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[1]), 0xf0, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[2]), 0x0f, TEST_LOCATION);

  // Apply alpha pre-multiplication
  imageData.MultiplyColorByAlpha();

  // Test that non-alpha channel item return true after call MultiplyColorByAlpha.
  DALI_TEST_EQUALS(imageData.IsAlphaPreMultiplied(), true, TEST_LOCATION);

  // Test that the buffer is still the same as the previous one.
  DALI_TEST_EQUALS(buffer, imageData.GetBuffer(), TEST_LOCATION);

  // Test MultiplyColorByAlpha have no effect
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[0]), 0xff, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[1]), 0xf0, TEST_LOCATION);
  DALI_TEST_EQUALS(static_cast<uint32_t>(buffer[2]), 0x0f, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferMultiplyColorByAlpha03(void)
{
  TestApplication application;

  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(10, 10, Pixel::COMPRESSED_RGBA_ASTC_4x4_KHR);

  DALI_TEST_EQUALS(imageData.GetWidth(), 10, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 10, TEST_LOCATION);

  // For now, we cannot create compressed pixel buffer directly.

  // Test that alpha channel item return false.
  DALI_TEST_EQUALS(imageData.IsAlphaPreMultiplied(), false, TEST_LOCATION);

  // Apply alpha pre-multiplication
  imageData.MultiplyColorByAlpha();

  // Test compressed type have no effect.
  DALI_TEST_EQUALS(imageData.IsAlphaPreMultiplied(), false, TEST_LOCATION);

  END_TEST;
}

int UtcDaliPixelBufferMultiplyColorByAlpha04(void)
{
  TestApplication application;

  Devel::PixelBuffer imageData = Devel::PixelBuffer::New(10, 10, Pixel::COMPRESSED_RGB8_ETC2);

  DALI_TEST_EQUALS(imageData.GetWidth(), 10, TEST_LOCATION);
  DALI_TEST_EQUALS(imageData.GetHeight(), 10, TEST_LOCATION);

  // For now, we cannot create compressed pixel buffer directly.

  // Test that non-alpha channel item return false.
  DALI_TEST_EQUALS(imageData.IsAlphaPreMultiplied(), false, TEST_LOCATION);

  // Apply alpha pre-multiplication
  imageData.MultiplyColorByAlpha();

  // Test compressed type have no effect.
  DALI_TEST_EQUALS(imageData.IsAlphaPreMultiplied(), false, TEST_LOCATION);

  END_TEST;
}
