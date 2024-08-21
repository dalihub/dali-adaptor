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
#include <test-graphics-egl-application.h>
#include <test-graphics-framebuffer.h>

using namespace Dali;

namespace
{
RenderTask CreateRenderTask(TestGraphicsApplication& application,
                            FrameBuffer              framebuffer)
{
  Actor rootActor = Actor::New();
  application.GetScene().Add(rootActor);
  Texture img         = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, 1, 1);
  Actor   sourceActor = CreateRenderableActor(img);
  application.GetScene().Add(sourceActor);

  CameraActor offscreenCameraActor = CameraActor::New(Size(TestApplication::DEFAULT_SURFACE_WIDTH,
                                                           TestApplication::DEFAULT_SURFACE_HEIGHT));
  application.GetScene().Add(offscreenCameraActor);

  // Change main render task to use a different root
  RenderTaskList taskList = application.GetScene().GetRenderTaskList();
  taskList.GetTask(0u).SetSourceActor(rootActor);

  RenderTask newTask = taskList.CreateTask();
  newTask.SetCameraActor(offscreenCameraActor);
  newTask.SetSourceActor(sourceActor);
  newTask.SetInputEnabled(false);
  newTask.SetClearColor(Vector4(0.f, 0.f, 0.f, 0.f));
  newTask.SetClearEnabled(true);
  newTask.SetExclusive(true);
  newTask.SetFrameBuffer(framebuffer);

  return newTask;
}
} // namespace

void utc_dali_graphics_framebuffer_startup(void)
{
  test_return_value = TET_UNDEF;
}
void utc_dali_graphics_framebuffer_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliGraphicsFramebufferAttachDepth(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliGraphicsFramebufferAttachDepth - Test for GLES specific behavior");

  auto& gl = app.GetGlAbstraction();

  uint32_t width  = 16u;
  uint32_t height = 24u;

  FrameBuffer framebuffer = FrameBuffer::New(width, height, FrameBuffer::Attachment::DEPTH);

  DALI_TEST_CHECK(framebuffer);

  Texture dummyColorTexture = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height);
  Actor   dummyActor        = CreateRenderableActor(dummyColorTexture);
  framebuffer.AttachColorTexture(dummyColorTexture);

  app.GetScene().Add(dummyActor);

  auto renderTask = CreateRenderTask(app, framebuffer);

  DALI_TEST_CHECK(renderTask);

  app.SendNotification();
  app.Render(16); // The above actor will get rendered and drawn once.

  DALI_TEST_EQUALS(gl.CheckFramebufferColorAttachmentCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthAttachmentCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferStencilAttachmentCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthStencilAttachmentCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthAttachment(), (GLenum)GL_TRUE, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferStencilAttachment(), (GLenum)GL_FALSE, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthStencilAttachment(), (GLenum)GL_FALSE, TEST_LOCATION); // Check whether renderbuffer attached by DEPTH_STENCIL.

  END_TEST;
}

int UtcDaliGraphicsFramebufferAttachStencil(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliGraphicsFramebufferAttachStencil - Test for GLES specific behavior");

  auto& gl = app.GetGlAbstraction();

  uint32_t width  = 16u;
  uint32_t height = 24u;

  FrameBuffer framebuffer = FrameBuffer::New(width, height, FrameBuffer::Attachment::STENCIL);

  DALI_TEST_CHECK(framebuffer);

  Texture dummyColorTexture = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height);
  Actor   dummyActor        = CreateRenderableActor(dummyColorTexture);
  framebuffer.AttachColorTexture(dummyColorTexture);

  app.GetScene().Add(dummyActor);

  auto renderTask = CreateRenderTask(app, framebuffer);

  DALI_TEST_CHECK(renderTask);

  app.SendNotification();
  app.Render(16); // The above actor will get rendered and drawn once.

  DALI_TEST_EQUALS(gl.CheckFramebufferColorAttachmentCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthAttachmentCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferStencilAttachmentCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthStencilAttachmentCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthAttachment(), (GLenum)GL_FALSE, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferStencilAttachment(), (GLenum)GL_TRUE, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthStencilAttachment(), (GLenum)GL_FALSE, TEST_LOCATION); // Check whether renderbuffer attached by DEPTH_STENCIL.

  END_TEST;
}

