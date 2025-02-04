/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

#include <adaptor-test-application.h>
#include <dali-test-suite-utils.h>
#include <dali/dali.h>
#include <dali/devel-api/adaptor-framework/gl-window.h>
#include <dali/internal/system/linux/dali-ecore-x.h>
#include <test-addon-manager.h>

using namespace Dali;

void utc_dali_glwindow_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_glwindow_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliGlWindowConstructorP(void)
{
  Dali::GlWindow window;
  DALI_TEST_CHECK(!window);
  END_TEST;
}

int UtcDaliGlWindowCopyConstructorP(void)
{
  Dali::GlWindow window;
  Dali::GlWindow copy(window);
  DALI_TEST_CHECK(copy == window);

  END_TEST;
}

int UtcDaliGlWindowConstructorFromInternalPointerN(void)
{
  Internal::Adaptor::GlWindow* internalWindow = NULL;
  Dali::GlWindow               window(internalWindow);
  DALI_TEST_CHECK(!window);

  END_TEST;
}

int UtcDaliGlWindowAssignmentOperatorP(void)
{
  const Dali::GlWindow window;
  Dali::GlWindow       copy;
  DALI_TEST_CHECK(!copy);
  copy = window;
  DALI_TEST_CHECK(copy == window);

  END_TEST;
}

int UtcDaliGlWindowDestructorP(void)
{
  Dali::GlWindow* window = new Dali::GlWindow();
  delete window;

  DALI_TEST_CHECK(true);
  END_TEST;
}

int UtcDaliGlWindowNew1(void)
{
  TestApplication application;
  Test::AddOnManager::Initialize(); // GlWindow requires Gl Window addon so initialize the manager

  try
  {
    PositionSize   windowPosition(0, 0, 10, 10);
    Dali::GlWindow window = Dali::GlWindow::New(windowPosition, "test-window", "test-window-class", true);
    tet_result(TET_FAIL);
  }
  catch(DaliException& e)
  {
    DALI_TEST_ASSERT(e, "Failed to create X window", TEST_LOCATION);
  }

  END_TEST;
}

int UtcDaliGlWindowNew2(void)
{
  TestApplication application;
  Test::AddOnManager::Initialize(); // GlWindow requires Gl Window addon so initialize the manager

  try
  {
    PositionSize   windowPosition(20, 10, 10, 10);
    Dali::GlWindow window = Dali::GlWindow::New(windowPosition, "test-window", "test-window-class", true);

    tet_result(TET_FAIL);
  }
  catch(DaliException& e)
  {
    DALI_TEST_ASSERT(e, "Failed to create X window", TEST_LOCATION);
  }
  END_TEST;
}

