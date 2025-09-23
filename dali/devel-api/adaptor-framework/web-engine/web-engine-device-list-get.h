#ifndef DALI_WEB_ENGINE_DEVICE_LIST_GET_H
#define DALI_WEB_ENGINE_DEVICE_LIST_GET_H

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

// EXTERNAL INCLUDES
#include <list>
#include <string>

namespace Dali
{

/**
 * @brief A class WebEngineDeviceListGet for getting connection device list of web engine.
 */
class WebEngineDeviceListGet
{
public:
  /**
   * @brief A item structure to be stored in the device_list.
   */
  struct DeviceItem
  {
    std::string device_id;
    std::string label;
    int32_t     device_type;
    bool        connected;
  };

  /**
   * @brief Constructor.
   */
  WebEngineDeviceListGet() = default;

  /**
   * @brief Destructor.
   */
  virtual ~WebEngineDeviceListGet() = default;

  /**
   * @brief Get size of device_list.
   * @return Size of device_list.
   */
  virtual int GetSize() = 0;

  /**
   * @brief Reset the device_list.
   */
  virtual void Reset() = 0;

  /**
   * @brief Get items by index.
   */
  virtual void GetTypeAndConnect(int32_t* type, bool* connect, int index) = 0;

  /**
   * @brief Get device id by index.
   * @return Device id corresponding to the index.
   */
  virtual std::string GetDeviceId(int idx) = 0;

  /**
   * @brief Get device label by index.
   * @return Device label corresponding to the index.
   */
  virtual std::string GetDeviceLabel(int idx) = 0;
};
} // namespace Dali

#endif // DALI_WEB_ENGINE_DEVICE_LIST_GET_H
