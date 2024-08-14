/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <adaptor-environment-variable.h>
#include <dali-test-suite-utils.h>

#include <dali/dali.h>
#include <dali/devel-api/common/addon-binder.h>
#include <dali/integration-api/addon-manager.h>
#include <dali/internal/addons/common/addon-manager-factory.h>

std::unique_ptr<Dali::Integration::AddOnManager> CreateAddOnManager()
{
  // Set env variables
  Dali::EnvironmentVariable::SetTestEnvironmentVariable("DALI_ADDONS_PATH", ADDON_LIBS_PATH);
  Dali::EnvironmentVariable::SetTestEnvironmentVariable("DALI_ADDONS_LIBS", "libSampleAddOn.so");

  return std::unique_ptr<Dali::Integration::AddOnManager>(Dali::Internal::AddOnManagerFactory::CreateAddOnManager());
}

struct TestAddOn : public Dali::AddOn::AddOnBinder
{
  TestAddOn()
  : AddOnBinder("SampleAddOn", 0u)
  {
  }

  ADDON_BIND_FUNCTION(GetLifecycleStatus, bool());
};

int UtcDaliTestAddOnInterface(void)
{
  TestApplication application;
  // Create AddOnManager using internal factory
  auto addOnManager = CreateAddOnManager();

  TestAddOn addon;

  DALI_TEST_EQUALS(addon.IsValid(), true, TEST_LOCATION);

  const auto& info = addon.GetAddOnInfo();

  // Test returned addon version and type
  DALI_TEST_EQUALS(info.version, Dali::DALI_ADDON_VERSION(1, 0, 0), TEST_LOCATION);
  DALI_TEST_EQUALS(info.type, Dali::AddOnType::GENERIC, TEST_LOCATION);

  // Test lifecycle
  addOnManager->Pause();

  auto result1 = addon.GetLifecycleStatus();
  DALI_TEST_EQUALS(result1, true, TEST_LOCATION);

  addOnManager->Resume();
  auto result2 = addon.GetLifecycleStatus();
  DALI_TEST_EQUALS(result2, false, TEST_LOCATION);

  END_TEST;
}

int UtcDaliTestAddOnManager(void)
{
  TestApplication application;

  // Create AddOnManager using internal factory
  auto addOnManagerUPTR = CreateAddOnManager();

  // Get addon-manager
  auto* addonManager = Dali::Integration::AddOnManager::Get();

  bool result = addonManager != nullptr;

  DALI_TEST_EQUALS(result, true, TEST_LOCATION);

  auto availableAddons = addonManager->EnumerateAddOns();

  // must be 1 addon available
  DALI_TEST_EQUALS(availableAddons.size(), 1u, TEST_LOCATION);

  Dali::AddOnInfo info{};
  addonManager->GetAddOnInfo(availableAddons[0], info);

  // Test returned addon version and type
  DALI_TEST_EQUALS(info.version, Dali::DALI_ADDON_VERSION(1, 0, 0), TEST_LOCATION);
  DALI_TEST_EQUALS(info.type, Dali::AddOnType::GENERIC, TEST_LOCATION);

  // Get addon handle
  auto testAddon = addonManager->GetAddOn(availableAddons[0]);
  result         = testAddon != 0;
  DALI_TEST_EQUALS(result, true, TEST_LOCATION);

  // Get addon global function
  auto createInstance = addonManager->GetGlobalProc<void*()>(testAddon, "CreateInstance");
  result              = createInstance != nullptr;
  DALI_TEST_EQUALS(result, true, TEST_LOCATION);

  // Test for false positive (queried function must not be found)
  auto dummyfunction = addonManager->GetGlobalProc<void*()>(testAddon, "ThisFunctionDoesntExist");
  result             = dummyfunction == nullptr;
  DALI_TEST_EQUALS(result, true, TEST_LOCATION);

  // Get Instance function and call it, expect answer 42
  auto  instanceFunction = addonManager->GetInstanceProc<uint32_t(void*)>(testAddon, "InstanceCall");
  auto* instance         = createInstance();
  auto  answer           = instanceFunction(instance);
  DALI_TEST_EQUALS(answer, 42, TEST_LOCATION);

  // Test lifecycle
  auto GetLifecycleStatus = addonManager->GetGlobalProc<bool()>(testAddon, "GetLifecycleStatus");
  addonManager->Pause();

  DALI_TEST_EQUALS(GetLifecycleStatus(), true, TEST_LOCATION);

  addonManager->Resume();
  DALI_TEST_EQUALS(GetLifecycleStatus(), false, TEST_LOCATION);

  END_TEST;
}