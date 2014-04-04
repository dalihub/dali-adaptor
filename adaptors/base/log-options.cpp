//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// CLASS HEADER
#include "log-options.h"

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

LogOptions::LogOptions()
: mFpsFrequency(0),
  mUpdateStatusFrequency(0),
  mPerformanceLoggingLevel(0),
  mPanGestureLoggingLevel(0),
  mLogFunction( NULL )
{
}

LogOptions::~LogOptions()
{
}

void LogOptions::SetOptions( const Dali::Integration::Log::LogFunction& logFunction,
                             unsigned int logFrameRateFrequency,
                             unsigned int logupdateStatusFrequency,
                             unsigned int logPerformanceLevel,
                             unsigned int logPanGestureLevel )
{
  mLogFunction = logFunction;
  mFpsFrequency = logFrameRateFrequency;
  mUpdateStatusFrequency = logupdateStatusFrequency;
  mPerformanceLoggingLevel = logPerformanceLevel;
  mPanGestureLoggingLevel = logPanGestureLevel;
}

void LogOptions::InstallLogFunction() const
{
  Dali::Integration::Log::InstallLogFunction( mLogFunction );
}

void LogOptions::UnInstallLogFunction() const
{
  Dali::Integration::Log::UninstallLogFunction();
}

unsigned int LogOptions::GetFrameRateLoggingFrequency() const
{
  return mFpsFrequency;
}

unsigned int LogOptions::GetUpdateStatusLoggingFrequency() const
{
  return mUpdateStatusFrequency;
}

unsigned int LogOptions::GetPerformanceLoggingLevel() const
{
  return mPerformanceLoggingLevel;
}

unsigned int LogOptions::GetPanGestureLoggingLevel() const
{
  return mPanGestureLoggingLevel;
}

} // Adaptor

} // Internal

} // Dali
