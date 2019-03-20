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
#include <dali/internal/graphics/common/vsync-monitor.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
// constants to keep code readability with unsigned int has to be used as boolean (due to multithreading)
const unsigned int TRUE = 1u;
const unsigned int FALSE = 0u;

const int FD_NONE( -1 );

} // unnamed namespace

VSyncMonitor::VSyncMonitor()
: mFileDescriptor( FD_NONE ),
  mUseHardwareVSync( FALSE ),
  mHardwareVSyncAvailable( FALSE )
{
}

VSyncMonitor::~VSyncMonitor()
{
  Terminate();
}

void VSyncMonitor::SetUseHardwareVSync( bool useHardware )
{
  mUseHardwareVSync = useHardware;
}

void VSyncMonitor::SetHardwareVSyncAvailable( bool hardwareVSyncAvailable )
{
  mHardwareVSyncAvailable = hardwareVSyncAvailable;
}

void VSyncMonitor::Initialize()
{
}

void VSyncMonitor::Terminate()
{
}

bool VSyncMonitor::UseHardware()
{
  return mUseHardwareVSync && mHardwareVSyncAvailable && (FD_NONE != mFileDescriptor );
}


bool VSyncMonitor::DoSync( unsigned int& frameNumber, unsigned int& seconds, unsigned int& microseconds )
{
  return false;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
