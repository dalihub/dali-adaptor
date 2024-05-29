/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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

#ifndef DALI_INTERNAL_PLATFORM_IMAGE_OPERATIONS_H
#define DALI_INTERNAL_PLATFORM_IMAGE_OPERATIONS_H

// EXTERNAL INCLUDES
#include <stdint.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/pixel-buffer.h>
#include <dali/integration-api/bitmap.h>
#include <dali/public-api/images/image-operations.h>
#include <third-party/resampler/resampler.h>

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
 * @brief The integer dimensions of an image or a region of an image packed into
 *        16 bits per component.
 * @note  This can only be used for images of up to 65535 x 65535 pixels.
  */
typedef Uint16Pair ImageDimensions;

/**
 * @brief Work out the true desired width and height, accounting for special
 * rules for zeros in either or both input requested dimensions.
 *
 * @param[in] rawDimensions Width and height of image before processing.
 * @param[in] requestedDimensions Width and height of area to scale image into. Can be zero.
 * @return Dimensions of area to scale image into after special rules are applied.
 */
ImageDimensions CalculateDesiredDimensions(ImageDimensions rawDimensions, ImageDimensions requestedDimensions, FittingMode::Type fittingMode);

/**
 * @defgroup BitmapOperations Bitmap-to-Bitmap Image operations.
 * @{
 */

/**
 * @brief Apply requested attributes to bitmap.
 *
 * This is the top-level function which runs the on-load image post-processing
 * pipeline. Bitmaps enter here as loaded from the file system by the file
 * loaders and leave downscaled and filtered as requested by the application,
 * ready for use.
 *
 * @param[in] bitmap The input bitmap.
 * @param[in] requestedAttributes Attributes which should be applied to bitmap.
 * @return A bitmap which results from applying the requested attributes to the
 *         bitmap passed-in, or the original bitmap passed in if the attributes
 *         have no effect.
 */
Dali::Devel::PixelBuffer ApplyAttributesToBitmap(Dali::Devel::PixelBuffer bitmap, ImageDimensions dimensions, FittingMode::Type fittingMode = FittingMode::DEFAULT, SamplingMode::Type samplingMode = SamplingMode::DEFAULT);

/**
 * @brief Apply downscaling to a bitmap according to requested attributes.
 * @note The input bitmap pixel buffer may be modified and used as scratch working space for efficiency, so it must be discarded.
 **/
Dali::Devel::PixelBuffer DownscaleBitmap(Dali::Devel::PixelBuffer bitmap,
                                         ImageDimensions          desired,
                                         FittingMode::Type        fittingMode,
                                         SamplingMode::Type       samplingMode);
/**@}*/

/**
 * @defgroup ImageBufferScalingAlgorithms Pixel buffer-level scaling algorithms.
 * @{
 */

/**
 * @brief Destructive in-place downscaling by a power of 2 factor.
 *
 * A box filter with a 2x2 kernel is repeatedly applied as long as the result
 * of the next downscaling step would not be smaller than the desired
 * dimensions.
 * @param[in,out] pixels The buffer both to read from and write the result to.
 * @param[in]     pixelFormat The format of the image pointed at by pixels.
 * @param[in]     inputWidth The width of the input image.
 * @param[in]     inputHeight The height of the input image.
 * @param[in]     inputStride The stride of the input image.
 * @param[in]     desiredWidth The width the client is requesting.
 * @param[in]     desiredHeight The height the client is requesting.
 * @param[out]    outWidth  The resulting width after downscaling.
 * @param[out]    outHeight The resulting height after downscaling.
 * @param[out]    outStride The resulting stride after downscaling.
 */
void DownscaleInPlacePow2(uint8_t* const     pixels,
                          Pixel::Format      pixelFormat,
                          uint32_t           inputWidth,
                          uint32_t           inputHeight,
                          uint32_t           inputStride,
                          uint32_t           desiredWidth,
                          uint32_t           desiredHeight,
                          FittingMode::Type  fittingMode,
                          SamplingMode::Type samplingMode,
                          unsigned&          outWidth,
                          unsigned&          outHeight,
                          unsigned&          outStride);

