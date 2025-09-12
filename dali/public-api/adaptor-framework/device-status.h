#ifndef DALI_DEVICE_STATUS_H
#define DALI_DEVICE_STATUS_H

/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

namespace Dali
{
/**
 * @addtogroup dali_adaptor_framework
 * @{
 */

namespace DeviceStatus
{
/**
 * @brief Struct for battery of the device.
 * @SINCE_1_2.62
 */
struct Battery
{
  /**
   * @brief Enumeration for battery status.
   * @SINCE_1_2.62
   */
  enum Status
  {
    NORMAL,         ///< Battery is over 5% @SINCE_1_2.62
    CRITICALLY_LOW, ///< Battery is under 5% @SINCE_1_2.62
    POWER_OFF       ///< Battery is under 1% @SINCE_1_2.62
  };
};

/**
 * @brief Struct for memory of the device.
 * @SINCE_1_2.62
 */
struct Memory
{
  /**
   * @brief Enumeration for memory status.
   * @SINCE_1_2.62
   */
  enum Status
  {
    NORMAL,        ///< Normal Status @SINCE_1_2.62
    LOW,           ///< Memory is low but not critical @SINCE_1_2.62
    CRITICALLY_LOW ///< Memory is critically low @SINCE_1_2.62
  };
};

/**
 * @brief Struct for orientation of the device.
 * @SINCE_2_2.1
 */
struct Orientation
{
  /**
   * @brief Enumeration for orientation status.
   * @SINCE_2_2.1
   */
  enum Status
  {
    ORIENTATION_0   = 0,   ///< The device is oriented in a natural position @SINCE_2_2.1
    ORIENTATION_90  = 90,  ///< The device's left side is at the top @SINCE_2_2.1
    ORIENTATION_180 = 180, ///< The device is upside down @SINCE_2_2.1
    ORIENTATION_270 = 270  ///< The device's right side is at the top @SINCE_2_2.1
  };
};

} // namespace DeviceStatus

/**
 * @}
 */
} // namespace Dali

#endif // DALI_DEVICE_STATUS_H
