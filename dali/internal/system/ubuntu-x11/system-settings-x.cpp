/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <stdlib.h>
#include <string>

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
  // "DALI_APPLICATION_PACKAGE" is used by Ubuntu specifically to get the already configured Application package path.
  const char* ubuntuEnvironmentVariable = "DALI_APPLICATION_PACKAGE";
  char*       value                     = getenv(ubuntuEnvironmentVariable);
  std::string resourcePath;
  if(value != NULL)
  {
    resourcePath = value;
  }

  if(resourcePath.back() != '/')
  {
    resourcePath += "/";
  }

  return resourcePath;
}

std::string GetDataPath()
{
  const char* ubuntuEnvironmentVariable = "DALI_APPLICATION_DATA_DIR";
  char*       value                     = getenv(ubuntuEnvironmentVariable);
  std::string dataPath;
  if(value != NULL)
  {
    dataPath = value;
  }

  return dataPath;
}

} // namespace SystemSettings

int GetLongPressTime( int defaultTime )
{
  return defaultTime;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
