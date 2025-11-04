#ifndef DALI_INTERNAL_UNIFIED_TRIGGER_EVENT_MANAGER_H
#define DALI_INTERNAL_UNIFIED_TRIGGER_EVENT_MANAGER_H

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
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/dali-adaptor-common.h>
#include <dali/public-api/object/base-handle.h>
#include <dali/public-api/signals/callback.h>
#include <memory>
#include <unordered_set>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/trigger-event-interface.h>
#include <dali/internal/system/common/file-descriptor-monitor.h>

namespace Dali
{
namespace Internal::Adaptor
{
class UnifiedTriggerEventManager;
} // namespace Internal::Adaptor

/**
 * @brief Handle of UnifiedTriggerEventManager.
 */
class UnifiedTriggerEventManager : public BaseHandle
{
public:
  /**
   * @brief Constructor
   */
  UnifiedTriggerEventManager() = default;

  /**
   * @brief Copy constructor.
   *
   * @param[in] copy The UnifiedTriggerEventManager to copy
   */
  UnifiedTriggerEventManager(const UnifiedTriggerEventManager& copy) = default;

  /**
   * @brief Assignment operator
   *
   * @param[in] rhs The UnifiedTriggerEventManager to copy
   * @return A reference to this
   */
  UnifiedTriggerEventManager& operator=(const UnifiedTriggerEventManager& rhs) = default;

  /**
   * @brief Move constructor.
   *
   * @param[in] move The UnifiedTriggerEventManager to move
   */
  UnifiedTriggerEventManager(UnifiedTriggerEventManager&& move) noexcept = default;

  /**
   * @brief Move assignment operator
   *
   * @param[in] rhs The UnifiedTriggerEventManager to move
   * @return A reference to this
   */
  UnifiedTriggerEventManager& operator=(UnifiedTriggerEventManager&& rhs) noexcept = default;

  /**
   * @brief Destructor
   */
  ~UnifiedTriggerEventManager() = default;

public: // Not intended for application developers
  /**
   * @brief This constructor is used internally to create a handle from an object pointer.
   * @param [in] lifecycleController A pointer to the internal UnifiedTriggerEventManager.
   */
  explicit UnifiedTriggerEventManager(Internal::Adaptor::UnifiedTriggerEventManager* unifiedTriggerEventManager);
};
} // namespace Dali

#endif // DALI_INTERNAL_UNIFIED_TRIGGER_EVENT_MANAGER_H
