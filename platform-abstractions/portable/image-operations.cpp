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

// INTERNAL INCLUDES
#include "image-operations.h"
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/ref-counted-dali-vector.h>
#include <dali/public-api/images/image-attributes.h>
#include <dali/integration-api/bitmap.h>

// EXTERNAL INCLUDES
#include <cstring>

namespace Dali
{
namespace Internal
{
namespace Platform
{

namespace
{
using Integration::Bitmap;
using Integration::BitmapPtr;
typedef unsigned char PixelBuffer;

#if defined(DEBUG_ENABLED)
/**
 * Disable logging of image operations or make it verbose from the commandline
 * as follows (e.g., for dali demo app):
 * <code>
 * LOG_IMAGE_OPERATIONS=0 dali-demo #< off
 * LOG_IMAGE_OPERATIONS=3 dali-demo #< on, verbose
 * </code>
 */
Debug::Filter* gImageOpsLogFilter = Debug::Filter::New( Debug::NoLogging, false, "LOG_IMAGE_OPERATIONS" );
#endif

/** @return The greatest even number less than or equal to the argument. */
inline unsigned int EvenDown( const unsigned int a )
{
  const unsigned int evened = a & ~1u;
  return evened;
}

/**
 * @brief Log bad parameters.
 */
void ValidateScalingParameters(
  const unsigned int inputWidth,    const unsigned int inputHeight,
  const unsigned int desiredWidth, const unsigned int desiredHeight )
{
  if( desiredWidth > inputWidth || desiredHeight > inputHeight )
  {
    DALI_LOG_INFO( gImageOpsLogFilter, Dali::Integration::Log::Verbose, "Upscaling not supported (%u, %u -> %u, %u).\n", inputWidth, inputHeight, desiredWidth, desiredHeight );
  }

  if( desiredWidth == 0u || desiredHeight == 0u )
  {
    DALI_LOG_INFO( gImageOpsLogFilter, Dali::Integration::Log::Verbose, "Downscaling to a zero-area target is pointless." );
  }

  if( inputWidth == 0u || inputHeight == 0u )
  {
    DALI_LOG_INFO( gImageOpsLogFilter, Dali::Integration::Log::Verbose, "Zero area images cannot be scaled" );
  }
}

/**
 * @brief Do debug assertions common to all scanline halving functions.
 * @note Inline and in anon namespace so should boil away in release builds.
 */
inline void DebugAssertScanlineParameters(
    unsigned char * const pixels,
    const unsigned int width )
{
  DALI_ASSERT_DEBUG( pixels && "Null pointer." );
  DALI_ASSERT_DEBUG( width > 1u && "Can't average fewer than two pixels." );
  DALI_ASSERT_DEBUG( width < 131072u && "Unusually wide image: are you sure you meant to pass that value in?" );
}

/**
 * @brief Assertions on params to functions averaging pairs of scanlines.
 */
inline void DebugAssertDualScanlineParameters(
    const unsigned char * const scanline1,
    const unsigned char * const scanline2,
    unsigned char* const outputScanline,
    const size_t widthInComponents )
{
  DALI_ASSERT_DEBUG( scanline1 && "Null pointer." );
  DALI_ASSERT_DEBUG( scanline2 && "Null pointer." );
  DALI_ASSERT_DEBUG( outputScanline && "Null pointer." );
  DALI_ASSERT_DEBUG( ((scanline1 >= scanline2 + widthInComponents) || (scanline2 >= scanline1 + widthInComponents )) && "Scanlines alias." );
  DALI_ASSERT_DEBUG( ((((void*)outputScanline) >= (void*)(scanline2 + widthInComponents)) || (((void*)scanline2) >= (void*)(scanline1 + widthInComponents))) && "Scanline 2 aliases output." );
}

} // namespace - unnamed

/**
 * @brief Implement ImageAttributes::ScaleTofill scaling mode.
 *
 * Implement the ImageAttributes::ScaleToFill mode, returning a new bitmap with the aspect ratio specified by the scaling mode.
 * @note This fakes the scaling with a crop and relies on the GPU scaling at
 * render time. If the input bitmap was previously maximally downscaled using a
 * repeated box filter, this is a reasonable approach.
 * @return The bitmap passed in if no scaling is needed or possible, else a new,
 * smaller bitmap with the scaling mode applied.
 */
Integration::BitmapPtr ProcessBitmapScaleToFill( Integration::BitmapPtr bitmap, const ImageAttributes& requestedAttributes );


BitmapPtr ApplyAttributesToBitmap( BitmapPtr bitmap, const ImageAttributes& requestedAttributes )
{
  // If a different size than the raw one has been requested, resize the image
  // maximally using a repeated box filter without making it smaller than the
  // requested size in either dimension:
  if( bitmap )
  {
    bitmap = DownscaleBitmap( *bitmap, requestedAttributes );
  }

  // Cut the bitmap according to the desired width and height so that the
  // resulting bitmap has the same aspect ratio as the desired dimensions:
  if( bitmap && bitmap->GetPackedPixelsProfile() && requestedAttributes.GetScalingMode() == ImageAttributes::ScaleToFill )
  {
    bitmap = ProcessBitmapScaleToFill( bitmap, requestedAttributes );
  }

  // Examine the image pixels remaining after cropping and scaling to see if all
  // are opaque, allowing faster rendering, or some have non-1.0 alpha:
  if( bitmap && bitmap->GetPackedPixelsProfile() && Pixel::HasAlpha( bitmap->GetPixelFormat() ) )
  {
    bitmap->GetPackedPixelsProfile()->TestForTransparency();
  }
  return bitmap;
}

BitmapPtr ProcessBitmapScaleToFill( BitmapPtr bitmap, const ImageAttributes& requestedAttributes )
{
  const unsigned loadedWidth = bitmap->GetImageWidth();
  const unsigned loadedHeight = bitmap->GetImageHeight();
  const unsigned desiredWidth = requestedAttributes.GetWidth();
  const unsigned desiredHeight = requestedAttributes.GetHeight();

  if( desiredWidth < 1U || desiredHeight < 1U )
  {
    DALI_LOG_WARNING( "Image scaling aborted as desired dimensions too small (%u, %u)\n.", desiredWidth, desiredHeight );
  }
  else if( loadedWidth != desiredWidth || loadedHeight != desiredHeight )
  {
    const Vector2 desiredDims( desiredWidth, desiredHeight );

    // Scale the desired rectangle back to fit inside the rectangle of the loaded bitmap:
    // There are two candidates (scaled by x, and scaled by y) and we choose the smallest area one.
    const float widthsRatio = loadedWidth / float(desiredWidth);
    const Vector2 scaledByWidth = desiredDims * widthsRatio;
    const float heightsRatio = loadedHeight / float(desiredHeight);
    const Vector2 scaledByHeight = desiredDims * heightsRatio;
    // Trim top and bottom if the area of the horizontally-fitted candidate is less, else trim the sides:
    const bool trimTopAndBottom = scaledByWidth.width * scaledByWidth.height < scaledByHeight.width * scaledByHeight.height;
    const Vector2 scaledDims = trimTopAndBottom ? scaledByWidth : scaledByHeight;

    // Work out how many pixels to trim from top and bottom, and left and right:
    // (We only ever do one dimension)
    const unsigned scanlinesToTrim = trimTopAndBottom ? fabsf( (scaledDims.y - loadedHeight) * 0.5f ) : 0;
    const unsigned columnsToTrim = trimTopAndBottom ? 0 : fabsf( (scaledDims.x - loadedWidth) * 0.5f );

    DALI_LOG_INFO( gImageOpsLogFilter, Debug::Concise, "Bitmap, desired(%f, %f), loaded(%u,%u), cut_target(%f, %f), trimmed(%u, %u), vertical = %s.\n", desiredDims.x, desiredDims.y, loadedWidth, loadedHeight, scaledDims.x, scaledDims.y, columnsToTrim, scanlinesToTrim, trimTopAndBottom ? "true" : "false" );

    // Make a new bitmap with the central part of the loaded one if required:
    if( scanlinesToTrim > 0 || columnsToTrim > 0 )
    {
      const unsigned newWidth = loadedWidth - 2 * columnsToTrim;
      const unsigned newHeight = loadedHeight - 2 * scanlinesToTrim;
      BitmapPtr croppedBitmap = Integration::Bitmap::New( Integration::Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::DISCARD );
      Integration::Bitmap::PackedPixelsProfile * packedView = croppedBitmap->GetPackedPixelsProfile();
      DALI_ASSERT_DEBUG( packedView );
      const Pixel::Format pixelFormat = bitmap->GetPixelFormat();
      packedView->ReserveBuffer( pixelFormat, newWidth, newHeight, newWidth, newHeight );

      const unsigned bytesPerPixel = Pixel::GetBytesPerPixel( pixelFormat );

      const PixelBuffer * const srcPixels = bitmap->GetBuffer() + scanlinesToTrim * loadedWidth * bytesPerPixel;
      PixelBuffer * const destPixels = croppedBitmap->GetBuffer();
      DALI_ASSERT_DEBUG( srcPixels && destPixels );

      // Optimize to a single memcpy if the left and right edges don't need a crop, else copy a scanline at a time:
      if( trimTopAndBottom )
      {
        memcpy( destPixels, srcPixels, newHeight * newWidth * bytesPerPixel );
      }
      else
      {
        for( unsigned y = 0; y < newHeight; ++y )
        {
          memcpy( &destPixels[y * newWidth * bytesPerPixel], &srcPixels[y * loadedWidth * bytesPerPixel + columnsToTrim * bytesPerPixel], newWidth * bytesPerPixel );
        }
      }

      // Overwrite the loaded bitmap with the cropped version:
      bitmap = croppedBitmap;
    }
  }

  return bitmap;
}

namespace
{

/**
 * @brief Converts a scaling mode to the definition of which dimensions matter when box filtering as a part of that mode.
 */
BoxDimensionTest DimensionTestForScalingMode( ImageAttributes::ScalingMode scalingMode )
{
  BoxDimensionTest dimensionTest;
  dimensionTest = BoxDimensionTestEither;

  switch( scalingMode )
  {
    // Shrink to fit attempts to make one or zero dimensions smaller than the
    // desired dimensions and one or two dimensions exactly the same as the desired
    // ones, so as long as one dimension is larger than the desired size, box
    // filtering can continue even if the second dimension is smaller than the
    // desired dimensions:
    case ImageAttributes::ShrinkToFit:
      dimensionTest = BoxDimensionTestEither;
      break;
    // Scale to fill mode keeps both dimensions at least as large as desired:
    case ImageAttributes::ScaleToFill:
      dimensionTest = BoxDimensionTestBoth;
      break;
    // Y dimension is irrelevant when downscaling in FitWidth mode:
    case ImageAttributes::FitWidth:
      dimensionTest = BoxDimensionTestX;
      break;
    // X Dimension is ignored by definition in FitHeight mode:
    case ImageAttributes::FitHeight:
      dimensionTest = BoxDimensionTestY;
  }

  return dimensionTest;
}

}

// The top-level function to return a downscaled version of a bitmap:
Integration::BitmapPtr DownscaleBitmap( Integration::Bitmap& bitmap, const ImageAttributes& requestedAttributes )
{
  const unsigned int bitmapWidth  = bitmap.GetImageWidth();
  const unsigned int bitmapHeight = bitmap.GetImageHeight();
  const Size requestedSize = requestedAttributes.GetSize();

  // If a different size than the raw one has been requested, resize the image:
  if( bitmap.GetPackedPixelsProfile() &&
      (requestedSize.x > 0.0f) && (requestedSize.y > 0.0f) &&
      (requestedSize.x < bitmapWidth) &&
      (requestedSize.y < bitmapHeight) )
  {
    const Pixel::Format pixelFormat = bitmap.GetPixelFormat();
    const ImageAttributes::ScalingMode scalingMode = requestedAttributes.GetScalingMode();
    const ImageAttributes::FilterMode filterMode = requestedAttributes.GetFilterMode();

    // Perform power of 2 iterated 4:1 box filtering if the requested filter mode requires it:
    if( filterMode == ImageAttributes::Box || filterMode == ImageAttributes::BoxThenNearest || filterMode == ImageAttributes::BoxThenLinear )
    {
      // Check the pixel format is one that is supported:
      if( pixelFormat == Pixel::RGBA8888 || pixelFormat == Pixel::RGB888 || pixelFormat == Pixel::RGB565 || pixelFormat == Pixel::LA88 || pixelFormat == Pixel::L8 || pixelFormat == Pixel::A8 )
      {
        unsigned int shrunkWidth = -1, shrunkHeight = -1;
        const BoxDimensionTest dimensionTest = DimensionTestForScalingMode( scalingMode );

        if( pixelFormat == Pixel::RGBA8888 )
        {
          Internal::Platform::DownscaleInPlacePow2RGBA8888( bitmap.GetBuffer(), bitmapWidth, bitmapHeight, requestedSize.x, requestedSize.y, dimensionTest, shrunkWidth, shrunkHeight );
        }
        else if( pixelFormat == Pixel::RGB888 )
        {
          Internal::Platform::DownscaleInPlacePow2RGB888( bitmap.GetBuffer(), bitmapWidth, bitmapHeight, requestedSize.x, requestedSize.y, dimensionTest, shrunkWidth, shrunkHeight );
        }
        else if( pixelFormat == Pixel::RGB565 )
        {
          Internal::Platform::DownscaleInPlacePow2RGB565( bitmap.GetBuffer(), bitmapWidth, bitmapHeight, requestedSize.x, requestedSize.y, dimensionTest, shrunkWidth, shrunkHeight );
        }
        else if( pixelFormat == Pixel::LA88 )
        {
          Internal::Platform::DownscaleInPlacePow2ComponentPair( bitmap.GetBuffer(), bitmapWidth, bitmapHeight, requestedSize.x, requestedSize.y, dimensionTest, shrunkWidth, shrunkHeight );
        }
        else if( pixelFormat == Pixel::L8  || pixelFormat == Pixel::A8 )
        {
          Internal::Platform::DownscaleInPlacePow2SingleBytePerPixel( bitmap.GetBuffer(), bitmapWidth, bitmapHeight, requestedSize.x, requestedSize.y, dimensionTest, shrunkWidth, shrunkHeight );
        }

        if( shrunkWidth != bitmapWidth && shrunkHeight != bitmapHeight )
        {
          // Allocate a pixel buffer to hold the shrunk image:
          Integration::BitmapPtr shrunk = Integration::Bitmap::New( Integration::Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::DISCARD );
          shrunk->GetPackedPixelsProfile()->ReserveBuffer( pixelFormat, shrunkWidth, shrunkHeight, shrunkWidth, shrunkHeight );

          // Copy over the pixels from the downscaled image that was generated in-place in the pixel buffer of the input bitmap:
          DALI_ASSERT_DEBUG( bitmap.GetBuffer() && "Null loaded bitmap buffer." );
          DALI_ASSERT_DEBUG( shrunk->GetBuffer() && "Null shrunk bitmap buffer." );
          memcpy( shrunk->GetBuffer(), bitmap.GetBuffer(), shrunkWidth * shrunkHeight * Pixel::GetBytesPerPixel( pixelFormat ) );
          return shrunk;
        }
      }
      else
      {
        DALI_LOG_INFO( gImageOpsLogFilter, Dali::Integration::Log::Verbose, "Bitmap was not shrunk: unsupported pixel format: %u.\n", unsigned(pixelFormat) );
      }
    }
  }
  return Integration::BitmapPtr(&bitmap);
}

namespace
{
/**
 * @brief Returns whether to keep box filtering based on whether downscaled dimensions will overshoot the desired ones aty the next step.
 * @param test Which combination of the two dimensions matter for terminating the filtering.
 * @param scaledWidth The width of the current downscaled image.
 * @param scaledHeight The height of the current downscaled image.
 * @param desiredWidth The target width for the downscaling.
 * @param desiredHeight The target height for the downscaling.
 */
bool ContinueScaling( BoxDimensionTest test, unsigned int scaledWidth, unsigned int scaledHeight, unsigned int desiredWidth, unsigned int desiredHeight )
{
  bool keepScaling = false;
  const unsigned int nextWidth = scaledWidth >> 1u;
  const unsigned int nextHeight = scaledHeight >> 1u;

  if( nextWidth >= 1u && nextHeight >= 1u )
  {
    switch( test )
    {
      case BoxDimensionTestEither:
        keepScaling = nextWidth >= desiredWidth || nextHeight >= desiredHeight;
        break;
      case BoxDimensionTestBoth:
        keepScaling = nextWidth >= desiredWidth && nextHeight >= desiredHeight;
        break;
      case BoxDimensionTestX:
        keepScaling = nextWidth >= desiredWidth;
        break;
      case BoxDimensionTestY:
        keepScaling = nextHeight >= desiredHeight;
    }
  }

  return keepScaling;
}

/**
 * @brief A shared implementation of the overall iterative downscaling algorithm.
 *
 * Specialise this for particular pixel formats by supplying the number of bytes
 * per pixel and two functions: one for averaging pairs of neighbouring pixels
 * on a single scanline, and a second for averaging pixels at corresponding
 * positions on different scanlines.
 **/
template<
  int BYTES_PER_PIXEL,
  void (*HalveScanlineInPlace)( unsigned char * const pixels, const unsigned int width ),
  void (*AverageScanlines) ( const unsigned char * const scanline1, const unsigned char * const __restrict__ scanline2, unsigned char* const outputScanline, const unsigned int width )
>
void DownscaleInPlacePow2Generic(
    unsigned char * const pixels,
    const unsigned int inputWidth, const unsigned int inputHeight,
    const unsigned int desiredWidth, const unsigned int desiredHeight,
    BoxDimensionTest dimensionTest,
    unsigned& outWidth, unsigned& outHeight )
{
  if( pixels == 0 )
  {
    return;
  }
  ValidateScalingParameters( inputWidth, inputHeight, desiredWidth, desiredHeight );

  // Scale the image until it would be smaller than desired, stopping if the
  // resulting height or width would be less than 1:
  unsigned int scaledWidth = inputWidth, scaledHeight = inputHeight;
  while( ContinueScaling( dimensionTest, scaledWidth, scaledHeight, desiredWidth, desiredHeight ) )
      //scaledWidth >> 1u >= desiredWidth && scaledHeight >> 1u >= desiredHeight &&
      //    scaledWidth >> 1u >= 1u           && scaledHeight >> 1u >= 1u )
  {
    const unsigned int lastWidth = scaledWidth;
    scaledWidth  >>= 1u;
    scaledHeight >>= 1u;

    DALI_LOG_INFO( gImageOpsLogFilter, Dali::Integration::Log::Verbose, "Scaling to %u\t%u.\n", scaledWidth, scaledHeight );

    const unsigned int lastScanlinePair = scaledHeight - 1;

    // Scale pairs of scanlines until any spare one at the end is dropped:
    for( unsigned int y = 0; y <= lastScanlinePair; ++y )
    {
      // Scale two scanlines horizontally:
      HalveScanlineInPlace( &pixels[y * 2 * lastWidth * BYTES_PER_PIXEL], lastWidth );
      HalveScanlineInPlace( &pixels[(y * 2 + 1) * lastWidth * BYTES_PER_PIXEL], lastWidth );

      // Scale vertical pairs of pixels while the last two scanlines are still warm in
      // the CPU cache(s):
      // Note, better access patterns for cache-coherence are possible for very large
      // images but even a 4k RGB888 image will use just 24kB of cache (4k pixels
      // * 3 Bpp * 2 scanlines) for two scanlines on the first iteration.
      AverageScanlines(
          &pixels[y * 2 * lastWidth * BYTES_PER_PIXEL],
          &pixels[(y * 2 + 1) * lastWidth * BYTES_PER_PIXEL],
          &pixels[y * scaledWidth * BYTES_PER_PIXEL],
          scaledWidth );
    }
  }

  ///@note: we could finish off with one of two mutually exclusive passes, one squashing horizontally as far as possible, and the other vertically, if we knew a following cpu point or bilinear filter would restore the desired aspect ratio.
  outWidth = scaledWidth;
  outHeight = scaledHeight;
}

}

void HalveScanlineInPlaceRGB888(
    unsigned char * const pixels,
    const unsigned int width )
{
  DebugAssertScanlineParameters( pixels, width );

  const unsigned int lastPair = EvenDown( width - 2 );

  for( unsigned int pixel = 0, outPixel = 0; pixel <= lastPair; pixel += 2, ++outPixel )
  {
    // Load all the byte pixel components we need:
    const unsigned int c11 = pixels[pixel * 3];
    const unsigned int c12 = pixels[pixel * 3 + 1];
    const unsigned int c13 = pixels[pixel * 3 + 2];
    const unsigned int c21 = pixels[pixel * 3 + 3];
    const unsigned int c22 = pixels[pixel * 3 + 4];
    const unsigned int c23 = pixels[pixel * 3 + 5];

    // Save the averaged byte pixel components:
    pixels[outPixel * 3]     = AverageComponent( c11, c21 );
    pixels[outPixel * 3 + 1] = AverageComponent( c12, c22 );
    pixels[outPixel * 3 + 2] = AverageComponent( c13, c23 );
  }
}

void HalveScanlineInPlaceRGBA8888(
    unsigned char * const pixels,
    const unsigned int width )
{
  DebugAssertScanlineParameters( pixels, width );
  DALI_ASSERT_DEBUG( ((reinterpret_cast<ptrdiff_t>(pixels) & 3u) == 0u) && "Pointer should be 4-byte aligned for performance on some platforms." );

  uint32_t* const alignedPixels = reinterpret_cast<uint32_t*>(pixels);

  const unsigned int lastPair = EvenDown( width - 2 );

  for( unsigned int pixel = 0, outPixel = 0; pixel <= lastPair; pixel += 2, ++outPixel )
  {
    const uint32_t averaged = AveragePixelRGBA8888( alignedPixels[pixel], alignedPixels[pixel + 1] );
    alignedPixels[outPixel] = averaged;
  }
}

void HalveScanlineInPlaceRGB565( unsigned char * pixels, unsigned int width )
{
  DebugAssertScanlineParameters( pixels, width );
  DALI_ASSERT_DEBUG( ((reinterpret_cast<ptrdiff_t>(pixels) & 1u) == 0u) && "Pointer should be 2-byte aligned for performance on some platforms." );

  uint16_t* const alignedPixels = reinterpret_cast<uint16_t*>(pixels);

  const unsigned int lastPair = EvenDown( width - 2 );

  for( unsigned int pixel = 0, outPixel = 0; pixel <= lastPair; pixel += 2, ++outPixel )
  {
    const uint32_t averaged = AveragePixelRGB565( alignedPixels[pixel], alignedPixels[pixel + 1] );
    alignedPixels[outPixel] = averaged;
  }
}

void HalveScanlineInPlace2Bytes(
    unsigned char * const pixels,
    const unsigned int width )
{
  DebugAssertScanlineParameters( pixels, width );

  const unsigned int lastPair = EvenDown( width - 2 );

  for( unsigned int pixel = 0, outPixel = 0; pixel <= lastPair; pixel += 2, ++outPixel )
  {
    // Load all the byte pixel components we need:
    const unsigned int c11 = pixels[pixel * 2];
    const unsigned int c12 = pixels[pixel * 2 + 1];
    const unsigned int c21 = pixels[pixel * 2 + 2];
    const unsigned int c22 = pixels[pixel * 2 + 3];

    // Save the averaged byte pixel components:
    pixels[outPixel * 2]     = AverageComponent( c11, c21 );
    pixels[outPixel * 2 + 1] = AverageComponent( c12, c22 );
  }
}

void HalveScanlineInPlace1Byte(
    unsigned char * const pixels,
    const unsigned int width )
{
  DebugAssertScanlineParameters( pixels, width );

  const unsigned int lastPair = EvenDown( width - 2 );

  for( unsigned int pixel = 0, outPixel = 0; pixel <= lastPair; pixel += 2, ++outPixel )
  {
    // Load all the byte pixel components we need:
    const unsigned int c1 = pixels[pixel];
    const unsigned int c2 = pixels[pixel + 1];

    // Save the averaged byte pixel component:
    pixels[outPixel] = AverageComponent( c1, c2 );
  }
}

/**
 * @ToDo: Optimise for ARM using a 4 bytes at a time loop wrapped around the single ARMV6 instruction: UHADD8  R4, R0, R5. Note, this is not neon. It runs in the normal integer pipeline so there is no downside like a stall moving between integer and copro, or extra power for clocking-up the idle copro.
 * if (widthInComponents >= 7) { word32* aligned1 = scanline1 + 3 & 3; word32* aligned1_end = scanline1 + widthInPixels & 3; while(aligned1 < aligned1_end) { UHADD8  *aligned1++, *aligned2++, *alignedoutput++ } .. + 0 to 3 spare pixels at each end.
 */
void AverageScanlines1(
    const unsigned char * const scanline1,
    const unsigned char * const __restrict__ scanline2,
    unsigned char* const outputScanline,
    const unsigned int width )
{
  DebugAssertDualScanlineParameters( scanline1, scanline2, outputScanline, width );

  for( unsigned int component = 0; component < width; ++component )
  {
    outputScanline[component] = AverageComponent( scanline1[component], scanline2[component] );
  }
}

void AverageScanlines2(
    const unsigned char * const scanline1,
    const unsigned char * const __restrict__ scanline2,
    unsigned char* const outputScanline,
    const unsigned int width )
{
  DebugAssertDualScanlineParameters( scanline1, scanline2, outputScanline, width * 2 );

  for( unsigned int component = 0; component < width * 2; ++component )
  {
    outputScanline[component] = AverageComponent( scanline1[component], scanline2[component] );
  }
}

void AverageScanlines3(
    const unsigned char * const scanline1,
    const unsigned char * const __restrict__ scanline2,
    unsigned char* const outputScanline,
    const unsigned int width )
{
  DebugAssertDualScanlineParameters( scanline1, scanline2, outputScanline, width * 3 );

  for( unsigned int component = 0; component < width * 3; ++component )
  {
    outputScanline[component] = AverageComponent( scanline1[component], scanline2[component] );
  }
}

void AverageScanlinesRGBA8888(
    const unsigned char * const scanline1,
    const unsigned char * const __restrict__ scanline2,
    unsigned char * const outputScanline,
    const unsigned int width )
{
  DebugAssertDualScanlineParameters( scanline1, scanline2, outputScanline, width * 4 );
  DALI_ASSERT_DEBUG( ((reinterpret_cast<ptrdiff_t>(scanline1) & 3u) == 0u) && "Pointer should be 4-byte aligned for performance on some platforms." );
  DALI_ASSERT_DEBUG( ((reinterpret_cast<ptrdiff_t>(scanline2) & 3u) == 0u) && "Pointer should be 4-byte aligned for performance on some platforms." );
  DALI_ASSERT_DEBUG( ((reinterpret_cast<ptrdiff_t>(outputScanline) & 3u) == 0u) && "Pointer should be 4-byte aligned for performance on some platforms." );

  const uint32_t* const alignedScanline1 = reinterpret_cast<const uint32_t*>(scanline1);
  const uint32_t* const alignedScanline2 = reinterpret_cast<const uint32_t*>(scanline2);
  uint32_t* const alignedOutput = reinterpret_cast<uint32_t*>(outputScanline);

  for( unsigned int pixel = 0; pixel < width; ++pixel )
  {
    alignedOutput[pixel] = AveragePixelRGBA8888( alignedScanline1[pixel], alignedScanline2[pixel] );
  }
}

void AverageScanlinesRGB565(
    const unsigned char * const scanline1,
    const unsigned char * const __restrict__ scanline2,
    unsigned char * const outputScanline,
    const unsigned int width )
{
  DebugAssertDualScanlineParameters( scanline1, scanline2, outputScanline, width * 2 );
  DALI_ASSERT_DEBUG( ((reinterpret_cast<ptrdiff_t>(scanline1) & 1u) == 0u) && "Pointer should be 2-byte aligned for performance on some platforms." );
  DALI_ASSERT_DEBUG( ((reinterpret_cast<ptrdiff_t>(scanline2) & 1u) == 0u) && "Pointer should be 2-byte aligned for performance on some platforms." );
  DALI_ASSERT_DEBUG( ((reinterpret_cast<ptrdiff_t>(outputScanline) & 1u) == 0u) && "Pointer should be 2-byte aligned for performance on some platforms." );

  const uint16_t* const alignedScanline1 = reinterpret_cast<const uint16_t*>(scanline1);
  const uint16_t* const alignedScanline2 = reinterpret_cast<const uint16_t*>(scanline2);
  uint16_t* const alignedOutput = reinterpret_cast<uint16_t*>(outputScanline);

  for( unsigned int pixel = 0; pixel < width; ++pixel )
  {
    alignedOutput[pixel] = AveragePixelRGB565( alignedScanline1[pixel], alignedScanline2[pixel] );
  }
}

void DownscaleInPlacePow2RGB888(
    unsigned char * const pixels,
    const unsigned int inputWidth, const unsigned int inputHeight,
    const unsigned int desiredWidth, const unsigned int desiredHeight,
    BoxDimensionTest dimensionTest,
    unsigned& outWidth, unsigned& outHeight )
{
  DownscaleInPlacePow2Generic<3, HalveScanlineInPlaceRGB888, AverageScanlines3>( pixels, inputWidth, inputHeight, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight );
}

void DownscaleInPlacePow2RGBA8888(
    unsigned char * const pixels,
    const unsigned int inputWidth, const unsigned int inputHeight,
    const unsigned int desiredWidth, const unsigned int desiredHeight,
    BoxDimensionTest dimensionTest,
    unsigned& outWidth, unsigned& outHeight )
{
  DALI_ASSERT_DEBUG( ((reinterpret_cast<ptrdiff_t>(pixels) & 3u) == 0u) && "Pointer should be 4-byte aligned for performance on some platforms." );
  DownscaleInPlacePow2Generic<4, HalveScanlineInPlaceRGBA8888, AverageScanlinesRGBA8888>( pixels, inputWidth, inputHeight, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight );
}

void DownscaleInPlacePow2RGB565(
    unsigned char * pixels,
    unsigned int inputWidth, unsigned int inputHeight,
    unsigned int desiredWidth, unsigned int desiredHeight,
    BoxDimensionTest dimensionTest,
    unsigned int& outWidth, unsigned int& outHeight )
{
  DownscaleInPlacePow2Generic<2, HalveScanlineInPlaceRGB565, AverageScanlinesRGB565>( pixels, inputWidth, inputHeight, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight );
}

/**
 * @copydoc DownscaleInPlacePow2RGB888
 *
 * For 2-byte formats such as lum8alpha8, but not packed 16 bit formats like RGB565.
 */
void DownscaleInPlacePow2ComponentPair(
    unsigned char * const pixels,
    const unsigned int inputWidth, const unsigned int inputHeight,
    const unsigned int desiredWidth, const unsigned int desiredHeight,
    BoxDimensionTest dimensionTest,
    unsigned& outWidth, unsigned& outHeight )
{
  DownscaleInPlacePow2Generic<2, HalveScanlineInPlace2Bytes, AverageScanlines2>( pixels, inputWidth, inputHeight, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight );
}

void DownscaleInPlacePow2SingleBytePerPixel(
    unsigned char * pixels,
    unsigned int inputWidth, unsigned int inputHeight,
    unsigned int desiredWidth, unsigned int desiredHeight,
    BoxDimensionTest dimensionTest,
    unsigned int& outWidth, unsigned int& outHeight )
{
  DownscaleInPlacePow2Generic<1, HalveScanlineInPlace1Byte, AverageScanlines1>( pixels, inputWidth, inputHeight, desiredWidth, desiredHeight, dimensionTest, outWidth, outHeight );
}

} /* namespace Platform */
} /* namespace Internal */
} /* namespace Dali */
