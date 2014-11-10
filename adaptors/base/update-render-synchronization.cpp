/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include "update-render-synchronization.h"

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <base/interfaces/adaptor-internal-services.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
const unsigned int TIME_PER_FRAME_IN_MICROSECONDS = 16667;
const unsigned int MICROSECONDS_PER_SECOND( 1000000 );
const unsigned int INPUT_EVENT_UPDATE_PERIOD( MICROSECONDS_PER_SECOND / 90 ); // period between ecore x event updates

} // unnamed namespace

UpdateRenderSynchronization::UpdateRenderSynchronization( AdaptorInternalServices& adaptorInterfaces,
                                                          unsigned int numberOfVSyncsPerRender)
: mMaximumUpdateCount( adaptorInterfaces.GetCore().GetMaximumUpdateCount()),
  mNumberOfVSyncsPerRender( numberOfVSyncsPerRender ),
  mUpdateReadyCount( 0u ),
  mRunning( false ),
  mUpdateRequired( false ),
  mPaused( false ),
  mUpdateRequested( false ),
  mAllowUpdateWhilePaused( false ),
  mVSyncSleep( false ),
  mSyncFrameNumber( 0u ),
  mSyncSeconds( 0u ),
  mSyncMicroseconds( 0u ),
  mFrameTime( adaptorInterfaces.GetPlatformAbstractionInterface() ),
  mPerformanceInterface( adaptorInterfaces.GetPerformanceInterface() ),
  mReplaceSurfaceRequest(),
  mReplaceSurfaceRequested( false )
{
}

UpdateRenderSynchronization::~UpdateRenderSynchronization()
{
}

void UpdateRenderSynchronization::Start()
{
  mFrameTime.SetMinimumFrameTimeInterval( mNumberOfVSyncsPerRender * TIME_PER_FRAME_IN_MICROSECONDS );
  mRunning = true;
}

void UpdateRenderSynchronization::Stop()
{
  mRunning = false;

  // Wake if sleeping
  UpdateRequested();

  // we may be paused so need to resume
  Resume();

  // Notify all condition variables, so if threads are waiting
  // they can break out, and check the running status.
  mUpdateFinishedCondition.notify_one();
  mRenderFinishedCondition.notify_one();
  mVSyncSleepCondition.notify_one();
  mVSyncReceivedCondition.notify_one();

  mFrameTime.Suspend();
}

void UpdateRenderSynchronization::Pause()
{
  mPaused = true;

  AddPerformanceMarker( PerformanceInterface::PAUSED );
  mFrameTime.Suspend();
}

void UpdateRenderSynchronization::ResumeFrameTime()
{
  mFrameTime.Resume();
}

void UpdateRenderSynchronization::Resume()
{
  mPaused = false;
  mVSyncSleep = false;

  mPausedCondition.notify_one();
  mVSyncSleepCondition.notify_one();

  AddPerformanceMarker( PerformanceInterface::RESUME);
}

void UpdateRenderSynchronization::UpdateRequested()
{
  mUpdateRequested = true;

  // Wake update thread if sleeping
  mUpdateSleepCondition.notify_one();
}

void UpdateRenderSynchronization::UpdateWhilePaused()
{
  {
    boost::unique_lock< boost::mutex > lock( mMutex );

    mAllowUpdateWhilePaused = true;
  }

  // wake vsync if sleeping
  mVSyncSleepCondition.notify_one();
  // Wake update if sleeping
  mUpdateSleepCondition.notify_one();
  // stay paused but notify the pause condition
  mPausedCondition.notify_one();
}

bool UpdateRenderSynchronization::ReplaceSurface( RenderSurface* newSurface )
{
  bool result=false;

  UpdateRequested();
  UpdateWhilePaused();
  {
    boost::unique_lock< boost::mutex > lock( mMutex );

    mReplaceSurfaceRequest.SetSurface(newSurface);
    mReplaceSurfaceRequested = true;

    mRequestFinishedCondition.wait(lock); // wait unlocks the mutex on entry, and locks again on exit.

    mReplaceSurfaceRequested = false;
    result = mReplaceSurfaceRequest.GetReplaceCompleted();
  }

  return result;
}

