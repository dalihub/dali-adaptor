/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/common/environment-options.h>

// EXTERNAL INCLUDES
#include <cstdlib>
#include <dali/integration-api/render-controller.h>
#include <dali/public-api/math/math-utils.h>

// INTERNAL INCLUDES
#include <dali/internal/trace/common/trace-factory.h>
#include <dali/internal/system/common/environment-variables.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

namespace
{
const unsigned int DEFAULT_STATISTICS_LOG_FREQUENCY = 2;
const int DEFAULT_MULTI_SAMPLING_LEVEL = -1;
const bool DEFAULT_DEPTH_BUFFER_REQUIRED_SETTING = true;
const bool DEFAULT_STENCIL_BUFFER_REQUIRED_SETTING = true;

unsigned int GetIntegerEnvironmentVariable( const char* variable, unsigned int defaultValue )
{
  const char* variableParameter = std::getenv(variable);

  // if the parameter exists convert it to an integer, else return the default value
  unsigned int intValue = variableParameter ? std::atoi(variableParameter) : defaultValue;
  return intValue;
}

bool GetIntegerEnvironmentVariable( const char* variable, int& intValue )
{
  const char* variableParameter = std::getenv(variable);

  if( !variableParameter )
  {
    return false;
  }
  // if the parameter exists convert it to an integer, else return the default value
  intValue = std::atoi(variableParameter);
  return true;
}

bool GetFloatEnvironmentVariable( const char* variable, float& floatValue )
{
  const char* variableParameter = std::getenv(variable);

  if( !variableParameter )
  {
    return false;
  }
  // if the parameter exists convert it to an integer, else return the default value
  floatValue = std::atof(variableParameter);
  return true;
}

const char * GetCharEnvironmentVariable( const char * variable )
{
  return std::getenv( variable );
}

} // unnamed namespace

EnvironmentOptions::EnvironmentOptions()
: mLogFunction( NULL ),
  mWindowName(),
  mWindowClassName(),
  mNetworkControl( 0 ),
  mFpsFrequency( 0 ),
  mUpdateStatusFrequency( 0 ),
  mObjectProfilerInterval( 0 ),
  mPerformanceStatsLevel( 0 ),
  mPerformanceStatsFrequency( DEFAULT_STATISTICS_LOG_FREQUENCY ),
  mPerformanceTimeStampOutput( 0 ),
  mPanGestureLoggingLevel( 0 ),
  mWindowWidth( 0u ),
  mWindowHeight( 0u ),
  mRenderRefreshRate( 1u ),
  mMaxTextureSize( 0 ),
  mRenderToFboInterval( 0u ),
  mPanGesturePredictionMode( -1 ),
  mPanGesturePredictionAmount( -1 ), ///< only sets value in pan gesture if greater than 0
  mPanGestureMaxPredictionAmount( -1 ),
  mPanGestureMinPredictionAmount( -1 ),
  mPanGesturePredictionAmountAdjustment( -1 ),
  mPanGestureSmoothingMode( -1 ),
  mPanGestureSmoothingAmount( -1.0f ),
  mPanGestureUseActualTimes( -1 ),
  mPanGestureInterpolationTimeRange( -1 ),
  mPanGestureScalarOnlyPredictionEnabled( -1 ),
  mPanGestureTwoPointPredictionEnabled( -1 ),
  mPanGestureTwoPointInterpolatePastTime( -1 ),
  mPanGestureTwoPointVelocityBias( -1.0f ),
  mPanGestureTwoPointAccelerationBias( -1.0f ),
  mPanGestureMultitapSmoothingRange( -1 ),
  mPanMinimumDistance( -1 ),
  mPanMinimumEvents( -1 ),
  mGlesCallTime( 0 ),
  mMultiSamplingLevel( DEFAULT_MULTI_SAMPLING_LEVEL ),
  mIndicatorVisibleMode( -1 ),
  mThreadingMode( ThreadingMode::COMBINED_UPDATE_RENDER ),
  mGlesCallAccumulate( false ),
  mDepthBufferRequired( DEFAULT_DEPTH_BUFFER_REQUIRED_SETTING ),
  mStencilBufferRequired( DEFAULT_STENCIL_BUFFER_REQUIRED_SETTING )
{
  ParseEnvironmentOptions();
}

EnvironmentOptions::~EnvironmentOptions()
{
}

void EnvironmentOptions::CreateTraceManager( PerformanceInterface* performanceInterface )
{
  mTraceManager = TraceManagerFactory::CreateTraceFactory( performanceInterface );
}

void EnvironmentOptions::InstallTraceFunction() const
{
  if( mTraceManager )
  {
    mTraceManager->Initialise();
  }
}

void EnvironmentOptions::SetLogFunction( const Dali::Integration::Log::LogFunction& logFunction )
{
  mLogFunction = logFunction;
}

