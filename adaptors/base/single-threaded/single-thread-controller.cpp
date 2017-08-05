/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include "single-thread-controller.h"

// EXTERNAL INCLUDES
#include <iostream>
#include <dali/integration-api/platform-abstraction.h>

// INTERNAL INCLUDES
#include <base/interfaces/adaptor-internal-services.h>
#include <base/environment-options.h>
#include <base/time-service.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
const unsigned int MILLISECONDS_PER_FRAME = 17u;
const float SECONDS_PER_FRAME = MILLISECONDS_PER_FRAME * 0.001f;

const unsigned int NANOSECONDS_PER_MICROSECOND( 1000u );
const float        NANOSECONDS_TO_SECONDS( 1e-9f );

#if defined(DEBUG_ENABLED)
Integration::Log::Filter* gLogFilter = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_THREAD_SYNC");
#endif

} // unnamed namespace

SingleThreadController::SingleThreadController( AdaptorInternalServices& adaptorInterfaces, const EnvironmentOptions& environmentOptions )
: ConnectionTracker(),
  ThreadControllerInterface(),
  mTimer(),
  mFpsTracker( environmentOptions ),
  mUpdateStatusLogger( environmentOptions ),
  mRenderHelper( adaptorInterfaces ),
  mCore( adaptorInterfaces.GetCore()),
  mPerformanceInterface( adaptorInterfaces.GetPerformanceInterface() ),
  mLastUpdateRenderTime( 0 ),
  mSystemTime( 0 ),
  mRefreshRate( environmentOptions.GetRenderRefreshRate() ),
  mState( State::STOPPED ),
  mUpdatingAndRendering( false ),
  mStopRequestedWhileRendering( false )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "%s()\n", __FUNCTION__ );
}

SingleThreadController::~SingleThreadController()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "%s()\n", __FUNCTION__ );

  Stop();
}

void SingleThreadController::Initialize()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "%s()\n", __FUNCTION__ );

  mTimer = Dali::Timer::New( mRefreshRate * MILLISECONDS_PER_FRAME );

  // Create a tick-signal so that we can update and render every frame
  mTimer.TickSignal().Connect( this, &SingleThreadController::OnTimerTick );
}

void SingleThreadController::Start()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "%s()\n", __FUNCTION__ );

  mRenderHelper.Start();
  mRenderHelper.InitializeEgl();

  // tell core it has a context
  mCore.ContextCreated();

  // Do an update/render straight away
  UpdateTimeSinceLastRender();
  UpdateRender( false );

  ChangeState( State::RUNNING );
}

void SingleThreadController::Pause()
{
  if( mState == State::RUNNING )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "%s()\n", __FUNCTION__ );

    ChangeState( State::PAUSED );

    AddPerformanceMarker( PerformanceInterface::PAUSED );
  }
}

void SingleThreadController::Resume()
{
  if( mState == State::PAUSED )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "%s()\n", __FUNCTION__ );

    // Do an update/render straight away
    UpdateTimeSinceLastRender();
    UpdateRender( false );

    ChangeState( State::RUNNING );

    AddPerformanceMarker( PerformanceInterface::RESUME );
  }
}

void SingleThreadController::Stop()
{
  if( mState != State::STOPPED )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "%s()\n", __FUNCTION__ );

    ChangeState( State::STOPPED );

    if( mUpdatingAndRendering )
    {
      // If we interrupted an update/render for this stop, then we should NOT terminate GL just yet
      mStopRequestedWhileRendering = true;
    }
    else
    {
      StopRendering();
    }
  }
}

void SingleThreadController::RequestUpdate()
{
  if( mState == State::SLEEPING )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "%s()\n", __FUNCTION__ );

    // Do an update/render straight away
    UpdateTimeSinceLastRender();
    UpdateRender( false );

    ChangeState( State::RUNNING );
  }
}

void SingleThreadController::RequestUpdateOnce()
{
  if( mState == State::SLEEPING || mState == State::PAUSED )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "%s()\n", __FUNCTION__ );

    // Just do one update and render

    Integration::UpdateStatus status;
    mCore.Update( 0.0f, mLastUpdateRenderTime, mLastUpdateRenderTime + mRefreshRate * MILLISECONDS_PER_FRAME, status );

    Integration::RenderStatus renderStatus;
    mRenderHelper.PreRender();
    mCore.Render( renderStatus );
    mRenderHelper.PostRender();
  }
}

