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

double GetTimeMilliseconds(Dali::Integration::PlatformAbstraction& abstraction)
{
  timespec timeSpec;
  clock_gettime(CLOCK_MONOTONIC, &timeSpec);
  return (timeSpec.tv_sec * 1e3) + (timeSpec.tv_nsec / 1e6);
}

/** Live platform abstraction recreated for each test case. */
TizenPlatform::TizenPlatformAbstraction* gAbstraction = 0;

/** A variety of parameters to reach different code paths in image loading code. */
std::vector<ImageParameters> gCancelAttributes;

void utc_dali_loading_startup(void)
{
  test_return_value = TET_UNDEF;
  gAbstraction      = TizenPlatform::CreatePlatformAbstraction();

  // Setup some loading parameters to engage post-processing stages:

  ImageParameters defaultAttributes;
  defaultAttributes.first = ImageDimensions(160, 120);
  gCancelAttributes.push_back(defaultAttributes);

  // Hit the derived dimensions code:
  ImageParameters deriveWidthAttributes;
  deriveWidthAttributes.first = ImageDimensions(0, 120);
  gCancelAttributes.push_back(deriveWidthAttributes);

  ImageParameters deriveHeightAttributes;
  deriveHeightAttributes.first = ImageDimensions(160, 0);
  gCancelAttributes.push_back(deriveHeightAttributes);

  // Try to push a tall crop:
  ImageParameters tallAttributes1;
  tallAttributes1.first = ImageDimensions(160, 480);
  ImageParameters tallAttributes2;
  tallAttributes2.first = ImageDimensions(160, 509);
  ImageParameters tallAttributes3;
  tallAttributes3.first = ImageDimensions(37, 251);
  gCancelAttributes.push_back(tallAttributes1);
  gCancelAttributes.push_back(tallAttributes2);
  gCancelAttributes.push_back(tallAttributes3);

  // Try to push a wide crop:
  ImageParameters wideAttributes1;
  wideAttributes1.first = ImageDimensions(320, 60);
  ImageParameters wideAttributes2;
  wideAttributes2.first = ImageDimensions(317, 60);
  ImageParameters wideAttributes3;
  wideAttributes3.first = ImageDimensions(317, 53);
  gCancelAttributes.push_back(wideAttributes1);
  gCancelAttributes.push_back(wideAttributes2);
  gCancelAttributes.push_back(wideAttributes3);

  ///@ToDo: Add attribute variants for all filter modes.

  // Pad the array to a prime number to mitigate any accidental periodic
  // patterns in which image file has which attributes applied to its load:
  srand48(104729);
  const float lastUniques = gCancelAttributes.size() - 0.001f;
  while(gCancelAttributes.size() < 61u)
  {
    gCancelAttributes.push_back(gCancelAttributes[unsigned(drand48() * lastUniques)]);
  }
}

void utc_dali_loading_cleanup(void)
{
  delete gAbstraction;
  gAbstraction = 0;

  test_return_value = TET_PASS;
}
