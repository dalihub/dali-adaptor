/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <errno.h>
#include <dali/integration-api/platform-abstraction.h>

// INTERNAL INCLUDES
#include <dali/integration-api/trigger-event-factory.h>
#include <dali/internal/adaptor/common/combined-update-render-controller-debug.h>
#include <dali/internal/system/common/environment-options.h>
#include <dali/internal/system/common/time-service.h>
#include <dali/internal/adaptor/common/adaptor-internal-services.h>
#include <dali/devel-api/adaptor-framework/thread-settings.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/graphics/gles/egl-implementation.h>
#include <dali/internal/graphics/common/graphics-interface.h>

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
const int ONCE = 1;

const unsigned int TRUE = 1u;
const unsigned int FALSE = 0u;

const unsigned int MILLISECONDS_PER_SECOND( 1e+3 );
const float        NANOSECONDS_TO_SECOND( 1e-9f );
const unsigned int NANOSECONDS_PER_SECOND( 1e+9 );
const unsigned int NANOSECONDS_PER_MILLISECOND( 1e+6 );

// The following values will get calculated at compile time
const float        DEFAULT_FRAME_DURATION_IN_SECONDS( 1.0f / 60.0f );
const uint64_t DEFAULT_FRAME_DURATION_IN_MILLISECONDS( DEFAULT_FRAME_DURATION_IN_SECONDS * MILLISECONDS_PER_SECOND );
const uint64_t DEFAULT_FRAME_DURATION_IN_NANOSECONDS( DEFAULT_FRAME_DURATION_IN_SECONDS * NANOSECONDS_PER_SECOND );

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
} // unnamed namespace

///////////////////////////////////////////////////////////////////////////////////////////////////
// EVENT THREAD
///////////////////////////////////////////////////////////////////////////////////////////////////

CombinedUpdateRenderController::CombinedUpdateRenderController( AdaptorInternalServices& adaptorInterfaces, const EnvironmentOptions& environmentOptions )
: mFpsTracker( environmentOptions ),
  mUpdateStatusLogger( environmentOptions ),
  mEventThreadSemaphore(),
  mUpdateRenderThreadWaitCondition(),
  mAdaptorInterfaces( adaptorInterfaces ),
  mPerformanceInterface( adaptorInterfaces.GetPerformanceInterface() ),
  mCore( adaptorInterfaces.GetCore() ),
  mEnvironmentOptions( environmentOptions ),
  mNotificationTrigger( adaptorInterfaces.GetProcessCoreEventsTrigger() ),
  mSleepTrigger( NULL ),
  mPreRenderCallback( NULL ),
  mUpdateRenderThread( NULL ),
  mDefaultFrameDelta( 0.0f ),
  mDefaultFrameDurationMilliseconds( 0u ),
  mDefaultFrameDurationNanoseconds( 0u ),
  mDefaultHalfFrameNanoseconds( 0u ),
  mUpdateRequestCount( 0u ),
  mRunning( FALSE ),
  mUpdateRenderRunCount( 0 ),
  mDestroyUpdateRenderThread( FALSE ),
  mUpdateRenderThreadCanSleep( FALSE ),
  mPendingRequestUpdate( FALSE ),
  mUseElapsedTimeAfterWait( FALSE ),
  mNewSurface( NULL ),
  mDeletedSurface( nullptr ),
  mPostRendering( FALSE ),
  mSurfaceResized( FALSE ),
  mForceClear( FALSE ),
  mUploadWithoutRendering( FALSE ),
  mFirstFrameAfterResume( FALSE )
{
  LOG_EVENT_TRACE;

  // Initialise frame delta/duration variables first
  SetRenderRefreshRate( environmentOptions.GetRenderRefreshRate() );

  // Set the thread-synchronization interface on the render-surface
  Dali::RenderSurfaceInterface* currentSurface = mAdaptorInterfaces.GetRenderSurfaceInterface();
  if( currentSurface )
  {
    currentSurface->SetThreadSynchronization( *this );
  }

  TriggerEventFactoryInterface& triggerFactory = mAdaptorInterfaces.GetTriggerEventFactoryInterface();
  mSleepTrigger = triggerFactory.CreateTriggerEvent( MakeCallback( this, &CombinedUpdateRenderController::ProcessSleepRequest ), TriggerEventInterface::KEEP_ALIVE_AFTER_TRIGGER );

  sem_init( &mEventThreadSemaphore, 0, 0 ); // Initialize to 0 so that it just waits if sem_post has not been called
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
  DALI_ASSERT_ALWAYS( ! mUpdateRenderThread );

  // Create Update/Render Thread
  mUpdateRenderThread = new pthread_t();
  int error = pthread_create( mUpdateRenderThread, NULL, InternalUpdateRenderThreadEntryFunc, this );
  DALI_ASSERT_ALWAYS( !error && "Return code from pthread_create() when creating UpdateRenderThread" );

  // The Update/Render thread will now run and initialise the graphics interface etc. and will then wait for Start to be called
  // When this function returns, the application initialisation on the event thread should occur
}

