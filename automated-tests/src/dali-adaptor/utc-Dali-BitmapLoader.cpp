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

#include <stdlib.h>
#include <dali/dali.h>
#include <dali-test-suite-utils.h>
#include <dali/devel-api/adaptor-framework/bitmap-loader.h>

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

void utc_dali_bitmap_loader_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_bitmap_loader_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliBitmapLoaderNew(void)
{
  TestApplication application;

  // invoke default handle constructor
  BitmapLoader loader;

  DALI_TEST_CHECK( !loader );

  // initialise handle
  loader = BitmapLoader::New( gImage_34_RGBA );

  DALI_TEST_CHECK( loader );
  END_TEST;
}

int UtcDaliBitmapLoaderCopyConstructor(void)
{
  TestApplication application;

  BitmapLoader loader = BitmapLoader::New( gImage_34_RGBA);
  BitmapLoader loaderCopy(loader);

  DALI_TEST_EQUALS( (bool)loaderCopy, true, TEST_LOCATION );
  END_TEST;
}

int UtcDaliBitmapLoaderAssignmentOperator(void)
{
  TestApplication application;

  BitmapLoader loader = BitmapLoader::New( gImage_34_RGBA );

  BitmapLoader loader2;
  DALI_TEST_EQUALS( (bool)loader2, false, TEST_LOCATION );

  loader2 = loader;
  DALI_TEST_EQUALS( (bool)loader2, true, TEST_LOCATION );

  END_TEST;
}

int UtcDaliBitmapLoaderGetUrl(void)
{
  TestApplication application;

  BitmapLoader loader = BitmapLoader::New( gImage_34_RGBA );
  DALI_TEST_CHECK( loader.GetUrl() == gImage_34_RGBA );

  END_TEST;
}


int UtcDaliBitmapLoaderLoadP(void)
{
  TestApplication application;

  BitmapLoader loader1 = BitmapLoader::New( gImage_34_RGBA );
  DALI_TEST_CHECK( ! loader1.IsLoaded() );
  loader1.Load();
  DALI_TEST_CHECK( loader1.IsLoaded() );
  PixelDataPtr pixelData1 = loader1.GetPixelData();
  DALI_TEST_CHECK( pixelData1 );
  DALI_TEST_CHECK( pixelData1->GetWidth() == 34u );
  DALI_TEST_CHECK( pixelData1->GetHeight() == 34u );
  DALI_TEST_CHECK( pixelData1->GetPixelFormat() == Pixel::RGBA8888 );

  BitmapLoader loader2 = BitmapLoader::New( gImage_128_RGB );
  DALI_TEST_CHECK( ! loader2.IsLoaded() );
  loader2.Load();
  DALI_TEST_CHECK( loader2.IsLoaded() );
  PixelDataPtr pixelData2 = loader2.GetPixelData();
  DALI_TEST_CHECK( pixelData2 );
  DALI_TEST_CHECK( pixelData2->GetWidth() == 128u );
  DALI_TEST_CHECK( pixelData2->GetHeight() == 128u );
  DALI_TEST_CHECK( pixelData2->GetPixelFormat() == Pixel::RGB888 );

  END_TEST;
}

int UtcDaliBitmapLoaderLoadN(void)
{
  TestApplication application;

  BitmapLoader loader = BitmapLoader::New( gImageNonExist );
  DALI_TEST_CHECK( ! loader.IsLoaded() );
  loader.Load();

  // cannot load image that is not exist
  DALI_TEST_CHECK( ! loader.IsLoaded() );
  PixelDataPtr pixelData = loader.GetPixelData();
  DALI_TEST_CHECK( !pixelData);

  END_TEST;
}
