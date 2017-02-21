/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#include "utc-image-loading-common.h"

#include "platform-abstractions/portable/image-operations.h"

using Dali::Internal::Platform::ApplyAttributesToBitmap;

#define ANSI_BLACK   "\x1B[0m"
#define ANSI_RED     "\x1B[31m"
#define ANSI_GREEN   "\x1B[32m"
#define ANSI_YELLOW  "\x1B[33m"
#define ANSI_BLUE    "\x1B[34m"
#define ANSI_MAGENTA "\x1B[35m"
#define ANSI_CYAN    "\x1B[36m"
#define ANSI_WHITE   "\x1B[37m"
#define ANSI_RESET   "\033[0m"

const unsigned char BORDER_FILL_VALUE = 0xff;
const char* ASCII_FILL_VALUE = ANSI_YELLOW "#";
const char* ASCII_PAD_VALUE = ANSI_BLUE "#";
typedef unsigned char PixelBuffer;


void FillBitmap( BitmapPtr bitmap )
{
  // Fill the given bitmap fully.
  const Pixel::Format pixelFormat = bitmap->GetPixelFormat();
  const unsigned int bytesPerPixel = Pixel::GetBytesPerPixel( pixelFormat );
  PixelBuffer * const targetPixels = bitmap->GetBuffer();
  const int bytesToFill = bitmap.Get()->GetImageWidth() * bitmap.Get()->GetImageHeight() * bytesPerPixel;

  memset( targetPixels, BORDER_FILL_VALUE, bytesToFill );
}

typedef Rect< int > ActiveArea;

// This struct defines all information for one test.
struct ImageFittingTestParameters
{
  unsigned int sourceWidth;
  unsigned int sourceHeight;
  unsigned int desiredWidth;
  unsigned int desiredHeight;
  FittingMode::Type fittingMode;

  unsigned int expectedWidth;
  unsigned int expectedHeight;
  ActiveArea expectedActiveImageArea;

  ImageFittingTestParameters( unsigned int newSourceWidth, unsigned int newSourceHeight, unsigned int newDesiredWidth, unsigned int newDesiredHeight, FittingMode::Type newFittingMode,
      unsigned int newExpectedWidth, unsigned int newExpectedHeight, ActiveArea newExpectedActiveImageArea )
  : sourceWidth( newSourceWidth ),
    sourceHeight( newSourceHeight ),
    desiredWidth( newDesiredWidth ),
    desiredHeight( newDesiredHeight ),
    fittingMode( newFittingMode ),
    expectedWidth( newExpectedWidth ),
    expectedHeight( newExpectedHeight ),
    expectedActiveImageArea( newExpectedActiveImageArea )
  {
  }
};

typedef std::vector< ImageFittingTestParameters > TestContainer;


