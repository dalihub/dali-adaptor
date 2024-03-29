#ifndef DALI_INTERNAL_BASE_PERFORMANCE_INTERFACE_H
#define DALI_INTERNAL_BASE_PERFORMANCE_INTERFACE_H

/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
/**
 * Abstract Performance Interface.
 * Used by the Adaptor to store performance metrics.
 *
 */
class PerformanceInterface
{
public:
  typedef unsigned short ContextId; ///< Type to represent a context ID

  /**
   * bitmask of statistics logging options.
   * Used for output data like min/max/average time spent in event, update,render and custom tasks.
   * E.g.
   * Event, min 0.04 ms, max 5.27 ms, total (0.1 secs), avg 0.28 ms, std dev 0.73 ms
   * Update, min 0.29 ms, max 0.91 ms, total (0.5 secs), avg 0.68 ms, std dev 0.15 ms
   * Render, min 0.33 ms, max 0.97 ms, total (0.6 secs), avg 0.73 ms, std dev 0.17 ms
   * TableViewInit, min 76.55 ms, max 76.55 ms, total (0.1 secs), avg 76.55 ms, std dev 0.00 ms
   */
  enum StatisticsLogOptions
  {
    DISABLED           = 0,
    LOG_EVERYTHING     = 1 << 0, ///< Bit 0 (1), log all statistics to the DALi log
    LOG_UPDATE_RENDER  = 1 << 1, ///< Bit 1 (2), log update and render statistics to the DALi log
    LOG_EVENT_PROCESS  = 1 << 2, ///< Bit 2 (4), log event task statistics to the DALi log
    LOG_CUSTOM_MARKERS = 1 << 3, ///< Bit 3 (8), log custom marker statistics to the DALi log
  };

  /**
   * bitmask of time stamp output options.
   * E.g. DALI_PERFORMANCE_TIMESTAMP_OUTPUT = 1 dali-demo
   * Used for logging out time stamped markers for detailed analysis (see MarkerType, for the markers logged)
   * Typical output would look like:
   *   379.059025 (seconds), V_SYNC
   *   379.059066 (seconds), UPDATE_START
   *   379.059747 (seconds), UPDATE_END
   *   379.059820 (seconds), RENDER_START
   *   379.060708 (seconds), RENDER_END
   *   379.075795 (seconds), V_SYNC
   *   379.076444 (seconds), MY_CUSTOM_MARKER_START  ( customer marker using PerformanceLogger public API).
   *   379.077353 (seconds), MY_CUSTOM_MARKER_END  ( customer marker using PerformanceLogger public API).
   */
  enum TimeStampOutput
  {
    NO_TIME_STAMP_OUTPUT = 0,
    OUTPUT_DALI_LOG      = 1 << 0, ///< Bit 0 (1), log markers to DALi log
    OUTPUT_KERNEL_TRACE  = 1 << 1, ///< Bit 1 (2), log makers to kernel trace
    OUTPUT_SYSTEM_TRACE  = 1 << 2, ///< Bit 2 (4), log markers to system trace
    OUTPUT_NETWORK       = 1 << 3, ///< Bit 3 (8), log markers to network client
  };

  /**
   * enum for difference performance markers.
   * Please modify the name lookup table in performance-interface.cpp
   * file if adding new markers (the order must match one to one).
   */
  enum MarkerType
  {
    VSYNC = 0,            ///< V-Sync
    UPDATE_START,         ///< Update start
    UPDATE_END,           ///< Update end
    RENDER_START,         ///< Render start
    RENDER_END,           ///< Render end
    SWAP_START,           ///< SwapBuffers Start
    SWAP_END,             ///< SwapBuffers End
    PROCESS_EVENTS_START, ///< Process events start (e.g. touch event)
    PROCESS_EVENTS_END,   ///< Process events end
    PAUSED,               ///< Pause start
    RESUME,               ///< Resume start
    START,                ///< The start of custom tracking
    END                   ///< The end of custom tracking
  };

  /**
   * Constructor.
   */
  PerformanceInterface()
  {
  }

  /**
   * Virtual destructor
   */
  virtual ~PerformanceInterface(){};

  /**
   * @brief Add a new context with a given name
   *
   * @param[in] name The name of the context
   * @return Return the unique id for this context
   */
  virtual ContextId AddContext(const char* name) = 0;

  /**
   * @brief Get the context for a custom name.
   *
   * @param[in] name The name of the context
   * @return Return the unique id for this context
   */
  virtual ContextId GetContextId( const char* name ) = 0;

  /**
   * @brief Remove a context from use
   *
   * @param[in] contextId The ID of the context to remove
   */
  virtual void RemoveContext(ContextId contextId) = 0;

  /**
   * @brief Add a performance marker
   * This function can be called from ANY THREAD.
   * The default context 0 Event/Update/Render is assumed.
   * @param[in] markerType performance marker type
   */
  virtual void AddMarker(MarkerType markerType) = 0;

  /**
   * @brief Add a performance marker for a used defined context
   * This function can be called from ANY THREAD.
   * @param[in] markerType performance marker type
   * @param[in] contextId The context of the marker. This must be one generated by AddContext.
   */
  virtual void AddMarker(MarkerType markerType, ContextId contextId) = 0;

  /**
   * @brief Set the logging level and frequency
   * @param[in] StatisticsLogOptions  0 = disabled, >0 bitmask of StatisticsLogOptions
   * @param[in] timeStampOutput 0 = disabled, > 0 bitmask of TimeStampOutput options.
   * @param[in] logFrequency how often to log out in seconds
   */
  virtual void SetLogging(unsigned int statisticsLogOptions, unsigned int timeStampOutput, unsigned int logFrequency) = 0;

  /**
   * @brief Set the logging frequency for an individual context
   *
   * @param[in] logFrequency how often to log out in seconds
   */
  virtual void SetLoggingFrequency(unsigned int logFrequency, ContextId contextId) = 0;

  /**
   * @brief Set logging on or off for a particular context
   *
   * @param[in] enable Enable logging or not
   * @param[in] contextId The id of the context to log. This must be one generated by AddContext.
   */
  virtual void EnableLogging(bool enable, ContextId contextId) = 0;

private:
  // Undefined copy constructor.
  PerformanceInterface(const PerformanceInterface&);

  // Undefined assignment operator.
  PerformanceInterface& operator=(const PerformanceInterface&);
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif
