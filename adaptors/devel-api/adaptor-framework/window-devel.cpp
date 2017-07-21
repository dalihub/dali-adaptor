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

// INTERNAL INCLUDES
#include <adaptors/devel-api/adaptor-framework/window-devel.h>
#include <adaptors/common/window-impl.h>

namespace Dali
{

namespace DevelWindow
{

FocusSignalType& FocusChangedSignal( Window window )
{
  return GetImplementation( window ).FocusChangedSignal();
}

void SetAcceptFocus( Window window, bool accept )
{
  GetImplementation( window ).SetAcceptFocus( accept );
}

bool IsFocusAcceptable( Window window )
{
  return GetImplementation( window ).IsFocusAcceptable();
}

void Show( Window window )
{
  GetImplementation( window ).Show();
}

void Hide( Window window )
{
  GetImplementation( window ).Hide();
}

bool IsVisible( Window window )
{
  return GetImplementation( window ).IsVisible();
}

unsigned int GetSupportedAuxiliaryHintCount( Window window )
{
  return GetImplementation( window ).GetSupportedAuxiliaryHintCount();
}

std::string GetSupportedAuxiliaryHint( Window window, unsigned int index )
{
  return GetImplementation( window ).GetSupportedAuxiliaryHint( index );
}

unsigned int AddAuxiliaryHint( Window window, const std::string& hint, const std::string& value )
{
  return GetImplementation( window ).AddAuxiliaryHint( hint, value );
}

bool RemoveAuxiliaryHint( Window window, unsigned int id )
{
  return GetImplementation( window ).RemoveAuxiliaryHint( id );
}

bool SetAuxiliaryHintValue( Window window, unsigned int id, const std::string& value )
{
  return GetImplementation( window ).SetAuxiliaryHintValue( id, value );
}

std::string GetAuxiliaryHintValue( Window window, unsigned int id )
{
  return GetImplementation( window ).GetAuxiliaryHintValue( id );
}

unsigned int GetAuxiliaryHintId( Window window, const std::string& hint )
{
  return GetImplementation( window ).GetAuxiliaryHintId( hint );
}

void SetInputRegion( Window window, const Rect< int >& inputRegion )
{
  return GetImplementation( window ).SetInputRegion( inputRegion );
}

void SetType( Window window, Type type )
{
  GetImplementation( window ).SetType( type );
}

Type GetType( Window window )
{
  return GetImplementation( window ).GetType();
}

bool SetNotificationLevel( Window window, NotificationLevel::Type level )
{
  return GetImplementation( window ).SetNotificationLevel( level );
}

NotificationLevel::Type GetNotificationLevel( Window window )
{
  return GetImplementation( window ).GetNotificationLevel();
}

void SetOpaqueState( Window window, bool opaque )
{
  GetImplementation( window ).SetOpaqueState( opaque );
}

bool IsOpaqueState( Window window )
{
  return GetImplementation( window ).IsOpaqueState();
}

bool SetScreenMode( Window window, ScreenMode::Type screenMode )
{
  return GetImplementation( window ).SetScreenMode( screenMode );
}

ScreenMode::Type GetScreenMode( Window window )
{
  return GetImplementation( window ).GetScreenMode();
}

bool SetBrightness( Window window, int brightness )
{
  return GetImplementation( window ).SetBrightness( brightness );
}

int GetBrightness( Window window )
{
  return GetImplementation( window ).GetBrightness();
}

ResizedSignalType& ResizedSignal( Window window )
{
  return GetImplementation( window ).ResizedSignal();
}

void SetSize( Window window, WindowSize size )
{
  GetImplementation( window ).SetSize( size );
}

WindowSize GetSize( Window window )
{
  return GetImplementation( window ).GetSize();
}

void SetPosition( Window window, WindowPosition position )
{
  GetImplementation( window ).SetPosition( position );
}

WindowPosition GetPosition( Window window )
{
  return GetImplementation( window ).GetPosition();
}

void SetTransparency( Window window, bool transparent )
{
  GetImplementation( window ).SetTransparency( transparent );
}

} // namespace DevelWindow

} // namespace Dali