int UtcDaliGraphicsFramebufferAttachDepthStencil(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliGraphicsFramebufferAttachDepthStencil - Test for GLES specific behavior");

  auto& gl = app.GetGlAbstraction();

  uint32_t width  = 16u;
  uint32_t height = 24u;

  FrameBuffer framebuffer = FrameBuffer::New(width, height, FrameBuffer::Attachment::DEPTH_STENCIL);

  DALI_TEST_CHECK(framebuffer);

  Texture dummyColorTexture = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height);
  Actor   dummyActor        = CreateRenderableActor(dummyColorTexture);
  framebuffer.AttachColorTexture(dummyColorTexture);

  app.GetScene().Add(dummyActor);

  auto renderTask = CreateRenderTask(app, framebuffer);

  DALI_TEST_CHECK(renderTask);

  app.SendNotification();
  app.Render(16); // The above actor will get rendered and drawn once.

  DALI_TEST_EQUALS(gl.CheckFramebufferColorAttachmentCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthAttachmentCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferStencilAttachmentCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthStencilAttachmentCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthAttachment(), (GLenum)GL_TRUE, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferStencilAttachment(), (GLenum)GL_TRUE, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthStencilAttachment(), (GLenum)GL_TRUE, TEST_LOCATION); // Check whether renderbuffer attached by DEPTH_STENCIL.

  END_TEST;
}

int UtcDaliGraphicsFramebufferAttachDepthTexture(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliGraphicsFramebufferAttachDepthTexture - Test for GLES specific behavior");

  auto& gl = app.GetGlAbstraction();

  uint32_t width  = 16u;
  uint32_t height = 24u;

  FrameBuffer framebuffer = FrameBuffer::New(width, height, FrameBuffer::Attachment::NONE);

  DALI_TEST_CHECK(framebuffer);

  Texture dummyColorTexture = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height);
  Texture dummyDepthTexture = CreateTexture(TextureType::TEXTURE_2D, Pixel::DEPTH_UNSIGNED_INT, width, height);
  Actor   dummyActor        = CreateRenderableActor(dummyColorTexture);
  framebuffer.AttachColorTexture(dummyColorTexture);
  DevelFrameBuffer::AttachDepthTexture(framebuffer, dummyDepthTexture);

  app.GetScene().Add(dummyActor);

  auto renderTask = CreateRenderTask(app, framebuffer);

  DALI_TEST_CHECK(renderTask);

  app.SendNotification();
  app.Render(16); // The above actor will get rendered and drawn once.

  DALI_TEST_EQUALS(gl.CheckFramebufferColorAttachmentCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthAttachmentCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferStencilAttachmentCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthStencilAttachmentCount(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthAttachment(), (GLenum)GL_FALSE, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferStencilAttachment(), (GLenum)GL_FALSE, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthStencilAttachment(), (GLenum)GL_FALSE, TEST_LOCATION); // Check whether renderbuffer attached by DEPTH_STENCIL.

  END_TEST;
}

