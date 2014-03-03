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

LogOptions::LogOptions(  )
: mLogOpts(0),
  mFpsFrequency(0),
  mUpdateStatusFrequency(0),
  mPerformanceLoggingLevel(0),
  mLogFunction( NULL )
{
}

LogOptions::~LogOptions()
{

}

void LogOptions::SetOptions( const Dali::Integration::Log::LogFunction& logFunction,
                             unsigned int logFilterOptions,
                             unsigned int logFrameRateFrequency,
                             unsigned int logupdateStatusFrequency,
                             unsigned int logPerformanceLevel )
{
  mLogFunction = logFunction;
  mLogOpts = logFilterOptions;
  mFpsFrequency = logFrameRateFrequency;
  mUpdateStatusFrequency = logupdateStatusFrequency;
  mPerformanceLoggingLevel = logPerformanceLevel;
}

void LogOptions::InstallLogFunction() const
{
  Dali::Integration::Log::InstallLogFunction( mLogFunction, mLogOpts );
}

void LogOptions::UnInstallLogFunction() const
{
  Dali::Integration::Log::UninstallLogFunction();
}

bool LogOptions::IsFilterEnabled( Dali::Integration::Log::LoggingOptions filter ) const
{
  return (mLogOpts & filter);
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


} // Adaptor
} // Internal
} // Dali
