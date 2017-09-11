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
#include "widget-data-impl.h"

// EXTERNAL INCLUDES

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

Dali::WidgetData WidgetData::New( const char* instanceId, bundle* args, char* content )
{
  WidgetDataPtr widgetData ( new WidgetData( instanceId, args, content ) );
  Dali::WidgetData handle( widgetData.Get() );
  return handle;
}

WidgetData::WidgetData( const char* instanceId, bundle* args, char* content )
:  mArgs(args),
   mInstanceId(instanceId),
   mContent(content)
{
}

WidgetData::~WidgetData()
{
}

const char* WidgetData::GetInstanceId()
{
  return mInstanceId;
}

bundle* WidgetData::GetArgs()
{
  return mArgs;
}

char* WidgetData::GetContent()
{
  return mContent;
}

Dali::Window WidgetData::GetWindow()
{
  return mWindow;
}

void WidgetData::SetArgs( bundle* args )
{
  mArgs = args;
}

void WidgetData::SetContent( char* content )
{
  mContent = content;
}

void WidgetData::SetWindow( Dali::Window window )
{
  mWindow = window;
}

} // Adaptor

} // Internal

} // Dali
