/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include "virtual-keyboard-impl.h"

// EXTERNAL INCLUDES
#include <X11/Xlib.h>
#include <Ecore_X.h>
#include <algorithm>

#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/adaptor-framework/common/adaptor.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <internal/common/locale-utils.h>
#include <internal/common/imf-manager-impl.h>


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
Debug::Filter* gLogFilter = Debug::Filter::New(Debug::Verbose, false, "LOG_VIRTUAL_KEYBOARD");
#endif

//forward declarations
void InputPanelGeometryChangedCallback ( void *data, Ecore_IMF_Context *context, int value );
void InputPanelLanguageChangeCallback( void* data, Ecore_IMF_Context* context, int value );

// Signals
Dali::VirtualKeyboard::StatusSignalV2 gKeyboardStatusSignalV2;
Dali::VirtualKeyboard::VoidSignalV2   gKeyboardResizeSignalV2;
Dali::VirtualKeyboard::VoidSignalV2   gKeyboardLanguageChangedSignalV2;

Dali::VirtualKeyboard::ReturnKeyType gReturnKeyType = Dali::VirtualKeyboard::DEFAULT;  // the currently used return key type.

void InputPanelStateChangeCallback( void* data, Ecore_IMF_Context* context, int value )
{
  switch (value)
  {
    case ECORE_IMF_INPUT_PANEL_STATE_SHOW:
    {
      DALI_LOG_INFO( gLogFilter, Debug::General, "VKB ECORE_IMF_INPUT_PANEL_STATE_SHOW\n" );

      gKeyboardStatusSignalV2.Emit( true );

      break;
    }

    case ECORE_IMF_INPUT_PANEL_STATE_HIDE:
    {
      DALI_LOG_INFO( gLogFilter, Debug::General, "VKB ECORE_IMF_INPUT_PANEL_STATE_HIDE\n" );

      gKeyboardStatusSignalV2.Emit( false );

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
  DALI_LOG_INFO( gLogFilter, Debug::General, "VKB InputPanelLanguageChangeCallback" );

  // Emit the signal that the language has changed
  gKeyboardLanguageChangedSignalV2.Emit();
}

void InputPanelGeometryChangedCallback ( void *data, Ecore_IMF_Context *context, int value )
{
  DALI_LOG_INFO( gLogFilter, Debug::General, "VKB InputPanelGeometryChangedCallback\n" );

  // Emit signal that the keyboard is resized
  gKeyboardResizeSignalV2.Emit();
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
  if( Dali::Adaptor::IsAvailable() )
  {
    Dali::ImfManager imfManager = ImfManager::Get();
    Ecore_IMF_Context* imfContext = reinterpret_cast<Ecore_IMF_Context*>( imfManager.GetContext() );

    if( imfContext )
    {
      ecore_imf_context_input_panel_show( imfContext );
    }
  }
}

void Hide()
{
  if( Dali::Adaptor::IsAvailable() )
  {
    Dali::ImfManager imfManager = ImfManager::Get();
    Ecore_IMF_Context* imfContext = reinterpret_cast<Ecore_IMF_Context*>( imfManager.GetContext() );

    if( imfContext )
    {
      ecore_imf_context_input_panel_hide( imfContext );
    }
  }
}

bool IsVisible()
{
  if( Dali::Adaptor::IsAvailable() )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "IsVisible\n" );

    Dali::ImfManager imfManager = ImfManager::Get();
    Ecore_IMF_Context* imfContext = reinterpret_cast<Ecore_IMF_Context*>( imfManager.GetContext() );

    if ( imfContext )
    {
      if (ecore_imf_context_input_panel_state_get(imfContext) == ECORE_IMF_INPUT_PANEL_STATE_SHOW ||
          ecore_imf_context_input_panel_state_get(imfContext) == ECORE_IMF_INPUT_PANEL_STATE_WILL_SHOW)
      {
        return true;
      }
    }
  }

  return false;
}