void CombinedUpdateRenderController::Start()
{
  LOG_EVENT_TRACE;

  DALI_ASSERT_ALWAYS( !mRunning && mUpdateRenderThread );

  // Wait until all threads created in Initialise are up and running
  for( unsigned int i = 0; i < CREATED_THREAD_COUNT; ++i )
  {
    sem_wait( &mEventThreadSemaphore );
  }

  Integration::RenderSurface* currentSurface = mAdaptorInterfaces.GetRenderSurfaceInterface();
  if( currentSurface )
  {
    currentSurface->StartRender();
  }

  mRunning = TRUE;

  LOG_EVENT( "Startup Complete, starting Update/Render Thread" );

  RunUpdateRenderThread( CONTINUOUS, AnimationProgression::NONE, UpdateMode::NORMAL );

  DALI_LOG_RELEASE_INFO( "CombinedUpdateRenderController::Start\n" );
}

void CombinedUpdateRenderController::Pause()
{
  LOG_EVENT_TRACE;

  mRunning = FALSE;

  PauseUpdateRenderThread();

  AddPerformanceMarker( PerformanceInterface::PAUSED );

  DALI_LOG_RELEASE_INFO( "CombinedUpdateRenderController::Pause\n" );
}

void CombinedUpdateRenderController::Resume()
{
  LOG_EVENT_TRACE;

  if( !mRunning && IsUpdateRenderThreadPaused() )
  {
    LOG_EVENT( "Resuming" );

    RunUpdateRenderThread( CONTINUOUS, AnimationProgression::USE_ELAPSED_TIME, UpdateMode::NORMAL );

    AddPerformanceMarker( PerformanceInterface::RESUME );

    mRunning = TRUE;
    mForceClear = TRUE;
    mFirstFrameAfterResume = TRUE;

    DALI_LOG_RELEASE_INFO( "CombinedUpdateRenderController::Resume\n" );
  }
  else
  {
    DALI_LOG_RELEASE_INFO( "CombinedUpdateRenderController::Resume: Already resumed [%d, %d, %d]\n", mRunning, mUpdateRenderRunCount, mUpdateRenderThreadCanSleep );
  }
}

void CombinedUpdateRenderController::Stop()
{
  LOG_EVENT_TRACE;

  // Stop Rendering and the Update/Render Thread
  Integration::RenderSurface* currentSurface = mAdaptorInterfaces.GetRenderSurfaceInterface();
  if( currentSurface )
  {
    currentSurface->StopRender();
  }

  StopUpdateRenderThread();

  if( mUpdateRenderThread )
  {
    LOG_EVENT( "Destroying UpdateRenderThread" );

    // wait for the thread to finish
    pthread_join( *mUpdateRenderThread, NULL );

    delete mUpdateRenderThread;
    mUpdateRenderThread = NULL;
  }

  mRunning = FALSE;

  DALI_LOG_RELEASE_INFO( "CombinedUpdateRenderController::Stop\n" );
}

