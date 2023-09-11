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
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/internal/system/linux/dali-ecore-x.h>

using namespace Dali;

void utc_dali_window_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_window_cleanup(void)
{
  test_return_value = TET_PASS;
}

namespace
{
intptr_t screenId = 0; // intptr_t has the same size as a pointer and is platform independent so this can be returned as a pointer in ecore_x_default_screen_get below without compilation warnings

} // unnamed namespace

extern "C"
{
  Ecore_X_Screen* ecore_x_default_screen_get(void)
  {
    screenId += 8;
    return (Ecore_X_Screen*)screenId;
  }

  void ecore_x_screen_size_get(const Ecore_X_Screen* screen, int* w, int* h)
  {
    *w = 100;
    *h = 100;
  }

  Ecore_X_Window ecore_x_window_argb_new(Ecore_X_Window parent, int x, int y, int w, int h)
  {
    return 0;
  }
}

int UtcDaliWindowConstructorP(void)
{
  Dali::Window window;
  DALI_TEST_CHECK(!window);
  END_TEST;
}

int UtcDaliWindowCopyConstructorP(void)
{
  Dali::Window window;
  Dali::Window copy(window);
  DALI_TEST_CHECK(copy == window);

  END_TEST;
}

int UtcDaliWindowConstructorFromInternalPointerN(void)
{
  Internal::Adaptor::Window* internalWindow = NULL;
  Dali::Window               window(internalWindow);
  DALI_TEST_CHECK(!window); // Should not reach here!

  END_TEST;
}

int UtcDaliWindowAssignmentOperatorP(void)
{
  const Dali::Window window;
  Dali::Window       copy;
  DALI_TEST_CHECK(!copy);
  copy = window;
  DALI_TEST_CHECK(copy == window);

  END_TEST;
}

int UtcDaliWindowDestructorP(void)
{
  Dali::Window* window = new Dali::Window();
  delete window;

  DALI_TEST_CHECK(true);
  END_TEST;
}

int UtcDaliWindowNewN(void)
{
  // Attempt to create a new window
  try
  {
    PositionSize windowPosition(0, 0, 0, 0);
    Dali::Window window = Dali::Window::New(windowPosition, "test-window", true);

    tet_result(TET_FAIL);
  }
  catch(DaliException& e)
  {
    DALI_TEST_ASSERT(e, "Failed to create X window", TEST_LOCATION);
  }

  // Attempt to create a new window
  try
  {
    PositionSize windowPosition(0, 0, 0, 0);
    Dali::Window window = Dali::Window::New(windowPosition, "test-window", "test-window-class", true);

    tet_result(TET_FAIL);
  }
  catch(DaliException& e)
  {
    DALI_TEST_ASSERT(e, "Failed to create X window", TEST_LOCATION);
  }

  END_TEST;
}