/**
 * @brief Destructive in-place downscaling by a power of 2 factor.
 *
 * A box filter with a 2x2 kernel is repeatedly applied as long as the result
 * of the next downscaling step would not be smaller than the desired
 * dimensions.
 * @param[in,out] pixels The buffer both to read from and write the result to.
 * @param[in]     inputWidth The width of the input image.
 * @param[in]     inputHeight The height of the input image.
 * @param[in]     inputStride The stride of the input image.
 * @param[in]     desiredWidth The width the client is requesting.
 * @param[in]     desiredHeight The height the client is requesting.
 * @param[out]    outWidth  The resulting width after downscaling.
 * @param[out]    outHeight The resulting height after downscaling.
 * @param[out]    outStride The resulting stride after downscaling.
 */
void DownscaleInPlacePow2RGB888(uint8_t*         pixels,
                                uint32_t         inputWidth,
                                uint32_t         inputHeight,
                                uint32_t         inputStride,
                                uint32_t         desiredWidth,
                                uint32_t         desiredHeight,
                                BoxDimensionTest dimensionTest,
                                uint32_t&        outWidth,
                                uint32_t&        outHeight,
                                uint32_t&        outStride);

/**
 * @copydoc DownscaleInPlacePow2RGB888
 */
void DownscaleInPlacePow2RGBA8888(uint8_t*         pixels,
                                  uint32_t         inputWidth,
                                  uint32_t         inputHeight,
                                  uint32_t         inputStride,
                                  uint32_t         desiredWidth,
                                  uint32_t         desiredHeight,
                                  BoxDimensionTest dimensionTest,
                                  uint32_t&        outWidth,
                                  uint32_t&        outHeight,
                                  uint32_t&        outStride);

/**
 * @copydoc DownscaleInPlacePow2RGB888
 *
 * For the 2-byte packed 16 bit format RGB565.
 */
void DownscaleInPlacePow2RGB565(uint8_t*         pixels,
                                uint32_t         inputWidth,
                                uint32_t         inputHeight,
                                uint32_t         inputStride,
                                uint32_t         desiredWidth,
                                uint32_t         desiredHeight,
                                BoxDimensionTest dimensionTest,
                                uint32_t&        outWidth,
                                uint32_t&        outHeight,
                                uint32_t&        outStride);

/**
 * @copydoc DownscaleInPlacePow2RGB888
 *
 * For 2-byte formats such as lum8alpha8, but not packed 16 bit formats like RGB565.
 */
void DownscaleInPlacePow2ComponentPair(uint8_t*         pixels,
                                       uint32_t         inputWidth,
                                       uint32_t         inputHeight,
                                       uint32_t         inputStride,
                                       uint32_t         desiredWidth,
                                       uint32_t         desiredHeight,
                                       BoxDimensionTest dimensionTest,
                                       uint32_t&        outWidth,
                                       uint32_t&        outHeight,
                                       uint32_t&        outStride);

/**
 * @copydoc DownscaleInPlacePow2RGB888
 *
 * For single-byte formats such as lum8 or alpha8.
 */
void DownscaleInPlacePow2SingleBytePerPixel(uint8_t*         pixels,
                                            uint32_t         inputWidth,
                                            uint32_t         inputHeight,
                                            uint32_t         inputStride,
                                            uint32_t         desiredWidth,
                                            uint32_t         desiredHeight,
                                            BoxDimensionTest dimensionTest,
                                            uint32_t&        outWidth,
                                            uint32_t&        outHeight,
                                            uint32_t&        outStride);

/**
 * @brief Rescales an input image into the exact output dimensions passed-in.
 *
 * Uses point sampling, equivalent to GL_NEAREST texture filter mode, for the
 * fastest results, at the expense of aliasing (noisy images) when downscaling.
 * @note inPixels is allowed to alias outPixels if this is a downscaling,
 * but not for upscaling.
 */
void PointSample(const uint8_t* inPixels,
                 uint32_t       inputWidth,
                 uint32_t       inputHeight,
                 uint32_t       inputStride,
                 Pixel::Format  pixelFormat,
                 uint8_t*       outPixels,
                 uint32_t       desiredWidth,
                 uint32_t       desiredHeight);

/**
 * @copydoc PointSample
 *
 * Specialised for 4-byte formats like RGBA8888 and BGRA8888.
 */
