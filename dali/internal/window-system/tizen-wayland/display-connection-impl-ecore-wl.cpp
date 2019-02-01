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
#include <dali/internal/window-system/tizen-wayland/display-connection-impl-ecore-wl.h>

// EXTERNAL_HEADERS
#include <dali/integration-api/debug.h>

#ifdef ECORE_WAYLAND2
#include <Ecore_Wl2.h>
#else
#include <Ecore_Wayland.h>
#endif

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

DisplayConnection* DisplayConnectionEcoreWl::New()
{
  DisplayConnection* pDisplayConnection(new DisplayConnectionEcoreWl());

  return pDisplayConnection;
}

DisplayConnectionEcoreWl::DisplayConnectionEcoreWl()
: mDisplay(),
  mSurfaceType( RenderSurface::WINDOW_RENDER_SURFACE ),
  mGraphics( nullptr )
{
}

DisplayConnectionEcoreWl::~DisplayConnectionEcoreWl()
{
  if( mSurfaceType == RenderSurface::NATIVE_RENDER_SURFACE )
  {
    ReleaseNativeDisplay();
  }
}

Any DisplayConnectionEcoreWl::GetDisplay()
{
  return Any( mDisplay );
}

void DisplayConnectionEcoreWl::ConsumeEvents()
{
}

bool DisplayConnectionEcoreWl::InitializeGraphics()
{
  return true;
}

void DisplayConnectionEcoreWl::SetSurfaceType( RenderSurface::Type type )
{
  mSurfaceType = type;

  if( mSurfaceType == RenderSurface::NATIVE_RENDER_SURFACE )
  {
    mDisplay = GetNativeDisplay();
  }
  else
  {
#ifdef ECORE_WAYLAND2
    Ecore_Wl2_Display* display = ecore_wl2_connected_display_get( NULL );
    mDisplay = Any( ecore_wl2_display_get( display ) );
#else
    mDisplay = Any( ecore_wl_display_get() );
#endif
  }
}

void DisplayConnectionEcoreWl::SetGraphicsInterface( Graphics::GraphicsInterface& graphics )
{
  mGraphics = &graphics;
}

Any DisplayConnectionEcoreWl::GetNativeDisplay()
{
  return Any();
}

void DisplayConnectionEcoreWl::ReleaseNativeDisplay()
{
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
