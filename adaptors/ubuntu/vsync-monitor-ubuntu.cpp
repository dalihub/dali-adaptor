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
#include "vsync-monitor.h"

// EXTERNAL INCLUDES
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{

#if defined(DEBUG_ENABLED)
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VSYNC_MONITOR");
#endif

const int FD_NONE( -1 );

} // unnamed namespace

VSyncMonitor::VSyncMonitor()
: mFileDescriptor( FD_NONE ),
  mUseHardwareVSync( false ),
  mHardwareVsyncAvailable( false )
{
}

VSyncMonitor::~VSyncMonitor()
{
  Terminate();
}

void VSyncMonitor::SetUseHardwareVsync( bool useHardware )
{
  mUseHardwareVSync = useHardware;
}

void VSyncMonitor::SetHardwareVSyncAvailable( bool hardwareVSyncAvailable )
{
  mHardwareVsyncAvailable = hardwareVSyncAvailable;
}

void VSyncMonitor::Initialize()
{
  DALI_ASSERT_DEBUG( mFileDescriptor == FD_NONE && "VSyncMonitor::Initialize() called twice" );

  // setup vblank request - block and wait for next vblank
  mVBlankInfo.request.type = DRM_VBLANK_NEXTONMISS;
  mVBlankInfo.request.sequence = 0;
  mVBlankInfo.request.signal = 0;

  // setup vblank reply - block and wait for next vblank
  mVBlankInfo.reply.type = DRM_VBLANK_NEXTONMISS;
  mVBlankInfo.reply.sequence = 0;
  mVBlankInfo.reply.tval_sec = 0;
  mVBlankInfo.reply.tval_usec = 0;
}

void VSyncMonitor::Terminate()
{
}

bool VSyncMonitor::UseHardware()
{
  return mUseHardware && (FD_NONE != mFileDescriptor );
}


bool VSyncMonitor::DoSync( unsigned int& frameNumber, unsigned int& seconds, unsigned int& microseconds )
{
  DALI_ASSERT_DEBUG( mFileDescriptor != FD_NONE && "ECoreX::VSyncMonitor is not initialized" );

  if( 0 == drmWaitVBlank( mFileDescriptor, &mVBlankInfo ) )
  {
    frameNumber = mVBlankInfo.reply.sequence;
    seconds = mVBlankInfo.reply.tval_sec;
    microseconds = mVBlankInfo.reply.tval_usec;

    return true;
  }

  return false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