void PointSample4BPP(const uint8_t* inPixels,
                     uint32_t       inputWidth,
                     uint32_t       inputHeight,
                     uint32_t       inputStride,
                     uint8_t*       outPixels,
                     uint32_t       desiredWidth,
                     uint32_t       desiredHeight);

/**
 * @copydoc PointSample
 *
 * Specialised for 3-byte formats like RGB888 and BGR888.
 */
void PointSample3BPP(const uint8_t* inPixels,
                     uint32_t       inputWidth,
                     uint32_t       inputHeight,
                     uint32_t       inputStride,
                     uint8_t*       outPixels,
                     uint32_t       desiredWidth,
                     uint32_t       desiredHeight);

/**
 * @copydoc PointSample
 *
 * Specialised for 2-byte formats like LA88.
 */
void PointSample2BPP(const uint8_t* inPixels,
                     uint32_t       inputWidth,
                     uint32_t       inputHeight,
                     uint32_t       inputStride,
                     uint8_t*       outPixels,
                     uint32_t       desiredWidth,
                     uint32_t       desiredHeight);

/**
 * @copydoc PointSample
 *
 * Specialised for 1-byte formats like L8 and A8.
 */
void PointSample1BPP(const uint8_t* inPixels,
                     uint32_t       inputWidth,
                     uint32_t       inputHeight,
                     uint32_t       inputStride,
                     uint8_t*       outPixels,
                     uint32_t       desiredWidth,
                     uint32_t       desiredHeight);

/**
 * @brief Resample input image to output image using a bilinear filter.
 *
 * Each output pixel is formed of a weighted sum of a 2x2 block of four input
 * pixels
 * @pre inPixels must not alias outPixels. The input image should be a totally
 * separate buffer from the input one.
 */
void LinearSample(const uint8_t* __restrict__ inPixels,
                  ImageDimensions inDimensions,
                  uint32_t        inStride,
                  Pixel::Format   pixelFormat,
                  uint8_t* __restrict__ outPixels,
                  ImageDimensions outDimensions);

/**
 * @copydoc LinearSample
 *
 * Specialised for one byte per pixel formats.
 */
void LinearSample1BPP(const uint8_t* __restrict__ inPixels,
                      ImageDimensions inputDimensions,
                      uint32_t        inputStride,
                      uint8_t* __restrict__ outPixels,
                      ImageDimensions desiredDimensions);

/**
 * @copydoc LinearSample
 *
 * Specialised for two byte per pixel formats.
 */
void LinearSample2BPP(const uint8_t* __restrict__ inPixels,
                      ImageDimensions inputDimensions,
                      uint32_t        inputStride,
                      uint8_t* __restrict__ outPixels,
                      ImageDimensions desiredDimensions);

/**
 * @copydoc LinearSample
 *
 * Specialised for RGB565 16 bit pixel format.
 */
void LinearSampleRGB565(const uint8_t* __restrict__ inPixels,
                        ImageDimensions inputDimensions,
                        uint32_t        inputStride,
                        uint8_t* __restrict__ outPixels,
                        ImageDimensions desiredDimensions);

/**
 * @copydoc LinearSample
 *
 * Specialised for three byte per pixel formats like RGB888.
 */
void LinearSample3BPP(const uint8_t* __restrict__ inPixels,
                      ImageDimensions inputDimensions,
                      uint32_t        inputStride,
                      uint8_t* __restrict__ outPixels,
                      ImageDimensions desiredDimensions);

/**
 * @copydoc LinearSample
 *
 * Specialised for four byte per pixel formats like RGBA8888.
 * @note, If used on RGBA8888, the A component will be blended independently.
 */
void LinearSample4BPP(const uint8_t* __restrict__ inPixels,
                      ImageDimensions inputDimensions,
                      uint32_t        inputStride,
                      uint8_t* __restrict__ outPixels,
                      ImageDimensions desiredDimensions);

