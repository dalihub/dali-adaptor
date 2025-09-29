/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/common/widget-application-impl.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
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
WidgetApplicationPtr Create(int* argc, char** argv[], const std::string& stylesheet, const WindowData& windowData);

} // namespace WidgetApplicationFactory

WidgetApplicationPtr WidgetApplication::New(
  int*               argc,
  char**             argv[],
  const std::string& stylesheet,
  const WindowData&  windowData)
{
  bool                                                         preInitializedDataSetted = false;
  Internal::Adaptor::Application::PreInitializeApplicationData preInitializedData;

  // WidgetApplication can't use pre-initialized application.
  // So get pre-initialized window / adaptor and reset it.
  // Note tat we should reset pre-initialized application to remove abort handler.
  Internal::Adaptor::ApplicationPtr preInitializedApplication = Internal::Adaptor::Application::GetPreInitializedApplication();

  if(preInitializedApplication)
  {
    DALI_LOG_RELEASE_INFO("WidgetApplication willnot use pre-initialized application. Destroy it first.\n");
    preInitializedData = preInitializedApplication->ReleasePreInitializedApplicationData();
    preInitializedApplication.Reset();
    preInitializedDataSetted = true;
  }

  // WidgetApplicationPtr //widgetApplication( new WidgetApplication (argc, argv, stylesheet ) );
  auto widgetApplicationPtr = WidgetApplicationFactory::Create(argc, argv, stylesheet, windowData);

  if(preInitializedDataSetted)
  {
    widgetApplicationPtr->ApplyPreInitializedApplicationData(std::move(preInitializedData));
  }
  return widgetApplicationPtr;
}

WidgetApplication::WidgetApplication(int* argc, char** argv[], const std::string& stylesheet, const WindowData& windowData)
: Application(argc, argv, stylesheet, Framework::WIDGET, false, windowData)
{
  DALI_LOG_ERROR("WidgetApplication is not implemented in UBUNTU profile.\n");
}

WidgetApplication::~WidgetApplication()
{
  DALI_LOG_RELEASE_INFO("Application::~WidgetApplication\n");
}

void WidgetApplication::RegisterWidgetCreatingFunction(const std::string& widgetName, Dali::WidgetApplication::CreateWidgetFunction createFunction)
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
