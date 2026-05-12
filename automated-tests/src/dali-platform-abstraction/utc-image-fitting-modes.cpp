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
 *
 */

#include "utc-image-loading-common.h"

#include <dali/internal/imaging/common/image-operations.h>

using Dali::Internal::Platform::ApplyAttributesToBitmap;

/**
 * @brief Tests that ApplyAttributesToBitmap produces the expected output size.
 *
 * The unified loader behavior preserves the source aspect ratio and never
 * upscales. It downscales to fit within the desired box (matching one
 * dimension exactly, the other may be larger), so the result always has
 * the source aspect ratio.
 */
void PerformApplyAttributesTest(
  unsigned int sourceWidth,
  unsigned int sourceHeight,
  unsigned int desiredWidth,
  unsigned int desiredHeight,
  unsigned int expectedWidth,
  unsigned int expectedHeight,
  const char*  location)
{
  auto sourceBitmap = Dali::Devel::PixelBuffer::New(sourceWidth, sourceHeight, Pixel::Format::RGBA8888);

  // Fill the bitmap to make the result non-empty.
  memset(sourceBitmap.GetBuffer(), 0xff, sourceWidth * sourceHeight * Pixel::GetBytesPerPixel(Pixel::Format::RGBA8888));

  ImageDimensions    desiredDimensions(desiredWidth, desiredHeight);
  SamplingMode::Type samplingMode = SamplingMode::BOX_THEN_LINEAR;

  auto result = ApplyAttributesToBitmap(sourceBitmap, desiredDimensions, samplingMode);

  DALI_TEST_CHECK(result);
  DALI_TEST_EQUALS(result.GetWidth(), expectedWidth, location);
  DALI_TEST_EQUALS(result.GetHeight(), expectedHeight, location);
}

// Positive test cases for the unified VISUAL_FITTING loader behavior.
//
// The loader always preserves the source aspect ratio and never upscales.
// When both desired dimensions are given, the result covers at least one
// of the desired dimensions exactly, while the other dimension is set by
// the source aspect ratio.

int UtcDaliApplyAttributesToBitmapSameSize(void)
{
  tet_printf("Source == desired, same aspect: output should be unchanged.\n");

  // Source 4x4, desired 4x4 → 4x4 (same size, same aspect)
  PerformApplyAttributesTest(4, 4, 4, 4, 4, 4, TEST_LOCATION);

  END_TEST;
}

int UtcDaliApplyAttributesToBitmapScaleDown(void)
{
  tet_printf("Source > desired, same aspect: output should scale down.\n");

  // Source 4x4, desired 2x2 → 2x2 (scale down, same aspect)
  PerformApplyAttributesTest(4, 4, 2, 2, 2, 2, TEST_LOCATION);

  END_TEST;
}

int UtcDaliApplyAttributesToBitmapNoUpscale(void)
{
  tet_printf("Source < desired: output should NOT upscale.\n");

  // Source 2x2, desired 4x4 → 2x2 (no upscale)
  PerformApplyAttributesTest(2, 2, 4, 4, 2, 2, TEST_LOCATION);

  END_TEST;
}

int UtcDaliApplyAttributesToBitmapAspectPreservedTallSource(void)
{
  tet_printf("Tall source, square desired: aspect preserved, no upscale.\n");

  // Source 4x8, desired 4x4:
  //   adjustedDesired = 4x8 (source is taller than desired, clamp to requestedWidth=4)
  //   DownscaleBitmap(4x8, 4x8) → 4x8 (no scaling needed)
  //   Result: 4x8
  PerformApplyAttributesTest(4, 8, 4, 4, 4, 8, TEST_LOCATION);

  END_TEST;
}

int UtcDaliApplyAttributesToBitmapAspectPreservedWideSource(void)
{
  tet_printf("Wide source, square desired: aspect preserved, no upscale.\n");

  // Source 8x4, desired 4x4:
  //   adjustedDesired = 8x4 (source is wider than desired, clamp to requestedHeight=4)
  //   DownscaleBitmap(8x4, 8x4) → 8x4 (no scaling needed)
  //   Result: 8x4
  PerformApplyAttributesTest(8, 4, 4, 4, 8, 4, TEST_LOCATION);

  END_TEST;
}

int UtcDaliApplyAttributesToBitmapScaleDownWideSource(void)
{
  tet_printf("Wide source larger than desired, same aspect: output scales down.\n");

  // Source 8x4, desired 4x2 → 4x2 (scale down, same aspect)
  PerformApplyAttributesTest(8, 4, 4, 2, 4, 2, TEST_LOCATION);

  END_TEST;
}
