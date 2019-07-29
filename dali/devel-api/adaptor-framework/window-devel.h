#ifndef DALI_WINDOW_DEVEL_H
#define DALI_WINDOW_DEVEL_H

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
#include <dali/public-api/adaptor-framework/window.h>

namespace Dali
{
class KeyEvent;
class TouchData;
class WheelEvent;
class RenderTaskList;

namespace DevelWindow
{

typedef Signal< void () > EventProcessingFinishedSignalType;       ///< Event Processing finished signal type

typedef Signal< void (const KeyEvent&) > KeyEventSignalType;       ///< Key event signal type

typedef Signal< bool (const KeyEvent&) > KeyEventGeneratedSignalType;       ///< Key event generated signal type

typedef Signal< void (const TouchData&) > TouchSignalType;         ///< Touch signal type

typedef Signal< void (const WheelEvent&) > WheelEventSignalType;   ///< Touched signal type

/**
 * @brief Sets position and size of the window. This API guarantees that both moving and resizing of window will appear on the screen at once.
 *
 * @param[in] window The window instance
 * @param[in] positionSize The new window position and size
 */
DALI_ADAPTOR_API void SetPositionSize( Window window, PositionSize positionSize );

/**
 * @brief Retrieves the list of render-tasks in the window.
 *
 * @param[in] window The window instance
 * @return A valid handle to a RenderTaskList
 */
DALI_ADAPTOR_API Dali::RenderTaskList GetRenderTaskList( Window window );

/**
 * @brief Retrieve the window that the given actor is added to.
 *
 * @param[in] actor The actor
 * @return The window the actor is added to or an empty handle if the actor is not added to any window.
 */
DALI_ADAPTOR_API Window Get( Actor actor );

/**
 * @brief This signal is emitted just after the event processing is finished.
 *
 * @param[in] window The window instance
 * @return The signal to connect to
 */
DALI_ADAPTOR_API EventProcessingFinishedSignalType& EventProcessingFinishedSignal( Window window );

/**
 * @brief This signal is emitted when key event is received.
 *
 * A callback of the following type may be connected:
 * @code
 *   void YourCallbackName(const KeyEvent& event);
 * @endcode
 * @param[in] window The window instance
 * @return The signal to connect to
 */
DALI_ADAPTOR_API KeyEventSignalType& KeyEventSignal( Window window );

/**
 * @brief This signal is emitted when key event is received.
 *
 * A callback of the following type may be connected:
 * @code
 *   bool YourCallbackName(const KeyEvent& event);
 * @endcode
 * @param[in] window The window instance
 * @return The signal to connect to
 */
DALI_ADAPTOR_API KeyEventGeneratedSignalType& KeyEventGeneratedSignal( Window window );

/**
 * @brief This signal is emitted when the screen is touched and when the touch ends
 * (i.e. the down & up touch events only).
 *
 * If there are multiple touch points, then this will be emitted when the first touch occurs and
 * then when the last finger is lifted.
 * An interrupted event will also be emitted (if it occurs).
 * A callback of the following type may be connected:
 * @code
 *   void YourCallbackName( TouchData event );
 * @endcode
 *
 * @param[in] window The window instance
 * @return The touch signal to connect to
 * @note Motion events are not emitted.
 */
DALI_ADAPTOR_API TouchSignalType& TouchSignal( Window window );

/**
 * @brief This signal is emitted when wheel event is received.
 *
 * A callback of the following type may be connected:
 * @code
 *   void YourCallbackName(const WheelEvent& event);
 * @endcode
 * @param[in] window The window instance
 * @return The signal to connect to
 */
DALI_ADAPTOR_API WheelEventSignalType& WheelEventSignal( Window window );

} // namespace DevelWindow

} // namespace Dali

#endif // DALI_WINDOW_DEVEL_H
