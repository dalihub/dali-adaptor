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

#ifndef DALI_INTERNAL_PLATFORM_IMAGE_OPERATIONS_H_
#define DALI_INTERNAL_PLATFORM_IMAGE_OPERATIONS_H_

// INTERNAL INCLUDES
#include <dali/integration-api/bitmap.h>
#include <dali/public-api/images/image-attributes.h>

// EXTERNAL INCLUDES
#include <stdint.h>

namespace Dali
{
namespace Internal
{
namespace Platform
{

/**
 * @brief Identify which combination of x and y dimensions matter in terminating iterative box filtering.
 */
enum BoxDimensionTest
{
  BoxDimensionTestEither,
  BoxDimensionTestBoth,
  BoxDimensionTestX,
  BoxDimensionTestY
};

/**
 * @brief Apply requested attributes to bitmap.
 * @param[in] bitmap The input bitmap.
 * @param[in] requestedAttributes Attributes which should be applied to bitmap.
 * @return A bitmap which results from applying the requested attributes to the bitmap passed-in, or the original bitmap passed in if the attributes have no effect.
 */
Integration::BitmapPtr ApplyAttributesToBitmap( Integration::BitmapPtr bitmap, const ImageAttributes& requestedAttributes );

/**
 * @brief Apply downscaling to a bitmap according to requested attributes.
 * @note Only rough power of 2 box filtering is currently performed.
 * @note The input bitmap may be modified and left in an invalid state so must be discarded.
 **/
Integration::BitmapPtr DownscaleBitmap( Integration::Bitmap& bitmap, const ImageAttributes& requestedAttributes );

/**
 * @brief Destructive in-place downscaling by a power of 2 factor.
 *
 * A box filter with a 2x2 kernel is repeatedly applied as long as the result
 * of the next downscaling step would not be smaller than the desired
 * dimensions.
 * @param[in,out] pixels The buffer both to read from and write the result to.
 * @param[in]     inputWidth The width of the input image.
 * @param[in]     inputHeight The height of the input image.
 * @param[in]     desiredWidth The width the client is requesting.
 * @param[in]     desiredHeight The height the client is requesting.
 * @param[out]    outWidth  The resulting width after downscaling.
 * @param[out]    outHeight The resulting height after downscaling.
 */
void DownscaleInPlacePow2RGB888(
    unsigned char * pixels,
    unsigned int inputWidth, unsigned int inputHeight,
    unsigned int desiredWidth, unsigned int desiredHeight,
    BoxDimensionTest dimensionTest,
    unsigned int& outWidth, unsigned int& outHeight );

/**
 * @copydoc DownscaleInPlacePow2RGB888
 */
void DownscaleInPlacePow2RGBA8888(
    unsigned char * pixels,
    unsigned int inputWidth, unsigned int inputHeight,
    unsigned int desiredWidth, unsigned int desiredHeight,
    BoxDimensionTest dimensionTest,
    unsigned int& outWidth, unsigned int& outHeight );

/**
 * @copydoc DownscaleInPlacePow2RGB888
 *
 * For the 2-byte packed 16 bit format RGB565.
 */
void DownscaleInPlacePow2RGB565(
    unsigned char * pixels,
    unsigned int inputWidth, unsigned int inputHeight,
    unsigned int desiredWidth, unsigned int desiredHeight,
    BoxDimensionTest dimensionTest,
    unsigned int& outWidth, unsigned int& outHeight );

/**
 * @copydoc DownscaleInPlacePow2RGB888
 *
 * For 2-byte formats such as lum8alpha8, but not packed 16 bit formats like RGB565.
 */
void DownscaleInPlacePow2ComponentPair(
    unsigned char * pixels,
    unsigned int inputWidth, unsigned int inputHeight,
    unsigned int desiredWidth, unsigned int desiredHeight,
    BoxDimensionTest dimensionTest,
    unsigned int& outWidth, unsigned int& outHeight );

/**
 * @copydoc DownscaleInPlacePow2RGB888
 *
 * For single-byte formats such as lum8 or alpha8.
 */
void DownscaleInPlacePow2SingleBytePerPixel(
    unsigned char * pixels,
    unsigned int inputWidth, unsigned int inputHeight,
    unsigned int desiredWidth, unsigned int desiredHeight,
    BoxDimensionTest dimensionTest,
    unsigned int& outWidth, unsigned int& outHeight );

/**
 * @brief Average adjacent pairs of pixels, overwriting the input array.
 * @param[in,out] pixels The array of pixels to work on.
 * @param[i]      width  The number of pixels in the array passed-in.
 */
void HalveScanlineInPlaceRGB888( unsigned char * pixels, unsigned int width );

/**
 * @copydoc HalveScanlineInPlaceRGB888
 */
void HalveScanlineInPlaceRGBA8888(
    unsigned char * pixels,
    unsigned int width );

/**
 * @copydoc HalveScanlineInPlaceRGB888
 */
void HalveScanlineInPlaceRGB565( unsigned char * pixels, unsigned int width );

/**
 * @copydoc HalveScanlineInPlaceRGB888
 */
void HalveScanlineInPlace2Bytes(
    unsigned char * pixels,
    unsigned int width );

/**
 * @copydoc HalveScanlineInPlaceRGB888
 */
void HalveScanlineInPlace1Byte(
    unsigned char * pixels,
    unsigned int width );

/**
 * @brief Average pixels at corresponding offsets in two scanlines.
 *
 * outputScanline is allowed to alias scanline1.
 * @param[in] scanline1 First scanline of pixels to average.
 * @param[in] scanline2 Second scanline of pixels to average.
 * @param[out] outputScanline Destination for the averaged pixels.
 * @param[in] width The widths of all the scanlines passed-in.
 */
void AverageScanlines1(
    const unsigned char * scanline1,
    const unsigned char * scanline2,
    unsigned char* outputScanline,
    /** Image width in pixels (1 byte == 1 pixel: e.g. lum8 or alpha8).*/
    unsigned int width );

/**
 * @copydoc AverageScanlines1
 */
void AverageScanlines2(
    const unsigned char * scanline1,
    const unsigned char * scanline2,
    unsigned char* outputScanline,
    /** Image width in pixels (2 bytes == 1 pixel: e.g. lum8alpha8).*/
    unsigned int width );

/**
 * @copydoc AverageScanlines1
 */
void AverageScanlines3(
    const unsigned char * scanline1,
    const unsigned char * scanline2,
    unsigned char* outputScanline,
    /** Image width in pixels (3 bytes == 1 pixel: e.g. RGB888).*/
    unsigned int width );

/**
 * @copydoc AverageScanlines1
 */
void AverageScanlinesRGBA8888(
    const unsigned char * scanline1,
    const unsigned char * scanline2,
    unsigned char * outputScanline,
    unsigned int width );

/**
 * @copydoc AverageScanlines1
 */
void AverageScanlinesRGB565(
    const unsigned char * scanline1,
    const unsigned char * scanline2,
    unsigned char* outputScanline,
    unsigned int width );

/**
 * @brief Inline functions exposed in header to allow unit testing.
 */
namespace
{
  /**
   * @brief Average two integer arguments.
   * @return The average of two uint arguments.
   * @param[in] a First component to average.
   * @param[in] b Second component to average.
   **/
  inline unsigned int AverageComponent( unsigned int a, unsigned int b )
  {
    unsigned int avg = (a + b) >> 1u;
    return avg;
  }

