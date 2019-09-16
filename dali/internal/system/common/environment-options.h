#ifndef DALI_INTERNAL_ADAPTOR_ENVIRONMENT_OPTIONS_H
#define DALI_INTERNAL_ADAPTOR_ENVIRONMENT_OPTIONS_H

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

// EXTERNAL INCLUDES
#include <memory>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/adaptor/common/threading-mode.h>
#include <dali/integration-api/log-factory-interface.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class TraceManager;
class PerformanceInterface;

/**
 * This class provides the environment options which define settings as well as
 * the ability to install a log function.
 *
 */
class EnvironmentOptions : public Dali::LogFactoryInterface
{
public:

  /**
   * Constructor
   */
  EnvironmentOptions();

  /**
   * Virtual Destructor for interface cleanup
   */
  virtual ~EnvironmentOptions();

  /**
   * Create a TraceManager which is used for tracing.
   * @param PerformanceInterface for using network logging for tracing on Ubuntu
   */
  void CreateTraceManager( PerformanceInterface* performanceInterface );

  /**
   * Initialize TraceManager by installing Trace function.
   */
  void InstallTraceFunction() const;

  /**
   * @param logFunction logging function
   */
  void SetLogFunction( const Dali::Integration::Log::LogFunction& logFunction );

  /**
   * Install the log function for the current thread.
   */
  virtual void InstallLogFunction() const;

  /**
   * Un-install the log function for the current thread.
   */
  void UnInstallLogFunction() const;

  /**
   * @return whether network control is enabled or not ( 0 = off, 1 = on )
   */
  unsigned int GetNetworkControlMode() const;

  /**
   * @return frequency of how often FPS is logged out (e.g. 0 = off, 2 = every 2 seconds).
   */
  unsigned int GetFrameRateLoggingFrequency() const;

  /**
   * @return frequency of how often Update Status is logged out (e.g. 0 = off, 60 = log every 60 frames = 1 second @ 60FPS).
   */
  unsigned int GetUpdateStatusLoggingFrequency() const;

  /**
   * @return object profiler status interval ( 0 == off )
   */
  unsigned int GetObjectProfilerInterval() const;

  /**
   * @return performance statistics log level ( 0 == off )
   */
  unsigned int GetPerformanceStatsLoggingOptions() const;

  /**
   * @return performance statistics log frequency in seconds
   */
  unsigned int GetPerformanceStatsLoggingFrequency() const;

  /**
   * @return performance time stamp output ( 0 == off)
   */
  unsigned int GetPerformanceTimeStampOutput() const;

  /**
   * @return pan-gesture logging level ( 0 == off )
   */
  unsigned int GetPanGestureLoggingLevel() const;

  /**
   * @return pan-gesture prediction mode ( -1 means not set so no prediction, 0 = no prediction )
   */
  int GetPanGesturePredictionMode() const;

  /**
   * @return pan-gesture prediction amount
   */
  int GetPanGesturePredictionAmount() const;

  /**
   * @return maximum pan-gesture prediction amount
   */
  int GetPanGestureMaximumPredictionAmount() const;

  /**
   * @return minimum pan-gesture prediction amount
   */
  int GetPanGestureMinimumPredictionAmount() const;

  /**
   * @brief Gets the prediction amount to adjust when the pan velocity is changed.
   *
   * If the pan velocity is accelerating, the prediction amount will be increased
   * by the specified amount until it reaches the upper bound. If the pan velocity
   * is decelerating, the prediction amount will be decreased by the specified
   * amount until it reaches the lower bound.
   *
   * @return pan-gesture prediction amount adjustment
   */
  int GetPanGesturePredictionAmountAdjustment() const;

  /**
   * @return pan-gesture smoothing mode ( -1 means not set so no smoothing, 0 = no smoothing )
   */
  int GetPanGestureSmoothingMode() const;

  /**
   * @return pan-gesture smoothing amount
   */
  float GetPanGestureSmoothingAmount() const;

  /**
   * @return pan-gesture use actual times is true if real gesture and frame times are to be used.
   */
  int GetPanGestureUseActualTimes() const;

