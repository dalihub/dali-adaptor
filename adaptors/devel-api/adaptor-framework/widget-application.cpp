/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include "widget-application.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <widget-application-impl.h>

namespace Dali
{

WidgetApplication WidgetApplication::New( int* argc, char **argv[], const std::string& stylesheet )
{
  Internal::Adaptor::WidgetApplicationPtr internal = Internal::Adaptor::WidgetApplication::New( argc, argv, stylesheet);
  return WidgetApplication(internal.Get());
}

WidgetApplication::~WidgetApplication()
{
}

WidgetApplication::WidgetApplication()
{
}

WidgetApplication::WidgetApplication(const WidgetApplication& widgetApplication)
: BaseHandle(widgetApplication)
{
}

WidgetApplication& WidgetApplication::operator=(const WidgetApplication& widgetApplication)
{
  if( *this != widgetApplication )
  {
    BaseHandle::operator=( widgetApplication );
  }
  return *this;
}

void WidgetApplication::MainLoop()
{
  Internal::Adaptor::GetImplementation(*this).MainLoop();
}

void WidgetApplication::Quit()
{
  Internal::Adaptor::GetImplementation(*this).Quit();
}

std::string WidgetApplication::GetResourcePath()
{
  return Internal::Adaptor::WidgetApplication::GetResourcePath();
}

std::string WidgetApplication::GetRegion()
{
  return Internal::Adaptor::GetImplementation(*this).GetRegion();
}

std::string WidgetApplication::GetLanguage()
{
  return Internal::Adaptor::GetImplementation(*this).GetLanguage();
}

WidgetApplication::AppSignalType& WidgetApplication::InitSignal()
{
  return Internal::Adaptor::GetImplementation(*this).InitSignal();
}

WidgetApplication::AppSignalType& WidgetApplication::TerminateSignal()
{
  return Internal::Adaptor::GetImplementation(*this).TerminateSignal();
}

WidgetApplication::AppSignalType& WidgetApplication::LanguageChangedSignal()
{
  return Internal::Adaptor::GetImplementation(*this).LanguageChangedSignal();
}

WidgetApplication::AppSignalType& WidgetApplication::RegionChangedSignal()
{
  return Internal::Adaptor::GetImplementation(*this).RegionChangedSignal();
}

WidgetApplication::LowBatterySignalType& WidgetApplication::LowBatterySignal()
{
  return Internal::Adaptor::GetImplementation(*this).LowBatterySignal();
}

WidgetApplication::LowMemorySignalType& WidgetApplication::LowMemorySignal()
{
  return Internal::Adaptor::GetImplementation(*this).LowMemorySignal();
}

WidgetApplication::WidgetApplication(Internal::Adaptor::WidgetApplication* widgetApplication)
: BaseHandle(widgetApplication)
{
}

} // namespace Dali
