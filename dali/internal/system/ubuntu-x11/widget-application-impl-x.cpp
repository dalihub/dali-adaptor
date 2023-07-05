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
#include <dali/integration-api/debug.h>
#include <dali/internal/system/ubuntu-x11/widget-application-impl-x.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
WidgetApplicationPtr WidgetApplicationUbuntu::New(
  int*               argc,
  char**             argv[],
  const std::string& stylesheet,
  const WindowData&  windowData)
{
  return new WidgetApplicationUbuntu(argc, argv, stylesheet, windowData);
}

WidgetApplicationUbuntu::WidgetApplicationUbuntu(int* argc, char** argv[], const std::string& stylesheet, const WindowData& windowData)
: WidgetApplication(argc, argv, stylesheet, windowData)
{
  DALI_LOG_ERROR("WidgetApplication is not implemented in UBUNTU profile.\n");
}

WidgetApplicationUbuntu::~WidgetApplicationUbuntu()
{
}

void WidgetApplicationUbuntu::RegisterWidgetCreatingFunction(const std::string& widgetName, Dali::WidgetApplication::CreateWidgetFunction createFunction)
{
}

// factory function, must be implemented
namespace WidgetApplicationFactory
{
/**
 * Create a new widget application
 * @param[in]  argc         A pointer to the number of arguments
 * @param[in]  argv         A pointer to the argument list
 * @param[in]  stylesheet   The path to user defined theme file
 * @param[in]  windowData   The window data
 */
WidgetApplicationPtr Create(int* argc, char** argv[], const std::string& stylesheet, const WindowData& windowData)
{
  return WidgetApplicationUbuntu::New(argc, argv, stylesheet, windowData);
}

} // namespace WidgetApplicationFactory

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
