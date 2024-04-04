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
#include <dali/internal/input/common/virtual-keyboard-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
namespace VirtualKeyboard
{
namespace
{
#if defined(DEBUG_ENABLED)
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_INPUT_METHOD_CONTEXT");
#endif
} // namespace

void Show()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "VirtualKeyboard::Show\n");
}

void Hide()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "VirtualKeyboard::Hide\n");
}

bool IsVisible()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "VirtualKeyboard::IsVisible\n");

  return false;
}

void ApplySettings(const Property::Map& settingsMap)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "VirtualKeyboard::ApplySettings\n");
}

void SetReturnKeyType(const InputMethod::ButtonAction::Type type)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "VirtualKeyboard::SetReturnKeyType\n");
}

Dali::InputMethod::ButtonAction::Type GetReturnKeyType()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "VirtualKeyboard::GetReturnKeyType\n");

  return Dali::InputMethod::ButtonAction::DEFAULT;
}

void EnablePrediction(const bool enable)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "VirtualKeyboard::EnablePrediction\n");
}

bool IsPredictionEnabled()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "VirtualKeyboard::IsPredictionEnabled\n");

  return false;
}

Rect<int> GetSizeAndPosition()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "VirtualKeyboard::GetSizeAndPosition\n");

  Rect<int> ret;
  return ret;
}

void RotateTo(int angle)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "VirtualKeyboard::RotateTo\n");
}

Dali::VirtualKeyboard::TextDirection GetTextDirection()
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "VirtualKeyboard::GetTextDirection\n");

  return Dali::VirtualKeyboard::LeftToRight;
}

} // namespace VirtualKeyboard

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
