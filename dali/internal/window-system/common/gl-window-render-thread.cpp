/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

#include <dali/devel-api/adaptor-framework/thread-settings.h>

#include <dali/internal/adaptor/common/adaptor-impl.h>
#include <dali/internal/system/common/time-service.h>
#include <dali/internal/window-system/common/gl-window-render-thread.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const unsigned int NANOSECONDS_PER_SECOND(1e+9);

// The following values will get calculated at compile time
const float    DEFAULT_FRAME_DURATION_IN_SECONDS(1.0f / 60.0f);
const uint64_t DEFAULT_FRAME_DURATION_IN_NANOSECONDS(DEFAULT_FRAME_DURATION_IN_SECONDS* NANOSECONDS_PER_SECOND);

} // namespace

GlWindowRenderThread::GlWindowRenderThread(PositionSize positionSize, ColorDepth colorDepth)
: mGraphics(nullptr),
  mWindowBase(nullptr),
  mLogFactory(Dali::Adaptor::Get().GetLogFactory()),
  mPositionSize(positionSize),
  mColorDepth(colorDepth),
  mGLInitCallback(),
  mGLRenderFrameCallback(),
  mGLTerminateCallback(),
  mEGLSurface(nullptr),
  mEGLContext(nullptr),
  mDepth(false),
  mStencil(false),
  mIsEGLInitialize(false),
  mGLESVersion(30), //Default GLES version 30
  mMSAA(0),
  mRenderThreadWaitCondition(),
  mDestroyRenderThread(0),
  mPauseRenderThread(0),
  mRenderingMode(0),
  mRequestRenderOnce(0)
{
  unsigned int refrashRate         = 1u;
  mDefaultFrameDurationNanoseconds = uint64_t(refrashRate) * DEFAULT_FRAME_DURATION_IN_NANOSECONDS;
}

GlWindowRenderThread::~GlWindowRenderThread()
{
}

void GlWindowRenderThread::SetGraphicsInterface(GraphicsInterface* graphics)
{
  mGraphics = graphics;
}

void GlWindowRenderThread::SetWindowBase(WindowBase* windowBase)
{
  mWindowBase = windowBase;
}

void GlWindowRenderThread::SetEglConfig(bool depth, bool stencil, int msaa, int version)
{
  mDepth       = depth;
  mStencil     = stencil;
  mMSAA        = msaa;
  mGLESVersion = version;
}

void GlWindowRenderThread::Pause()
{
  ConditionalWait::ScopedLock lock(mRenderThreadWaitCondition);
  mPauseRenderThread = 1;
  DALI_LOG_RELEASE_INFO("GlWindowRenderThread::Pause()\n");
}

void GlWindowRenderThread::Resume()
{
  ConditionalWait::ScopedLock lock(mRenderThreadWaitCondition);
  mPauseRenderThread = 0;
  DALI_LOG_RELEASE_INFO("GlWindowRenderThread::Resume()\n");
  mRenderThreadWaitCondition.Notify(lock);
}

void GlWindowRenderThread::Stop()
{
  ConditionalWait::ScopedLock lock(mRenderThreadWaitCondition);
  mDestroyRenderThread = 1;
  DALI_LOG_RELEASE_INFO("GlWindowRenderThread::Stop()\n");
  mRenderThreadWaitCondition.Notify(lock);
}

void GlWindowRenderThread::RegisterGlCallback(CallbackBase* initCallback,
                                              CallbackBase* renderFrameCallback,
                                              CallbackBase* terminateCallback)
{
  mGLInitCallback        = std::unique_ptr<CallbackBase>(initCallback);
  mGLRenderFrameCallback = std::unique_ptr<CallbackBase>(renderFrameCallback);
  mGLTerminateCallback   = std::unique_ptr<CallbackBase>(terminateCallback);
}

void GlWindowRenderThread::SetOnDemandRenderMode(bool onDemand)
{
  ConditionalWait::ScopedLock lock(mRenderThreadWaitCondition);
  mRenderingMode = static_cast<unsigned int>(onDemand);
  DALI_LOG_RELEASE_INFO("GlWindowRenderThread::SetOnDemandRenderMode(): mRenderingMode: %d\n", mRenderingMode);
  if(!onDemand)
  {
    mRenderThreadWaitCondition.Notify(lock);
  }
}

