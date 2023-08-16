#ifndef DALI_WINDOW_DEVEL_MOUSE_RELATIVE_EVENT_H
#define DALI_WINDOW_DEVEL_MOUSE_RELATIVE_EVENT_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/events/device.h>
#include <dali/public-api/math/vector2.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace DevelWindow
{
/**
 * @brief MouseRelativeEvent occurs when relative mouse movement occurs in the window.
 *
 * A signal is emitted when relative mouse movement occurs in the window.
 */
struct DALI_ADAPTOR_API MouseRelativeEvent
{
  enum class Type
  {
    NONE = 0,
    RELATIVE_MOVE
  };

  /**
   * @brief Constructor which creates a MouseRelativeEvent instance
   * @param[in] type The type of the event.
   * @param[in] modifiers The modifier keys pressed during the event (such as shift, alt and control).
   * @param[in] timeStamp The time when the event being started.
   * @param[in] diffPosition The co-ordinates of the cursor relative to the top-left of the screen
   * @param[in] unaccelatedPosition The co-ordinates of the cursor relative to the top-left of the screen
   * @param[in] deviceClass The device class the event originated from.
   * @param[in] deviceSubclass The device subclass the event originated from.
   */
  MouseRelativeEvent(Type type, uint32_t modifiers, uint32_t timeStamp, Vector2 diffPosition, Vector2 unaccelatedPosition, const Device::Class::Type deviceClass, const Device::Subclass::Type deviceSubclass)
  : type(type),
    modifiers(modifiers),
    timeStamp(timeStamp),
    diffPosition(diffPosition),
    unaccelatedPosition(unaccelatedPosition),
    deviceClass(deviceClass),
    deviceSubclass(deviceSubclass)
  {
  }

  Type                         type;
  uint32_t                     modifiers;
  uint32_t                     timeStamp;
  Vector2                      diffPosition;
  Vector2                      unaccelatedPosition;
  const Device::Class::Type    deviceClass;
  const Device::Subclass::Type deviceSubclass;
};

} // namespace DevelWindow

} // namespace Dali

#endif // DALI_WINDOW_DEVEL_MOUSE_RELATIVE_EVENT_H