void SetReturnKeyType( Dali::VirtualKeyboard::ReturnKeyType type )
{
  if ( Dali::Adaptor::IsAvailable() )
  {
    Dali::ImfManager imfManager = ImfManager::Get();
    Ecore_IMF_Context* imfContext = reinterpret_cast<Ecore_IMF_Context*>( imfManager.GetContext() );

    if( imfContext )
    {
      DALI_LOG_INFO( gLogFilter, Debug::General, "VKB Retrun key type is changed[%d]\n", type );

      gReturnKeyType = type;
      ecore_imf_context_input_panel_return_key_type_set( imfContext, static_cast<Ecore_IMF_Input_Panel_Return_Key_Type>( type ) );
    }
  }
}

Dali::VirtualKeyboard::ReturnKeyType GetReturnKeyType()
{
  return gReturnKeyType;
}

void EnablePrediction(const bool enable)
{
  if ( Dali::Adaptor::IsAvailable() )
  {
    Dali::ImfManager imfManager = ImfManager::Get();
    Ecore_IMF_Context* imfContext = reinterpret_cast<Ecore_IMF_Context*>( imfManager.GetContext() );

    if ( imfContext )
    {
      ecore_imf_context_prediction_allow_set( imfContext, (enable)? EINA_TRUE : EINA_FALSE);
    }
  }
}

bool IsPredictionEnabled()
{
  if ( Dali::Adaptor::IsAvailable() )
  {
    Dali::ImfManager imfManager = ImfManager::Get();
    Ecore_IMF_Context* imfContext = reinterpret_cast<Ecore_IMF_Context*>( imfManager.GetContext() );

    if ( imfContext )
    {
      // predictive text is enabled.
      if ( ecore_imf_context_input_panel_enabled_get( imfContext ) == EINA_TRUE )
      {
        return true;
      }
    }
  }

  return false;
}

Rect<int> GetSizeAndPosition()
{
  int xPos, yPos, width, height;

  width = height = xPos = yPos = 0;
  if ( Dali::Adaptor::IsAvailable() )
  {
    Dali::ImfManager imfManager = ImfManager::Get();
    Ecore_IMF_Context* imfContext = reinterpret_cast<Ecore_IMF_Context*>( imfManager.GetContext() );

    if( imfContext )
    {
      ecore_imf_context_input_panel_geometry_get(imfContext, &xPos, &yPos, &width, &height);
    }
    else
    {
      DALI_LOG_WARNING("VKB Unable to get IMF Context so GetSize unavailable\n");
      // return 0 as real size unknown.
    }
  }

  return Rect<int>(xPos,yPos,width,height);
}

void RotateTo(int angle)
{
  // Get focus window used by Keyboard and rotate it
  Display* display = XOpenDisplay(0);
  if (display)
  {
    ::Window focusWindow;
    int revert;
    // Get Focus window
    XGetInputFocus(display, &focusWindow, &revert);

    ecore_x_window_prop_property_set(focusWindow,
                                      ECORE_X_ATOM_E_ILLUME_ROTATE_WINDOW_ANGLE,
                                      ECORE_X_ATOM_CARDINAL, 32, &angle, 1);
    XCloseDisplay(display);
  }
}

Dali::VirtualKeyboard::StatusSignalV2& StatusChangedSignal()
{
  return gKeyboardStatusSignalV2;
}

Dali::VirtualKeyboard::VoidSignalV2& ResizedSignal()
{
  return gKeyboardResizeSignalV2;
}

Dali::VirtualKeyboard::VoidSignalV2& LanguageChangedSignal()
{
  return gKeyboardLanguageChangedSignalV2;
}

Dali::VirtualKeyboard::TextDirection GetTextDirection()
{
  Dali::VirtualKeyboard::TextDirection direction ( Dali::VirtualKeyboard::LeftToRight );

  if ( Dali::Adaptor::IsAvailable() )
  {
    Dali::ImfManager imfManager = ImfManager::Get();

    if ( imfManager )
    {
      Ecore_IMF_Context* imfContext = reinterpret_cast<Ecore_IMF_Context*>( imfManager.GetContext() );

      if ( imfContext )
      {
        char* locale( NULL );
        ecore_imf_context_input_panel_language_locale_get( imfContext, &locale );

        if ( locale )
        {
          direction = Locale::GetTextDirection( std::string( locale ) );
          free( locale );
        }
      }
    }
  }
  return direction;
}

} // namespace VirtualKeyboard

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