void PerformFittingTests( TestContainer& tests )
{
  // Iterate through all pre-defined tests.
  for( unsigned int testNumber = 0; testNumber < tests.size(); ++testNumber )
  {
    // Gather info for this test.
    ImageFittingTestParameters &test = tests[ testNumber ];

    unsigned int sourceWidth = test.sourceWidth;
    unsigned int sourceHeight = test.sourceHeight;
    unsigned int desiredWidth = test.desiredWidth;
    unsigned int desiredHeight = test.desiredHeight;
    FittingMode::Type fittingMode = test.fittingMode;

    // Create a source bitmap.
    ImageDimensions desiredDimensions( desiredWidth, desiredHeight );
    SamplingMode::Type samplingMode = SamplingMode::BOX_THEN_LINEAR;
    BitmapPtr sourceBitmap = Integration::Bitmap::New( Integration::Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::OWNED_DISCARD );
    Integration::Bitmap::PackedPixelsProfile *packedView = sourceBitmap->GetPackedPixelsProfile();
    const Pixel::Format pixelFormat = sourceBitmap->GetPixelFormat();
    packedView->ReserveBuffer( pixelFormat, sourceWidth, sourceHeight, sourceWidth, sourceHeight );

    // Completely fill the source bitmap (with white).
    FillBitmap( sourceBitmap );

    // Perform fitting operations (this is the method we are testing).
    BitmapPtr newBitmap = ApplyAttributesToBitmap( sourceBitmap, desiredDimensions, fittingMode, samplingMode );

    DALI_TEST_CHECK( newBitmap );

    // As we do not need performance within this test, we branch to exit here (for readability, maintainability).
    if( !newBitmap )
    {
      return;
    }

    Bitmap *bitmap = newBitmap.Get();

    unsigned int resultWidth = bitmap->GetImageWidth();
    unsigned int resultHeight = bitmap->GetImageHeight();

    // Check the dimensions of the modified image match against the expected values defined in the test.
    DALI_TEST_EQUALS( resultWidth, test.expectedWidth, TEST_LOCATION );
    DALI_TEST_EQUALS( resultHeight, test.expectedHeight, TEST_LOCATION );

    PixelBuffer* resultBuffer = bitmap->GetBuffer();
    const unsigned int bytesPerPixel = Pixel::GetBytesPerPixel( pixelFormat );

    // We generate an ASCII representation of the source, desired and result images to log, purely as a debugging aid.
    // (0 = border, 1 = active image area - from the source image).
    std::string xSourceImageString( sourceWidth, '#' );
    std::string xDesiredSizeString( desiredWidth - 2, '-' );
    std::string xDesiredSizePadString( desiredWidth - 2, ' ' );
    tet_printf( "%sRunning test: %d%s\n", ANSI_RED, testNumber + 1, ANSI_RESET );
    tet_printf( "Source image: %s%s%s\n", ANSI_YELLOW, xSourceImageString.c_str(), ANSI_RESET );
    for( unsigned int i = 0; i < sourceHeight - 1; ++i )
    {
      tet_printf( "              %s%s%s\n", ANSI_YELLOW, xSourceImageString.c_str(), ANSI_RESET );
    }
    tet_printf( "Desired size: %s+%s+%s\n", ANSI_YELLOW, xDesiredSizeString.c_str(), ANSI_RESET );
    for( unsigned int i = 0; i < desiredHeight - 2; ++i )
    {
      tet_printf( "              %s|%s|%s\n", ANSI_YELLOW, xDesiredSizePadString.c_str(), ANSI_RESET );
    }
    tet_printf( "              %s+%s+%s\n", ANSI_YELLOW, xDesiredSizeString.c_str(), ANSI_RESET );

    // We want to calculate the active image area (the area filled with image data as opposed to borders).
    // This is so we can determine if the fitting modes worked correctly.
    ActiveArea resultActiveArea( -1, -1, -1, -1 );

    // Iterate over the result image data to find the active area.
    for( unsigned int y = 0; y < resultHeight; ++y )
    {
      int activeStartX = -1;
      int activeEndX = -1;
      std::string xResultImageString;

      for( unsigned int x = 0; x < resultWidth; ++x )
      {
        bool pixelPopulated = resultBuffer[ x * bytesPerPixel ] != 0x00;

        // If the pixel is filled AND we haven't found a filled pixel yet,
        // this is the horizontal start of the active pixel area (for this line).
        if( pixelPopulated && ( activeStartX == -1 ) )
        {
          activeStartX = x;
        }
        else if( !pixelPopulated && ( activeStartX != -1 ) && ( activeEndX == -1 ) )
        {
          // If the pixel is NOT filled AND we HAVE rpeviously found a filled pixel,
          // then this is the horizontal end of the active pixel area (for this line).
          activeEndX = x + 1;
        }

        // Populate a string with the filled state of the result pixels, to facilitate debugging.
        xResultImageString += pixelPopulated ? ASCII_FILL_VALUE : ASCII_PAD_VALUE;
      }

      // First calculate the X-end span value, if we ran out of image before reaching the end of active image area.
      if( ( activeStartX != -1 ) && ( activeEndX == -1 ) )
      {
        activeEndX = resultWidth - activeStartX;
      }

      // If the X-start pixel on this line is earlier than other lines, the overall active area starts earlier.
      // Note: This is ignored if there was no pixels found.
      if( ( activeStartX != -1 ) && ( ( activeStartX < resultActiveArea.x ) || ( resultActiveArea.x == -1 ) ) )
      {
        resultActiveArea.x = activeStartX;
      }

      // If the X-end pixel on this line is later than other lines, the overall active area starts later.
      // Note: This is ignored if there was no pixels found.
      if( ( activeEndX != -1 ) && ( ( activeEndX > resultActiveArea.width ) || ( resultActiveArea.width == -1 ) ) )
      {
        resultActiveArea.width = activeEndX;
      }

      // If there was an X-start pixel on this line AND we don't yet have a Y-start, this line IS the Y-start.
      if( ( activeStartX != -1 ) && ( resultActiveArea.y == -1 ) )
      {
        resultActiveArea.y = y;
      }

      // If there was no X-start pixel on this line AND we already have a Y-start value,
      // then the last Y becomes the new Y-end value.
      if( ( activeStartX == -1 ) && ( resultActiveArea.y != -1 ) && ( resultActiveArea.height == -1 ) )
      {
        resultActiveArea.height = y - 1;
      }

      if( y == 0 )
      {
        tet_printf( "Result image: %s\n", xResultImageString.c_str() );
      }
      else
      {
        tet_printf( "              %s\n", xResultImageString.c_str() );
      }

      resultBuffer += resultWidth * bytesPerPixel;
    }

    // Calculate the Y-end value, if we ran out of image before reaching the end of active image area.
    if( ( resultActiveArea.y != -1 ) && ( resultActiveArea.height == -1 ) )
    {
      resultActiveArea.height = resultHeight - resultActiveArea.y;
    }

    tet_printf( "%s", ANSI_RESET );
    tet_printf( "Test: %d  Result image dimensions: %d,%d  ActiveArea: %d,%d,%d,%d\n",
        testNumber + 1, resultWidth, resultHeight, resultActiveArea.x, resultActiveArea.y, resultActiveArea.width, resultActiveArea.height );

    // Test the result images active area matches the expected active area defined in the test.
    DALI_TEST_EQUALS( resultActiveArea, test.expectedActiveImageArea, TEST_LOCATION );
  }
}