/**
 * @brief Resample input image to output image using a Lanczos algorithm.
 *
 * @pre @p inPixels must not alias @p outPixels. The input image should be a totally
 * separate buffer from the output buffer.
 *
 * @param[in] inPixels Pointer to the input image buffer.
 * @param[in] inputDimensions The input dimensions of the image.
 * @param[in] inputStride The input stride of the image.
 * @param[in] pixelFormat The format of the image pointed at by pixels.
 * @param[out] outPixels Pointer to the output image buffer.
 * @param[in] desiredDimensions The output dimensions of the image.
 */
void LanczosSample(const uint8_t* __restrict__ inPixels,
                   ImageDimensions inDimensions,
                   uint32_t        inStride,
                   Pixel::Format   pixelFormat,
                   uint8_t* __restrict__ outPixels,
                   ImageDimensions outDimensions);

/**
 * @brief Resamples the input image with the Lanczos algorithm.
 *
 * @pre @p inPixels must not alias @p outPixels. The input image should be a totally
 * separate buffer from the output buffer.
 *
 * @param[in] inPixels Pointer to the input image buffer.
 * @param[in] inputDimensions The input dimensions of the image.
 * @param[in] inputStride The input stride of the image.
 * @param[out] outPixels Pointer to the output image buffer.
 * @param[in] desiredDimensions The output dimensions of the image.
 */
void LanczosSample4BPP(const uint8_t* __restrict__ inPixels,
                       ImageDimensions inputDimensions,
                       uint32_t        inputStride,
                       uint8_t* __restrict__ outPixels,
                       ImageDimensions desiredDimensions);

/**
 * @brief Resamples the input image with the Lanczos algorithm.
 *
 * @pre @p inPixels must not alias @p outPixels. The input image should be a totally
 * separate buffer from the output buffer.
 *
 * @param[in] inPixels Pointer to the input image buffer.
 * @param[in] inputDimensions The input dimensions of the image.
 * @param[in] inputStride The input stride of the image.
 * @param[out] outPixels Pointer to the output image buffer.
 * @param[in] desiredDimensions The output dimensions of the image.
 */
void LanczosSample1BPP(const uint8_t* __restrict__ inPixels,
                       ImageDimensions inputDimensions,
                       uint32_t        inputStride,
                       uint8_t* __restrict__ outPixels,
                       ImageDimensions desiredDimensions);

/**
 * @brief Resamples the input image with the Lanczos algorithm.
 *
 * @pre @p inPixels must not alias @p outPixels. The input image should be a totally
 * separate buffer from the output buffer.
 *
 * @param[in] inPixels Pointer to the input image buffer.
 * @param[in] inputDimensions The input dimensions of the image.
 * @param[in] inputStride The input stride of the image.
 * @param[out] outPixels Pointer to the output image buffer.
 * @param[in] desiredDimensions The output dimensions of the image.
 */
void Resample(const uint8_t* __restrict__ inPixels,
              ImageDimensions inputDimensions,
              uint32_t        inputStride,
              uint8_t* __restrict__ outPixels,
              ImageDimensions   desiredDimensions,
              Resampler::Filter filterType,
              int               numChannels,
              bool              hasAlpha);

/**
 * @brief Rotates the input image with an implementation of the 'Rotate by Shear' algorithm.
 *
 * @pre @p pixelsIn must not alias @p pixelsOut. The input image should be a totally
 * separate buffer from the output buffer.
 *
 * @note This function allocates memory in @p pixelsOut which has to be released by calling @e free()
 *
 * @param[in] pixelsIn The input buffer.
 * @param[in] widthIn The width of the input buffer.
 * @param[in] heightIn The height of the input buffer.
 * @param[in] strideIn The stride of the input buffer.
 * @param[in] pixelSize The size of the pixel.
 * @param[in] radians The rotation angle in radians.
 * @param[out] pixelsOut The rotated output buffer.
 * @param[out] widthOut The width of the output buffer.
 * @param[out] heightOut The height of the output buffer.
 */
void RotateByShear(const uint8_t* const pixelsIn,
                   uint32_t             widthIn,
                   uint32_t             heightIn,
                   uint32_t             strideIn,
                   uint32_t             pixelSize,
                   float                radians,
                   uint8_t*&            pixelsOut,
                   uint32_t&            widthOut,
                   uint32_t&            heightOut);

