/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/render-controller.h>
#include <dali/public-api/math/math-utils.h>
#include <algorithm>
#include <cstdlib>
#include <functional>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/system/common/environment-variables.h>
#include <dali/internal/trace/common/trace-factory.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
const unsigned int DEFAULT_STATISTICS_LOG_FREQUENCY        = 2;
const int          DEFAULT_MULTI_SAMPLING_LEVEL            = -1;
const bool         DEFAULT_DEPTH_BUFFER_REQUIRED_SETTING   = true;
const bool         DEFAULT_STENCIL_BUFFER_REQUIRED_SETTING = true;
const bool         DEFAULT_PARTIAL_UPDATE_REQUIRED_SETTING = true;
const bool         DEFAULT_VSYNC_RENDER_REQUIRED_SETTING   = true;

unsigned int GetEnvironmentVariable(const char* variable, unsigned int defaultValue)
{
  const char* variableParameter = Dali::EnvironmentVariable::GetEnvironmentVariable(variable);

  // if the parameter exists convert it to an integer, else return the default value
  unsigned int intValue = variableParameter ? std::atoi(variableParameter) : defaultValue;
  return intValue;
}

bool GetEnvironmentVariable(const char* variable, int& intValue)
{
  const char* variableParameter = Dali::EnvironmentVariable::GetEnvironmentVariable(variable);

  if(!variableParameter)
  {
    return false;
  }
  // if the parameter exists convert it to an integer, else return the default value
  intValue = std::atoi(variableParameter);
  return true;
}

bool GetEnvironmentVariable(const char* variable, float& floatValue)
{
  const char* variableParameter = Dali::EnvironmentVariable::GetEnvironmentVariable(variable);

  if(!variableParameter)
  {
    return false;
  }
  // if the parameter exists convert it to an integer, else return the default value
  floatValue = std::atof(variableParameter);
  return true;
}

void SetFromEnvironmentVariable(const char* variable, std::string& stringValue)
{
  const char* charValue = Dali::EnvironmentVariable::GetEnvironmentVariable(variable);
  if(charValue)
  {
    stringValue = charValue;
  }
}

template<typename Type>
void SetFromEnvironmentVariable(const char* variable, Type& memberVariable)
{
  Type envVarValue = -1;
  if(GetEnvironmentVariable(variable, envVarValue))
  {
    memberVariable = envVarValue;
  }
}

template<typename Type>
void SetFromEnvironmentVariable(const char* variable, std::function<void(Type)> function)
{
  Type envVarValue = -1;
  if(GetEnvironmentVariable(variable, envVarValue))
  {
    function(envVarValue);
  }
}

void SetGraphicsBackendFromEnvironmentVariable(Graphics::Backend& api)
{
  const char* charValue = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_GRAPHICS_BACKEND);
  if(charValue)
  {
    // Expecting upper/lower case variations of GLES and VULKAN/VK, and 0 and 1 too so just check the first character
    switch(*charValue)
    {
      case '0':
      case '1':
      {
        api = static_cast<Graphics::Backend>(*charValue - '0');
        break;
      }

      case 'g':
      case 'G':
      {
        std::string stringValue(charValue);
        if(stringValue.size() == 4)
        {
          std::transform(stringValue.begin(), stringValue.end(), stringValue.begin(), ::toupper);
          if(stringValue == "GLES")
          {
            api = Graphics::Backend::GLES;
          }
        }
        break;
      }

      case 'v':
      case 'V':
      {
        std::string stringValue(charValue);
        const auto  stringValueSize = stringValue.size();
        if(stringValueSize == 2 || stringValueSize == 6)
        {
          std::transform(stringValue.begin(), stringValue.end(), stringValue.begin(), ::toupper);
          if(stringValue == "VULKAN" || stringValue == "VK")
          {
            api = Graphics::Backend::VULKAN;
          }
        }
        break;
      }
    }
  }
}

/// Provides a functor which ensures a non-negative number is set for the given member variable
struct MinimumZero
{
  MinimumZero(int& memberValue)
  : mMemberValue(memberValue)
  {
  }

