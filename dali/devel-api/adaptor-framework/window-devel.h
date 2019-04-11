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

namespace DevelWindow
{

typedef Signal< void () > EventProcessingFinishedSignalType;       ///< Event Processing finished signal type

typedef Signal< void (const KeyEvent&) > KeyEventSignalType;       ///< Key event signal type

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
 * @brief Adds a child Actor to the Window.
 *
 * The child will be referenced.
 * @param[in] window The window instance
 * @param[in] actor The child
 * @pre The actor has been initialized.
 * @pre The actor does not have a parent.
 */
DALI_ADAPTOR_API void Add( Window window, Dali::Actor actor );

/**
 * @brief Removes a child Actor from the Window.
 *
 * The child will be unreferenced.
 * @param[in] window The window instance
 * @param[in] actor The child
 * @pre The actor has been added to the stage.
 */
DALI_ADAPTOR_API void Remove( Window window, Dali::Actor actor );

/**
 * @brief Sets the background color of the window.
 *
 * @param[in] window The window instance
 * @param[in] color The new background color
 */
DALI_ADAPTOR_API void SetBackgroundColor( Window window, Vector4 color );

/**
 * @brief Gets the background color of the surface.
 *
 * @param[in] window The window instance
 * @return The background color
 */
DALI_ADAPTOR_API Vector4 GetBackgroundColor( Window window );

/**
 * @brief Returns the Scene's Root Layer.
 *
 * @param[in] window The window instance
 * @return The root layer
 */
DALI_ADAPTOR_API Dali::Layer GetRootLayer( Window window );

/**
 * @brief Queries the number of on-stage layers.
 *
 * Note that a default layer is always provided (count >= 1).
 * @param[in] window The window instance
 * @return The number of layers
 */
DALI_ADAPTOR_API uint32_t GetLayerCount( Window window );

/**
 * @brief Retrieves the layer at a specified depth.
 *
 * @param[in] window The window instance
 * @param[in] depth The depth
 * @return The layer found at the given depth
 * @pre Depth is less than layer count; see GetLayerCount().
 */
DALI_ADAPTOR_API Dali::Layer GetLayer( Window window, uint32_t depth );

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
