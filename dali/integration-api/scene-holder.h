#ifndef DALI_INTEGRATION_SCENEHOLDER_H
#define DALI_INTEGRATION_SCENEHOLDER_H

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
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/math/vector4.h>
#include <dali/public-api/signals/dali-signal.h>

namespace Dali
{

class Actor;
class Layer;
class Any;
class TouchData;
struct TouchPoint;
struct WheelEvent;
struct KeyEvent;

namespace Internal DALI_INTERNAL
{

namespace Adaptor
{

class SceneHolder;

}

}

namespace Integration
{

/**
 * @brief SceneHolder is responsible for creating a Scene for rendering.
 */
class DALI_ADAPTOR_API SceneHolder : public BaseHandle
{
public:

  typedef Signal< void (const Dali::KeyEvent&) > KeyEventSignalType;          ///< Key event signal type

  typedef Signal< bool (const Dali::KeyEvent&) > KeyEventGeneratedSignalType; ///< Key event generated signal type

  typedef Signal< void (const Dali::TouchData&) > TouchSignalType;            ///< Touch signal type

  typedef Signal< void (const Dali::WheelEvent&) > WheelEventSignalType;      ///< Touched signal type

  /**
   * @brief Create an uninitialized SceneHolder handle.
   */
  SceneHolder();

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~SceneHolder();

  /**
   * @brief This copy constructor is required for (smart) pointer semantics.
   *
   * @param [in] handle A reference to the copied handle
   */
  SceneHolder( const SceneHolder& handle );

  /**
   * @brief This assignment operator is required for (smart) pointer semantics.
   *
   * @param [in] rhs  A reference to the copied handle
   * @return A reference to this
   */
  SceneHolder& operator=( const SceneHolder& rhs );

  /**
   * @brief Adds a child Actor to the SceneHolder.
   *
   * The child will be referenced.
   * @param[in] actor The child
   * @pre The actor has been initialized.
   * @pre The actor does not have a parent.
   */
  void Add( Actor actor );

  /**
   * @brief Removes a child Actor from the SceneHolder.
   *
   * The child will be unreferenced.
   * @param[in] actor The child
   * @pre The actor has been added to the SceneHolder.
   */
  void Remove( Actor actor );

  /**
   * @brief Returns the Scene's Root Layer.
   *
   * @return The root layer
   */
  Layer GetRootLayer() const;

  /**
   * @brief Sets the background color.
   *
   * @param[in] color The new background color
   */
  void SetBackgroundColor( Vector4 color );

  /**
   * @brief Gets the background color.
   *
   * @return The background color
   */
  Vector4 GetBackgroundColor() const;

  /**
   * @brief Gets the native handle.
   *
   * When users call this function, it wraps the actual type used by the underlying system.
   *
   * @return The native handle or an empty handle
   */
  Any GetNativeHandle() const;

  /**
   * @brief Feed (Send) touch event to core
   * @param[in] point The touch point
   * @param[in] timeStamp The time stamp
   */
  void FeedTouchPoint( Dali::TouchPoint& point, int timeStamp );

  /**
   * @brief Feed (Send) wheel event to core
   * @param[in] wheelEvent The wheel event
   */
  void FeedWheelEvent( Dali::WheelEvent& wheelEvent );

  /**
   * @brief Feed (Send) key event to core
   * @param[in] keyEvent The key event holding the key information.
   */
  void FeedKeyEvent( Dali::KeyEvent& keyEvent );

  /**
   * @brief Retrieve the SceneHolder that the given actor is added to.
   *
   * @param[in] actor The actor
   * @return The SceneHolder the actor is added to or an empty handle if the actor is not added to any SceneHolder.
   */
  static SceneHolder Get( Actor actor );

  /**
   * @brief This signal is emitted when key event is received.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(const KeyEvent& event);
   * @endcode
   * @return The signal to connect to
   */
  KeyEventSignalType& KeyEventSignal();

  /**
   * @brief This signal is emitted when key event is received.
   *
   * A callback of the following type may be connected:
   * @code
   *   bool YourCallbackName(const KeyEvent& event);
   * @endcode
   * @return The signal to connect to
   */
  KeyEventGeneratedSignalType& KeyEventGeneratedSignal();

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
   * @return The touch signal to connect to
   * @note Motion events are not emitted.
   */
  TouchSignalType& TouchSignal();

  /**
   * @brief This signal is emitted when wheel event is received.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(const WheelEvent& event);
   * @endcode
   * @return The signal to connect to
   */
  WheelEventSignalType& WheelEventSignal();

public: // Not intended for application developers

  /**
   * @brief This constructor is used internally to create additional SceneHolder handles.
   *
   * @param[in] sceneHolder A pointer to a newly allocated Dali resource
   */
  explicit SceneHolder( Internal::Adaptor::SceneHolder* sceneHolder );

};

} // namespace Integration

} // namespace Dali

#endif // DALI_INTEGRATION_SCENEHOLDER_H
