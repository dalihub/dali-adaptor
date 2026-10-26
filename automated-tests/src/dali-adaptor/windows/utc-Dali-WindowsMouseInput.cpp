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
#include <windows.h>
#include <cstdint>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>

using namespace Dali;

void utc_dali_windows_mouse_input_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_windows_mouse_input_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliWindowsMouseInputEnabledPerWindow(void)
{
  Window first  = Window::New(PositionSize(0, 0, 100, 100), "mouse-input-first");
  Window second = Window::New(PositionSize(0, 0, 100, 100), "mouse-input-second");
  DALI_TEST_CHECK(DevelWindow::IsMouseInputEnabled(first));
  DALI_TEST_CHECK(DevelWindow::IsMouseInputEnabled(second));
  DALI_TEST_CHECK(DevelWindow::SetMouseInputEnabled(first, false));
  DALI_TEST_CHECK(!DevelWindow::IsMouseInputEnabled(first));
  DALI_TEST_CHECK(DevelWindow::IsMouseInputEnabled(second));
  DALI_TEST_CHECK(DevelWindow::SetMouseInputEnabled(first, false));
  DALI_TEST_CHECK(!DevelWindow::IsMouseInputEnabled(first));
  DALI_TEST_CHECK(DevelWindow::SetMouseInputEnabled(first, true));
  DALI_TEST_CHECK(DevelWindow::IsMouseInputEnabled(first));
  END_TEST;
}

int UtcDaliWindowsMouseInputReleasesCapture(void)
{
  Window window = Window::New(PositionSize(0, 0, 100, 100), "mouse-input-capture");
  HWND   native = reinterpret_cast<HWND>(AnyCast<uintptr_t>(window.GetNativeHandle()));
  DALI_TEST_CHECK(native != nullptr);
  SetCapture(native);
  DALI_TEST_CHECK(GetCapture() == native);

  DALI_TEST_CHECK(DevelWindow::SetMouseInputEnabled(window, false));
  DALI_TEST_CHECK(GetCapture() != native);
  DALI_TEST_CHECK(IsWindowEnabled(native));
  DALI_TEST_CHECK(DevelWindow::SetMouseInputEnabled(window, true));
  DALI_TEST_CHECK(DevelWindow::IsMouseInputEnabled(window));
  END_TEST;
}
