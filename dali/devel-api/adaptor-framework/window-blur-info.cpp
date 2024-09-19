/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/window-blur-info.h>

namespace Dali
{

bool WindowBlurInfo::operator==(const WindowBlurInfo& blurInfo) const
{
  if(windowBlurType != blurInfo.windowBlurType)
  {
    return false;
  }
  if(windowBlurRadius != blurInfo.windowBlurRadius)
  {
    return false;
  }
  if(backgroundCornerRadius != blurInfo.backgroundCornerRadius)
  {
    return false;
  }

  return true;
}

void WindowBlurInfo::SetBlurType(WindowBlurType type)
{
  windowBlurType = type;
}

WindowBlurType WindowBlurInfo::GetBlurType() const
{
  return windowBlurType;
}

void WindowBlurInfo::SetBlurRadius(int blurRadius)
{
  windowBlurRadius = blurRadius;
}

int WindowBlurInfo::GetBlurRadius() const
{
  return windowBlurRadius;
}

void WindowBlurInfo::SetCornerRadiusForBackground(int cornerRadius)
{
  backgroundCornerRadius = cornerRadius;
}

int WindowBlurInfo::GetCornerRadiusForBackground() const
{
  return backgroundCornerRadius;
}

} // namespace Dali
