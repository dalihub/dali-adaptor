#ifndef __DALI_INTERNAL_PAN_GESTURE_PROFILING_H__
#define __DALI_INTERNAL_PAN_GESTURE_PROFILING_H__

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
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/common/vector-wrapper.h>

namespace Dali
{

namespace Internal
{

struct PanGestureProfiling
{
  struct Position
  {
    Position( unsigned int time, Vector2 position )
    : time( time ), position( position )
    {
    }

    unsigned int time;
    Vector2 position;
  };

  typedef std::vector< PanGestureProfiling::Position > PanPositionContainer;

  void PrintData() const;

  void PrintData( const PanPositionContainer& dataContainer, const char * const prefix ) const;

  void ClearData();

  PanPositionContainer mRawData;
  PanPositionContainer mLatestData;
  PanPositionContainer mAveragedData;
};


} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_PAN_GESTURE_PROFILING_H__
