/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
// test gif image, resolution: 100*100, 5 frames, delay: 1 second, disposal method: none for first frame and previous for the rest
static const char* gGif_100_Prev = TEST_RESOURCE_DIR "/canvas-prev.gif";
// test gif image, resolution: 100*100, 5 frames, delay: 1 second, disposal method: background
static const char* gGif_100_Bgnd = TEST_RESOURCE_DIR "/canvas-bgnd.gif";

// this image if not exist, for negative test
static const char* gGifNonExist = "non-exist.gif";

void VerifyLoad(std::vector<Dali::PixelData>& pixelDataList, Dali::Vector<uint32_t>& frameDelayList, uint32_t frameCount, uint32_t width, uint32_t height, uint32_t delay)
{
  DALI_TEST_EQUALS(pixelDataList.size(), frameCount, TEST_LOCATION);
  DALI_TEST_EQUALS(frameDelayList.Size(), frameCount, TEST_LOCATION);

  for(uint32_t idx = 0; idx < frameCount; idx++)
  {
    // Check the image size and delay of each frame
    DALI_TEST_EQUALS(pixelDataList[idx].GetWidth(), width, TEST_LOCATION);
    DALI_TEST_EQUALS(pixelDataList[idx].GetHeight(), height, TEST_LOCATION);
    DALI_TEST_EQUALS(frameDelayList[idx], delay, TEST_LOCATION);
  }
}
} // namespace

void utc_dali_animated_image_loader_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_animated_image_loader_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliAnimatedImageLoadingP(void)
{
  std::vector<Dali::PixelData> pixelDataList;
  Dali::Vector<uint32_t>       frameDelayList;

  Dali::AnimatedImageLoading animatedImageLoading = Dali::AnimatedImageLoading::New(gGif_100_None, true);
  bool                       succeed              = animatedImageLoading.LoadNextNFrames(0u, animatedImageLoading.GetImageCount(), pixelDataList);
  frameDelayList.Clear();
  frameDelayList.Resize(animatedImageLoading.GetImageCount(), 0);
  for(uint32_t i = 0; i < animatedImageLoading.GetImageCount(); ++i)
  {
    frameDelayList[i] = animatedImageLoading.GetFrameInterval(i);
  }

  // Check that the loading succeed
  DALI_TEST_CHECK(succeed);
  VerifyLoad(pixelDataList, frameDelayList, 5u, 100u, 100u, 1000u);

  pixelDataList.clear();
  animatedImageLoading = Dali::AnimatedImageLoading::New(gGif_100_Prev, true);
  succeed              = animatedImageLoading.LoadNextNFrames(0u, animatedImageLoading.GetImageCount(), pixelDataList);
  frameDelayList.Clear();
  frameDelayList.Resize(animatedImageLoading.GetImageCount(), 0);
  for(uint32_t i = 0; i < animatedImageLoading.GetImageCount(); ++i)
  {
    frameDelayList[i] = animatedImageLoading.GetFrameInterval(i);
  }

  // Check that the loading succeed
  DALI_TEST_CHECK(succeed);
  VerifyLoad(pixelDataList, frameDelayList, 5u, 100u, 100u, 1000u);

  pixelDataList.clear();
  animatedImageLoading = Dali::AnimatedImageLoading::New(gGif_100_Bgnd, true);
  succeed              = animatedImageLoading.LoadNextNFrames(0u, animatedImageLoading.GetImageCount(), pixelDataList);
  frameDelayList.Clear();
  frameDelayList.Resize(animatedImageLoading.GetImageCount(), 0);
  for(uint32_t i = 0; i < animatedImageLoading.GetImageCount(); ++i)
  {
    frameDelayList[i] = animatedImageLoading.GetFrameInterval(i);
  }

  // Check that the loading succeed
  DALI_TEST_CHECK(succeed);
  VerifyLoad(pixelDataList, frameDelayList, 5u, 100u, 100u, 1000u);

  END_TEST;
}

int UtcDaliAnimatedImageLoadingN(void)
{
  std::vector<Dali::PixelData> pixelDataList;
  Dali::Vector<uint32_t>       frameDelayList;

  Dali::AnimatedImageLoading animatedImageLoading = Dali::AnimatedImageLoading::New(gGifNonExist, true);
  bool                       succeed              = animatedImageLoading.LoadNextNFrames(0u, animatedImageLoading.GetImageCount(), pixelDataList);

  // Check that the loading failed
  DALI_TEST_CHECK(!succeed);

  // Check that both pixelDataList and frameDelayList are empty
  DALI_TEST_EQUALS(pixelDataList.size(), 0u, TEST_LOCATION);

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
