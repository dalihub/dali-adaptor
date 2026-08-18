#ifndef DALI_INTERNAL_APP_ENTITY_COMMON_ENTITY_DATA_BUILDER_H
#define DALI_INTERNAL_APP_ENTITY_COMMON_ENTITY_DATA_BUILDER_H

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

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/focused-actor-provider.h>
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
 * @brief Creates EntityData from an Actor.
 *
 * Uses the Accessibility Bridge (Accessible/Component) when available,
 * falling back to Actor properties otherwise. Annotation is always read
 * from the Actor annotation API. Focus is queried via the
 * FocusedActorProvider.
 *
 * @param[in] actor    The Actor to convert
 * @param[in] provider The focus provider (may be null)
 * @return EntityData with all fields populated
 */
DALI_ADAPTOR_API EntityData MakeEntityData(Dali::Actor actor, FocusedActorProvider* provider);

/**
 * @brief Creates EntityData from an Actor using an annotation already read
 * from that Actor.
 *
 * This overload avoids reading the same annotation again when the caller has
 * already used it to select the Actor.
 *
 * @param[in] actor      The Actor to convert
 * @param[in] provider   The focus provider (may be null)
 * @param[in] annotation Annotation previously read from the Actor
 * @return EntityData with all fields populated
 */
DALI_ADAPTOR_API EntityData MakeEntityData(Dali::Actor actor, FocusedActorProvider* provider, EntityData::Annotation annotation);

/**
 * @brief Reads the annotation from an Actor's custom property.
 *
 * @param[in]  actor      The Actor to check
 * @param[out] annotation Filled if an annotation exists
 * @return true if an annotation exists and entityId is non-empty
 */
DALI_ADAPTOR_API bool ReadActorAnnotation(Dali::Actor actor, EntityData::Annotation& annotation);

/**
 * @brief Returns whether an Actor is effectively visible.
 *
 * Uses the Accessibility Bridge states when available, falling back to the
 * Actor visibility property otherwise. Computed on demand rather than stored,
 * because visibility is not part of the Tizen.Entity.View contract.
 *
 * @param[in] actor The Actor to test
 * @return true if the Actor is effectively visible
 */
DALI_ADAPTOR_API bool IsActorVisible(Dali::Actor actor);

/**
 * @brief Returns whether an Actor and all of its ancestors are visible.
 *
 * Parent visibility affects a child regardless of whether the parent clips its
 * children, so this check is independent of CLIPPING_MODE.
 *
 * @param[in] actor The Actor to test
 * @return true if the Actor and every ancestor are visible
 */
DALI_ADAPTOR_API bool IsActorHierarchyVisible(Dali::Actor actor);

/**
 * @brief Returns an Actor's bounds in the window coordinate system.
 *
 * Cheaper than MakeEntityData() when only the clip rectangle is
 * needed, as it skips the id, type, description, annotation and focus lookups.
 *
 * @param[in] actor The Actor to measure
 * @return The Actor bounds relative to its window
 */
DALI_ADAPTOR_API Dali::Bounds GetActorWindowBounds(Dali::Actor actor);

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_APP_ENTITY_COMMON_ENTITY_DATA_BUILDER_H