int UtcDaliGraphicsFramebufferAttachDepthStencilTexture(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliGraphicsFramebufferAttachDepthStencilTexture - Test for GLES specific behavior");

  auto& gl = app.GetGlAbstraction();

  {
    uint32_t width  = 16u;
    uint32_t height = 24u;

    FrameBuffer framebuffer = FrameBuffer::New(width, height, FrameBuffer::Attachment::STENCIL);

    DALI_TEST_CHECK(framebuffer);

    Texture dummyColorTexture        = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height);
    Texture dummyDepthStencilTexture = CreateTexture(TextureType::TEXTURE_2D, Pixel::DEPTH_STENCIL, width, height);
    Actor   dummyActor               = CreateRenderableActor(dummyColorTexture);
    framebuffer.AttachColorTexture(dummyColorTexture);
    DevelFrameBuffer::AttachDepthStencilTexture(framebuffer, dummyDepthStencilTexture);

    app.GetScene().Add(dummyActor);

    auto renderTask = CreateRenderTask(app, framebuffer);

    DALI_TEST_CHECK(renderTask);

    app.SendNotification();
    app.Render(16); // The above actor will get rendered and drawn once.

    DALI_TEST_EQUALS(gl.CheckFramebufferColorAttachmentCount(), 1u, TEST_LOCATION);
    DALI_TEST_EQUALS(gl.CheckFramebufferDepthAttachmentCount(), 1u, TEST_LOCATION);
    DALI_TEST_EQUALS(gl.CheckFramebufferStencilAttachmentCount(), 1u, TEST_LOCATION);
    DALI_TEST_EQUALS(gl.CheckFramebufferDepthStencilAttachmentCount(), 1u, TEST_LOCATION);
    DALI_TEST_EQUALS(gl.CheckFramebufferDepthAttachment(), (GLenum)GL_FALSE, TEST_LOCATION);
    DALI_TEST_EQUALS(gl.CheckFramebufferStencilAttachment(), (GLenum)GL_FALSE, TEST_LOCATION);
    DALI_TEST_EQUALS(gl.CheckFramebufferDepthStencilAttachment(), (GLenum)GL_FALSE, TEST_LOCATION); // Check whether renderbuffer attached by DEPTH_STENCIL.

    UnparentAndReset(dummyActor);
  }
  // Ensure some cleanup happens!
  app.SendNotification();
  app.Render(16);
  app.SendNotification();
  app.Render(16);

  END_TEST;
}

int UtcDaliGraphicsFramebufferAttachStencilAndDepthTexture(void)
{
  TestGraphicsApplication app;
  tet_infoline("UtcDaliGraphicsFramebufferAttachStencilAndDepthTexture - Test for GLES specific behavior");

  auto& gl = app.GetGlAbstraction();

  uint32_t width  = 16u;
  uint32_t height = 24u;

  FrameBuffer framebuffer = FrameBuffer::New(width, height, FrameBuffer::Attachment::STENCIL);

  DALI_TEST_CHECK(framebuffer);

  Texture dummyColorTexture = CreateTexture(TextureType::TEXTURE_2D, Pixel::RGBA8888, width, height);
  // Note : Current GLES cannot seperate destination of depth result and stencil result. We need to make the texture as DEPTH_STENCIL
  Texture dummyDepthTexture = CreateTexture(TextureType::TEXTURE_2D, Pixel::DEPTH_STENCIL, width, height);
  Actor   dummyActor        = CreateRenderableActor(dummyColorTexture);
  framebuffer.AttachColorTexture(dummyColorTexture);
  DevelFrameBuffer::AttachDepthTexture(framebuffer, dummyDepthTexture);

  app.GetScene().Add(dummyActor);

  auto renderTask = CreateRenderTask(app, framebuffer);

  DALI_TEST_CHECK(renderTask);

  app.SendNotification();
  app.Render(16); // The above actor will get rendered and drawn once.

  DALI_TEST_EQUALS(gl.CheckFramebufferColorAttachmentCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthAttachmentCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferStencilAttachmentCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthStencilAttachmentCount(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthAttachment(), (GLenum)GL_FALSE, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferStencilAttachment(), (GLenum)GL_FALSE, TEST_LOCATION);
  DALI_TEST_EQUALS(gl.CheckFramebufferDepthStencilAttachment(), (GLenum)GL_FALSE, TEST_LOCATION); // Check whether renderbuffer attached by DEPTH_STENCIL.

  END_TEST;
}
