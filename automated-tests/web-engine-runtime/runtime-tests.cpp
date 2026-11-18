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

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-profile.h>
#include <cassert>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <iostream>

using Runtime = Dali::WebEngineRuntime;
using Result = Dali::WebEngineArgumentsResult;

int main(int argc, char** argv)
{
  assert(argc >= 2);
  const std::string scenario(argv[1]);
  if(scenario.find("alias-") == 0)
  {
    assert(argc == 5);
    const int explicitType = std::atoi(argv[2]);
    const bool defaultFirst = std::strcmp(argv[3], "default-first") == 0;
    if(std::strcmp(argv[4], "symlink") == 0)
    {
      unsetenv("DALI_WEB_ENGINE_NAME");
    }
    else
    {
      setenv("DALI_WEB_ENGINE_NAME", argv[4], 1);
    }
    assert(Runtime::SelectType(defaultFirst ? -1 : explicitType));
    char argument[] = "--preserve-startup-option";
    char* arguments[] = {argument};
    assert(Runtime::SetArguments(1, arguments) == Result::SUCCESS);
    auto first = defaultFirst ? Dali::WebEngine::New() : Dali::WebEngine::New(explicitType);
    assert(first);
    auto profile = Runtime::GetDefaultProfile();
    assert(profile);
    assert(Runtime::CreateView(first, profile, 10, 10));
    // Resolve the alternate selection for the first time after initialization.
    assert(Runtime::SelectType(defaultFirst ? explicitType : -1));
    auto second = defaultFirst ? Dali::WebEngine::New(explicitType) : Dali::WebEngine::New();
    assert(second);
    assert(profile == Runtime::GetDefaultProfile());
    assert(Runtime::CreateView(second, profile, 10, 10));
    assert(!Runtime::SelectType(1 - explicitType));
    assert(!Dali::WebEngine::New(1 - explicitType));
    assert(Runtime::CreateView(first, profile, 10, 10));
    void* selected = dlopen(explicitType == 0 ? TEST_PLUGIN_NAME : TEST_LWE_PLUGIN_NAME, RTLD_NOW);
    assert(selected);
    auto api = reinterpret_cast<const Dali::WebEngineProfileApi* (*)(uint32_t)>(dlsym(selected, "GetWebEngineProfileApi"))(1);
    uint32_t count = 0;
    const char* const* stored = nullptr;
    api->readArguments(&count, &stored);
    assert(count == 1 && std::strcmp(stored[0], argument) == 0);
    auto calls = reinterpret_cast<int (*)()>(dlsym(selected, "InitializationCalls"));
    assert(calls() == 1);
    dlclose(selected);
    std::cout << scenario << " passed\n";
    return 0;
  }
  assert(Runtime::SelectType(0));
  void* library = dlopen(TEST_PLUGIN_NAME, RTLD_NOW);
  assert(library);
  auto getApi = reinterpret_cast<const Dali::WebEngineProfileApi* (*)(uint32_t)>(dlsym(library, "GetWebEngineProfileApi"));
  auto initCalls = reinterpret_cast<int (*)()>(dlsym(library, "InitializationCalls"));
  auto releases = reinterpret_cast<int (*)()>(dlsym(library, "ProfileReleases"));
  assert(getApi && initCalls && releases);
  if(scenario == "unsupported")
  {
    setenv("TEST_UNSUPPORTED", "1", 1);
    assert(Runtime::SetArguments(0, nullptr) == Result::BACKEND_UNAVAILABLE);
    assert(!Runtime::CreateProfile(false, ""));
    assert(!Runtime::HasInitializationStarted());
    assert(initCalls() == 0);
  }
  else if(scenario == "transfer")
  {
    char program[] = "browser";
    char option[] = "--incognito";
    char* input[] = {program, option};
    assert(Runtime::SetArguments(2, input) == Result::SUCCESS);
    option[2] = 'X';
    assert(!Runtime::HasInitializationStarted());
    assert(initCalls() == 0);
    assert(Runtime::SetArguments(1, nullptr) == Result::INVALID_ARGUMENT);
    char* invalid[] = {nullptr};
    assert(Runtime::SetArguments(1, invalid) == Result::INVALID_ARGUMENT);
    assert(Runtime::SetArguments(UINT32_MAX, invalid) == Result::INVALID_ARGUMENT);
    assert(Runtime::SelectType(1));
    // Roundtrip forces transfer from the second plugin's independent storage.
    assert(Runtime::SelectType(0));
    uint32_t count;
    const char* const* stored;
    getApi(1)->readArguments(&count, &stored);
    assert(count == 2 && stored[count] == nullptr);
    assert(std::strcmp(stored[1], "--incognito") == 0);
    assert(Runtime::SetArguments(0, nullptr) == Result::SUCCESS);
    getApi(1)->readArguments(&count, &stored);
    assert(count == 0 && stored[0] == nullptr);
    auto profile = Runtime::CreateProfile(true, "");
    assert(profile && Runtime::HasInitializationStarted());
    assert(initCalls() == 1);
    assert(Runtime::SetArguments(0, nullptr) == Result::ALREADY_INITIALIZED);
    assert(!Runtime::SelectType(1));
    assert(Runtime::SelectType(0));
  }
  else if(scenario == "failure")
  {
    setenv("TEST_INIT_FAILURE", "1", 1);
    setenv("DALI_WEB_ENGINE_NAME", "chromium", 1);
    assert(Runtime::SelectType(-1));
    assert(!Runtime::HasInitializationStarted());
    assert(!Runtime::CreateProfile(false, ""));
    assert(Runtime::HasInitializationStarted());
    assert(Runtime::SetArguments(0, nullptr) == Result::ALREADY_INITIALIZED);
    unsetenv("TEST_INIT_FAILURE");
    setenv("DALI_WEB_ENGINE_NAME", "chromium", 1);
    assert(Runtime::SelectType(-1));
    assert(!Runtime::SelectType(1));
    assert(!Runtime::GetDefaultProfile());
    assert(initCalls() == 1);
  }
  else if(scenario == "lifetime")
  {
    auto profile = Runtime::CreateProfile(true, "");
    assert(profile);
    auto copy = profile;
    profile.reset();
    assert(releases() == 0 && copy->GetContext());
    copy.reset();
    assert(releases() == 1);
    auto defaultProfile = Runtime::GetDefaultProfile();
    assert(defaultProfile == Runtime::GetDefaultProfile());
    setenv("TEST_TERMINATED", "1", 1);
    assert(!defaultProfile->GetContext());
    assert(!Runtime::GetDefaultProfile());
    defaultProfile.reset();
    assert(releases() == 1);
  }
  else
  {
    return 1;
  }
  dlclose(library);
  std::cout << scenario << " passed\n";
}
