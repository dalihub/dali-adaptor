#ifndef DALI_INTERNAL_ADAPTOR_SYSTEM_WINDOWS_CALLBACK_MANAGER_H
#define DALI_INTERNAL_ADAPTOR_SYSTEM_WINDOWS_CALLBACK_MANAGER_H

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
#include <dali/public-api/common/list-wrapper.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/callback-manager.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
struct WindowsCallbackData;

/**
 * @brief Windows callback manager used to install call backs in the applications main loop.
 * The manager keeps track of all callbacks, so that if Stop() is called it can remove them.
 */
class WinCallbackManager : public CallbackManager
{
public:
  /**
   * @brief constructor
   */
  WinCallbackManager();

  /**
   * @brief destructor
   */
  ~WinCallbackManager();

  /**
   * @copydoc CallbackManager::AddIdleCallback()
   */
  bool AddIdleCallback(CallbackBase* callback, bool hasReturnValue) override;

  /**
   * @copydoc CallbackManager::RemoveIdleCallback()
   */
  void RemoveIdleCallback(CallbackBase* callback) override;

  /**
   * @copydoc CallbackManager::ProcessIdle()
   */
  bool ProcessIdle() override;

  /**
   * @copydoc CallbackManager::ClearIdleCallbacks()
   */
  void ClearIdleCallbacks() override;

  /**
   * @brief Adds a @p callback to be run when entering an idle state.
   * @note Must be called from the main thread only.
   *
   * A callback of the following type should be used:
   * @code
   *   bool MyFunction();
   * @endcode
   * This callback will be called repeatedly as long as it returns true. A return of 0 deletes this callback.
   *
   * @param[in] callback custom callback function.
   *
   * @return true on success
   */
  bool AddIdleEntererCallback(CallbackBase* callback) override;

  /**
   * @brief Removes a previously added the idle enterer callback.
   * @note Must be called from main thread only.
   *
   * Does nothing if the @p callback doesn't exist.
   *
   * @param[in] callback The callback to be removed.
   */
  void RemoveIdleEntererCallback(CallbackBase* callback) override;

  /**
   * @copydoc CallbackManager::Start()
   */
  void Start() override;

  /**
   * @copydoc CallbackManager::Stop()
   */
  void Stop() override;

private:
  // Undefined
  WinCallbackManager(const WinCallbackManager&)      = delete;
  WinCallbackManager& operator=(WinCallbackManager&) = delete;

  /**
   * @brief Callback function comes from framework.
   * It will be self callback.
   */
  void ProcessIdleFromFramework();

private:
  CallbackBase* mSelfCallback{nullptr};
  bool          mSelfCallbackRegistered{false}; ///< flag is set to true if we send processIdle callback register.

  typedef std::list<WindowsCallbackData*> CallbackList;

  CallbackList mCallbackContainer;
  bool         mRunning; ///< flag is set to true if when running
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_SYSTEM_WINDOWS_CALLBACK_MANAGER_H
