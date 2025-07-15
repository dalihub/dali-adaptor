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
 */

#include <dali-test-suite-utils.h>
#include <dali/dali.h>

#include <dali/internal/graphics/gles-impl/egl-graphics-controller.h>
#include <test-actor-utils.h>
#include <test-graphics-egl-application.h>
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

int UtcDaliNativeImageTexture01(void)
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

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 2, TEST_LOCATION); // Call 2 times due to it is first time.

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

int UtcDaliGraphicsNativeImagePrepareReturnImageChanged(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliGraphicsNativeImagePrepareReturnImageChanged");

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

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 2, TEST_LOCATION); // Call 2 times due to it is first time.

    TraceCallStack::NamedParams params;
    params["target"] << std::hex << GL_TEXTURE_EXTERNAL_OES;
    DALI_TEST_EQUALS(textureCallStack.FindMethodAndParams("BindTexture", params), 1u, TEST_LOCATION);

    callStack.Reset();
    textureCallStack.Reset();

    app.SendNotification();
    app.Render(16);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, 1, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, 0, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 0, TEST_LOCATION); // Should not be called.

    callStack.Reset();
    textureCallStack.Reset();

    imageInterface->mPrepareTextureResult = Dali::NativeImageInterface::PrepareTextureResult::IMAGE_CHANGED;

    app.SendNotification();
    app.Render(16);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, 1, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, 0, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 1, TEST_LOCATION); // Call 1 time due to image changed at prepare time.

    imageInterface->mPrepareTextureResult = Dali::NativeImageInterface::PrepareTextureResult::NO_ERROR;

    callStack.Reset();
    textureCallStack.Reset();

    app.SendNotification();
    app.Render(16);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, 1, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, 0, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 0, TEST_LOCATION); // Should not be called.

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

int UtcDaliGraphicsNativeImagePrepareReturnError(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliGraphicsNativeImagePrepareReturnError");

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

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 2, TEST_LOCATION); // Call 2 times due to it is first time.

    TraceCallStack::NamedParams params;
    params["target"] << std::hex << GL_TEXTURE_EXTERNAL_OES;
    DALI_TEST_EQUALS(textureCallStack.FindMethodAndParams("BindTexture", params), 1u, TEST_LOCATION);

    callStack.Reset();
    textureCallStack.Reset();

    app.SendNotification();
    app.Render(16);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, 1, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, 0, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 0, TEST_LOCATION); // Should not be called.

    imageInterface->mPrepareTextureResult = Dali::NativeImageInterface::PrepareTextureResult::UNKNOWN_ERROR;

    callStack.Reset();
    textureCallStack.Reset();

    app.SendNotification();
    app.Render(16);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, 1, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, 0, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 0, TEST_LOCATION); // Should not be called.

    // Should not draw the result.
    // TODO : How can we check it?

    callStack.Reset();
    textureCallStack.Reset();

    app.SendNotification();
    app.Render(16);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, 1, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, 0, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 0, TEST_LOCATION); // Should not be called.

    // Should not draw the result.
    // TODO : How can we check it?

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

int UtcDaliGraphicsNativeImageTargetFailed01(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliGraphicsNativeImageTargetFailed01");

  TestNativeImagePointer imageInterface = TestNativeImage::New(16, 16);
  TraceCallStack&        callStack      = imageInterface->mCallStack;
  callStack.EnableLogging(true);
  callStack.Enable(true);

  auto& gl               = app.GetGlAbstraction();
  auto& textureCallStack = gl.GetTextureTrace();
  textureCallStack.EnableLogging(true);
  textureCallStack.Enable(true);

  {
    // Make TargetTexture failed always
    imageInterface->mTargetTextureError = 1000000000u;

    Texture texture = Texture::New(*(imageInterface.Get()));
    Actor   actor   = CreateRenderableActor(texture, "", "");
    app.GetScene().Add(actor);

    app.SendNotification();
    app.Render(16);

    // Note that we could call Flush multiple times. The number of create / destroy call is not determined
    int createCalled = imageInterface->mExtensionCreateCalls;
    tet_printf("Flush called : %d\n", createCalled);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, createCalled, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, createCalled, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 0, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), createCalled, TEST_LOCATION); // Call whenever we try to create.

    callStack.Reset();
    textureCallStack.Reset();
    imageInterface->mExtensionCreateCalls  = 0;
    imageInterface->mExtensionDestroyCalls = 0;

    app.SendNotification();
    app.Render(16);

    // Note that we could call Flush multiple times. The number of create / destroy call is not determined
    createCalled = imageInterface->mExtensionCreateCalls;
    tet_printf("Flush called : %d\n", createCalled);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, createCalled, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, createCalled, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 0, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), createCalled, TEST_LOCATION); // Call whenever we try to create.

    callStack.Reset();
    textureCallStack.Reset();
    imageInterface->mExtensionCreateCalls  = 0;
    imageInterface->mExtensionDestroyCalls = 0;

    // Make TargetTexture success now
    imageInterface->mTargetTextureError = 0u;

    app.SendNotification();
    app.Render(16);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, 1, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, 0, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 2, TEST_LOCATION); // Call 2 times due to it is first time.

    TraceCallStack::NamedParams params;
    params["target"] << std::hex << GL_TEXTURE_EXTERNAL_OES;
    DALI_TEST_EQUALS(textureCallStack.FindMethodAndParams("BindTexture", params), 1u, TEST_LOCATION);

    callStack.Reset();
    textureCallStack.Reset();

    app.SendNotification();
    app.Render(16);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, 1, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, 0, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 0, TEST_LOCATION); // Should not be called.

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

