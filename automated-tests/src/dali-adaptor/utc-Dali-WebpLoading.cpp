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
#include <dali/devel-api/adaptor-framework/animated-image-loading.h>

using namespace Dali;

namespace
{
// Static lossy WebP image, resolution: 200x100.
const char* gWebp_200_100 = TEST_RESOURCE_DIR "/webp-200x100.webp";
} // namespace

void utc_dali_webp_loading_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_webp_loading_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliWebpLoadingVisualFittingP(void)
{
  Dali::AnimatedImageLoading animatedImageLoading = Dali::AnimatedImageLoading::New(gWebp_200_100, true);

  Dali::PixelBuffer pixelBuffer = animatedImageLoading.LoadFrame(0u,
                                                                 ImageDimensions(50u, 50u),
                                                                 SamplingMode::BOX_THEN_LINEAR);

  DALI_TEST_CHECK(pixelBuffer);
  DALI_TEST_EQUALS(pixelBuffer.GetWidth(), 100u, TEST_LOCATION);
  DALI_TEST_EQUALS(pixelBuffer.GetHeight(), 50u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliWebpLoadingPlanesKeepAspectRatioP(void)
{
  Dali::AnimatedImageLoading animatedImageLoading = Dali::AnimatedImageLoading::New(gWebp_200_100, true);
  std::vector<Dali::PixelBuffer> pixelBuffers;

  const bool loaded = animatedImageLoading.LoadFramePlanes(0u, pixelBuffers, ImageDimensions(50u, 50u));

  DALI_TEST_CHECK(loaded);
  DALI_TEST_CHECK(!pixelBuffers.empty());
  DALI_TEST_EQUALS(pixelBuffers[0].GetWidth(), 100u, TEST_LOCATION);
  DALI_TEST_EQUALS(pixelBuffers[0].GetHeight(), 50u, TEST_LOCATION);

  END_TEST;
}
