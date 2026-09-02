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
#include <test-render-surface.h>

using namespace Dali;

namespace
{
Graphics::UniquePtr<Graphics::GLES::SyncObject> CreateTestSync(Graphics::EglGraphicsController& controller, GLsync handle)
{
  auto& gl = static_cast<TestGlAbstraction&>(controller.GetGlAbstraction());
  gl.SetFenceSyncResult(handle);
  Graphics::DefaultDeleter<Graphics::GLES::SyncObject> deleter;
  deleter.deleteFunction = [](auto* object)
  {
    object->DestroyResource();
    delete object;
  };
  Graphics::UniquePtr<Graphics::GLES::SyncObject> sync(new Graphics::GLES::SyncObject(Graphics::SyncObjectCreateInfo{}, controller), deleter);
  sync->InitializeResource();
  return sync;
}
} // namespace

void utc_dali_native_image_startup(void)
{
  test_return_value = TET_UNDEF;
}
void utc_dali_native_image_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliGraphicsNativeImageImportWithoutDraw(void)
{
  TestGraphicsApplication app;
  auto&                   controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
  auto&                   gl         = app.GetGlAbstraction();
  int                     resource   = 0;
  uint32_t                releases   = 0u;
  controller.SetGLESVersion(Graphics::GLES::GLESVersion::GLES_30);
  gl.ResetSyncCallCounts();
  gl.SetFenceSyncResult(reinterpret_cast<GLsync>(0x1));
  gl.SetClientWaitSyncResult(GL_TIMEOUT_EXPIRED);

  // The initialization import may be the image's only GPU use.
  controller.UpdateNativeImageResourceSync(&resource);
  controller.ReleaseNativeImageResource(&resource, [&]()
  { ++releases; }, nullptr, {}, {});
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 0u, TEST_LOCATION);
  DALI_TEST_CHECK(controller.NeedsNativeImageReleaseProcessing());
  gl.SetClientWaitSyncResult(GL_CONDITION_SATISFIED);
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetFenceSyncCallCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetDeleteSyncCallCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetFinishCallCount(), 0u, TEST_LOCATION);
  END_TEST;
}

int UtcDaliGraphicsNativeImageDrawReplacesImportFence(void)
{
  TestGraphicsApplication app;
  auto&                   controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
  auto&                   gl         = app.GetGlAbstraction();
  int                     resource   = 0;
  uint32_t                releases   = 0u;
  controller.SetGLESVersion(Graphics::GLES::GLESVersion::GLES_30);
  gl.ResetSyncCallCounts();
  gl.SetFenceSyncResult(reinterpret_cast<GLsync>(0x1));
  controller.UpdateNativeImageResourceSync(&resource);
  gl.SetFenceSyncResult(reinterpret_cast<GLsync>(0x2));
  controller.UpdateNativeImageResourceSync(&resource);
  DALI_TEST_EQUALS(gl.GetDeleteSyncCallCount(), 1u, TEST_LOCATION);

  gl.SetClientWaitSyncResult(reinterpret_cast<GLsync>(0x1), GL_TIMEOUT_EXPIRED);
  gl.SetClientWaitSyncResult(reinterpret_cast<GLsync>(0x2), GL_CONDITION_SATISFIED);
  controller.ReleaseNativeImageResource(&resource, [&]()
  { ++releases; }, nullptr, {}, {});
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetClientWaitSyncCallCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetDeleteSyncCallCount(), 2u, TEST_LOCATION);
  END_TEST;
}

