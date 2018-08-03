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
#include <dali/internal/system/common/widget-application-impl.h>
#include <dali/integration-api/debug.h>

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
 */
WidgetApplicationPtr Create( int* argc, char **argv[], const std::string& stylesheet );

} // namespace Factory

WidgetApplicationPtr WidgetApplication::New(
  int* argc,
  char **argv[],
  const std::string& stylesheet)
{
  //WidgetApplicationPtr //widgetApplication( new WidgetApplication (argc, argv, stylesheet ) );
  return WidgetApplicationFactory::Create( argc, argv, stylesheet );
}

WidgetApplication::WidgetApplication( int* argc, char** argv[], const std::string& stylesheet )
: Application(argc, argv, stylesheet, Dali::WidgetApplication::OPAQUE, PositionSize(), Framework::WIDGET)
{
  DALI_LOG_ERROR("WidgetApplication is not implemented in UBUNTU profile.\n");
}

WidgetApplication::~WidgetApplication()
{
}

void WidgetApplication::RegisterWidgetCreatingFunction( const std::string& widgetName, Dali::WidgetApplication::CreateWidgetFunction createFunction )
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
