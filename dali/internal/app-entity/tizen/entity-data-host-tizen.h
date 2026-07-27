#ifndef DALI_INTERNAL_APP_ENTITY_TIZEN_ENTITY_DATA_HOST_TIZEN_H
#define DALI_INTERNAL_APP_ENTITY_TIZEN_ENTITY_DATA_HOST_TIZEN_H

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
#include <memory>

// INTERNAL INCLUDES
#include <dali/internal/app-entity/common/entity-data-host.h>

#ifdef ENABLE_ENTITY_DATA_TIDL
#include <dali/internal/app-entity/tizen/tidl-stub/view-stub.h>
#endif // ENABLE_ENTITY_DATA_TIDL

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
#ifdef ENABLE_ENTITY_DATA_TIDL
using GeneratedEntityDataStub = ::rpc_port::implview::stub::TizenActionView;
#endif // ENABLE_ENTITY_DATA_TIDL


/**
 * @brief Owns the EntityDataService and the generated TIDL server stub.
 *
 * The TIDL server stub is generated from the Tizen.Action.View
 * action category and stored under the Tizen-only tidl-stub directory.
 */
class EntityDataHostTizen : public EntityDataHost
{
public:
  /**
   * @brief Constructor.
   * @param[in] provider Focus provider forwarded to the EntityDataService.
   */
  explicit EntityDataHostTizen(Dali::Integration::FocusedActorProvider* provider = nullptr);

  /**
   * @brief Destructor.
   */
  ~EntityDataHostTizen() override;

  // Non-copyable.
  EntityDataHostTizen(const EntityDataHostTizen&)            = delete;
  EntityDataHostTizen& operator=(const EntityDataHostTizen&) = delete;

  /**
   * @brief Returns the service that implements the four view actions.
   */
  EntityDataService& GetService() override;

  /**
   * @brief Returns true when a live TIDL server stub is listening.
   *
   * Always false when built without actionc (ENABLE_ENTITY_DATA_TIDL undefined) or
   * when Listen() failed (e.g. rpc-port not registered / insufficient
   * privileges).
   */
  bool IsListening() const override;

private:
  std::unique_ptr<EntityDataService> mService;

#ifdef ENABLE_ENTITY_DATA_TIDL
  /// The actionc-generated TIDL server stub, listening for the lifetime of the host.
  std::unique_ptr<GeneratedEntityDataStub> mStub;
#endif // ENABLE_ENTITY_DATA_TIDL
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_APP_ENTITY_TIZEN_ENTITY_DATA_HOST_TIZEN_H
