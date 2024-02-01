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

// CLASS HEADER
#include <dali/internal/adaptor/common/combined-update-render-controller.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/platform-abstraction.h>
#include <errno.h>
#include <unistd.h>
#include "dali/public-api/common/dali-common.h"

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/shader-precompiler.h>
#include <dali/integration-api/adaptor-framework/trigger-event-factory.h>
#include <dali/internal/adaptor/common/adaptor-internal-services.h>
#include <dali/internal/adaptor/common/combined-update-render-controller-debug.h>
#include <dali/internal/graphics/common/graphics-interface.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/system/common/environment-options.h>
#include <dali/internal/system/common/texture-upload-manager-impl.h>
#include <dali/internal/system/common/time-service.h>
#include <dali/internal/thread/common/thread-settings-impl.h>
#include <dali/internal/window-system/common/window-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const unsigned int CREATED_THREAD_COUNT = 1u;

const int CONTINUOUS = -1;
const int ONCE       = 1;

const unsigned int TRUE  = 1u;
const unsigned int FALSE = 0u;

const unsigned int MILLISECONDS_PER_SECOND(1e+3);
const float        NANOSECONDS_TO_SECOND(1e-9f);
const unsigned int NANOSECONDS_PER_SECOND(1e+9);
const unsigned int NANOSECONDS_PER_MILLISECOND(1e+6);

// The following values will get calculated at compile time
const float    DEFAULT_FRAME_DURATION_IN_SECONDS(1.0f / 60.0f);
const uint64_t DEFAULT_FRAME_DURATION_IN_MILLISECONDS(DEFAULT_FRAME_DURATION_IN_SECONDS* MILLISECONDS_PER_SECOND);
const uint64_t DEFAULT_FRAME_DURATION_IN_NANOSECONDS(DEFAULT_FRAME_DURATION_IN_SECONDS* NANOSECONDS_PER_SECOND);

/**
 * Handles the use case when an update-request is received JUST before we process a sleep-request. If we did not have an update-request count then
 * there is a danger that, on the event-thread we could have:
 *  1) An update-request where we do nothing as Update/Render thread still running.
 *  2) Quickly followed by a sleep-request being handled where we pause the Update/Render Thread (even though we have an update to process).
 *
 * Using a counter means we increment the counter on an update-request, and decrement it on a sleep-request. This handles the above scenario because:
 *  1) MAIN THREAD:           Update Request: COUNTER = 1
 *  2) UPDATE/RENDER THREAD:  Do Update/Render, then no Updates required -> Sleep Trigger
 *  3) MAIN THREAD:           Update Request: COUNTER = 2
 *  4) MAIN THREAD:           Sleep Request:  COUNTER = 1 -> We do not sleep just yet
 *
 * Also ensures we preserve battery life by only doing ONE update when the above use case is not triggered.
 *  1) MAIN THREAD:           Update Request: COUNTER = 1
 *  2) UPDATE/RENDER THREAD:  Do Update/Render, then no Updates required -> Sleep Trigger
 *  3) MAIN THREAD:           Sleep Request:  COUNTER = 0 -> Go to sleep
 */
const unsigned int MAXIMUM_UPDATE_REQUESTS = 2;

inline std::vector<char> StringToVector(const std::string& str)
{
  auto retval = std::vector<char>{};
  retval.insert(retval.begin(), str.begin(), str.end());
  retval.push_back('\0');
  return retval;
}

} // unnamed namespace

///////////////////////////////////////////////////////////////////////////////////////////////////
// EVENT THREAD
///////////////////////////////////////////////////////////////////////////////////////////////////

CombinedUpdateRenderController::CombinedUpdateRenderController(AdaptorInternalServices& adaptorInterfaces, const EnvironmentOptions& environmentOptions, ThreadMode threadMode)
: mFpsTracker(environmentOptions),
  mUpdateStatusLogger(environmentOptions),
  mEventThreadSemaphore(0),
  mSurfaceSemaphore(0),
  mUpdateRenderThreadWaitCondition(),
  mAdaptorInterfaces(adaptorInterfaces),
  mPerformanceInterface(adaptorInterfaces.GetPerformanceInterface()),
  mCore(adaptorInterfaces.GetCore()),
  mEnvironmentOptions(environmentOptions),
  mNotificationTrigger(adaptorInterfaces.GetProcessCoreEventsTrigger()),
  mSleepTrigger(NULL),
  mPreRenderCallback(NULL),
  mTextureUploadManager(adaptorInterfaces.GetTextureUploadManager()),
  mUpdateRenderThread(NULL),
  mDefaultFrameDelta(0.0f),
  mDefaultFrameDurationMilliseconds(0u),
  mDefaultFrameDurationNanoseconds(0u),
  mDefaultHalfFrameNanoseconds(0u),
  mUpdateRequestCount(0u),
  mRunning(FALSE),
  mVsyncRender(TRUE),
  mThreadId(0),
  mThreadMode(threadMode),
  mUpdateRenderRunCount(0),
  mDestroyUpdateRenderThread(FALSE),
  mUpdateRenderThreadCanSleep(FALSE),
  mPendingRequestUpdate(FALSE),
  mUseElapsedTimeAfterWait(FALSE),
  mIsPreCompileCancelled(FALSE),
  mNewSurface(NULL),
  mDeletedSurface(nullptr),
  mPostRendering(FALSE),
  mSurfaceResized(0),
  mForceClear(FALSE),
  mUploadWithoutRendering(FALSE),
  mFirstFrameAfterResume(FALSE)
{
  LOG_EVENT_TRACE;

  // Initialise frame delta/duration variables first
  SetRenderRefreshRate(environmentOptions.GetRenderRefreshRate());

  // Set the thread-synchronization interface on the render-surface
  Dali::RenderSurfaceInterface* currentSurface = mAdaptorInterfaces.GetRenderSurfaceInterface();
  if(currentSurface)
  {
    currentSurface->SetThreadSynchronization(*this);
  }

  mVsyncRender = environmentOptions.VsyncRenderRequired();

  mSleepTrigger = TriggerEventFactory::CreateTriggerEvent(MakeCallback(this, &CombinedUpdateRenderController::ProcessSleepRequest), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER);
}