void GlWindowRenderThread::RenderOnce()
{
  // Most of all, this function is called in event thread
  ConditionalWait::ScopedLock lock(mRenderThreadWaitCondition);
  mRequestRenderOnce = 1;
  mRenderThreadWaitCondition.Notify(lock);
}

void GlWindowRenderThread::Run()
{
  Dali::SetThreadName("GlWindowRenderThread");
  mLogFactory.InstallLogFunction();

  int          renderFrameResult = 0;
  EglGraphics* eglGraphics       = static_cast<EglGraphics*>(mGraphics);

  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  InitializeGraphics(eglGraphics);

  eglImpl.MakeContextCurrent(mEGLSurface, mEGLContext);

  if(mGLInitCallback)
  {
    CallbackBase::Execute(*mGLInitCallback);
  }

  uint64_t timeToSleepUntil = 0;

  while(RenderReady(timeToSleepUntil))
  {
    uint64_t currentFrameStartTime = 0;
    TimeService::GetNanoseconds(currentFrameStartTime);

    if(mGLRenderFrameCallback)
    {
      renderFrameResult = CallbackBase::ExecuteReturn<int>(*mGLRenderFrameCallback);

      if(renderFrameResult)
      {
        eglImpl.SwapBuffers(mEGLSurface);
      }
    }
    renderFrameResult = 0;

    if(timeToSleepUntil == 0)
    {
      timeToSleepUntil = currentFrameStartTime + mDefaultFrameDurationNanoseconds;
    }
    else
    {
      timeToSleepUntil += mDefaultFrameDurationNanoseconds;
      uint64_t currentFrameEndTime = 0;
      TimeService::GetNanoseconds(currentFrameEndTime);
      while(currentFrameEndTime > timeToSleepUntil + mDefaultFrameDurationNanoseconds)
      {
        timeToSleepUntil += mDefaultFrameDurationNanoseconds;
      }
    }

    TimeService::SleepUntil(timeToSleepUntil);
  }

  if(mGLTerminateCallback)
  {
    CallbackBase::Execute(*mGLTerminateCallback);
  }

  if(mIsEGLInitialize)
  {
    EglGraphics*                          eglGraphics = static_cast<EglGraphics*>(mGraphics);
    Internal::Adaptor::EglImplementation& eglImpl     = eglGraphics->GetEglImplementation();

    if(mEGLSurface)
    {
      eglImpl.DestroySurface(mEGLSurface);
      mEGLSurface = nullptr;
    }

    if(mEGLContext)
    {
      eglImpl.DestroyContext(mEGLContext);
      mEGLContext = nullptr;
    }

    eglImpl.TerminateGles();
  }
}

void GlWindowRenderThread::InitializeGraphics(EglGraphics* eglGraphics)
{
  mIsEGLInitialize = true;

  Internal::Adaptor::EglImplementation& eglImpl = eglGraphics->GetEglImplementation();
  eglImpl.SetGlesVersion(mGLESVersion);

  if(eglImpl.ChooseConfig(true, mColorDepth) == false)
  {
    if(mGLESVersion == 30)
    {
      DALI_LOG_RELEASE_INFO("InitializeGraphics: Fail to choose config with GLES30, retry with GLES20\n");
      eglImpl.SetGlesVersion(20);
      mGLESVersion = 20;
      if(eglImpl.ChooseConfig(true, mColorDepth) == false)
      {
        DALI_LOG_ERROR("InitializeGraphics: Fail to choose config with GLES20");
        return;
      }
    }
    else
    {
      DALI_LOG_ERROR("InitializeGraphics: Fail to choose config with GLES20");
      return;
    }
  }
  eglImpl.CreateWindowContext(mEGLContext);

  // Create the EGL window
  EGLNativeWindowType window = mWindowBase->CreateEglWindow(mPositionSize.width, mPositionSize.height);
  mEGLSurface                = eglImpl.CreateSurfaceWindow(window, mColorDepth);
}

bool GlWindowRenderThread::RenderReady(uint64_t& timeToSleepUntil)
{
  ConditionalWait::ScopedLock updateLock(mRenderThreadWaitCondition);
  while((!mDestroyRenderThread && mRenderingMode && !mRequestRenderOnce) || mPauseRenderThread)
  {
    timeToSleepUntil = 0;
    mRenderThreadWaitCondition.Wait(updateLock);
  }

  mRequestRenderOnce = 0;
  // Keep the update-render thread alive if this thread is NOT to be destroyed
  return !mDestroyRenderThread;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
