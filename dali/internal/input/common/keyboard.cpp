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

// CLASS HEADER
#include <dali/devel-api/adaptor-framework/keyboard.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/window-system.h>

namespace Dali
{
namespace Keyboard
{
bool SetRepeatInfo(float rate, float delay)
{
  return Dali::Internal::Adaptor::WindowSystem::SetKeyboardRepeatInfo(rate, delay);
}

bool GetRepeatInfo(float& rate, float& delay)
{
  return Dali::Internal::Adaptor::WindowSystem::GetKeyboardRepeatInfo(rate, delay);
}

bool SetHorizontalRepeatInfo(float rate, float delay)
{
  return Dali::Internal::Adaptor::WindowSystem::SetKeyboardHorizontalRepeatInfo(rate, delay);
}

bool GetHorizontalRepeatInfo(float& rate, float& delay)
{
  return Dali::Internal::Adaptor::WindowSystem::GetKeyboardHorizontalRepeatInfo(rate, delay);
}

bool SetVerticalRepeatInfo(float rate, float delay)
{
  return Dali::Internal::Adaptor::WindowSystem::SetKeyboardVerticalRepeatInfo(rate, delay);
}

bool GetVerticalRepeatInfo(float& rate, float& delay)
{
  return Dali::Internal::Adaptor::WindowSystem::GetKeyboardVerticalRepeatInfo(rate, delay);
}

} // namespace Keyboard

} // namespace Dali