void UpdateRenderSynchronization::UpdateReadyToRun()
{
  bool wokenFromPause( false );

  // atomic check first to avoid mutex lock in 99.99% of cases
  if( mPaused )
  {
    boost::unique_lock< boost::mutex > lock( mMutex );

    // wait while paused
    while( mPaused && !mAllowUpdateWhilePaused )
    {
      // this will automatically unlock mMutex
      mPausedCondition.wait( lock );

      wokenFromPause = true;
    }
  }

  if ( !wokenFromPause )
  {
    // Wait for the next Sync
    WaitSync();
  }

  AddPerformanceMarker( PerformanceInterface::UPDATE_START );
}

bool UpdateRenderSynchronization::UpdateSyncWithRender( bool& renderNeedsUpdate )
{

  AddPerformanceMarker( PerformanceInterface::UPDATE_END );

  boost::unique_lock< boost::mutex > lock( mMutex );

  // Another frame was prepared for rendering; increment counter
  ++mUpdateReadyCount;
  DALI_ASSERT_DEBUG( mUpdateReadyCount <= mMaximumUpdateCount );

  // Notify the render-thread that an update has completed
  mUpdateFinishedCondition.notify_one();

  // The update-thread must wait until a frame has been rendered, when mMaximumUpdateCount is reached
  while( mRunning && ( mMaximumUpdateCount == mUpdateReadyCount ) )
  {
    // Wait will atomically add the thread to the set of threads waiting on
    // the condition variable mRenderFinishedCondition and unlock the mutex.
    mRenderFinishedCondition.wait( lock );
  }

  renderNeedsUpdate = mUpdateRequired;

  // Flag is used to during UpdateThread::Stop() to exit the update/render loops
  return mRunning;
}

void UpdateRenderSynchronization::UpdateWaitForAllRenderingToFinish()
{
  boost::unique_lock< boost::mutex > lock( mMutex );

  // Wait for all of the prepared frames to be rendered
  while ( mRunning && ( 0u != mUpdateReadyCount ) && !mUpdateRequested )
  {
    // Wait will atomically add the thread to the set of threads waiting on
    // the condition variable mRenderFinishedCondition and unlock the mutex.
    mRenderFinishedCondition.wait( lock );
  }
}

bool UpdateRenderSynchronization::UpdateTryToSleep()
{
  if ( !mUpdateRequired && !mUpdateRequested )
  {
    // there's nothing to update in the scene, so wait for render to finish processing
    UpdateWaitForAllRenderingToFinish();
  }

  boost::mutex sleepMutex;
  boost::unique_lock< boost::mutex > lock( sleepMutex );

  while( mRunning && !mUpdateRequired && !mUpdateRequested )
  {
    //
    // Going to sleep
    //

    // 1. put VSync thread to sleep.
    mVSyncSleep = true;

    // 2. inform frame time
    mFrameTime.Sleep();

    // 3. block thread and wait for wakeup event
    mUpdateSleepCondition.wait( lock );

    //
    // Woken up
    //

    // 1. inform frame timer
    mFrameTime.WakeUp();

    // 2. wake VSync thread.
    mVSyncSleep = false;
    mVSyncSleepCondition.notify_one();
  }

  mUpdateRequested = false;

  return mRunning;
}

bool UpdateRenderSynchronization::RenderSyncWithUpdate(RenderRequest*& requestPtr)
{
  boost::unique_lock< boost::mutex > lock( mMutex );

  // Wait for update to produce a buffer, or for the mRunning state to change
  while ( mRunning && ( 0u == mUpdateReadyCount ) )
  {
    // Wait will atomically add the thread to the set of threads waiting on
    // the condition variable mUpdateFinishedCondition and unlock the mutex.
    mUpdateFinishedCondition.wait( lock );
  }

  if( mRunning )
  {
    AddPerformanceMarker( PerformanceInterface::RENDER_START );
  }

  // write any new requests
  if( mReplaceSurfaceRequested )
  {
    requestPtr = &mReplaceSurfaceRequest;
  }
  mReplaceSurfaceRequested = false;

  // Flag is used to during UpdateThread::Stop() to exit the update/render loops
  return mRunning;
}

