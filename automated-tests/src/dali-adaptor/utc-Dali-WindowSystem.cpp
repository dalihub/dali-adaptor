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
#include <dali/dali.h>
#include <dali/devel-api/adaptor-framework/window-system-devel.h>

using namespace Dali;

void utc_dali_window_system_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_window_system_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliWindowSystemGetScreenSizeP(void)
{
  try
  {
    int width, height;
    DevelWindowSystem::GetScreenSize(width, height);

    // The width and height refer ecore_x_screen_size_get defined in utc-Dali-Window.cpp.
    DALI_TEST_CHECK((width == 100) && (height == 100));
  }
  catch(...)
  {
    DALI_TEST_CHECK(false);
  }

  END_TEST;
}