int UtcDaliGraphicsNativeImageWaitsForEveryContext(void)
{
  TestGraphicsApplication app;
  auto&                   controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
  auto&                   gl         = app.GetGlAbstraction();
  TestRenderSurface       surface(PositionSize(0, 0, 64, 64));
  int                     resource = 0;
  uint32_t                releases = 0u;
  controller.SetGLESVersion(Graphics::GLES::GLESVersion::GLES_30);
  gl.ResetSyncCallCounts();
  gl.SetFenceSyncResult(reinterpret_cast<GLsync>(0x1));
  controller.UpdateNativeImageResourceSync(&resource);
  controller.CreateSurfaceContext(&surface);
  controller.ActivateSurfaceContext(&surface);
  gl.SetFenceSyncResult(reinterpret_cast<GLsync>(0x2));
  controller.UpdateNativeImageResourceSync(&resource);
  DALI_TEST_EQUALS(gl.GetDeleteSyncCallCount(), 0u, TEST_LOCATION);
  controller.ActivateResourceContext();

  gl.SetClientWaitSyncResult(reinterpret_cast<GLsync>(0x1), GL_CONDITION_SATISFIED);
  gl.SetClientWaitSyncResult(reinterpret_cast<GLsync>(0x2), GL_TIMEOUT_EXPIRED);
  controller.ReleaseNativeImageResource(&resource, [&]()
  { ++releases; }, nullptr, {}, {});
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetClientWaitSyncCallCount(), 2u, TEST_LOCATION);
  gl.SetClientWaitSyncResult(reinterpret_cast<GLsync>(0x2), GL_CONDITION_SATISFIED);
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetClientWaitSyncCallCount(), 3u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetDeleteSyncCallCount(), 2u, TEST_LOCATION);
  controller.DeleteSurfaceContext(&surface);
  END_TEST;
}

int UtcDaliGraphicsNativeImageControllerFallback(void)
{
  TestGraphicsApplication app;
  auto&                   controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
  auto&                   gl         = app.GetGlAbstraction();
  int                     resource   = 0;
  uint32_t                releases   = 0u;
  for(auto version : {Graphics::GLES::GLESVersion::GLES_20, Graphics::GLES::GLESVersion::GLES_30})
  {
    controller.SetGLESVersion(version);
    gl.SetFenceSyncResult(nullptr);
    gl.ResetSyncCallCounts();
    controller.UpdateNativeImageResourceSync(&resource);
    DALI_TEST_EQUALS(gl.GetFinishCallCount(), 1u, TEST_LOCATION);
    DALI_TEST_EQUALS(gl.GetFenceSyncCallCount(), version == Graphics::GLES::GLESVersion::GLES_30 ? 1u : 0u, TEST_LOCATION);
    controller.ReleaseNativeImageResource(&resource, [&]()
    { ++releases; }, nullptr, {}, {});
    controller.PostRender();
    DALI_TEST_EQUALS(gl.GetClientWaitSyncCallCount(), 0u, TEST_LOCATION);
  }
  DALI_TEST_EQUALS(releases, 2u, TEST_LOCATION);
  END_TEST;
}

int UtcDaliGraphicsNativeImageFallbackPreservesOtherContext(void)
{
  TestGraphicsApplication app;
  auto&                   controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
  auto&                   gl         = app.GetGlAbstraction();
  TestRenderSurface       surface(PositionSize(0, 0, 64, 64));
  int                     resource = 0;
  uint32_t                releases = 0u;
  controller.SetGLESVersion(Graphics::GLES::GLESVersion::GLES_30);
  gl.ResetSyncCallCounts();
  gl.SetFenceSyncResult(reinterpret_cast<GLsync>(0x1));
  controller.UpdateNativeImageResourceSync(&resource);
  controller.CreateSurfaceContext(&surface);
  controller.ActivateSurfaceContext(&surface);
  gl.SetFenceSyncResult(nullptr);
  controller.UpdateNativeImageResourceSync(&resource);
  DALI_TEST_EQUALS(gl.GetFinishCallCount(), 1u, TEST_LOCATION);
  controller.ActivateResourceContext();

  gl.SetClientWaitSyncResult(GL_TIMEOUT_EXPIRED);
  controller.ReleaseNativeImageResource(&resource, [&]()
  { ++releases; }, nullptr, {}, {});
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 0u, TEST_LOCATION);
  gl.SetClientWaitSyncResult(GL_CONDITION_SATISFIED);
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 1u, TEST_LOCATION);
  controller.DeleteSurfaceContext(&surface);
  END_TEST;
}

int UtcDaliGraphicsNativeImageLegacyDoesNotCreateReleaseFence(void)
{
  TestGraphicsApplication app;
  auto&                   gl      = app.GetGlAbstraction();
  auto                    image   = TestNativeImage::New(16, 16);
  Texture                 texture = Texture::New(*image);
  Actor                   actor   = CreateRenderableActor(texture, "", "");
  app.GetScene().Add(actor);
  gl.ResetSyncCallCounts();
  app.SendNotification();
  app.Render(16);
  app.SendNotification();
  app.Render(16);
  DALI_TEST_EQUALS(gl.GetFenceSyncCallCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetFinishCallCount(), 0u, TEST_LOCATION);
  END_TEST;
}

