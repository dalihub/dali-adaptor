/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/dali.h>
#include <dali/devel-api/adaptor-framework/animated-image-loading.h>
#include <stdlib.h>

using namespace Dali;

namespace
{
// test gif image, resolution: 100*100, 5 frames, delay: 1 second, disposal method: none
static const char* gGif_100_None = TEST_RESOURCE_DIR "/canvas-none.gif";

// this image if not exist, for negative test
static const char* gGifNonExist = "non-exist.gif";

// this image exists but it is not a gif file.
static const char* gGifInvalid = TEST_RESOURCE_DIR "/invalid.gif";

} // namespace

void utc_dali_animated_image_loader_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_animated_image_loader_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliAnimatedImageLoadingCopyMoveP(void)
{
  Dali::AnimatedImageLoading animatedImageLoading = Dali::AnimatedImageLoading::New(gGif_100_None, true);

  Dali::AnimatedImageLoading copied = animatedImageLoading;

  DALI_TEST_EQUALS((bool)animatedImageLoading, true, TEST_LOCATION);
  DALI_TEST_EQUALS((bool)copied, true, TEST_LOCATION);

  Dali::AnimatedImageLoading moved = std::move(animatedImageLoading);

  DALI_TEST_EQUALS((bool)animatedImageLoading, false, TEST_LOCATION);
  DALI_TEST_EQUALS((bool)copied, true, TEST_LOCATION);
  DALI_TEST_EQUALS((bool)moved, true, TEST_LOCATION);

  Dali::AnimatedImageLoading copiedAssign;
  copiedAssign = copied;

  Dali::AnimatedImageLoading movedAssign;
  movedAssign = std::move(moved);

  DALI_TEST_EQUALS((bool)animatedImageLoading, false, TEST_LOCATION);
  DALI_TEST_EQUALS((bool)copied, true, TEST_LOCATION);
  DALI_TEST_EQUALS((bool)moved, false, TEST_LOCATION);
  DALI_TEST_EQUALS((bool)copiedAssign, true, TEST_LOCATION);
  DALI_TEST_EQUALS((bool)movedAssign, true, TEST_LOCATION);

  END_TEST;
}

int UtcDaliAnimatedImageLoadingGetImageSizeP(void)
{
  Dali::AnimatedImageLoading animatedImageLoading = Dali::AnimatedImageLoading::New(gGif_100_None, true);
  ImageDimensions            imageSize            = animatedImageLoading.GetImageSize();

  // Check that the image size is [100, 100]
  DALI_TEST_EQUALS(imageSize.GetWidth(), 100u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageSize.GetHeight(), 100u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliAnimatedImageLoadingGetImageSizeN(void)
{
  Dali::AnimatedImageLoading animatedImageLoading = Dali::AnimatedImageLoading::New(gGifNonExist, true);
  ImageDimensions            imageSize            = animatedImageLoading.GetImageSize();

  // Check that it returns zero size when the animated image is not valid
  DALI_TEST_EQUALS(imageSize.GetWidth(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(imageSize.GetHeight(), 0u, TEST_LOCATION);

  END_TEST;
}

int UtcDaliAnimatedImageLoadingInvalidGif(void)
{
  Dali::AnimatedImageLoading animatedImageLoading = Dali::AnimatedImageLoading::New(gGifInvalid, true);
  Dali::Devel::PixelBuffer   pixelBuffer          = animatedImageLoading.LoadFrame(0);

  // The pixel buffer should be empty.
  DALI_TEST_CHECK(!pixelBuffer);

  END_TEST;
}
