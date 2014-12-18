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
#include "performance-server.h"

// INTERNAL INCLUDES
#include <base/environment-options.h>
#include <dali/integration-api/platform-abstraction.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

#define TIME_FMT "%0.2f ms" // 2 decimal places, e.g. 5.34 ms
#define TOTAL_TIME_FMT "%0.1f secs" // 1 decimal place, e.g. 4.5 seconds


namespace
{

const unsigned int DEFAULT_LOG_FREQUENCEY = 2;        ///< default log frequency = 2
const unsigned int MILLISECONDS_PER_SECOND = 1000;    ///< 1000 milliseconds per second
const unsigned int MICROSECONDS_PER_SECOND = 1000000; ///< 1000000 microseconds per second

const char* UPDATE_CONTEXT_NAME = "Update";
const char* RENDER_CONTEXT_NAME = "Render";
const char* EVENT_CONTEXT_NAME = "Event";

} // Anonymous namespace


PerformanceServer::PerformanceServer( AdaptorInternalServices& adaptorServices,
                                      const EnvironmentOptions& environmentOptions)
:mPlatformAbstraction( adaptorServices.GetPlatformAbstractionInterface() ),
 mEnvironmentOptions( environmentOptions ),
 mKernelTrace( adaptorServices.GetKernelTraceInterface() ),
 mLogLevel( 0 ),
 mNextContextId( 0 ),
 mLoggingEnabled( false ),
 mLogFunctionInstalled( false )
{
  // Add defaults
  mUpdateStats = AddContext( UPDATE_CONTEXT_NAME );
  mRenderStats = AddContext( RENDER_CONTEXT_NAME );
  mEventStats = AddContext( EVENT_CONTEXT_NAME );

  SetLogging( mEnvironmentOptions.GetPerformanceLoggingLevel(), mEnvironmentOptions.GetFrameRateLoggingFrequency());
}

PerformanceServer::~PerformanceServer()
{
  if( mLogFunctionInstalled )
  {
    mEnvironmentOptions.UnInstallLogFunction();
  }

  for( StatContexts::Iterator it = mStatContexts.Begin(), itEnd = mStatContexts.End(); it != itEnd; ++it )
  {
    StatContext* context = *it;

    delete context;
  }
}

void PerformanceServer::SetLogging( unsigned int level, unsigned int logFrequency)
{
  if( level == 0 )
  {
    mLoggingEnabled = false;
    return;
  }
  mLogLevel = level;

  EnableLogging( mLogLevel & LOG_UPDATE_RENDER, mUpdateStats );
  EnableLogging( mLogLevel & LOG_UPDATE_RENDER, mRenderStats );
  EnableLogging( mLogLevel & LOG_EVENT_PROCESS, mEventStats );

  SetLoggingFrequency( logFrequency, mUpdateStats );
  SetLoggingFrequency( logFrequency, mRenderStats );
  SetLoggingFrequency( logFrequency, mEventStats );

  mLoggingEnabled = true;
}

void PerformanceServer::SetLoggingFrequency( unsigned int logFrequency, ContextId contextId )
{
  StatContext* context = GetContext( contextId );
  if( context )
  {
    unsigned int logFrequencyMicroseconds = logFrequency * MICROSECONDS_PER_SECOND;

    if( logFrequencyMicroseconds == 0 )
    {
      logFrequencyMicroseconds = DEFAULT_LOG_FREQUENCEY * MICROSECONDS_PER_SECOND;
    }

    context->SetLogFrequency( logFrequencyMicroseconds );
  }
}

void PerformanceServer::EnableLogging( bool enable, ContextId contextId )
{
  StatContext* context = GetContext( contextId );
  if( context )
  {
    context->EnableLogging( enable );
  }
}

PerformanceServer::StatContext::StatContext()
: mName( NULL ),
  mId( 0 ),
  mLogFrequencyMicroseconds( DEFAULT_LOG_FREQUENCEY * MICROSECONDS_PER_SECOND ),
  mLog( true )
{
}

PerformanceServer::StatContext::StatContext( unsigned int id, const char* contextName )
: mName( contextName ),
  mId( id ),
  mLogFrequencyMicroseconds( DEFAULT_LOG_FREQUENCEY * MICROSECONDS_PER_SECOND ),
  mLog( true )
{
}

PerformanceInterface::ContextId PerformanceServer::AddContext( const char* name )
{
  unsigned int contextId = mNextContextId++;

  DALI_ASSERT_DEBUG( !GetContext( contextId ) );

  mStatContexts.PushBack( new StatContext( contextId, name ) );

  return contextId;
}

void PerformanceServer::RemoveContext( ContextId contextId )
{
  for( StatContexts::Iterator it = mStatContexts.Begin(), itEnd = mStatContexts.End(); it != itEnd; ++it )
  {
    StatContext* context = *it;

    if( context->GetId() == contextId )
    {
      delete context;

      mStatContexts.Erase( it );
      break;
    }
  }
}

PerformanceServer::StatContext* PerformanceServer::GetContext( ContextId contextId )
{
  for( StatContexts::Iterator it = mStatContexts.Begin(), itEnd = mStatContexts.End(); it != itEnd; ++it )
  {
    StatContext* context = *it;

    if( context->GetId() == contextId )
    {
      return context;
    }
  }

  return NULL;
}