int UtcDaliGraphicsNativeImageUntrackedAndUnusedRelease(void)
{
  TestGraphicsApplication app;
  auto&                   controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
  auto&                   gl         = app.GetGlAbstraction();
  int                     resource   = 0;
  uint32_t                releases   = 0u;
  gl.ResetSyncCallCounts();
  controller.UpdateNativeImageResourceSync(nullptr);
  DALI_TEST_EQUALS(gl.GetFenceSyncCallCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetFinishCallCount(), 0u, TEST_LOCATION);
  controller.ReleaseNativeImageResource(&resource, [&]()
  { ++releases; }, nullptr, {}, {});
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetClientWaitSyncCallCount(), 0u, TEST_LOCATION);
  END_TEST;
}

int UtcDaliGraphicsDeferredNativeImageRelease(void)
{
  TestGraphicsApplication app;
  auto&                   controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
  auto&                   gl         = app.GetGlAbstraction();
  uint32_t                releases   = 0u;
  uint32_t                errors     = 0u;
  GLsync                  sync       = reinterpret_cast<GLsync>(0x1);

  gl.ResetSyncCallCounts();
  gl.SetClientWaitSyncResult(GL_TIMEOUT_EXPIRED);
  controller.AddDeferredNativeImageRelease(CreateTestSync(controller, sync), [&]()
  { ++releases; }, nullptr, nullptr, [&]()
  { ++errors; });
  // A pending fence is not an error, irrespective of the number of frames.
  for(uint32_t frame = 0u; frame < 1000u; ++frame)
  {
    controller.PostRender();
  }
  DALI_TEST_EQUALS(releases, 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(errors, 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetClientWaitSyncCallCount(), 1000u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetDeleteSyncCallCount(), 0u, TEST_LOCATION);
  DALI_TEST_CHECK(controller.HasPendingNativeImageReleases());

  gl.SetClientWaitSyncResult(GL_CONDITION_SATISFIED);
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 1u, TEST_LOCATION);
  DALI_TEST_CHECK(!controller.HasPendingNativeImageReleases());
  DALI_TEST_EQUALS(gl.GetDeleteSyncCallCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetFinishCallCount(), 0u, TEST_LOCATION);
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 1u, TEST_LOCATION);
  END_TEST;
}

