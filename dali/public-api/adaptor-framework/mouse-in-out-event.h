#ifndef DALI_MOUSE_IN_OUT_EVENT_H
#define DALI_MOUSE_IN_OUT_EVENT_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/common/unique-ptr.h>
#include <dali/public-api/events/device.h>
#include <dali/public-api/math/vector2.h>

// INTERNAL INCLUDES
#include <dali/public-api/dali-adaptor-common.h>

/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace Dali
{
/**
 * @brief MouseInOutEvent occurs when the mouse enters or leaves the window.
 *
 * A signal is emitted whenever the mouse enters or leaves the window.
 * @SINCE_2_5.28
 */
class DALI_ADAPTOR_API MouseInOutEvent
{
public:
  /**
   * @brief Enumeration for the type of the mouse in/out event.
   * @SINCE_2_5.28
   */
  enum class Type
  {
    NONE = 0, ///< No event @SINCE_2_5.28
    IN,       ///< Mouse entered the window @SINCE_2_5.28
    OUT       ///< Mouse left the window @SINCE_2_5.28
  };

  /**
   * @brief Constructor.
   * @SINCE_2_5.28
   * @param[in] type The type of the event
   * @param[in] modifiers The modifier keys pressed during the event (such as shift, alt and control)
   * @param[in] point The co-ordinates of the cursor relative to the top-left of the screen
   * @param[in] timeStamp The time when the event started
   * @param[in] deviceClass The device class the event originated from
   * @param[in] deviceSubclass The device subclass the event originated from
   */
  MouseInOutEvent(Type type, uint32_t modifiers, Vector2 point, uint32_t timeStamp, Device::Class::Type deviceClass, Device::Subclass::Type deviceSubclass);

  /**
   * @brief Destructor.
   * @SINCE_2_5.28
   */
  ~MouseInOutEvent();

  /**
   * @brief Copy constructor.
   * @SINCE_2_5.28
   * @param[in] rhs A reference to the copied instance
   */
  MouseInOutEvent(const MouseInOutEvent& rhs);

  /**
   * @brief Copy assignment operator.
   * @SINCE_2_5.28
   * @param[in] rhs A reference to the copied instance
   * @return A reference to this
   */
  MouseInOutEvent& operator=(const MouseInOutEvent& rhs);

  /**
   * @brief Returns the type of the event.
   * @SINCE_2_5.28
   * @return The event type
   */
  Type GetType() const;

  /**
   * @brief Returns the modifier keys pressed during the event.
   * @SINCE_2_5.28
   * @return The modifier keys bitmask
   */
  uint32_t GetModifiers() const;

  /**
   * @brief Returns the co-ordinates of the cursor.
   * @SINCE_2_5.28
   * @return The cursor position relative to the top-left of the screen
   */
  const Vector2& GetPoint() const;

  /**
   * @brief Returns the time stamp of the event.
   * @SINCE_2_5.28
   * @return The time when the event started
   */
  uint32_t GetTimeStamp() const;

  /**
   * @brief Returns the device class the event originated from.
   * @SINCE_2_5.28
   * @return The device class
   */
  Device::Class::Type GetDeviceClass() const;

  /**
   * @brief Returns the device subclass the event originated from.
   * @SINCE_2_5.28
   * @return The device subclass
   */
  Device::Subclass::Type GetDeviceSubclass() const;

private:
  struct Impl;
  Dali::UniquePtr<Impl> mImpl;
};

} // namespace Dali

/**
 * @}
 */
#endif // DALI_MOUSE_IN_OUT_EVENT_H
