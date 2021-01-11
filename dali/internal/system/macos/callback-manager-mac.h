#pragma once

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

#include <dali/internal/system/common/callback-manager.h>
#include <memory>

namespace Dali::Internal::Adaptor
{

/**
 * @brief Cocoa Implementation of CallbackManager
 */
class CocoaCallbackManager : public CallbackManager
{
public:
  CocoaCallbackManager();

  /**
     * @copydoc CallbackManager::AddIdleCallback()
   */
  bool AddIdleCallback( CallbackBase* callback, bool hasReturnValue ) override;

  /**
   * @caopydoc CallbackManager::RemoveIdleCallback
   */
  void RemoveIdleCallback( CallbackBase* callback ) override;

  /**
   * @copydoc CallbackManager::ProcessIdle
   */
  bool ProcessIdle() override;

  /**
   * @copydoc CallbackManager::ClearIdleCallbacks
   */
  void ClearIdleCallbacks() override;

  /**
   * @copydoc CallbackManager::AddIdleEntererCallback
   */
  bool AddIdleEntererCallback( CallbackBase* callback ) override;

  /**
   * @copydoc CallbackManager::RemoveIdleEntererCallback
   */
  void RemoveIdleEntererCallback( CallbackBase* callback ) override;

  /**
   * @copydoc CallbackManager::Start
   */
  void Start() override;

  /**
   * @copydoc CallbackManager::Stop
   */
  void Stop() override;

  struct Impl;

private:
  std::unique_ptr<Impl> mImpl;
  bool mRunning;
};

}
