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

// CLASS HEADER
#include <dali-test-suite-utils.h>
#include <dali/dali.h>
#include <dali/extension-api/adaptor-framework/adaptor-extension.h>

using namespace Dali;

void utc_dali_adaptor_extension_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_adaptor_extension_cleanup(void)
{
  test_return_value = TET_PASS;
}

// The test environment has no display, so no Adaptor is ever instantiated.
// These cases verify that the extension-api entry points are exported and that
// they surface the "no adaptor" condition as a DaliException rather than crashing.

int UtcDaliAdaptorExtensionGetRenderThreadIdN(void)
{
  try
  {
    Dali::Extension::GetRenderThreadId();

    tet_result(TET_FAIL);
  }
  catch(DaliException& e)
  {
    DALI_TEST_ASSERT(e, "Adaptor not instantiated", TEST_LOCATION);
  }

  END_TEST;
}

int UtcDaliAdaptorExtensionGetWindowsN(void)
{
  try
  {
    Dali::Extension::GetWindows();

    tet_result(TET_FAIL);
  }
  catch(DaliException& e)
  {
    DALI_TEST_ASSERT(e, "Adaptor not instantiated", TEST_LOCATION);
  }

  END_TEST;
}
