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
#include "update-thread.h"

// EXTERNAL INCLUDES
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <boost/thread.hpp>

// INTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <base/interfaces/adaptor-internal-services.h>
#include <base/update-render-synchronization.h>
#include <base/environment-options.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
const unsigned int MICROSECONDS_PER_MILLISECOND( 1000 );

#if defined(DEBUG_ENABLED)
Integration::Log::Filter* gUpdateLogFilter = Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_UPDATE_THREAD");
#endif
} // unnamed namespace

UpdateThread::UpdateThread( UpdateRenderSynchronization& sync,
                            AdaptorInternalServices& adaptorInterfaces,
                            const EnvironmentOptions& environmentOptions )
: mUpdateRenderSync( sync ),
  mCore( adaptorInterfaces.GetCore()),
  mFpsTrackingSeconds( environmentOptions.GetFrameRateLoggingFrequency() ),
  mElapsedTime( 0.0f ),
  mElapsedSeconds( 0u ),
  mStatusLogInterval( environmentOptions.GetUpdateStatusLoggingFrequency() ),
  mStatusLogCount( 0u ),
  mNotificationTrigger( adaptorInterfaces.GetTriggerEventInterface() ),
  mThread( NULL ),
  mEnvironmentOptions( environmentOptions )
{
  if( mFpsTrackingSeconds > 0 )
  {
    mFpsRecord.resize( mFpsTrackingSeconds, 0.0f );
  }
}

UpdateThread::~UpdateThread()
{
  if(mFpsTrackingSeconds > 0)
  {
    OutputFPSRecord();
  }
  Stop();
}

void UpdateThread::Start()
{
  DALI_LOG_INFO( gUpdateLogFilter, Debug::Verbose, "UpdateThread::Start()\n");
  if ( !mThread )
  {
    // Create and run the update-thread
    mThread = new boost::thread( boost::bind( &UpdateThread::Run, this ) );
  }
}

void UpdateThread::Stop()
{
  DALI_LOG_INFO( gUpdateLogFilter, Debug::Verbose, "UpdateThread::Stop()\n");
  if( mThread )
  {
    // wait for the thread to finish
    mThread->join();

    delete mThread;
    mThread = NULL;
  }
}

bool UpdateThread::Run()
{
  DALI_LOG_INFO( gUpdateLogFilter, Debug::Verbose, "UpdateThread::Run()\n");
  Integration::UpdateStatus status;

  // install a function for logging
  mEnvironmentOptions.InstallLogFunction();

  bool running( true );

  // Update loop, we stay inside here while the update-thread is running
  while ( running )
  {
    DALI_LOG_INFO( gUpdateLogFilter, Debug::Verbose, "UpdateThread::Run. 1 - Sync()\n");

    // Inform synchronization object update is ready to run, this will pause update thread if required.
    mUpdateRenderSync.UpdateReadyToRun();
    DALI_LOG_INFO( gUpdateLogFilter, Debug::Verbose, "UpdateThread::Run. 2 - Ready()\n");

    // get the last delta and the predict when this update will be rendered
    float lastFrameDelta( 0.0f );
    unsigned int lastSyncTime( 0 );
    unsigned int nextSyncTime( 0 );
    mUpdateRenderSync.PredictNextSyncTime( lastFrameDelta, lastSyncTime, nextSyncTime );

    DALI_LOG_INFO( gUpdateLogFilter, Debug::Verbose, "UpdateThread::Run. 3 - Update(delta:%f, lastSync:%u, nextSync:%u)\n", lastFrameDelta, lastSyncTime, nextSyncTime);

    mCore.Update( lastFrameDelta, lastSyncTime, nextSyncTime, status );

    if( mFpsTrackingSeconds > 0 )
    {
      FPSTracking(status.SecondsFromLastFrame());
    }

    // Do the notifications first so the actor-thread can start processing them
    if( status.NeedsNotification() )
    {
      // Tell the event-thread to wake up (if asleep) and send a notification event to Core
      mNotificationTrigger.Trigger();
    }

    bool renderNeedsUpdate;

    // tell the synchronisation class that a buffer has been written to,
    // and to wait until there is a free buffer to write to
    running = mUpdateRenderSync.UpdateSyncWithRender( renderNeedsUpdate );
    DALI_LOG_INFO( gUpdateLogFilter, Debug::Verbose, "UpdateThread::Run. 4 - UpdateSyncWithRender complete\n");

    if( running )
    {
      unsigned int keepUpdatingStatus = status.KeepUpdating();

      // Optional logging of update/render status
      if ( mStatusLogInterval )
      {
        UpdateStatusLogging( keepUpdatingStatus, renderNeedsUpdate );
      }

      //  2 things can keep update running.
      // - The status of the last update
      // - The status of the last render
      bool runUpdate = (Integration::KeepUpdating::NOT_REQUESTED != keepUpdatingStatus) || renderNeedsUpdate;

      if( !runUpdate )
      {
        DALI_LOG_INFO( gUpdateLogFilter, Debug::Verbose, "UpdateThread::Run. 5 - Nothing to update, trying to sleep\n");

        running = mUpdateRenderSync.UpdateTryToSleep();
      }
    }
  }

  // uninstall a function for logging
  mEnvironmentOptions.UnInstallLogFunction();

  return true;
}

