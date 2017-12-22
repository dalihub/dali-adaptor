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
#include "widget-application-impl.h"

// INTERNAL INCLUDE
#include <widget.h>
#include <widget-impl.h>
#include <widget-controller.h>
#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

WidgetApplicationPtr WidgetApplication::New(
  int* argc,
  char **argv[],
  const std::string& stylesheet)
{
  WidgetApplicationPtr widgetApplication( new WidgetApplication (argc, argv, stylesheet ) );
  return widgetApplication;
}

WidgetApplication::WidgetApplication( int* argc, char** argv[], const std::string& stylesheet )
:Application(argc, argv, stylesheet, Dali::WidgetApplication::OPAQUE, PositionSize(), Framework::WIDGET)
{
  DALI_LOG_ERROR("WidgetApplication is not implemented in 3.0 tizen profile.\n");
}

WidgetApplication::~WidgetApplication()
{
}


void WidgetApplication::RegisterWidgetCreatingFunction( const std::string& widgetName, Dali::WidgetApplication::CreateWidgetFunction createFunction )
{
}

void WidgetApplication::AddWidgetCreatingFunctionPair( CreateWidgetFunctionPair pair )
{
  mCreateWidgetFunctionContainer.push_back( pair );
}

WidgetApplication::CreateWidgetFunctionPair WidgetApplication::GetWidgetCreatingFunctionPair( const std::string& widgetName )
{
  for( CreateWidgetFunctionContainer::const_iterator iter = mCreateWidgetFunctionContainer.begin(); iter != mCreateWidgetFunctionContainer.end(); ++iter )
  {
    if( widgetName.find((*iter).first) != std::string::npos  )
    {
      return *iter;
    }
  }

  return CreateWidgetFunctionPair( "", NULL );
}

void WidgetApplication::AddWidget( widget_base_instance_h widgetBaseInstance, Dali::Widget widget )
{
  mWidgetInstanceContainer.push_back( WidgetInstancePair(widgetBaseInstance, widget) );
}

Dali::Widget WidgetApplication::GetWidget( widget_base_instance_h widgetBaseInstance )
{
  for( WidgetInstanceContainer::const_iterator iter = mWidgetInstanceContainer.begin(); iter != mWidgetInstanceContainer.end(); ++iter )
  {
    if( (*iter).first == widgetBaseInstance  )
    {
      return (*iter).second;
    }
  }
  return Dali::Widget();
}

void WidgetApplication::DeleteWidget( widget_base_instance_h widgetBaseInstance )
{
  for( WidgetInstanceContainer::const_iterator iter = mWidgetInstanceContainer.begin(); iter != mWidgetInstanceContainer.end(); ++iter )
  {
    if( (*iter).first == widgetBaseInstance  )
    {
      mWidgetInstanceContainer.erase(iter);
      break;
    }
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
