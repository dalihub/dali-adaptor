/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/adaptor-framework/widget-application.h>
#include <dali/public-api/adaptor-framework/widget.h>

using namespace Dali;

void utc_dali_widget_application_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_widget_application_cleanup(void)
{
  test_return_value = TET_PASS;
}

namespace
{
Widget CreateWidgetFunction(const std::string&)
{
  return Dali::Widget();
}
} // namespace

int UtcDaliWidgetApplicationRegisterWidgetCreatingFunctionNegative(void)
{
  Dali::WidgetApplication instance;
  try
  {
    std::string arg1;
    instance.RegisterWidgetCreatingFunction(arg1, &CreateWidgetFunction);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWidgetApplicationConstructorsPositive(void)
{
  WidgetApplication widget1;

  // copy constructor
  WidgetApplication widget2 = WidgetApplication(widget1);

  // copy assignment
  widget1 = widget2;

  // move constructor
  WidgetApplication widget3 = WidgetApplication(std::move(widget1));

  // move assignemnt
  widget2 = std::move(widget3);

  DALI_TEST_CHECK(!widget1);
  DALI_TEST_CHECK(!widget2);
  DALI_TEST_CHECK(!widget3);

  END_TEST;
}
