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

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-system.h>

// EXTERNAL HEADERS
#include <Windows.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace WindowSystem
{
namespace
{
static bool gGeometryHittest = false;
} // unnamed namespace

void Initialize()
{
}

void GetScreenSize(int32_t& width, int32_t& height)
{
  width  = GetSystemMetrics(SM_CXSCREEN);
  height = GetSystemMetrics(SM_CYSCREEN);
}

std::vector<Dali::ScreenInformation> GetAvailableScreens()
{
  return std::vector<Dali::ScreenInformation>();
}

void UpdateScreenSize()
{
}

bool SetKeyboardRepeatInfo(float rate, float delay)
{
  return false;
}

bool GetKeyboardRepeatInfo(float& rate, float& delay)
{
  return false;
}

bool SetKeyboardHorizontalRepeatInfo(float rate, float delay)
{
  return false;
}

bool GetKeyboardHorizontalRepeatInfo(float& rate, float& delay)
{
  return false;
}

bool SetKeyboardVerticalRepeatInfo(float rate, float delay)
{
  return false;
}

bool GetKeyboardVerticalRepeatInfo(float& rate, float& delay)
{
  return false;
}

void SetGeometryHittestEnabled(bool enable)
{
  DALI_LOG_RELEASE_INFO("GeometryHittest : %d \n", enable);
  if(gGeometryHittest != enable && Dali::Adaptor::IsAvailable())
  {
    Dali::SceneHolderList sceneHolders = Dali::Adaptor::Get().GetSceneHolders();
    for(auto iter = sceneHolders.begin(); iter != sceneHolders.end(); ++iter)
    {
      if(*iter)
      {
        (*iter).SetGeometryHittestEnabled(enable);
      }
    }
  }
  gGeometryHittest = enable;
}

bool IsGeometryHittestEnabled()
{
  return gGeometryHittest;
}

} // namespace WindowSystem

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