CombinedUpdateRenderController::~CombinedUpdateRenderController()
{
  LOG_EVENT_TRACE;

  Stop();

  delete mPreRenderCallback;
  delete mSleepTrigger;
}

void CombinedUpdateRenderController::Initialize()
{
  LOG_EVENT_TRACE;

  // Ensure Update/Render Thread not already created
  DALI_ASSERT_ALWAYS(!mUpdateRenderThread);

  // Create Update/Render Thread
  ConditionalWait::ScopedLock lock(mGraphicsInitializeWait);
  mUpdateRenderThread = new pthread_t();
  int error           = pthread_create(mUpdateRenderThread, NULL, InternalUpdateRenderThreadEntryFunc, this);
  DALI_ASSERT_ALWAYS(!error && "Return code from pthread_create() when creating UpdateRenderThread");

  // The Update/Render thread will now run and initialise the graphics interface etc. and will then wait for Start to be called
  // When this function returns, the application initialisation on the event thread should occur
}

void CombinedUpdateRenderController::Start()
{
  LOG_EVENT_TRACE;

  DALI_ASSERT_ALWAYS(!mRunning && mUpdateRenderThread);

  // Wait until all threads created in Initialise are up and running
  for(unsigned int i = 0; i < CREATED_THREAD_COUNT; ++i)
  {
    mEventThreadSemaphore.Acquire();
  }

  mRunning = TRUE;

  LOG_EVENT("Startup Complete, starting Update/Render Thread");

  RunUpdateRenderThread(CONTINUOUS, AnimationProgression::NONE, UpdateMode::NORMAL);

  Dali::RenderSurfaceInterface* currentSurface = mAdaptorInterfaces.GetRenderSurfaceInterface();
  if(currentSurface)
  {
    currentSurface->StartRender();
  }

  DALI_LOG_RELEASE_INFO("CombinedUpdateRenderController::Start\n");
}

void CombinedUpdateRenderController::Pause()
{
  LOG_EVENT_TRACE;

  mRunning = FALSE;

  PauseUpdateRenderThread();

  AddPerformanceMarker(PerformanceInterface::PAUSED);

  DALI_LOG_RELEASE_INFO("CombinedUpdateRenderController::Pause\n");
}

void CombinedUpdateRenderController::Resume()
{
  LOG_EVENT_TRACE;

  if(!mRunning && IsUpdateRenderThreadPaused())
  {
    LOG_EVENT("Resuming");

    RunUpdateRenderThread(CONTINUOUS, AnimationProgression::USE_ELAPSED_TIME, UpdateMode::NORMAL);

    AddPerformanceMarker(PerformanceInterface::RESUME);

    mRunning               = TRUE;
    mForceClear            = TRUE;
    mFirstFrameAfterResume = TRUE;

    DALI_LOG_RELEASE_INFO("CombinedUpdateRenderController::Resume\n");
  }
  else
  {
    DALI_LOG_RELEASE_INFO("CombinedUpdateRenderController::Resume: Already resumed [%d, %d, %d]\n", mRunning, mUpdateRenderRunCount, mUpdateRenderThreadCanSleep);
  }
}

void CombinedUpdateRenderController::Stop()
{
  LOG_EVENT_TRACE;

  // Stop Rendering and the Update/Render Thread
  Dali::RenderSurfaceInterface* currentSurface = mAdaptorInterfaces.GetRenderSurfaceInterface();
  if(currentSurface)
  {
    currentSurface->StopRender();
  }

  StopUpdateRenderThread();

  if(mUpdateRenderThread)
  {
    LOG_EVENT("Destroying UpdateRenderThread");

    // wait for the thread to finish
    pthread_join(*mUpdateRenderThread, NULL);

    delete mUpdateRenderThread;
    mUpdateRenderThread = NULL;
  }

  mRunning = FALSE;

  DALI_LOG_RELEASE_INFO("CombinedUpdateRenderController::Stop\n");
}

void CombinedUpdateRenderController::RequestUpdate()
{
  LOG_EVENT_TRACE;

  // Increment the update-request count to the maximum
  if(mUpdateRequestCount < MAXIMUM_UPDATE_REQUESTS)
  {
    ++mUpdateRequestCount;
  }

  if(mRunning && IsUpdateRenderThreadPaused())
  {
    LOG_EVENT("Processing");

    RunUpdateRenderThread(CONTINUOUS, AnimationProgression::NONE, UpdateMode::NORMAL);
  }

  ConditionalWait::ScopedLock updateLock(mUpdateRenderThreadWaitCondition);
  mPendingRequestUpdate = TRUE;
}

void CombinedUpdateRenderController::RequestUpdateOnce(UpdateMode updateMode)
{
  // Increment the update-request count to the maximum
  if(mUpdateRequestCount < MAXIMUM_UPDATE_REQUESTS)
  {
    ++mUpdateRequestCount;
  }

  if(IsUpdateRenderThreadPaused() || updateMode == UpdateMode::FORCE_RENDER)
  {
    LOG_EVENT_TRACE;

    // Run Update/Render once
    RunUpdateRenderThread(ONCE, AnimationProgression::NONE, updateMode);
  }
}