int UtcDaliGlWindowSetGraphicsConfigGles20(void)
{
  Dali::GlWindow window;
  try
  {
    window.SetGraphicsConfig(true, true, 0, Dali::GlWindow::GlesVersion::VERSION_2_0);

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowSetGraphicsConfigGles30(void)
{
  Dali::GlWindow window;
  try
  {
    window.SetGraphicsConfig(true, true, 0, Dali::GlWindow::GlesVersion::VERSION_3_0);

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowRaise(void)
{
  Dali::GlWindow window;

  try
  {
    window.Raise();

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowLower(void)
{
  Dali::GlWindow window;

  try
  {
    window.Lower();

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowActivate(void)
{
  Dali::GlWindow window;

  try
  {
    window.Activate();

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowShow(void)
{
  Dali::GlWindow window;

  try
  {
    window.Show();

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowHide(void)
{
  Dali::GlWindow window;

  try
  {
    window.Hide();

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowSetGetPositionSize(void)
{
  Dali::GlWindow window;

  try
  {
    PositionSize setPositionSize(0, 0, 100, 100);
    window.SetPositionSize(setPositionSize);
    PositionSize getPositionSize = window.GetPositionSize();
    DALI_TEST_CHECK(setPositionSize == getPositionSize);

    setPositionSize.x = 10;
    setPositionSize.y = 20;
    window.SetPositionSize(setPositionSize);
    getPositionSize = window.GetPositionSize();
    DALI_TEST_CHECK(setPositionSize == getPositionSize);

    setPositionSize.width  = 50;
    setPositionSize.height = 50;
    window.SetPositionSize(setPositionSize);
    getPositionSize = window.GetPositionSize();
    DALI_TEST_CHECK(setPositionSize == getPositionSize);

    setPositionSize.x      = 0;
    setPositionSize.y      = 0;
    setPositionSize.width  = 100;
    setPositionSize.height = 100;
    window.SetPositionSize(setPositionSize);
    getPositionSize = window.GetPositionSize();
    DALI_TEST_CHECK(setPositionSize == getPositionSize);

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowSetInputRegion(void)
{
  Dali::GlWindow window;

  try
  {
    window.SetInputRegion(Rect<int>(0, 0, 100, 10));

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowOpaqueState(void)
{
  Dali::GlWindow window;

  try
  {
    bool opaquestate = window.IsOpaqueState();
    DALI_TEST_CHECK(opaquestate == true);

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowSetAvailableOrientations(void)
{
  Dali::GlWindow window;

  try
  {
    Dali::Vector<Dali::WindowOrientation> orientations;
    orientations.PushBack(Dali::WindowOrientation::PORTRAIT);
    orientations.PushBack(Dali::WindowOrientation::LANDSCAPE);
    orientations.PushBack(Dali::WindowOrientation::PORTRAIT_INVERSE);
    orientations.PushBack(Dali::WindowOrientation::LANDSCAPE_INVERSE);
    orientations.PushBack(Dali::WindowOrientation::NO_ORIENTATION_PREFERENCE);
    orientations.PushBack(Dali::WindowOrientation::PORTRAIT);
    orientations.PushBack(Dali::WindowOrientation::LANDSCAPE);
    orientations.PushBack(Dali::WindowOrientation::PORTRAIT_INVERSE);
    window.SetAvailableOrientations(orientations);

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowSetPreferredOrientation(void)
{
  Dali::GlWindow window;

  try
  {
    window.SetPreferredOrientation(Dali::WindowOrientation::PORTRAIT);

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowSetPreferredOrientation1(void)
{
  Dali::GlWindow window;

  try
  {
    window.SetPreferredOrientation(Dali::WindowOrientation::NO_ORIENTATION_PREFERENCE);

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliWindowGetCurrentOrientation(void)
{
  Dali::GlWindow window;

  try
  {
    Dali::WindowOrientation orientation = window.GetCurrentOrientation();
    DALI_TEST_CHECK(orientation == Dali::WindowOrientation::PORTRAIT);

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

// Internal callback function
void glInit(void)
{
}

int glRenderFrame(void)
{
  static unsigned int retFlag = 0;
  return retFlag++;
}

void glTerminate(void)
{
}

int UtcDaliGlWindowRegisterGlCallbacks(void)
{
  Dali::GlWindow window;

  try
  {
    window.RegisterGlCallbacks(Dali::MakeCallback(glInit), Dali::MakeCallback(glRenderFrame), Dali::MakeCallback(glTerminate));

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowRenderOnce(void)
{
  Dali::GlWindow window;

  try
  {
    window.RegisterGlCallbacks(Dali::MakeCallback(glInit), Dali::MakeCallback(glRenderFrame), Dali::MakeCallback(glTerminate));
    window.RenderOnce();

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowGetSupportedAuxiliaryHintCount(void)
{
  Dali::GlWindow window;

  try
  {
    window.GetSupportedAuxiliaryHintCount();

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowGetSupportedAuxiliaryHint(void)
{
  Dali::GlWindow window;

  try
  {
    window.GetSupportedAuxiliaryHint(0);

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowAddAuxiliaryHint(void)
{
  Dali::GlWindow window;

  try
  {
    window.AddAuxiliaryHint("stack_pop_to", "1");

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowRemoveAuxiliaryHint(void)
{
  Dali::GlWindow window;

  try
  {
    window.RemoveAuxiliaryHint(0);

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowSetAuxiliaryHintValue(void)
{
  Dali::GlWindow window;

  try
  {
    window.SetAuxiliaryHintValue(0, "0");

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowGetAuxiliaryHintValue(void)
{
  Dali::GlWindow window;

  try
  {
    window.GetAuxiliaryHintValue(0);

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowGetAuxiliaryHintId(void)
{
  Dali::GlWindow window;

  try
  {
    window.GetAuxiliaryHintId("0");

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowFocusChangeSignal(void)
{
  Dali::GlWindow window;

  try
  {
    window.FocusChangeSignal();

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowResizeSignal(void)
{
  Dali::GlWindow window;

  try
  {
    window.ResizeSignal();

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowKeyEventSignal(void)
{
  Dali::GlWindow window;

  try
  {
    window.KeyEventSignal();

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowTouchedSignal(void)
{
  Dali::GlWindow window;

  try
  {
    window.TouchedSignal();

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}

int UtcDaliGlWindowVisibilityChangedSignal(void)
{
  Dali::GlWindow window;

  try
  {
    window.VisibilityChangedSignal();

    DALI_TEST_CHECK(false);
  }
  catch(...)
  {
    DALI_TEST_CHECK(true);
  }
  END_TEST;
}
