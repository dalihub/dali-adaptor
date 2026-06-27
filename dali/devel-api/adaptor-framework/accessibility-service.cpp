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

#include <dali/integration-api/adaptor-framework/accessibility/accessibility-bridge.h>
#include <dali/integration-api/adaptor-framework/accessibility/accessibility-service.h>

void Dali::Integration::Accessibility::Pause()
{
  if(auto bridge = Dali::Integration::Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->Pause();
  }
}

void Dali::Integration::Accessibility::Resume()
{
  if(auto bridge = Dali::Integration::Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->Resume();
  }
}

void Dali::Integration::Accessibility::StopReading(bool alsoNonDiscardable)
{
  if(auto bridge = Dali::Integration::Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->StopReading(alsoNonDiscardable);
  }
}

bool Dali::Integration::Accessibility::SuppressScreenReader(bool suppress)
{
  if(auto bridge = Dali::Integration::Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->SuppressScreenReader(suppress);
    return true;
  }

  return false;
}

void Dali::Integration::Accessibility::Say(const std::string& text, bool discardable, std::function<void(std::string)> callback)
{
  if(auto bridge = Dali::Integration::Accessibility::Bridge::GetCurrentBridge())
  {
    bridge->Say(text, discardable, callback);
  }
}

bool Dali::Integration::Accessibility::IsEnabled()
{
  return Dali::Integration::Accessibility::IsUp();
}

bool Dali::Integration::Accessibility::IsScreenReaderEnabled()
{
  if(auto bridge = Dali::Integration::Accessibility::Bridge::GetCurrentBridge())
  {
    return bridge->GetScreenReaderEnabled();
  }

  return false;
}
