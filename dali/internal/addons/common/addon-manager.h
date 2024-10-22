#ifndef DALI_ADAPTOR_COMMON_ADDON_MANAGER
#define DALI_ADAPTOR_COMMON_ADDON_MANAGER

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

namespace Dali
{
namespace Internal
{
class AddOnManager;
};

namespace Adaptor
{
class AddOnManager : public Dali::Integration::AddOnManager
{
public:
  /**
   * @brief Constructor
   * @param impl Pointer to the platform specific implementation
   */
  explicit AddOnManager(Internal::AddOnManager* impl);

  /**
   * @brief Destructor
   */
  ~AddOnManager() override;

  /**
   * @brief Registers a dispatch table for new AddOn. Dispatch table contains essential
   * functions that will be called by the AddOnManager. It also includes lifecycle
   * event callbacks.
   * @param[in] dispatchTable Valid pointer to the DispatchTable object
   */
  void RegisterAddOnDispatchTable(const AddOnDispatchTable* dispatchTable) override;

  /**
   * @brief Retrieves list of all the extensions available
   * @return List of AddOn names
   */
  std::vector<std::string> EnumerateAddOns() override;

  /**
   * @brief Returns AddOnInfo structure for specified extension name
   * @param[in] name Name of extension
   * @param[out]] info Output reference
   * @return True on success, False if extension info cannot be retrieved
   */
  bool GetAddOnInfo(const std::string& name, AddOnInfo& info) override;

  /**
   * @brief Loads and initialises specified extensions
   *
   * @param[in] extensionNames Array of extension names
   * @return vector of initialised extension handles
   */
  std::vector<AddOnLibrary> LoadAddOns(const std::vector<std::string>& addonNames) override;

  /**
   * @copydoc Integration::AddOnManager::LoadAddOn
   */
  AddOnLibrary LoadAddOn(const std::string& addonName, const std::string& libraryName) override;

  /**
   * @brief Returns addon global function pointer
   * @param[in] addonHandle Addon handle
   * @param[in] procName Name of the function to retrieve
   * @return Pointer to the function or null if function doesn't exist
   */
  void* GetGlobalProc(const Dali::AddOnLibrary& addonHandle, const char* procName) override;

  /**
   * @brief Returns addon instance function pointer
   * @param[in] addonHandle Addon handle
   * @param[in] procName Name of the function to retrieve
   * @return Pointer to the function or null if function doesn't exist
   */
  void* GetInstanceProc(const Dali::AddOnLibrary& addonHandle, const char* procName) override;

  /**
   * @brief Lifecycle pause function
   */
  void Pause() override;

  /**
   * @brief Lifecycle resume function
   */
  void Resume() override;

  /**
   * @brief Lifecycle start function
   */
  void Start() override;

  /**
   * @brief Lifecycle stop function
   */
  void Stop() override;

private:
  std::unique_ptr<Internal::AddOnManager> mImpl; /// Implementation of the AddOnManager
};
} // namespace Adaptor
} // namespace Dali

#endif // DALI_ADAPTOR_COMMON_ADDON_MANAGER
