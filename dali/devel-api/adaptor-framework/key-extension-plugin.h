#ifndef DALI_KEY_EXTENSION_PLUGIN_H
#define DALI_KEY_EXTENSION_PLUGIN_H

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
#include <cstdlib>

namespace Dali
{

/**
 * @brief KeyExtensionPlugin is an abstract interface, used by dali-adaptor to access key extension plugin.
 * A concrete implementation must be created for each platform and provided as dynamic library.
 * @SINCE_1_2.41
 */
class KeyExtensionPlugin
{
public:

  struct KeyLookup
  {
    const char* keyName;          ///< XF86 key name
    const int   daliKeyCode;      ///< Dali key code
    const bool  deviceButton;     ///< Whether the key is from a button on the device
  };

  /**
   * @brief Constructor.
   * @SINCE_1_2.41
   */
  KeyExtensionPlugin(){}

  /**
   * @brief Destructor.
   * @SINCE_1_2.41
   */
  virtual ~KeyExtensionPlugin(){}

  /**
   * @brief Get extension key lookup table.
   *
   * @SINCE_1_2.41
   * @return Pointer of extension Key lookup table.
   */
  virtual KeyLookup* GetKeyLookupTable() = 0;

  /**
   * @brief Get count of extension key lookup table.
   *
   * @SINCE_1_2.41
   * @return count of extension Key lookup table.
   */
  virtual std::size_t GetKeyLookupTableCount() = 0;
};

} // namespace Dali;

#endif
