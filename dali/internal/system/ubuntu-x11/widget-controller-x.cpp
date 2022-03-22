/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/ubuntu-x11/widget-controller-x.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
WidgetImplUbuntu::WidgetImplUbuntu()
{
}

WidgetImplUbuntu::~WidgetImplUbuntu()
{
}

void WidgetImplUbuntu::SetContentInfo(const std::string& contentInfo)
{
}

bool WidgetImplUbuntu::IsKeyEventUsing() const
{
  return false;
}

void WidgetImplUbuntu::SetUsingKeyEvent(bool flag)
{
}

void WidgetImplUbuntu::SetInformation(Dali::Window window, const std::string& widgetId)
{
}

Dali::Window WidgetImplUbuntu::GetWindow() const
{
  return Dali::Window();
}

std::string WidgetImplUbuntu::GetWidgetId() const
{
  return std::string();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
