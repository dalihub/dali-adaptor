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
#include <dali-test-img-utils.h>
#include <dali/devel-api/adaptor-framework/image-loading.h>

using namespace Dali;

namespace
{
// resolution: 34*34, pixel format: RGBA8888
const char* IMAGE_34_RGBA = TEST_RESOURCE_DIR "/icon-edit.png";
// resolution: 128*128, pixel format: RGB888
const char* IMAGE_128_RGB = TEST_RESOURCE_DIR "/gallery-small-1.jpg";

// resolution: 2000*2560, pixel format: RGB888
const char* IMAGE_LARGE_EXIF3_RGB = TEST_RESOURCE_DIR "/f-large-exif-3.jpg";

// resolution: 55*64, pixel format: RGB888
const char* IMAGE_WIDTH_ODD_EXIF1_RGB = TEST_RESOURCE_DIR "/f-odd-exif-1.jpg";
// resolution: 55*64, pixel format: RGB888
const char* IMAGE_WIDTH_ODD_EXIF2_RGB = TEST_RESOURCE_DIR "/f-odd-exif-2.jpg";
// resolution: 55*64, pixel format: RGB888
const char* IMAGE_WIDTH_ODD_EXIF3_RGB = TEST_RESOURCE_DIR "/f-odd-exif-3.jpg";
// resolution: 55*64, pixel format: RGB888
const char* IMAGE_WIDTH_ODD_EXIF4_RGB = TEST_RESOURCE_DIR "/f-odd-exif-4.jpg";
// resolution: 55*64, pixel format: RGB888
const char* IMAGE_WIDTH_ODD_EXIF5_RGB = TEST_RESOURCE_DIR "/f-odd-exif-5.jpg";
// resolution: 55*64, pixel format: RGB888
const char* IMAGE_WIDTH_ODD_EXIF6_RGB = TEST_RESOURCE_DIR "/f-odd-exif-6.jpg";
// resolution: 55*64, pixel format: RGB888
const char* IMAGE_WIDTH_ODD_EXIF7_RGB = TEST_RESOURCE_DIR "/f-odd-exif-7.jpg";
// resolution: 55*64, pixel format: RGB888
const char* IMAGE_WIDTH_ODD_EXIF8_RGB = TEST_RESOURCE_DIR "/f-odd-exif-8.jpg";

// resolution: 50*64, pixel format: RGB888
const char* IMAGE_WIDTH_EVEN_EXIF1_RGB = TEST_RESOURCE_DIR "/f-even-exif-1.jpg";
// resolution: 50*64, pixel format: RGB888
const char* IMAGE_WIDTH_EVEN_EXIF2_RGB = TEST_RESOURCE_DIR "/f-even-exif-2.jpg";
// resolution: 50*64, pixel format: RGB888
const char* IMAGE_WIDTH_EVEN_EXIF3_RGB = TEST_RESOURCE_DIR "/f-even-exif-3.jpg";
// resolution: 50*64, pixel format: RGB888
const char* IMAGE_WIDTH_EVEN_EXIF4_RGB = TEST_RESOURCE_DIR "/f-even-exif-4.jpg";
// resolution: 50*64, pixel format: RGB888
const char* IMAGE_WIDTH_EVEN_EXIF5_RGB = TEST_RESOURCE_DIR "/f-even-exif-5.jpg";
// resolution: 50*64, pixel format: RGB888
const char* IMAGE_WIDTH_EVEN_EXIF6_RGB = TEST_RESOURCE_DIR "/f-even-exif-6.jpg";
// resolution: 50*64, pixel format: RGB888
const char* IMAGE_WIDTH_EVEN_EXIF7_RGB = TEST_RESOURCE_DIR "/f-even-exif-7.jpg";
// resolution: 50*64, pixel format: RGB888
const char* IMAGE_WIDTH_EVEN_EXIF8_RGB = TEST_RESOURCE_DIR "/f-even-exif-8.jpg";


// this is image is not exist, for negative test
const char* IMAGENONEXIST = "non-exist.jpg";
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
  Devel::PixelBuffer pixelBuffer = Dali::LoadImageFromFile( IMAGE_34_RGBA );
  DALI_TEST_CHECK( pixelBuffer );
  DALI_TEST_EQUALS( pixelBuffer.GetWidth(), 34u, TEST_LOCATION );
  DALI_TEST_EQUALS( pixelBuffer.GetHeight(), 34u, TEST_LOCATION );
  DALI_TEST_EQUALS( pixelBuffer.GetPixelFormat(), Pixel::RGBA8888, TEST_LOCATION  );

  Devel::PixelBuffer pixelBuffer2 = Dali::LoadImageFromFile( IMAGE_128_RGB );
  DALI_TEST_CHECK( pixelBuffer2 );
  DALI_TEST_EQUALS( pixelBuffer2.GetWidth(), 128u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBuffer2.GetHeight(), 128u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBuffer2.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );

  Devel::PixelBuffer pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_LARGE_EXIF3_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 2000u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 2560u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );

  Devel::PixelBuffer BufferJpeg1 = Dali::LoadImageFromFile( IMAGE_WIDTH_ODD_EXIF1_RGB );
  DALI_TEST_CHECK( BufferJpeg1 );
  DALI_TEST_EQUALS( BufferJpeg1.GetWidth(), 55u, TEST_LOCATION  );
  DALI_TEST_EQUALS( BufferJpeg1.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( BufferJpeg1.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );

  pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_WIDTH_ODD_EXIF2_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 55u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );
  DALI_IMAGE_TEST_EQUALS( BufferJpeg1, pixelBufferJpeg, 8, TEST_LOCATION);

  pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_WIDTH_ODD_EXIF3_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 55u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );
  DALI_IMAGE_TEST_EQUALS( BufferJpeg1, pixelBufferJpeg, 8, TEST_LOCATION);

  pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_WIDTH_ODD_EXIF4_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 55u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );
  DALI_IMAGE_TEST_EQUALS( BufferJpeg1, pixelBufferJpeg, 8, TEST_LOCATION);

  pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_WIDTH_ODD_EXIF5_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 55u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );
  DALI_IMAGE_TEST_EQUALS( BufferJpeg1, pixelBufferJpeg, 8, TEST_LOCATION);

  pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_WIDTH_ODD_EXIF6_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 55u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );
  DALI_IMAGE_TEST_EQUALS( BufferJpeg1, pixelBufferJpeg, 8, TEST_LOCATION);

  pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_WIDTH_ODD_EXIF7_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 55u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );
  DALI_IMAGE_TEST_EQUALS( BufferJpeg1, pixelBufferJpeg, 8, TEST_LOCATION);

  pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_WIDTH_ODD_EXIF8_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 55u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );
  DALI_IMAGE_TEST_EQUALS( BufferJpeg1, pixelBufferJpeg, 8, TEST_LOCATION);

  BufferJpeg1 = Dali::LoadImageFromFile( IMAGE_WIDTH_EVEN_EXIF1_RGB );
  DALI_TEST_CHECK( BufferJpeg1 );
  DALI_TEST_EQUALS( BufferJpeg1.GetWidth(), 50u, TEST_LOCATION  );
  DALI_TEST_EQUALS( BufferJpeg1.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( BufferJpeg1.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );

  pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_WIDTH_EVEN_EXIF2_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 50u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );
  DALI_IMAGE_TEST_EQUALS( BufferJpeg1, pixelBufferJpeg, 8, TEST_LOCATION);

  pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_WIDTH_EVEN_EXIF3_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 50u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );
  DALI_IMAGE_TEST_EQUALS( BufferJpeg1, pixelBufferJpeg, 8, TEST_LOCATION);

  pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_WIDTH_EVEN_EXIF4_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 50u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );
  DALI_IMAGE_TEST_EQUALS( BufferJpeg1, pixelBufferJpeg, 8, TEST_LOCATION);

  pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_WIDTH_EVEN_EXIF5_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 50u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );
  DALI_IMAGE_TEST_EQUALS( BufferJpeg1, pixelBufferJpeg, 8, TEST_LOCATION);

  pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_WIDTH_EVEN_EXIF6_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 50u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );
  DALI_IMAGE_TEST_EQUALS( BufferJpeg1, pixelBufferJpeg, 8, TEST_LOCATION);

  pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_WIDTH_EVEN_EXIF7_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 50u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );
  DALI_IMAGE_TEST_EQUALS( BufferJpeg1, pixelBufferJpeg, 8, TEST_LOCATION);

  pixelBufferJpeg = Dali::LoadImageFromFile( IMAGE_WIDTH_EVEN_EXIF8_RGB );
  DALI_TEST_CHECK( pixelBufferJpeg );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetWidth(), 50u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetHeight(), 64u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBufferJpeg.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );
  DALI_IMAGE_TEST_EQUALS( BufferJpeg1, pixelBufferJpeg, 8, TEST_LOCATION);

  END_TEST;
}

int UtcDaliLoadImageN(void)
{
  Devel::PixelBuffer pixelBuffer = Dali::LoadImageFromFile( IMAGENONEXIST );
  DALI_TEST_CHECK( !pixelBuffer );

  END_TEST;
}


int UtcDaliDownloadImageP(void)
{
  std::string url("file://");
  url.append( IMAGE_34_RGBA );

  std::string url2("file://");
  url2.append( IMAGE_128_RGB );

  Devel::PixelBuffer pixelBuffer = Dali::DownloadImageSynchronously( url );
  DALI_TEST_CHECK( pixelBuffer );
  DALI_TEST_EQUALS( pixelBuffer.GetWidth(), 34u, TEST_LOCATION );
  DALI_TEST_EQUALS( pixelBuffer.GetHeight(), 34u, TEST_LOCATION );
  DALI_TEST_EQUALS( pixelBuffer.GetPixelFormat(), Pixel::RGBA8888, TEST_LOCATION  );

  Devel::PixelBuffer pixelBuffer2 = Dali::DownloadImageSynchronously( url2 );
  DALI_TEST_CHECK( pixelBuffer2 );
  DALI_TEST_EQUALS( pixelBuffer2.GetWidth(), 128u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBuffer2.GetHeight(), 128u, TEST_LOCATION  );
  DALI_TEST_EQUALS( pixelBuffer2.GetPixelFormat(), Pixel::RGB888, TEST_LOCATION  );

  END_TEST;
}

int UtcDaliDownloadImageN(void)
{
  Devel::PixelBuffer pixelBuffer = Dali::DownloadImageSynchronously( IMAGENONEXIST );
  DALI_TEST_CHECK( !pixelBuffer );

  END_TEST;
}


int UtcDaliDownloadRemoteChunkedImage(void)
{
  std::string url("http://d2k43l0oslhof9.cloudfront.net/platform/image/contents/vc/20/01/58/20170629100630071189_0bf6b911-a847-cba4-e518-be40fe2f579420170629192203240.jpg");

  Devel::PixelBuffer pixelBuffer = Dali::DownloadImageSynchronously( url );
  DALI_TEST_CHECK( pixelBuffer );
  DALI_TEST_EQUALS( pixelBuffer.GetWidth(), 279u, TEST_LOCATION );
  DALI_TEST_EQUALS( pixelBuffer.GetHeight(), 156u, TEST_LOCATION );
  DALI_TEST_EQUALS( pixelBuffer.GetPixelFormat(), Pixel::RGBA8888, TEST_LOCATION  );

  END_TEST;
}
