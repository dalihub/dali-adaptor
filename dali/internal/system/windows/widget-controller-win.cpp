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
#include <dali/internal/system/windows/widget-controller-win.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
WidgetImplWin::WidgetImplWin()
{
}

WidgetImplWin::~WidgetImplWin()
{
}

void WidgetImplWin::SetContentInfo(const std::string& contentInfo)
{
}

bool WidgetImplWin::IsKeyEventUsing() const
{
  return false;
}

void WidgetImplWin::SetUsingKeyEvent(bool flag)
{
}

void WidgetImplWin::SetInformation(Dali::Window window, const std::string& widgetId)
{
}

Dali::Window WidgetImplWin::GetWindow() const
{
  return Dali::Window();
}

std::string WidgetImplWin::GetWidgetId() const
{
  return std::string();
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
