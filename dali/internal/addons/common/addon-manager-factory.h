#ifndef DALI_ADDON_MANAGER_FACTORY
#define DALI_ADDON_MANAGER_FACTORY

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

#include <dali/integration-api/addon-manager.h>

namespace Dali
{
namespace Internal
{
/**
 * @brief The base AddOnManager factory class
 */
class AddOnManagerFactory
{
public:
  /**
   * @brief Creates AddOnManager
   * @return returns AddOnManager object or nullptr if no support
   */
  static Integration::AddOnManager* CreateAddOnManager();
};
} // namespace Internal
} // namespace Dali

#endif // DALI_ADDON_MANAGER_FACTORY