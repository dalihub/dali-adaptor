/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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
#include <dali/public-api/adaptor-framework/widget.h>

// INTERNAL INCLUDES
#include <dali/public-api/adaptor-framework/widget-impl.h>

namespace Dali
{
Widget Widget::New()
{
  Internal::Adaptor::WidgetPtr internal = Internal::Adaptor::Widget::New();
  return Widget(internal.Get());
}

Widget::~Widget()
{
}

Widget::Widget()
{
}

Widget::Widget(const Widget& copy) = default;

Widget& Widget::operator=(const Widget& rhs) = default;

Widget::Widget(Widget&& rhs) noexcept = default;

Widget& Widget::operator=(Widget&& rhs) noexcept = default;

Widget::Widget(Internal::Adaptor::Widget* widget)
: BaseHandle(widget)
{
}

} // namespace Dali
