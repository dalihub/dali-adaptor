#ifndef __DALI_INTERNAL_ADAPTOR_PERFORMANCE_MARKER_H__
#define __DALI_INTERNAL_ADAPTOR_PERFORMANCE_MARKER_H__

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

// INTERNAL INCLUDES
#include <base/interfaces/performance-interface.h>
#include <base/performance-logging/frame-time-stamp.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Marker used to record an event with a time stamp in Dali
 */
class PerformanceMarker
{
public:

  /**
   * Constructor
   * @param type marker type
   */
  PerformanceMarker( PerformanceInterface::MarkerType type );

  /**
   * Constructor
   * @param type marker type
   * @param time time stamp
   */
  PerformanceMarker( PerformanceInterface::MarkerType type,  FrameTimeStamp time );

  /**
   * @return the time stamp
   */
  const FrameTimeStamp& GetTimeStamp() const
  {
    return mTimeStamp;
  }

  /**
   * @return the type of marker
   */
  PerformanceInterface::MarkerType GetType() const
  {
    return mType;
  }

  /**
   * @param start the start marker
   * @param end the end marker
   * @return difference in microseconds between two markers
   */
  static unsigned int MicrosecondDiff( const PerformanceMarker& start, const PerformanceMarker& end  );

private:

  PerformanceInterface::MarkerType mType;         ///< marker type
  FrameTimeStamp mTimeStamp;                      ///< frame time stamp

};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_ADAPTOR_PERFORMANCE_MARKER_H__