// Test cases:

// Positive test case for fitting mode: FIT_WIDTH.
int UtcDaliFittingModesFitWidth(void)
{
  tet_printf("Running fitting mode test for: FIT_WIDTH\n");

  TestContainer tests;

  // Here we can define the input and expected output of each test on a single line.
  // Source Width, Source Height, Desired Width, Desired Height, Fitting Mode, Expected Width, Expected Height, ActiveArea: X-start, Y-start, width, height

  // Test Image source size = desired size. Output should be the same.
  tests.push_back( ImageFittingTestParameters( 4, 4, 4, 4, FittingMode::FIT_WIDTH,     4, 4, ActiveArea( 0, 0, 4, 4 ) ) );
  // Test Image source size > desired size, but aspect same. Should scale size down.
  tests.push_back( ImageFittingTestParameters( 4, 4, 2, 2, FittingMode::FIT_WIDTH,     2, 2, ActiveArea( 0, 0, 2, 2 ) ) );
  // Test Image source size < desired size, but aspect same. Should not scale size up.
  tests.push_back( ImageFittingTestParameters( 2, 2, 4, 4, FittingMode::FIT_WIDTH,     2, 2, ActiveArea( 0, 0, 2, 2 ) ) );
  // Test Image source size < desired size, but aspect different. Should crop height, so no borders. No scale up as result has same aspect after crop.
  tests.push_back( ImageFittingTestParameters( 2, 4, 8, 8, FittingMode::FIT_WIDTH,     2, 2, ActiveArea( 0, 0, 2, 2 ) ) );
  // Test Image source size > desired size, but aspect different (w < h). Should crop height, so no borders. No scale as result is same size as desired size.
  tests.push_back( ImageFittingTestParameters( 4, 8, 4, 4, FittingMode::FIT_WIDTH,     4, 4, ActiveArea( 0, 0, 4, 4 ) ) );
  // Test Image source size > desired size, but aspect different (w > h). Should add borders, AND scale down to desired size.
  tests.push_back( ImageFittingTestParameters( 8, 4, 4, 4, FittingMode::FIT_WIDTH,     4, 4, ActiveArea( 0, 1, 4, 2 ) ) );

  PerformFittingTests( tests );

  END_TEST;
}

// Positive test case for fitting mode: FIT_HEIGHT.
int UtcDaliFittingModesFitHeight(void)
{
  tet_printf("Running fitting mode test for: FIT_HEIGHT\n");

  TestContainer tests;

  // Here we can define the input and expected output of each test on a single line.
  // Source Width, Source Height, Desired Width, Desired Height, Fitting Mode, Expected Width, Expected Height, ActiveArea: X-start, Y-start, width, height

  // Test Image source size = desired size. Output should be the same.
  tests.push_back( ImageFittingTestParameters( 4, 4, 4, 4, FittingMode::FIT_HEIGHT,    4, 4, ActiveArea( 0, 0, 4, 4 ) ) );
  // Test Image source size > desired size, but aspect same. Should scale size down.
  tests.push_back( ImageFittingTestParameters( 4, 4, 2, 2, FittingMode::FIT_HEIGHT,    2, 2, ActiveArea( 0, 0, 2, 2 ) ) );
  // Test Image source size < desired size, but aspect same. Should not scale size up.
  tests.push_back( ImageFittingTestParameters( 2, 2, 4, 4, FittingMode::FIT_HEIGHT,    2, 2, ActiveArea( 0, 0, 2, 2 ) ) );
  // Test Image source size < desired size, but aspect different. Should add borders, but not scale overall size up.
  tests.push_back( ImageFittingTestParameters( 2, 4, 8, 8, FittingMode::FIT_HEIGHT,    4, 4, ActiveArea( 1, 0, 4, 4 ) ) );
  // Test Image source size > desired size, but aspect different (w < h). Should add borders, AND scale down to desired size.
  tests.push_back( ImageFittingTestParameters( 4, 8, 4, 4, FittingMode::FIT_HEIGHT,    4, 4, ActiveArea( 1, 0, 4, 4 ) ) );
  // Test Image source size > desired size, but aspect different (w > h). Should crop width, so no borders. No scale as result is same size as desired size.
  tests.push_back( ImageFittingTestParameters( 8, 4, 4, 4, FittingMode::FIT_HEIGHT,    4, 4, ActiveArea( 0, 0, 4, 4 ) ) );

  PerformFittingTests( tests );

  END_TEST;
}

