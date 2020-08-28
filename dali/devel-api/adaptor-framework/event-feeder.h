#ifndef DALI_EVENT_FEEDER_H
#define DALI_EVENT_FEEDER_H

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

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{

struct KeyEvent;
struct WheelEvent;
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
DALI_ADAPTOR_API void FeedTouchPoint( TouchPoint& point, int timeStamp );

/**
 * Feed a wheel event to the adaptor.
 *
 * @param[in]  wheelEvent wheel event
 *
 * @note For testing/automation purposes only.
 */
DALI_ADAPTOR_API void FeedWheelEvent( WheelEvent& wheelEvent );

/**
 * Feed a key event to the adaptor.
 *
 * @param[in] keyEvent The key event holding the key information.
 *
 * @note For testing/automation purposes only.
 */
DALI_ADAPTOR_API void FeedKeyEvent( KeyEvent& keyEvent );

} // namespace EventFeeder

} // namespace Dali

#endif // DALI_EVENT_FEEDER_H
