/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include <dali-test-suite-utils.h>

#include "platform-abstractions/portable/image-operations.h"

using namespace Dali::Internal::Platform;

namespace
{

/**
 * @brief Generate a random integer between zero and the parameter passed in.
 **/
uint32_t RandomInRange( uint32_t max )
{
  const uint32_t randToMax = lrand48() % (max + 1);
  return randToMax;
}

/**
 * @brief Random number representable in an 8 bit color component.
 */
inline uint32_t RandomComponent8()
{
  return RandomInRange( 255u );
}

/**
 * @brief Random number representable in a 5 bit color component.
 */
inline uint32_t RandomComponent5()
{
  return RandomInRange( 31u );
}

/**
 * @brief Random number representable in a 6 bit color component.
 */
inline uint32_t RandomComponent6()
{
  return RandomInRange( 63u );
}

/**
 * @brief RGBA8888 Pixels from separate color components.
 */
inline uint32_t PixelRGBA8888( uint32_t r, uint32_t g, uint32_t b, uint32_t a )
{
  return (r << 24) + (g << 16) + (b << 8) + a;
}

/**
 * @brief RGB565 Pixels from color components in the low bits of passed-in words.
 */
inline uint16_t PixelRGB565( uint32_t r, uint32_t g, uint32_t b )
{
  return (r << 11) + (g << 5) + b;
}

/**
 * @brief RGBA8888 Pixels with random color components.
 */
inline uint32_t RandomPixelRGBA8888( )
{
  const uint32_t randomPixel = PixelRGBA8888( RandomComponent8(), RandomComponent8(), RandomComponent8(), RandomComponent8() );
  return randomPixel;
}

/**
 * @brief Return a hash over a set of pixels.
 *
 * Used to check a buffer of pixels is unmodified by an operation given inputs
 * that should mean that it is not changed.
 */
inline uint32_t HashPixels( const uint32_t* const pixels, unsigned int numPixels )
{
  uint32_t hash = 5381;

  for( unsigned int i = 0; i < numPixels; ++i )
  {
    hash = hash * 33 + pixels[i];
  }

  return hash;
}

/**
 * @brief Build some dummy scanlines to exercise scanline averaging code on.
 */
void SetupScanlineForHalvingTestsRGBA8888( size_t scanlineLength, Dali::Vector<uint32_t>& scanline, Dali::Vector<uint32_t>& reference )
{
  scanline.Resize( scanlineLength );
  reference.Reserve( scanlineLength / 2 + 32 );

  // Prepare some random pixels:
  srand( 19 * 23 * 47 * 53 );
  for( size_t i = 0; i < scanlineLength / 2; ++i )
  {
    // Generate random colors:
    const uint32_t red1   = RandomComponent8();
    const uint32_t red2   = RandomComponent8();
    const uint32_t green1 = RandomComponent8();
    const uint32_t green2 = RandomComponent8();
    const uint32_t blue1  = RandomComponent8();
    const uint32_t blue2  = RandomComponent8();
    const uint32_t alpha1 = RandomComponent8();
    const uint32_t alpha2 = RandomComponent8();

    // The average of these pixels should equal the reference:
    scanline[i * 2]     = PixelRGBA8888( red1, green1, blue1, alpha1 );
    scanline[i * 2 + 1] = PixelRGBA8888( red2, green2, blue2, alpha2 );

    // Average the two pixels manually as a reference:
    reference.PushBack( PixelRGBA8888( (red1 + red2) >> 1u, (green1 + green2) >> 1u, (blue1 + blue2) >> 1u, (alpha1 + alpha2) >> 1u ) );
  }

  for( size_t i = scanlineLength / 2; i < reference.Capacity(); ++i )
  {
    reference[i] = 0xEEEEEEEE;
  }
}

/**
 * @brief Build some dummy scanlines to exercise scanline averaging code on.
 */
void SetupScanlineForHalvingTestsRGB565( size_t scanlineLength, Dali::Vector<uint16_t>& scanline, Dali::Vector<uint16_t>& reference )
{
  scanline.Resize( scanlineLength );
  reference.Reserve( scanlineLength / 2 + 32 );

  // Prepare some random pixels:
  srand48( 19 * 23 * 47 * 53 );
  for( size_t i = 0; i < scanlineLength / 2; ++i )
  {
    // Generate random colors:
    const uint32_t red1   = RandomComponent5();
    const uint32_t red2   = RandomComponent5();
    const uint32_t green1 = RandomComponent6();
    const uint32_t green2 = RandomComponent6();
    const uint32_t blue1  = RandomComponent5();
    const uint32_t blue2  = RandomComponent5();

    // The average of these pixels should equal the reference:
    scanline[i * 2]     = PixelRGB565( red1, green1, blue1 );
    scanline[i * 2 + 1] = PixelRGB565( red2, green2, blue2 );

    // Average the two pixels manually as a reference:
    reference.PushBack( PixelRGB565( (red1 + red2) >> 1u, (green1 + green2) >> 1u, (blue1 + blue2) >> 1u ) );
  }

  for( size_t i = scanlineLength / 2; i < reference.Capacity(); ++i )
  {
    reference[i] = 0xEEEE;
  }
}

/**
 * @brief Build some dummy scanlines to exercise scanline averaging code on.
 */
void SetupScanlineForHalvingTests2Bytes( size_t scanlineLength, Dali::Vector<uint8_t>& scanline, Dali::Vector<uint8_t>& reference )
{
  scanline.Resize( scanlineLength * 2 );
  reference.Reserve( scanlineLength + 32 );

  // Prepare some random pixels:
  srand48( 19 * 23 * 47 * 53 * 59 );
  for( size_t i = 0; i < scanlineLength / 2; ++i )
  {
    // Generate random colors:
    const uint32_t c11   = RandomComponent8();
    const uint32_t c12   = RandomComponent8();
    const uint32_t c21   = RandomComponent8();
    const uint32_t c22   = RandomComponent8();

    // The average of these pixels should equal the reference:
    scanline[i * 4]     = c11;
    scanline[i * 4 + 1] = c12;
    scanline[i * 4 + 2] = c21;
    scanline[i * 4 + 3] = c22;

    // Average the two pixels manually as a reference:
    reference.PushBack( (c11 + c21) >> 1u );
    reference.PushBack( (c12 + c22) >> 1u );
  }

  for( size_t i = scanlineLength; i < reference.Capacity(); ++i )
  {
    reference[i] = 0xEE;
  }
}

/**
 * @brief Build some dummy 1 byte per pixel scanlines to exercise scanline averaging code on.
 */
void SetupScanlineForHalvingTests1Byte( size_t scanlineLength, Dali::Vector<uint8_t>& scanline, Dali::Vector<uint8_t>& reference )
{
  scanline.Resize( scanlineLength * 2 );
  reference.Reserve( scanlineLength + 32 );

  // Prepare some random pixels:
  srand48( 19 * 23 * 47 * 53 * 63 );
  for( size_t i = 0; i < scanlineLength / 2; ++i )
  {
    // Generate random colors:
    const uint32_t c1 = RandomComponent8();
    const uint32_t c2 = RandomComponent8();

    // The average of these pixels should equal the reference:
    scanline[i * 2]     = c1;
    scanline[i * 2 + 1] = c2;

    // Average the two pixels manually as a reference:
    reference.PushBack( (c1 + c2) >> 1u );

  }

  for( size_t i = scanlineLength; i < reference.Capacity(); ++i )
  {
    reference[i] = 0xEE;
  }
}

/**
 * @brief Build some dummy scanlines to exercise vertical averaging code on.
 *
 * All tested formats bar RGB565 can share this setup.
 */
void SetupScanlinesRGBA8888( size_t scanlineLength, Dali::Vector<uint32_t>& scanline1, Dali::Vector<uint32_t>& scanline2, Dali::Vector<uint32_t>& reference, Dali::Vector<uint32_t>& output )
{
  scanline1.Reserve( scanlineLength );
  scanline2.Reserve( scanlineLength );
  reference.Reserve( scanlineLength + 32 );
  output.Reserve( scanlineLength + 32 );

  for( size_t i = scanlineLength; i < output.Capacity(); ++i )
  {
    output[i]    = 0xDEADBEEF;
    reference[i] = 0xDEADBEEF;
  }

  // Prepare some random pixels:
  srand48( 19 * 23 * 47 );
  for( size_t i = 0; i < scanlineLength; ++i )
  {
    // Generate random colors:
    const uint32_t red1   = RandomComponent8();
    const uint32_t red2   = RandomComponent8();
    const uint32_t green1 = RandomComponent8();
    const uint32_t green2 = RandomComponent8();
    const uint32_t blue1  = RandomComponent8();
    const uint32_t blue2  = RandomComponent8();
    const uint32_t alpha1 = RandomComponent8();
    const uint32_t alpha2 = RandomComponent8();

    // The average of these pixels should equal the reference:
    scanline1.PushBack( PixelRGBA8888( red1, green1, blue1, alpha1 ) );
    scanline2.PushBack( PixelRGBA8888( red2, green2, blue2, alpha2 ) );

    // Average the two pixels manually as a reference:
    reference.PushBack( PixelRGBA8888( (red1 + red2) >> 1u, (green1 + green2) >> 1u, (blue1 + blue2) >> 1u, (alpha1 + alpha2) >> 1u ) );
  }
}

/**
 * @brief Compares a scanline of interest to a reference, testing each pixel is the same.
 */
void MatchScanlinesRGBA8888( Dali::Vector<uint32_t>& reference, Dali::Vector<uint32_t>& output, size_t& numMatches, const char * const location )
{
  numMatches = 0;
  for( size_t i = 0, length = reference.Capacity(); i < length; ++i )
  {
    DALI_TEST_EQUALS( output[i], reference[i], location );
    numMatches += output[i] == reference[i];
  }
}

} //< namespace unnamed