void CombinedUpdateRenderController::ReplaceSurface(Dali::RenderSurfaceInterface* newSurface)
{
  LOG_EVENT_TRACE;

  if(mUpdateRenderThread)
  {
    // Set the ThreadSyncronizationInterface on the new surface
    newSurface->SetThreadSynchronization(*this);

    LOG_EVENT("Starting to replace the surface, event-thread blocked");

    // Start replacing the surface.
    {
      ConditionalWait::ScopedLock lock(mUpdateRenderThreadWaitCondition);
      mPostRendering = FALSE; // Clear the post-rendering flag as Update/Render thread will replace the surface now
      mNewSurface    = newSurface;
      CancelPreCompile();
      mUpdateRenderThreadWaitCondition.Notify(lock);
    }

    // Wait until the surface has been replaced
    mSurfaceSemaphore.Acquire();

    LOG_EVENT("Surface replaced, event-thread continuing");
  }
}

void CombinedUpdateRenderController::DeleteSurface(Dali::RenderSurfaceInterface* surface)
{
  LOG_EVENT_TRACE;

  if(mUpdateRenderThread)
  {
    LOG_EVENT("Starting to delete the surface, event-thread blocked");

    // Start replacing the surface.
    {
      ConditionalWait::ScopedLock lock(mUpdateRenderThreadWaitCondition);
      mPostRendering  = FALSE; // Clear the post-rendering flag as Update/Render thread will delete the surface now
      mDeletedSurface = surface;
      CancelPreCompile();
      mUpdateRenderThreadWaitCondition.Notify(lock);
    }

    // Wait until the surface has been deleted
    mSurfaceSemaphore.Acquire();

    LOG_EVENT("Surface deleted, event-thread continuing");
  }
}

void CombinedUpdateRenderController::WaitForGraphicsInitialization()
{
  ConditionalWait::ScopedLock lk(mGraphicsInitializeWait);
  LOG_EVENT_TRACE;

  if(mUpdateRenderThread)
  {
    LOG_EVENT("Waiting for graphics initialisation, event-thread blocked");

    // Wait until the graphics has been initialised
    mGraphicsInitializeWait.Wait(lk);

    LOG_EVENT("graphics initialised, event-thread continuing");
  }
}

void CombinedUpdateRenderController::ResizeSurface()
{
  LOG_EVENT_TRACE;

  LOG_EVENT("Resize the surface");

  {
    ConditionalWait::ScopedLock lock(mUpdateRenderThreadWaitCondition);
    // Surface is resized and the surface resized count is increased.
    mSurfaceResized++;
    CancelPreCompile();
    mUpdateRenderThreadWaitCondition.Notify(lock);
  }
}

void CombinedUpdateRenderController::SetRenderRefreshRate(unsigned int numberOfFramesPerRender)
{
  // Not protected by lock, but written to rarely so not worth adding a lock when reading
  mDefaultFrameDelta                = numberOfFramesPerRender * DEFAULT_FRAME_DURATION_IN_SECONDS;
  mDefaultFrameDurationMilliseconds = uint64_t(numberOfFramesPerRender) * DEFAULT_FRAME_DURATION_IN_MILLISECONDS;
  mDefaultFrameDurationNanoseconds  = uint64_t(numberOfFramesPerRender) * DEFAULT_FRAME_DURATION_IN_NANOSECONDS;
  mDefaultHalfFrameNanoseconds      = mDefaultFrameDurationNanoseconds / 2u;

  LOG_EVENT("mDefaultFrameDelta(%.6f), mDefaultFrameDurationMilliseconds(%lld), mDefaultFrameDurationNanoseconds(%lld)", mDefaultFrameDelta, mDefaultFrameDurationMilliseconds, mDefaultFrameDurationNanoseconds);
}

void CombinedUpdateRenderController::SetPreRenderCallback(CallbackBase* callback)
{
  LOG_EVENT_TRACE;
  LOG_EVENT("Set PreRender Callback");

  ConditionalWait::ScopedLock updateLock(mUpdateRenderThreadWaitCondition);
  if(mPreRenderCallback)
  {
    delete mPreRenderCallback;
  }
  mPreRenderCallback = callback;
}

void CombinedUpdateRenderController::AddSurface(Dali::RenderSurfaceInterface* surface)
{
  LOG_EVENT_TRACE;
  LOG_EVENT("Surface is added");
  if(mUpdateRenderThread)
  {
    // Set the ThreadSyncronizationInterface on the added surface
    surface->SetThreadSynchronization(*this);
  }
}