void CombinedUpdateRenderController::RequestUpdate()
{
  LOG_EVENT_TRACE;

  // Increment the update-request count to the maximum
  if( mUpdateRequestCount < MAXIMUM_UPDATE_REQUESTS )
  {
    ++mUpdateRequestCount;
  }

  if( mRunning && IsUpdateRenderThreadPaused() )
  {
    LOG_EVENT( "Processing" );

    RunUpdateRenderThread( CONTINUOUS, AnimationProgression::NONE, UpdateMode::NORMAL );
  }

  ConditionalWait::ScopedLock updateLock( mUpdateRenderThreadWaitCondition );
  mPendingRequestUpdate = TRUE;
}

void CombinedUpdateRenderController::RequestUpdateOnce( UpdateMode updateMode )
{
  // Increment the update-request count to the maximum
  if( mUpdateRequestCount < MAXIMUM_UPDATE_REQUESTS )
  {
    ++mUpdateRequestCount;
  }

  if( IsUpdateRenderThreadPaused() )
  {
    LOG_EVENT_TRACE;

    // Run Update/Render once
    RunUpdateRenderThread( ONCE, AnimationProgression::NONE, updateMode );
  }
}

void CombinedUpdateRenderController::ReplaceSurface( Dali::RenderSurfaceInterface* newSurface )
{
  LOG_EVENT_TRACE;

  if( mUpdateRenderThread )
  {
    // Set the ThreadSyncronizationInterface on the new surface
    newSurface->SetThreadSynchronization( *this );

    LOG_EVENT( "Starting to replace the surface, event-thread blocked" );

    // Start replacing the surface.
    {
      ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );
      mPostRendering = FALSE; // Clear the post-rendering flag as Update/Render thread will replace the surface now
      mNewSurface = newSurface;
      mUpdateRenderThreadWaitCondition.Notify( lock );
    }

    // Wait until the surface has been replaced
    sem_wait( &mEventThreadSemaphore );

    LOG_EVENT( "Surface replaced, event-thread continuing" );
  }
}

void CombinedUpdateRenderController::DeleteSurface( Dali::RenderSurfaceInterface* surface )
{
  LOG_EVENT_TRACE;

  if( mUpdateRenderThread )
  {
    LOG_EVENT( "Starting to delete the surface, event-thread blocked" );

    // Start replacing the surface.
    {
      ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );
      mPostRendering = FALSE; // Clear the post-rendering flag as Update/Render thread will delete the surface now
      mDeletedSurface = surface;
      mUpdateRenderThreadWaitCondition.Notify( lock );
    }

    // Wait until the surface has been deleted
    sem_wait( &mEventThreadSemaphore );

    LOG_EVENT( "Surface deleted, event-thread continuing" );
  }
}

void CombinedUpdateRenderController::ResizeSurface()
{
  LOG_EVENT_TRACE;

  LOG_EVENT( "Resize the surface" );

  {
    ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );
    mPostRendering = FALSE; // Clear the post-rendering flag as Update/Render thread will resize the surface now
    mSurfaceResized = TRUE;
    mUpdateRenderThreadWaitCondition.Notify( lock );
  }
}

void CombinedUpdateRenderController::SetRenderRefreshRate( unsigned int numberOfFramesPerRender )
{
  // Not protected by lock, but written to rarely so not worth adding a lock when reading
  mDefaultFrameDelta                  = numberOfFramesPerRender * DEFAULT_FRAME_DURATION_IN_SECONDS;
  mDefaultFrameDurationMilliseconds   = uint64_t( numberOfFramesPerRender ) * DEFAULT_FRAME_DURATION_IN_MILLISECONDS;
  mDefaultFrameDurationNanoseconds    = uint64_t( numberOfFramesPerRender ) * DEFAULT_FRAME_DURATION_IN_NANOSECONDS;
  mDefaultHalfFrameNanoseconds        = mDefaultFrameDurationNanoseconds / 2u;

  LOG_EVENT( "mDefaultFrameDelta(%.6f), mDefaultFrameDurationMilliseconds(%lld), mDefaultFrameDurationNanoseconds(%lld)", mDefaultFrameDelta, mDefaultFrameDurationMilliseconds, mDefaultFrameDurationNanoseconds );
}

