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

// EXTERNAL INCLUDES
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/events/touch-event.h>
#include <dali/public-api/events/wheel-event.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/window-devel.h>
#include <dali/internal/window-system/common/window-impl.h>

namespace Dali
{

namespace DevelWindow
{

void SetPositionSize( Window window, PositionSize positionSize )
{
  GetImplementation( window ).SetPositionSize( positionSize );
}

Dali::RenderTaskList GetRenderTaskList( Window window )
{
  return GetImplementation( window ).GetRenderTaskList();
}

Window Get( Actor actor )
{
  return Internal::Adaptor::Window::Get( actor );
}

EventProcessingFinishedSignalType& EventProcessingFinishedSignal( Window window )
{
  return GetImplementation( window ).EventProcessingFinishedSignal();
}

KeyEventSignalType& KeyEventSignal( Window window )
{
  return GetImplementation( window ).KeyEventSignal();
}

KeyEventGeneratedSignalType& KeyEventGeneratedSignal( Window window )
{
  return GetImplementation( window ).KeyEventGeneratedSignal();
}

TouchSignalType& TouchSignal( Window window )
{
  return GetImplementation( window ).TouchSignal();
}

WheelEventSignalType& WheelEventSignal( Window window )
{
  return GetImplementation( window ).WheelEventSignal();
}

void SetParent( Window window, Window parent )
{
  GetImplementation( window ).SetParent( parent );
}

void Unparent( Window window )
{
  GetImplementation( window ).Unparent();
}

Window GetParent( Window window )
{
  return GetImplementation( window ).GetParent();
}

} // namespace DevelWindow

} // namespace Dali