int32_t CombinedUpdateRenderController::GetThreadId() const
{
  return mThreadId;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// EVENT THREAD
///////////////////////////////////////////////////////////////////////////////////////////////////

void CombinedUpdateRenderController::RunUpdateRenderThread(int numberOfCycles, AnimationProgression animationProgression, UpdateMode updateMode)
{
  ConditionalWait::ScopedLock lock(mUpdateRenderThreadWaitCondition);

  switch(mThreadMode)
  {
    case ThreadMode::NORMAL:
    {
      mUpdateRenderRunCount    = numberOfCycles;
      mUseElapsedTimeAfterWait = (animationProgression == AnimationProgression::USE_ELAPSED_TIME);
      break;
    }
    case ThreadMode::RUN_IF_REQUESTED:
    {
      if(updateMode != UpdateMode::FORCE_RENDER)
      {
        // Render only if the update mode is FORCE_RENDER which means the application requests it.
        // We don't want to awake the update thread.
        return;
      }

      mUpdateRenderRunCount++;         // Increase the update request count
      mUseElapsedTimeAfterWait = TRUE; // The elapsed time should be used. We want animations to proceed.
      break;
    }
  }

  mUpdateRenderThreadCanSleep = FALSE;
  mUploadWithoutRendering     = (updateMode == UpdateMode::SKIP_RENDER);
  LOG_COUNTER_EVENT("mUpdateRenderRunCount: %d, mUseElapsedTimeAfterWait: %d", mUpdateRenderRunCount, mUseElapsedTimeAfterWait);
  CancelPreCompile();
  mUpdateRenderThreadWaitCondition.Notify(lock);
}

void CombinedUpdateRenderController::PauseUpdateRenderThread()
{
  ConditionalWait::ScopedLock lock(mUpdateRenderThreadWaitCondition);
  mUpdateRenderRunCount = 0;
}

void CombinedUpdateRenderController::StopUpdateRenderThread()
{
  ConditionalWait::ScopedLock lock(mUpdateRenderThreadWaitCondition);
  mDestroyUpdateRenderThread = TRUE;
  CancelPreCompile();
  mUpdateRenderThreadWaitCondition.Notify(lock);
}

bool CombinedUpdateRenderController::IsUpdateRenderThreadPaused()
{
  ConditionalWait::ScopedLock lock(mUpdateRenderThreadWaitCondition);

  if(mThreadMode == ThreadMode::RUN_IF_REQUESTED)
  {
    return !mRunning || mUpdateRenderThreadCanSleep;
  }

  return (mUpdateRenderRunCount != CONTINUOUS) || // Report paused if NOT continuously running
         mUpdateRenderThreadCanSleep;             // Report paused if sleeping
}

void CombinedUpdateRenderController::ProcessSleepRequest()
{
  LOG_EVENT_TRACE;

  // Decrement Update request count
  if(mUpdateRequestCount > 0)
  {
    --mUpdateRequestCount;
  }

  // Can sleep if our update-request count is 0
  // Update/Render thread can choose to carry on updating if it determines more update/renders are required
  if(mUpdateRequestCount == 0)
  {
    LOG_EVENT("Going to sleep");

    ConditionalWait::ScopedLock lock(mUpdateRenderThreadWaitCondition);
    mUpdateRenderThreadCanSleep = TRUE;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// UPDATE/RENDER THREAD
///////////////////////////////////////////////////////////////////////////////////////////////////

void CombinedUpdateRenderController::UpdateRenderThread()
{
  ThreadSettings::SetThreadName("RenderThread\0");
  mThreadId = ThreadSettings::GetThreadId();

  // Install a function for logging
  mEnvironmentOptions.InstallLogFunction();

  // Install a function for tracing
  mEnvironmentOptions.InstallTraceFunction();

  TRACE_UPDATE_RENDER_BEGIN("DALI_RENDER_THREAD_INIT");

  LOG_UPDATE_RENDER("THREAD CREATED");

  // Initialize graphics
  GraphicsInterface& graphics = mAdaptorInterfaces.GetGraphicsInterface();
  graphics.Initialize();

  Dali::DisplayConnection& displayConnection = mAdaptorInterfaces.GetDisplayConnectionInterface();
  displayConnection.Initialize(); //@todo Move InitializeGraphics code into graphics implementation

  // Setup graphics controller into upload manager.
  GetImplementation(mTextureUploadManager).InitalizeGraphicsController(graphics.GetController());

  NotifyGraphicsInitialised();

  //@todo Vk swaps this around, but we need to support surfaceless context for multi-window
  graphics.ConfigureSurface(mAdaptorInterfaces.GetRenderSurfaceInterface());

  // Tell core it has a context
  mCore.ContextCreated();

  NotifyThreadInitialised();

  // Initialize and create graphics resource for the shared context.
  WindowContainer windows;
  mAdaptorInterfaces.GetWindowContainerInterface(windows);

  for(auto&& window : windows)
  {
    Dali::Integration::Scene      scene         = window->GetScene();
    Dali::RenderSurfaceInterface* windowSurface = window->GetSurface();

    if(scene && windowSurface)
    {
      windowSurface->InitializeGraphics();
    }
  }

  // Update time
  uint64_t lastFrameTime;
  TimeService::GetNanoseconds(lastFrameTime);
  uint64_t lastMemPoolLogTime = lastFrameTime;

  LOG_UPDATE_RENDER("THREAD INITIALISED");

  bool     useElapsedTime     = true;
  bool     updateRequired     = true;
  uint64_t timeToSleepUntil   = 0;
  int      extraFramesDropped = 0;

  const uint64_t memPoolInterval = 1e9 * float(mEnvironmentOptions.GetMemoryPoolInterval());

  const unsigned int renderToFboInterval = mEnvironmentOptions.GetRenderToFboInterval();
  const bool         renderToFboEnabled  = 0u != renderToFboInterval;
  unsigned int       frameCount          = 0u;

  TRACE_UPDATE_RENDER_END("DALI_RENDER_THREAD_INIT");
  if(!mDestroyUpdateRenderThread)
  {
    ShaderPreCompiler::Get().Wait();
    if(ShaderPreCompiler::Get().IsEnable())
    {
      std::vector<RawShaderData> precompiledShaderList;
      ShaderPreCompiler::Get().GetPreCompileShaderList(precompiledShaderList);
      DALI_LOG_RELEASE_INFO("ShaderPreCompiler[ENABLE], list size:%d \n", precompiledShaderList.size());
      for(auto precompiledShader = precompiledShaderList.begin(); precompiledShader != precompiledShaderList.end(); ++precompiledShader)
      {
        if(mIsPreCompileCancelled == TRUE)
        {
          ShaderPreCompiler::Get().Awake();
          DALI_LOG_RELEASE_INFO("ShaderPreCompiler[ENABLE], but stop precompile");
          break;
        }

        auto numberOfPrecompiledShader = precompiledShader->shaderCount;
        for(int i = 0; i < numberOfPrecompiledShader; ++i)
        {
          auto vertexShader   = graphics.GetController().GetGraphicsConfig().GetVertexShaderPrefix() + std::string(precompiledShader->vertexPrefix[i].data()) + std::string(precompiledShader->vertexShader.data());
          auto fragmentShader = graphics.GetController().GetGraphicsConfig().GetFragmentShaderPrefix() + std::string(precompiledShader->fragmentPrefix[i].data()) + std::string(precompiledShader->fragmentShader.data());
          PreCompileShader(std::move(vertexShader), std::move(fragmentShader), static_cast<uint32_t>(i) < precompiledShader->shaderName.size() ? std::string(precompiledShader->shaderName[i]) : "");
        }
        DALI_LOG_RELEASE_INFO("ShaderPreCompiler[ENABLE], shader count :%d \n", numberOfPrecompiledShader);
      }
    }
    else
    {
      DALI_LOG_RELEASE_INFO("ShaderPreCompiler[DISABLE] \n");
    }
  }

  while(UpdateRenderReady(useElapsedTime, updateRequired, timeToSleepUntil))
  {
    LOG_UPDATE_RENDER_TRACE;
    TRACE_UPDATE_RENDER_BEGIN("DALI_UPDATE_RENDER");

    // For thread safe
    bool                          uploadOnly     = mUploadWithoutRendering;
    unsigned int                  surfaceResized = mSurfaceResized;
    Dali::RenderSurfaceInterface* deletedSurface = ShouldSurfaceBeDeleted();

    // Performance statistics are logged upon a VSYNC tick so use this point for a VSync marker
    AddPerformanceMarker(PerformanceInterface::VSYNC);

    uint64_t currentFrameStartTime = 0;
    TimeService::GetNanoseconds(currentFrameStartTime);

    uint64_t timeSinceLastFrame = currentFrameStartTime - lastFrameTime;

    // Optional FPS Tracking when continuously rendering
    if(useElapsedTime && mFpsTracker.Enabled())
    {
      float absoluteTimeSinceLastRender = timeSinceLastFrame * NANOSECONDS_TO_SECOND;
      mFpsTracker.Track(absoluteTimeSinceLastRender);
    }

    lastFrameTime = currentFrameStartTime; // Store frame start time

    //////////////////////////////
    // REPLACE SURFACE
    //////////////////////////////

    Dali::RenderSurfaceInterface* newSurface = ShouldSurfaceBeReplaced();
    if(DALI_UNLIKELY(newSurface))
    {
      LOG_UPDATE_RENDER_TRACE_FMT("Replacing Surface");
      // This is designed for replacing pixmap surfaces, but should work for window as well
      // we need to delete the surface and renderable (pixmap / window)
      // Then create a new pixmap/window and new surface
      // If the new surface has a different display connection, then the context will be lost
      mAdaptorInterfaces.GetDisplayConnectionInterface().Initialize();
      graphics.ActivateSurfaceContext(newSurface);
      // TODO: ReplaceGraphicsSurface doesn't work, InitializeGraphics()
      // already creates new surface window, the surface and the context.
      // We probably don't need ReplaceGraphicsSurface at all.
      // newSurface->ReplaceGraphicsSurface();
      SurfaceReplaced();
    }

    //////////////////////////////
    // TextureUploadRequest (phase #1)
    //////////////////////////////

    // Upload requested resources after resource context activated.
    graphics.ActivateResourceContext();

    const bool textureUploaded = mTextureUploadManager.ResourceUpload();

    // Update & Render forcely if there exist some uploaded texture.
    uploadOnly = textureUploaded ? false : uploadOnly;

    const bool isRenderingToFbo = renderToFboEnabled && ((0u == frameCount) || (0u != frameCount % renderToFboInterval));
    ++frameCount;

    //////////////////////////////
    // UPDATE
    //////////////////////////////

    const uint32_t currentTime   = static_cast<uint32_t>(currentFrameStartTime / NANOSECONDS_PER_MILLISECOND);
    const uint32_t nextFrameTime = currentTime + static_cast<uint32_t>(mDefaultFrameDurationMilliseconds);

    uint64_t noOfFramesSinceLastUpdate = 1;
    float    frameDelta                = 0.0f;
    if(useElapsedTime)
    {
      if(mThreadMode == ThreadMode::RUN_IF_REQUESTED)
      {
        extraFramesDropped = 0;
        while(timeSinceLastFrame >= mDefaultFrameDurationNanoseconds)
        {
          timeSinceLastFrame -= mDefaultFrameDurationNanoseconds;
          extraFramesDropped++;
        }
      }

      // If using the elapsed time, then calculate frameDelta as a multiple of mDefaultFrameDelta
      noOfFramesSinceLastUpdate += extraFramesDropped;

      frameDelta = mDefaultFrameDelta * noOfFramesSinceLastUpdate;
    }
    LOG_UPDATE_RENDER("timeSinceLastFrame(%llu) noOfFramesSinceLastUpdate(%u) frameDelta(%.6f)", timeSinceLastFrame, noOfFramesSinceLastUpdate, frameDelta);

    Integration::UpdateStatus updateStatus;

    AddPerformanceMarker(PerformanceInterface::UPDATE_START);
    TRACE_UPDATE_RENDER_BEGIN("DALI_UPDATE");
    mCore.Update(frameDelta,
                 currentTime,
                 nextFrameTime,
                 updateStatus,
                 renderToFboEnabled,
                 isRenderingToFbo,
                 uploadOnly);
    TRACE_UPDATE_RENDER_END("DALI_UPDATE");
    AddPerformanceMarker(PerformanceInterface::UPDATE_END);

    unsigned int keepUpdatingStatus = updateStatus.KeepUpdating();

    // Tell the event-thread to wake up (if asleep) and send a notification event to Core if required
    if(updateStatus.NeedsNotification())
    {
      mNotificationTrigger.Trigger();
      LOG_UPDATE_RENDER("Notification Triggered");
    }

    if(uploadOnly && (keepUpdatingStatus & (Dali::Integration::KeepUpdating::STAGE_KEEP_RENDERING | Dali::Integration::KeepUpdating::FRAME_UPDATE_CALLBACK)))
    {
      // Render forcely if there exist some keep rendering required.
      uploadOnly = false;
    }

    // Optional logging of update/render status
    mUpdateStatusLogger.Log(keepUpdatingStatus);

    //////////////////////////////
    // RENDER
    //////////////////////////////

    graphics.FrameStart();
    mAdaptorInterfaces.GetDisplayConnectionInterface().ConsumeEvents();

    if(mPreRenderCallback != NULL)
    {
      bool keepCallback = CallbackBase::ExecuteReturn<bool>(*mPreRenderCallback);
      if(!keepCallback)
      {
        delete mPreRenderCallback;
        mPreRenderCallback = NULL;
      }
    }

    //////////////////////////////
    // TextureUploadRequest (phase #2)
    //////////////////////////////

    // Upload requested resources after resource context activated.
    graphics.ActivateResourceContext();

    // Since uploadOnly value used at Update side, we should not change uploadOnly value now even some textures are uploaded.
    mTextureUploadManager.ResourceUpload();

    if(mFirstFrameAfterResume)
    {
      // mFirstFrameAfterResume is set to true when the thread is resumed
      // Let graphics know the first frame after thread initialized or resumed.
      graphics.SetFirstFrameAfterResume();
      mFirstFrameAfterResume = FALSE;
    }

    Integration::RenderStatus renderStatus;

    AddPerformanceMarker(PerformanceInterface::RENDER_START);
    TRACE_UPDATE_RENDER_BEGIN("DALI_RENDER");

    // Upload shared resources
    TRACE_UPDATE_RENDER_BEGIN("DALI_PRE_RENDER");
    mCore.PreRender(renderStatus, mForceClear);
    TRACE_UPDATE_RENDER_END("DALI_PRE_RENDER");

    if(!uploadOnly || surfaceResized)
    {
      // Go through each window
      windows.clear();
      mAdaptorInterfaces.GetWindowContainerInterface(windows);

      for(auto&& window : windows)
      {
        Dali::Integration::Scene      scene         = window->GetScene();
        Dali::RenderSurfaceInterface* windowSurface = window->GetSurface();

        if(scene && windowSurface)
        {
          TRACE_UPDATE_RENDER_SCOPE("DALI_RENDER_SCENE");
          Integration::RenderStatus windowRenderStatus;

          const uint32_t sceneSurfaceResized = scene.GetSurfaceRectChangedCount();

          // clear previous frame damaged render items rects, buffer history is tracked on surface level
          mDamagedRects.clear();

          // Collect damage rects
          mCore.PreRender(scene, mDamagedRects);

          // Render off-screen frame buffers first if any
          mCore.RenderScene(windowRenderStatus, scene, true);

          Rect<int> clippingRect; // Empty for fbo rendering

          // Switch to the context of the surface, merge damaged areas for previous frames
          windowSurface->PreRender(sceneSurfaceResized > 0u, mDamagedRects, clippingRect); // Switch GL context

          // Render the surface
          mCore.RenderScene(windowRenderStatus, scene, false, clippingRect);

          // Buffer swapping now happens when the surface render target is presented.

          // If surface is resized, the surface resized count is decreased.
          if(DALI_UNLIKELY(sceneSurfaceResized > 0u))
          {
            SurfaceResized(sceneSurfaceResized);
          }
        }
      }
    }
    else
    {
      DALI_LOG_RELEASE_INFO("DALI Rendering skip (upload only)\n");
    }

    TRACE_UPDATE_RENDER_BEGIN("DALI_POST_RENDER");
    if(!uploadOnly)
    {
      graphics.PostRender();
    }

    mCore.PostRender();
    TRACE_UPDATE_RENDER_END("DALI_POST_RENDER");

    //////////////////////////////
    // DELETE SURFACE
    //////////////////////////////
    if(DALI_UNLIKELY(deletedSurface))
    {
      LOG_UPDATE_RENDER_TRACE_FMT("Deleting Surface");

      deletedSurface->DestroySurface();

      SurfaceDeleted();
    }

    TRACE_UPDATE_RENDER_END("DALI_RENDER");
    AddPerformanceMarker(PerformanceInterface::RENDER_END);

    // if the memory pool interval is set and has elapsed, log the graphics memory pools
    if(0 < memPoolInterval && memPoolInterval < lastFrameTime - lastMemPoolLogTime)
    {
      lastMemPoolLogTime = lastFrameTime;
      graphics.LogMemoryPools();
    }

    mForceClear = false;

    // Trigger event thread to request Update/Render thread to sleep if update not required.
    if((Integration::KeepUpdating::NOT_REQUESTED == keepUpdatingStatus) && !renderStatus.NeedsUpdate())
    {
      mSleepTrigger->Trigger();
      updateRequired = false;
      LOG_UPDATE_RENDER("Sleep Triggered");
    }
    else
    {
      updateRequired = true;
    }

    //////////////////////////////
    // FRAME TIME
    //////////////////////////////

    extraFramesDropped = 0;

    if(timeToSleepUntil == 0)
    {
      // If this is the first frame after the thread is initialized or resumed, we
      // use the actual time the current frame starts from to calculate the time to
      // sleep until the next frame.
      timeToSleepUntil = currentFrameStartTime + mDefaultFrameDurationNanoseconds;
    }
    else
    {
      // Otherwise, always use the sleep-until time calculated in the last frame to
      // calculate the time to sleep until the next frame. In this way, if there is
      // any time gap between the current frame and the next frame, or if update or
      // rendering in the current frame takes too much time so that the specified
      // sleep-until time has already passed, it will try to keep the frames syncing
      // by shortening the duration of the next frame.
      timeToSleepUntil += mDefaultFrameDurationNanoseconds;

      // Check the current time at the end of the frame
      uint64_t currentFrameEndTime = 0;
      TimeService::GetNanoseconds(currentFrameEndTime);
      while(currentFrameEndTime > timeToSleepUntil + mDefaultFrameDurationNanoseconds)
      {
        // We are more than one frame behind already, so just drop the next frames
        // until the sleep-until time is later than the current time so that we can
        // catch up.
        timeToSleepUntil += mDefaultFrameDurationNanoseconds;
        extraFramesDropped++;
      }
    }

    TRACE_UPDATE_RENDER_END("DALI_UPDATE_RENDER");

    // Render to FBO is intended to measure fps above 60 so sleep is not wanted.
    if(mVsyncRender && 0u == renderToFboInterval)
    {
      TRACE_UPDATE_RENDER_SCOPE("DALI_UPDATE_RENDER_SLEEP");
      // Sleep until at least the the default frame duration has elapsed. This will return immediately if the specified end-time has already passed.
      TimeService::SleepUntil(timeToSleepUntil);
    }
  }
  TRACE_UPDATE_RENDER_BEGIN("DALI_RENDER_THREAD_FINISH");

  // Inform core of context destruction
  mCore.ContextDestroyed();

  windows.clear();
  mAdaptorInterfaces.GetWindowContainerInterface(windows);

  // Destroy surfaces
  for(auto&& window : windows)
  {
    Dali::RenderSurfaceInterface* surface = window->GetSurface();
    surface->DestroySurface();
  }

  graphics.Shutdown();

  LOG_UPDATE_RENDER("THREAD DESTROYED");

  TRACE_UPDATE_RENDER_END("DALI_RENDER_THREAD_FINISH");

  // Uninstall the logging function
  mEnvironmentOptions.UnInstallLogFunction();
}

bool CombinedUpdateRenderController::UpdateRenderReady(bool& useElapsedTime, bool updateRequired, uint64_t& timeToSleepUntil)
{
  useElapsedTime = true;

  ConditionalWait::ScopedLock updateLock(mUpdateRenderThreadWaitCondition);
  while((!mUpdateRenderRunCount ||                                                      // Should try to wait if event-thread has paused the Update/Render thread
         (mUpdateRenderThreadCanSleep && !updateRequired && !mPendingRequestUpdate)) && // Ensure we wait if we're supposed to be sleeping AND do not require another update
        !mDestroyUpdateRenderThread &&                                                  // Ensure we don't wait if the update-render-thread is supposed to be destroyed
        !mNewSurface &&                                                                 // Ensure we don't wait if we need to replace the surface
        !mDeletedSurface &&                                                             // Ensure we don't wait if we need to delete the surface
        !mSurfaceResized)                                                               // Ensure we don't wait if we need to resize the surface
  {
    LOG_UPDATE_RENDER("WAIT: mUpdateRenderRunCount:       %d", mUpdateRenderRunCount);
    LOG_UPDATE_RENDER("      mUpdateRenderThreadCanSleep: %d, updateRequired: %d, mPendingRequestUpdate: %d", mUpdateRenderThreadCanSleep, updateRequired, mPendingRequestUpdate);
    LOG_UPDATE_RENDER("      mDestroyUpdateRenderThread:  %d", mDestroyUpdateRenderThread);
    LOG_UPDATE_RENDER("      mNewSurface:                 %d", mNewSurface);
    LOG_UPDATE_RENDER("      mDeletedSurface:             %d", mDeletedSurface);
    LOG_UPDATE_RENDER("      mSurfaceResized:             %d", mSurfaceResized);

    // Reset the time when the thread is waiting, so the sleep-until time for
    // the first frame after resuming should be based on the actual start time
    // of the first frame.
    timeToSleepUntil = 0;

    TRACE_UPDATE_RENDER_BEGIN("DALI_UPDATE_RENDER_THREAD_WAIT_CONDITION");
    mUpdateRenderThreadWaitCondition.Wait(updateLock);
    TRACE_UPDATE_RENDER_END("DALI_UPDATE_RENDER_THREAD_WAIT_CONDITION");

    if(!mUseElapsedTimeAfterWait)
    {
      useElapsedTime = false;
    }
  }

  LOG_COUNTER_UPDATE_RENDER("mUpdateRenderRunCount:       %d", mUpdateRenderRunCount);
  LOG_COUNTER_UPDATE_RENDER("mUpdateRenderThreadCanSleep: %d, updateRequired: %d, mPendingRequestUpdate: %d", mUpdateRenderThreadCanSleep, updateRequired, mPendingRequestUpdate);
  LOG_COUNTER_UPDATE_RENDER("mDestroyUpdateRenderThread:  %d", mDestroyUpdateRenderThread);
  LOG_COUNTER_UPDATE_RENDER("mNewSurface:                 %d", mNewSurface);
  LOG_COUNTER_UPDATE_RENDER("mDeletedSurface:             %d", mDeletedSurface);
  LOG_COUNTER_UPDATE_RENDER("mSurfaceResized:             %d", mSurfaceResized);

  mUseElapsedTimeAfterWait    = FALSE;
  mUpdateRenderThreadCanSleep = FALSE;
  mPendingRequestUpdate       = FALSE;

  // If we've been asked to run Update/Render cycles a finite number of times then decrement so we wait after the
  // requested number of cycles
  if(mUpdateRenderRunCount > 0)
  {
    --mUpdateRenderRunCount;
  }

  // Keep the update-render thread alive if this thread is NOT to be destroyed
  return !mDestroyUpdateRenderThread;
}

Dali::RenderSurfaceInterface* CombinedUpdateRenderController::ShouldSurfaceBeReplaced()
{
  ConditionalWait::ScopedLock lock(mUpdateRenderThreadWaitCondition);

  Dali::RenderSurfaceInterface* newSurface = mNewSurface;
  mNewSurface                              = NULL;

  return newSurface;
}

void CombinedUpdateRenderController::SurfaceReplaced()
{
  // Just increment the semaphore
  mSurfaceSemaphore.Release(1);
}

Dali::RenderSurfaceInterface* CombinedUpdateRenderController::ShouldSurfaceBeDeleted()
{
  ConditionalWait::ScopedLock lock(mUpdateRenderThreadWaitCondition);

  Dali::RenderSurfaceInterface* deletedSurface = mDeletedSurface;
  mDeletedSurface                              = NULL;

  return deletedSurface;
}

void CombinedUpdateRenderController::SurfaceDeleted()
{
  // Just increment the semaphore
  mSurfaceSemaphore.Release(1);
}

void CombinedUpdateRenderController::SurfaceResized(uint32_t resizedCount)
{
  ConditionalWait::ScopedLock lock(mUpdateRenderThreadWaitCondition);

  if(mSurfaceResized >= resizedCount)
  {
    mSurfaceResized -= resizedCount;
  }
  else
  {
    mSurfaceResized = 0u;
  }
}

void CombinedUpdateRenderController::PreCompileShader(std::string vertexShader, std::string fragmentShader, std::string shaderName)
{
  GraphicsInterface& graphics = mAdaptorInterfaces.GetGraphicsInterface();

  Graphics::ShaderCreateInfo vertexShaderCreateInfo;
  vertexShaderCreateInfo.SetPipelineStage(Graphics::PipelineStage::VERTEX_SHADER);
  vertexShaderCreateInfo.SetSourceMode(Graphics::ShaderSourceMode::TEXT);
  const std::vector<char>& vertexShaderSrc = StringToVector(std::move(vertexShader));
  vertexShaderCreateInfo.SetSourceSize(vertexShaderSrc.size());
  vertexShaderCreateInfo.SetSourceData(static_cast<const void*>(vertexShaderSrc.data()));
  auto vertexGraphicsShader = graphics.GetController().CreateShader(vertexShaderCreateInfo, nullptr);

  Graphics::ShaderCreateInfo fragmentShaderCreateInfo;
  fragmentShaderCreateInfo.SetPipelineStage(Graphics::PipelineStage::FRAGMENT_SHADER);
  fragmentShaderCreateInfo.SetSourceMode(Graphics::ShaderSourceMode::TEXT);
  const std::vector<char>& fragmentShaderSrc = StringToVector(std::move(fragmentShader));
  fragmentShaderCreateInfo.SetSourceSize(fragmentShaderSrc.size());
  fragmentShaderCreateInfo.SetSourceData(static_cast<const void*>(fragmentShaderSrc.data()));
  auto fragmentGraphicsShader = graphics.GetController().CreateShader(fragmentShaderCreateInfo, nullptr);

  std::vector<Graphics::ShaderState> shaderStates{
    Graphics::ShaderState()
      .SetShader(*vertexGraphicsShader.get())
      .SetPipelineStage(Graphics::PipelineStage::VERTEX_SHADER),
    Graphics::ShaderState()
      .SetShader(*fragmentGraphicsShader.get())
      .SetPipelineStage(Graphics::PipelineStage::FRAGMENT_SHADER)};

  auto createInfo = Graphics::ProgramCreateInfo();
  createInfo.SetShaderState(shaderStates);
  createInfo.SetName(shaderName);

  auto graphicsProgram = graphics.GetController().CreateProgram(createInfo, nullptr);
  ShaderPreCompiler::Get().AddPreCompiledProgram(std::move(graphicsProgram));
}

void CombinedUpdateRenderController::CancelPreCompile()
{
  if(mIsPreCompileCancelled == FALSE)
  {
    mIsPreCompileCancelled = TRUE;
    ShaderPreCompiler::Get().Awake();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ALL THREADS
///////////////////////////////////////////////////////////////////////////////////////////////////

void CombinedUpdateRenderController::NotifyThreadInitialised()
{
  // Just increment the semaphore
  mEventThreadSemaphore.Release(1);
}

void CombinedUpdateRenderController::NotifyGraphicsInitialised()
{
  mGraphicsInitializeWait.Notify();
}

void CombinedUpdateRenderController::AddPerformanceMarker(PerformanceInterface::MarkerType type)
{
  if(mPerformanceInterface)
  {
    mPerformanceInterface->AddMarker(type);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// POST RENDERING: EVENT THREAD
/////////////////////////////////////////////////////////////////////////////////////////////////

void CombinedUpdateRenderController::PostRenderComplete()
{
  ConditionalWait::ScopedLock lock(mUpdateRenderThreadWaitCondition);
  mPostRendering = FALSE;
  CancelPreCompile();
  mUpdateRenderThreadWaitCondition.Notify(lock);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// POST RENDERING: RENDER THREAD
///////////////////////////////////////////////////////////////////////////////////////////////////

void CombinedUpdateRenderController::PostRenderStarted()
{
  ConditionalWait::ScopedLock lock(mUpdateRenderThreadWaitCondition);
  mPostRendering = TRUE;
}

void CombinedUpdateRenderController::PostRenderWaitForCompletion()
{
  ConditionalWait::ScopedLock lock(mUpdateRenderThreadWaitCondition);
  while(mPostRendering &&
        !mNewSurface &&     // We should NOT wait if we're replacing the surface
        !mDeletedSurface && // We should NOT wait if we're deleting the surface
        !mDestroyUpdateRenderThread)
  {
    mUpdateRenderThreadWaitCondition.Wait(lock);
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
