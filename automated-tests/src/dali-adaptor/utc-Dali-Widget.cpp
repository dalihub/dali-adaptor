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
#include <dali/public-api/adaptor-framework/widget-impl.h>
#include <dali/public-api/adaptor-framework/widget.h>

using namespace Dali;

void utc_dali_widget_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_widget_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliWidgetConstructorsP(void)
{
  Widget widget1 = Widget::New();
  DALI_TEST_CHECK(widget1);

  // copy constructor
  Widget widget2 = Widget(widget1);
  DALI_TEST_CHECK(widget1 == widget2);

  // copy assignment
  widget1.Reset();
  DALI_TEST_CHECK(!widget1);
  DALI_TEST_CHECK(widget1 != widget2);
  widget1 = widget2;
  DALI_TEST_CHECK(widget1 == widget2);

  // move constructor
  Widget widget3 = Widget(std::move(widget1));
  DALI_TEST_CHECK(widget3);

  // move assignemnt
  widget2.Reset();
  DALI_TEST_CHECK(!widget2);
  widget2 = std::move(widget3);
  DALI_TEST_CHECK(widget2);

  Widget widget4;
  DALI_TEST_CHECK(!widget4);
  widget4 = Widget::New();
  DALI_TEST_CHECK(widget4);

  END_TEST;
}

int UtcDaliWidgetImplOnMethodsP(void)
{
  /// No real test in this function, purely for function and line coverage

  Widget widget = Widget::New();
  DALI_TEST_CHECK(widget);
  Internal::Adaptor::Widget& widgetImpl = Internal::Adaptor::GetImplementation(widget);

  try
  {
    widgetImpl.OnCreate(std::string(), Dali::Window());
    widgetImpl.OnTerminate(std::string(), Dali::Widget::Termination::PERMANENT);
    widgetImpl.OnPause();
    widgetImpl.OnResume();
    widgetImpl.OnResize(Dali::Window());
    widgetImpl.OnUpdate(std::string(), 1);
    DALI_TEST_CHECK(true);
  }
  catch(...)
  {
    DALI_TEST_CHECK(false); // Should not come here
  }

  END_TEST;
}

int UtcDaliWidgetImplSetContentInfoP(void)
{
  Widget                     widget     = Widget::New();
  Internal::Adaptor::Widget& widgetImpl = Internal::Adaptor::GetImplementation(widget);

  try
  {
    widgetImpl.SetContentInfo(std::string());
    DALI_TEST_CHECK(true);
  }
  catch(...)
  {
    DALI_TEST_CHECK(false); // Should not come here
  }

  END_TEST;
}

int UtcDaliWidgetImplKeyEventUsingP(void)
{
  Widget                     widget     = Widget::New();
  Internal::Adaptor::Widget& widgetImpl = Internal::Adaptor::GetImplementation(widget);

  DALI_TEST_CHECK(!widgetImpl.IsKeyEventUsing());
  widgetImpl.SetUsingKeyEvent(true);
  DALI_TEST_CHECK(!widgetImpl.IsKeyEventUsing()); // Still false as Impl is not set WidgetImpl

  END_TEST;
}

int UtcDaliWidgetImplSetInformationP(void)
{
  Widget                     widget     = Widget::New();
  Internal::Adaptor::Widget& widgetImpl = Internal::Adaptor::GetImplementation(widget);

  try
  {
    widgetImpl.SetInformation(Dali::Window(), std::string());
    DALI_TEST_CHECK(true);
  }
  catch(...)
  {
    DALI_TEST_CHECK(false); // Should not come here
  }

  END_TEST;
}

int UtcDaliWidgetImplGetWindowP(void)
{
  const Widget                     widget     = Widget::New();
  const Internal::Adaptor::Widget& widgetImpl = Internal::Adaptor::GetImplementation(widget);

  DALI_TEST_CHECK(!widgetImpl.GetWindow());

  END_TEST;
}

int UtcDaliWidgetImplGetWidgetIdP(void)
{
  const Widget                     widget     = Widget::New();
  const Internal::Adaptor::Widget& widgetImpl = Internal::Adaptor::GetImplementation(widget);

  DALI_TEST_CHECK(widgetImpl.GetWidgetId().empty());

  END_TEST;
}
