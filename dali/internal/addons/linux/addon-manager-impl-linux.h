#ifndef DALI_ADDON_MANAGER_IMPL_LINUX
#define DALI_ADDON_MANAGER_IMPL_LINUX
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

// INTERNAL INCLUDES
#include <dali/internal/addons/common/addon-manager-impl.h>
#include <dali/internal/addons/common/addon-manager.h>

// EXTERNAL INCLUDES
#include <dali/public-api/common/vector-wrapper.h>

#include <memory>
#include <string>

namespace Dali
{
namespace Internal
{
/**
 * Implementation of AddOnManager for Linux based platforms (ie. Tizen, Ubuntu)
 */
class AddOnManagerLinux : public Internal::AddOnManager
{
public:
  /**
   * @copydoc Dali::Internal::AddOnManager()
   * @params[in] dlopenFlags The flags to pass to dlopen when opening an addon/library.
   */
  AddOnManagerLinux(int dlopenFlags);

  /**
   * @copydoc Dali::Internal::~AddOnManager()
   */
  ~AddOnManagerLinux() override;

  /**
   * @copydoc Dali::Internal::AddOnManager::RegisterAddOnDispatchTable()
   */
  void RegisterAddOnDispatchTable(const AddOnDispatchTable* dispatchTable) override;

  /**
   * @copydoc Dali::Internal::AddOnManager::EnumerateAddOns()
   */
  std::vector<std::string> EnumerateAddOns() override;

  /**
   * @copydoc Dali::Internal::AddOnManager::GetAddOnInfo()
   */
  bool GetAddOnInfo(const std::string& name, AddOnInfo& info) override;

  /**
   * @copydoc Dali::Internal::AddOnManager::LoadAddOns()
   */
  std::vector<Dali::AddOnLibrary> LoadAddOns(const std::vector<std::string>& extensionNames) override;

  /**
   * @copydoc Dali::Internal::AddOnManager::LoadAddOn()
   */
  AddOnLibrary LoadAddOn(const std::string& addonName, const std::string& libraryName) override;

  /**
   * @copydoc Dali::Internal::AddOnManager::GetGlobalProc()
   */
  void* GetGlobalProc(const Dali::AddOnLibrary& addonHandle, const char* procName) override;

  /**
   * @copydoc Dali::Internal::AddOnManager::GetInstanceProc()
   */
  void* GetInstanceProc(const Dali::AddOnLibrary& addonHandle, const char* procName) override;

  /**
   * @copydoc Dali::Internal::AddOnManager::Pause()
   */
  void Pause() override;

  /**
   * @copydoc Dali::Internal::AddOnManager::Resume()
   */
  void Resume() override;

  /**
   * @copydoc Dali::Internal::AddOnManager::Start()
   */
  void Start() override;

  /**
   * @copydoc Dali::Internal::AddOnManager::Stop()
   */
  void Stop() override;

private:
  /**
   * @brief Invokes lifecycle event handling function based on incoming event
   * @param[in] lifecycleEvent The lifecycle event
   */
  void InvokeLifecycleFunction(uint32_t lifecycleEvent);

  /**
   * @struct Lifecycle callback structure
   * The instance of the LifecycleCallback handles a single lifecycle
   * event and is bound to an AddOn lifecycle function. The lifecycle
   * function is optional and in case it doesn't exist, the event
   * will be ignored.
   */
  struct LifecycleCallback
  {
    const static uint32_t EVENT_PAUSE  = 0u; ///< pause event
    const static uint32_t EVENT_RESUME = 1u; ///< resume event
    const static uint32_t EVENT_START  = 2u; ///< start event
    const static uint32_t EVENT_STOP   = 3u; ///< stop event

    /**
     * @brief Constructor
     * @param[in] funcName name of the lifecycle function
     */
    explicit LifecycleCallback(const char* funcName)
    {
      functionName = funcName;
    }

    std::string functionName;     ///< Name of lifecycle function
    void (*function)() = nullptr; ///< Lifecycle function pointer
    bool initialized{false};      ///< Flag indicates whether LifecycleCallback is initialized
  };

  /**
   * @struct AddOnCacheEntry
   * @brief Instance of AddOnCacheEntry stores AddOn essential
   * function pointers and library handle.
   */
  struct AddOnCacheEntry
  {
    std::string addOnLib{};
    AddOnInfo   info{};

    // library handle
    void* libHandle{nullptr};

    // main function pointers
    void (*GetAddOnInfo)(AddOnInfo&)      = nullptr; ///< Returns AddOnInfo structure
    void* (*GetInstanceProc)(const char*) = nullptr; ///< Returns pointer of instance function (member funtion)
    void* (*GetGlobalProc)(const char*)   = nullptr; ///< Returns pointer of global function (non-member function)

    // lifecycle functions
    std::vector<LifecycleCallback> lifecycleCallbacks =
      {
        LifecycleCallback{"OnPause"},
        LifecycleCallback{"OnResume"},
        LifecycleCallback{"OnStart"},
        LifecycleCallback{"OnStop"},
      };
    bool opened{false};
  };

  std::vector<AddOnCacheEntry> mAddOnCache;
  std::vector<std::string>     mAddOnNames;
  int                          mDlopenFlags{0};
};

} // namespace Internal
} // namespace Dali
#endif //DALI_CMAKE_EXTENSION_MANAGER_IMPL_UBUNTU
