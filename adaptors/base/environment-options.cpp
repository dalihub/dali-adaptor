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
#include "environment-options.h"

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
const unsigned int DEFAULT_STATISTICS_LOG_FREQUENCY = 2;
}
EnvironmentOptions::EnvironmentOptions()
: mFpsFrequency(0),
  mUpdateStatusFrequency(0),
  mPerformanceStatsLevel(0),
  mPerformanceStatsFrequency( DEFAULT_STATISTICS_LOG_FREQUENCY),
  mPerformanceTimeStampOutput(0),
  mPanGestureLoggingLevel(0),
  mPanGesturePredictionMode(-1),
  mPanGesturePredictionAmount(-1), ///< only sets value in pan gesture if greater than 0
  mPanGestureMaxPredictionAmount(-1),
  mPanGestureMinPredictionAmount(-1),
  mPanGesturePredictionAmountAdjustment(-1),
  mPanGestureSmoothingMode(-1),
  mPanGestureSmoothingAmount(-1.0f),
  mPanMinimumDistance(-1),
  mPanMinimumEvents(-1),
  mGlesCallTime(0),
  mWindowWidth( 0 ),
  mWindowHeight( 0 ),
  mLogFunction( NULL )
{
}

EnvironmentOptions::~EnvironmentOptions()
{
}

void EnvironmentOptions::SetLogOptions( const Dali::Integration::Log::LogFunction& logFunction,
                             unsigned int logFrameRateFrequency,
                             unsigned int logupdateStatusFrequency,
                             unsigned int logPerformanceStats,
                             unsigned int logPerformanceStatsFrequency,
                             unsigned int performanceTimeStampOutput,
                             unsigned int logPanGestureLevel )
{
  mLogFunction = logFunction;
  mFpsFrequency = logFrameRateFrequency;
  mUpdateStatusFrequency = logupdateStatusFrequency;
  mPerformanceStatsLevel = logPerformanceStats;
  mPerformanceStatsFrequency = logPerformanceStatsFrequency;
  mPerformanceTimeStampOutput= performanceTimeStampOutput;
  mPanGestureLoggingLevel = logPanGestureLevel;
}

void EnvironmentOptions::InstallLogFunction() const
{
  Dali::Integration::Log::InstallLogFunction( mLogFunction );
}

void EnvironmentOptions::UnInstallLogFunction() const
{
  Dali::Integration::Log::UninstallLogFunction();
}

unsigned int EnvironmentOptions::GetFrameRateLoggingFrequency() const
{
  return mFpsFrequency;
}

unsigned int EnvironmentOptions::GetUpdateStatusLoggingFrequency() const
{
  return mUpdateStatusFrequency;
}

unsigned int EnvironmentOptions::GetPerformanceStatsLoggingOptions() const
{
  return mPerformanceStatsLevel;
}
unsigned int EnvironmentOptions::GetPerformanceStatsLoggingFrequency() const
{
  return mPerformanceStatsFrequency;
}
unsigned int EnvironmentOptions::GetPerformanceTimeStampOutput() const
{
  return mPerformanceTimeStampOutput;
}

unsigned int EnvironmentOptions::GetPanGestureLoggingLevel() const
{
  return mPanGestureLoggingLevel;
}

int EnvironmentOptions::GetPanGesturePredictionMode() const
{
  return mPanGesturePredictionMode;
}

int EnvironmentOptions::GetPanGesturePredictionAmount() const
{
  return mPanGesturePredictionAmount;
}

int EnvironmentOptions::GetPanGestureMaximumPredictionAmount() const
{
  return mPanGestureMaxPredictionAmount;
}

int EnvironmentOptions::GetPanGestureMinimumPredictionAmount() const
{
  return mPanGestureMinPredictionAmount;
}

int EnvironmentOptions::GetPanGesturePredictionAmountAdjustment() const
{
  return mPanGesturePredictionAmountAdjustment;
}

int EnvironmentOptions::GetPanGestureSmoothingMode() const
{
  return mPanGestureSmoothingMode;
}

float EnvironmentOptions::GetPanGestureSmoothingAmount() const
{
  return mPanGestureSmoothingAmount;
}

int EnvironmentOptions::GetMinimumPanDistance() const
{
  return mPanMinimumDistance;
}

int EnvironmentOptions::GetMinimumPanEvents() const
{
  return mPanMinimumEvents;
}

unsigned int EnvironmentOptions::GetWindowWidth() const
{
  return mWindowWidth;
}

unsigned int EnvironmentOptions::GetWindowHeight() const
{
  return mWindowHeight;
}

void EnvironmentOptions::SetPanGesturePredictionMode( unsigned int mode )
{
  mPanGesturePredictionMode = mode;
}

void EnvironmentOptions::SetPanGesturePredictionAmount( unsigned int amount )
{
  mPanGesturePredictionAmount = amount;
}

void EnvironmentOptions::SetPanGestureMaximumPredictionAmount( unsigned int amount )
{
  mPanGestureMaxPredictionAmount = amount;
}

void EnvironmentOptions::SetPanGestureMinimumPredictionAmount( unsigned int amount )
{
  mPanGestureMinPredictionAmount = amount;
}

void EnvironmentOptions::SetPanGesturePredictionAmountAdjustment( unsigned int amount )
{
  mPanGesturePredictionAmountAdjustment = amount;
}

void EnvironmentOptions::SetPanGestureSmoothingMode( unsigned int mode )
{
  mPanGestureSmoothingMode = mode;
}

void EnvironmentOptions::SetPanGestureSmoothingAmount( float amount )
{
  mPanGestureSmoothingAmount = amount;
}

void EnvironmentOptions::SetMinimumPanDistance( int distance )
{
  mPanMinimumDistance = distance;
}

void EnvironmentOptions::SetMinimumPanEvents( int events )
{
  mPanMinimumEvents = events;
}

void EnvironmentOptions::SetGlesCallTime( int time )
{
  mGlesCallTime = time;
}

int EnvironmentOptions::GetGlesCallTime() const
{
  return mGlesCallTime;
}

void EnvironmentOptions::SetWindowWidth( int width )
{
  mWindowWidth = width;
}

void EnvironmentOptions::SetWindowHeight( int height )
{
  mWindowHeight = height;
}

bool EnvironmentOptions::PerformanceServerRequired() const
{
  return ( (GetPerformanceStatsLoggingOptions() > 0) ||
           ( GetPerformanceTimeStampOutput() > 0 ) );
}

} // Adaptor

} // Internal

} // Dali