void UpdateRenderSynchronization::RenderFinished( bool updateRequired, bool requestProcessed )
{
  {
    boost::unique_lock< boost::mutex > lock( mMutex );

    // Set the flag to say if update needs to run again.
    mUpdateRequired = updateRequired;

    // A frame has been rendered; decrement counter
    --mUpdateReadyCount;
    DALI_ASSERT_DEBUG( mUpdateReadyCount < mMaximumUpdateCount );
  }

  // Notify the update-thread that a render has completed
  mRenderFinishedCondition.notify_one();

  if( requestProcessed )
  {
    // Notify the event thread that a request has completed
    mRequestFinishedCondition.notify_one();
  }

  AddPerformanceMarker( PerformanceInterface::RENDER_END );
}

void UpdateRenderSynchronization::WaitSync()
{
  // Block until the start of a new sync.
  // If we're experiencing slowdown and are behind by more than a frame
  // then we should wait for the next frame

  unsigned int updateFrameNumber = mSyncFrameNumber;

  boost::unique_lock< boost::mutex > lock( mMutex );

  while ( mRunning && ( updateFrameNumber == mSyncFrameNumber ) )
  {
    // Wait will atomically add the thread to the set of threads waiting on
    // the condition variable mVSyncReceivedCondition and unlock the mutex.
    mVSyncReceivedCondition.wait( lock );
  }

  // reset update while paused flag
  mAllowUpdateWhilePaused = false;
}

bool UpdateRenderSynchronization::VSyncNotifierSyncWithUpdateAndRender( bool validSync, unsigned int frameNumber, unsigned int seconds, unsigned int microseconds, unsigned int& numberOfVSyncsPerRender )
{
  // This may have changed since the last sync. Update VSyncNotifier's copy here if so.
  if( numberOfVSyncsPerRender != mNumberOfVSyncsPerRender )
  {
    numberOfVSyncsPerRender = mNumberOfVSyncsPerRender; // save it back
    mFrameTime.SetMinimumFrameTimeInterval( mNumberOfVSyncsPerRender * TIME_PER_FRAME_IN_MICROSECONDS );
  }

  if( validSync )
  {
    mFrameTime.SetSyncTime( frameNumber );
  }

  boost::unique_lock< boost::mutex > lock( mMutex );

  mSyncFrameNumber = frameNumber;
  mSyncSeconds = seconds;
  mSyncMicroseconds = microseconds;

  mVSyncReceivedCondition.notify_all();

  AddPerformanceMarker( PerformanceInterface::VSYNC );

  while( mRunning && // sleep on condition variable WHILE still running
         !mAllowUpdateWhilePaused &&             // AND NOT allowing updates while paused
         ( mVSyncSleep || mPaused ) )            // AND sleeping OR paused
  {
    // Wait will atomically add the thread to the set of threads waiting on
    // the condition variable mVSyncSleepCondition and unlock the mutex.
    mVSyncSleepCondition.wait( lock );
  }

  return mRunning;
}

unsigned int UpdateRenderSynchronization::GetFrameNumber() const
{
  return mSyncFrameNumber;
}

uint64_t UpdateRenderSynchronization::GetTimeMicroseconds()
{
  uint64_t currentTime(0);

  {
    boost::unique_lock< boost::mutex > lock( mMutex );

    currentTime = mSyncSeconds;
    currentTime *= MICROSECONDS_PER_SECOND;
    currentTime += mSyncMicroseconds;
  }

  return currentTime;
}

void UpdateRenderSynchronization::SetRenderRefreshRate( unsigned int numberOfVSyncsPerRender )
{
  mNumberOfVSyncsPerRender = numberOfVSyncsPerRender;
}

inline void UpdateRenderSynchronization::AddPerformanceMarker( PerformanceInterface::MarkerType type )
{
  if( mPerformanceInterface )
  {
    mPerformanceInterface->AddMarker( type );
  }
}

void UpdateRenderSynchronization::PredictNextSyncTime(
  float& lastFrameDeltaSeconds,
  unsigned int& lastSyncTimeMilliseconds,
  unsigned int& nextSyncTimeMilliseconds )
{
  mFrameTime.PredictNextSyncTime( lastFrameDeltaSeconds, lastSyncTimeMilliseconds, nextSyncTimeMilliseconds );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