  /**
   * @return pan-gesture interpolation time range is the time range (ms) of past points to use (with weights) when interpolating.
   */
  int GetPanGestureInterpolationTimeRange() const;

  /**
   * @return pan-gesture scalar only prediction, when enabled, ignores acceleration.
   */
  int GetPanGestureScalarOnlyPredictionEnabled() const;

  /**
   * @return pan-gesture two point prediction combines two interpolated points to get more steady acceleration and velocity values.
   */
  int GetPanGestureTwoPointPredictionEnabled() const;

  /**
   * @return pan-gesture two point interpolate past time is the time delta (ms) in the past to interpolate the second point.
   */
  int GetPanGestureTwoPointInterpolatePastTime() const;

  /**
   * @return pan-gesture two point velocity bias is the ratio of first and second points to use for velocity.
   * 0.0f = 100% of first point. 1.0f = 100% of second point.
   */
  float GetPanGestureTwoPointVelocityBias() const;

  /**
   * @return pan-gesture two point acceleration bias is the ratio of first and second points to use for acceleration.
   * 0.0f = 100% of first point. 1.0f = 100% of second point.
   */
  float GetPanGestureTwoPointAccelerationBias() const;

  /**
   * @return pan-gesture multitap smoothing range is the range in time (ms) of points in the history to smooth the final output against.
   */
  int GetPanGestureMultitapSmoothingRange() const;

  /**
   * @return The minimum distance before a pan can be started (-1 means it's not set)
   */
  int GetMinimumPanDistance() const;

  /**
   * @return The minimum events before a pan can be started (-1 means it's not set)
   */
  int GetMinimumPanEvents() const;

  /**
   * @return The minimum pixels before a pinch can be started (-1 means it's not set)
   */
  float GetMinimumPinchDistance() const;

  /**
   * @return The width of the window
   */
  unsigned int GetWindowWidth() const;

  /**
   * @return The height of the window
   */
  unsigned int GetWindowHeight() const;

  /**
   * @brief Get the graphics status time
   */
  int GetGlesCallTime() const;

  /**
   * @brief Get whether or not to accumulate gles call statistics
   */
  bool GetGlesCallAccumulate() const;

  /**
   * @return true if performance server is required
   */
  bool PerformanceServerRequired() const;

  /**
   * @return Gets the window name.
   */
  const std::string& GetWindowName() const;

  /**
   * @return Gets the window class.
   */
  const std::string& GetWindowClassName() const;

  /**
   * @return The thread mode that DALi should use.
   */
  ThreadingMode::Type GetThreadingMode() const;

  /**
   * @return The render refresh rate.
   */
  unsigned int GetRenderRefreshRate() const;

  /**
   * @return The number of samples required in multisample buffers
   */
  int GetMultiSamplingLevel() const;

  /**
   * @return The maximum texture size
   */
  unsigned int GetMaxTextureSize() const;

  /**
   * @brief Retrieves the interval of frames to be rendered into the Frame Buffer Object and the Frame Buffer.
   *
   * @return The number of frames that are going to be rendered into the Frame Buffer Object but the last one which is going to be rendered into the Frame Buffer.
   */
  unsigned int GetRenderToFboInterval() const;

  /**
   * @return Whether the depth buffer is required.
   */
  bool DepthBufferRequired() const;

  /**
   * @return Whether the stencil buffer is required.
   */
  bool StencilBufferRequired() const;

  /// Deleted copy constructor.
  EnvironmentOptions( const EnvironmentOptions& ) = delete;

  /// Deleted move constructor.
  EnvironmentOptions( const EnvironmentOptions&& ) = delete;

  /// Deleted assignment operator.
  EnvironmentOptions& operator=( const EnvironmentOptions& ) = delete;

  /// Deleted move assignment operator.
  EnvironmentOptions& operator=( const EnvironmentOptions&& ) = delete;

private: // Internal

  /**
   * Parses the environment options.
   * Called from the constructor
   */
  void ParseEnvironmentOptions();

private: // Data