int UtcDaliGraphicsNativeImageTargetFailed02(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliGraphicsNativeImageTargetFailed02 - Destroy uninitialized native image");

  TestNativeImagePointer imageInterface = TestNativeImage::New(16, 16);
  TraceCallStack&        callStack      = imageInterface->mCallStack;
  callStack.EnableLogging(true);
  callStack.Enable(true);

  auto& gl               = app.GetGlAbstraction();
  auto& textureCallStack = gl.GetTextureTrace();
  textureCallStack.EnableLogging(true);
  textureCallStack.Enable(true);

  {
    // Make TargetTexture failed always
    imageInterface->mTargetTextureError = 1000000000u;

    Texture texture = Texture::New(*(imageInterface.Get()));
    Actor   actor   = CreateRenderableActor(texture, "", "");
    app.GetScene().Add(actor);

    app.SendNotification();
    app.Render(16);

    // Note that we could call Flush multiple times. The number of create / destroy call is not determined
    int createCalled = imageInterface->mExtensionCreateCalls;
    tet_printf("Flush called : %d\n", createCalled);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, createCalled, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, createCalled, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 0, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), createCalled, TEST_LOCATION); // Call whenever we try to create.

    callStack.Reset();
    textureCallStack.Reset();
    imageInterface->mExtensionCreateCalls  = 0;
    imageInterface->mExtensionDestroyCalls = 0;

    app.SendNotification();
    app.Render(16);

    // Note that we could call Flush multiple times. The number of create / destroy call is not determined
    createCalled = imageInterface->mExtensionCreateCalls;
    tet_printf("Flush called : %d\n", createCalled);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, createCalled, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, createCalled, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 0, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), createCalled, TEST_LOCATION); // Call whenever we try to create.

    callStack.Reset();
    textureCallStack.Reset();
    imageInterface->mExtensionCreateCalls  = 0;
    imageInterface->mExtensionDestroyCalls = 0;

    // Destroy native image texture during failed.
    Dali::UnparentAndReset(actor);
    texture.Reset();

    // Add dummy actor, to make ensure we render something.
    app.GetScene().Add(CreateRenderableActor(CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 16, 16)));

    // Render several times that we can assume that texture removed ensurely.
    app.SendNotification();
    app.Render(16);

    app.SendNotification();
    app.Render(16);

    app.SendNotification();
    app.Render(16);

    callStack.Reset();
    textureCallStack.Reset();
    imageInterface->mExtensionCreateCalls  = 0;
    imageInterface->mExtensionDestroyCalls = 0;

    // Create new texture
    texture = Texture::New(*(imageInterface.Get()));
    actor   = CreateRenderableActor(texture, "", "");
    app.GetScene().Add(actor);

    // Make it valid again.
    imageInterface->mTargetTextureError = 0u;

    app.SendNotification();
    app.Render(16);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, 1, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, 0, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 2, TEST_LOCATION); // Call 2 times due to it is first time.

    TraceCallStack::NamedParams params;
    params["target"] << std::hex << GL_TEXTURE_EXTERNAL_OES;
    DALI_TEST_EQUALS(textureCallStack.FindMethodAndParams("BindTexture", params), 1u, TEST_LOCATION);

    callStack.Reset();
    textureCallStack.Reset();

    app.SendNotification();
    app.Render(16);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, 1, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, 0, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 0, TEST_LOCATION); // Should not be called.

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