/**
 * @brief Applies to the input image a horizontal shear transformation.
 *
 * @pre @p pixelsIn must not alias @p pixelsOut. The input image should be a totally
 * separate buffer from the output buffer.
 * @pre The maximun/minimum shear angle is +/-45 degrees (PI/4 around 0.79 radians).
 *
 * @note This function allocates memory in @p pixelsOut which has to be released by calling @e free()
 *
 * @param[in] pixelsIn The input buffer.
 * @param[in] widthIn The width of the input buffer.
 * @param[in] heightIn The height of the input buffer.
 * @param[in] strideIn The stride of the input buffer.
 * @param[in] pixelSize The size of the pixel.
 * @param[in] radians The shear angle in radians.
 * @param[out] pixelsOut The rotated output buffer.
 * @param[out] widthOut The width of the output buffer.
 * @param[out] heightOut The height of the output buffer.
 */
void HorizontalShear(const uint8_t* const pixelsIn,
                     uint32_t             widthIn,
                     uint32_t             heightIn,
                     uint32_t             strideIn,
                     uint32_t             pixelSize,
                     float                radians,
                     uint8_t*&            pixelsOut,
                     uint32_t&            widthOut,
                     uint32_t&            heightOut);

/**@}*/

/**
 * @defgroup ScalingAlgorithmFragments Composable subunits of the scaling algorithms.
 * @{
 */

/**
 * @brief Average adjacent pairs of pixels, overwriting the input array.
 * @param[in,out] pixels The array of pixels to work on.
 * @param[i]      width  The number of pixels in the array passed-in.
 */
void HalveScanlineInPlaceRGB888(uint8_t* pixels, uint32_t width);

/**
 * @copydoc HalveScanlineInPlaceRGB888
 */
void HalveScanlineInPlaceRGBA8888(uint8_t* pixels, uint32_t width);

/**
 * @copydoc HalveScanlineInPlaceRGB888
 */
void HalveScanlineInPlaceRGB565(uint8_t* pixels, uint32_t width);

/**
 * @copydoc HalveScanlineInPlaceRGB888
 */
void HalveScanlineInPlace2Bytes(uint8_t* pixels, uint32_t width);

/**
 * @copydoc HalveScanlineInPlaceRGB888
 */
void HalveScanlineInPlace1Byte(uint8_t* pixels, uint32_t width);

/**
 * @brief Average pixels at corresponding offsets in two scanlines.
 *
 * outputScanline is allowed to alias scanline1.
 * @param[in] scanline1 First scanline of pixels to average.
 * @param[in] scanline2 Second scanline of pixels to average.
 * @param[out] outputScanline Destination for the averaged pixels.
 * @param[in] width The widths of all the scanlines passed-in.
 */
void AverageScanlines1(const uint8_t* scanline1,
                       const uint8_t* scanline2,
                       uint8_t*       outputScanline,
                       /** Image width in pixels (1 byte == 1 pixel: e.g. lum8 or alpha8).*/
                       uint32_t width);

/**
 * @copydoc AverageScanlines1
 */
void AverageScanlines2(const uint8_t* scanline1,
                       const uint8_t* scanline2,
                       uint8_t*       outputScanline,
                       /** Image width in pixels (2 bytes == 1 pixel: e.g. lum8alpha8).*/
                       uint32_t width);

/**
 * @copydoc AverageScanlines1
 */
void AverageScanlines3(const uint8_t* scanline1,
                       const uint8_t* scanline2,
                       uint8_t*       outputScanline,
                       /** Image width in pixels (3 bytes == 1 pixel: e.g. RGB888).*/
                       uint32_t width);

/**
 * @copydoc AverageScanlines1
 */
void AverageScanlinesRGBA8888(const uint8_t* scanline1,
                              const uint8_t* scanline2,
                              uint8_t*       outputScanline,
                              uint32_t       width);

/**
 * @copydoc AverageScanlines1
 */
void AverageScanlinesRGB565(const uint8_t* scanline1,
                            const uint8_t* scanline2,
                            uint8_t*       outputScanline,
                            uint32_t       width);
/**@}*/

/**
 * @defgroup TestableInlines Inline functions exposed in header to allow unit testing.
 * @{
 */