/**
 * @brief Test component averaging code.
 */
int UtcDaliImageOperationsAverageComponent(void)
{
  DALI_TEST_EQUALS( Dali::Internal::Platform::AverageComponent( 0u, 0u ), 0u, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AverageComponent( 1u, 1u ), 1u, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AverageComponent( 0xffffffffu >> 1u, 0xffffffffu >> 1u ), 0xffffffffu >> 1u, TEST_LOCATION );
  const unsigned int avg3 = Dali::Internal::Platform::AverageComponent( 0xfffffffeu, 1u );
  DALI_TEST_EQUALS( avg3, 0x7fffffffu, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AverageComponent( 255u, 255u ), 255u, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AverageComponent( 512u, 0u ), 256u, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AverageComponent( 511u, 0u ), 255u, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AverageComponent( 510u, 0u ), 255u, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AverageComponent( 509u, 0u ), 254u, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AverageComponent( 0u, 509u ), 254u, TEST_LOCATION );
  END_TEST;
}

/**
 * @brief Test Pixel averaging code.
 */
int UtcDaliImageOperationsAveragePixelRGBA8888(void)
{
  DALI_TEST_EQUALS( Dali::Internal::Platform::AveragePixelRGBA8888( 0u, 0u ), 0u, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AveragePixelRGBA8888( 0x01010101, 0x01010101 ), 0x01010101u, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AveragePixelRGBA8888( 0x01010101, 0x03030303 ), 0x02020202u, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AveragePixelRGBA8888( 0xffffffff, 0xffffffff ), 0xffffffffu, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AveragePixelRGBA8888( 0xffffffff, 0u ), 0x7f7f7f7fu, TEST_LOCATION );
  END_TEST;
}

/**
 * @brief Test RGBA565 pixel averaging function.
 */
int UtcDaliImageOperationsAveragePixelRGB565(void)
{
  DALI_TEST_EQUALS( Dali::Internal::Platform::AveragePixelRGB565( 0u, 0u ), 0u, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AveragePixelRGB565( 0xf800u, 0xf800u ), 0xf800u, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AveragePixelRGB565( 0xf800u, 0x800u ), 1u << 15, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AveragePixelRGB565( 0x7e0u, 0x7e0u ), 0x7e0u, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AveragePixelRGB565( 0x7e0u, 0x20u ), 1u << 10, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AveragePixelRGB565( 0x1f, 0x1f ), 0x1fu, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AveragePixelRGB565( 0x1f, 0x1 ), 1u << 4, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AveragePixelRGB565( 0xf800u, 0x7e0u ), 0x7800u + 0x3e0u, TEST_LOCATION );
  DALI_TEST_EQUALS( Dali::Internal::Platform::AveragePixelRGB565( 0xffff, 0xffff ), 0xffffu, TEST_LOCATION );
  END_TEST;
}

/**
 * @brief Build a square bitmap, downscale it and assert the resulting bitmap has the right dimensions.
 */
void TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::Format format, uint32_t sourceDimension, uint32_t targetDimension, uint32_t expectedDimension, const char * const location )
{
  ImageAttributes attributes;
  attributes.SetScalingMode( ImageAttributes::ShrinkToFit );
  attributes.SetSize( targetDimension, targetDimension );

  Integration::BitmapPtr sourceBitmap = Integration::Bitmap::New( Integration::Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::DISCARD );
  sourceBitmap->GetPackedPixelsProfile()->ReserveBuffer( format, sourceDimension, sourceDimension, sourceDimension, sourceDimension );

  Integration::BitmapPtr downScaled = DownscaleBitmap( *sourceBitmap, attributes );
  DALI_TEST_EQUALS( downScaled->GetImageWidth(), expectedDimension, location );
  DALI_TEST_EQUALS( downScaled->GetImageHeight(), expectedDimension, location );
  DALI_TEST_EQUALS( downScaled->GetPixelFormat(), format, location );
}

/**
 * @brief Test the top-level function for reducing the dimension of a bitmap,
 * feeding it each of the five pixel formats that are output by image loaders.
 * Simply assert that the resulting bitmaps have the expected dimensions and
 * formats.
 */
int UtcDaliImageOperationsDownscaleBitmap(void)
{
  // Do Scalings that are expected to work for all pixels modes and assert the resulting bitmap dimensions:

  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::RGBA8888, 1024, 8, 8, TEST_LOCATION );
  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::RGB888, 1024, 8, 8, TEST_LOCATION );
  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::RGB565, 1024, 8, 8, TEST_LOCATION );
  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::LA88, 1024, 8, 8, TEST_LOCATION );
  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::L8, 1024, 8, 8, TEST_LOCATION );

  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::RGBA8888, 773, 1, 1, TEST_LOCATION );
  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::RGB888, 787, 1, 1, TEST_LOCATION );
  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::RGB565, 797, 1, 1, TEST_LOCATION );
  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::LA88, 809, 1, 1, TEST_LOCATION );
  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::L8, 811, 1, 1, TEST_LOCATION );

  // Do Scalings that are expected to produce a slightly larger than requested image:
  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::RGBA8888, 47, 7, 11, TEST_LOCATION );
  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::RGB888, 73, 17, 18, TEST_LOCATION );
  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::RGB565, 61, 8, 15, TEST_LOCATION );
  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::LA88, 19, 5, 9, TEST_LOCATION );
  TestDownscaledBitmapHasRightDimensionsAndFormat( Pixel::L8, 353, 23, 44, TEST_LOCATION );

  END_TEST;
}

/**
 * @brief Test downscaling of RGB888 images as raw pixel arrays.
 */
int UtcDaliImageOperationsDownscaleInPlacePow2RGB888(void)
{
  unsigned outWidth = -1, outHeight = -1;

  // Do downscaling to 1 x 1 so we can easily assert the value of the single pixel produced:

  // Scale down a black/white checkerboard to mid-grey:
  unsigned char check_4x4 [16 * 3] = {
      0xff, 0xff, 0xff,  0x00, 0x00, 0x00,  0xff, 0xff, 0xff,  0x00, 0x00, 0x00,
      0x00, 0x00, 0x00,  0xff, 0xff, 0xff,  0x00, 0x00, 0x00,  0xff, 0xff, 0xff,
      0xff, 0xff, 0xff,  0x00, 0x00, 0x00,  0xff, 0xff, 0xff,  0x00, 0x00, 0x00,
      0x00, 0x00, 0x00,  0xff, 0xff, 0xff,  0x00, 0x00, 0x00,  0xff, 0xff, 0xff
  };

  Dali::Internal::Platform::DownscaleInPlacePow2RGB888(check_4x4, 4, 4, 1, 1, BoxDimensionTestBoth, outWidth, outHeight );
  DALI_TEST_EQUALS( outWidth, 1u, TEST_LOCATION );
  DALI_TEST_EQUALS( outHeight, 1u, TEST_LOCATION );
  DALI_TEST_EQUALS( check_4x4[0], 0x7f, TEST_LOCATION );

  // Scale down a 16 pixel black image with a single white pixel to a 1/16th grey single pixel:
  unsigned char single_4x4 [16 * 3] = {
    0xff, 0xff, 0xff,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00
  };
  Dali::Internal::Platform::DownscaleInPlacePow2RGB888(single_4x4, 4, 4, 1, 1, BoxDimensionTestBoth, outWidth, outHeight );
  DALI_TEST_EQUALS( outWidth, 1u, TEST_LOCATION );
  DALI_TEST_EQUALS( outHeight, 1u, TEST_LOCATION );
  DALI_TEST_EQUALS( single_4x4[0], 0xf, TEST_LOCATION );

  // Scale down a 16 pixel black image with a single white pixel to a 1/16th grey single pixel:
  // (white pixel at bottom-right of image)
  unsigned char single_4x4_2 [16 * 3] = {
      0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
      0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
      0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
      0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0xff, 0xff, 0xff
    };
  Dali::Internal::Platform::DownscaleInPlacePow2RGB888(single_4x4_2, 4, 4, 1, 1, BoxDimensionTestBoth, outWidth, outHeight );
  DALI_TEST_EQUALS( outWidth, 1u, TEST_LOCATION );
  DALI_TEST_EQUALS( outHeight, 1u, TEST_LOCATION );
  DALI_TEST_EQUALS( single_4x4_2[0], 0xf, TEST_LOCATION );

  // Build a larger ~600 x ~600 uniform magenta image for tests which only test output dimensions:

  unsigned char magenta_600_x_600[608*608 * 3];
  for( unsigned int i = 0; i < sizeof(magenta_600_x_600); i += 3 )
  {
    magenta_600_x_600[i] = 0xff;
    magenta_600_x_600[i + 1] = 0;
    magenta_600_x_600[i + 2] = 0xff;
  }

  // Scaling to 0 x 0 should stop at 1 x 1:
  Dali::Internal::Platform::DownscaleInPlacePow2RGB888( magenta_600_x_600, 352, 352, 0, 0, BoxDimensionTestBoth, outWidth, outHeight );
  DALI_TEST_EQUALS( outWidth, 1u, TEST_LOCATION );
  DALI_TEST_EQUALS( outHeight, 1u, TEST_LOCATION );

  // Scaling to 1 x 1 should hit 1 x 1:
  Dali::Internal::Platform::DownscaleInPlacePow2RGB888( magenta_600_x_600, 608, 608, 1, 1, BoxDimensionTestBoth, outWidth, outHeight );
  DALI_TEST_EQUALS( outWidth, 1u, TEST_LOCATION );
  DALI_TEST_EQUALS( outHeight, 1u, TEST_LOCATION );

  // Scaling to original dimensions should NOP:
  Dali::Internal::Platform::DownscaleInPlacePow2RGB888( magenta_600_x_600, 384, 384, 384, 384, BoxDimensionTestBoth, outWidth, outHeight );
  DALI_TEST_EQUALS( outWidth, 384u, TEST_LOCATION );
  DALI_TEST_EQUALS( outHeight, 384u, TEST_LOCATION );

  // More dimension tests:

  Dali::Internal::Platform::DownscaleInPlacePow2RGB888( magenta_600_x_600, 352, 352, 44, 11, BoxDimensionTestBoth, outWidth, outHeight );
  DALI_TEST_EQUALS( outWidth, 44u, TEST_LOCATION );
  DALI_TEST_EQUALS( outHeight, 44u, TEST_LOCATION );

  Dali::Internal::Platform::DownscaleInPlacePow2RGB888( magenta_600_x_600, 384, 384, 3, 48, BoxDimensionTestBoth, outWidth, outHeight );
  DALI_TEST_EQUALS( outWidth, 48u, TEST_LOCATION );
  DALI_TEST_EQUALS( outHeight, 48u, TEST_LOCATION );

  Dali::Internal::Platform::DownscaleInPlacePow2RGB888( magenta_600_x_600, 384, 384, 3, 3, BoxDimensionTestBoth, outWidth, outHeight );
  DALI_TEST_CHECK( outWidth == 3u && outHeight == 3u );

  Dali::Internal::Platform::DownscaleInPlacePow2RGB888( magenta_600_x_600, 320, 320, 5, 5, BoxDimensionTestBoth, outWidth, outHeight );
  DALI_TEST_CHECK( outWidth == 5u && outHeight == 5u );

  Dali::Internal::Platform::DownscaleInPlacePow2RGB888( magenta_600_x_600, 448, 448, 7, 7, BoxDimensionTestBoth, outWidth, outHeight );
  DALI_TEST_CHECK( outWidth == 7u && outHeight == 7u );

  Dali::Internal::Platform::DownscaleInPlacePow2RGB888( magenta_600_x_600, 352, 352, 11, 11, BoxDimensionTestBoth, outWidth, outHeight );
  DALI_TEST_CHECK( outWidth == 11u && outHeight == 11u );

  // Check that no pixel values were modified by the repeated averaging of identical pixels in tests above:
  unsigned int numNonMagenta = 0u;
  for( unsigned i = 0; i < sizeof(magenta_600_x_600); i += 3 )
  {
    numNonMagenta += magenta_600_x_600[i] == 0xff && magenta_600_x_600[i + 1] == 0x00 && magenta_600_x_600[i + 2] == 0xff ? 0 : 1;
  }
  DALI_TEST_EQUALS( numNonMagenta, 0u, TEST_LOCATION );

  END_TEST;
}