void PerformanceServer::AddMarker( MarkerType markerType )
{
  switch( markerType )
  {
    case VSYNC:
    {
      for( StatContexts::Iterator it = mStatContexts.Begin(), itEnd = mStatContexts.End(); it != itEnd; ++it )
      {
        StatContext* context = *it;

        AddMarker( VSYNC, context->GetId() );
      }
      break;
    }
    case UPDATE_START:
    {
      AddMarker( START, mUpdateStats );
      break;
    }
    case UPDATE_END:
    {
      AddMarker( END, mUpdateStats );
      break;
    }
    case RENDER_START:
    {
      AddMarker( START, mRenderStats );
      break;
    }
    case RENDER_END:
    {
      AddMarker( END, mRenderStats );
      break;
    }
    case PROCESS_EVENTS_START:
    {
      AddMarker( START, mEventStats );
      break;
    }
    case PROCESS_EVENTS_END:
    {
      AddMarker( END, mEventStats );
      break;
    }
    default:
    {
      break;
    }
  }
}

void PerformanceServer::AddMarker( MarkerType markerType, ContextId contextId )
{
  if( !mLoggingEnabled )
  {
    return;
  }

  // Get the time
  unsigned int seconds = 0;
  unsigned int microseconds = 0;
  mPlatformAbstraction.GetTimeMicroseconds( seconds, microseconds );

  // Create a marker and add it to the log
  PerformanceMarker marker( markerType, FrameTimeStamp( 0, seconds, microseconds ) );
  AddMarkerToLog( marker, contextId );
}

void PerformanceServer::AddMarkerToLog( const PerformanceMarker& marker, ContextId contextId )
{
  StatContext* context = GetContext( contextId );
  if( context )
  {
    if( mLogLevel & LOG_EVENTS_TO_KERNEL )
    {
      std::stringstream ss;
      ss << GetMarkerName( marker.GetType() ) << ":" << ( ( context->GetName() ) ? context->GetName() : "" );
      mKernelTrace.Trace( ss.str() );
    }

    // Only log on the v-sync thread, so we have less impact on update/render
    if( marker.GetType() == VSYNC )
    {
      // make sure log function is installed, note this will be called only from v-sync thread
      // if the v-sync thread has already installed one, it won't make any difference.
      if( !mLogFunctionInstalled )
      {
        mEnvironmentOptions.InstallLogFunction();
        mLogFunctionInstalled = true;
      }
    }

    context->LogMarker( marker );
  }
}

void PerformanceServer::StatContext::LogMarker()
{
  float mean, standardDeviation;
  mStats.CalculateMean( mean, standardDeviation );

  // this will always log regardless of debug / release mode
  Integration::Log::LogMessage( Dali::Integration::Log::DebugInfo,
                                    "%s, min " TIME_FMT ", max " TIME_FMT ", total (" TOTAL_TIME_FMT "), avg " TIME_FMT ", std dev " TIME_FMT "\n",
                                     ( mName ) ? mName : "",
                                     mStats.GetMinTime() * MILLISECONDS_PER_SECOND,
                                     mStats.GetMaxTime() * MILLISECONDS_PER_SECOND,
                                     mStats.GetTotalTime(),
                                     mean * MILLISECONDS_PER_SECOND,
                                     standardDeviation * MILLISECONDS_PER_SECOND);
}

void PerformanceServer::StatContext::TimeMarkers()
{
  // insert time stamps into a frame-time-stats object, based on type
  for( MarkerVector::SizeType i = 0; i < mMarkers.Size(); ++i)
  {
    const PerformanceMarker& marker = mMarkers[i];
    switch( marker.GetType() )
    {
      case START:
      {
        mStats.StartTime( marker.GetTimeStamp() );
        break;
      }
      case END:
      {
        mStats.EndTime( marker.GetTimeStamp() );
        break;
      }
      default:
      {
        break;
      }
    }
  }

  mMarkers.Clear();
}

const char* PerformanceServer::GetMarkerName( MarkerType type ) const
{
  switch( type )
  {
    case VSYNC:
    {
      return "VSYNC";
    }

    case START:
    {
      return "START";
    }

    case END:
    {
      return "END";
    }

    default:
    {
      DALI_ASSERT_DEBUG( !"Unsupported MarkerType" );
      return "";
    }
  }
}

void PerformanceServer::StatContext::LogMarker( const PerformanceMarker& marker )
{
  // Add Marker can be called from any thread
  boost::mutex::scoped_lock sharedDatalock( mDataMutex );

  mMarkers.PushBack( marker );

  // Only log on the v-sync thread, so we have less impact on update/render
  if( marker.GetType() == VSYNC )
  {
    // log out every mLogFrequency.
    // check difference between first and last frame
    unsigned int microseconds = PerformanceMarker::MicrosecondDiff( mMarkers[0], marker );
    if( microseconds >= mLogFrequencyMicroseconds )
    {
      TimeMarkers();

      if( mLog )
      {
        LogMarker();
      }

      mStats.Reset();     // reset data for statistics
    }
  }
}

void PerformanceServer::StatContext::SetLogFrequency( unsigned int frequencyMicroseconds )
{
  mLogFrequencyMicroseconds = frequencyMicroseconds;
}

void PerformanceServer::StatContext::EnableLogging( bool enableLogging )
{
  mLog = enableLogging;
}

const char* PerformanceServer::StatContext::GetName() const
{
  return mName;
}

unsigned int PerformanceServer::StatContext::GetId() const
{
  return mId;
}

} // namespace Internal

} // namespace Adaptor

} // namespace Dali