// Positive test case for fitting mode: SHRINK_TO_FIT.
int UtcDaliFittingModesShrinkToFit(void)
{
  tet_printf("Running fitting mode test for: SHRINK_TO_FIT\n");

  TestContainer tests;

  // Here we can define the input and expected output of each test on a single line.
  // Source Width, Source Height, Desired Width, Desired Height, Fitting Mode, Expected Width, Expected Height, ActiveArea: X-start, Y-start, width, height

  // Test Image source size = desired size. Output should be the same.
  tests.push_back( ImageFittingTestParameters( 4, 4, 4, 4, FittingMode::SHRINK_TO_FIT, 4, 4, ActiveArea( 0, 0, 4, 4 ) ) );
  // Test Image source size > desired size, but aspect same. Should scale size down.
  tests.push_back( ImageFittingTestParameters( 4, 4, 2, 2, FittingMode::SHRINK_TO_FIT, 2, 2, ActiveArea( 0, 0, 2, 2 ) ) );
  // Test Image source size < desired size, but aspect same. Should not scale size up.
  tests.push_back( ImageFittingTestParameters( 2, 2, 4, 4, FittingMode::SHRINK_TO_FIT, 2, 2, ActiveArea( 0, 0, 2, 2 ) ) );
  // Test Image source size < desired size, but aspect different. Should add borders, but not scale overall size up, as although image is smaller than desired size, aspect is the same.
  tests.push_back( ImageFittingTestParameters( 2, 4, 8, 8, FittingMode::SHRINK_TO_FIT, 4, 4, ActiveArea( 1, 0, 4, 4 ) ) );
  // Test Image source size > desired size, but aspect different (w < h). Should add borders, AND scale down to desired size.
  tests.push_back( ImageFittingTestParameters( 4, 8, 4, 4, FittingMode::SHRINK_TO_FIT, 4, 4, ActiveArea( 1, 0, 4, 4 ) ) );
  // Test Image source size > desired size, but aspect different (w > h). Should add borders, AND scale down to desired size.
  tests.push_back( ImageFittingTestParameters( 8, 4, 4, 4, FittingMode::SHRINK_TO_FIT, 4, 4, ActiveArea( 0, 1, 4, 2 ) ) );

  PerformFittingTests( tests );

  END_TEST;
}

// Positive test case for fitting mode: SCALE_TO_FILL.
int UtcDaliFittingModesScaleToFill(void)
{
  tet_printf("Running fitting mode test for: SCALE_TO_FILL\n");

  TestContainer tests;

  // Here we can define the input and expected output of each test on a single line.
  // Source Width, Source Height, Desired Width, Desired Height, Fitting Mode, Expected Width, Expected Height, ActiveArea: X-start, Y-start, width, height

  // Test Image source size = desired size. Output should be the same.
  tests.push_back( ImageFittingTestParameters( 4, 4, 4, 4, FittingMode::SCALE_TO_FILL, 4, 4, ActiveArea( 0, 0, 4, 4 ) ) );
  // Test Image source size > desired size, but aspect same. Should scale size down.
  tests.push_back( ImageFittingTestParameters( 4, 4, 2, 2, FittingMode::SCALE_TO_FILL, 2, 2, ActiveArea( 0, 0, 2, 2 ) ) );
  // Test Image source size < desired size, but aspect same. Should not scale size up.
  tests.push_back( ImageFittingTestParameters( 2, 2, 4, 4, FittingMode::SCALE_TO_FILL, 2, 2, ActiveArea( 0, 0, 2, 2 ) ) );
  // Test Image source size < desired size, but aspect different. Should crop height, so no borders. No scale up as result has same aspect after crop.
  tests.push_back( ImageFittingTestParameters( 2, 4, 8, 8, FittingMode::SCALE_TO_FILL, 2, 2, ActiveArea( 0, 0, 2, 2 ) ) );
  // Test Image source size > desired size, but aspect different (w < h). Should crop height, so no borders. No scale as result is same size as desired size.
  tests.push_back( ImageFittingTestParameters( 4, 8, 4, 4, FittingMode::SCALE_TO_FILL, 4, 4, ActiveArea( 0, 0, 4, 4 ) ) );
  // Test Image source size > desired size, but aspect different (w > h). Should crop width, so no borders. No scale as result is same size as desired size.
  tests.push_back( ImageFittingTestParameters( 8, 4, 4, 4, FittingMode::SCALE_TO_FILL, 4, 4, ActiveArea( 0, 0, 4, 4 ) ) );

  PerformFittingTests( tests );

  END_TEST;
}