void SingleThreadController::ReplaceSurface( RenderSurface* newSurface )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "%s()\n", __FUNCTION__ );
  mRenderHelper.ReplaceSurface( newSurface );
}

void SingleThreadController::ResizeSurface()
{
  DALI_LOG_ERROR( "SingleThreadController::ResizeSurface: Not supported\n" );
}

void SingleThreadController::SetRenderRefreshRate( unsigned int refreshRate )
{
  if ( refreshRate != mRefreshRate )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "%s()\n", __FUNCTION__ );

    mRefreshRate = refreshRate;

    if( mTimer )
    {
      mTimer.SetInterval( mRefreshRate * MILLISECONDS_PER_FRAME );
    }
  }
}

bool SingleThreadController::OnTimerTick()
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "%s()\n", __FUNCTION__ );

  if( mState == State::RUNNING )
  {
    UpdateRender( true );
  }
  else if( mState == State::STOPPED &&
           mStopRequestedWhileRendering )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "%s(): STOPPING\n", __FUNCTION__ );

    StopRendering();

    mStopRequestedWhileRendering = false;

    return false; // Stop the timer
  }
  return true;
}

void SingleThreadController::UpdateRender( bool incrementTime )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "%s():START\n", __FUNCTION__ );

  mUpdatingAndRendering = true;

  float lastFrameDelta( 0.0f );

  if( incrementTime )
  {
    // Use our usual time per frame for smoother animations rather than the real elapsed time

    lastFrameDelta = mRefreshRate * SECONDS_PER_FRAME;
    mLastUpdateRenderTime += mRefreshRate * MILLISECONDS_PER_FRAME;
  }

  Integration::UpdateStatus updateStatus;
  AddPerformanceMarker( PerformanceInterface::UPDATE_START );
  mCore.Update( lastFrameDelta, mLastUpdateRenderTime, mLastUpdateRenderTime + mRefreshRate * MILLISECONDS_PER_FRAME, updateStatus );
  AddPerformanceMarker( PerformanceInterface::UPDATE_END );

  mFpsTracker.Track( UpdateTimeSinceLastRender() );

  unsigned int keepUpdatingStatus = updateStatus.KeepUpdating();

  // Optional logging of update/render status
  mUpdateStatusLogger.Log( keepUpdatingStatus );

  // Ensure we did not get interrupted an STOPPED
  if( mState != State::STOPPED )
  {
    mRenderHelper.ConsumeEvents();
    mRenderHelper.PreRender();

    Integration::RenderStatus renderStatus;
    AddPerformanceMarker( PerformanceInterface::RENDER_START );
    mCore.Render( renderStatus );
    AddPerformanceMarker( PerformanceInterface::RENDER_END );

    mRenderHelper.PostRender();

    if( ! keepUpdatingStatus &&
        ! renderStatus.NeedsUpdate() )
    {
      ChangeState( State::SLEEPING );
    }
  }

  mUpdatingAndRendering = false;

  DALI_LOG_INFO( gLogFilter, Debug::General, "%s():END\n", __FUNCTION__ );
}

float SingleThreadController::UpdateTimeSinceLastRender()
{
  float timeSinceLastRender = 0.0f;

  // No need calculating if FPS tracking is NOT enabled
  if( mFpsTracker.Enabled() )
  {
    uint64_t currentTime = 0;
    TimeService::GetNanoseconds( currentTime );

    uint64_t delta = currentTime - mSystemTime;
    mSystemTime = currentTime;

    timeSinceLastRender = delta * NANOSECONDS_TO_SECONDS;
  }

  return timeSinceLastRender;
}


void SingleThreadController::AddPerformanceMarker( PerformanceInterface::MarkerType type )
{
  if( mPerformanceInterface )
  {
    mPerformanceInterface->AddMarker( type );
  }
}

void SingleThreadController::ChangeState( State::Type state )
{
  mState = state;

  switch( state )
  {
    case State::RUNNING:
    {
      mTimer.Start();
      break;
    }

    case State::STOPPED:
    case State::PAUSED:
    case State::SLEEPING:
    {
      mTimer.Stop();
    }
  }
}

void SingleThreadController::StopRendering()
{
  mRenderHelper.Stop();

  // Inform core of context destruction & shutdown EGL
  mCore.ContextDestroyed();
  mRenderHelper.ShutdownEgl();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