  Dali::Integration::Log::LogFunction mLogFunction;
  std::string mWindowName;                        ///< name of the window
  std::string mWindowClassName;                   ///< name of the class the window belongs to
  unsigned int mNetworkControl;                   ///< whether network control is enabled
  unsigned int mFpsFrequency;                     ///< how often fps is logged out in seconds
  unsigned int mUpdateStatusFrequency;            ///< how often update status is logged out in frames
  unsigned int mObjectProfilerInterval;           ///< how often object counts are logged out in seconds
  unsigned int mPerformanceStatsLevel;            ///< performance statistics logging bitmask
  unsigned int mPerformanceStatsFrequency;        ///< performance statistics logging frequency (seconds)
  unsigned int mPerformanceTimeStampOutput;       ///< performance time stamp output ( bitmask)
  unsigned int mPanGestureLoggingLevel;           ///< pan-gesture log level
  unsigned int mWindowWidth;                      ///< width of the window
  unsigned int mWindowHeight;                     ///< height of the window
  unsigned int mRenderRefreshRate;                ///< render refresh rate
  unsigned int mMaxTextureSize;                   ///< The maximum texture size that GL can handle
  unsigned int mRenderToFboInterval;              ///< The number of frames that are going to be rendered into the Frame Buffer Object but the last one which is going to be rendered into the Frame Buffer.
  int mPanGesturePredictionMode;                  ///< prediction mode for pan gestures
  int mPanGesturePredictionAmount;                ///< prediction amount for pan gestures
  int mPanGestureMaxPredictionAmount;             ///< maximum prediction amount for pan gestures
  int mPanGestureMinPredictionAmount;             ///< minimum prediction amount for pan gestures
  int mPanGesturePredictionAmountAdjustment;      ///< adjustment of prediction amount for pan gestures
  int mPanGestureSmoothingMode;                   ///< prediction mode for pan gestures
  float mPanGestureSmoothingAmount;               ///< prediction amount for pan gestures
  int mPanGestureUseActualTimes;                  ///< Disable to optionally override actual times if they make results worse.
  int mPanGestureInterpolationTimeRange;          ///< Time into past history (ms) to use points to interpolate the first point.
  int mPanGestureScalarOnlyPredictionEnabled;     ///< If enabled, prediction is done using velocity alone (no integration or acceleration).
  int mPanGestureTwoPointPredictionEnabled;       ///< If enabled, a second interpolated point is predicted and combined with the first to get more stable values.
  int mPanGestureTwoPointInterpolatePastTime;     ///< The target time in the past to generate the second interpolated point.
  float mPanGestureTwoPointVelocityBias;          ///< The ratio of first and second interpolated points to use for velocity. 0.0f = 100% of first point. 1.0f = 100% of second point.
  float mPanGestureTwoPointAccelerationBias;      ///< The ratio of first and second interpolated points to use for acceleration. 0.0f = 100% of first point. 1.0f = 100% of second point.
  int mPanGestureMultitapSmoothingRange;          ///< The range in time (ms) of points in the history to smooth the final output against.
  int mPanMinimumDistance;                        ///< minimum distance required before pan starts
  int mPanMinimumEvents;                          ///< minimum events required before pan starts
  float mPinchMinimumDistance;                    ///< minimum number of pixels moved before a pinch starts
  int mGlesCallTime;                              ///< time in seconds between status updates
  int mMultiSamplingLevel;                        ///< The number of samples required in multisample buffers
  ThreadingMode::Type mThreadingMode;             ///< threading mode
  bool mGlesCallAccumulate;                       ///< Whether or not to accumulate gles call statistics
  bool mDepthBufferRequired;                      ///< Whether the depth buffer is required
  bool mStencilBufferRequired;                    ///< Whether the stencil buffer is required
  std::unique_ptr<TraceManager> mTraceManager;    ///< TraceManager
};

} // Adaptor
} // Internal
} // Dali

#endif // DALI_INTERNAL_ADAPTOR_ENVIRONMENT_OPTIONS_H