void EnvironmentOptions::InstallLogFunction() const
{
  Dali::Integration::Log::InstallLogFunction( mLogFunction );
}

void EnvironmentOptions::UnInstallLogFunction() const
{
  Dali::Integration::Log::UninstallLogFunction();
}

unsigned int EnvironmentOptions::GetNetworkControlMode() const
{
  return mNetworkControl;
}
unsigned int EnvironmentOptions::GetFrameRateLoggingFrequency() const
{
  return mFpsFrequency;
}

unsigned int EnvironmentOptions::GetUpdateStatusLoggingFrequency() const
{
  return mUpdateStatusFrequency;
}

unsigned int EnvironmentOptions::GetObjectProfilerInterval() const
{
  return mObjectProfilerInterval;
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

int EnvironmentOptions::GetPanGestureUseActualTimes() const
{
  return mPanGestureUseActualTimes;
}

int EnvironmentOptions::GetPanGestureInterpolationTimeRange() const
{
  return mPanGestureInterpolationTimeRange;
}

int EnvironmentOptions::GetPanGestureScalarOnlyPredictionEnabled() const
{
  return mPanGestureScalarOnlyPredictionEnabled;
}

int EnvironmentOptions::GetPanGestureTwoPointPredictionEnabled() const
{
  return mPanGestureTwoPointPredictionEnabled;
}

int EnvironmentOptions::GetPanGestureTwoPointInterpolatePastTime() const
{
  return mPanGestureTwoPointInterpolatePastTime;
}

float EnvironmentOptions::GetPanGestureTwoPointVelocityBias() const
{
  return mPanGestureTwoPointVelocityBias;
}

float EnvironmentOptions::GetPanGestureTwoPointAccelerationBias() const
{
  return mPanGestureTwoPointAccelerationBias;
}

int EnvironmentOptions::GetPanGestureMultitapSmoothingRange() const
{
  return mPanGestureMultitapSmoothingRange;
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

int EnvironmentOptions::GetGlesCallTime() const
{
  return mGlesCallTime;
}

bool EnvironmentOptions::GetGlesCallAccumulate() const
{
  return mGlesCallAccumulate;
}

const std::string& EnvironmentOptions::GetWindowName() const
{
  return mWindowName;
}

const std::string& EnvironmentOptions::GetWindowClassName() const
{
  return mWindowClassName;
}

ThreadingMode::Type EnvironmentOptions::GetThreadingMode() const
{
  return mThreadingMode;
}

unsigned int EnvironmentOptions::GetRenderRefreshRate() const
{
  return mRenderRefreshRate;
}

int EnvironmentOptions::GetMultiSamplingLevel() const
{
  return mMultiSamplingLevel;
}

unsigned int EnvironmentOptions::GetMaxTextureSize() const
{
  return mMaxTextureSize;
}

int EnvironmentOptions::GetIndicatorVisibleMode() const
{
  return mIndicatorVisibleMode;
}

unsigned int EnvironmentOptions::GetRenderToFboInterval() const
{
  return mRenderToFboInterval;
}

bool EnvironmentOptions::PerformanceServerRequired() const
{
  return ( ( GetPerformanceStatsLoggingOptions() > 0) ||
           ( GetPerformanceTimeStampOutput() > 0 ) ||
           ( GetNetworkControlMode() > 0) );
}

bool EnvironmentOptions::DepthBufferRequired() const
{
  return mDepthBufferRequired;
}

bool EnvironmentOptions::StencilBufferRequired() const
{
  return mStencilBufferRequired;
}

void EnvironmentOptions::ParseEnvironmentOptions()
{
  // get logging options
  mFpsFrequency = GetIntegerEnvironmentVariable( DALI_ENV_FPS_TRACKING, 0 );
  mUpdateStatusFrequency = GetIntegerEnvironmentVariable( DALI_ENV_UPDATE_STATUS_INTERVAL, 0 );
  mObjectProfilerInterval = GetIntegerEnvironmentVariable( DALI_ENV_OBJECT_PROFILER_INTERVAL, 0 );
  mPerformanceStatsLevel = GetIntegerEnvironmentVariable( DALI_ENV_LOG_PERFORMANCE_STATS, 0 );
  mPerformanceStatsFrequency = GetIntegerEnvironmentVariable( DALI_ENV_LOG_PERFORMANCE_STATS_FREQUENCY, 0 );
  mPerformanceTimeStampOutput = GetIntegerEnvironmentVariable( DALI_ENV_PERFORMANCE_TIMESTAMP_OUTPUT, 0 );
  mNetworkControl = GetIntegerEnvironmentVariable( DALI_ENV_NETWORK_CONTROL, 0 );
  mPanGestureLoggingLevel = GetIntegerEnvironmentVariable( DALI_ENV_LOG_PAN_GESTURE, 0 );

  int predictionMode;
  if( GetIntegerEnvironmentVariable(DALI_ENV_PAN_PREDICTION_MODE, predictionMode) )
  {
    mPanGesturePredictionMode = predictionMode;
  }
  int predictionAmount(-1);
  if( GetIntegerEnvironmentVariable(DALI_ENV_PAN_PREDICTION_AMOUNT, predictionAmount) )
  {
    if( predictionAmount < 0 )
    {
      // do not support times in the past
      predictionAmount = 0;
    }
    mPanGesturePredictionAmount = predictionAmount;
  }
  int minPredictionAmount(-1);
  if( GetIntegerEnvironmentVariable(DALI_ENV_PAN_MIN_PREDICTION_AMOUNT, minPredictionAmount) )
  {
    if( minPredictionAmount < 0 )
    {
      // do not support times in the past
      minPredictionAmount = 0;
    }
    mPanGestureMinPredictionAmount = minPredictionAmount;
  }
  int maxPredictionAmount(-1);
  if( GetIntegerEnvironmentVariable(DALI_ENV_PAN_MAX_PREDICTION_AMOUNT, maxPredictionAmount) )
  {
    if( minPredictionAmount > -1 && maxPredictionAmount < minPredictionAmount )
    {
      // maximum amount should not be smaller than minimum amount
      maxPredictionAmount = minPredictionAmount;
    }
    mPanGestureMaxPredictionAmount = maxPredictionAmount;
  }
  int predictionAmountAdjustment(-1);
  if( GetIntegerEnvironmentVariable(DALI_ENV_PAN_PREDICTION_AMOUNT_ADJUSTMENT, predictionAmountAdjustment) )
  {
    if( predictionAmountAdjustment < 0 )
    {
      // negative amount doesn't make sense
      predictionAmountAdjustment = 0;
    }
    mPanGesturePredictionAmountAdjustment = predictionAmountAdjustment;
  }
  int smoothingMode;
  if( GetIntegerEnvironmentVariable(DALI_ENV_PAN_SMOOTHING_MODE, smoothingMode) )
  {
    mPanGestureSmoothingMode = smoothingMode;
  }
  float smoothingAmount = 1.0f;
  if( GetFloatEnvironmentVariable(DALI_ENV_PAN_SMOOTHING_AMOUNT, smoothingAmount) )
  {
    smoothingAmount = Clamp(smoothingAmount, 0.0f, 1.0f);
    mPanGestureSmoothingAmount = smoothingAmount;
  }

  int useActualTimes( -1 );
  if( GetIntegerEnvironmentVariable( DALI_ENV_PAN_USE_ACTUAL_TIMES, useActualTimes ) )
  {
    mPanGestureUseActualTimes = useActualTimes == 0 ? 0 : 1;
  }

  int interpolationTimeRange( -1 );
  if( GetIntegerEnvironmentVariable( DALI_ENV_PAN_INTERPOLATION_TIME_RANGE, interpolationTimeRange ) )
  {
    if( interpolationTimeRange < 0 )
    {
      interpolationTimeRange = 0;
    }
    mPanGestureInterpolationTimeRange = interpolationTimeRange;
  }

  int scalarOnlyPredictionEnabled( -1 );
  if( GetIntegerEnvironmentVariable( DALI_ENV_PAN_SCALAR_ONLY_PREDICTION_ENABLED, scalarOnlyPredictionEnabled ) )
  {
    mPanGestureScalarOnlyPredictionEnabled = scalarOnlyPredictionEnabled == 0 ? 0 : 1;
  }

  int twoPointPredictionEnabled( -1 );
  if( GetIntegerEnvironmentVariable( DALI_ENV_PAN_TWO_POINT_PREDICTION_ENABLED, twoPointPredictionEnabled ) )
  {
    mPanGestureTwoPointPredictionEnabled = twoPointPredictionEnabled == 0 ? 0 : 1;
  }

  int twoPointPastInterpolateTime( -1 );
  if( GetIntegerEnvironmentVariable( DALI_ENV_PAN_TWO_POINT_PAST_INTERPOLATE_TIME, twoPointPastInterpolateTime ) )
  {
    if( twoPointPastInterpolateTime < 0 )
    {
      twoPointPastInterpolateTime = 0;
    }
    mPanGestureTwoPointInterpolatePastTime = twoPointPastInterpolateTime;
  }

  float twoPointVelocityBias = -1.0f;
  if( GetFloatEnvironmentVariable( DALI_ENV_PAN_TWO_POINT_VELOCITY_BIAS, twoPointVelocityBias ) )
  {
    twoPointVelocityBias = Clamp( twoPointVelocityBias, 0.0f, 1.0f );
    mPanGestureTwoPointVelocityBias = twoPointVelocityBias;
  }

  float twoPointAccelerationBias = -1.0f;
  if( GetFloatEnvironmentVariable( DALI_ENV_PAN_TWO_POINT_ACCELERATION_BIAS, twoPointAccelerationBias ) )
  {
    twoPointAccelerationBias = Clamp( twoPointAccelerationBias, 0.0f, 1.0f );
    mPanGestureTwoPointAccelerationBias = twoPointAccelerationBias;
  }

  int multitapSmoothingRange( -1 );
  if( GetIntegerEnvironmentVariable( DALI_ENV_PAN_MULTITAP_SMOOTHING_RANGE, multitapSmoothingRange ) )
  {
    if( multitapSmoothingRange < 0 )
    {
      multitapSmoothingRange = 0;
    }
    mPanGestureMultitapSmoothingRange = multitapSmoothingRange;
  }

  int minimumDistance(-1);
  if ( GetIntegerEnvironmentVariable(DALI_ENV_PAN_MINIMUM_DISTANCE, minimumDistance ))
  {
    mPanMinimumDistance = minimumDistance;
  }

  int minimumEvents(-1);
  if ( GetIntegerEnvironmentVariable(DALI_ENV_PAN_MINIMUM_EVENTS, minimumEvents ))
  {
    mPanMinimumEvents = minimumEvents;
  }

  int glesCallTime(0);
  if ( GetIntegerEnvironmentVariable(DALI_GLES_CALL_TIME, glesCallTime ))
  {
    mGlesCallTime = glesCallTime;
  }

  int glesCallAccumulate( 0 );
  if ( GetIntegerEnvironmentVariable( DALI_GLES_CALL_ACCUMULATE, glesCallAccumulate ) )
  {
    mGlesCallAccumulate = glesCallAccumulate != 0;
  }

  int windowWidth(0), windowHeight(0);
  if ( GetIntegerEnvironmentVariable( DALI_WINDOW_WIDTH, windowWidth ) && GetIntegerEnvironmentVariable( DALI_WINDOW_HEIGHT, windowHeight ) )
  {
    mWindowWidth = windowWidth;
    mWindowHeight = windowHeight;
  }

  const char * windowName = GetCharEnvironmentVariable( DALI_WINDOW_NAME );
  if ( windowName )
  {
    mWindowName = windowName;
  }

  const char * windowClassName = GetCharEnvironmentVariable( DALI_WINDOW_CLASS_NAME );
  if ( windowClassName )
  {
    mWindowClassName = windowClassName;
  }

  int threadingMode(0);
  if ( GetIntegerEnvironmentVariable( DALI_THREADING_MODE, threadingMode ) )
  {
    switch( threadingMode )
    {
      case ThreadingMode::COMBINED_UPDATE_RENDER:
      {
        mThreadingMode = static_cast< ThreadingMode::Type >( threadingMode );
        break;
      }
    }
  }

  int renderRefreshRate(0);
  if ( GetIntegerEnvironmentVariable( DALI_REFRESH_RATE, renderRefreshRate ) )
  {
    // Only change it if it's valid
    if( renderRefreshRate > 1 )
    {
      mRenderRefreshRate = renderRefreshRate;
    }
  }

  int multiSamplingLevel( 0 );
  if( GetIntegerEnvironmentVariable( DALI_ENV_MULTI_SAMPLING_LEVEL, multiSamplingLevel ) )
  {
    mMultiSamplingLevel = multiSamplingLevel;
  }

  int maxTextureSize( 0 );
  if( GetIntegerEnvironmentVariable( DALI_ENV_MAX_TEXTURE_SIZE, maxTextureSize ) )
  {
    if( maxTextureSize > 0 )
    {
      mMaxTextureSize = maxTextureSize;
    }
  }

  int indicatorVisibleMode( -1 );
  if( GetIntegerEnvironmentVariable( DALI_ENV_INDICATOR_VISIBLE_MODE, indicatorVisibleMode ) )
  {
    if( indicatorVisibleMode > -1 )
    {
      mIndicatorVisibleMode = indicatorVisibleMode;
    }
  }

  mRenderToFboInterval = GetIntegerEnvironmentVariable( DALI_RENDER_TO_FBO, 0u );


  int depthBufferRequired( -1 );
  if( GetIntegerEnvironmentVariable( DALI_ENV_DISABLE_DEPTH_BUFFER, depthBufferRequired ) )
  {
    if( depthBufferRequired > 0 )
    {
      mDepthBufferRequired = false;
      mStencilBufferRequired = false; // Disable stencil buffer as well
    }
  }

  int stencilBufferRequired( -1 );
  if( GetIntegerEnvironmentVariable( DALI_ENV_DISABLE_STENCIL_BUFFER, stencilBufferRequired ) )
  {
    if( stencilBufferRequired > 0 )
    {
      mStencilBufferRequired = false;
    }
  }
}

} // Adaptor

} // Internal

} // Dali
