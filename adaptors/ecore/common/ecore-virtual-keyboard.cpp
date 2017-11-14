/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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
#include <algorithm>

#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
// Ecore is littered with C style cast
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include "ecore-virtual-keyboard.h"
#include <adaptor.h>
#include <locale-utils.h>
#include <imf-manager-impl.h>

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
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: Show() is deprecated and will be removed from next release. Use ImfManager.Activate() instead.\n" );

  Dali::ImfManager imfManager = ImfManager::Get(); // Create ImfManager instance (if required) to show the keyboard

  if( imfManager )
  {
    Ecore_IMF_Context* imfContext = ImfManager::GetImplementation( imfManager ).GetContext();

    if( imfContext )
    {
      ecore_imf_context_input_panel_show( imfContext );
    }
  }
}

void Hide()
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: Hide() is deprecated and will be removed from next release. Use ImfManager.Deactivate() instead.\n" );

  if( ImfManager::IsAvailable() /* We do not want to create an ImfManager instance*/ )
  {
    Dali::ImfManager imfManager = ImfManager::Get();
    Ecore_IMF_Context* imfContext = ImfManager::GetImplementation( imfManager ).GetContext();

    if( imfContext )
    {
      ecore_imf_context_input_panel_hide( imfContext );
    }
  }
}

bool IsVisible()
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: IsVisible() is deprecated and will be removed from next release.\n" );

  if( ImfManager::IsAvailable() /* We do not want to create an ImfManager instance */ )
  {
    DALI_LOG_INFO( gLogFilter, Debug::General, "IMF IsVisible\n" );

    Dali::ImfManager imfManager = ImfManager::Get();
    Ecore_IMF_Context* imfContext = ImfManager::GetImplementation( imfManager ).GetContext();

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

void ApplySettings( const Property::Map& settingsMap )
{
  using namespace InputMethod; // Allows exclusion of namespace in TOKEN_STRING.

  for ( unsigned int i = 0, count = settingsMap.Count(); i < count; ++i )
  {
    Property::Key key = settingsMap.GetKeyAt( i );
    if( key.type == Property::Key::INDEX )
    {
      continue;
    }

    Property::Value item = settingsMap.GetValue(i);

    if ( key == TOKEN_STRING( ACTION_BUTTON ) )
    {
      if ( item.GetType() == Property::INTEGER )
      {
        int value = item.Get< int >();
        VirtualKeyboard::SetReturnKeyType( static_cast<InputMethod::ActionButton>(value) );
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
  Dali::ImfManager imfManager = ImfManager::Get(); // Create ImfManager instance (if required) when enabling prediction

  if( imfManager )
  {
    Ecore_IMF_Context* imfContext = ImfManager::GetImplementation( imfManager ).GetContext();

    if ( imfContext )
    {
      ecore_imf_context_prediction_allow_set( imfContext, (enable)? EINA_TRUE : EINA_FALSE);
    }
  }
}

bool IsPredictionEnabled()
{
  if ( ImfManager::IsAvailable() /* We do not want to create an instance of ImfManger */ )
  {
    Dali::ImfManager imfManager = ImfManager::Get();
    Ecore_IMF_Context* imfContext = ImfManager::GetImplementation( imfManager ).GetContext();

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

  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: GetSizeAndPosition() is deprecated and will be removed from next release. Use ImfManager.GetInputMethodArea() instead.\n" );

  width = height = xPos = yPos = 0;
  Dali::ImfManager imfManager = ImfManager::Get(); // Create ImfManager instance (if required) as we may need to do some size related setup in the application

  if( imfManager )
  {
    Ecore_IMF_Context* imfContext = ImfManager::GetImplementation( imfManager ).GetContext();

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

Dali::VirtualKeyboard::StatusSignalType& StatusChangedSignal()
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: StatusChangedSignal() is deprecated and will be removed from next release. Use ImfManager.StatusChangedSignal() instead.\n" );

  Dali::ImfManager imfManager = ImfManager::Get();
  return imfManager.StatusChangedSignal();
}

Dali::VirtualKeyboard::KeyboardResizedSignalType& ResizedSignal()
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: ResizedSignal() is deprecated and will be removed from next release. Use ImfManager.ResizedSignal() instead.\n" );

  Dali::ImfManager imfManager = ImfManager::Get();
  return imfManager.ResizedSignal();
}

Dali::VirtualKeyboard::LanguageChangedSignalType& LanguageChangedSignal()
{
  DALI_LOG_WARNING_NOFN("DEPRECATION WARNING: LanguageChangedSignal() is deprecated and will be removed from next release. Use ImfManager.LanguageChangedSignal() instead.\n" );

  Dali::ImfManager imfManager = ImfManager::Get();
  return imfManager.LanguageChangedSignal();
}

Dali::VirtualKeyboard::TextDirection GetTextDirection()
{
  Dali::VirtualKeyboard::TextDirection direction ( Dali::VirtualKeyboard::LeftToRight );

  if ( ImfManager::IsAvailable() /* We do not want to create an instance of ImfManager */ )
  {
    Dali::ImfManager imfManager = ImfManager::Get();

    if ( imfManager )
    {
      Ecore_IMF_Context* imfContext = ImfManager::GetImplementation( imfManager ).GetContext();

      if ( imfContext )
      {
        char* locale( NULL );
        ecore_imf_context_input_panel_language_locale_get( imfContext, &locale );

        if ( locale )
        {
          direction = static_cast< Dali::VirtualKeyboard::TextDirection >( Locale::GetDirection( std::string( locale ) ) );
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

#pragma GCC diagnostic pop
