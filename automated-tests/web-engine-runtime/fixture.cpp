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
#include "test-plugin.h"
#include <climits>
#include <cstdlib>
#include <string>
#include <vector>

namespace
{
std::vector<std::string> arguments;
std::vector<const char*> pointers{nullptr};
bool started = false;
int initializationCalls = 0;
int profileReleases = 0;
struct Profile { bool owned; };
Dali::WebEngineArgumentsResult Store(uint32_t argc, const char* const* argv)
{
  if(started)
  {
    return Dali::WebEngineArgumentsResult::ALREADY_INITIALIZED;
  }
  if(argc > INT_MAX || (argc && !argv))
  {
    return Dali::WebEngineArgumentsResult::INVALID_ARGUMENT;
  }
  std::vector<std::string> replacement;
  for(uint32_t i = 0; i < argc; ++i)
  {
    if(!argv[i])
    {
      return Dali::WebEngineArgumentsResult::INVALID_ARGUMENT;
    }
    replacement.emplace_back(argv[i]);
  }
  arguments = std::move(replacement);
  pointers.clear();
  for(const auto& argument : arguments)
  {
    pointers.push_back(argument.c_str());
  }
  pointers.push_back(nullptr);
  return Dali::WebEngineArgumentsResult::SUCCESS;
}
void Read(uint32_t* argc, const char* const** argv)
{
  *argc = arguments.size();
  *argv = pointers.data();
}
bool Initialize()
{
  started = true;
  ++initializationCalls;
  return !std::getenv("TEST_INIT_FAILURE");
}
void* Default()
{
  return new Profile{false};
}
void* Create(bool, const char* path)
{
  return path ? new Profile{true} : nullptr;
}
void Destroy(void* token)
{
  auto* profile = static_cast<Profile*>(token);
  profileReleases += profile->owned ? 1 : 0;
  delete profile;
}
Dali::WebEngineContext* Context(void* token)
{
  return std::getenv("TEST_TERMINATED") ? nullptr : reinterpret_cast<Dali::WebEngineContext*>(token);
}
Dali::WebEngineCookieManager* Cookies(void*)
{
  return nullptr;
}
bool View(Dali::WebEnginePlugin* plugin, void* token, uint32_t, uint32_t)
{
  return plugin && token;
}
const Dali::WebEngineProfileApi api{1, sizeof(Dali::WebEngineProfileApi), Store, Read, Initialize,
  Default, Create, Destroy, Context, Cookies, View};
}
extern "C" Dali::WebEnginePlugin* CreateWebEnginePlugin()
{
  return new Dali::RuntimeTestPlugin;
}
extern "C" void DestroyWebEnginePlugin(Dali::WebEnginePlugin* plugin)
{
  delete plugin;
}
extern "C" const Dali::WebEngineProfileApi* GetWebEngineProfileApi(uint32_t version)
{
  return version == 1 && !std::getenv("TEST_UNSUPPORTED") ? &api : nullptr;
}
extern "C" int InitializationCalls()
{
  return initializationCalls;
}
extern "C" int ProfileReleases()
{
  return profileReleases;
}