  /**
   * @brief Average a pair of RGB565 pixels.
   * @return The average of two RGBA8888 pixels.
   * @param[in] a First pixel to average.
   * @param[in] b Second pixel to average
   **/
  inline uint32_t AveragePixelRGBA8888( uint32_t a, uint32_t b )
  {
    const unsigned int avg =
      ((AverageComponent( (a & 0xff000000) >> 1u, (b & 0xff000000) >> 1u ) << 1u) & 0xff000000 ) +
      (AverageComponent( a & 0x00ff0000, b & 0x00ff0000 ) & 0x00ff0000 ) +
      (AverageComponent( a & 0x0000ff00, b & 0x0000ff00 ) & 0x0000ff00 ) +
      (AverageComponent( a & 0x000000ff, b & 0x000000ff ) );
    return avg;
    ///@ToDo: Optimise by trying return (((a ^ b) & 0xfefefefeUL) >> 1) + (a & b);
    ///@ToDo: Optimise for ARM using the single ARMV6 instruction: UHADD8  R4, R0, R5. This is not neon. It runs in the normal integer pipeline so there is no downside like a stall moving between integer and copro.
  }

  /**
   * @brief Average a pair of RGB565 pixels.
   * @param a[in] Low 16 bits hold a color value as RGB565 to average with parameter b.
   * @param b[in] Low 16 bits hold a color value as RGB565 to average with parameter a.
   * @return The average color of the two RGB565 pixels passed in, in the low 16 bits of the returned value.
   **/
  inline uint32_t AveragePixelRGB565( uint32_t a, uint32_t b )
  {
    const unsigned int avg =
      (AverageComponent( a & 0xf800, b & 0xf800 ) & 0xf800 ) +
      (AverageComponent( a & 0x7e0,  b & 0x7e0 )  & 0x7e0 ) +
      (AverageComponent( a & 0x1f,   b & 0x1f ) );
    return avg;
  }

} // namespace - unnamed
} /* namespace Platform */
} /* namespace Internal */
} /* namespace Dali */

#endif /* DALI_INTERNAL_PLATFORM_IMAGE_OPERATIONS_H_ */
