#ifndef DALI_INTERNAL_ADAPTOR_SYSTEM_GLIB_CALLBACK_MANAGER_H
#define DALI_INTERNAL_ADAPTOR_SYSTEM_GLIB_CALLBACK_MANAGER_H

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

// INTERNAL INCLUDES
#include <dali/internal/system/common/callback-manager.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
struct GlibCallbackData;

/**
 * @brief Glib callback manager used to install call backs in the applications main loop.
 * The manager keeps track of all callbacks, so that if Stop() is called it can remove them.
 */
class GlibCallbackManager : public CallbackManager
{
public:
  /**
   * @brief constructor
   */
  GlibCallbackManager();

  /**
   * @brief destructor
   */
  virtual ~GlibCallbackManager() = default;

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
   * @brief Removes a single call back from the container
   * Always called from main thread
   * @param callbackData callback data
   */
  void RemoveCallbackFromContainer(GlibCallbackData* callbackData);

  // Undefined
  GlibCallbackManager(const GlibCallbackManager&)      = delete;
  GlibCallbackManager& operator=(GlibCallbackManager&) = delete;

private:
  typedef std::list<GlibCallbackData*> CallbackList; ///< list of callbacks installed

  bool         mRunning;           ///< flag is set to true if when running
  CallbackList mCallbackContainer; ///< container of live callbacks
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_ADAPTOR_SYSTEM_GLIB_CALLBACK_MANAGER_H