/**
 * @brief Test that resizing RGBA8888 images as raw pixel arrays produces a result of the correct dimensions.
 */
void TestDownscaleOutputsExpectedDimensionsRGBA8888( uint32_t pixels[], unsigned inputWidth, unsigned inputHeight, unsigned int desiredWidth, unsigned int desiredHeight, unsigned int expectedWidth, unsigned int expectedHeight, const char * const location )
{
  unsigned int resultingWidth = -1, resultingHeight = -1;
  Dali::Internal::Platform::DownscaleInPlacePow2RGBA8888(
      reinterpret_cast<unsigned char *> (pixels),
      inputWidth, inputHeight,
      desiredWidth, desiredHeight, BoxDimensionTestBoth,
      resultingWidth, resultingHeight );

  DALI_TEST_EQUALS( resultingWidth, expectedWidth, location );
  DALI_TEST_EQUALS( resultingHeight, expectedHeight, location );
}

/**
 * @brief Test that resizing RGB565 images as raw pixel arrays produces a result of the correct dimensions.
 */
void TestDownscaleOutputsExpectedDimensionsRGB565( uint16_t pixels[], unsigned inputWidth, unsigned inputHeight, unsigned int desiredWidth, unsigned int desiredHeight, unsigned int expectedWidth, unsigned int expectedHeight, const char * const location )
{
  unsigned int resultingWidth = -1, resultingHeight = -1;
  Dali::Internal::Platform::DownscaleInPlacePow2RGB565(
      reinterpret_cast<unsigned char *> (pixels),
      inputWidth, inputHeight,
      desiredWidth, desiredHeight, BoxDimensionTestBoth,
      resultingWidth, resultingHeight );

  DALI_TEST_EQUALS( resultingWidth, expectedWidth, location );
  DALI_TEST_EQUALS( resultingHeight, expectedHeight, location );
}

/**
 * @brief Test that resizing 2-byte-per-pixel images as raw pixel arrays produces a result of the correct dimensions.
 */
void TestDownscaleOutputsExpectedDimensions2ComponentPair( uint8_t pixels[], unsigned inputWidth, unsigned inputHeight, unsigned int desiredWidth, unsigned int desiredHeight, unsigned int expectedWidth, unsigned int expectedHeight, const char * const location )
{
  unsigned int resultingWidth = -1, resultingHeight = -1;
  Dali::Internal::Platform::DownscaleInPlacePow2ComponentPair(
      pixels,
      inputWidth, inputHeight,
      desiredWidth, desiredHeight, BoxDimensionTestBoth,
      resultingWidth, resultingHeight );

  DALI_TEST_EQUALS( resultingWidth, expectedWidth, location );
  DALI_TEST_EQUALS( resultingHeight, expectedHeight, location );
}

/**
 * @brief Test that resizing single-byte-per-pixel images as raw pixel arrays produces a result of the correct dimensions.
 */
void TestDownscaleOutputsExpectedDimensionsSingleComponent( uint8_t pixels[], unsigned inputWidth, unsigned inputHeight, unsigned int desiredWidth, unsigned int desiredHeight, unsigned int expectedWidth, unsigned int expectedHeight, const char * const location )
{
  unsigned int resultingWidth = -1, resultingHeight = -1;
  Dali::Internal::Platform::DownscaleInPlacePow2SingleBytePerPixel(
      pixels,
      inputWidth, inputHeight,
      desiredWidth, desiredHeight, BoxDimensionTestBoth,
      resultingWidth, resultingHeight );

  DALI_TEST_EQUALS( resultingWidth, expectedWidth, location );
  DALI_TEST_EQUALS( resultingHeight, expectedHeight, location );
}

/**
 * @brief Test downscaling of RGBA8888 images in raw image arrays.
 */
