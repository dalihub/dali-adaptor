#ifndef DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_ACTIONS_H
#define DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_ACTIONS_H
/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#include <dali/public-api/actors/actor.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <cstdint>

namespace Dali
{
namespace Devel
{
namespace Accessibility
{

/**
 * @brief Enumeration describing type of gesture.
 */
enum class Gesture : int32_t
{
  ONE_FINGER_HOVER,
  TWO_FINGER_HOVER,
  THREE_FINGER_HOVER,
  ONE_FINGER_FLICK_LEFT,
  ONE_FINGER_FLICK_RIGHT,
  ONE_FINGER_FLICK_UP,
  ONE_FINGER_FLICK_DOWN,
  TWO_FINGERS_FLICK_LEFT,
  TWO_FINGERS_FLICK_RIGHT,
  TWO_FINGERS_FLICK_UP,
  TWO_FINGERS_FLICK_DOWN,
  THREE_FINGERS_FLICK_LEFT,
  THREE_FINGERS_FLICK_RIGHT,
  THREE_FINGERS_FLICK_UP,
  THREE_FINGERS_FLICK_DOWN,
  ONE_FINGER_SINGLE_TAP,
  ONE_FINGER_DOUBLE_TAP,
  ONE_FINGER_TRIPLE_TAP,
  TWO_FINGERS_SINGLE_TAP,
  TWO_FINGERS_DOUBLE_TAP,
  TWO_FINGERS_TRIPLE_TAP,
  THREE_FINGERS_SINGLE_TAP,
  THREE_FINGERS_DOUBLE_TAP,
  THREE_FINGERS_TRIPLE_TAP,
  ONE_FINGER_FLICK_LEFT_RETURN,
  ONE_FINGER_FLICK_RIGHT_RETURN,
  ONE_FINGER_FLICK_UP_RETURN,
  ONE_FINGER_FLICK_DOWN_RETURN,
  TWO_FINGERS_FLICK_LEFT_RETURN,
  TWO_FINGERS_FLICK_RIGHT_RETURN,
  TWO_FINGERS_FLICK_UP_RETURN,
  TWO_FINGERS_FLICK_DOWN_RETURN,
  THREE_FINGERS_FLICK_LEFT_RETURN,
  THREE_FINGERS_FLICK_RIGHT_RETURN,
  THREE_FINGERS_FLICK_UP_RETURN,
  THREE_FINGERS_FLICK_DOWN_RETURN,
  ONE_FINGER_DOUBLE_TAP_N_HOLD,
  TWO_FINGERS_DOUBLE_TAP_N_HOLD,
  THREE_FINGERS_DOUBLE_TAP_N_HOLD,
  MAX_COUNT
};

/**
 * @brief Enumeration indicating current state of gesture.
 */
enum class GestureState : int32_t
{
  BEGIN,
  ONGOING,
  ENDED,
  ABORTED
};

enum class ActionType
{
  ACTIVATE,
  ESCAPE,
  INCREMENT,
  DECREMENT,
  SCROLL_TO_CHILD,
  MAX_COUNT
};

/**
 * @brief Structure containing all values needed to invoke Accessible::DoGesture.
 */
struct DALI_ADAPTOR_API GestureInfo
{
  GestureInfo() = default;
  GestureInfo(Gesture type, int32_t startPositionX, int32_t endPositionX, int32_t startPositionY, int32_t endPositionY, GestureState state, uint32_t eventTime)
  : type(type),
    startPointX(startPositionX),
    endPointX(endPositionX),
    startPointY(startPositionY),
    endPointY(endPositionY),
    state(state),
    eventTime(eventTime)
  {
  }

  Gesture      type{};
  int32_t      startPointX{};
  int32_t      endPointX{};
  int32_t      startPointY{};
  int32_t      endPointY{};
  GestureState state{};
  uint32_t     eventTime{};
};

struct DALI_ADAPTOR_API ActionInfo
{
  ActionInfo() = default;
  ActionInfo(ActionType type, Actor target)
  : type{type},
    target{target}
  {
  }

  ActionType type{ActionType::MAX_COUNT};
  Actor      target{};
};

} // namespace Accessibility
} // namespace Devel
} // namespace Dali

#endif // DALI_DEVEL_API_ADAPTOR_FRAMEWORK_ACCESSIBILITY_ACTIONS_H
