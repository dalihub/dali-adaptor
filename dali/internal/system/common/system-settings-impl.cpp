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
#include <dali/internal/system/common/system-settings-impl.h>

// INTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>

namespace
{
static constexpr Dali::TypeInfoId gSingletonTypeInfoId = DALI_TYPE_ID(Dali::Integration::SystemSettings);
} // unnamed namespace

namespace DALI_NAMESPACE
{
namespace Internal
{
namespace Adaptor
{

Dali::Integration::SystemSettings SystemSettings::Get()
{
  Dali::Integration::SystemSettings systemSettingsHandle;

  Dali::SingletonService service(Dali::SingletonService::Get());
  if(service)
  {
    Dali::BaseHandle handle = service.GetSingleton(gSingletonTypeInfoId);
    if(handle)
    {
      systemSettingsHandle = Dali::Integration::SystemSettings(static_cast<SystemSettings*>(handle.GetObjectPtr()));
    }
    else
    {
      systemSettingsHandle = Dali::Integration::SystemSettings(new SystemSettings);
      service.Register(gSingletonTypeInfoId, systemSettingsHandle);
    }
  }

  return systemSettingsHandle;
}

bool SystemSettings::IsAvailable()
{
  Dali::SingletonService service(Dali::SingletonService::Get());
  if(service)
  {
    return static_cast<bool>(service.GetSingleton(gSingletonTypeInfoId));
  }

  return false;
}

SystemSettings::SystemSettings() = default;

SystemSettings::~SystemSettings()
{
  Shutdown();
}

Dali::Integration::SystemSettings::FontSize SystemSettings::GetFontSize() const
{
  return mFontSize;
}

Dali::Integration::SystemSettings::FontSizeChangedSignalType& SystemSettings::FontSizeChangedSignal()
{
  return mFontSizeChangedSignal;
}

} // namespace Adaptor
} // namespace Internal
} //namespace DALI_NAMESPACE
