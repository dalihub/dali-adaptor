/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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

#include <dali/devel-api/adaptor-framework/accessibility-bridge.h>
#include <dali/devel-api/adaptor-framework/atspi-accessibility.h>

void Dali::AtspiAccessibility::Pause()
{
  if(auto bridge = Dali::Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->Pause();
  }
}

void Dali::AtspiAccessibility::Resume()
{
  if(auto bridge = Dali::Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->Resume();
  }
}

void Dali::AtspiAccessibility::StopReading(bool alsoNonDiscardable)
{
  if(auto bridge = Dali::Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->StopReading(alsoNonDiscardable);
  }
}

bool Dali::AtspiAccessibility::SuppressScreenReader(bool suppress)
{
  if(auto bridge = Dali::Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->SuppressScreenReader(suppress);
    return true;
  }

  return false;
}

void Dali::AtspiAccessibility::Say(const std::string& text, bool discardable, std::function<void(std::string)> callback)
{
  if(auto bridge = Dali::Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->Say(text, discardable, callback);
  }
}

int Dali::AtspiAccessibility::SetForcefully(bool turnOn)
{
  if(turnOn)
  {
    if(auto bridge = Dali::Accessibility::Bridge::GetCurrentBridge())
    {
      bridge->Initialize();
      auto ret = bridge->ForceUp();
      return (int)ret;
    }
  }
  else
  {
    if(auto bridge = Dali::Accessibility::Bridge::GetCurrentBridge())
    {
      bridge->ForceDown();
      return 0;
    }
  }
  return -1;
}

int Dali::AtspiAccessibility::GetStatus()
{
  //0(ATSPI OFF, ScreenReader OFF), 1(ATSPI ON, ScreenReader OFF), 2 (ATSPI OFF, ScreenReader ON), 3(ATSPI ON, ScreenReader ON)
  if(auto bridge = Dali::Accessibility::Bridge::GetCurrentBridge())
  {
    if(bridge->GetScreenReaderEnabled())
    {
      if(bridge->IsEnabled())
      {
        return 3;
      }
      else
      {
        return 2;
      }
    }
    else
    {
      if(bridge->IsEnabled())
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
  }
  return -1;
}

bool Dali::AtspiAccessibility::IsEnabled()
{
  return Dali::Accessibility::IsUp();
}

bool Dali::AtspiAccessibility::IsScreenReaderEnabled()
{
  return Dali::Accessibility::Bridge::GetCurrentBridge()->GetScreenReaderEnabled();
}