int UtcDaliImageOperationsDownscaleInPlacePow2RGBA8888(void)
{
  uint32_t image[608*608];
  for( unsigned i = 0; i < sizeof(image) / sizeof(image[0]); ++i )
  {
    image[i] = 0xffffffff;
  }
  unsigned char* const pixels = reinterpret_cast<unsigned char *> (image);
  unsigned int resultingWidth = -1, resultingHeight = -1;

  // Test downscaling where the input size is an exact multiple of the desired size:
  // (We expect a perfect result here)

  DownscaleInPlacePow2RGBA8888( pixels, 600, 600, 75, 75, BoxDimensionTestBoth, resultingWidth, resultingHeight );
  DALI_TEST_EQUALS( resultingWidth, 75u, TEST_LOCATION );
  DALI_TEST_EQUALS( resultingHeight, 75u, TEST_LOCATION );

  DownscaleInPlacePow2RGBA8888( pixels, 512, 512, 16, 16, BoxDimensionTestBoth, resultingWidth, resultingHeight );
  DALI_TEST_EQUALS( resultingWidth, 16u, TEST_LOCATION );
  DALI_TEST_EQUALS( resultingHeight, 16u, TEST_LOCATION );

  DownscaleInPlacePow2RGBA8888( pixels, 512, 64, 16, 2, BoxDimensionTestBoth, resultingWidth, resultingHeight );
  DALI_TEST_EQUALS( resultingWidth, 16u, TEST_LOCATION  );
  DALI_TEST_EQUALS( resultingHeight, 2u, TEST_LOCATION );

  DownscaleInPlacePow2RGBA8888( pixels, 64, 1024, 4, 64, BoxDimensionTestBoth, resultingWidth, resultingHeight );
  DALI_TEST_EQUALS( resultingWidth, 4u, TEST_LOCATION  );
  DALI_TEST_EQUALS( resultingHeight, 64u, TEST_LOCATION );

  // Test downscaling where the input size is slightly off being an exact multiple of the desired size:
  // (We expect a perfect match at the end because of rounding-down to an even width and height at each step)

  DownscaleInPlacePow2RGBA8888( pixels, 601, 603, 75, 75, BoxDimensionTestBoth, resultingWidth, resultingHeight );
  DALI_TEST_EQUALS( resultingWidth, 75u, TEST_LOCATION  );
  DALI_TEST_EQUALS( resultingHeight, 75u, TEST_LOCATION );

  DownscaleInPlacePow2RGBA8888( pixels, 736 + 1, 352 + 3, 23, 11, BoxDimensionTestBoth, resultingWidth, resultingHeight );
  DALI_TEST_EQUALS( resultingWidth, 23u, TEST_LOCATION  );
  DALI_TEST_EQUALS( resultingHeight, 11u, TEST_LOCATION );

  DownscaleInPlacePow2RGBA8888( pixels, 384 + 3, 896 + 1, 3, 7, BoxDimensionTestBoth, resultingWidth, resultingHeight );
  DALI_TEST_EQUALS( resultingWidth, 3u, TEST_LOCATION  );
  DALI_TEST_EQUALS( resultingHeight, 7u, TEST_LOCATION );

  // Test downscales with source dimensions which are under a nice power of two by one:

  // The target is hit exactly due to losing spare columns or rows at each iteration:
  DownscaleInPlacePow2RGBA8888( pixels, 63, 31, 7, 3, BoxDimensionTestBoth, resultingWidth, resultingHeight );
  DALI_TEST_EQUALS( resultingWidth, 7u, TEST_LOCATION  );
  DALI_TEST_EQUALS( resultingHeight, 3u, TEST_LOCATION );

  // Asking to downscale a bit smaller should stop at the dimensions of the last test as one more halving would go down to 3 x 1, which is too small.
  DownscaleInPlacePow2RGBA8888( pixels, 63, 31, 4, 2, BoxDimensionTestBoth, resultingWidth, resultingHeight );
  DALI_TEST_EQUALS( resultingWidth, 7u, TEST_LOCATION  );
  DALI_TEST_EQUALS( resultingHeight, 3u, TEST_LOCATION );

  // Should stop at almost twice the requested dimensions:
  DownscaleInPlacePow2RGBA8888( pixels, 15, 127, 4, 32, BoxDimensionTestBoth, resultingWidth, resultingHeight );
  DALI_TEST_EQUALS( resultingWidth, 7u, TEST_LOCATION  );
  DALI_TEST_EQUALS( resultingHeight, 63u, TEST_LOCATION );

  // Test downscales to 1 in one or both dimensions:
  // Parameters:                                         input-x  input-y, desired-x, desired-y, expected-x, expected-y
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 512,     512,     1,         1,         1,          1,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 512,     32,      16,        1,         16,         1,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 512,     32,      7,         1,         16,         1,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 512,     32,      7,         1,         16,         1,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 512,     32,      5,         1,         16,         1,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 512,     32,      3,         1,         16,         1,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 32,      512,     1,         1,         1,          16,        TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 32,      512,     1,         16,        1,          16,        TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 32,      512,     1,         3,         1,          16,        TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 33,      33,      1,         1,         1,          1,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 17*19,   17*19,   1,         1,         1,          1,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 33,      33,      3,         1,         4,          4,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 33,      9,       3,         1,         4,          1,         TEST_LOCATION );



  // Test downscales to zero in one or both dimensions:
  // Scaling should stop when one or both dimensions reach 1.
  // Parameters:                                         input-x  input-y, desired-x, desired-y, expected-x, expected-y
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 512,     512,     0,         0,         1,          1,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 512,     256,     0,         0,         2,          1,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 512,     128,     0,         0,         4,          1,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 512,     16,      0,         0,         32,         1,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 128,     512,     0,         0,         1,          4,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 32,      512,     0,         0,         1,          16,        TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 8,       512,     0,         0,         1,          64,        TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 2,       512,     0,         0,         1,          256,       TEST_LOCATION );

  END_TEST;
}

/**
 * @brief Test downscalings of RGBA8888 images in raw image arrays that should have no effect on the input.
 */