  void operator()(int value)
  {
    // Negative Amounts do not make sense
    mMemberValue = std::max(0, value);
  }

  int& mMemberValue;
};

/// Provides a functor which clamps the environment variable between 0.0f and 1.0f
struct ClampBetweenZeroAndOne
{
  ClampBetweenZeroAndOne(float& memberValue)
  : mMemberValue(memberValue)
  {
  }

  void operator()(float value)
  {
    value        = Clamp(value, 0.0f, 1.0f);
    mMemberValue = value;
  }

  float& mMemberValue;
};

/// Provides a functor which only sets the member variable if the environment variable value is greater than the specified constant
struct GreaterThan
{
  GreaterThan(unsigned int& memberValue, int greaterThanValue)
  : mMemberValue(memberValue),
    mGreaterThanValue(greaterThanValue)
  {
  }

  void operator()(int value)
  {
    if(value > mGreaterThanValue)
    {
      mMemberValue = value;
    }
  }

  unsigned int& mMemberValue;
  const int     mGreaterThanValue;
};

/// Provides a functor which sets the member to 1 if if the environment variable value is not zero
struct EnableIfNonZero
{
  EnableIfNonZero(int& memberValue)
  : mMemberValue(memberValue)
  {
  }

  void operator()(int value)
  {
    mMemberValue = (value == 0) ? 0 : 1;
  }

  int& mMemberValue;
};

/// Provides a functor which sets the member to false if the environment variable value is not zero
struct DisableIfNonZero
{
  DisableIfNonZero(bool& memberValue)
  : mMemberValue(memberValue)
  {
  }

  void operator()(int value)
  {
    if(value > 0)
    {
      mMemberValue = false;
    }
  }

  bool& mMemberValue;
};

} // unnamed namespace

EnvironmentOptions::EnvironmentOptions()
: mLogFunction(NULL),
  mWindowName(),
  mWindowClassName(),
  mNetworkControl(0),
  mFpsFrequency(0),
  mUpdateStatusFrequency(0),
  mObjectProfilerInterval(0),
  mMemoryPoolInterval(0),
  mPerformanceStatsLevel(0),
  mPerformanceStatsFrequency(DEFAULT_STATISTICS_LOG_FREQUENCY),
  mPerformanceTimeStampOutput(0),
  mPanGestureLoggingLevel(0),
  mWindowWidth(0u),
  mWindowHeight(0u),
  mRenderRefreshRate(1u),
  mMaxTextureSize(0),
  mRenderToFboInterval(0u),
  mPanGesturePredictionMode(-1),
  mPanGesturePredictionAmount(-1), ///< only sets value in pan gesture if greater than 0
  mPanGestureMaxPredictionAmount(-1),
  mPanGestureMinPredictionAmount(-1),
  mPanGesturePredictionAmountAdjustment(-1),
  mPanGestureSmoothingMode(-1),
  mPanGestureSmoothingAmount(-1.0f),
  mPanGestureUseActualTimes(-1),
  mPanGestureInterpolationTimeRange(-1),
  mPanGestureScalarOnlyPredictionEnabled(-1),
  mPanGestureTwoPointPredictionEnabled(-1),
  mPanGestureTwoPointInterpolatePastTime(-1),
  mPanGestureTwoPointVelocityBias(-1.0f),
  mPanGestureTwoPointAccelerationBias(-1.0f),
  mPanGestureMultitapSmoothingRange(-1),
  mPanMinimumDistance(-1),
  mPanMinimumEvents(-1),
  mPinchMinimumDistance(-1.0f),
  mPinchMinimumTouchEvents(-1),
  mPinchMinimumTouchEventsAfterStart(-1),
  mRotationMinimumTouchEvents(-1),
  mRotationMinimumTouchEventsAfterStart(-1),
  mLongPressMinimumHoldingTime(-1),
  mTapMaximumAllowedTime(-1),
  mGlesCallTime(0),
  mMultiSamplingLevel(DEFAULT_MULTI_SAMPLING_LEVEL),
  mThreadingMode(ThreadingMode::COMBINED_UPDATE_RENDER),
  mGraphicsBackend(Graphics::Backend::DEFAULT),
  mGlesCallAccumulate(false),
  mDepthBufferRequired(DEFAULT_DEPTH_BUFFER_REQUIRED_SETTING),
  mStencilBufferRequired(DEFAULT_STENCIL_BUFFER_REQUIRED_SETTING),
  mPartialUpdateRequired(DEFAULT_PARTIAL_UPDATE_REQUIRED_SETTING),
  mVsyncRenderRequired(DEFAULT_VSYNC_RENDER_REQUIRED_SETTING)
{
  ParseEnvironmentOptions();
}

