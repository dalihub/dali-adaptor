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

// EXTERNAL INCLUDES
#include <app_common.h>
#include <system_settings.h>
#include <cstdlib>

#if defined(TIZEN_PLATFORM_CONFIG_SUPPORTED) && TIZEN_PLATFORM_CONFIG_SUPPORTED
#include <tzplatform_config.h>
#endif // TIZEN_PLATFORM_CONFIG_SUPPORTED

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/environment-variable.h>
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{

Dali::Integration::SystemSettings::FontSize QueryFontSize()
{
  int fontSize = SYSTEM_SETTINGS_FONT_SIZE_NORMAL;
  if(system_settings_get_value_int(SYSTEM_SETTINGS_KEY_FONT_SIZE, &fontSize) != SYSTEM_SETTINGS_ERROR_NONE)
  {
    DALI_LOG_ERROR("DALI system_settings_get_value_int failed.\n");
    return Dali::Integration::SystemSettings::FontSize::NORMAL;
  }

  switch(fontSize)
  {
    case SYSTEM_SETTINGS_FONT_SIZE_SMALL:
    {
      return Dali::Integration::SystemSettings::FontSize::SMALL;
    }
    case SYSTEM_SETTINGS_FONT_SIZE_NORMAL:
    {
      return Dali::Integration::SystemSettings::FontSize::NORMAL;
    }
    case SYSTEM_SETTINGS_FONT_SIZE_LARGE:
    {
      return Dali::Integration::SystemSettings::FontSize::LARGE;
    }
    case SYSTEM_SETTINGS_FONT_SIZE_HUGE:
    {
      return Dali::Integration::SystemSettings::FontSize::EXTRA_LARGE;
    }
    case SYSTEM_SETTINGS_FONT_SIZE_GIANT:
    {
      return Dali::Integration::SystemSettings::FontSize::GIANT;
    }
  }

  return Dali::Integration::SystemSettings::FontSize::NORMAL;
}

std::string QueryLocale(system_settings_key_e key)
{
  std::string localeString;
  char*       locale = nullptr;
  if(system_settings_get_value_string(key, &locale) == SYSTEM_SETTINGS_ERROR_NONE &&
     locale != nullptr)
  {
    localeString = locale;
    free(locale);
  }

  return localeString;
}

void OnLocaleLanguageChanged(system_settings_key_e, void*)
{
  if(!Dali::Internal::Adaptor::SystemSettings::IsAvailable())
  {
    return;
  }

  auto systemSettings = Dali::Internal::Adaptor::SystemSettings::Get();
  if(systemSettings)
  {
    Dali::Internal::Adaptor::GetImplementation(systemSettings).NotifyLocaleLanguageChanged();
  }
}

void OnFontSizeChanged(system_settings_key_e, void*)
{
  if(!Dali::Internal::Adaptor::SystemSettings::IsAvailable())
  {
    return;
  }

  auto systemSettings = Dali::Internal::Adaptor::SystemSettings::Get();
  if(systemSettings)
  {
    Dali::Internal::Adaptor::GetImplementation(systemSettings).NotifyFontSizeChanged();
  }
}

} // namespace

std::string SystemSettings::GetResourcePath()
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
  if(value != nullptr)
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

std::string SystemSettings::GetDataPath()
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

std::string SystemSettings::QueryLocaleLanguage()
{
  return QueryLocale(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE);
}

std::string SystemSettings::QueryLocaleCountry()
{
  return QueryLocale(SYSTEM_SETTINGS_KEY_LOCALE_COUNTRY);
}

void SystemSettings::Initialize()
{
  if(mInitialized)
  {
    return;
  }

  mLocaleLanguage = QueryLocaleLanguage();
  mFontSize       = QueryFontSize();
  mInitialized    = true;

  if(system_settings_add_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE,
                                    OnLocaleLanguageChanged,
                                    nullptr) != SYSTEM_SETTINGS_ERROR_NONE)
  {
    DALI_LOG_ERROR("DALI system_settings_add_changed_cb for locale failed.\n");
  }
  else
  {
    mLocaleChangedCallbackRegistered = true;
  }

  if(system_settings_add_changed_cb(SYSTEM_SETTINGS_KEY_FONT_SIZE,
                                    OnFontSizeChanged,
                                    nullptr) != SYSTEM_SETTINGS_ERROR_NONE)
  {
    DALI_LOG_ERROR("DALI system_settings_add_changed_cb for font size failed.\n");
  }
  else
  {
    mFontSizeChangedCallbackRegistered = true;
  }
}

void SystemSettings::Shutdown()
{
  if(!mInitialized)
  {
    return;
  }

  mInitialized = false;

  if(mFontSizeChangedCallbackRegistered)
  {
    if(system_settings_remove_changed_cb(SYSTEM_SETTINGS_KEY_FONT_SIZE,
                                         OnFontSizeChanged) != SYSTEM_SETTINGS_ERROR_NONE)
    {
      DALI_LOG_ERROR("DALI system_settings_remove_changed_cb for font size failed.\n");
    }
    mFontSizeChangedCallbackRegistered = false;
  }

  if(mLocaleChangedCallbackRegistered)
  {
    if(system_settings_remove_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE,
                                         OnLocaleLanguageChanged) != SYSTEM_SETTINGS_ERROR_NONE)
    {
      DALI_LOG_ERROR("DALI system_settings_remove_changed_cb for locale failed.\n");
    }
    mLocaleChangedCallbackRegistered = false;
  }
}

std::string SystemSettings::GetLocaleLanguage()
{
  if(mLocaleLanguage.empty())
  {
    mLocaleLanguage = QueryLocaleLanguage();
  }

  return mLocaleLanguage;
}

std::string SystemSettings::GetLocaleCountry() const
{
  return QueryLocaleCountry();
}

void SystemSettings::NotifyLocaleLanguageChanged()
{
  if(!mInitialized)
  {
    return;
  }

  mLocaleLanguage = QueryLocaleLanguage();
  // If DALi needs to expose a locale language changed signal later, emit it from here.
}

void SystemSettings::NotifyFontSizeChanged()
{
  if(!mInitialized)
  {
    return;
  }

  const auto fontSize = QueryFontSize();
  if(fontSize == mFontSize)
  {
    return;
  }

  mFontSize = fontSize;
  mFontSizeChangedSignal.Emit(mFontSize);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