int UtcDaliImageOperationsDownscaleInPlacePow2RGBA8888Nops(void)
{
  uint32_t image[608*608];
  const uint32_t numPixels = sizeof(image) / sizeof(image[0]);
  for( unsigned i = 0; i < numPixels; ++i )
  {
    image[i] = RandomPixelRGBA8888();
  }
  const uint32_t imageHash = HashPixels( image, numPixels );
  unsigned char* const pixels = reinterpret_cast<unsigned char *> (image);
  unsigned int resultingWidth = -1, resultingHeight = -1;

  // Test downscales to the same size:
  // The point is just to be sure the downscale is a NOP in this case:

  DownscaleInPlacePow2RGBA8888( pixels, 600, 600, 600, 600, BoxDimensionTestBoth, resultingWidth, resultingHeight );
  DALI_TEST_EQUALS( resultingWidth, 600u, TEST_LOCATION );
  DALI_TEST_EQUALS( resultingHeight, 600u, TEST_LOCATION );

  DownscaleInPlacePow2RGBA8888( pixels, 512, 128, 512, 128, BoxDimensionTestBoth, resultingWidth, resultingHeight );
  DALI_TEST_EQUALS( resultingWidth, 512u, TEST_LOCATION );
  DALI_TEST_EQUALS( resultingHeight, 128u, TEST_LOCATION );

  DownscaleInPlacePow2RGBA8888( pixels, 17, 1001, 17, 1001, BoxDimensionTestBoth, resultingWidth, resultingHeight );
  DALI_TEST_EQUALS( resultingWidth, 17u, TEST_LOCATION );
  DALI_TEST_EQUALS( resultingHeight, 1001u, TEST_LOCATION );

  // Test downscales that request a larger size (we never upscale so these are NOPs too):
  // Parameters:                                         input-x  input-y, desired-x, desired-y, expected-x, expected-y
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 300,     300,     600,       600,       300,        300,       TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 3,       127,     99,        599,       3,          127,       TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGBA8888( image, 600,     600,     999,       999,       600,        600,       TEST_LOCATION ); //< checks no out of bounds mem access in this case


  // Make sure that none of these NOP downscalings has affected the pixels of the image:
  DALI_TEST_EQUALS( HashPixels( image, numPixels ), imageHash, TEST_LOCATION );

  END_TEST;
}

/**
 * @brief Do additional downscaling testing using RGB565 images in raw image
 * arrays to shake out differences relating to the pixel format.
 */
