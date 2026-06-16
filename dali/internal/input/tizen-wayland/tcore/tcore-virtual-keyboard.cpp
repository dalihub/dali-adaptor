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

// CLASS HEADER
#include <dali/internal/input/common/virtual-keyboard-impl.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>
#include <algorithm>

// INTERNAL INCLUDES
#include <dali/internal/input/common/input-method-context-impl.h>
#include <dali/internal/input/tizen-wayland/tcore/tcore-virtual-keyboard.h>
#include <dali/internal/system/common/locale-utils.h>

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
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::NoLogging, false, "LOG_VIRTUAL_KEYBOARD");
#endif

#define TOKEN_STRING(x) #x

//forward declarations
void InputPanelGeometryChangedCallback(tizen_core_imf_context_h ctx, int value, void* data);
void InputPanelLanguageChangeCallback(tizen_core_imf_context_h ctx, int value, void* data);

// Signals
Dali::VirtualKeyboard::StatusSignalType gKeyboardStatusSignal;
Dali::VirtualKeyboard::VoidSignalType   gKeyboardResizeSignal;
Dali::VirtualKeyboard::VoidSignalType   gKeyboardLanguageChangedSignal;

void InputPanelStateChangeCallback(tizen_core_imf_context_h ctx, int value, void* data)
{
  switch(value)
  {
    case TIZEN_CORE_IMF_INPUT_PANEL_STATE_SHOW:
    {
      DALI_LOG_INFO(gLogFilter, Debug::General, "VKB TIZEN_CORE_IMF_INPUT_PANEL_STATE_SHOW\n");

      gKeyboardStatusSignal.Emit(true);

      break;
    }

    case TIZEN_CORE_IMF_INPUT_PANEL_STATE_HIDE:
    {
      DALI_LOG_INFO(gLogFilter, Debug::General, "VKB TIZEN_CORE_IMF_INPUT_PANEL_STATE_HIDE\n");

      gKeyboardStatusSignal.Emit(false);

      break;
    }

    case TIZEN_CORE_IMF_INPUT_PANEL_STATE_WILL_SHOW:
    default:
    {
      // Do nothing
      break;
    }
  }
}

void InputPanelLanguageChangeCallback(tizen_core_imf_context_h ctx, int value, void* data)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "VKB InputPanelLanguageChangeCallback\n");

  // Emit the signal that the language has changed
  gKeyboardLanguageChangedSignal.Emit();
}

void InputPanelGeometryChangedCallback(tizen_core_imf_context_h ctx, int value, void* data)
{
  DALI_LOG_INFO(gLogFilter, Debug::General, "VKB InputPanelGeometryChangedCallback\n");

  // Emit signal that the keyboard is resized
  gKeyboardResizeSignal.Emit();
}

} // unnamed namespace

void ConnectCallbacks(tizen_core_imf_context_h imfContext)
{
  if(imfContext)
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "VKB ConnectPanelCallbacks\n");

    tizen_core_imf_context_add_input_panel_event_callback(imfContext, TIZEN_CORE_IMF_INPUT_PANEL_EVENT_STATE, InputPanelStateChangeCallback, nullptr);
    tizen_core_imf_context_add_input_panel_event_callback(imfContext, TIZEN_CORE_IMF_INPUT_PANEL_EVENT_LANGUAGE, InputPanelLanguageChangeCallback, nullptr);
    tizen_core_imf_context_add_input_panel_event_callback(imfContext, TIZEN_CORE_IMF_INPUT_PANEL_EVENT_GEOMETRY, InputPanelGeometryChangedCallback, nullptr);
  }
}

void DisconnectCallbacks(tizen_core_imf_context_h imfContext)
{
  if(imfContext)
  {
    DALI_LOG_INFO(gLogFilter, Debug::General, "VKB DisconnectPanelCallbacks\n");

    tizen_core_imf_context_del_input_panel_event_callback(imfContext, TIZEN_CORE_IMF_INPUT_PANEL_EVENT_STATE, InputPanelStateChangeCallback);
    tizen_core_imf_context_del_input_panel_event_callback(imfContext, TIZEN_CORE_IMF_INPUT_PANEL_EVENT_LANGUAGE, InputPanelLanguageChangeCallback);
    tizen_core_imf_context_del_input_panel_event_callback(imfContext, TIZEN_CORE_IMF_INPUT_PANEL_EVENT_GEOMETRY, InputPanelGeometryChangedCallback);
  }
}

void Show()
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: Show() is deprecated and will be removed from next release. Use InputMethodContext.Activate() instead.\n");
}

void Hide()
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: Hide() is deprecated and will be removed from next release. Use InputMethodContext.Deactivate() instead.\n");
}

bool IsVisible()
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: IsVisible() is deprecated and will be removed from next release.\n");
  return false;
}

void ApplySettings(const Property::Map& settingsMap)
{
  using namespace InputMethod; // Allows exclusion of namespace in TOKEN_STRING.

  for(unsigned long i = 0, count = settingsMap.Count(); i < count; ++i)
  {
    Property::Key key = settingsMap.GetKeyAt(i);
    if(key.type == Property::Key::INDEX)
    {
      continue;
    }

    Property::Value item = settingsMap.GetValue(i);

    if(key == TOKEN_STRING(BUTTON_ACTION))
    {
      if(item.GetType() == Property::INTEGER)
      {
        int value = item.Get<int>();
        VirtualKeyboard::SetReturnKeyType(static_cast<InputMethod::ReturnKeyType>(value));
      }
    }
    else
    {
      DALI_LOG_INFO(gLogFilter, Debug::General, "Provided Settings Key not supported\n");
    }
  }
}

void EnablePrediction(const bool enable)
{
}

bool IsPredictionEnabled()
{
  return false;
}

BoundsInteger GetSizeAndPosition()
{
  int xPos, yPos, width, height;

  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: GetSizeAndPosition() is deprecated and will be removed from next release. Use InputMethodContext.GetInputPanelArea() instead.\n");

  width = height = xPos = yPos = 0;
  return BoundsInteger(xPos, yPos, width, height);
}

Dali::VirtualKeyboard::TextDirection GetTextDirection()
{
  Dali::VirtualKeyboard::TextDirection direction(Dali::VirtualKeyboard::LEFT_TO_RIGHT);
  return direction;
}

} // namespace VirtualKeyboard

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
