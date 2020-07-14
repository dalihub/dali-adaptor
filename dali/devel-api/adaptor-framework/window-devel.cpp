/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

Window New(Any surface, PositionSize windowPosition, const std::string& name, bool isTransparent)
{
  return DevelWindow::New(surface, windowPosition, name, "", isTransparent);
}

Window New(Any surface, PositionSize windowPosition, const std::string& name, const std::string& className, bool isTransparent)
{
  Window newWindow;

  const bool isAdaptorAvailable = Dali::Adaptor::IsAvailable();
  bool isNewWindowAllowed = true;

  if (isAdaptorAvailable)
  {
    Dali::Adaptor& adaptor = Internal::Adaptor::Adaptor::Get();
    isNewWindowAllowed = Internal::Adaptor::Adaptor::GetImplementation(adaptor).IsMultipleWindowSupported();
  }

  if (isNewWindowAllowed)
  {
    Internal::Adaptor::Window* window = Internal::Adaptor::Window::New(surface, windowPosition, name, className, isTransparent);

    Integration::SceneHolder sceneHolder = Integration::SceneHolder(window);
    if (isAdaptorAvailable)
    {
      Dali::Adaptor& adaptor = Internal::Adaptor::Adaptor::Get();
      Internal::Adaptor::Adaptor::GetImplementation(adaptor).AddWindow(sceneHolder, name, className, isTransparent);
    }
    newWindow = Window(window);
  }
  else
  {
    DALI_LOG_ERROR("This device can't support multiple windows.\n");
  }

  return newWindow;
}

void SetPositionSize( Window window, PositionSize positionSize )
{
  GetImplementation( window ).SetPositionSize( positionSize );
}

Window Get( Actor actor )
{
  return Internal::Adaptor::Window::Get( actor );
}

EventProcessingFinishedSignalType& EventProcessingFinishedSignal( Window window )
{
  return GetImplementation( window ).EventProcessingFinishedSignal();
}

WheelEventSignalType& WheelEventSignal( Window window )
{
  return GetImplementation( window ).WheelEventSignal();
}

VisibilityChangedSignalType& VisibilityChangedSignal( Window window )
{
  return GetImplementation( window ).VisibilityChangedSignal();
}

TransitionEffectEventSignalType& TransitionEffectEventSignal( Window window )
{
  return GetImplementation( window ).TransitionEffectEventSignal();
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

Window DownCast( BaseHandle handle )
{
  return Window( dynamic_cast<Dali::Internal::Adaptor::Window*>( handle.GetObjectPtr()) );
}

Dali::Window::WindowOrientation GetCurrentOrientation( Window window )
{
  return GetImplementation( window ).GetCurrentOrientation();
}

void SetAvailableOrientations( Window window, const Dali::Vector<Dali::Window::WindowOrientation>& orientations )
{
  GetImplementation( window ).SetAvailableOrientations( orientations );
}

int32_t GetNativeId( Window window )
{
  return GetImplementation( window ).GetNativeId();
}

void SetDamagedAreas(Window window, std::vector<Dali::Rect<int>>& areas)
{
  GetImplementation(window).SetDamagedAreas(areas);
}

void AddFrameRenderedCallback( Window window, std::unique_ptr< CallbackBase > callback, int32_t frameId )
{
  GetImplementation( window ).AddFrameRenderedCallback( std::move( callback ), frameId );
}

void AddFramePresentedCallback( Window window, std::unique_ptr< CallbackBase > callback, int32_t frameId )
{
  GetImplementation( window ).AddFramePresentedCallback( std::move( callback ), frameId );
}

} // namespace DevelWindow

} // namespace Dali