int UtcDaliImageOperationsDownscaleInPlacePow2RGB565(void)
{
  // Test that calling with null and zero parameters doesn't blow up:
  unsigned int outWidth, outHeight;
  DownscaleInPlacePow2RGB565( 0, 0, 0, 0, 0, BoxDimensionTestBoth, outWidth, outHeight );

  uint16_t image[608*608];
  for( unsigned i = 0; i < sizeof(image) / sizeof(image[0]); ++i )
  {
    image[i] = 0xffff;
  }

  // Do a straightforward test using an exact divisor target size:
  TestDownscaleOutputsExpectedDimensionsRGB565( image, 600, 600, 75, 75, 75, 75, TEST_LOCATION );
  // Test that a slightly smaller than possible to achieve target results in the
  // next-higher exact divisor output image dimensions:
  TestDownscaleOutputsExpectedDimensionsRGB565( image, 600, 600, 71, 69, 75, 75, TEST_LOCATION );
  // Test that resizing from a starting size that is slightly larger than an exact
  // multiple of the desired dimensions still results in the desired ones being
  // reached:
  // Parameters:                                       input-x  input-y, desired-x, desired-y, expected-x, expected-y
  TestDownscaleOutputsExpectedDimensionsRGB565( image, 600 + 1, 600 + 1, 75,        75,        75,         75,        TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGB565( image, 256 + 1, 512 + 1, 2,         4,         2,          4,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGB565( image, 512 + 1, 128 + 1, 16,        4,         16,         4,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGB565( image, 512 + 1, 64  + 1, 16,        2,         16,         2,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGB565( image, 512 + 3, 512 + 3, 16,        16,        16,         16,        TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGB565( image, 512 + 3, 256 + 3, 16,        8,         16,         8,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGB565( image, 256 + 3, 512 + 3, 4,         8,         4,          8,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGB565( image, 256 + 7, 512 + 7, 4,         8,         4,          8,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGB565( image, 256 + 7, 512 + 7, 2,         4,         2,          4,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGB565( image, 512 + 7, 128 + 7, 16,        4,         16,         4,         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsRGB565( image, 512 + 7, 64  + 7, 16,        2,         16,         2,         TEST_LOCATION );


  END_TEST;
}

/**
 * @brief Do additional downscaling testing using 2-byte-per-pixel images in
 * raw image arrays to shake out differences relating to the pixel format.
 */
int UtcDaliImageOperationsDownscaleInPlacePow2ComponentPair(void)
{
  // Simple test that a null pointer does not get dereferenced in the function:
  unsigned int outWidth, outHeight;
  DownscaleInPlacePow2ComponentPair( 0, 0, 0, 0, 0, BoxDimensionTestBoth, outWidth, outHeight );

  // Simple tests of dimensions output:

  uint8_t image[608*608*2];
  for( unsigned i = 0; i < sizeof(image) / sizeof(image[0]); ++i )
  {
    image[i] = 0xff;
  }

  TestDownscaleOutputsExpectedDimensions2ComponentPair( image,
                                                        600, 600, //< Input dimensions
                                                        37, 37,   //< Requested dimensions
                                                        37, 37,   //< Expected output dimensions
                                                        TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensions2ComponentPair( image,
                                                        600, 600, //< Input dimensions
                                                        34, 35,   //< Requested dimensions to scale-down to
                                                        37, 37,   //< Expected output dimensions achieved
                                                        TEST_LOCATION );
  ///@note: No need to be as comprehensive as with RGB888 and RGBA8888 as the logic is shared.

  END_TEST;
}

/**
 * @brief Do additional downscaling testing using 1-byte-per-pixel images in
 * raw image arrays to shake out differences relating to the pixel format.
 */
int UtcDaliImageOperationsDownscaleInPlacePow2SingleBytePerPixel(void)
{
  // Simple test that a null pointer does not get dereferenced in the function:
  unsigned int outWidth, outHeight;
  DownscaleInPlacePow2SingleBytePerPixel( 0, 0, 0, 0, 0, BoxDimensionTestBoth, outWidth, outHeight );

  // Tests of output dimensions from downscaling:
  uint8_t image[608*608];
  for( unsigned i = 0; i < sizeof(image) / sizeof(image[0]); ++i )
  {
    image[i] = 0xff;
  }

  TestDownscaleOutputsExpectedDimensionsSingleComponent( image,
                                                         600, 300,  //< Input dimensions
                                                         150, 75,   //< Requested dimensions to scale-down to
                                                         150, 75,   //< Expected output dimensions achieved
                                                         TEST_LOCATION );
  TestDownscaleOutputsExpectedDimensionsSingleComponent( image, 577, 411, 142, 99, 144, 102, TEST_LOCATION );

  END_TEST;
}

/**
 * @brief Test the function for averaging pairs of pixels on a scanline.
 */
int UtcDaliImageOperationsHalveScanlineInPlaceRGB888(void)
{
  // Red and cyan, averaging to grey:
  unsigned char shortEven[] =    { 0xff, 0, 0,   0, 0xff, 0xff,   0xff, 0, 0,   0, 0xff, 0xff };
  unsigned char shortOdd[] =     { 0xff, 0, 0,  0, 0xff, 0xff,  0xff, 0, 0,  0, 0xff, 0xff,  0xC, 0xC, 0xC };

  Dali::Internal::Platform::HalveScanlineInPlaceRGB888( shortEven, 4u );
  Dali::Internal::Platform::HalveScanlineInPlaceRGB888( shortOdd, 4u );
  for( unsigned i = 0; i < sizeof(shortEven) >> 1u ; ++i )
  {
    DALI_TEST_EQUALS( unsigned(shortEven[i]), 0x7fu, TEST_LOCATION );
    DALI_TEST_EQUALS( unsigned(shortOdd[i]), 0x7fu, TEST_LOCATION );
  }

  END_TEST;
}

/**
 * @brief Test the function for averaging pairs of pixels on a scanline.
 */
int UtcDaliImageOperationsHalveScanlineInPlaceRGBA8888(void)
{
  const size_t scanlineLength = 4096u;
  Dali::Vector<uint32_t> scanline;
  Dali::Vector<uint32_t> reference;
  SetupScanlineForHalvingTestsRGBA8888( scanlineLength, scanline, reference );

  HalveScanlineInPlaceRGBA8888( (uint8_t *) &scanline[0], scanlineLength );

  // Check that the halving matches the independently calculated reference:
  size_t numMatches = 0;
  for( int i = 0, length = reference.Size(); i < length; ++i )
  {
    DALI_TEST_EQUALS( scanline[i], reference[i], TEST_LOCATION );
    numMatches += scanline[i] == reference[i];
  }
  DALI_TEST_EQUALS( numMatches, scanlineLength / 2, TEST_LOCATION );

  // Test for no beyond-bounds writes:
  for( size_t i = scanlineLength / 2; i < reference.Capacity(); ++i )
  {
    DALI_TEST_EQUALS( reference[i],  0xEEEEEEEE, TEST_LOCATION );
  }

  END_TEST;
}

/**
 * @brief Test the function for averaging pairs of pixels on a scanline.
 */
int UtcDaliImageOperationsHalveScanlineInPlaceRGB565(void)
{
  const size_t scanlineLength = 4096u;
  Dali::Vector<uint16_t> scanline;
  Dali::Vector<uint16_t> reference;
  SetupScanlineForHalvingTestsRGB565( scanlineLength, scanline, reference );

  HalveScanlineInPlaceRGB565( (unsigned char *) (&scanline[0]), scanlineLength );

  // Check output against reference:
  size_t numMatches = 0;
  for( int i = 0, length = reference.Size(); i < length; ++i )
  {
    DALI_TEST_EQUALS( scanline[i], reference[i], TEST_LOCATION );
    numMatches += scanline[i] == reference[i];
  }
  DALI_TEST_EQUALS( numMatches, scanlineLength / 2, TEST_LOCATION );

  // Test for no beyond-bounds writes:
  for( size_t i = scanlineLength / 2; i < reference.Capacity(); ++i )
  {
    DALI_TEST_EQUALS( reference[i],  0xEEEE, TEST_LOCATION );
  }

  END_TEST;
}

/**
 * @brief Test the function for averaging pairs of pixels on a scanline.
 */
int UtcDaliImageOperationsHalveScanlineInPlace2Bytes(void)
{
  const size_t scanlineLength = 4096u;
  Dali::Vector<uint8_t> scanline;
  Dali::Vector<uint8_t> reference;
  SetupScanlineForHalvingTests2Bytes( scanlineLength, scanline, reference );

  HalveScanlineInPlace2Bytes( &scanline[0], scanlineLength );

  // Test the output against the reference (no differences):
  size_t numMatches = 0;
  for( int i = 0, length = reference.Size(); i < length; ++i )
  {
    DALI_TEST_EQUALS( 1u * scanline[i], 1u * reference[i], TEST_LOCATION );
    numMatches += scanline[i] == reference[i];
  }
  // The number of matching bytes should be double the number of pixels, which happens to be the original scanline length in pixels:
  DALI_TEST_EQUALS( numMatches, scanlineLength, TEST_LOCATION );

  END_TEST;
}

/**
 * @brief Test the function for averaging pairs of pixels on a scanline.
 */
int UtcDaliImageOperationsHalveScanlineInPlace1Byte(void)
{
  const size_t scanlineLength = 4096u;
  Dali::Vector<uint8_t> scanline;
  Dali::Vector<uint8_t> reference;
  SetupScanlineForHalvingTests1Byte( scanlineLength, scanline, reference );

  HalveScanlineInPlace1Byte( &scanline[0], scanlineLength );

  // Test the reference matches the output:
  size_t numMatches = 0;
  for( int i = 0, length = reference.Size(); i < length; ++i )
  {
    DALI_TEST_EQUALS( 1u * scanline[i], 1u * reference[i], TEST_LOCATION );
    numMatches += scanline[i] == reference[i];
  }
  DALI_TEST_EQUALS( numMatches, scanlineLength / 2, TEST_LOCATION );

  END_TEST;
}

/**
 * @brief Test the function for averaging vertically-adjacent pairs of single-byte-per-pixel pixels on a scanline.
 */
int UtcDaliImageOperationsAverageScanlines1(void)
{
  // Red and cyan, averaging to grey:
  unsigned char shortEven1[] =    { 0xff, 0, 0,    0, 0xff, 0xff,  0xff, 0, 0,      0, 0xff, 0xff };
  unsigned char shortEven2[] =    { 0, 0xff, 0xff, 0xff, 0, 0,     0, 0xff,  0xff,  0xff, 0, 0 };
  unsigned char outputBuffer[sizeof(shortEven1)];

  AverageScanlines1( shortEven1, shortEven2, outputBuffer, sizeof(shortEven1) );
  for( unsigned i = 0; i < sizeof(shortEven1) ; ++i )
  {
    DALI_TEST_EQUALS( unsigned(outputBuffer[i]), 0x7fu, TEST_LOCATION );
  }

  // Longer test reusing RGBA setup/test logic:
  const size_t scanlineLength = 4096u;
  Dali::Vector<uint32_t> scanline1;
  Dali::Vector<uint32_t> scanline2;
  Dali::Vector<uint32_t> reference;
  Dali::Vector<uint32_t> output;
  SetupScanlinesRGBA8888( scanlineLength, scanline1, scanline2, reference, output );

  AverageScanlines1( (const unsigned char*) &scanline1[0], (const unsigned char*) &scanline2[0], (unsigned char*) &output[0], scanlineLength * 4 );

  // Check the output matches the independently generated reference:
  size_t numMatches = 0;
  MatchScanlinesRGBA8888( reference, output, numMatches, TEST_LOCATION );
  DALI_TEST_EQUALS( numMatches, reference.Capacity(), TEST_LOCATION );

  END_TEST;
}

/**
 * @brief Test the function for averaging vertically-adjacent pairs of 2-byte-per-pixel pixels on a scanline.
 */
int UtcDaliImageOperationsAverageScanlines2(void)
{
  // Red and cyan, averaging to grey:
  unsigned char shortEven1[] =    { 0xff, 0, 0,    0, 0xff, 0xff,  0xff, 0, 0,      0, 0xff, 0xff };
  unsigned char shortEven2[] =    { 0, 0xff, 0xff, 0xff, 0, 0,     0, 0xff,  0xff,  0xff, 0, 0 };
  unsigned char outputBuffer[sizeof(shortEven1)];

  AverageScanlines2( shortEven1, shortEven2, outputBuffer, sizeof(shortEven1) / 2 );

  for( unsigned i = 0; i < sizeof(shortEven1); ++i )
  {
    DALI_TEST_EQUALS( unsigned(outputBuffer[i]), 0x7fu, TEST_LOCATION );
  }

  // Longer test reusing RGBA setup/test logic:
  const size_t scanlineLength = 4096u;
  Dali::Vector<uint32_t> scanline1;
  Dali::Vector<uint32_t> scanline2;
  Dali::Vector<uint32_t> reference;
  Dali::Vector<uint32_t> output;
  SetupScanlinesRGBA8888( scanlineLength, scanline1, scanline2, reference, output );

  AverageScanlines2( (const unsigned char*) &scanline1[0], (const unsigned char*) &scanline2[0], (unsigned char*) &output[0], scanlineLength * 2 );

  // Check the output matches the independently generated reference:
  size_t numMatches = 0;
  MatchScanlinesRGBA8888( reference, output, numMatches, TEST_LOCATION );
  DALI_TEST_EQUALS( numMatches, reference.Capacity(), TEST_LOCATION );

  END_TEST;
}

/**
 * @brief Test the function for averaging vertically-adjacent pairs of RGB888 pixels on a scanline.
 */
int UtcDaliImageOperationsAverageScanlines3(void)
{
  // Red and cyan, averaging to grey:
  unsigned char shortEven1[] =    { 0xff, 0, 0,    0, 0xff, 0xff,  0xff, 0, 0,      0, 0xff, 0xff };
  unsigned char shortEven2[] =    { 0, 0xff, 0xff, 0xff, 0, 0,     0, 0xff,  0xff,  0xff, 0, 0 };
  unsigned char outputBuffer[sizeof(shortEven1)];

  AverageScanlines3( shortEven1, shortEven2, outputBuffer, sizeof(shortEven1) / 3 );
  for( unsigned i = 0; i < sizeof(shortEven1) ; ++i )
  {
    DALI_TEST_EQUALS( unsigned(outputBuffer[i]), 0x7fu, TEST_LOCATION );
  }

  // Longer test reusing RGBA setup/test logic:
  const size_t scanlineLength = 3 * 4 * 90u;
  Dali::Vector<uint32_t> scanline1;
  Dali::Vector<uint32_t> scanline2;
  Dali::Vector<uint32_t> reference;
  Dali::Vector<uint32_t> output;
  SetupScanlinesRGBA8888( scanlineLength, scanline1, scanline2, reference, output );

  AverageScanlines3( (const unsigned char*) &scanline1[0], (const unsigned char*) &scanline2[0], (unsigned char*) &output[0], scanlineLength * 4 / 3 );

  // Check the output matches the independently generated reference:
  size_t numMatches = 0;
  MatchScanlinesRGBA8888( reference, output, numMatches, TEST_LOCATION );
  DALI_TEST_EQUALS( numMatches, reference.Capacity(), TEST_LOCATION );

  END_TEST;
}

/**
 * @brief Test the function for averaging vertically-adjacent pairs of RGBA8888 pixels on a scanline.
 */
int UtcDaliImageOperationsAverageScanlinesRGBA8888(void)
{
  const size_t scanlineLength = 4096u;
  Dali::Vector<uint32_t> scanline1;
  Dali::Vector<uint32_t> scanline2;
  Dali::Vector<uint32_t> reference;
  Dali::Vector<uint32_t> output;
  SetupScanlinesRGBA8888( scanlineLength, scanline1, scanline2, reference, output );

  AverageScanlinesRGBA8888( (const unsigned char*) &scanline1[0], (const unsigned char*) &scanline2[0], (unsigned char*) &output[0], scanlineLength );

  // Check the output matches the independently generated reference:
  size_t numMatches = 0;
  MatchScanlinesRGBA8888( reference, output, numMatches, TEST_LOCATION );
  DALI_TEST_EQUALS( numMatches, reference.Capacity(), TEST_LOCATION );

  END_TEST;
}

/**
 * @brief Test the function for averaging vertically-adjacent pairs of RGB565 pixels on a scanline.
 */
int UtcDaliImageOperationsAverageScanlinesRGB565(void)
{
  // Red and cyan, averaging to grey:
  const uint16_t shortEven1[] =    { 0xf800, 0xf800, 0xf800, 0xf800, 0xf800, 0xf800, 0xBEEF, 0xBEEF };
  const uint16_t shortEven2[] =    { 0x7ff,  0x7ff,  0x7ff,  0x7ff,  0x7ff,  0x7ff, 0xBEEF, 0xBEEF };
  const size_t arrayLength = sizeof(shortEven1) / sizeof(shortEven1[0]) - 2;
  uint16_t outputBuffer[arrayLength + 2];
  outputBuffer[arrayLength] = 0xDEAD;
  outputBuffer[arrayLength+1] = 0xDEAD;

  Dali::Internal::Platform::AverageScanlinesRGB565( (const unsigned char*) shortEven1, (const unsigned char*) shortEven2, (unsigned char*) outputBuffer,  arrayLength );
  for( unsigned i = 0; i <  arrayLength ; ++i )
  {
    DALI_TEST_EQUALS( unsigned(outputBuffer[i]), 0xffff - (1u << 15) - (1u << 10) - (1u << 4), TEST_LOCATION );
  }

  // Check for buffer overrun:
  DALI_TEST_EQUALS( outputBuffer[arrayLength], 0xDEAD, TEST_LOCATION );
  DALI_TEST_EQUALS( outputBuffer[arrayLength+1], 0xDEAD, TEST_LOCATION );

  END_TEST;
}