void CombinedUpdateRenderController::SetPreRenderCallback( CallbackBase* callback )
{
  LOG_EVENT_TRACE;
  LOG_EVENT( "Set PreRender Callback" );

  ConditionalWait::ScopedLock updateLock( mUpdateRenderThreadWaitCondition );
  if( mPreRenderCallback )
  {
    delete mPreRenderCallback;
  }
  mPreRenderCallback = callback;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// EVENT THREAD
///////////////////////////////////////////////////////////////////////////////////////////////////

void CombinedUpdateRenderController::RunUpdateRenderThread( int numberOfCycles, AnimationProgression animationProgression, UpdateMode updateMode )
{
  ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );
  mUpdateRenderRunCount = numberOfCycles;
  mUpdateRenderThreadCanSleep = FALSE;
  mUseElapsedTimeAfterWait = ( animationProgression == AnimationProgression::USE_ELAPSED_TIME );
  mUploadWithoutRendering = ( updateMode == UpdateMode::SKIP_RENDER );
  LOG_COUNTER_EVENT( "mUpdateRenderRunCount: %d, mUseElapsedTimeAfterWait: %d", mUpdateRenderRunCount, mUseElapsedTimeAfterWait );
  mUpdateRenderThreadWaitCondition.Notify( lock );
}

void CombinedUpdateRenderController::PauseUpdateRenderThread()
{
  ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );
  mUpdateRenderRunCount = 0;
}

void CombinedUpdateRenderController::StopUpdateRenderThread()
{
  ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );
  mDestroyUpdateRenderThread = TRUE;
  mUpdateRenderThreadWaitCondition.Notify( lock );
}

bool CombinedUpdateRenderController::IsUpdateRenderThreadPaused()
{
  ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );
  return ( mUpdateRenderRunCount != CONTINUOUS ) || // Report paused if NOT continuously running
         mUpdateRenderThreadCanSleep;               // Report paused if sleeping
}

