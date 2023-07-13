/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/internal/addons/common/addon-manager.h>
#include <dali/internal/addons/common/addon-manager-impl.h>

namespace Dali
{
namespace Internal
{
namespace AddOnManagerFactory
{
__attribute__((weak)) Dali::Integration::AddOnManager* CreateAddOnManager()
{
  return nullptr;
}

} // namespace AddOnManagerFactory
} // namespace Internal

namespace Adaptor
{
AddOnManager::AddOnManager(Internal::AddOnManager* impl)
: Integration::AddOnManager()
{
  mImpl.reset(impl);
}

AddOnManager::~AddOnManager() = default;

void AddOnManager::RegisterAddOnDispatchTable(const AddOnDispatchTable* dispatchTable)
{
  mImpl->RegisterAddOnDispatchTable(dispatchTable);
}

std::vector<std::string> AddOnManager::EnumerateAddOns()
{
  return mImpl->EnumerateAddOns();
}

bool AddOnManager::GetAddOnInfo(const std::string& name, AddOnInfo& info)
{
  return mImpl->GetAddOnInfo(name, info);
}

std::vector<Dali::AddOnLibrary> AddOnManager::LoadAddOns(const std::vector<std::string>& addonNames)
{
  return mImpl->LoadAddOns(addonNames);
}

void* AddOnManager::GetGlobalProc(const Dali::AddOnLibrary& addonHandle, const char* procName)
{
  return mImpl->GetGlobalProc(addonHandle, procName);
}

void* AddOnManager::GetInstanceProc(const Dali::AddOnLibrary& addonHandle, const char* procName)
{
  return mImpl->GetInstanceProc(addonHandle, procName);
}

void AddOnManager::Pause()
{
  mImpl->Pause();
}

void AddOnManager::Resume()
{
  mImpl->Resume();
}

void AddOnManager::Start()
{
  mImpl->Start();
}

void AddOnManager::Stop()
{
  mImpl->Stop();
}

} // namespace Adaptor
} // namespace Dali