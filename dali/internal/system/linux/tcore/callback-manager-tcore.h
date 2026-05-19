#ifndef DALI_INTERNAL_ADAPTOR_SYSTEM_LINUX_TCORE_CALLBACK_MANAGER_H
#define DALI_INTERNAL_ADAPTOR_SYSTEM_LINUX_TCORE_CALLBACK_MANAGER_H

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
#include <list>
#include <tizen_core.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/callback-manager.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
struct TcoreCallbackData;

/**
 * Tcore interface to install callbacks in the application's main loop.
 */
class TcoreCallbackManager : public CallbackManager
{
public:
  /**
   * @brief constructor
   */
  TcoreCallbackManager();

  /**
   * @brief destructor
   */
  ~TcoreCallbackManager()
  {
  }

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
   * @copydoc CallbackManager::ProcessIdle()
   */
  void ClearIdleCallbacks() override;

  /**
   * @copydoc CallbackManager::AddIdleEntererCallback()
   */
  bool AddIdleEntererCallback(CallbackBase* callback) override;

  /**
   * @copydoc CallbackManager::RemoveIdleEntererCallback()
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
  /**
   * @brief Remove all idle call backs that are pending
   * Called by Stop()
   * Always called from the main thread
   */
  void RemoveAllCallbacks();

  /**
   * @brief Removes a single call back from the container
   * Always called from main thread
   * @param callbackData callback data
   */
  void RemoveCallbackFromContainer(TcoreCallbackData* callbackData);

  /**
   * @brief Remove a standard call back from ecore
   * Always called from main thread
   * @param callbackData callback data
   */
  void RemoveStandardCallback(TcoreCallbackData* callbackData);

  // Undefined
  TcoreCallbackManager(const TcoreCallbackManager&)      = delete;
  TcoreCallbackManager& operator=(const TcoreCallbackManager&) = delete;

private:
  typedef std::list<TcoreCallbackData*> CallbackList;

  bool         mRunning;           ///< flag is set to true if when running
  tizen_core_h mCore;              ///< cached tizen core handle
  CallbackList mCallbackContainer; ///< container of live idle callbacks
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_SYSTEM_LINUX_CALLBACK_MANAGER_H
