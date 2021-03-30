/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

using namespace Dali;

void utc_dali_native_image_source_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_native_image_source_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliNativeImageSourceNewN(void)
{
  unsigned int width  = 256u;
  unsigned int height = 256u;

  try
  {
    NativeImageSourcePtr nativeImageSource = NativeImageSource::New(width, height, NativeImageSource::COLOR_DEPTH_DEFAULT);
  }
  catch(Dali::DaliException& e)
  {
    DALI_TEST_PRINT_ASSERT(e);
    DALI_TEST_ASSERT(e, "Adaptor::IsAvailable()", TEST_LOCATION);
  }
  catch(...)
  {
    tet_printf("Assertion test failed - wrong Exception\n");
    tet_result(TET_FAIL);
  }

  END_TEST;
}
