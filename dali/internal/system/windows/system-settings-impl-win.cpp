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

// CLASS HEADER
#include <dali/internal/system/common/system-settings-impl.h>

// EXTERNAL INCLUDES
#include <stdlib.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
std::string SystemSettings::GetResourcePath()
{
  // "DALI_APPLICATION_PACKAGE" is used by Windows specifically to get the already configured Application package path.
  const char* winEnvironmentVariable = "DALI_APPLICATION_PACKAGE";
  const char* value                  = Dali::EnvironmentVariable::GetEnvironmentVariable(winEnvironmentVariable);
  std::string resourcePath;
  if(value != NULL)
  {
    resourcePath = value;
  }

  if(resourcePath.empty() || resourcePath.back() != '/')
  {
    resourcePath += "/";
  }

  return resourcePath;
}

std::string SystemSettings::GetDataPath()
{
  const char* winEnvironmentVariable = "DALI_APPLICATION_DATA_DIR";
  const char* value                  = Dali::EnvironmentVariable::GetEnvironmentVariable(winEnvironmentVariable);
  std::string dataPath;
  if(value != NULL)
  {
    dataPath = value;
  }

  return dataPath;
}

void SystemSettings::Initialize()
{
  if(mInitialized)
  {
    return;
  }

  mInitialized = true;
}

void SystemSettings::Shutdown()
{
  mInitialized = false;
}

std::string SystemSettings::QueryLocaleLanguage()
{
  return "";
}

std::string SystemSettings::QueryLocaleCountry()
{
  return "";
}

std::string SystemSettings::GetLocaleLanguage()
{
  return mLocaleLanguage;
}

std::string SystemSettings::GetLocaleCountry() const
{
  return "";
}

void SystemSettings::NotifyLocaleLanguageChanged()
{
}

void SystemSettings::NotifyFontSizeChanged()
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