int UtcDaliGraphicsNativeImageReleaseErrorNotification(void)
{
  TestGraphicsApplication app;
  auto&                   controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
  auto&                   gl         = app.GetGlAbstraction();
  uint32_t                errors     = 0u;
  uint32_t                releases   = 0u;
  GLsync                  sync       = reinterpret_cast<GLsync>(0x1);

  gl.ResetSyncCallCounts();
  gl.SetClientWaitSyncResult(GL_WAIT_FAILED);
  controller.AddDeferredNativeImageRelease(CreateTestSync(controller, sync), [&]()
  { ++releases; }, nullptr, nullptr, [&]()
  { ++errors; });
  controller.PostRender();
  DALI_TEST_EQUALS(errors, 1u, TEST_LOCATION);
  // A failed wait is quarantined: later frames neither retry nor return it.
  gl.SetClientWaitSyncResult(GL_CONDITION_SATISFIED);
  controller.PostRender();
  controller.PostRender();
  DALI_TEST_EQUALS(errors, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(releases, 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetClientWaitSyncCallCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetFinishCallCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetDeleteSyncCallCount(), 0u, TEST_LOCATION);
  controller.DiscardNativeImageReleases();
  DALI_TEST_EQUALS(releases, 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(errors, 1u, TEST_LOCATION);
  END_TEST;
}

int UtcDaliGraphicsNativeImageDiscardPendingReleases(void)
{
  TestGraphicsApplication app;
  auto&                   controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
  auto&                   gl         = app.GetGlAbstraction();
  uint32_t                releases   = 0u;
  uint32_t                errors     = 0u;
  uint32_t                discards   = 0u;
  GLsync                  sync       = reinterpret_cast<GLsync>(0x1);
  auto                    resource   = std::make_shared<uint32_t>(0u);
  std::weak_ptr<uint32_t> lifetime   = resource;
  controller.AddDeferredNativeImageRelease(CreateTestSync(controller, sync), [&, resource]()
  { ++releases; }, nullptr, nullptr,
                                           [&]()
  { ++errors; }, [&, resource]()
  { ++discards; });
  resource.reset();
  gl.ResetSyncCallCounts();
  gl.SetClientWaitSyncResult(GL_TIMEOUT_EXPIRED);
  controller.PostRender();
  DALI_TEST_CHECK(!lifetime.expired());
  // Shutdown discards ownership, without treating this as GPU completion.
  controller.DiscardNativeImageReleases();
  DALI_TEST_CHECK(lifetime.expired());
  DALI_TEST_CHECK(!controller.HasPendingNativeImageReleases());
  DALI_TEST_EQUALS(releases, 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(errors, 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(discards, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetFinishCallCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetDeleteSyncCallCount(), 1u, TEST_LOCATION);
  controller.DiscardNativeImageReleases();
  controller.PostRender();
  DALI_TEST_EQUALS(discards, 1u, TEST_LOCATION);
  END_TEST;
}

int UtcDaliGraphicsNativeImageDeferredResourceLifetime(void)
{
  TestGraphicsApplication app;
  auto&                   controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
  auto&                   gl         = app.GetGlAbstraction();
  uint32_t                releases   = 0u;
  GLsync                  sync       = reinterpret_cast<GLsync>(0x1);

  for(uint32_t cycle = 0u; cycle < 1000u; ++cycle)
  {
    auto                    resource = std::make_shared<uint32_t>(cycle);
    std::weak_ptr<uint32_t> lifetime = resource;
    controller.AddDeferredNativeImageRelease(CreateTestSync(controller, sync), [resource, &releases]()
    { ++releases; }, nullptr, nullptr);
    resource.reset();
    gl.SetClientWaitSyncResult(GL_TIMEOUT_EXPIRED);
    controller.PostRender();
    DALI_TEST_CHECK(!lifetime.expired());
    DALI_TEST_EQUALS(releases, cycle, TEST_LOCATION);
    gl.SetClientWaitSyncResult(GL_CONDITION_SATISFIED);
    controller.PostRender();
    DALI_TEST_CHECK(lifetime.expired());
    DALI_TEST_CHECK(!controller.HasPendingNativeImageReleases());
    DALI_TEST_EQUALS(releases, cycle + 1u, TEST_LOCATION);
  }
  END_TEST;
}

int UtcDaliGraphicsNativeImageReleaseCallbackEnqueuesRelease(void)
{
  TestGraphicsApplication app;
  auto&                   controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
  auto&                   gl         = app.GetGlAbstraction();
  uint32_t                releases   = 0u;
  gl.SetClientWaitSyncResult(GL_CONDITION_SATISFIED);
  controller.AddDeferredNativeImageRelease(nullptr, [&]()
  {
    ++releases;
    controller.AddDeferredNativeImageRelease(nullptr, [&]()
    { ++releases; }, nullptr, nullptr);
  }, nullptr, nullptr);
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 1u, TEST_LOCATION);
  DALI_TEST_CHECK(controller.HasPendingNativeImageReleases());
  DALI_TEST_CHECK(controller.NeedsNativeImageReleaseProcessing());
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 2u, TEST_LOCATION);
  DALI_TEST_CHECK(!controller.HasPendingNativeImageReleases());
  DALI_TEST_CHECK(!controller.NeedsNativeImageReleaseProcessing());
  END_TEST;
}

int UtcDaliGraphicsNativeImageReleaseKeepsFramesUntilComplete(void)
{
  TestGraphicsApplication app;
  auto&                   controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
  auto&                   gl         = app.GetGlAbstraction();
  uint32_t                releases   = 0u;

  // Holding a native image is not itself a reason to keep rendering.
  auto displayedImage = TestNativeImage::New(16, 16);
  DALI_TEST_CHECK(!controller.NeedsNativeImageReleaseProcessing());

  // Two retired buffers plus the displayed image model a full three-buffer
  // producer pool. No new provider frame is needed to finish the releases.
  for(uintptr_t id = 1u; id <= 2u; ++id)
  {
    controller.AddDeferredNativeImageRelease(CreateTestSync(controller, reinterpret_cast<GLsync>(id)), [&]()
    { ++releases; }, nullptr, nullptr);
  }
  gl.SetClientWaitSyncResult(GL_TIMEOUT_EXPIRED);
  DALI_TEST_CHECK(controller.NeedsNativeImageReleaseProcessing());
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 0u, TEST_LOCATION);
  DALI_TEST_CHECK(controller.NeedsNativeImageReleaseProcessing());

  gl.SetClientWaitSyncResult(GL_CONDITION_SATISFIED);
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 2u, TEST_LOCATION);
  DALI_TEST_CHECK(!controller.NeedsNativeImageReleaseProcessing());
  DALI_TEST_CHECK(!controller.HasPendingNativeImageReleases());
  DALI_TEST_EQUALS(gl.GetFinishCallCount(), 0u, TEST_LOCATION);
  END_TEST;
}

