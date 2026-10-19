#ifndef DALI_INTERNAL_INPUT_WINDOWS_BRIDGE_IME_BRIDGE_HOST_REGISTRY_H
#define DALI_INTERNAL_INPUT_WINDOWS_BRIDGE_IME_BRIDGE_HOST_REGISTRY_H

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
#include <functional>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/ime-bridge.h>

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{
/**
 * @brief Tracks the live ImeBridgeHost instances in this process.
 *
 * Adaptor-internal, deliberately not installed. A bridge may register after
 * input method contexts already exist, so RegisterImeBridge() needs to reach
 * the contexts that are already alive. The registry is built with the Windows
 * bridge backend and directly into the Ubuntu UTC executable.
 *
 * Main/event thread only.
 */
namespace ImeBridgeHostRegistry
{
/**
 * @brief Adds a host to the registry.
 * @param[in] host The host to add
 */
void Add(Dali::Integration::ImeBridgeHost* host);

/**
 * @brief Removes a host from the registry.
 * @param[in] host The host to remove
 */
void Remove(Dali::Integration::ImeBridgeHost* host);

/**
 * @brief Invokes @a callback for every registered host, in registration order.
 * @param[in] callback The callback to invoke
 */
void ForEach(const std::function<void(Dali::Integration::ImeBridgeHost*)>& callback);

} // namespace ImeBridgeHostRegistry

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_INPUT_WINDOWS_BRIDGE_IME_BRIDGE_HOST_REGISTRY_H
