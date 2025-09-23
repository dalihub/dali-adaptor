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

// EXTERNAL INCLUDES
#include <dali-test-suite-utils.h>
#include <cstdio>

// INTERNAL INCLUDES
#include <dali/internal/imaging/common/loader-heic.h>
#include <dali/public-api/images/pixel-buffer.h>

using namespace Dali;

int UtcDaliHeicLoaderRejectsNullInput(void)
{
  const ImageLoader::Input input(nullptr);
  unsigned int             width  = 0u;
  unsigned int             height = 0u;
  PixelBuffer              bitmap;
  DALI_TEST_CHECK(!TizenPlatform::LoadHeicHeader(input, width, height));
  DALI_TEST_CHECK(!TizenPlatform::LoadBitmapFromHeic(input, bitmap));
  DALI_TEST_CHECK(!bitmap);
  END_TEST;
}

int UtcDaliHeicLoaderRejectsInvalidInput(void)
{
  FILE* file = tmpfile();
  DALI_TEST_CHECK(file);
  const ImageLoader::Input input(file);
  unsigned int             width  = 0u;
  unsigned int             height = 0u;
  PixelBuffer              bitmap;
  const bool               emptyHeader = TizenPlatform::LoadHeicHeader(input, width, height);
  const bool               emptyBitmap = TizenPlatform::LoadBitmapFromHeic(input, bitmap);
  const unsigned char      invalid[]   = {0, 0, 0, 16, 'f', 't', 'y', 'p', 'h', 'e', 'i', 'c'};
  fwrite(invalid, 1, sizeof(invalid), file);
  fflush(file);
  const bool invalidHeader = TizenPlatform::LoadHeicHeader(input, width, height);
  const bool invalidBitmap = TizenPlatform::LoadBitmapFromHeic(input, bitmap);
  fclose(file);
  DALI_TEST_CHECK(!emptyHeader);
  DALI_TEST_CHECK(!emptyBitmap);
  DALI_TEST_CHECK(!invalidHeader);
  DALI_TEST_CHECK(!invalidBitmap);
  DALI_TEST_CHECK(!bitmap);
  END_TEST;
}