/**
 * @brief Average two integer arguments.
 * @return The average of two uint arguments.
 * @param[in] a First component to average.
 * @param[in] b Second component to average.
 **/
inline uint32_t AverageComponent(uint32_t a, uint32_t b)
{
  uint32_t avg = (a + b) >> 1u;
  return avg;
}

/**
 * @brief Average a pair of RGBA8888 pixels.
 * @return The average of two RGBA8888 pixels.
 * @param[in] a First pixel to average.
 * @param[in] b Second pixel to average
 **/
inline uint32_t AveragePixelRGBA8888(uint32_t a, uint32_t b)
{
  /**
   * @code
   * const uint32_t avg =
   *   (AverageComponent((a & 0xff000000) >> 1u, (b & 0xff000000) >> 1u) << 1u) & 0xff000000) +
   *   (AverageComponent(a & 0x00ff0000, b & 0x00ff0000) & 0x00ff0000) +
   *   (AverageComponent(a & 0x0000ff00, b & 0x0000ff00) & 0x0000ff00) +
   *   (AverageComponent(a & 0x000000ff, b & 0x000000ff);
   * return avg;
   * @endcode
   */
  return (((a ^ b) & 0xfefefefeu) >> 1) + (a & b);
  ///@ToDo: Optimise for ARM using the single ARMV6 instruction: UHADD8  R4, R0, R5. This is not Neon. It runs in the normal integer pipeline so there is no downside like a stall moving between integer and copro.
}

/**
 * @brief Average a pair of RGB565 pixels.
 * @param a[in] Low 16 bits hold a color value as RGB565 to average with parameter b.
 * @param b[in] Low 16 bits hold a color value as RGB565 to average with parameter a.
 * @return The average color of the two RGB565 pixels passed in, in the low 16 bits of the returned value.
 **/
inline uint32_t AveragePixelRGB565(uint32_t a, uint32_t b)
{
  /**
   * @code
   * const uint32_t avg =
   *   (AverageComponent(a & 0xf800, b & 0xf800) & 0xf800) +
   *   (AverageComponent(a & 0x7e0, b & 0x7e0) & 0x7e0) +
   *   (AverageComponent(a & 0x1f, b & 0x1f));
   * return avg;
   * @endcode
   */
  return (((a ^ b) & 0xf7deu) >> 1) + (a & b);
}

/** @return The weighted blend of two integers as a 16.16 fixed-point number, given a 0.16 fixed-point blending factor. */
inline uint32_t WeightedBlendIntToFixed1616(uint32_t a, uint32_t b, uint32_t fractBlend)
{
  DALI_ASSERT_DEBUG(fractBlend <= 65535u && "Factor should be in 0.16 fixed-point.");
  /**
   * @code
   * const uint32_t weightedAFixed = a * (65535u - fractBlend);
   * const uint32_t weightedBFixed = b * fractBlend;
   * const unsigned     blended        = (weightedAFixed + weightedBFixed);
   * @endcode
   */
  const uint32_t blended = (a << 16) - a + (static_cast<int32_t>(b) - static_cast<int32_t>(a)) * fractBlend;
  return blended;
}

/** @brief Blend two 16.16 inputs to give a 16.32 output. */
inline uint64_t WeightedBlendFixed1616ToFixed1632(uint32_t a, uint32_t b, uint32_t fractBlend)
{
  DALI_ASSERT_DEBUG(fractBlend <= 65535u && "Factor should be in 0.16 fixed-point.");
  /**
   * @code
   * // Blend while promoting intermediates to 16.32 fixed point:
   * const uint64_t weightedAFixed = uint64_t(a) * (65535u - fractBlend);
   * const uint64_t weightedBFixed = uint64_t(b) * fractBlend;
   * const uint64_t blended        = (weightedAFixed + weightedBFixed);
   * @endcode
   */
  const uint64_t blended = (static_cast<uint64_t>(a) << 16) - a + (static_cast<int64_t>(b) - static_cast<int64_t>(a)) * fractBlend;
  return blended;
}

/**
 * @brief Blend 4 taps into one value using horizontal and vertical weights.
 */
