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
#include <dali/internal/system/common/fps-tracker.h>

// EXTERNAL INCLUDES
#include <cstdio>
#include <cmath>
#include <sys/stat.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/environment-options.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
const char* DALI_TEMP_UPDATE_FPS_FILE( "/tmp/dalifps.txt" );
} // unnamed namespace

FpsTracker::FpsTracker( const EnvironmentOptions& environmentOptions )
: mFpsTrackingSeconds( fabsf( environmentOptions.GetFrameRateLoggingFrequency() ) ),
  mFrameCount( 0.0f ),
  mElapsedTime( 0.0f )
{
}

FpsTracker::~FpsTracker()
{
  if( mFpsTrackingSeconds > 0.f )
  {
    OutputFPSRecord();
  }
}

void FpsTracker::Track( float secondsFromLastFrame )
{
  if( mFpsTrackingSeconds > 0.f )
  {
    if ( mElapsedTime < mFpsTrackingSeconds )
    {
      mElapsedTime += secondsFromLastFrame;
      mFrameCount += 1.f;
    }
    else
    {
      OutputFPSRecord();
      mFrameCount = 0.f;
      mElapsedTime = 0.f;
    }
  }
}

bool FpsTracker::Enabled() const
{
  return mFpsTrackingSeconds > 0.0f;
}

void FpsTracker::OutputFPSRecord()
{
  float fps = mFrameCount / mElapsedTime;
  DALI_LOG_FPS("Frame count %.0f, elapsed time %.1fs, FPS: %.2f\n", mFrameCount, mElapsedTime, fps );

  struct stat fileStat;

  // Check file path
  if( lstat( DALI_TEMP_UPDATE_FPS_FILE, &fileStat ) != 0 )
  {
    return;
  }

  if( !S_ISREG( fileStat.st_mode ) )
  {
    return;
  }

  // Dumps out the frame rate.
  FILE* outfile = fopen( DALI_TEMP_UPDATE_FPS_FILE, "w" );
  if( outfile )
  {
    char fpsString[10];
    snprintf(fpsString,sizeof(fpsString),"%.2f \n", fps );
    fputs( fpsString, outfile ); // ignore the error on purpose
    fclose( outfile );
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
