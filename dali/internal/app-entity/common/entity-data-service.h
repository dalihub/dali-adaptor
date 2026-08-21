#ifndef DALI_INTERNAL_APP_ENTITY_COMMON_ENTITY_DATA_SERVICE_H
#define DALI_INTERNAL_APP_ENTITY_COMMON_ENTITY_DATA_SERVICE_H

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
#include <string>
#include <vector>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/focused-actor-provider.h>
#include <dali/internal/app-entity/common/entity-data-builder.h>
#include <dali/internal/app-entity/common/entity-data.h>
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
using Dali::Integration::FocusedActorProvider;

/**
 * @brief Implements platform-neutral business logic for the entity-data action
 * contract.
 *
 * The four methods provide the shared entity-data operations:
 * FindByActorId, GetFocusedEntityData, GetAnnotatedEntities and ToPresentation.
 * Each returns a boolean success flag; a platform backend maps this to its
 * transport-specific response type.
 */
class DALI_ADAPTOR_API EntityDataService
{
public:
  /// Minimum visible-area ratio for an entity to be included in GetAnnotatedEntities.
  static constexpr float VISIBILITY_THRESHOLD = 0.70f;

  /**
   * @brief Constructor.
   * @param[in] provider Focus provider (may be null; can be set later).
   */
  explicit EntityDataService(FocusedActorProvider* provider = nullptr);

  /**
   * @brief Sets the focus information provider.
   * @param[in] provider Focus provider (may be null).
   */
  void SetFocusedActorProvider(FocusedActorProvider* provider);

  /**
   * @brief Finds EntityData by its Actor ID string across all windows.
   * @param[in]  actorId Actor ID encoded as a string
   * @param[out] outEntityData EntityData when found
   * @return true when the Actor is alive and converted to EntityData
   */
  bool FindByActorId(const std::string& actorId, EntityData& outEntityData);

  /**
   * @brief Finds EntityData by its Actor ID string below a supplied root.
   *
   * This platform-neutral overload is also useful when a caller already owns
   * the scene root and does not need Adaptor window discovery.
   *
   * @param[in] actorId Actor ID encoded as a strict unsigned decimal string
   * @param[in] root Root Actor to search
   * @param[out] outEntityData EntityData when found
   * @return true when the Actor is found and converted to EntityData
   */
  bool FindByActorId(const std::string& actorId, Dali::Actor root, EntityData& outEntityData);

  /**
   * @brief Returns the exact focused EntityData.
   * @param[out] outEntityData Focused EntityData
   * @return true when a focused entity exists
   */
  bool GetFocusedEntityData(EntityData& outEntityData);

  /**
   * @brief Enumerates annotated entities that are at least 70% visible.
   * @param[out] outEntityDataList Sorted list of qualifying EntityData (may be empty)
   * @return true (success even when empty)
   */
  bool GetAnnotatedEntities(std::vector<EntityData>& outEntityDataList);

  /**
   * @brief Enumerates annotated entities below a supplied root.
   *
   * @param[in] root Root Actor to enumerate
   * @param[in] clipBounds Visible bounds in the root's window coordinates
   * @param[out] outEntityDataList Sorted list of qualifying EntityData
   * @return true (success even when the root is empty)
   */
  bool GetAnnotatedEntities(Dali::Actor root, const Dali::Rect<float>& clipBounds, std::vector<EntityData>& outEntityDataList);

  /**
   * @brief Converts EntityData into a presentation representation.
   * @param[in]  entityData      The EntityData to convert
   * @param[out] outPresentation Presentation payload
   * @return true on success
   */
  bool ToPresentation(const EntityData& entityData, std::string& outPresentation);

private:
  /**
   * @brief Returns the provider to use: the injected one if set, otherwise the
   * process-wide provider registered via RegisterFocusedActorProvider().
   */
  FocusedActorProvider* GetEffectiveProvider() const;

  FocusedActorProvider* mProvider;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_APP_ENTITY_COMMON_ENTITY_DATA_SERVICE_H