void UpdateThread::FPSTracking(float secondsFromLastFrame)
{
  if (mElapsedSeconds < mFpsTrackingSeconds)
  {
    mElapsedTime += secondsFromLastFrame;
    if( secondsFromLastFrame  > 1.0 )
    {
      int seconds = floor(mElapsedTime);
      mElapsedSeconds += seconds;
      mElapsedTime -= static_cast<float>(seconds);
    }
    else
    {
      if( mElapsedTime>=1.0f )
      {
        mElapsedTime -= 1.0f;
        mFpsRecord[mElapsedSeconds] += 1.0f - mElapsedTime/secondsFromLastFrame;
        mElapsedSeconds++;
        mFpsRecord[mElapsedSeconds] += mElapsedTime/secondsFromLastFrame;
      }
      else
      {
        mFpsRecord[mElapsedSeconds] += 1.0f;
      }
    }
  }
  else
  {
    OutputFPSRecord();
    mFpsRecord.clear();
    mFpsTrackingSeconds = 0;
  }
}

void UpdateThread::OutputFPSRecord()
{
  for(unsigned int i = 0; i < mElapsedSeconds; i++)
  {
    DALI_LOG_FPS("fps( %d ):%f\n",i ,mFpsRecord[i]);
  }
  std::ofstream outFile("/tmp/dalifps.txt");
  if(outFile.is_open())
  {
    for(unsigned int i = 0; i < mElapsedSeconds; i++)
    {
      outFile << mFpsRecord[i]<<std::endl;
    }
    outFile.close();
  }
}

void UpdateThread::UpdateStatusLogging( unsigned int keepUpdatingStatus, bool renderNeedsUpdate )
{
  DALI_ASSERT_ALWAYS( mStatusLogInterval );

  std::ostringstream oss;

  if ( !(++mStatusLogCount % mStatusLogInterval) )
  {
    oss << "UpdateStatusLogging keepUpdating: " << keepUpdatingStatus << " ";

    if ( keepUpdatingStatus )
    {
      oss << "because: ";
    }

    if ( keepUpdatingStatus & Integration::KeepUpdating::STAGE_KEEP_RENDERING )
    {
      oss << "<Stage::KeepRendering() used> ";
    }

    if ( keepUpdatingStatus & Integration::KeepUpdating::INCOMING_MESSAGES )
    {
      oss << "<Messages sent to Update> ";
    }

    if ( keepUpdatingStatus & Integration::KeepUpdating::ANIMATIONS_RUNNING )
    {
      oss << "<Animations running> ";
    }

    if ( keepUpdatingStatus & Integration::KeepUpdating::DYNAMICS_CHANGED )
    {
      oss << "<Dynamics running> ";
    }

    if ( keepUpdatingStatus & Integration::KeepUpdating::LOADING_RESOURCES )
    {
      oss << "<Resources loading> ";
    }

    if ( keepUpdatingStatus & Integration::KeepUpdating::NOTIFICATIONS_PENDING )
    {
      oss << "<Notifications pending> ";
    }

    if ( keepUpdatingStatus & Integration::KeepUpdating::MONITORING_PERFORMANCE )
    {
      oss << "<Monitoring performance> ";
    }

    if ( keepUpdatingStatus & Integration::KeepUpdating::RENDER_TASK_SYNC )
    {
      oss << "<Render task waiting for completion> ";
    }

    if ( renderNeedsUpdate )
    {
      oss << "<Render needs Update> ";
    }

    DALI_LOG_UPDATE_STATUS( "%s\n", oss.str().c_str() );
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
