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

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/internal/addons/linux/addon-manager-impl-linux.h>
#include <dali/internal/system/common/environment-variables.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

#include <dlfcn.h>
#include <algorithm>
#include <functional>
#include <iterator>
#include <sstream>

namespace Dali
{
namespace Internal
{
AddOnManagerLinux::AddOnManagerLinux() = default;

AddOnManagerLinux::~AddOnManagerLinux() = default;

void AddOnManagerLinux::RegisterAddOnDispatchTable(const AddOnDispatchTable* dispatchTable)
{
  mAddOnNames.emplace_back(dispatchTable->name);
  mAddOnCache.emplace_back();
  mAddOnCache.back().GetGlobalProc   = dispatchTable->GetGlobalProc;
  mAddOnCache.back().GetInstanceProc = dispatchTable->GetInstanceProc;
  mAddOnCache.back().GetAddOnInfo    = dispatchTable->GetAddOnInfo;

  auto& callbacks = mAddOnCache.back().lifecycleCallbacks;
  auto  initEvent = [&callbacks](uint32_t event, void (*fn)()) {
    callbacks[event].function    = fn;
    callbacks[event].initialized = true;
  };

  initEvent(LifecycleCallback::EVENT_START, dispatchTable->OnStart);
  initEvent(LifecycleCallback::EVENT_STOP, dispatchTable->OnStop);
  initEvent(LifecycleCallback::EVENT_PAUSE, dispatchTable->OnPause);
  initEvent(LifecycleCallback::EVENT_RESUME, dispatchTable->OnResume);
}

std::vector<std::string> AddOnManagerLinux::EnumerateAddOns()
{
  if(mAddOnNames.empty())
  {
    // AddOn libs must be separated with ':' character
    const char* addonsLibs = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_ADDONS_LIBS);
    if(!addonsLibs)
    {
      return {};
    }

    // Get the path where addon libs are stored
    const char* addonsPath = Dali::EnvironmentVariable::GetEnvironmentVariable(DALI_ENV_ADDONS_PATH);
    std::string addonsPathStr(addonsPath ? addonsPath : "/usr/lib");

    // Split libs
    std::string              addonLibsStr(addonsLibs);
    std::vector<std::string> results;
    results.emplace_back();

    for(auto&& c : addonLibsStr)
    {
      if(c == ':')
      {
        results.emplace_back();
      }
      else
      {
        results.back() += c;
      }
    }

    const char* EXTENSION_PATH = (addonsPath) ? addonsPath : "/usr/lib";

    for(auto& name : results)
    {
      std::string fullPath(EXTENSION_PATH);
      fullPath += "/";
      fullPath += name;

      // open lib, look for essential symbols. The libary is opened with RTLD_DEEPBIND flag
      // to make sure the local symbol table is going to be used during lookup first.
      auto* handle = dlopen(fullPath.c_str(), RTLD_DEEPBIND | RTLD_LAZY);
      if(handle)
      {
        DALI_ASSERT_ALWAYS(!mAddOnCache.empty() && "AddOnCache should not be empty!");

        auto&     cacheEntry = mAddOnCache.back();
        AddOnInfo info{};
        cacheEntry.GetAddOnInfo(info);
        cacheEntry.info      = info;
        cacheEntry.addOnLib  = fullPath;
        cacheEntry.libHandle = handle;
        cacheEntry.opened    = false;
      }
      else
      {
        DALI_LOG_ERROR("Can't open library: %s, error: %s\n", fullPath.c_str(), dlerror());
      }
    }
  }
  return mAddOnNames;
}

bool AddOnManagerLinux::GetAddOnInfo(const std::string& name, AddOnInfo& info)
{
  if(mAddOnNames.empty())
  {
    EnumerateAddOns();
  }

  if(mAddOnNames.empty())
  {
    return false;
  }

  auto iter = std::find_if(mAddOnCache.begin(), mAddOnCache.end(), [name](AddOnCacheEntry& item) {
    return (item.info.name == name);
  });

  if(iter == mAddOnCache.end())
  {
    return false;
  }

  info = iter->info;
  return true;
}

std::vector<Dali::AddOnLibrary> AddOnManagerLinux::LoadAddOns(const std::vector<std::string>& addonNames)
{
  std::vector<AddOnLibrary> retval{};
  retval.resize(addonNames.size());
  std::fill(retval.begin(), retval.end(), nullptr);

  if(mAddOnCache.empty())
  {
    EnumerateAddOns();
    if(mAddOnCache.empty())
    {
      // no any extensions
      return retval;
    }
  }

  auto nameIndex = 0u;
  for(const auto& name : addonNames)
  {
    auto index = 0u;
    nameIndex++;
    auto iter = std::find_if(mAddOnCache.begin(), mAddOnCache.end(), [&index, name](AddOnCacheEntry& item) {
      ++index;
      return (item.info.name == name);
    });

    if(iter == mAddOnCache.end())
    {
      continue;
    }

    if(!iter->opened && iter->libHandle)
    {
      // Open library, pull symbols and keep the handle
      auto& entry  = *iter;
      entry.opened = true;
    }

    // Store cache index of extension for indirect calling
    // Stored number in this implementation is always +1 (0 is nullptr, unsuccessful)
    retval[nameIndex - 1] = reinterpret_cast<void*>(index);
  }

  return retval;
}

AddOnLibrary AddOnManagerLinux::LoadAddOn(const std::string& addonName, const std::string& libraryName)
{
  AddOnLibrary addOnLibrary = nullptr;

  auto index = 0u;
  auto iter  = std::find_if(mAddOnCache.begin(), mAddOnCache.end(), [&index, &addonName](AddOnCacheEntry& item) {
    ++index;
    return (item.info.name == addonName);
  });

  if(iter != mAddOnCache.end())
  {
    addOnLibrary = reinterpret_cast<void*>(index);
  }
  else
  {
    // Attempt to load the library if not found in the cache
    auto* handle = dlopen(libraryName.c_str(), RTLD_DEEPBIND | RTLD_LAZY);
    if(handle)
    {
      // Can only have one addon per library so just check if the last added item to the cache is the addon we want
      DALI_ASSERT_ALWAYS(!mAddOnCache.empty() && "AddOnCache should not be empty!");

      auto&     cacheEntry = mAddOnCache.back();
      AddOnInfo info{};
      cacheEntry.GetAddOnInfo(info);
      if(info.name == addonName)
      {
        cacheEntry.info      = info;
        cacheEntry.addOnLib  = libraryName;
        cacheEntry.libHandle = handle;
        cacheEntry.opened    = true;
        addOnLibrary         = reinterpret_cast<void*>(mAddOnCache.size());
      }
      else
      {
        DALI_LOG_ERROR("Can't find %s addon in %s library\n", addonName.c_str(), libraryName.c_str());
      }
    }
    else
    {
      DALI_LOG_ERROR("Can't open library: %s, error: %s\n", libraryName.c_str(), dlerror());
    }
  }
  return addOnLibrary;
}

void* AddOnManagerLinux::GetGlobalProc(const Dali::AddOnLibrary& addonHandle, const char* procName)
{
  if(!addonHandle)
  {
    return nullptr;
  }

  auto index = (intptr_t(addonHandle));
  DALI_ASSERT_ALWAYS(index >= 1 && index <= static_cast<intptr_t>(mAddOnCache.size()) && "Invalid AddOn handle!");

  const auto& entry = mAddOnCache[index - 1];

  if(entry.opened && entry.libHandle)
  {
    // First call into dispatch table
    auto retval = entry.GetGlobalProc(procName);
    if(!retval)
    {
      // fallback
      retval = dlsym(entry.libHandle, procName);
    }
    return retval;
  }
  else
  {
    DALI_LOG_ERROR("AddOn: GetGlobalProc() library failed!\n");
  }
  return nullptr;
}

void* AddOnManagerLinux::GetInstanceProc(const Dali::AddOnLibrary& addonHandle, const char* procName)
{
  if(!addonHandle)
  {
    return nullptr;
  }

  auto index = (intptr_t(addonHandle));
  DALI_ASSERT_ALWAYS(index >= 1 && index <= static_cast<intptr_t>(mAddOnCache.size()) && "Invalid AddOn handle!");

  const auto& entry = mAddOnCache[index - 1];
  if(entry.opened && entry.libHandle)
  {
    // First call into dispatch table
    auto retval = entry.GetInstanceProc(procName);
    if(!retval)
    {
      // fallback
      retval = dlsym(entry.libHandle, procName);
    }
    return retval;
  }
  return nullptr;
}

void AddOnManagerLinux::Pause()
{
  InvokeLifecycleFunction(LifecycleCallback::EVENT_PAUSE);
}

void AddOnManagerLinux::Resume()
{
  InvokeLifecycleFunction(LifecycleCallback::EVENT_RESUME);
}

void AddOnManagerLinux::Start()
{
  InvokeLifecycleFunction(LifecycleCallback::EVENT_START);
}

void AddOnManagerLinux::Stop()
{
  InvokeLifecycleFunction(LifecycleCallback::EVENT_STOP);
}

void AddOnManagerLinux::InvokeLifecycleFunction(uint32_t lifecycleEvent)
{
  for(auto& entry : mAddOnCache)
  {
    auto& callback = entry.lifecycleCallbacks[lifecycleEvent];

    // If AddOn didn't auto-register try to pull symbols
    // directly out of the addon
    if(!callback.initialized)
    {
      callback.function    = reinterpret_cast<decltype(callback.function)>(entry.GetGlobalProc(callback.functionName.c_str()));
      callback.initialized = true;
    }
    if(callback.function)
    {
      callback.function();
    }
  }
}

} // namespace Internal
} // namespace Dali