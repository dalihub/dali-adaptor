#ifndef __DALI_EVENT_FEEDER_H_
#define __DALI_EVENT_FEEDER_H_

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

// EXTERNAL INCLUDES
#include <dali/public-api/common/dali-common.h>

namespace Dali DALI_IMPORT_API
{

struct KeyEvent;
struct MouseWheelEvent;
struct TouchPoint;

namespace EventFeeder
{

/**
 * Feed a touch point to the adaptor.
 *
 * @param[in] point touch point
 * @param[in] timeStamp time value of event
 *
 * @note For testing/automation purposes only.
 */
void FeedTouchPoint( TouchPoint& point, int timeStamp );

/**
 * Feed a mouse wheel event to the adaptor.
 *
 * @param[in]  wheelEvent mouse wheel event
 *
 * @note For testing/automation purposes only.
 */
void FeedWheelEvent( MouseWheelEvent& wheelEvent );

/**
 * Feed a key event to the adaptor.
 *
 * @param[in] keyEvent The key event holding the key information.
 *
 * @note For testing/automation purposes only.
 */
void FeedKeyEvent( KeyEvent& keyEvent );

} // namespace EventFeeder

} // namespace Dali

#endif // __DALI_EVENT_FEEDER_H_
