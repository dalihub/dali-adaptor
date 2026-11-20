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
#include <utility>

// INTERNAL INCLUDES
#include <dali-test-suite-utils.h>
#include <dali/public-api/adaptor-framework/canvas-renderer/canvas-renderer.h>

using namespace Dali;

void utc_dali_canvas_renderer_startup(void)
{
  test_return_value = TET_UNDEF;
}

void utc_dali_canvas_renderer_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliCanvasRendererCopyMove(void)
{
  CanvasRenderer source;
  CanvasRenderer copy(source);
  DALI_TEST_CHECK(!copy);
  CanvasRenderer copyAssigned;
  DALI_TEST_CHECK(&(copyAssigned = source) == &copyAssigned);
  DALI_TEST_CHECK(!copyAssigned);
  CanvasRenderer moved(std::move(copy));
  DALI_TEST_CHECK(!moved);
  CanvasRenderer moveAssigned;
  DALI_TEST_CHECK(&(moveAssigned = std::move(copyAssigned)) == &moveAssigned);
  DALI_TEST_CHECK(!moveAssigned);
  END_TEST;
}
