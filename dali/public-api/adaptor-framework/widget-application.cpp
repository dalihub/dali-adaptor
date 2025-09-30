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
#include <dali/public-api/adaptor-framework/widget-application.h>

// INTERNAL INCLUDES
#include <dali/internal/system/common/widget-application-impl.h>

namespace Dali
{
WidgetApplication WidgetApplication::New(int* argc, char** argv[], const std::string& stylesheet)
{
  Internal::Adaptor::ApplicationPtr preInitializedApplication = Internal::Adaptor::Application::GetPreInitializedApplication();
  if(preInitializedApplication)
  {
    // WidgetApplication can't use pre-initialized application. So reset it.
    preInitializedApplication.Reset();
  }

  WindowData                              windowData;
  Internal::Adaptor::WidgetApplicationPtr internal = Internal::Adaptor::WidgetApplication::New(argc, argv, stylesheet, windowData);
  return WidgetApplication(internal.Get());
}

WidgetApplication::~WidgetApplication()
{
}

WidgetApplication::WidgetApplication()
{
}

WidgetApplication::WidgetApplication(const WidgetApplication& copy) = default;

WidgetApplication& WidgetApplication::operator=(const WidgetApplication& rhs) = default;

WidgetApplication::WidgetApplication(WidgetApplication&& rhs) noexcept = default;

WidgetApplication& WidgetApplication::operator=(WidgetApplication&& rhs) noexcept = default;

void WidgetApplication::RegisterWidgetCreatingFunction(const std::string& widgetName, CreateWidgetFunction createFunction)
{
  Internal::Adaptor::GetImplementation(*this).RegisterWidgetCreatingFunction(widgetName, createFunction);
}

WidgetApplication::WidgetApplication(Internal::Adaptor::WidgetApplication* widgetApplication)
: Application(widgetApplication)
{
}

} // namespace Dali
