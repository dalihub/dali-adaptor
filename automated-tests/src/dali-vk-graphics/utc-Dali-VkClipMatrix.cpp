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
 */

#include <dali-test-suite-utils.h>
#include <dali/dali.h>

#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>
#include <test-actor-utils.h>
#include <test-graphics-framebuffer.h>
#include <test-graphics-vk-application.h>

using namespace Dali;

namespace
{
} // namespace

int UtcDaliClipMatrix01(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliClipMatrix01 Test that various points in clip space are mapped properly");

  auto& controller = app.GetGraphicsController();

  tet_printf("Run graphics loop at least once to ensure scene is setup\n");
  app.SendNotification();
  app.Render(16);

  const Graphics::RenderTargetCreateInfo& renderTargetCreateInfo{};
  auto renderTarget = controller.CreateRenderTarget(renderTargetCreateInfo, nullptr);

  // GL clip space:
  // Y up, x/y range: -1:1  z range -1:1
  //
  // Vulkan clip space:
  // Y down, x/y range: -1:1, z range 0:1

  const Vector3 testPoints[] = {Vector3(0.f, 0.f, 0.f),
                                Vector3(-1.f, 0.f, 0.f),
                                Vector3(1.f, 0.f, 0.f),
                                Vector3(0.f, -1.f, 0.f),
                                Vector3(0.f, 1.f, 0.f),
                                Vector3(0.f, 0.f, -1.f),
                                Vector3(0.f, 0.f, 1.f),
                                Vector3(0.2f, 0.2f, 0.2f)};

  const Vector3 resultPoints[] = {Vector3(0.f, 0.0f, 0.5f),
                                  Vector3(-1.f, 0.f, 0.5f),
                                  Vector3(1.f, 0.f, 0.5f),
                                  Vector3(0.f, 1.f, 0.5f),
                                  Vector3(0.f, -1.f, 0.5f),
                                  Vector3(0.f, 0.f, 0.0f),
                                  Vector3(0.f, 0.f, 1.f),
                                  Vector3(0.2f, -0.2f, 0.6f)};

  const int numTests = sizeof(testPoints) / sizeof(Vector3);

  DALI_TEST_EQUALS(controller.HasClipMatrix(), true, TEST_LOCATION);


  const Matrix& clipMatrix = controller.GetClipMatrix(renderTarget.get());

  for(int i = 0; i < numTests; ++i)
  {
    auto testVec   = Vector4(testPoints[i].x, testPoints[i].y, testPoints[i].z, 1.0f);
    auto resultVec = Vector4(resultPoints[i].x, resultPoints[i].y, resultPoints[i].z, 1.0f);

    auto outVec = clipMatrix * testVec;
    tet_printf(
      "In Vec: (%3.1f, %3.1f, %3.1f) => Out vec: (%3.1f, %3.1f, %3.1f)\n"
      "                            Expected: (%3.1f, %3.1f, %3.1f)    %s\n",
      testPoints[i].x,
      testPoints[i].y,
      testPoints[i].z,
      outVec.x,
      outVec.y,
      outVec.z,
      resultVec.x,
      resultVec.y,
      resultVec.z,
      resultVec == outVec ? "PASS" : "FAIL");
    DALI_TEST_EQUALS(outVec, resultVec, TEST_LOCATION);
  }

  END_TEST;
}
