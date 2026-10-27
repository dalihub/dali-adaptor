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

// CLASS HEADER
#include <dali/integration-api/adaptor-framework/ime-bridge.h>

// INTERNAL INCLUDES
#include <dali/internal/input/windows/bridge/ime-bridge-host-registry.h>

// EXTERNAL INCLUDES
#include <algorithm>
#include <vector>

namespace DALI_NAMESPACE
{
namespace Integration
{
namespace
{
/// Process-wide bridge, populated once by the layer that owns the IME.
/// Accessed on the main/event thread only, matching the
/// focused-actor-provider convention.
ImeBridge* gImeBridge = nullptr;

/// Detaches every live host from @a bridge and notifies each one.
/// gImeBridge must already have stopped pointing at @a bridge, so that
/// anything a host does while being notified cannot reach a bridge that has
/// just let go of it.
void DetachAllHosts(ImeBridge* bridge)
{
  if(!bridge)
  {
    return;
  }

  Dali::Internal::Adaptor::ImeBridgeHostRegistry::ForEach(
    [bridge](ImeBridgeHost* host) {
      bridge->DetachHost(host);
      host->OnBridgeDetached();
    });
}

} // unnamed namespace

void RegisterImeBridge(ImeBridge* bridge)
{
  ImeBridge* const previous = gImeBridge;
  if(previous == bridge)
  {
    // Already registered: attaching the same hosts twice would leave the
    // bridge holding each of them more than once.
    return;
  }

  // Retire the outgoing bridge before the incoming one becomes reachable.
  // Without this it is never told about the hosts it attached: a host is
  // detached by ~InputMethodContextBridge, which detaches from whatever
  // GetImeBridge() returns *then* -- the incoming bridge -- so the outgoing
  // one would keep dangling host pointers for ever and the incoming one would
  // receive a detach for a host it never attached.
  gImeBridge = nullptr;
  DetachAllHosts(previous);

  gImeBridge = bridge;

  // Contexts may already exist; attach them so a late registration still
  // reaches them.
  if(bridge)
  {
    Dali::Internal::Adaptor::ImeBridgeHostRegistry::ForEach(
      [bridge](ImeBridgeHost* host) {
        bridge->AttachHost(host);
        host->OnBridgeAttached();
      });
  }
}

void UnregisterImeBridge(ImeBridge* bridge)
{
  if(gImeBridge != bridge)
  {
    return;
  }

  gImeBridge = nullptr;
  DetachAllHosts(bridge);
}

ImeBridge* GetImeBridge()
{
  return gImeBridge;
}

} // namespace Integration

namespace Internal
{
namespace Adaptor
{
namespace ImeBridgeHostRegistry
{
namespace
{
/// Live hosts, in registration order. Main/event thread only.
std::vector<Dali::Integration::ImeBridgeHost*>& GetHosts()
{
  static std::vector<Dali::Integration::ImeBridgeHost*> hosts;
  return hosts;
}

} // unnamed namespace

void Add(Dali::Integration::ImeBridgeHost* host)
{
  if(!host)
  {
    return;
  }

  auto& hosts = GetHosts();
  if(std::find(hosts.begin(), hosts.end(), host) == hosts.end())
  {
    hosts.push_back(host);
  }
}

void Remove(Dali::Integration::ImeBridgeHost* host)
{
  auto& hosts = GetHosts();
  hosts.erase(std::remove(hosts.begin(), hosts.end(), host), hosts.end());
}

void ForEach(const std::function<void(Dali::Integration::ImeBridgeHost*)>& callback)
{
  // Copy first: a callback may attach or detach hosts.
  const std::vector<Dali::Integration::ImeBridgeHost*> snapshot = GetHosts();

  for(auto* host : snapshot)
  {
    // Hardening (task-3 final review): the snapshot stops the iteration from
    // being invalidated, but on its own it does not stop a use-after-free.
    // OnBridgeDetached() emits application-facing signals from inside this
    // loop (input-method-context-impl-bridge.cpp:817-824), and an application
    // handler may destroy a *different* text control -- whose input method
    // context removes itself from this registry in ~InputMethodContextBridge. A
    // later iteration would then dereference a freed ImeBridgeHost*.
    //
    // Re-check the live list before every callback and skip anything that has
    // gone. Linear per element, but the list holds one entry per live input
    // method context, so it stays tiny. The guard covers both the attach and
    // the detach loop in RegisterImeBridge()/UnregisterImeBridge(), since both
    // go through here.
    const auto& hosts = GetHosts();
    if(std::find(hosts.begin(), hosts.end(), host) == hosts.end())
    {
      continue;
    }

    callback(host);
  }
}

} // namespace ImeBridgeHostRegistry

} // namespace Adaptor
} // namespace Internal
} // namespace Dali
