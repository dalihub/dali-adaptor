#ifndef DALI_WINDOW_DEVEL_DEVICE_INFO_EVENT_H
#define DALI_WINDOW_DEVEL_DEVICE_INFO_EVENT_H

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
 * @brief DeviceInfoEvent occurs when a device such as a mouse or keyboard is connected or disconnected.
 *
 * A signal is emitted whenever when a device such as a mouse or keyboard is connected or disconnected.
 */
struct DALI_ADAPTOR_API DeviceInfoEvent
{
  enum class Type
  {
    NONE = 0,
    CONNECTED,
    DISCONNECTED
  };

  /**
   * @brief Constructor which creates a DeviceInfoEvent instance
   * @param[in] type The type of the event.
   * @param[in] name The device name.
   * @param[in] identifier The identifier.
   * @param[in] seatname The seat name.
   * @param[in] deviceClass The device class the event originated from.
   * @param[in] deviceSubclass The device subclass the event originated from.
   */
  DeviceInfoEvent(Type type, const std::string& name, const std::string& identifier, const std::string& seatname, const Device::Class::Type deviceClass, const Device::Subclass::Type deviceSubclass)
  : type(type),
    name(name),
    identifier(identifier),
    seatname(seatname),
    deviceClass(deviceClass),
    deviceSubclass(deviceSubclass)
  {
  }

  Type                         type;
  const std::string            name;
  const std::string            identifier;
  const std::string            seatname;
  const Device::Class::Type    deviceClass;
  const Device::Subclass::Type deviceSubclass;
};

} // namespace DevelWindow

} // namespace Dali

#endif // DALI_WINDOW_DEVEL_DEVICE_INFO_EVENT_H
