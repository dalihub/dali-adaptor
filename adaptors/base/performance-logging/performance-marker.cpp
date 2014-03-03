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

// CLASS HEADER
#include "performance-marker.h"

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

PerformanceMarker::PerformanceMarker( MarkerType type )
:mType(type)
{
}

PerformanceMarker::PerformanceMarker( MarkerType type, FrameTimeStamp frameInfo )
:mType(type),
 mTimeStamp(frameInfo)
{
}

unsigned int PerformanceMarker::MicrosecondDiff( const PerformanceMarker& start,const PerformanceMarker& end )
{
  return FrameTimeStamp::MicrosecondDiff( start.mTimeStamp, end.mTimeStamp );
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

