/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

// EXTERNAL_HEADERS
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>
#include <dali/integration-api/debug.h>
#include <cstdio>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
void WindowSystemBase::SetGeometryHittestEnabled(bool enabled)
{
  DALI_LOG_RELEASE_INFO("GeometryHittest : %d \n", enabled);
  if(mGeometryHittest != enabled && Dali::Adaptor::IsAvailable())
  {
    Dali::SceneHolderList sceneHolders = Dali::Adaptor::Get().GetSceneHolders();
    for(auto& sceneHolder : sceneHolders)
    {
      if(sceneHolder)
      {
        sceneHolder.SetGeometryHittestEnabled(enabled);
      }
    }
  }
  mGeometryHittest = enabled;
}

namespace WindowSystem
{
// Implemented per platform (e.g. window-system-ecore-wl2.cpp, window-system-x.cpp, etc.)
// Returns nullptr once Shutdown() has been called; the window system is not re-created.
WindowSystemBase* GetWindowSystem();

namespace
{
static uint32_t gDpiHorizontal = 0u;
static uint32_t gDpiVertical   = 0u;
} //namespace

void SetDpi(uint32_t dpiHorizontal, uint32_t dpiVertical)
{
  gDpiHorizontal = dpiHorizontal;
  gDpiVertical   = dpiVertical;
}

void GetDpi(uint32_t& dpiHorizontal, uint32_t& dpiVertical)
{
  dpiHorizontal = gDpiHorizontal;
  dpiVertical   = gDpiVertical;
}

void GetScreenSize(int32_t& width, int32_t& height)
{
  width  = 0;
  height = 0;
  if(auto* windowSystem = GetWindowSystem())
  {
    windowSystem->GetScreenSize(width, height);
  }
}

std::vector<Dali::ScreenInformation> GetAvailableScreens()
{
  auto* windowSystem = GetWindowSystem();
  return windowSystem ? windowSystem->GetAvailableScreens() : std::vector<Dali::ScreenInformation>{};
}

void UpdateScreenSize()
{
  if(auto* windowSystem = GetWindowSystem())
  {
    windowSystem->UpdateScreenSize();
  }
}

void SetGeometryHittestEnabled(bool enabled)
{
  if(auto* windowSystem = GetWindowSystem())
  {
    windowSystem->SetGeometryHittestEnabled(enabled);
  }
}

bool IsGeometryHittestEnabled()
{
  auto* windowSystem = GetWindowSystem();
  return windowSystem ? windowSystem->IsGeometryHittestEnabled() : false;
}

KeyboardRepeatSettingsChangedSignalType& KeyboardRepeatSettingsChangedSignal()
{
  auto* windowSystem = GetWindowSystem();
  if(!windowSystem)
  {
    // The window system is gone, but a valid reference has to be returned. Hand out a signal that
    // is never emitted. It is deliberately leaked so that it has no destructor running at exit.
    static auto* orphanSignal = new KeyboardRepeatSettingsChangedSignalType();
    return *orphanSignal;
  }
  return windowSystem->KeyboardRepeatSettingsChangedSignal();
}

bool SetKeyboardRepeatInfo(float rate, float delay)
{
  auto* windowSystem = GetWindowSystem();
  return windowSystem ? windowSystem->SetKeyboardRepeatInfo(rate, delay) : false;
}

bool GetKeyboardRepeatInfo(float& rate, float& delay)
{
  auto* windowSystem = GetWindowSystem();
  return windowSystem ? windowSystem->GetKeyboardRepeatInfo(rate, delay) : false;
}

bool SetKeyboardHorizontalRepeatInfo(float rate, float delay)
{
  auto* windowSystem = GetWindowSystem();
  return windowSystem ? windowSystem->SetKeyboardHorizontalRepeatInfo(rate, delay) : false;
}

bool GetKeyboardHorizontalRepeatInfo(float& rate, float& delay)
{
  auto* windowSystem = GetWindowSystem();
  return windowSystem ? windowSystem->GetKeyboardHorizontalRepeatInfo(rate, delay) : false;
}

bool SetKeyboardVerticalRepeatInfo(float rate, float delay)
{
  auto* windowSystem = GetWindowSystem();
  return windowSystem ? windowSystem->SetKeyboardVerticalRepeatInfo(rate, delay) : false;
}

bool GetKeyboardVerticalRepeatInfo(float& rate, float& delay)
{
  auto* windowSystem = GetWindowSystem();
  return windowSystem ? windowSystem->GetKeyboardVerticalRepeatInfo(rate, delay) : false;
}

} // namespace WindowSystem

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
