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

#include <dali-test-suite-utils.h>
#include <dali/integration-api/adaptor-framework/ime-bridge.h>
#include <dali/internal/input/windows/bridge/ime-bridge-host-registry.h>

#include <algorithm>
#include <vector>

using namespace Dali;

namespace
{
/// Minimal bridge that only records attach/detach; all other methods are no-ops.
class TestBridge : public Dali::Integration::ImeBridge
{
public:
  std::vector<Dali::Integration::ImeBridgeHost*> attached;

  void AttachHost(Dali::Integration::ImeBridgeHost* host) override { attached.push_back(host); }
  void DetachHost(Dali::Integration::ImeBridgeHost* host) override
  {
    attached.erase(std::remove(attached.begin(), attached.end(), host), attached.end());
  }
  void Activate(Dali::Integration::ImeBridgeHost*) override {}
  void Deactivate(Dali::Integration::ImeBridgeHost*) override {}
  void Reset(Dali::Integration::ImeBridgeHost*) override {}
  bool ShowInputPanel(Dali::Integration::ImeBridgeHost*) override { return true; }
  bool HideInputPanel(Dali::Integration::ImeBridgeHost*) override { return true; }
  bool FilterEventKey(Dali::Integration::ImeBridgeHost*, const Dali::KeyEvent&) override { return false; }
  void SetCursorPosition(Dali::Integration::ImeBridgeHost*, uint32_t) override {}
  void SetSurroundingText(Dali::Integration::ImeBridgeHost*, const Dali::String&) override {}
  void SetPanelLayout(Dali::Integration::ImeBridgeHost*, Dali::InputMethod::PanelLayout, Dali::InputMethod::PanelLayoutVariation) override {}
  void SetReturnKey(Dali::Integration::ImeBridgeHost*, Dali::InputMethod::ReturnKeyType, bool) override {}
  void SetAutoCapital(Dali::Integration::ImeBridgeHost*, Dali::InputMethod::AutoCapitalType) override {}
  void SetTextPrediction(Dali::Integration::ImeBridgeHost*, bool) override {}
  void SetUserData(Dali::Integration::ImeBridgeHost*, const Dali::String&) override {}
  Dali::String GetUserData(Dali::Integration::ImeBridgeHost*) override { return Dali::String(); }
  void SetLanguage(Dali::Integration::ImeBridgeHost*, Dali::Integration::InputMethodContext::InputPanelLanguage) override {}
  void SetContentMimeTypes(Dali::Integration::ImeBridgeHost*, const Dali::String&) override {}
  void SetPanelPosition(Dali::Integration::ImeBridgeHost*, int32_t, int32_t) override {}
  void SetPanelPositionAlign(Dali::Integration::ImeBridgeHost*, int32_t, int32_t, Dali::InputMethodContext::InputPanelAlign) override {}
  void SetMultiLine(Dali::Integration::ImeBridgeHost*, bool) override {}
};

/// Minimal host whose methods are all no-ops.
class TestHost : public Dali::Integration::ImeBridgeHost
{
public:
  void Commit(const Dali::String&) override {}
  void PreEdit(const Dali::String&, int32_t, const Dali::Integration::InputMethodContext::PreEditAttributeDataContainer&) override {}
  void PreEditStart() override {}
  void PreEditEnd() override {}
  void DeleteSurrounding(int32_t, int32_t) override {}
  void SetSelection(int32_t, int32_t) override {}
  void SendPrivateCommand(const Dali::String&) override {}
  void CommitContent(const Dali::String&, const Dali::String&, const Dali::String&) override {}
  void RecaptureString(int32_t, int32_t, const Dali::String&, const Dali::String&, const Dali::Integration::InputMethodContext::PreEditAttributeDataContainer&) override {}
  bool RetrieveSurrounding(Dali::String&, int32_t&) override { return false; }
  void FeedKeyEvent(const Dali::KeyEvent&) override {}
  void InputPanelGeometryChanged(const Dali::BoundsInteger&) override {}
  void InputPanelStateChanged(Dali::InputMethodContext::State) override {}
  void InputPanelLanguageChanged(Dali::Integration::InputMethodContext::InputPanelLanguage) override {}
  void KeyboardTypeChanged(Dali::InputMethodContext::KeyboardType) override {}
  void OnBridgeAttached() override {}
  void OnBridgeDetached() override { ++detachedCount; }

  int detachedCount{0};
};

} // unnamed namespace

int UtcDaliImeBridgeRegisterAndGet(void)
{
  DALI_TEST_CHECK(Dali::Integration::GetImeBridge() == nullptr);

  TestBridge bridge;
  Dali::Integration::RegisterImeBridge(&bridge);
  DALI_TEST_CHECK(Dali::Integration::GetImeBridge() == &bridge);

  Dali::Integration::UnregisterImeBridge(&bridge);
  DALI_TEST_CHECK(Dali::Integration::GetImeBridge() == nullptr);

  END_TEST;
}

