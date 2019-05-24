/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/gles/egl-graphics.h>

// EXTERNAL_HEADERS
#include <tbm_dummy_display.h>
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
: mDisplay( NULL ),
  mSurfaceType( Integration::RenderSurface::WINDOW_RENDER_SURFACE ),
  mGraphics( nullptr )
{
}

DisplayConnectionEcoreWl::~DisplayConnectionEcoreWl()
{
  if( mSurfaceType == Integration::RenderSurface::NATIVE_RENDER_SURFACE )
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
  auto eglGraphics = static_cast<EglGraphics *>(mGraphics);
  EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  if( !eglImpl.InitializeGles( mDisplay ) )
  {
    DALI_LOG_ERROR("Failed to initialize GLES.\n");
    return false;
  }

  return true;
}

void DisplayConnectionEcoreWl::SetSurfaceType( Integration::RenderSurface::Type type )
{
  mSurfaceType = type;

  if( mSurfaceType == Integration::RenderSurface::NATIVE_RENDER_SURFACE )
  {
    mDisplay = GetNativeDisplay();
  }
  else
  {
#ifdef ECORE_WAYLAND2
    Ecore_Wl2_Display* display = ecore_wl2_connected_display_get( NULL );
    mDisplay = reinterpret_cast< EGLNativeDisplayType >( ecore_wl2_display_get( display ) );
#else
    mDisplay = reinterpret_cast< EGLNativeDisplayType >( ecore_wl_display_get() );
#endif
  }
}

void DisplayConnectionEcoreWl::SetGraphicsInterface( GraphicsInterface& graphics )
{
  mGraphics = &graphics;
}

EGLNativeDisplayType DisplayConnectionEcoreWl::GetNativeDisplay()
{
  return reinterpret_cast< EGLNativeDisplayType >( tbm_dummy_display_create() );
}

void DisplayConnectionEcoreWl::ReleaseNativeDisplay()
{
  if( mDisplay )
  {
     tbm_dummy_display_destroy( reinterpret_cast< tbm_dummy_display* >( mDisplay ) );
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
