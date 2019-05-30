#ifndef DALI_INTERNAL_ADAPTOR_FRAME_TIME_STAMP_H
#define DALI_INTERNAL_ADAPTOR_FRAME_TIME_STAMP_H

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
#include <stdint.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * Simple structure to hold information about an event in time
 * within Dali. For example when rendering started.
 */
struct FrameTimeStamp
{

  static const unsigned int BUFFER_NOT_USED = 2;  ///< no index buffer was used

    /**
     * Constructor
     */
    FrameTimeStamp();

    /**
     * Constructor
     * @param frame the frame number
     * @param microseconds the time from a monotonic clock
     * @param bufferIndex  double buffered index used for performing an update / render
     */
    FrameTimeStamp( unsigned int frame, uint64_t microseconds, unsigned int bufferIndex = BUFFER_NOT_USED );

    /**
     * Constructor
     * @param bufferIndex  double buffered index used for performing an update / render
     */
    FrameTimeStamp( unsigned int bufferIndex );

    /**
     * @param start start time
     * @param end end time
     * @return difference in microseconds between two time stamps
     */
    static unsigned int MicrosecondDiff( const FrameTimeStamp& start,const FrameTimeStamp& end );

    unsigned int frame;            ///< Frame number ( not always available)
    uint64_t     microseconds;     ///< Microsecond time stamp
    unsigned int bufferIndex;      ///< The double buffered index used for performing an update / render
  };
} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_FRAME_TIME_STAMP_H