void CombinedUpdateRenderController::ProcessSleepRequest()
{
  LOG_EVENT_TRACE;

  // Decrement Update request count
  if( mUpdateRequestCount > 0 )
  {
    --mUpdateRequestCount;
  }

  // Can sleep if our update-request count is 0
  // Update/Render thread can choose to carry on updating if it determines more update/renders are required
  if( mUpdateRequestCount == 0 )
  {
    LOG_EVENT( "Going to sleep" );

    ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );
    mUpdateRenderThreadCanSleep = TRUE;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// UPDATE/RENDER THREAD
///////////////////////////////////////////////////////////////////////////////////////////////////

void CombinedUpdateRenderController::UpdateRenderThread()
{
  SetThreadName("RenderThread\0");

  // Install a function for logging
  mEnvironmentOptions.InstallLogFunction();

  // Install a function for tracing
  mEnvironmentOptions.InstallTraceFunction();

  LOG_UPDATE_RENDER( "THREAD CREATED" );

  // Initialize EGL & OpenGL
  Dali::DisplayConnection& displayConnection = mAdaptorInterfaces.GetDisplayConnectionInterface();
  displayConnection.Initialize();

  RenderSurfaceInterface* currentSurface = nullptr;

  GraphicsInterface& graphics = mAdaptorInterfaces.GetGraphicsInterface();
  EglGraphics* eglGraphics = static_cast<EglGraphics *>(&graphics);

  // This will only be created once
  EglInterface* eglInterface = &eglGraphics->GetEglInterface();

  Internal::Adaptor::EglImplementation& eglImpl = static_cast<Internal::Adaptor::EglImplementation&>( *eglInterface );

  // Try to use OpenGL es 3.0
  // ChooseConfig returns false here when the device only support gles 2.0.
  // Because eglChooseConfig with gles 3.0 setting fails when the device only support gles 2.0 and Our default setting is gles 3.0.
  if( !eglImpl.ChooseConfig( true, COLOR_DEPTH_32 ) )
  {
    // Retry to use OpenGL es 2.0
    eglGraphics->SetGlesVersion( 20 );
    eglImpl.ChooseConfig( true, COLOR_DEPTH_32 );
  }

  // Check whether surfaceless context is supported
  bool isSurfacelessContextSupported = eglImpl.IsSurfacelessContextSupported();
  eglGraphics->SetIsSurfacelessContextSupported( isSurfacelessContextSupported );

  if ( isSurfacelessContextSupported )
  {
    // Create a surfaceless OpenGL context for shared resources
    eglImpl.CreateContext();
    eglImpl.MakeContextCurrent( EGL_NO_SURFACE, eglImpl.GetContext() );
  }
  else
  {
    currentSurface = mAdaptorInterfaces.GetRenderSurfaceInterface();
    if( currentSurface )
    {
      currentSurface->InitializeGraphics();
      currentSurface->MakeContextCurrent();
    }
  }

  eglGraphics->GetGlesInterface().ContextCreated();

  // Tell core it has a context
  mCore.ContextCreated();

  NotifyThreadInitialised();

  // Update time
  uint64_t lastFrameTime;
  TimeService::GetNanoseconds( lastFrameTime );

  LOG_UPDATE_RENDER( "THREAD INITIALISED" );

  bool useElapsedTime = true;
  bool updateRequired = true;
  uint64_t timeToSleepUntil = 0;
  int extraFramesDropped = 0;

  const unsigned int renderToFboInterval = mEnvironmentOptions.GetRenderToFboInterval();
  const bool renderToFboEnabled = 0u != renderToFboInterval;
  unsigned int frameCount = 0u;

  while( UpdateRenderReady( useElapsedTime, updateRequired, timeToSleepUntil ) )
  {
    LOG_UPDATE_RENDER_TRACE;

    // Performance statistics are logged upon a VSYNC tick so use this point for a VSync marker
    AddPerformanceMarker( PerformanceInterface::VSYNC );

    uint64_t currentFrameStartTime = 0;
    TimeService::GetNanoseconds( currentFrameStartTime );

    const uint64_t timeSinceLastFrame = currentFrameStartTime - lastFrameTime;

    // Optional FPS Tracking when continuously rendering
    if( useElapsedTime && mFpsTracker.Enabled() )
    {
      float absoluteTimeSinceLastRender = timeSinceLastFrame * NANOSECONDS_TO_SECOND;
      mFpsTracker.Track( absoluteTimeSinceLastRender );
    }

    lastFrameTime = currentFrameStartTime; // Store frame start time

    //////////////////////////////
    // REPLACE SURFACE
    //////////////////////////////

    Integration::RenderSurface* newSurface = ShouldSurfaceBeReplaced();
    if( DALI_UNLIKELY( newSurface ) )
    {
      LOG_UPDATE_RENDER_TRACE_FMT( "Replacing Surface" );

      // This is designed for replacing pixmap surfaces, but should work for window as well
      // we need to delete the surface and renderable (pixmap / window)
      // Then create a new pixmap/window and new surface
      // If the new surface has a different display connection, then the context will be lost

      mAdaptorInterfaces.GetDisplayConnectionInterface().Initialize();
      newSurface->InitializeGraphics();
      newSurface->ReplaceGraphicsSurface();
      SurfaceReplaced();
    }

    const bool isRenderingToFbo = renderToFboEnabled && ( ( 0u == frameCount ) || ( 0u != frameCount % renderToFboInterval ) );
    ++frameCount;

    //////////////////////////////
    // UPDATE
    //////////////////////////////

    const unsigned int currentTime = currentFrameStartTime / NANOSECONDS_PER_MILLISECOND;
    const unsigned int nextFrameTime = currentTime + mDefaultFrameDurationMilliseconds;

    uint64_t noOfFramesSinceLastUpdate = 1;
    float frameDelta = 0.0f;
    if( useElapsedTime )
    {
      // If using the elapsed time, then calculate frameDelta as a multiple of mDefaultFrameDelta
      noOfFramesSinceLastUpdate += extraFramesDropped;

      frameDelta = mDefaultFrameDelta * noOfFramesSinceLastUpdate;
    }
    LOG_UPDATE_RENDER( "timeSinceLastFrame(%llu) noOfFramesSinceLastUpdate(%u) frameDelta(%.6f)", timeSinceLastFrame, noOfFramesSinceLastUpdate, frameDelta );

    Integration::UpdateStatus updateStatus;

    AddPerformanceMarker( PerformanceInterface::UPDATE_START );
    mCore.Update( frameDelta,
                  currentTime,
                  nextFrameTime,
                  updateStatus,
                  renderToFboEnabled,
                  isRenderingToFbo );
    AddPerformanceMarker( PerformanceInterface::UPDATE_END );

    unsigned int keepUpdatingStatus = updateStatus.KeepUpdating();

    // Tell the event-thread to wake up (if asleep) and send a notification event to Core if required
    if( updateStatus.NeedsNotification() )
    {
      mNotificationTrigger.Trigger();
      LOG_UPDATE_RENDER( "Notification Triggered" );
    }

    // Check resize
    bool shouldSurfaceBeResized = ShouldSurfaceBeResized();
    if( DALI_UNLIKELY( shouldSurfaceBeResized ) )
    {
      if( updateStatus.SurfaceRectChanged() )
      {
        LOG_UPDATE_RENDER_TRACE_FMT( "Resizing Surface" );
        SurfaceResized();
      }
    }

    // Optional logging of update/render status
    mUpdateStatusLogger.Log( keepUpdatingStatus );

    //////////////////////////////
    // RENDER
    //////////////////////////////

    mAdaptorInterfaces.GetDisplayConnectionInterface().ConsumeEvents();

    if( mPreRenderCallback != NULL )
    {
      bool keepCallback = CallbackBase::ExecuteReturn<bool>(*mPreRenderCallback);
      if( ! keepCallback )
      {
        delete mPreRenderCallback;
        mPreRenderCallback = NULL;
      }
    }

    if( eglImpl.IsSurfacelessContextSupported() )
    {
      // Make the shared surfaceless context as current before rendering
      eglImpl.MakeContextCurrent( EGL_NO_SURFACE, eglImpl.GetContext() );
    }

    if( mFirstFrameAfterResume )
    {
      // mFirstFrameAfterResume is set to true when the thread is resumed
      // Let eglImplementation know the first frame after thread initialized or resumed.
      eglImpl.SetFirstFrameAfterResume();
      mFirstFrameAfterResume = FALSE;
    }

    Integration::RenderStatus renderStatus;

    AddPerformanceMarker( PerformanceInterface::RENDER_START );
    mCore.Render( renderStatus, mForceClear, mUploadWithoutRendering );

    //////////////////////////////
    // DELETE SURFACE
    //////////////////////////////

    Integration::RenderSurface* deletedSurface = ShouldSurfaceBeDeleted();
    if( DALI_UNLIKELY( deletedSurface ) )
    {
      LOG_UPDATE_RENDER_TRACE_FMT( "Deleting Surface" );

      mCore.SurfaceDeleted( deletedSurface );

      SurfaceDeleted();
    }

    AddPerformanceMarker( PerformanceInterface::RENDER_END );

    mForceClear = false;

    // Trigger event thread to request Update/Render thread to sleep if update not required
    if( ( Integration::KeepUpdating::NOT_REQUESTED == keepUpdatingStatus ) && !renderStatus.NeedsUpdate() )
    {
      mSleepTrigger->Trigger();
      updateRequired = false;
      LOG_UPDATE_RENDER( "Sleep Triggered" );
    }
    else
    {
      updateRequired = true;
    }

    //////////////////////////////
    // FRAME TIME
    //////////////////////////////

    extraFramesDropped = 0;

    if (timeToSleepUntil == 0)
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
      TimeService::GetNanoseconds( currentFrameEndTime );
      while ( currentFrameEndTime > timeToSleepUntil + mDefaultFrameDurationNanoseconds )
      {
         // We are more than one frame behind already, so just drop the next frames
         // until the sleep-until time is later than the current time so that we can
         // catch up.
         timeToSleepUntil += mDefaultFrameDurationNanoseconds;
         extraFramesDropped++;
      }
    }

    // Render to FBO is intended to measure fps above 60 so sleep is not wanted.
    if( 0u == renderToFboInterval )
    {
      // Sleep until at least the the default frame duration has elapsed. This will return immediately if the specified end-time has already passed.
      TimeService::SleepUntil( timeToSleepUntil );
    }
  }

  // Inform core of context destruction & shutdown EGL
  mCore.ContextDestroyed();
  currentSurface = mAdaptorInterfaces.GetRenderSurfaceInterface();
  if( currentSurface )
  {
    currentSurface->DestroySurface();
    currentSurface = nullptr;
  }

  LOG_UPDATE_RENDER( "THREAD DESTROYED" );

  // Uninstall the logging function
  mEnvironmentOptions.UnInstallLogFunction();
}