int UtcDaliWindowSetClassN(void)
{
  Dali::Window window;
  try
  {
    window.SetClass("window-name", "window-class");
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowRaiseN(void)
{
  Dali::Window window;
  try
  {
    window.Raise();
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowLowerN(void)
{
  Dali::Window window;
  try
  {
    window.Lower();
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowActivateN(void)
{
  Dali::Window window;
  try
  {
    window.Activate();
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowMaximizeN(void)
{
  try
  {
    Dali::Window instance;
    DevelWindow::Maximize(instance, true);
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowIsMaximizedN(void)
{
  try
  {
    Dali::Window instance;
    DevelWindow::IsMaximized(instance);
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowSetMaximumSizeN(void)
{
  try
  {
    Dali::Window             instance;
    Dali::Window::WindowSize size(100, 100);
    DevelWindow::SetMaximumSize(instance, size);
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowMinimizeN(void)
{
  try
  {
    Dali::Window instance;
    DevelWindow::Minimize(instance, true);
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowIsMinimizedN(void)
{
  try
  {
    Dali::Window instance;
    DevelWindow::IsMinimized(instance);
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowSetMimimumSizeN(void)
{
  try
  {
    Dali::Window             instance;
    Dali::Window::WindowSize size(100, 100);
    DevelWindow::SetMimimumSize(instance, size);
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowAddAvailableOrientationN(void)
{
  Dali::Window window;
  try
  {
    window.AddAvailableOrientation(Dali::WindowOrientation::PORTRAIT);
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowRemoveAvailableOrientationN(void)
{
  Dali::Window window;
  try
  {
    window.RemoveAvailableOrientation(Dali::WindowOrientation::PORTRAIT);
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowSetPreferredOrientationN(void)
{
  Dali::Window window;
  try
  {
    window.SetPreferredOrientation(Dali::WindowOrientation::PORTRAIT);
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowGetPreferredOrientationN(void)
{
  Dali::Window window;
  try
  {
    Dali::WindowOrientation orientation = window.GetPreferredOrientation();
    DALI_TEST_CHECK(orientation == Dali::WindowOrientation::PORTRAIT); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowSetPositionSizeWithOrientationN(void)
{
  Dali::Window window;
  try
  {
    DevelWindow::SetPositionSizeWithOrientation(window, PositionSize(0, 0, 200, 100), Dali::WindowOrientation::PORTRAIT);
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowGetNativeHandleN(void)
{
  Dali::Window window;
  try
  {
    Dali::Any handle = window.GetNativeHandle();
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowSetAcceptFocusN(void)
{
  Dali::Window window;
  try
  {
    window.SetAcceptFocus(true);
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowIsFocusAcceptableN(void)
{
  Dali::Window window;
  try
  {
    window.IsFocusAcceptable();
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowFocusChangeSignalN(void)
{
  Dali::Window window;
  try
  {
    window.FocusChangeSignal();
    DALI_TEST_CHECK(false); // Should not reach here!
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }

  END_TEST;
}

int UtcDaliWindowSetPositionNegative(void)
{
  Dali::Window instance;
  try
  {
    Dali::Window::WindowPosition arg1;
    instance.SetPosition(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowResizeSignalNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.ResizeSignal();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetBrightnessNegative(void)
{
  Dali::Window instance;
  try
  {
    int arg1(0);
    instance.SetBrightness(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowTouchedSignalNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.TouchedSignal();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowKeyEventSignalNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.KeyEventSignal();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetAcceptFocusNegative(void)
{
  Dali::Window instance;
  try
  {
    bool arg1(false);
    instance.SetAcceptFocus(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetInputRegionNegative(void)
{
  Dali::Window instance;
  try
  {
    Dali::Rect<int> arg1;
    instance.SetInputRegion(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetOpaqueStateNegative(void)
{
  Dali::Window instance;
  try
  {
    bool arg1(false);
    instance.SetOpaqueState(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetTransparencyNegative(void)
{
  Dali::Window instance;
  try
  {
    bool arg1(false);
    instance.SetTransparency(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowAddAuxiliaryHintNegative(void)
{
  Dali::Window instance;
  try
  {
    std::string arg1;
    std::string arg2;
    instance.AddAuxiliaryHint(arg1, arg2);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetScreenOffModeNegative(void)
{
  Dali::Window instance;
  try
  {
    Dali::WindowScreenOffMode arg1(Dali::WindowScreenOffMode::NEVER);
    instance.SetScreenOffMode(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowFocusChangeSignalNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.FocusChangeSignal();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetRenderTaskListNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.GetRenderTaskList();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetBackgroundColorNegative(void)
{
  Dali::Window instance;
  try
  {
    Dali::Vector4 arg1;
    instance.SetBackgroundColor(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowRemoveAuxiliaryHintNegative(void)
{
  Dali::Window instance;
  try
  {
    unsigned int arg1(0u);
    instance.RemoveAuxiliaryHint(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetNotificationLevelNegative(void)
{
  Dali::Window instance;
  try
  {
    Dali::WindowNotificationLevel arg1(Dali::WindowNotificationLevel::NONE);
    instance.SetNotificationLevel(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetAuxiliaryHintValueNegative(void)
{
  Dali::Window instance;
  try
  {
    unsigned int arg1(0u);
    std::string  arg2;
    instance.SetAuxiliaryHintValue(arg1, arg2);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowAddAvailableOrientationNegative(void)
{
  Dali::Window instance;
  try
  {
    Dali::WindowOrientation arg1(Dali::WindowOrientation::PORTRAIT);
    instance.AddAvailableOrientation(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetPreferredOrientationNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.GetPreferredOrientation();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetPreferredOrientationNegative(void)
{
  Dali::Window instance;
  try
  {
    Dali::WindowOrientation arg1(Dali::WindowOrientation::PORTRAIT);
    instance.SetPreferredOrientation(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowRemoveAvailableOrientationNegative(void)
{
  Dali::Window instance;
  try
  {
    Dali::WindowOrientation arg1(Dali::WindowOrientation::PORTRAIT);
    instance.RemoveAvailableOrientation(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowAddNegative(void)
{
  Dali::Window instance;
  try
  {
    Dali::Actor arg1;
    instance.Add(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowHideNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.Hide();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowShowNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.Show();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowLowerNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.Lower();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowRaiseNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.Raise();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowRemoveNegative(void)
{
  Dali::Window instance;
  try
  {
    Dali::Actor arg1;
    instance.Remove(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetSizeNegative(void)
{
  Dali::Window instance;
  try
  {
    Dali::Uint16Pair arg1;
    instance.SetSize(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetTypeNegative(void)
{
  Dali::Window instance;
  try
  {
    Dali::WindowType arg1(Dali::WindowType::NORMAL);
    instance.SetType(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowActivateNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.Activate();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetClassNegative(void)
{
  Dali::Window instance;
  try
  {
    std::string arg1;
    std::string arg2;
    instance.SetClass(arg1, arg2);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetPositionNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.GetPosition();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetLayoutNegative(void)
{
  Dali::Window instance;
  try
  {
    unsigned int arg1(0);
    instance.SetLayout(arg1, arg1, arg1, arg1, arg1, arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetRootLayerNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.GetRootLayer();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetBrightnessNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.GetBrightness();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetLayerCountNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.GetLayerCount();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowIsOpaqueStateNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.IsOpaqueState();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetNativeHandleNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.GetNativeHandle();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetScreenOffModeNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.GetScreenOffMode();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowIsFocusAcceptableNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.IsFocusAcceptable();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetAuxiliaryHintIdNegative(void)
{
  Dali::Window instance;
  try
  {
    std::string arg1;
    instance.GetAuxiliaryHintId(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetBackgroundColorNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.GetBackgroundColor();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetNotificationLevelNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.GetNotificationLevel();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetAuxiliaryHintValueNegative(void)
{
  Dali::Window instance;
  try
  {
    unsigned int arg1(0u);
    instance.GetAuxiliaryHintValue(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetSupportedAuxiliaryHintNegative(void)
{
  Dali::Window instance;
  try
  {
    unsigned int arg1(0u);
    instance.GetSupportedAuxiliaryHint(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetSupportedAuxiliaryHintCountNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.GetSupportedAuxiliaryHintCount();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetDpiNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.GetDpi();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetSizeNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.GetSize();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetTypeNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.GetType();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetLayerNegative(void)
{
  Dali::Window instance;
  try
  {
    unsigned int arg1(0u);
    instance.GetLayer(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowIsVisibleNegative(void)
{
  Dali::Window instance;
  try
  {
    instance.IsVisible();
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetNativeIdNegative(void)
{
  try
  {
    Dali::Window arg1;
    DevelWindow::GetNativeId(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetPositionSizeNegative(void)
{
  try
  {
    Dali::Window    arg1;
    Dali::Rect<int> arg2;
    DevelWindow::SetPositionSize(arg1, arg2);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowWheelEventSignalNegative(void)
{
  try
  {
    Dali::Window arg1;
    DevelWindow::WheelEventSignal(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetCurrentOrientationNegative(void)
{
  try
  {
    Dali::Window arg1;
    DevelWindow::GetCurrentOrientation(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetPhysicalOrientationNegative(void)
{
  try
  {
    Dali::Window arg1;
    DevelWindow::GetPhysicalOrientation(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowVisibilityChangedSignalNegative(void)
{
  try
  {
    Dali::Window arg1;
    DevelWindow::VisibilityChangedSignal(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowAddFrameRenderedCallbackNegative(void)
{
  try
  {
    Dali::Window                        arg1;
    std::unique_ptr<Dali::CallbackBase> arg2;
    int                                 arg3(0);
    DevelWindow::AddFrameRenderedCallback(arg1, std::move(arg2), arg3);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetAvailableOrientationsNegative(void)
{
  try
  {
    Dali::Window                          arg1;
    Dali::Vector<Dali::WindowOrientation> arg2;
    DevelWindow::SetAvailableOrientations(arg1, arg2);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowAddFramePresentedCallbackNegative(void)
{
  try
  {
    Dali::Window                        arg1;
    std::unique_ptr<Dali::CallbackBase> arg2;
    int                                 arg3(0);
    DevelWindow::AddFramePresentedCallback(arg1, std::move(arg2), arg3);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowTransitionEffectEventSignalNegative(void)
{
  try
  {
    Dali::Window arg1;
    DevelWindow::TransitionEffectEventSignal(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowEventProcessingFinishedSignalNegative(void)
{
  try
  {
    Dali::Window arg1;
    DevelWindow::EventProcessingFinishedSignal(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowKeyboardRepeatSettingsChangedSignalNegative(void)
{
  try
  {
    Dali::Window arg1;
    DevelWindow::KeyboardRepeatSettingsChangedSignal(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowUnparentNegative(void)
{
  try
  {
    Dali::Window arg1;
    DevelWindow::Unparent(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowGetParentNegative(void)
{
  try
  {
    Dali::Window arg1;
    DevelWindow::GetParent(arg1);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetParentNegative(void)
{
  try
  {
    Dali::Window arg1;
    Dali::Window arg2;
    DevelWindow::SetParent(arg1, arg2);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetParentWithBelowParentNegative(void)
{
  try
  {
    Dali::Window arg1;
    Dali::Window arg2;
    DevelWindow::SetParent(arg1, arg2, true);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowAddInputRegion(void)
{
  Dali::Window instance;
  try
  {
    Rect<int> includedInputRegion(0, 0, 720, 640);
    DevelWindow::IncludeInputRegion(instance, includedInputRegion);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSubtractInputRegion(void)
{
  Dali::Window instance;
  try
  {
    Rect<int> includedInputRegion(0, 0, 720, 1280);
    DevelWindow::IncludeInputRegion(instance, includedInputRegion);

    Rect<int> excludedInputRegion(0, 641, 720, 640);
    DevelWindow::ExcludeInputRegion(instance, excludedInputRegion);

    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSetNeedsRotationCompletedAcknowledgementNegative(void)
{
  try
  {
    Dali::Window arg1;
    DevelWindow::SetNeedsRotationCompletedAcknowledgement(arg1, true);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowUnsetNeedsRotationCompletedAcknowledgementNegative(void)
{
  try
  {
    Dali::Window arg1;
    DevelWindow::SetNeedsRotationCompletedAcknowledgement(arg1, false);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowSendRotationCompletedAcknowledgementNegative(void)
{
  try
  {
    Dali::Window arg1;
    DevelWindow::SendRotationCompletedAcknowledgement(arg1);

    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowMovedSignalNegative(void)
{
  Dali::Window instance;
  try
  {
    DevelWindow::MovedSignal(instance);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowOrientationChangedSignalNegative(void)
{
  Dali::Window instance;
  try
  {
    DevelWindow::OrientationChangedSignal(instance);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowMouseInOutSignalNegative(void)
{
  Dali::Window instance;
  try
  {
    DevelWindow::MouseInOutEventSignal(instance);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowMouseRelativeSignalNegative(void)
{
  Dali::Window instance;
  try
  {
    DevelWindow::MouseRelativeEventSignal(instance);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowMoveCompletedSignalNegative(void)
{
  Dali::Window instance;
  try
  {
    DevelWindow::MoveCompletedSignal(instance);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowResizeCompletedSignalNegative(void)
{
  Dali::Window instance;
  try
  {
    DevelWindow::ResizeCompletedSignal(instance);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}

int UtcDaliWindowPointerConstraintsSignalNegative(void)
{
  Dali::Window instance;
  try
  {
    DevelWindow::PointerConstraintsSignal(instance);
    DALI_TEST_CHECK(false); // Should not get here
  }
  catch(...)
  {
    DALI_TEST_CHECK(true); // We expect an assert
  }
  END_TEST;
}
