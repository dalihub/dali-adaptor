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

#include <dali-test-suite-utils.h>
#include <stdlib.h>
#include <iostream>

#include <dali/internal/imaging/common/loader-bmp.h>
#include "image-loaders.h"

using namespace Dali;

namespace
{
static const LoadFunctions BmpLoaders(TizenPlatform::LoadBmpHeader, TizenPlatform::LoadBitmapFromBmp);

} // Unnamed namespace.

int UtcDaliBmp24bpp(void)
{
  ImageDetails image(TEST_IMAGE_DIR "/flag-24bpp.bmp", 32u, 32u);

  TestImageLoading(image, BmpLoaders);

  END_TEST;
}
int UtcDaliBmp32bpp(void)
{
  ImageDetails image(TEST_IMAGE_DIR "/test_32bpp.bmp", 127u, 64u);

  // Note : Currently we hard-overrite alpha channel as 255.
  // Please fix UTC if we change current policy.
  TestImageLoading(image, BmpLoaders);

  END_TEST;
}
int UtcDaliBmpRGB8(void)
{
  ImageDetails image(TEST_IMAGE_DIR "/w3c_home_256.bmp", 72u, 48u);

  TestImageLoading(image, BmpLoaders);

  END_TEST;
}