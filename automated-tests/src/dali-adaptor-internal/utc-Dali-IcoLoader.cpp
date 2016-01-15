/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include <iostream>
#include <stdlib.h>
#include <dali-test-suite-utils.h>

#include "platform-abstractions/tizen/image-loaders/loader-ico.h"
#include "image-loaders.h"

using namespace Dali;

namespace
{

static const LoadFunctions IcoLoaders( TizenPlatform::LoadIcoHeader, TizenPlatform::LoadBitmapFromIco );


// Golden master image data for each icon type:
// Note: The bottom right corner of each image is fully transparent.
// Note: The first 4 sets of data *happen* to be the same, but could be different depending on the image converted.
//       In this case as the original image contains few colors, they can easily fit inside an 8bit palette, thus
//       Causing no image degradation.

uint32_t ImageCheckData_Ico4x4_32bpp_8alpha[] = {
    0xff0000ff, 0x00ff00ff, 0x0000ffff, 0xff00ffff,
    0xc04040ff, 0x40c040ff, 0x4040c0ff, 0xffff00ff,
    0xa06060ff, 0x60a060ff, 0x6060a0ff, 0x00ffffff,
    0xffffffff, 0x808080ff, 0x000000ff, 0x00000000
};

uint32_t ImageCheckData_Ico4x4_24bpp[] = {
    0xff0000ff, 0x00ff00ff, 0x0000ffff, 0xff00ffff,
    0xc04040ff, 0x40c040ff, 0x4040c0ff, 0xffff00ff,
    0xa06060ff, 0x60a060ff, 0x6060a0ff, 0x00ffffff,
    0xffffffff, 0x808080ff, 0x000000ff, 0x00000000
};

uint32_t ImageCheckData_Ico4x4_8bpp[] = {
    0xff0000ff, 0x00ff00ff, 0x0000ffff, 0xff00ffff,
    0xc04040ff, 0x40c040ff, 0x4040c0ff, 0xffff00ff,
    0xa06060ff, 0x60a060ff, 0x6060a0ff, 0x00ffffff,
    0xffffffff, 0x808080ff, 0x000000ff, 0x00000000
};

uint32_t ImageCheckData_Ico4x4_4bpp[] = {
    0xff0000ff, 0x00ff00ff, 0x0000ffff, 0xff00ffff,
    0xc04040ff, 0x40c040ff, 0x4040c0ff, 0xffff00ff,
    0xa06060ff, 0x60a060ff, 0x6060a0ff, 0x00ffffff,
    0xffffffff, 0x808080ff, 0x000000ff, 0x00000000
};

uint32_t ImageCheckData_Ico4x4_1bpp[] = {
    0xA18783ff, 0xA18783ff, 0xA18783ff, 0xA18783ff,
    0xA18783ff, 0xA18783ff, 0xA18783ff, 0xA18783ff,
    0xA18783ff, 0xA18783ff, 0xA18783ff, 0xA18783ff,
    0xA18783ff, 0xA18783ff, 0xA18783ff, 0xA1878300
};


} // Unnamed namespace.


int UtcDaliIco32bpp8alpha(void)
{
  ImageDetails image( TEST_IMAGE_DIR "/test-image-4x4-32bpp.ico", 4u, 4u );

  CompareLoadedImageData( image, IcoLoaders, ImageCheckData_Ico4x4_32bpp_8alpha );

  END_TEST;
}


int UtcDaliIco24bpp1alpha(void)
{
  ImageDetails image( TEST_IMAGE_DIR "/test-image-4x4-24bpp.ico", 4u, 4u );

  CompareLoadedImageData( image, IcoLoaders, ImageCheckData_Ico4x4_24bpp );

  END_TEST;
}


int UtcDaliIco8bpp1alpha(void)
{
  ImageDetails image( TEST_IMAGE_DIR "/test-image-4x4-8bpp.ico", 4u, 4u );

  CompareLoadedImageData( image, IcoLoaders, ImageCheckData_Ico4x4_8bpp );

  END_TEST;
}


int UtcDaliIco4bpp1alpha(void)
{
  ImageDetails image( TEST_IMAGE_DIR "/test-image-4x4-4bpp.ico", 4u, 4u );

  CompareLoadedImageData( image, IcoLoaders, ImageCheckData_Ico4x4_4bpp );

  END_TEST;
}


int UtcDaliIco1bpp1alpha(void)
{
  ImageDetails image( TEST_IMAGE_DIR "/test-image-4x4-1bpp.ico", 4u, 4u );

  CompareLoadedImageData( image, IcoLoaders, ImageCheckData_Ico4x4_1bpp );

  END_TEST;
}



