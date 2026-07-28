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

// EXTERNAL HEADERS
#include <Windows.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>
#include <dali/integration-api/debug.h>
#include <algorithm>
#include <cmath>
#include <memory>

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
class WindowSystemWin : public WindowSystemBase
{
public:
  void Initialize()
  {
  }

  void Shutdown()
  {
  }

  void GetScreenSize(int32_t& width, int32_t& height) override
  {
    width  = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);
  }

  bool SetKeyboardRepeatInfo(float rate, float delay) override
  {
    if(!std::isfinite(rate) || !std::isfinite(delay) || rate <= 0.0f || delay < 0.0f)
    {
      return false;
    }

    UINT previousSpeed = 0u;
    if(!SystemParametersInfoW(SPI_GETKEYBOARDSPEED, 0u, &previousSpeed, 0u))
    {
      return false;
    }

    // Windows exposes 32 discrete speeds (approximately 2.5 to 30 repeats
    // per second) and four delays (250 to 1000 ms). DALi uses seconds.
    const float requestedFrequency = 1.0f / rate;
    const auto  speed              = static_cast<UINT>(std::lround(std::clamp((requestedFrequency - 2.5f) * 31.0f / 27.5f, 0.0f, 31.0f)));
    const auto  delaySetting       = static_cast<UINT>(std::lround(std::clamp(delay * 4.0f - 1.0f, 0.0f, 3.0f)));

    if(!SystemParametersInfoW(SPI_SETKEYBOARDSPEED, speed, nullptr, SPIF_SENDCHANGE))
    {
      return false;
    }
    if(!SystemParametersInfoW(SPI_SETKEYBOARDDELAY, delaySetting, nullptr, SPIF_SENDCHANGE))
    {
      SystemParametersInfoW(SPI_SETKEYBOARDSPEED, previousSpeed, nullptr, SPIF_SENDCHANGE);
      return false;
    }

    mKeyboardRepeatSettingsChangedSignal.Emit();
    return true;
  }

  bool GetKeyboardRepeatInfo(float& rate, float& delay) override
  {
    UINT speed        = 0u;
    UINT delaySetting = 0u;
    if(!SystemParametersInfoW(SPI_GETKEYBOARDSPEED, 0u, &speed, 0u) ||
       !SystemParametersInfoW(SPI_GETKEYBOARDDELAY, 0u, &delaySetting, 0u))
    {
      return false;
    }

    const float frequency = 2.5f + std::clamp(speed, 0u, 31u) * 27.5f / 31.0f;
    rate                  = 1.0f / frequency;
    delay                 = 0.25f * (std::clamp(delaySetting, 0u, 3u) + 1u);
    return true;
  }

  bool SetKeyboardHorizontalRepeatInfo(float rate, float delay) override
  {
    return SetKeyboardRepeatInfo(rate, delay);
  }

  bool GetKeyboardHorizontalRepeatInfo(float& rate, float& delay) override
  {
    return GetKeyboardRepeatInfo(rate, delay);
  }

  bool SetKeyboardVerticalRepeatInfo(float rate, float delay) override
  {
    return SetKeyboardRepeatInfo(rate, delay);
  }

  bool GetKeyboardVerticalRepeatInfo(float& rate, float& delay) override
  {
    return GetKeyboardRepeatInfo(rate, delay);
  }
};

std::unique_ptr<WindowSystemWin> gWindowSystem;

WindowSystemWin& GetImpl()
{
  if(!gWindowSystem)
  {
    gWindowSystem = std::make_unique<WindowSystemWin>();
  }
  return *gWindowSystem;
}
} // unnamed namespace

void Initialize()
{
  GetImpl().Initialize();
}

void Shutdown()
{
  if(gWindowSystem)
  {
    gWindowSystem->Shutdown();
    gWindowSystem.reset();
  }
}

WindowSystemBase* GetWindowSystem()
{
  return &GetImpl();
}

} // namespace WindowSystem

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
