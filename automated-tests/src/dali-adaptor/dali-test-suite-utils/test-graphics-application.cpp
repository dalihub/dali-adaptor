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
 *
 */

#include "test-graphics-application.h"
#include <test-graphics-sync-impl.h>

namespace Dali
{
bool TestGraphicsApplication::mLoggingEnabled = true;

TestGraphicsApplication::TestGraphicsApplication(uint32_t surfaceWidth,
                                                 uint32_t surfaceHeight,
                                                 uint32_t horizontalDpi,
                                                 uint32_t verticalDpi,
                                                 bool     initialize,
                                                 bool     enablePartialUpdate)
: mCore(NULL),
  mSurfaceWidth(surfaceWidth),
  mSurfaceHeight(surfaceHeight),
  mFrame(0u),
  mDpi{horizontalDpi, verticalDpi},
  mLastVSyncTime(0u),
  mPartialUpdateEnabled(enablePartialUpdate)
{
  if(initialize)
  {
    Initialize();
  }
}

void TestGraphicsApplication::Initialize()
{
  CreateCore();
  CreateScene();
  InitializeCore();
}

void TestGraphicsApplication::CreateCore()
{
  Dali::Integration::Log::LogFunction logFunction(&TestGraphicsApplication::LogMessage);
  Dali::Integration::Log::InstallLogFunction(logFunction);

  Dali::Integration::Trace::LogContextFunction logContextFunction(&TestGraphicsApplication::LogContext);
  Dali::Integration::Trace::InstallLogContextFunction(logContextFunction);

  // We always need the first update!
  mStatus.keepUpdating = Integration::KeepUpdating::STAGE_KEEP_RENDERING;

  mGraphics.Initialize();
  mGraphicsController.InitializeGLES(mGlAbstraction);
  mGraphicsController.Initialize(mGraphicsSyncImplementation, mGraphics);
  mGraphicsController.ActivateResourceContext();

  mCore = Dali::Integration::Core::New(mRenderController,
                                       mPlatformAbstraction,
                                       mGraphicsController,
                                       Integration::RenderToFrameBuffer::FALSE,
                                       Integration::DepthBufferAvailable::TRUE,
                                       Integration::StencilBufferAvailable::TRUE,
                                       mPartialUpdateEnabled ? Integration::PartialUpdateAvailable::TRUE : Integration::PartialUpdateAvailable::FALSE);

  mCore->ContextCreated();

  Dali::Integration::Trace::LogContext(true, "Test");
}

void TestGraphicsApplication::CreateScene()
{
  mScene = Dali::Integration::Scene::New(Size(static_cast<float>(mSurfaceWidth), static_cast<float>(mSurfaceHeight)));
  mScene.SetDpi(Vector2(static_cast<float>(mDpi.x), static_cast<float>(mDpi.y)));

  Graphics::RenderTargetCreateInfo createInfo{};
  createInfo.SetSurface({nullptr})
    .SetExtent({mSurfaceWidth, mSurfaceHeight})
    .SetPreTransform(0 | Graphics::RenderTargetTransformFlagBits::TRANSFORM_IDENTITY_BIT);
  //mRenderTarget = mGraphicsController.CreateRenderTarget(createInfo, nullptr);
  mScene.SetSurfaceRenderTarget(createInfo);
}

void TestGraphicsApplication::InitializeCore()
{
  mCore->SceneCreated();
  mCore->Initialize();
}

TestGraphicsApplication::~TestGraphicsApplication()
{
  mGraphicsController.Shutdown();
  Dali::Integration::Log::UninstallLogFunction();
  delete mCore;
}

void TestGraphicsApplication::LogContext(bool start, const char* tag, const char* message)
{
  if(start)
  {
    fprintf(stderr, "INFO: Trace Start: %s %s\n", tag, message ? message : "");
  }
  else
  {
    fprintf(stderr, "INFO: Trace End: %s %s\n", tag, message ? message : "");
  }
}

void TestGraphicsApplication::LogMessage(Dali::Integration::Log::DebugPriority level, std::string& message)
{
  if(mLoggingEnabled)
  {
    switch(level)
    {
      case Dali::Integration::Log::DEBUG:
        fprintf(stderr, "DEBUG: %s", message.c_str());
        break;
      case Dali::Integration::Log::INFO:
        fprintf(stderr, "INFO: %s", message.c_str());
        break;
      case Dali::Integration::Log::WARNING:
        fprintf(stderr, "WARN: %s", message.c_str());
        break;
      case Dali::Integration::Log::ERROR:
        fprintf(stderr, "ERROR: %s", message.c_str());
        break;
      default:
        fprintf(stderr, "DEFAULT: %s", message.c_str());
        break;
    }
  }
}

Dali::Integration::Core& TestGraphicsApplication::GetCore()
{
  return *mCore;
}

TestPlatformAbstraction& TestGraphicsApplication::GetPlatform()
{
  return mPlatformAbstraction;
}

TestRenderController& TestGraphicsApplication::GetRenderController()
{
  return mRenderController;
}

Graphics::Controller& TestGraphicsApplication::GetGraphicsController()
{
  return mGraphicsController;
}

TestGlAbstraction& TestGraphicsApplication::GetGlAbstraction()
{
  return static_cast<TestGlAbstraction&>(mGraphicsController.GetGlAbstraction());
}

void TestGraphicsApplication::ProcessEvent(const Integration::Event& event)
{
  mCore->QueueEvent(event);
  mCore->ProcessEvents();
}

void TestGraphicsApplication::SendNotification()
{
  mCore->ProcessEvents();
}

void TestGraphicsApplication::DoUpdate(uint32_t intervalMilliseconds, const char* location)
{
  if(GetUpdateStatus() == 0 &&
     mRenderStatus.NeedsUpdate() == false &&
     !GetRenderController().WasCalled(TestRenderController::RequestUpdateFunc))
  {
    fprintf(stderr, "WARNING - Update not required :%s\n", location == NULL ? "NULL" : location);
  }

  uint32_t nextVSyncTime  = mLastVSyncTime + intervalMilliseconds;
  float    elapsedSeconds = static_cast<float>(intervalMilliseconds) * 0.001f;

  mCore->Update(elapsedSeconds, mLastVSyncTime, nextVSyncTime, mStatus, false, false, false);

  GetRenderController().Initialize();

  mLastVSyncTime = nextVSyncTime;
}

bool TestGraphicsApplication::Render(uint32_t intervalMilliseconds, const char* location)
{
  DoUpdate(intervalMilliseconds, location);

  // Reset the status
  mRenderStatus.SetNeedsUpdate(false);
  mRenderStatus.SetNeedsPostRender(false);

  mCore->PreRender(mRenderStatus, false /*do not force clear*/);
  mCore->RenderScene(mRenderStatus, mScene, true /*render the off-screen buffers*/);
  mCore->RenderScene(mRenderStatus, mScene, false /*render the surface*/);
  mCore->PostRender();

  mFrame++;

  return mStatus.KeepUpdating() || mRenderStatus.NeedsUpdate();
}

bool TestGraphicsApplication::PreRenderWithPartialUpdate(uint32_t intervalMilliseconds, const char* location, std::vector<Rect<int>>& damagedRects)
{
  DoUpdate(intervalMilliseconds, location);

  mCore->PreRender(mRenderStatus, false /*do not force clear*/);
  mCore->PreRender(mScene, damagedRects);

  return mStatus.KeepUpdating() || mRenderStatus.NeedsUpdate();
}

bool TestGraphicsApplication::RenderWithPartialUpdate(std::vector<Rect<int>>& damagedRects, Rect<int>& clippingRect)
{
  mCore->RenderScene(mRenderStatus, mScene, true /*render the off-screen buffers*/, clippingRect);
  mCore->RenderScene(mRenderStatus, mScene, false /*render the surface*/, clippingRect);
  mCore->PostRender();

  mFrame++;

  return mStatus.KeepUpdating() || mRenderStatus.NeedsUpdate();
}

uint32_t TestGraphicsApplication::GetUpdateStatus()
{
  return mStatus.KeepUpdating();
}

bool TestGraphicsApplication::UpdateOnly(uint32_t intervalMilliseconds)
{
  DoUpdate(intervalMilliseconds);
  return mStatus.KeepUpdating();
}

bool TestGraphicsApplication::GetRenderNeedsUpdate()
{
  return mRenderStatus.NeedsUpdate();
}

bool TestGraphicsApplication::GetRenderNeedsPostRender()
{
  return mRenderStatus.NeedsPostRender();
}

bool TestGraphicsApplication::RenderOnly()
{
  // Update Time values
  mCore->PreRender(mRenderStatus, false /*do not force clear*/);
  mCore->RenderScene(mRenderStatus, mScene, true /*render the off-screen buffers*/);
  mCore->RenderScene(mRenderStatus, mScene, false /*render the surface*/);
  mCore->PostRender();

  mFrame++;

  return mRenderStatus.NeedsUpdate();
}

void TestGraphicsApplication::ResetContext()
{
  mCore->ContextDestroyed();
  mGraphicsController.InitializeGLES(mGlAbstraction);
  mGraphicsController.Initialize(mGraphicsSyncImplementation, mGraphics);
  mCore->ContextCreated();
}

uint32_t TestGraphicsApplication::Wait(uint32_t durationToWait)
{
  int time = 0;

  for(uint32_t i = 0; i <= (durationToWait / RENDER_FRAME_INTERVAL); i++)
  {
    SendNotification();
    Render(RENDER_FRAME_INTERVAL);
    time += RENDER_FRAME_INTERVAL;
  }
  return time;
}

} // namespace Dali