EnvironmentOptions::~EnvironmentOptions()
{
}

void EnvironmentOptions::CreateTraceManager(PerformanceInterface* performanceInterface)
{
  mTraceManager = TraceManagerFactory::CreateTraceFactory(performanceInterface);
}

void EnvironmentOptions::InstallTraceFunction() const
{
  if(mTraceManager)
  {
    mTraceManager->Initialise();
  }
}

void EnvironmentOptions::SetLogFunction(const Dali::Integration::Log::LogFunction& logFunction)
{
  mLogFunction = logFunction;
}

void EnvironmentOptions::InstallLogFunction() const
{
  Dali::Integration::Log::InstallLogFunction(mLogFunction);
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
unsigned int EnvironmentOptions::GetMemoryPoolInterval() const
{
  return mMemoryPoolInterval;
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

float EnvironmentOptions::GetMinimumPinchDistance() const
{
  return mPinchMinimumDistance;
}

int EnvironmentOptions::GetMinimumPinchTouchEvents() const
{
  return mPinchMinimumTouchEvents;
}

int EnvironmentOptions::GetMinimumPinchTouchEventsAfterStart() const
{
  return mPinchMinimumTouchEventsAfterStart;
}

int EnvironmentOptions::GetMinimumRotationTouchEvents() const
{
  return mRotationMinimumTouchEvents;
}

int EnvironmentOptions::GetMinimumRotationTouchEventsAfterStart() const
{
  return mRotationMinimumTouchEventsAfterStart;
}

int EnvironmentOptions::GetLongPressMinimumHoldingTime() const
{
  return mLongPressMinimumHoldingTime;
}

int EnvironmentOptions::GetTapMaximumAllowedTime() const
{
  return mTapMaximumAllowedTime;
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

Graphics::Backend EnvironmentOptions::GetGraphicsBackend() const
{
  return mGraphicsBackend;
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

unsigned int EnvironmentOptions::GetRenderToFboInterval() const
{
  return mRenderToFboInterval;
}

bool EnvironmentOptions::PerformanceServerRequired() const
{
  return ((GetPerformanceStatsLoggingOptions() > 0) ||
          (GetPerformanceTimeStampOutput() > 0) ||
          (GetNetworkControlMode() > 0));
}

bool EnvironmentOptions::DepthBufferRequired() const
{
  return mDepthBufferRequired;
}

bool EnvironmentOptions::StencilBufferRequired() const
{
  return mStencilBufferRequired;
}

bool EnvironmentOptions::PartialUpdateRequired() const
{
  return mPartialUpdateRequired;
}

bool EnvironmentOptions::VsyncRenderRequired() const
{
  return mVsyncRenderRequired;
}

void EnvironmentOptions::ParseEnvironmentOptions()
{
  // get logging options
  mFpsFrequency               = GetEnvironmentVariable(DALI_ENV_FPS_TRACKING, 0);
  mUpdateStatusFrequency      = GetEnvironmentVariable(DALI_ENV_UPDATE_STATUS_INTERVAL, 0);
  mObjectProfilerInterval     = GetEnvironmentVariable(DALI_ENV_OBJECT_PROFILER_INTERVAL, 0);
  mMemoryPoolInterval         = GetEnvironmentVariable(DALI_ENV_MEMORY_POOL_INTERVAL, 0);
  mPerformanceStatsLevel      = GetEnvironmentVariable(DALI_ENV_LOG_PERFORMANCE_STATS, 0);
  mPerformanceStatsFrequency  = GetEnvironmentVariable(DALI_ENV_LOG_PERFORMANCE_STATS_FREQUENCY, 0);
  mPerformanceTimeStampOutput = GetEnvironmentVariable(DALI_ENV_PERFORMANCE_TIMESTAMP_OUTPUT, 0);
  mNetworkControl             = GetEnvironmentVariable(DALI_ENV_NETWORK_CONTROL, 0);
  mPanGestureLoggingLevel     = GetEnvironmentVariable(DALI_ENV_LOG_PAN_GESTURE, 0);

  SetFromEnvironmentVariable(DALI_ENV_PAN_PREDICTION_MODE, mPanGesturePredictionMode);
  SetFromEnvironmentVariable<int>(DALI_ENV_PAN_PREDICTION_AMOUNT, MinimumZero(mPanGesturePredictionAmount));
  SetFromEnvironmentVariable<int>(DALI_ENV_PAN_MIN_PREDICTION_AMOUNT, MinimumZero(mPanGestureMinPredictionAmount));
  SetFromEnvironmentVariable<int>(DALI_ENV_PAN_MAX_PREDICTION_AMOUNT,
                                  [&](int maxPredictionAmount) {
                                    if(mPanGestureMinPredictionAmount > -1 && maxPredictionAmount < mPanGestureMinPredictionAmount)
                                    {
                                      // maximum amount should not be smaller than minimum amount
                                      maxPredictionAmount = mPanGestureMinPredictionAmount;
                                    }
                                    mPanGestureMaxPredictionAmount = maxPredictionAmount;
                                  });
  SetFromEnvironmentVariable<int>(DALI_ENV_PAN_PREDICTION_AMOUNT_ADJUSTMENT, MinimumZero(mPanGesturePredictionAmountAdjustment));
  SetFromEnvironmentVariable(DALI_ENV_PAN_SMOOTHING_MODE, mPanGestureSmoothingMode);
  SetFromEnvironmentVariable<float>(DALI_ENV_PAN_SMOOTHING_AMOUNT, ClampBetweenZeroAndOne(mPanGestureSmoothingAmount));
  SetFromEnvironmentVariable<int>(DALI_ENV_PAN_USE_ACTUAL_TIMES, EnableIfNonZero(mPanGestureUseActualTimes));
  SetFromEnvironmentVariable<int>(DALI_ENV_PAN_INTERPOLATION_TIME_RANGE, MinimumZero(mPanGestureInterpolationTimeRange));
  SetFromEnvironmentVariable<int>(DALI_ENV_PAN_SCALAR_ONLY_PREDICTION_ENABLED, EnableIfNonZero(mPanGestureScalarOnlyPredictionEnabled));
  SetFromEnvironmentVariable<int>(DALI_ENV_PAN_TWO_POINT_PREDICTION_ENABLED, EnableIfNonZero(mPanGestureTwoPointPredictionEnabled));
  SetFromEnvironmentVariable<int>(DALI_ENV_PAN_TWO_POINT_PAST_INTERPOLATE_TIME, MinimumZero(mPanGestureTwoPointInterpolatePastTime));
  SetFromEnvironmentVariable<float>(DALI_ENV_PAN_TWO_POINT_VELOCITY_BIAS, ClampBetweenZeroAndOne(mPanGestureTwoPointVelocityBias));
  SetFromEnvironmentVariable<float>(DALI_ENV_PAN_TWO_POINT_ACCELERATION_BIAS, ClampBetweenZeroAndOne(mPanGestureTwoPointAccelerationBias));
  SetFromEnvironmentVariable<int>(DALI_ENV_PAN_MULTITAP_SMOOTHING_RANGE, MinimumZero(mPanGestureMultitapSmoothingRange));
  SetFromEnvironmentVariable(DALI_ENV_PAN_MINIMUM_DISTANCE, mPanMinimumDistance);
  SetFromEnvironmentVariable(DALI_ENV_PAN_MINIMUM_EVENTS, mPanMinimumEvents);

  SetFromEnvironmentVariable(DALI_ENV_PINCH_MINIMUM_DISTANCE, mPinchMinimumDistance);
  SetFromEnvironmentVariable(DALI_ENV_PINCH_MINIMUM_TOUCH_EVENTS, mPinchMinimumTouchEvents);
  SetFromEnvironmentVariable(DALI_ENV_PINCH_MINIMUM_TOUCH_EVENTS_AFTER_START, mPinchMinimumTouchEventsAfterStart);

  SetFromEnvironmentVariable(DALI_ENV_ROTATION_MINIMUM_TOUCH_EVENTS, mRotationMinimumTouchEvents);
  SetFromEnvironmentVariable(DALI_ENV_ROTATION_MINIMUM_TOUCH_EVENTS_AFTER_START, mRotationMinimumTouchEventsAfterStart);

  SetFromEnvironmentVariable(DALI_ENV_LONG_PRESS_MINIMUM_HOLDING_TIME, mLongPressMinimumHoldingTime);
  SetFromEnvironmentVariable(DALI_ENV_TAP_MAXIMUM_ALLOWED_TIME, mTapMaximumAllowedTime);

  SetFromEnvironmentVariable(DALI_GLES_CALL_TIME, mGlesCallTime);
  SetFromEnvironmentVariable<int>(DALI_GLES_CALL_ACCUMULATE, [&](int glesCallAccumulate) { mGlesCallAccumulate = glesCallAccumulate != 0; });

  int windowWidth(0), windowHeight(0);
  if(GetEnvironmentVariable(DALI_WINDOW_WIDTH, windowWidth) && GetEnvironmentVariable(DALI_WINDOW_HEIGHT, windowHeight))
  {
    mWindowWidth  = windowWidth;
    mWindowHeight = windowHeight;
  }
  SetFromEnvironmentVariable(DALI_WINDOW_NAME, mWindowName);
  SetFromEnvironmentVariable(DALI_WINDOW_CLASS_NAME, mWindowClassName);

  SetFromEnvironmentVariable<int>(DALI_THREADING_MODE,
                                  [&](int threadingMode) {
                                    switch(threadingMode)
                                    {
                                      case ThreadingMode::COMBINED_UPDATE_RENDER:
                                      {
                                        mThreadingMode = static_cast<ThreadingMode::Type>(threadingMode);
                                        break;
                                      }
                                    }
                                  });

  SetGraphicsBackendFromEnvironmentVariable(mGraphicsBackend);

  SetFromEnvironmentVariable<int>(DALI_REFRESH_RATE, GreaterThan(mRenderRefreshRate, 1));

  SetFromEnvironmentVariable(DALI_ENV_MULTI_SAMPLING_LEVEL, mMultiSamplingLevel);

  SetFromEnvironmentVariable<int>(DALI_ENV_MAX_TEXTURE_SIZE, GreaterThan(mMaxTextureSize, 0));

  mRenderToFboInterval = GetEnvironmentVariable(DALI_RENDER_TO_FBO, 0u);

  SetFromEnvironmentVariable<int>(DALI_ENV_DISABLE_DEPTH_BUFFER,
                                  [&](int depthBufferRequired) {
                                    if(depthBufferRequired > 0)
                                    {
                                      mDepthBufferRequired   = false;
                                      mStencilBufferRequired = false; // Disable stencil buffer as well
                                    }
                                  });
  SetFromEnvironmentVariable<int>(DALI_ENV_DISABLE_STENCIL_BUFFER, DisableIfNonZero(mStencilBufferRequired));

  SetFromEnvironmentVariable<int>(DALI_ENV_DISABLE_PARTIAL_UPDATE, DisableIfNonZero(mPartialUpdateRequired));

  SetFromEnvironmentVariable<int>(DALI_ENV_DISABLE_VSYNC_RENDER, DisableIfNonZero(mVsyncRenderRequired));
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
