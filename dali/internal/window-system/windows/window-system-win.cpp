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
#include <dali/internal/window-system/windows/keyboard-repeat.h>

// EXTERNAL HEADERS
#include <Windows.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>
#include <dali/integration-api/debug.h>
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
    UINT speed        = 0u;
    UINT delaySetting = 0u;
    if(SystemParametersInfoW(SPI_GETKEYBOARDSPEED, 0u, &speed, 0u) &&
       SystemParametersInfoW(SPI_GETKEYBOARDDELAY, 0u, &delaySetting, 0u))
    {
      mKeyboardRepeatSettings.InitializeFromSystem(speed, delaySetting);
    }
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
    if(!mKeyboardRepeatSettings.Set(rate, delay))
    {
      return false;
    }

    mKeyboardRepeatSettingsChangedSignal.Emit();
    return true;
  }

  bool GetKeyboardRepeatInfo(float& rate, float& delay) override
  {
    mKeyboardRepeatSettings.Get(rate, delay);
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

private:
  WindowsPlatform::KeyboardRepeatSettings mKeyboardRepeatSettings;
};

// The lifetime is managed explicitly by Initialize() / Shutdown().
WindowSystemWin* gWindowSystem{nullptr};
bool             gShutdown{false}; ///< Set by Shutdown(), cleared by Initialize().

WindowSystemWin& GetImpl()
{
  if(!gWindowSystem)
  {
    gWindowSystem = new WindowSystemWin();
  }
  return *gWindowSystem;
}
} // unnamed namespace

void Initialize()
{
  gShutdown = false;
  GetImpl().Initialize();
}

void Shutdown()
{
  if(gWindowSystem)
  {
    gWindowSystem->Shutdown();
    delete gWindowSystem;
    gWindowSystem = nullptr;
  }
  gShutdown = true;
}

WindowSystemBase* GetWindowSystem()
{
  if(gShutdown)
  {
    // Do not create the window system again; that would re-initialize the platform during teardown.
    DALI_LOG_ERROR("WindowSystem is used after Shutdown()\n");
    return nullptr;
  }
  return &GetImpl();
}

} // namespace WindowSystem

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