int UtcDaliGraphicsNativeImageFailedReleaseDoesNotKeepFrames(void)
{
  TestGraphicsApplication app;
  auto&                   controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
  auto&                   gl         = app.GetGlAbstraction();
  uint32_t                releases   = 0u;
  uint32_t                errors     = 0u;
  auto                    onRelease  = [&]()
  { ++releases; };
  auto onError = [&]()
  { ++errors; };

  gl.SetClientWaitSyncResult(GL_WAIT_FAILED);
  controller.AddDeferredNativeImageRelease(CreateTestSync(controller, reinterpret_cast<GLsync>(0x1)), onRelease, nullptr, nullptr, onError);
  controller.PostRender();
  DALI_TEST_EQUALS(errors, 1u, TEST_LOCATION);
  DALI_TEST_CHECK(controller.HasPendingNativeImageReleases());
  DALI_TEST_CHECK(!controller.NeedsNativeImageReleaseProcessing());

  // A failed record must not hide a newer release that can still progress.
  gl.SetClientWaitSyncResult(GL_TIMEOUT_EXPIRED);
  controller.AddDeferredNativeImageRelease(CreateTestSync(controller, reinterpret_cast<GLsync>(0x2)), onRelease, nullptr, nullptr, onError);
  controller.PostRender();
  DALI_TEST_CHECK(controller.NeedsNativeImageReleaseProcessing());
  gl.SetClientWaitSyncResult(GL_CONDITION_SATISFIED);
  controller.PostRender();
  DALI_TEST_EQUALS(releases, 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(errors, 1u, TEST_LOCATION);
  DALI_TEST_CHECK(controller.HasPendingNativeImageReleases());
  DALI_TEST_CHECK(!controller.NeedsNativeImageReleaseProcessing());

  controller.DiscardNativeImageReleases();
  DALI_TEST_CHECK(!controller.HasPendingNativeImageReleases());
  DALI_TEST_CHECK(!controller.NeedsNativeImageReleaseProcessing());
  END_TEST;
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
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 1, TEST_LOCATION);

    callStack.Reset();
    textureCallStack.Reset();

    imageInterface->mPrepareTextureResult = Dali::NativeImageInterface::PrepareTextureResult::IMAGE_CHANGED;

    app.SendNotification();
    app.Render(16);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, 1, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, 0, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 1, TEST_LOCATION);

    imageInterface->mPrepareTextureResult = Dali::NativeImageInterface::PrepareTextureResult::NO_ERROR;

    callStack.Reset();
    textureCallStack.Reset();

    app.SendNotification();
    app.Render(16);

    DALI_TEST_EQUALS(imageInterface->mExtensionCreateCalls, 1, TEST_LOCATION);
    DALI_TEST_EQUALS(imageInterface->mExtensionDestroyCalls, 0, TEST_LOCATION);

    DALI_TEST_EQUALS(callStack.CountMethod("PrepareTexture"), 1, TEST_LOCATION);
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 1, TEST_LOCATION);

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
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 1, TEST_LOCATION);

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
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 1, TEST_LOCATION);

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
    DALI_TEST_EQUALS(callStack.CountMethod("TargetTexture"), 1, TEST_LOCATION);

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

int UtcDaliNativeImageUnattachedTextureLifetime(void)
{
  TestGraphicsApplication app;
  for(bool renderBeforeRelease : {false, true})
  {
    TestNativeImagePointer image = TestNativeImage::New(16, 16);
    {
      // A texture registered through ImageUrl may lose its last reference if
      // visual creation fails, including before the first render.
      Texture texture = Texture::New(*image);
      if(renderBeforeRelease)
      {
        app.SendNotification();
        app.Render(16);
        DALI_TEST_EQUALS(image->mExtensionCreateCalls, 1, TEST_LOCATION);
        DALI_TEST_EQUALS(image->mExtensionDestroyCalls, 0, TEST_LOCATION);
      }
    }
    app.SendNotification();
    app.Render(16);
    app.SendNotification();
    app.Render(16);
    Graphics::SubmitInfo submitInfo{{}, 0 | Graphics::SubmitFlagBits::FLUSH};
    app.GetGraphicsController().SubmitCommandBuffers(submitInfo);
    DALI_TEST_EQUALS(image->mExtensionDestroyCalls, 1, TEST_LOCATION);
  }
  END_TEST;
}