inline uint32_t BilinearFilter1Component(uint32_t tl, uint32_t tr, uint32_t bl, uint32_t br, uint32_t fractBlendHorizontal, uint32_t fractBlendVertical)
{
  DALI_ASSERT_DEBUG(fractBlendHorizontal <= 65535u && "Factor should be in 0.16 fixed-point.");
  DALI_ASSERT_DEBUG(fractBlendVertical <= 65535u && "Factor should be in 0.16 fixed-point.");

  /**
   * @code
   * const uint32_t topBlend   = WeightedBlendIntToFixed1616(tl, tr, fractBlendHorizontal);
   * const uint32_t botBlend   = WeightedBlendIntToFixed1616(bl, br, fractBlendHorizontal);
   * const uint64_t     blended2x2 = WeightedBlendFixed1616ToFixed1632(topBlend, botBlend, fractBlendVertical);
   * const uint32_t rounded    = (blended2x2 + (1u << 31u)) >> 32u;
   * @endcode
   */

  /**
   * Hard-coding optimize!
   *
   * Let p = 65536, s.t we can optimze it as << 16.
   * Let x = fractBlendHorizontal, y = fractBlendVertical.
   * topBlend = (tl*p - tl - tl*x + tr*x)
   * botBlend = (bl*p - bl - bl*x + br*x)
   * blended2x2 = topBlend*p - topBlend - topBlend*y + botBlend*y
   *
   * And now we can split all values.
   * tl*p*p - tl*p - tl*x*p + tr*x*p  -  tl*p + tl + tl*x - tr*x  -  tl*y*p + tl*y + tl*x*y - tr*x*y  +  bl*y*p - bl*y - bl*x*y + br*x*y;
   * --> (collect by p, x, and y)
   * (tl)*p*p + (-2tl + (-tl + tr)*x + (-tl+bl)*y)*p + tl + (tl - tr)*x + (tl - bl)*y + (tl - tr - bl + br)*x*y
   *
   * A = (tl - tr) * x;
   * B = (tl - bl) * y;
   * C = (tl - tr - bl + br) * x * y;
   * D = (2*tl + A + B)
   * -->
   * (tl << 32) - (D << 16) + tl + A + B + C
   *
   * Becareful of overflow and negative value.
   */
  const int32_t A = (static_cast<int32_t>(tl) - static_cast<int32_t>(tr)) * static_cast<int32_t>(fractBlendHorizontal);
  const int32_t B = (static_cast<int32_t>(tl) - static_cast<int32_t>(bl)) * static_cast<int32_t>(fractBlendVertical);
  const int64_t C = (static_cast<int64_t>(tl) - static_cast<int64_t>(tr) - static_cast<int64_t>(bl) + static_cast<int64_t>(br)) * static_cast<int64_t>(fractBlendHorizontal) * static_cast<int64_t>(fractBlendVertical);
  const int64_t D = ((static_cast<int64_t>(tl) << 1) + A + B);

  const uint64_t blended2x2 = (static_cast<int64_t>(tl) << 32u) - (D << 16u) + tl + A + B + C;
  const uint32_t rounded    = static_cast<uint32_t>((blended2x2 + (1u << 31u)) >> 32u);
  return rounded;
}

/**
 * @brief Fast multiply & divide by 255. It wiil be useful when we applying alpha value in color
 *
 * @param x The value between [0..255]
 * @param y The value between [0..255]
 * @return (x*y)/255
 */
inline uint8_t MultiplyAndNormalizeColor(const uint8_t x, const uint8_t y) noexcept
{
  const uint32_t xy = static_cast<const uint32_t>(x) * y;
  return ((xy << 15) + (xy << 7) + xy) >> 23;
}

/**
 * @brief Fast division by 17 and roundup. It will be useful when we compress 8bit luminance value as 4bit for text glyph.
 *
 * @param x The value between [0..255]
 * @return round(x / 17.0f).(same as (x+8)/17)
 */
inline uint8_t CompressBitPerPixel8To4(const uint8_t x) noexcept
{
  return ((((static_cast<const uint16_t>(x) << 4) - x + (x >> 4)) >> 7) + 1) >> 1;
}

/**@}*/

} /* namespace Platform */
} /* namespace Internal */
} /* namespace Dali */

#endif /* DALI_INTERNAL_PLATFORM_IMAGE_OPERATIONS_H */
