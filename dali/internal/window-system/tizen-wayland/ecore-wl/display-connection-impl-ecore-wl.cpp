/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/tizen-wayland/ecore-wl/display-connection-impl-ecore-wl.h>

// EXTERNAL_HEADERS
#include <Ecore_Wayland.h>
#include <dali/integration-api/debug.h>

// INTERNAL HEADERS
#include <dali/integration-api/wayland/native-render-surface.h>

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
  mSurfaceType( RenderSurface::ECORE_RENDER_SURFACE )
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

bool DisplayConnectionEcoreWl::InitializeEgl(EglInterface& egl)
{
  EglImplementation& eglImpl = static_cast<EglImplementation&>(egl);

  if( !eglImpl.InitializeGles( mDisplay ) )
  {
    DALI_LOG_ERROR("Failed to initialize GLES.\n");
    return false;
  }

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
    mDisplay = reinterpret_cast< EGLNativeDisplayType >( ecore_wl_display_get() );
  }
}

void DisplayConnectionEcoreWl::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  // calculate DPI
  float xres, yres;

  // 1 inch = 25.4 millimeters
  xres = ecore_wl_dpi_get();
  yres = ecore_wl_dpi_get();

  dpiHorizontal = int(xres + 0.5f);  // rounding
  dpiVertical   = int(yres + 0.5f);
}

EGLNativeDisplayType DisplayConnectionEcoreWl::GetNativeDisplay()
{
  return EGLNativeDisplayType();
}

void DisplayConnectionEcoreWl::ReleaseNativeDisplay()
{

}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