bool CombinedUpdateRenderController::UpdateRenderReady( bool& useElapsedTime, bool updateRequired, uint64_t& timeToSleepUntil )
{
  useElapsedTime = true;

  ConditionalWait::ScopedLock updateLock( mUpdateRenderThreadWaitCondition );
  while( ( ! mUpdateRenderRunCount || // Should try to wait if event-thread has paused the Update/Render thread
           ( mUpdateRenderThreadCanSleep && ! updateRequired && ! mPendingRequestUpdate ) ) && // Ensure we wait if we're supposed to be sleeping AND do not require another update
         ! mDestroyUpdateRenderThread && // Ensure we don't wait if the update-render-thread is supposed to be destroyed
         ! mNewSurface &&  // Ensure we don't wait if we need to replace the surface
         ! mDeletedSurface && // Ensure we don't wait if we need to delete the surface
         ! mSurfaceResized ) // Ensure we don't wait if we need to resize the surface
  {
    LOG_UPDATE_RENDER( "WAIT: mUpdateRenderRunCount:       %d", mUpdateRenderRunCount );
    LOG_UPDATE_RENDER( "      mUpdateRenderThreadCanSleep: %d, updateRequired: %d, mPendingRequestUpdate: %d", mUpdateRenderThreadCanSleep, updateRequired, mPendingRequestUpdate );
    LOG_UPDATE_RENDER( "      mDestroyUpdateRenderThread:  %d", mDestroyUpdateRenderThread );
    LOG_UPDATE_RENDER( "      mNewSurface:                 %d", mNewSurface );
    LOG_UPDATE_RENDER( "      mDeletedSurface:             %d", mDeletedSurface );
    LOG_UPDATE_RENDER( "      mSurfaceResized:             %d", mSurfaceResized );

    // Reset the time when the thread is waiting, so the sleep-until time for
    // the first frame after resuming should be based on the actual start time
    // of the first frame.
    timeToSleepUntil = 0;

    mUpdateRenderThreadWaitCondition.Wait( updateLock );

    if( ! mUseElapsedTimeAfterWait )
    {
      useElapsedTime = false;
    }
  }

  LOG_COUNTER_UPDATE_RENDER( "mUpdateRenderRunCount:       %d", mUpdateRenderRunCount );
  LOG_COUNTER_UPDATE_RENDER( "mUpdateRenderThreadCanSleep: %d, updateRequired: %d, mPendingRequestUpdate: %d", mUpdateRenderThreadCanSleep, updateRequired, mPendingRequestUpdate );
  LOG_COUNTER_UPDATE_RENDER( "mDestroyUpdateRenderThread:  %d", mDestroyUpdateRenderThread );
  LOG_COUNTER_UPDATE_RENDER( "mNewSurface:                 %d", mNewSurface );
  LOG_COUNTER_UPDATE_RENDER( "mDeletedSurface:             %d", mDeletedSurface );
  LOG_COUNTER_UPDATE_RENDER( "mSurfaceResized:             %d", mSurfaceResized );

  mUseElapsedTimeAfterWait = FALSE;
  mUpdateRenderThreadCanSleep = FALSE;
  mPendingRequestUpdate = FALSE;

  // If we've been asked to run Update/Render cycles a finite number of times then decrement so we wait after the
  // requested number of cycles
  if( mUpdateRenderRunCount > 0 )
  {
    --mUpdateRenderRunCount;
  }

  // Keep the update-render thread alive if this thread is NOT to be destroyed
  return ! mDestroyUpdateRenderThread;
}

Integration::RenderSurface* CombinedUpdateRenderController::ShouldSurfaceBeReplaced()
{
  ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );

  Integration::RenderSurface* newSurface = mNewSurface;
  mNewSurface = NULL;

  return newSurface;
}

void CombinedUpdateRenderController::SurfaceReplaced()
{
  // Just increment the semaphore
  sem_post( &mEventThreadSemaphore );
}

Integration::RenderSurface* CombinedUpdateRenderController::ShouldSurfaceBeDeleted()
{
  ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );

  Integration::RenderSurface* deletedSurface = mDeletedSurface;
  mDeletedSurface = NULL;

  return deletedSurface;
}

void CombinedUpdateRenderController::SurfaceDeleted()
{
  // Just increment the semaphore
  sem_post( &mEventThreadSemaphore );
}

bool CombinedUpdateRenderController::ShouldSurfaceBeResized()
{
  ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );
  return mSurfaceResized;
}

void CombinedUpdateRenderController::SurfaceResized()
{
  ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );
  mSurfaceResized = FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ALL THREADS
///////////////////////////////////////////////////////////////////////////////////////////////////

void CombinedUpdateRenderController::NotifyThreadInitialised()
{
  // Just increment the semaphore
  sem_post( &mEventThreadSemaphore );
}

void CombinedUpdateRenderController::AddPerformanceMarker( PerformanceInterface::MarkerType type )
{
  if( mPerformanceInterface )
  {
    mPerformanceInterface->AddMarker( type );
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// POST RENDERING: EVENT THREAD
/////////////////////////////////////////////////////////////////////////////////////////////////

void CombinedUpdateRenderController::PostRenderComplete()
{
  ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );
  mPostRendering = FALSE;
  mUpdateRenderThreadWaitCondition.Notify( lock );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// POST RENDERING: RENDER THREAD
///////////////////////////////////////////////////////////////////////////////////////////////////

void CombinedUpdateRenderController::PostRenderStarted()
{
  ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );
  mPostRendering = TRUE;
}

void CombinedUpdateRenderController::PostRenderWaitForCompletion()
{
  ConditionalWait::ScopedLock lock( mUpdateRenderThreadWaitCondition );
  while( mPostRendering &&
         ! mNewSurface &&                // We should NOT wait if we're replacing the surface
         ! mDeletedSurface &&            // We should NOT wait if we're deleting the surface
         ! mSurfaceResized &&            // We should NOT wait if we're resizing the surface
         ! mDestroyUpdateRenderThread )
  {
    mUpdateRenderThreadWaitCondition.Wait( lock );
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
