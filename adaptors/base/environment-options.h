#ifndef __DALI_INTERNAL_ADAPTOR_ENVIRONMENT_OPTIONS_H__
#define __DALI_INTERNAL_ADAPTOR_ENVIRONMENT_OPTIONS_H__

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

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

/**
 * Contains environment options which define settings and the ability to install a log function.
 */
class EnvironmentOptions
{

public:

  /**
   * Constructor
   */
  EnvironmentOptions();

  /**
   * non-virtual destructor, not intended as a base class
   */
  ~EnvironmentOptions();

  /**
   * @param logFunction logging function
   * @param logFilterOptions bitmask of the logging options defined in intergration/debug.h (e.g.
   * @param logFrameRateFrequency frequency of how often FPS is logged out (e.g. 0 = off, 2 = every 2 seconds).
   * @param logupdateStatusFrequency frequency of how often the update status is logged in number of frames
   * @param logPerformanceStats performance statistics logging, 0 = disabled,  1+ =  enabled
   * @param logPerformanceStatsFrequency statistics logging frequency in seconds
   * @param performanceTimeStampOutput where to output performance related time stamps to
   * @param logPanGestureLevel pan-gesture logging, 0 = disabled,  1 = enabled
   */
  void SetLogOptions( const Dali::Integration::Log::LogFunction& logFunction,
                       unsigned int logFrameRateFrequency,
                       unsigned int logupdateStatusFrequency,
                       unsigned int logPerformanceStats,
                       unsigned int logPerformanceStatsFrequency,
                       unsigned int performanceTimeStampOutput,
                       unsigned int logPanGestureLevel );

  /**
   * Install the log function for the current thread.
   */
  void InstallLogFunction() const;

  /**
   * Un-install the log function for the current thread.
   */
  void UnInstallLogFunction() const;

  /**
   * @return frequency of how often FPS is logged out (e.g. 0 = off, 2 = every 2 seconds).
   */
  unsigned int GetFrameRateLoggingFrequency() const;

  /**
   * @return frequency of how often Update Status is logged out (e.g. 0 = off, 60 = log every 60 frames = 1 second @ 60FPS).
   */
  unsigned int GetUpdateStatusLoggingFrequency() const;

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
   * @return The minimum distance before a pan can be started (-1 means it's not set)
   */
  int GetMinimumPanDistance() const;

  /**
   * @return The minimum events before a pan can be started (-1 means it's not set)
   */
  int GetMinimumPanEvents() const;

  /**
   * @brief Sets the mode used to predict pan gesture movement
   *
   * @param[in] mode The prediction mode to use
   */
  void SetPanGesturePredictionMode( unsigned int mode );

  /**
   * @brief Sets the prediction amount of the pan gesture
   *
   * @param[in] amount The prediction amount in milliseconds
   */
  void SetPanGesturePredictionAmount( unsigned int amount );

  /**
   * @brief Sets the upper bound of the prediction amount for clamping
   *
   * @param[in] amount The prediction amount in milliseconds
   */
  void SetPanGestureMaximumPredictionAmount( unsigned int amount );

  /**
   * @brief Sets the lower bound of the prediction amount for clamping
   *
   * @param[in] amount The prediction amount in milliseconds
   */
  void SetPanGestureMinimumPredictionAmount( unsigned int amount );

  /**
   * @brief Sets the prediction amount to adjust when the pan velocity is changed.
   * If the pan velocity is accelerating, the prediction amount will be increased
   * by the specified amount until it reaches the upper bound. If the pan velocity
   * is decelerating, the prediction amount will be decreased by the specified
   * amount until it reaches the lower bound.
   *
   * @param[in] amount The prediction amount in milliseconds
   */
  void SetPanGesturePredictionAmountAdjustment( unsigned int amount );

  /**
   * @brief Called to set how pan gestures smooth input
   *
   * @param[in] mode The smoothing mode to use
   */
  void SetPanGestureSmoothingMode( unsigned int mode );

  /**
   * @brief Sets the mode used to smooth pan gesture movement properties calculated on the Update thread
   *
   * @param[in] amount The smoothing amount [0.0f,1.0f] - 0.0f would be no smoothing, 1.0f maximum smoothing
   */
  void SetPanGestureSmoothingAmount( float amount );

  /**
   * @brief Sets the minimum distance required before a pan starts
   *
   * @param[in] distance The minimum distance before a pan starts
   */
  void SetMinimumPanDistance( int distance );

  /**
   * @brief Sets the minimum number of events required before a pan starts
   *
   * @param[in] events The minimum events before a pan starts
   */
  void SetMinimumPanEvents( int events );

  /**
   * @brief Sets how often the gles call logging occurs
   *
   * @param[in] time the number of seconds between logging output
   */
  void SetGlesCallTime( int time );

  /**
   * @brief Get the graphics status time
   */
  int GetGlesCallTime() const;

  /**
   * @return true if performance server is required
   */
  bool PerformanceServerRequired() const;

private:

  unsigned int mFpsFrequency;                     ///< how often fps is logged out in seconds
  unsigned int mUpdateStatusFrequency;            ///< how often update status is logged out in frames
  unsigned int mPerformanceStatsLevel;            ///< performance statistics logging bitmask
  unsigned int mPerformanceStatsFrequency;        ///< performance statistics logging frequency (seconds)
  unsigned int mPerformanceTimeStampOutput;       ///< performance time stamp output ( bitmask)
  unsigned int mPanGestureLoggingLevel;           ///< pan-gesture log level
  int mPanGesturePredictionMode;                  ///< prediction mode for pan gestures
  int mPanGesturePredictionAmount;                ///< prediction amount for pan gestures
  int mPanGestureMaxPredictionAmount;             ///< maximum prediction amount for pan gestures
  int mPanGestureMinPredictionAmount;             ///< minimum prediction amount for pan gestures
  int mPanGesturePredictionAmountAdjustment;      ///< adjustment of prediction amount for pan gestures
  int mPanGestureSmoothingMode;                  ///< prediction mode for pan gestures
  float mPanGestureSmoothingAmount;              ///< prediction amount for pan gestures
  int mPanMinimumDistance;                        ///< minimum distance required before pan starts
  int mPanMinimumEvents;                          ///< minimum events required before pan starts
  int mGlesCallTime;                              ///< time in seconds between status updates

  Dali::Integration::Log::LogFunction mLogFunction;

  // Undefined copy constructor.
  EnvironmentOptions( const EnvironmentOptions& );

  // Undefined assignment operator.
  EnvironmentOptions& operator=( const EnvironmentOptions& );

};

} // Adaptor
} // Internal
} // Dali

#endif // __DALI_INTERNAL_ADAPTOR_ENVIRONMENT_OPTIONS_H__
