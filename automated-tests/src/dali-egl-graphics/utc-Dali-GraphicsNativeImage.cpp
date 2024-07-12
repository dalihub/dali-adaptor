/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <test-graphics-application.h>
#include <test-native-image.h>

using namespace Dali;

void utc_dali_native_image_startup(void)
{
  test_return_value = TET_UNDEF;
}
void utc_dali_native_image_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliGraphicsNativeImage(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliNativeImageTexture01");

  TestNativeImagePointer imageInterface = TestNativeImage::New(16, 16);
  TraceCallStack&        callStack      = imageInterface->mCallStack;
  callStack.EnableLogging(true);
  callStack.Enable(true);

  auto& gl               = app.GetGlAbstraction();
  auto& textureCallStack = gl.GetTextureTrace();
  textureCallStack.EnableLogging(true);
  textureCallStack.Enable(true);

  {
    Texture texture = Texture::New(*(imageInterface.Get()));
    Actor   actor   = CreateRenderableActor(texture, "", "");
    app.GetScene().Add(actor);

    app.SendNotification();
    app.Render(16);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, 1, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, 0, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.FindMethod("PrepareTexture"), true, TEST_LOCATION);
    TraceCallStack::NamedParams params;
    params["target"] << std::hex << GL_TEXTURE_EXTERNAL_OES;
    DALI_TEST_EQUALS(textureCallStack.FindMethodAndParams("BindTexture", params), 1u, TEST_LOCATION);
    Dali::UnparentAndReset(actor);
  }

  app.SendNotification();
  app.Render(16); // Puts texture on discard queue.

  app.SendNotification();
  app.Render(16);

  // Flush the queues
  Graphics::SubmitInfo submitInfo{{}, 0 | Graphics::SubmitFlagBits::FLUSH};
  app.GetGraphicsController().SubmitCommandBuffers(submitInfo);

  DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, 1, TEST_LOCATION);

  END_TEST;
}