int UtcDaliGraphicsNativeImageSyncFailureStatus(void)
{
  TestGraphicsApplication    app;
  auto&                      controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
  auto&                      gl         = app.GetGlAbstraction();
  Graphics::GLES::SyncObject sync(Graphics::SyncObjectCreateInfo{}, controller);
  gl.SetFenceSyncResult(nullptr);
  DALI_TEST_CHECK(!sync.InitializeResource());
  DALI_TEST_CHECK(sync.Poll() == Graphics::GLES::SyncObject::PollResult::FAILED);
  gl.SetFenceSyncResult(reinterpret_cast<GLsync>(0x1));
  DALI_TEST_CHECK(sync.InitializeResource());
  gl.SetClientWaitSyncResult(GL_TIMEOUT_EXPIRED);
  DALI_TEST_CHECK(sync.Poll() == Graphics::GLES::SyncObject::PollResult::PENDING);
  gl.SetClientWaitSyncResult(GL_CONDITION_SATISFIED);
  DALI_TEST_CHECK(sync.Poll() == Graphics::GLES::SyncObject::PollResult::SIGNALED);
  gl.SetClientWaitSyncResult(GL_WAIT_FAILED);
  DALI_TEST_CHECK(sync.Poll() == Graphics::GLES::SyncObject::PollResult::FAILED);
  sync.DestroyResource();
  END_TEST;
}

int UtcDaliGraphicsNativeImageSyncBeforeControllerInitialization(void)
{
  TestGlAbstraction               gl;
  Graphics::EglGraphicsController controller;
  controller.InitializeGLES(gl);
  Graphics::GLES::SyncObject sync(Graphics::SyncObjectCreateInfo{}, controller);
  gl.SetFenceSyncResult(reinterpret_cast<GLsync>(0x1));
  gl.SetClientWaitSyncResult(GL_CONDITION_SATISFIED);
  gl.ResetSyncCallCounts();

  DALI_TEST_CHECK(Graphics::EglGraphicsController::IsShuttingDown());
  DALI_TEST_CHECK(!sync.InitializeResource());
  DALI_TEST_CHECK(sync.Poll() == Graphics::GLES::SyncObject::PollResult::FAILED);
  DALI_TEST_CHECK(!sync.IsSynced());
  sync.DestroyResource();
  DALI_TEST_EQUALS(gl.GetFenceSyncCallCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetClientWaitSyncCallCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.GetDeleteSyncCallCount(), 0u, TEST_LOCATION);
  END_TEST;
}

int UtcDaliGraphicsNativeImageSyncOutlivesController(void)
{
  std::unique_ptr<Graphics::GLES::SyncObject> sync;
  {
    TestGraphicsApplication app;
    auto&                   controller = static_cast<Graphics::EglGraphicsController&>(app.GetGraphicsController());
    auto&                   gl         = app.GetGlAbstraction();
    gl.SetFenceSyncResult(reinterpret_cast<GLsync>(0x1));
    gl.SetClientWaitSyncResult(GL_CONDITION_SATISFIED);
    sync = std::make_unique<Graphics::GLES::SyncObject>(Graphics::SyncObjectCreateInfo{}, controller);
    DALI_TEST_CHECK(sync->InitializeResource());
  }

  // The controller reference is dangling; only the static shutdown check is safe.
  DALI_TEST_CHECK(Graphics::EglGraphicsController::IsShuttingDown());
  DALI_TEST_CHECK(!sync->InitializeResource());
  DALI_TEST_CHECK(sync->Poll() == Graphics::GLES::SyncObject::PollResult::FAILED);
  DALI_TEST_CHECK(sync->Poll(true) == Graphics::GLES::SyncObject::PollResult::FAILED);
  DALI_TEST_CHECK(!sync->IsSynced());
  sync->DestroyResource();
  END_TEST;
}
