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
#include <dali/public-api/adaptor-framework/window-data.h>

using namespace Dali;

int UtcDaliWindowData01(void)
{
  // Test default values
  WindowData windowData;

  DALI_TEST_CHECK(windowData.GetWindowType() == WindowType::NORMAL);
  DALI_TEST_CHECK(windowData.GetTransparency() == true);
  DALI_TEST_CHECK(windowData.GetPositionSize().x == 0);
  DALI_TEST_CHECK(windowData.GetPositionSize().y == 0);
  DALI_TEST_CHECK(windowData.GetPositionSize().width == 0);
  DALI_TEST_CHECK(windowData.GetPositionSize().height == 0);

  END_TEST;
}

int UtcDaliWindowData02(void)
{
  // Test SetTransparency and GetTransparency
  WindowData windowData;
  windowData.SetTransparency(false);

  DALI_TEST_CHECK(windowData.GetTransparency() == false);

  END_TEST;
}

int UtcDaliWindowData03(void)
{
  // Test SetWindowType and GetWindowType
  WindowData windowData;
  windowData.SetWindowType(WindowType::UTILITY);

  DALI_TEST_CHECK(windowData.GetWindowType() == WindowType::UTILITY);

  END_TEST;
}

int UtcDaliWindowData04(void)
{
  // Test SetPositionSize and GetPositionSize
  WindowData      windowData;
  Dali::Rect<int> rect(100, 200, 300, 400);
  windowData.SetPositionSize(rect);

  DALI_TEST_CHECK(windowData.GetPositionSize().x == 100);
  DALI_TEST_CHECK(windowData.GetPositionSize().y == 200);
  DALI_TEST_CHECK(windowData.GetPositionSize().width == 300);
  DALI_TEST_CHECK(windowData.GetPositionSize().height == 400);

  END_TEST;
}

int UtcDaliWindowData05(void)
{
  // Test SetFrontBufferRendering and GetFrontBufferRendering
  WindowData windowData;
  windowData.SetFrontBufferRendering(true);

  DALI_TEST_CHECK(windowData.GetFrontBufferRendering() == true);

  END_TEST;
}