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

EnvironmentOptions::EnvironmentOptions()
: mFpsFrequency(0),
  mUpdateStatusFrequency(0),
  mPerformanceLoggingLevel(0),
  mPanGestureLoggingLevel(0),
  mPanGesturePredictionMode(-1),
  mPanGesturePredictionAmount(-1.0f), ///< only sets value in pan gesture if greater than 0
  mPanGestureSmoothingMode(-1),
  mPanGestureSmoothingAmount(-1.0f),
  mPanMinimumDistance(-1),
  mPanMinimumEvents(-1),
  mGlesCallTime(0),
  mLogFunction( NULL )
{
}

EnvironmentOptions::~EnvironmentOptions()
{
}

void EnvironmentOptions::SetLogOptions( const Dali::Integration::Log::LogFunction& logFunction,
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

unsigned int EnvironmentOptions::GetPerformanceLoggingLevel() const
{
  return mPerformanceLoggingLevel;
}

unsigned int EnvironmentOptions::GetPanGestureLoggingLevel() const
{
  return mPanGestureLoggingLevel;
}

int EnvironmentOptions::GetPanGesturePredictionMode() const
{
  return mPanGesturePredictionMode;
}

float EnvironmentOptions::GetPanGesturePredictionAmount() const
{
  return mPanGesturePredictionAmount;
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

void EnvironmentOptions::SetPanGesturePredictionMode( unsigned int mode )
{
  mPanGesturePredictionMode = mode;
}

void EnvironmentOptions::SetPanGesturePredictionAmount( unsigned int amount )
{
  mPanGesturePredictionAmount = amount;
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

int EnvironmentOptions::GetGlesCallTime()
{
  return mGlesCallTime;
}

} // Adaptor

} // Internal

} // Dali
