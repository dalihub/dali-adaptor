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
#include <dali/public-api/adaptor-framework/timer.h>

// Ubuntu/X development-only font size override:
//   DALI_SYSTEM_FONT_SIZE=SMALL|NORMAL|LARGE|EXTRA_LARGE|GIANT
//   DALI_SYSTEM_FONT_SIZE_WATCH=1 enables polling so runtime setenv() can emit FontSizeChangedSignal().
// This is only implemented by the Ubuntu/X backend.

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace
{

constexpr unsigned int      FONT_SIZE_WATCH_INTERVAL = 250u;
constexpr const char* const FONT_SIZE_ENV            = "DALI_SYSTEM_FONT_SIZE";
constexpr const char* const FONT_SIZE_WATCH_ENV      = "DALI_SYSTEM_FONT_SIZE_WATCH";

bool IsFontSizeWatchEnabled()
{
  const char* value = Dali::EnvironmentVariable::GetEnvironmentVariable(FONT_SIZE_WATCH_ENV);
  return value != nullptr && std::string(value) == "1";
}

Dali::Integration::SystemSettings::FontSize QueryFontSize()
{
  const char* value = Dali::EnvironmentVariable::GetEnvironmentVariable(FONT_SIZE_ENV);
  if(value == nullptr)
  {
    return Dali::Integration::SystemSettings::FontSize::NORMAL;
  }

  const std::string fontSizeString(value);
  if(fontSizeString == "SMALL")
  {
    return Dali::Integration::SystemSettings::FontSize::SMALL;
  }
  if(fontSizeString == "NORMAL")
  {
    return Dali::Integration::SystemSettings::FontSize::NORMAL;
  }
  if(fontSizeString == "LARGE")
  {
    return Dali::Integration::SystemSettings::FontSize::LARGE;
  }
  if(fontSizeString == "EXTRA_LARGE")
  {
    return Dali::Integration::SystemSettings::FontSize::EXTRA_LARGE;
  }
  if(fontSizeString == "GIANT")
  {
    return Dali::Integration::SystemSettings::FontSize::GIANT;
  }

  return Dali::Integration::SystemSettings::FontSize::NORMAL;
}

Dali::Timer& GetFontSizeWatchTimer()
{
  static Dali::Timer timer;
  return timer;
}

bool OnFontSizeWatchTimerTick()
{
  if(!Dali::Internal::Adaptor::SystemSettings::IsAvailable())
  {
    return true;
  }

  auto systemSettings = Dali::Internal::Adaptor::SystemSettings::Get();
  if(systemSettings)
  {
    Dali::Internal::Adaptor::GetImplementation(systemSettings).NotifyFontSizeChanged();
  }

  return true;
}

void StartFontSizeWatchTimer()
{
  auto& timer = GetFontSizeWatchTimer();
  if(!timer)
  {
    timer = Dali::Timer::New(FONT_SIZE_WATCH_INTERVAL);
    timer.TickSignal().Connect(&OnFontSizeWatchTimerTick);
  }

  if(!timer.IsRunning())
  {
    timer.Start();
  }
}

void StopFontSizeWatchTimer()
{
  auto& timer = GetFontSizeWatchTimer();
  if(timer)
  {
    timer.Stop();
    timer.Reset();
  }
}

} // namespace

std::string SystemSettings::GetResourcePath()
{
  // "DALI_APPLICATION_PACKAGE" is used by Ubuntu specifically to get the already configured Application package path.
  const char* ubuntuEnvironmentVariable = "DALI_APPLICATION_PACKAGE";
  const char* value                     = Dali::EnvironmentVariable::GetEnvironmentVariable(ubuntuEnvironmentVariable);
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
  const char* ubuntuEnvironmentVariable = "DALI_APPLICATION_DATA_DIR";
  const char* value                     = Dali::EnvironmentVariable::GetEnvironmentVariable(ubuntuEnvironmentVariable);
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

  mFontSize    = QueryFontSize();
  mInitialized = true;

  if(IsFontSizeWatchEnabled())
  {
    StartFontSizeWatchTimer();
  }
}

void SystemSettings::Shutdown()
{
  if(!mInitialized)
  {
    return;
  }

  StopFontSizeWatchTimer();

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
