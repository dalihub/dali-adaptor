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

#include <dali-test-suite-utils.h>
#include <dali/dali.h>
#include <stdlib.h>

using namespace Dali;

void utc_dali_ui_context_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_ui_context_cleanup(void)
{
  test_return_value = TET_PASS;
}

// Positive Tests

int UtcDaliUiContextGetP(void)
{
  // UiContext::Get() returns an empty handle when no Application is running.
  // This test verifies the call does not crash and returns a handle (possibly empty).
  UiContext uiContext = UiContext::Get();

  // In a non-running adaptor environment the handle is expected to be empty,
  // but the call itself must not throw.
  DALI_TEST_CHECK(true);

  END_TEST;
}

int UtcDaliUiContextConstructorP(void)
{
  // Default constructor must produce an empty (invalid) handle.
  UiContext uiContext;

  DALI_TEST_CHECK(!uiContext);

  END_TEST;
}

int UtcDaliUiContextCopyConstructorP(void)
{
  UiContext uiContext;
  UiContext copy(uiContext);

  DALI_TEST_CHECK(!copy);

  END_TEST;
}

int UtcDaliUiContextAssignmentOperatorP(void)
{
  UiContext uiContext;
  UiContext assigned;
  assigned = uiContext;

  DALI_TEST_CHECK(!assigned);

  END_TEST;
}

int UtcDaliUiContextMoveConstructorP(void)
{
  UiContext uiContext;
  UiContext moved = std::move(uiContext);

  DALI_TEST_CHECK(!moved);

  END_TEST;
}

int UtcDaliUiContextMoveAssignmentP(void)
{
  UiContext uiContext;
  UiContext moved;
  moved = std::move(uiContext);

  DALI_TEST_CHECK(!moved);

  END_TEST;
}

// Negative Tests

int UtcDaliUiContextGetDefaultWindowN(void)
{
  UiContext uiContext;

  try
  {
    (void)uiContext.GetDefaultWindow();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliUiContextAddIdleN(void)
{
  UiContext uiContext;

  try
  {
    CallbackBase* callback = nullptr;
    uiContext.AddIdle(callback);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliUiContextRemoveIdleN(void)
{
  UiContext uiContext;

  try
  {
    CallbackBase* callback = nullptr;
    uiContext.RemoveIdle(callback);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliUiContextFlushUpdateMessagesN(void)
{
  UiContext uiContext;

  try
  {
    uiContext.FlushUpdateMessages();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliUiContextSetApplicationLocaleN(void)
{
  UiContext uiContext;

  try
  {
    uiContext.SetApplicationLocale("en_US");
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}
