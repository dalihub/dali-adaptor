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
#include <dali/internal/adaptor/common/adaptor-impl.h>

// EXTERNAL INCLUDES
#include <app_common.h>

#include <aul.h>
#include <unistd.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
std::string Adaptor::GetApplicationPackageName()
{
  char appname[4096] = {0};
  int  pid           = getpid();
  aul_app_get_pkgname_bypid(pid, appname, sizeof(appname));
  return appname;
}

void Adaptor::GetResourceStoragePath(std::string& path)
{
#ifdef USE_APPFW
  char* pathInt = app_get_resource_path();
  if(pathInt)
  {
    path = pathInt;
    free(pathInt);
  }
  else
  {
    path = "";
  }
#endif
}

void Adaptor::GetDataStoragePath(std::string& path)
{
#ifdef USE_APPFW
  char* pathInt = app_get_data_path();
  if(pathInt)
  {
    path = pathInt;
    free(pathInt);
  }
  else
  {
    path = "";
  }
#endif
}

void Adaptor::GetAppId(std::string& appId)
{
#ifdef USE_APPFW
  char* id;
  app_get_id(&id);
  if(id)
  {
    appId = id;
    free(id);
  }
  else
  {
    appId = "";
  }
#endif
}

void Adaptor::SurfaceInitialized()
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
