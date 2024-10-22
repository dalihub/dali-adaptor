#ifndef DALI_INTERNAL_ADDON_MANAGER_IMPL
#define DALI_INTERNAL_ADDON_MANAGER_IMPL

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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

#include <memory>
#include <string>
#include <vector>

namespace Dali
{
namespace Internal
{
/**
 * Base class for platform-specific implementation of AddOnManager
 */
class AddOnManager
{
protected:
  /**
   * @brief Constructor
   */
  AddOnManager() = default;

public:
  /**
   * @brief Destructor
   */
  virtual ~AddOnManager() = default;

  /**
   * @brief Registers the dispatch table with AddOnManager.
   * @param[in] dispatchTable Pointer to the valid dispatch table
   */
  virtual void RegisterAddOnDispatchTable(const AddOnDispatchTable* dispatchTable) = 0;

  /**
   * @brief Retrieves list of the available AddOns
   * @return List of AddOn names
   */
  virtual std::vector<std::string> EnumerateAddOns() = 0;

  /**
   * @brief Returns AddOnInfo structure for specified AddOn name
   * @param[in] name Name of AddOn
   * @param[out]] info Output reference
   * @return True on success, False if extension info cannot be retrieved
   */
  virtual bool GetAddOnInfo(const std::string& name, AddOnInfo& info) = 0;

  /**
   * @brief Loads and initialises specified AddOns
   * @param[in] extensionNames Array of extension names
   * @return vector of initialised AddOnLibrary handles
   */
  virtual std::vector<Dali::AddOnLibrary> LoadAddOns(const std::vector<std::string>& addonNames) = 0;

  /**
   * @copydoc Integration::AddOnManager::LoadAddOn
   */
  virtual AddOnLibrary LoadAddOn(const std::string& addonName, const std::string& libraryName) = 0;

  /**
   * @brief Returns AddOn global function pointer
   * @param[in] addOnLibrary valid AddOn library object
   * @param[in] procName Name of the function to retrieve
   * @return Pointer to the function or null if function doesn't exist
   */
  virtual void* GetGlobalProc(const Dali::AddOnLibrary& addonHandle, const char* procName) = 0;

  /**
   * @brief Returns addon instance function pointer
   * @param[in] addOnLibrary valid AddOn library object
   * @param[in] procName Name of the function to retrieve
   * @return Pointer to the function or null if function doesn't exist
   */
  virtual void* GetInstanceProc(const Dali::AddOnLibrary& addonHandle, const char* procName) = 0;

  /**
   * @brief Pause lifecycle event
   * Implementation is optional and depends whether AddOn needs to handle lifecycle event.
   */
  virtual void Pause()
  {
  }

  /**
   * @brief Resume lifecycle event
   * Implementation is optional and depends whether AddOn needs to handle lifecycle event.
   */
  virtual void Resume()
  {
  }

  /**
   * @brief Start lifecycle event
   * Implementation is optional and depends whether AddOn needs to handle lifecycle event.
   */
  virtual void Start()
  {
  }

  /**
   * @brief Stop lifecycle event
   * Implementation is optional and depends whether AddOn needs to handle lifecycle event.
   */
  virtual void Stop()
  {
  }
};

} // namespace Internal

} // namespace Dali

#endif // DALI_CMAKE_EXTENSION_MANAGER_IMPL
