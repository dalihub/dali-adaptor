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

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/thread-settings.h>

// INTERNAL INCLUDES
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
constexpr unsigned int NANOSECONDS_PER_SECOND(1e+9);

// The following values will get calculated at compile time
constexpr float    DEFAULT_FRAME_DURATION_IN_SECONDS(1.0f / 60.0f);
constexpr uint64_t DEFAULT_FRAME_DURATION_IN_NANOSECONDS(DEFAULT_FRAME_DURATION_IN_SECONDS* NANOSECONDS_PER_SECOND);
constexpr uint64_t REFRESH_RATE(1u);

constexpr int MINIMUM_DIMENSION_CHANGE(1);
} // namespace

GlWindowRenderThread::GlWindowRenderThread(PositionSize positionSize, ColorDepth colorDepth)
: mGraphics(nullptr),
  mWindowBase(nullptr),
  mWindowRotationTrigger(),
  mLogFactory(Dali::Adaptor::Get().GetLogFactory()),
  mTraceFactory(Dali::Adaptor::Get().GetTraceFactory()),
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
  mWindowRotationAngle(0),
  mScreenRotationAngle(0),
  mRenderThreadWaitCondition(),
  mDestroyRenderThread(0),
  mPauseRenderThread(0),
  mRenderingMode(0),
  mRequestRenderOnce(0),
  mSurfaceStatus(0),
  mPostRendering(0),
  mDefaultFrameDurationNanoseconds(REFRESH_RATE * DEFAULT_FRAME_DURATION_IN_NANOSECONDS)
{
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

void GlWindowRenderThread::SetGraphicsConfig(bool depth, bool stencil, int msaa, int version)
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

void GlWindowRenderThread::RegisterGlCallbacks(CallbackBase* initCallback,
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

void GlWindowRenderThread::RequestWindowResize(int width, int height)
{
  ConditionalWait::ScopedLock lock(mRenderThreadWaitCondition);
  // Check resizing
  if((fabs(width - mPositionSize.width) > MINIMUM_DIMENSION_CHANGE) ||
     (fabs(height - mPositionSize.height) > MINIMUM_DIMENSION_CHANGE))
  {
    mSurfaceStatus |= static_cast<unsigned int>(SurfaceStatus::RESIZED); // Set bit for window resized
    mPositionSize.width  = width;
    mPositionSize.height = height;

    DALI_LOG_RELEASE_INFO("GlWindowRenderThread::RequestWindowResize(), width:%d, height:%d\n", width, height);
    mRenderThreadWaitCondition.Notify(lock);
  }
}

void GlWindowRenderThread::RequestWindowRotate(int windowAngle)
{
  if(!mWindowRotationTrigger)
  {
    mWindowRotationTrigger = std::unique_ptr<TriggerEventInterface>(TriggerEventFactory::CreateTriggerEvent(MakeCallback(this, &GlWindowRenderThread::WindowRotationCompleted),
                                                                                                            TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER));
  }

  ConditionalWait::ScopedLock lock(mRenderThreadWaitCondition);
  if(mWindowRotationAngle != windowAngle)
  {
    mSurfaceStatus |= static_cast<unsigned int>(SurfaceStatus::WINDOW_ROTATED); // Set bit for window rotation
    mWindowRotationAngle = windowAngle;
    DALI_LOG_RELEASE_INFO("GlWindowRenderThread::RequestWindowRotate(): %d\n", windowAngle);
    mRenderThreadWaitCondition.Notify(lock);
  }
}

void GlWindowRenderThread::RequestScreenRotate(int screenAngle)
{
  ConditionalWait::ScopedLock lock(mRenderThreadWaitCondition);
  if(mScreenRotationAngle != screenAngle)
  {
    mSurfaceStatus |= static_cast<unsigned int>(SurfaceStatus::SCREEN_ROTATED); // Set bit for screen rotation
    mScreenRotationAngle = screenAngle;
    DALI_LOG_RELEASE_INFO("GlWindowRenderThread::RequestScreenRotate(): %d\n", screenAngle);
    mRenderThreadWaitCondition.Notify(lock);
  }
}

void GlWindowRenderThread::WindowRotationCompleted()
{
  mWindowBase->WindowRotationCompleted(mWindowRotationAngle, mPositionSize.width, mPositionSize.height);

  PostRenderFinish();
}

unsigned int GlWindowRenderThread::GetSurfaceStatus(int& windowRotationAngle, int& screenRotationAngle)
{
  ConditionalWait::ScopedLock lock(mRenderThreadWaitCondition);

  // Get the surface status and reset that.
  unsigned int status = mSurfaceStatus;
  mSurfaceStatus      = static_cast<unsigned int>(SurfaceStatus::NO_CHANGED);

  windowRotationAngle = mWindowRotationAngle;
  screenRotationAngle = mScreenRotationAngle;

  return status;
}

void GlWindowRenderThread::Run()
{
  Dali::SetThreadName("GlWindowRenderThread");
  mLogFactory.InstallLogFunction();
  mTraceFactory.InstallTraceFunction();

  int          renderFrameResult = 0;
  unsigned int isSurfaceChanged  = 0;
  bool         isWindowResized = false, isWindowRotated = false, isScreenRotated = false;
  int          windowRotationAngle = 0, screenRotationAngle = 0, totalAngle = 0;
  EglGraphics* eglGraphics = static_cast<EglGraphics*>(mGraphics);

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
      // PreRender
      isSurfaceChanged = GetSurfaceStatus(windowRotationAngle, screenRotationAngle);
      if(DALI_UNLIKELY(isSurfaceChanged))
      {
        isWindowResized = (isSurfaceChanged & static_cast<unsigned int>(SurfaceStatus::RESIZED)) ? true : false;
        isWindowRotated = (isSurfaceChanged & static_cast<unsigned int>(SurfaceStatus::WINDOW_ROTATED)) ? true : false;
        isScreenRotated = (isSurfaceChanged & static_cast<unsigned int>(SurfaceStatus::SCREEN_ROTATED)) ? true : false;
        totalAngle      = (windowRotationAngle + screenRotationAngle) % 360;

        if(isWindowRotated || isScreenRotated)
        {
          mWindowBase->SetWindowBufferTransform(totalAngle);
          if(isWindowRotated)
          {
            mWindowBase->SetWindowTransform(windowRotationAngle);
          }
        }

        if(isWindowResized)
        {
          Dali::PositionSize positionSize;
          positionSize.x = mPositionSize.x;
          positionSize.y = mPositionSize.y;
          if(totalAngle == 0 || totalAngle == 180)
          {
            positionSize.width  = mPositionSize.width;
            positionSize.height = mPositionSize.height;
          }
          else
          {
            positionSize.width  = mPositionSize.height;
            positionSize.height = mPositionSize.width;
          }
          mWindowBase->ResizeWindow(positionSize);
        }
      }

      // Render
      renderFrameResult = CallbackBase::ExecuteReturn<int>(*mGLRenderFrameCallback);

      // PostRender
      if(DALI_UNLIKELY(isWindowRotated))
      {
        PostRenderStart();

        mWindowRotationTrigger->Trigger();

        PostRenderWaitForFinished();
        isWindowRotated = false;
      }

      // buffer commit
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
  Dali::Any window = mWindowBase->CreateWindow(mPositionSize.width, mPositionSize.height);
  mEGLSurface      = eglImpl.CreateSurfaceWindow(window.Get<EGLNativeWindowType>(), mColorDepth);
}

bool GlWindowRenderThread::RenderReady(uint64_t& timeToSleepUntil)
{
  ConditionalWait::ScopedLock updateLock(mRenderThreadWaitCondition);
  while((!mDestroyRenderThread && mRenderingMode && !mRequestRenderOnce && !mSurfaceStatus) || mPauseRenderThread)
  {
    timeToSleepUntil = 0;
    mRenderThreadWaitCondition.Wait(updateLock);
  }

  mRequestRenderOnce = 0;
  // Keep the update-render thread alive if this thread is NOT to be destroyed
  return !mDestroyRenderThread;
}

void GlWindowRenderThread::PostRenderStart()
{
  ConditionalWait::ScopedLock lock(mRenderThreadWaitCondition);
  mPostRendering = false;
}

void GlWindowRenderThread::PostRenderFinish()
{
  ConditionalWait::ScopedLock lock(mRenderThreadWaitCondition);
  mPostRendering = true;
  mRenderThreadWaitCondition.Notify(lock);
}

void GlWindowRenderThread::PostRenderWaitForFinished()
{
  ConditionalWait::ScopedLock lock(mRenderThreadWaitCondition);
  while(!mPostRendering && !mDestroyRenderThread)
  {
    mRenderThreadWaitCondition.Wait(lock);
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
