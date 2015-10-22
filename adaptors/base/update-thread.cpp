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
#include "update-thread.h"

// EXTERNAL INCLUDES
#include <cstdio>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <base/interfaces/adaptor-internal-services.h>
#include <base/thread-synchronization.h>
#include <base/environment-options.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
#if defined(DEBUG_ENABLED)
Integration::Log::Filter* gUpdateLogFilter = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_UPDATE_THREAD");
#endif
} // unnamed namespace

UpdateThread::UpdateThread( ThreadSynchronization& sync,
                            AdaptorInternalServices& adaptorInterfaces,
                            const EnvironmentOptions& environmentOptions )
: mThreadSynchronization( sync ),
  mCore( adaptorInterfaces.GetCore()),
  mFpsTracker( environmentOptions ),
  mUpdateStatusLogger( environmentOptions ),
  mThread( NULL ),
  mEnvironmentOptions( environmentOptions )
{
}

UpdateThread::~UpdateThread()
{
  Stop();
}

void UpdateThread::Start()
{
  DALI_LOG_INFO( gUpdateLogFilter, Debug::Verbose, "UpdateThread::Start()\n");
  if ( !mThread )
  {
    // Create and run the update-thread
    mThread =  new pthread_t();
    int error = pthread_create( mThread, NULL, InternalThreadEntryFunc, this );
    DALI_ASSERT_ALWAYS( !error && "Return code from pthread_create() in UpdateThread" );
  }
}

void UpdateThread::Stop()
{
  DALI_LOG_INFO( gUpdateLogFilter, Debug::Verbose, "UpdateThread::Stop()\n");
  if( mThread )
  {
    // wait for the thread to finish
    pthread_join(*mThread, NULL);

    delete mThread;
    mThread = NULL;
  }
}

bool UpdateThread::Run()
{
  DALI_LOG_INFO( gUpdateLogFilter, Debug::Verbose, "UpdateThread::Run()\n");

  // Install a function for logging
  mEnvironmentOptions.InstallLogFunction();

  Integration::UpdateStatus status;
  bool runUpdate = true;
  float lastFrameDelta( 0.0f );
  unsigned int lastSyncTime( 0 );
  unsigned int nextSyncTime( 0 );

  // Update loop, we stay inside here while the update-thread is running
  // We also get the last delta and the predict when this update will be rendered
  while ( mThreadSynchronization.UpdateReady( status.NeedsNotification(), runUpdate, lastFrameDelta, lastSyncTime, nextSyncTime ) )
  {
    DALI_LOG_INFO( gUpdateLogFilter, Debug::Verbose, "UpdateThread::Run. 1 - UpdateReady(delta:%f, lastSync:%u, nextSync:%u)\n", lastFrameDelta, lastSyncTime, nextSyncTime);

    DALI_LOG_INFO( gUpdateLogFilter, Debug::Verbose, "UpdateThread::Run. 2 - Core.Update()\n");

    mThreadSynchronization.AddPerformanceMarker( PerformanceInterface::UPDATE_START );
    mCore.Update( lastFrameDelta, lastSyncTime, nextSyncTime, status );
    mThreadSynchronization.AddPerformanceMarker( PerformanceInterface::UPDATE_END );

    mFpsTracker.Track( status.SecondsFromLastFrame() );

    unsigned int keepUpdatingStatus = status.KeepUpdating();

    // Optional logging of update/render status
    mUpdateStatusLogger.Log( keepUpdatingStatus );

    //  2 things can keep update running.
    // - The status of the last update
    // - The status of the last render
    runUpdate = (Integration::KeepUpdating::NOT_REQUESTED != keepUpdatingStatus);

    DALI_LOG_INFO( gUpdateLogFilter, Debug::Verbose, "UpdateThread::Run. 3 - runUpdate(%d)\n", runUpdate );

    // Reset time variables
    lastFrameDelta = 0.0f;
    lastSyncTime = 0;
    nextSyncTime = 0;
  }

  // Uninstall the logging function
  mEnvironmentOptions.UnInstallLogFunction();

  return true;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
