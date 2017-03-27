/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <stdlib.h>
#include <dali/dali.h>
#include <dali-test-suite-utils.h>
#include <dali/devel-api/adaptor-framework/image-loading.h>

using namespace Dali;

namespace
{
// resolution: 34*34, pixel format: RGBA8888
static const char* gImage_34_RGBA = TEST_RESOURCE_DIR "/icon-edit.png";
// resolution: 128*128, pixel format: RGB888
static const char* gImage_128_RGB = TEST_RESOURCE_DIR "/gallery-small-1.jpg";

// this is image is not exist, for negative test
static const char* gImageNonExist = "non-exist.jpg";
}

void utc_dali_load_image_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_load_image_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliLoadImageP(void)
{
  PixelData pixelData = Dali::LoadImageFromFile( gImage_34_RGBA );
  DALI_TEST_CHECK( pixelData );
  DALI_TEST_EQUALS( pixelData.GetWidth(), 34u, TEST_LOCATION );
  DALI_TEST_EQUALS( pixelData.GetHeight(), 34u, TEST_LOCATION );
  DALI_TEST_EQUALS( pixelData.GetPixelFormat(), Pixel::RGBA8888, TEST_LOCATION  );

  PixelData pixelData2 = Dali::LoadImageFromFile( gImage_128_RGB );
  DALI_TEST_CHECK( pixelData2 );
  DALI_TEST_EQUALS( pixelData2.GetWidth(), 128u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelData2.GetHeight(), 128u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelData2.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );

  END_TEST;
}

int UtcDaliLoadImageN(void)
{
  PixelData pixelData = Dali::LoadImageFromFile( gImageNonExist );
  DALI_TEST_CHECK( !pixelData );

  END_TEST;
}


int UtcDaliDownloadImageP(void)
{
  std::string url("file://");
  url.append( gImage_34_RGBA );

  std::string url2("file://");
  url2.append( gImage_128_RGB );

  PixelData pixelData = Dali::DownloadImageSynchronously( url );
  DALI_TEST_CHECK( pixelData );
  DALI_TEST_EQUALS( pixelData.GetWidth(), 34u, TEST_LOCATION );
  DALI_TEST_EQUALS( pixelData.GetHeight(), 34u, TEST_LOCATION );
  DALI_TEST_EQUALS( pixelData.GetPixelFormat(), Pixel::RGBA8888, TEST_LOCATION  );

  PixelData pixelData2 = Dali::DownloadImageSynchronously( url2 );
  DALI_TEST_CHECK( pixelData2 );
  DALI_TEST_EQUALS( pixelData2.GetWidth(), 128u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelData2.GetHeight(), 128u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelData2.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );

  END_TEST;
}

int UtcDaliDownloadImageN(void)
{
  PixelData pixelData = Dali::DownloadImageSynchronously( gImageNonExist );
  DALI_TEST_CHECK( !pixelData );

  END_TEST;
}
