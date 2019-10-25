/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <algorithm>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/internal/input/common/input-method-context-impl.h>
#include <dali/internal/input/tizen-wayland/ecore-virtual-keyboard.h>
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
void InputPanelGeometryChangedCallback ( void *data, Ecore_IMF_Context *context, int value );
void InputPanelLanguageChangeCallback( void* data, Ecore_IMF_Context* context, int value );

// Signals
Dali::VirtualKeyboard::StatusSignalType gKeyboardStatusSignal;
Dali::VirtualKeyboard::VoidSignalType   gKeyboardResizeSignal;
Dali::VirtualKeyboard::VoidSignalType   gKeyboardLanguageChangedSignal;

void InputPanelStateChangeCallback( void* data, Ecore_IMF_Context* context, int value )
{
  switch (value)
  {
    case ECORE_IMF_INPUT_PANEL_STATE_SHOW:
    {
      DALI_LOG_INFO( gLogFilter, Debug::General, "VKB ECORE_IMF_INPUT_PANEL_STATE_SHOW\n" );

      gKeyboardStatusSignal.Emit( true );

      break;
    }

    case ECORE_IMF_INPUT_PANEL_STATE_HIDE:
    {
      DALI_LOG_INFO( gLogFilter, Debug::General, "VKB ECORE_IMF_INPUT_PANEL_STATE_HIDE\n" );

      gKeyboardStatusSignal.Emit( false );

      break;
    }

    case ECORE_IMF_INPUT_PANEL_STATE_WILL_SHOW:
    default:
    {
      // Do nothing
      break;
    }
  }
}

void InputPanelLanguageChangeCallback( void* data, Ecore_IMF_Context* context, int value )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "VKB InputPanelLanguageChangeCallback\n" );

  // Emit the signal that the language has changed
  gKeyboardLanguageChangedSignal.Emit();
}

void InputPanelGeometryChangedCallback ( void *data, Ecore_IMF_Context *context, int value )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "VKB InputPanelGeometryChangedCallback\n" );

  // Emit signal that the keyboard is resized
  gKeyboardResizeSignal.Emit();
}

} // unnamed namespace

void ConnectCallbacks( Ecore_IMF_Context *imfContext )
{
  if( imfContext )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "VKB ConnectPanelCallbacks\n" );

    ecore_imf_context_input_panel_event_callback_add( imfContext, ECORE_IMF_INPUT_PANEL_STATE_EVENT,    InputPanelStateChangeCallback,     NULL );
    ecore_imf_context_input_panel_event_callback_add( imfContext, ECORE_IMF_INPUT_PANEL_LANGUAGE_EVENT, InputPanelLanguageChangeCallback,  NULL );
    ecore_imf_context_input_panel_event_callback_add( imfContext, ECORE_IMF_INPUT_PANEL_GEOMETRY_EVENT, InputPanelGeometryChangedCallback, NULL );
  }
}

void DisconnectCallbacks( Ecore_IMF_Context *imfContext )
{
  if( imfContext )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "VKB DisconnectPanelCallbacks\n" );

    ecore_imf_context_input_panel_event_callback_del( imfContext, ECORE_IMF_INPUT_PANEL_STATE_EVENT,    InputPanelStateChangeCallback     );
    ecore_imf_context_input_panel_event_callback_del( imfContext, ECORE_IMF_INPUT_PANEL_LANGUAGE_EVENT, InputPanelLanguageChangeCallback  );
    ecore_imf_context_input_panel_event_callback_del( imfContext, ECORE_IMF_INPUT_PANEL_GEOMETRY_EVENT, InputPanelGeometryChangedCallback );
  }
}

void Show()
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: Show() is deprecated and will be removed from next release. Use InputMethodContext.Activate() instead.\n" );
}

void Hide()
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: Hide() is deprecated and will be removed from next release. Use InputMethodContext.Deactivate() instead.\n" );
}

bool IsVisible()
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: IsVisible() is deprecated and will be removed from next release.\n" );
  return false;
}

void ApplySettings( const Property::Map& settingsMap )
{
  using namespace InputMethod; // Allows exclusion of namespace in TOKEN_STRING.

  for ( unsigned long i = 0, count = settingsMap.Count(); i < count; ++i )
  {
    Property::Key key = settingsMap.GetKeyAt( i );
    if( key.type == Property::Key::INDEX )
    {
      continue;
    }

    Property::Value item = settingsMap.GetValue(i);

    if ( key == TOKEN_STRING( BUTTON_ACTION ) )
    {
      if ( item.GetType() == Property::INTEGER )
      {
        int value = item.Get< int >();
        VirtualKeyboard::SetReturnKeyType( static_cast<InputMethod::ButtonAction::Type>(value) );
      }
    }
    else
    {
      DALI_LOG_INFO( gLogFilter, Debug::General, "Provided Settings Key not supported\n" );
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

Rect<int> GetSizeAndPosition()
{
  int xPos, yPos, width, height;

  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: GetSizeAndPosition() is deprecated and will be removed from next release. Use InputMethodContext.GetInputMethodArea() instead.\n" );

  width = height = xPos = yPos = 0;
  return Rect<int>(xPos,yPos,width,height);
}

Dali::VirtualKeyboard::TextDirection GetTextDirection()
{
  Dali::VirtualKeyboard::TextDirection direction ( Dali::VirtualKeyboard::LeftToRight );
  return direction;
}

} // namespace VirtualKeyboard

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
