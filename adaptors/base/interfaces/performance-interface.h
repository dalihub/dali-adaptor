#ifndef __DALI_INTERNAL_BASE_PERFORMANCE_INTERFACE_H__
#define __DALI_INTERNAL_BASE_PERFORMANCE_INTERFACE_H__

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

// INTERNAL INCLUDES
#include <base/performance-logging/performance-marker.h>


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

  /**
   * bitmask of logging options
   */
  enum LogLevel
  {
    DISABLED             = 0,
    LOG_UPDATE_RENDER    = 1 << 0, ///< Bit 0, log update and render times
    LOG_EVENT_PROCESS    = 1 << 1, ///< Bit 1, log event process times
    LOG_EVENTS_TO_KERNEL = 1 << 2, ///< Bit 2, log all events to kernel trace
  };

  /**
   * Constructor.
   */
  PerformanceInterface() {}

  /**
   * Virtual destructor
   */
  virtual ~PerformanceInterface() {};

  /**
   * Add a performance marker
   * This function can be called from ANY THREAD.
   * @param markerType performance marker type
   */
  virtual void AddMarker( PerformanceMarker::MarkerType markerType) = 0;

  /**
   * Set the logging level and frequency
   * @param level 0 = disabled, 1 = enabled
   * @param logFrequency how often to log out in seconds
   */
  virtual void SetLogging( unsigned int level, unsigned int logFrequency) = 0;

private:

  // Undefined copy constructor.
  PerformanceInterface( const PerformanceInterface& );

  // Undefined assignment operator.
  PerformanceInterface& operator=( const PerformanceInterface& );

};

} // namespace Internal

} // namespace Adaptor

} // namespace Dali

#endif
