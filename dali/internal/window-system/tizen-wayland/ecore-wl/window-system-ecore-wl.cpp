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

// INTERNAL HEADERS
#include <dali/internal/window-system/common/window-system.h>

// EXTERNAL_HEADERS
#include <Ecore_Wayland.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace WindowSystem
{
void Initialize()
{
  ecore_wl_init(NULL);
}

void Shutdown()
{
  ecore_wl_shutdown();
}

void GetScreenSize(int32_t& width, int32_t& height)
{
  ecore_wl_screen_size_get(&width, &height);
}

void UpdateScreenSize()
{
}

bool SetKeyboardRepeatInfo(float rate, float delay)
{
  return ecore_wl_keyboard_repeat_info_set(static_cast<double>(rate), static_cast<double>(delay));
}

bool GetKeyboardRepeatInfo(float& rate, float& delay)
{
  double rateVal, delayVal;
  bool   ret = ecore_wl_keyboard_repeat_info_get(&rateVal, &delayVal);
  rate       = static_cast<float>(rateVal);
  delay      = static_cast<float>(delayVal);

  return ret;
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

} // namespace WindowSystem

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#pragma GCC diagnostic pop
