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
#include "widget.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include "widget-impl.h"

namespace Dali
{

Widget Widget::New(const std::string& id)
{
  return Internal::Adaptor::Widget::New(id);
}

Widget::~Widget()
{
}

Widget::Widget()
{
}

Widget::Widget(const Widget& widget)
: BaseHandle(widget)
{
}

Widget& Widget::operator=(const Widget& widget)
{
  if( *this != widget )
  {
    BaseHandle::operator=( widget );
  }
  return *this;
}

Widget::WidgetInstanceCreateSignalType& Widget::CreateSignal()
{
  return Internal::Adaptor::GetImplementation(*this).CreateSignal();
}

Widget::WidgetInstanceTerminateSignalType& Widget::TerminateSignal()
{
  return Internal::Adaptor::GetImplementation(*this).TerminateSignal();
}

Widget::WidgetInstancePauseSignalType& Widget::PauseSignal()
{
  return Internal::Adaptor::GetImplementation(*this).PauseSignal();
}

Widget::WidgetInstanceResumeSignalType& Widget::ResumeSignal()
{
  return Internal::Adaptor::GetImplementation(*this).ResumeSignal();
}

Widget::WidgetInstanceResizeSignalType& Widget::ResizeSignal()
{
  return Internal::Adaptor::GetImplementation(*this).ResizeSignal();
}

Widget::WidgetInstanceUpdateSignalType& Widget::UpdateSignal()
{
  return Internal::Adaptor::GetImplementation(*this).UpdateSignal();
}

Widget::Widget(Internal::Adaptor::Widget* widget)
: BaseHandle(widget)
{
}

} // namespace Dali
