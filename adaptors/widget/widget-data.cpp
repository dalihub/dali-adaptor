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
#include "widget-data.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <widget-data-impl.h>

namespace Dali
{

WidgetData WidgetData::New( const char* instanceId, bundle* args, char* content )
{
  return Internal::Adaptor::WidgetData::New( instanceId, args, content );
}

WidgetData::~WidgetData()
{
}

WidgetData::WidgetData()
{
}

WidgetData::WidgetData(const WidgetData& widgetData)
: BaseHandle(widgetData)
{
}

WidgetData& WidgetData::operator=(const WidgetData& widgetData)
{
  if( *this != widgetData )
  {
    BaseHandle::operator=( widgetData );
  }
  return *this;
}

const char* WidgetData::GetInstanceId()
{
  return Internal::Adaptor::GetImplementation(*this).GetInstanceId();
}

bundle* WidgetData::GetArgs()
{
  return Internal::Adaptor::GetImplementation(*this).GetArgs();
}

char* WidgetData::GetContent()
{
  return Internal::Adaptor::GetImplementation(*this).GetContent();
}

Window WidgetData::GetWindow()
{
  return Internal::Adaptor::GetImplementation(*this).GetWindow();
}

void WidgetData::SetArgs( bundle* args )
{
  Internal::Adaptor::GetImplementation(*this).SetArgs(args);
}

void WidgetData::SetContent( char* content )
{
  Internal::Adaptor::GetImplementation(*this).SetContent(content);
}

void WidgetData::SetWindow( Window window )
{
  Internal::Adaptor::GetImplementation(*this).SetWindow(window);
}

WidgetData::WidgetData(Internal::Adaptor::WidgetData* widgetData)
: BaseHandle(widgetData)
{
}

} // namespace Dali
