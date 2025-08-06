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
#include <dali/devel-api/adaptor-framework/screen-information.h>

namespace Dali
{

bool ScreenInformation::operator==(const ScreenInformation& screenInfo) const
{
  if(screenName != screenInfo.screenName)
  {
    return false;
  }
  if(screenWidth != screenInfo.screenWidth)
  {
    return false;
  }
  if(screenHeight != screenInfo.screenHeight)
  {
    return false;
  }

  return true;
}

std::string ScreenInformation::GetScreenName() const
{
  return screenName;
}

int ScreenInformation::GetScreenWidth() const
{
  return screenWidth;
}

int ScreenInformation::GetScreenHeight() const
{
  return screenHeight;
}

} // namespace Dali
