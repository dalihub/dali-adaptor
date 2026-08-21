#ifndef DALI_INTEGRATION_API_ADAPTOR_FRAMEWORK_FOCUSED_ACTOR_PROVIDER_H
#define DALI_INTEGRATION_API_ADAPTOR_FRAMEWORK_FOCUSED_ACTOR_PROVIDER_H

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
#include <dali/public-api/actors/actor.h>
#include <dali/public-api/dali-adaptor-common.h>

namespace Dali
{
namespace Integration
{

/**
 * @brief Interface for querying focused Actor information.
 *
 * Implemented by the layer that owns focus management and registered with the
 * Adaptor so that internal services can obtain the exact focused Actor without
 * a reverse dependency on that layer.
 */
class DALI_ADAPTOR_API FocusedActorProvider
{
public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~FocusedActorProvider() = default;

  /**
   * @brief Returns the currently focused Actor, or an empty handle.
   * @return The focused Actor, or an empty Actor handle when none is focused.
   */
  virtual Actor GetFocusedActor() = 0;
};

/**
 * @brief Registers the process-wide focused Actor provider.
 *
 * Called on the main/event thread by the layer that owns focus management.
 * The caller retains ownership; the pointer must outlive its registration.
 *
 * @param[in] provider The provider implementation to register
 */
DALI_ADAPTOR_API void RegisterFocusedActorProvider(FocusedActorProvider* provider);

/**
 * @brief Unregisters the focused Actor provider if it is currently registered.
 *
 * @param[in] provider The provider implementation to unregister
 */
DALI_ADAPTOR_API void UnregisterFocusedActorProvider(FocusedActorProvider* provider);

/**
 * @brief Returns the process-wide focused Actor provider.
 *
 * @return The registered provider, or nullptr when none is registered
 */
DALI_ADAPTOR_API FocusedActorProvider* GetFocusedActorProvider();

} // namespace Integration
} // namespace Dali

#endif // DALI_INTEGRATION_API_ADAPTOR_FRAMEWORK_FOCUSED_ACTOR_PROVIDER_H
