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
#include <dali/internal/system/common/system-settings.h>

// EXTERNAL INCLUDES
#include <app_common.h>

#if defined(TIZEN_PLATFORM_CONFIG_SUPPORTED) && TIZEN_PLATFORM_CONFIG_SUPPORTED
#include <tzplatform_config.h>
#endif // TIZEN_PLATFORM_CONFIG_SUPPORTED
#include <dali/devel-api/adaptor-framework/environment-variable.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace SystemSettings
{
std::string GetResourcePath()
{
  std::string resourcePath = "";
#if defined(TIZEN_PLATFORM_CONFIG_SUPPORTED) && TIZEN_PLATFORM_CONFIG_SUPPORTED
  char* app_rsc_path = app_get_resource_path();
  if(app_rsc_path)
  {
    resourcePath = app_rsc_path;
    free(app_rsc_path);
  }
#else // For backwards compatibility with older Tizen versions

  // "DALI_APPLICATION_PACKAGE" is used to get the already configured Application package path.
  const char* environmentVariable = "DALI_APPLICATION_PACKAGE";
  const char* value               = Dali::EnvironmentVariable::GetEnvironmentVariable(environmentVariable);
  if(value != NULL)
  {
    resourcePath = value;
  }

  if(resourcePath.empty() || resourcePath.back() != '/')
  {
    resourcePath += "/";
  }

#endif // TIZEN_PLATFORM_CONFIG_SUPPORTED

  return resourcePath;
}

std::string GetDataPath()
{
  std::string result;
  char*       dataPath = app_get_data_path();
  if(dataPath)
  {
    result = dataPath;
    free(dataPath);
  }
  return result;
}

} // namespace SystemSettings

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
