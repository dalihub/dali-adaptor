#ifndef __DALI_ANDROID_CALLBACK_MANAGER_H__
#define __DALI_ANDROID_CALLBACK_MANAGER_H__

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/internal/system/common/callback-manager.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
struct CallbackData;

/**
 * Ecore interface to install call backs in the applications main loop.
 */
class AndroidCallbackManager : public CallbackManager
{
public:
  /**
     * @brief constructor
     */
  AndroidCallbackManager();

  /**
     * @brief destructor
     */
  ~AndroidCallbackManager() = default;

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
  void RemoveCallbackFromContainer(CallbackData* callbackData);

  /**
     * @brief Remove a standard call back from ecore
     * Always called from main thread
     * @param callbackData callback data
     */
  void RemoveStandardCallback(CallbackData* callbackData);

  typedef std::list<CallbackData*> CallbackList;

  bool         mRunning;           ///< flag is set to true if when running
  CallbackList mCallbackContainer; ///< container of live idle callbacks
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // __DALI_ANDROID_CALLBACK_MANAGER_H__