int UtcDaliImeBridgeUnregisterOnlyClearsMatchingBridge(void)
{
  TestBridge first;
  TestBridge second;

  Dali::Integration::RegisterImeBridge(&first);

  // Trying to unregister an unregistered bridge must not clear the current one.
  Dali::Integration::UnregisterImeBridge(&second);
  DALI_TEST_CHECK(Dali::Integration::GetImeBridge() == &first);

  Dali::Integration::UnregisterImeBridge(&first);
  DALI_TEST_CHECK(Dali::Integration::GetImeBridge() == nullptr);

  END_TEST;
}

int UtcDaliImeBridgeHostRegistryForEach(void)
{
  using namespace Dali::Internal::Adaptor;

  TestHost hostA;
  TestHost hostB;

  ImeBridgeHostRegistry::Add(&hostA);
  ImeBridgeHostRegistry::Add(&hostB);

  std::vector<Dali::Integration::ImeBridgeHost*> visited;
  ImeBridgeHostRegistry::ForEach([&visited](Dali::Integration::ImeBridgeHost* host) { visited.push_back(host); });

  DALI_TEST_EQUALS(visited.size(), 2u, TEST_LOCATION);

  ImeBridgeHostRegistry::Remove(&hostA);
  visited.clear();
  ImeBridgeHostRegistry::ForEach([&visited](Dali::Integration::ImeBridgeHost* host) { visited.push_back(host); });

  DALI_TEST_EQUALS(visited.size(), 1u, TEST_LOCATION);
  DALI_TEST_CHECK(visited[0] == &hostB);

  ImeBridgeHostRegistry::Remove(&hostB);
  END_TEST;
}

int UtcDaliImeBridgeUnregisterNotifiesHosts(void)
{
  // Regression test for review finding I6: UnregisterImeBridge() used to call
  // DetachHost() on each host and tell the hosts nothing, so a context kept
  // reporting a vanished input panel as visible.
  using namespace Dali::Internal::Adaptor;

  TestBridge bridge;
  TestHost   host;

  ImeBridgeHostRegistry::Add(&host);
  Dali::Integration::RegisterImeBridge(&bridge);

  DALI_TEST_EQUALS(bridge.attached.size(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(host.detachedCount, 0, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);

  DALI_TEST_EQUALS(bridge.attached.size(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(host.detachedCount, 1, TEST_LOCATION);

  ImeBridgeHostRegistry::Remove(&host);
  END_TEST;
}

int UtcDaliImeBridgeRegisteringSecondBridgeRetiresTheFirst(void)
{
  // Regression test for review finding I5: RegisterImeBridge() used to
  // overwrite the process-wide bridge without detaching the outgoing one's
  // hosts, so the first bridge kept a host pointer for ever while the second
  // would later receive a detach for a host it never attached.
  using namespace Dali::Internal::Adaptor;

  TestBridge first;
  TestBridge second;
  TestHost   host;

  ImeBridgeHostRegistry::Add(&host);

  Dali::Integration::RegisterImeBridge(&first);
  DALI_TEST_EQUALS(first.attached.size(), 1u, TEST_LOCATION);

  Dali::Integration::RegisterImeBridge(&second);

  DALI_TEST_CHECK(Dali::Integration::GetImeBridge() == &second);
  DALI_TEST_EQUALS(first.attached.size(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(second.attached.size(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(host.detachedCount, 1, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&second);
  DALI_TEST_EQUALS(second.attached.size(), 0u, TEST_LOCATION);
  DALI_TEST_EQUALS(host.detachedCount, 2, TEST_LOCATION);

  ImeBridgeHostRegistry::Remove(&host);
  END_TEST;
}

int UtcDaliImeBridgeRegisteringTheSameBridgeTwiceIsANoOp(void)
{
  // Attaching the same host twice would leave the bridge holding it more than
  // once, and a single DetachHost() would then not be enough to let it go.
  using namespace Dali::Internal::Adaptor;

  TestBridge bridge;
  TestHost   host;

  ImeBridgeHostRegistry::Add(&host);

  Dali::Integration::RegisterImeBridge(&bridge);
  Dali::Integration::RegisterImeBridge(&bridge);

  DALI_TEST_EQUALS(bridge.attached.size(), 1u, TEST_LOCATION);
  DALI_TEST_EQUALS(host.detachedCount, 0, TEST_LOCATION);

  Dali::Integration::UnregisterImeBridge(&bridge);
  ImeBridgeHostRegistry::Remove(&host);
  END_TEST;
}
