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

// CLASS HEADER
#include <dali/internal/adaptor/common/adaptor-impl.h>

// EXTERNAL INCLUDES
#include <app_common.h>
#include <system_settings.h>

#ifdef APPCORE_WATCH_AVAILABLE
#include <screen_connector_provider.h>
#endif

#ifdef ECORE_WAYLAND2
#include <dali/internal/adaptor/tizen-wayland/dali-ecore-wl2.h>
#else
#include <dali/internal/adaptor/tizen-wayland/dali-ecore-wayland.h>
#endif

#include <aul.h>
#include <unistd.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{
static void OnSystemLanguageChanged(system_settings_key_e key, void* data)
{
  char* locale = NULL;
  if(system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale) != SYSTEM_SETTINGS_ERROR_NONE ||
     locale == NULL)
  {
    DALI_LOG_ERROR("DALI OnSystemLanguageChanged failed ");
    return;
  }

  Adaptor* adaptor = static_cast<Adaptor*>(data);
  if(adaptor != NULL)
  {
    adaptor->SetRootLayoutDirection(locale);
  }

  free(locale);
}

} // namespace

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
#ifdef APPCORE_WATCH_AVAILABLE
  if(!mUseRemoteSurface)
  {
    return;
  }
  char* appId;
  app_get_id(&appId);

  // Use strdup() in app_get_id(), so need to free memory
  if(appId)
  {
#ifdef ECORE_WAYLAND2
    Ecore_Wl2_Window* ecoreWlWindow = AnyCast<Ecore_Wl2_Window*>(mWindows.front()->GetNativeHandle());
    screen_connector_provider_remote_enable(appId, ecore_wl2_window_surface_get(ecoreWlWindow));
#else
    Ecore_Wl_Window* ecoreWlWindow = AnyCast<Ecore_Wl_Window*>(mWindows.front()->GetNativeHandle());
    screen_connector_provider_remote_enable(appId, ecore_wl_window_surface_get(ecoreWlWindow));
#endif
    free(appId);
  }
#endif
}

void Adaptor::SetupSystemInformation()
{
  if(system_settings_add_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, OnSystemLanguageChanged, this) != SYSTEM_SETTINGS_ERROR_NONE)
  {
    DALI_LOG_ERROR("DALI system_settings_add_changed_cb failed.\n");
    return;
  }

  char* locale = NULL;
  if(system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale) != SYSTEM_SETTINGS_ERROR_NONE ||
     locale == NULL)
  {
    DALI_LOG_ERROR("DALI OnSystemLanguageChanged failed ");
    return;
  }

  SetRootLayoutDirection(locale);

  free(locale);
}

void Adaptor::RemoveSystemInformation()
{
  if(system_settings_remove_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, OnSystemLanguageChanged) != SYSTEM_SETTINGS_ERROR_NONE)
  {
    DALI_LOG_ERROR("DALI system_settings_remove_changed_cb failed.\n");
    return;
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
