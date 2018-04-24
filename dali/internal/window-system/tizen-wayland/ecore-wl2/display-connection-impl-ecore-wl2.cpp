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
#include <dali/internal/window-system/tizen-wayland/ecore-wl2/display-connection-impl-ecore-wl2.h>

// EXTERNAL_HEADERS
#include <Ecore_Wl2.h>
#include <dali/integration-api/debug.h>

// INTERNAL HEADERS
#include <dali/integration-api/wayland/native-render-surface.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

DisplayConnection* DisplayConnectionEcoreWl2::New()
{
  DisplayConnection* pDisplayConnection(new DisplayConnectionEcoreWl2());

  return pDisplayConnection;
}

DisplayConnectionEcoreWl2::DisplayConnectionEcoreWl2()
: mDisplay( NULL ),
  mSurfaceType( RenderSurface::ECORE_RENDER_SURFACE )
{
}

DisplayConnectionEcoreWl2::~DisplayConnectionEcoreWl2()
{
  if( mSurfaceType == RenderSurface::NATIVE_RENDER_SURFACE )
  {
    ReleaseNativeDisplay();
  }
}

Any DisplayConnectionEcoreWl2::GetDisplay()
{
  return Any( mDisplay );
}

void DisplayConnectionEcoreWl2::ConsumeEvents()
{
}

bool DisplayConnectionEcoreWl2::InitializeEgl(EglInterface& egl)
{
  EglImplementation& eglImpl = static_cast<EglImplementation&>(egl);

  if( !eglImpl.InitializeGles( mDisplay ) )
  {
    DALI_LOG_ERROR("Failed to initialize GLES.\n");
    return false;
  }

  return true;
}

void DisplayConnectionEcoreWl2::SetSurfaceType( RenderSurface::Type type )
{
  mSurfaceType = type;

  if( mSurfaceType == RenderSurface::NATIVE_RENDER_SURFACE )
  {
    mDisplay = GetNativeDisplay();
  }
  else
  {
    Ecore_Wl2_Display* ecore_wl2_display = ecore_wl2_connected_display_get(NULL);
    mDisplay = reinterpret_cast< EGLNativeDisplayType >( ecore_wl2_display_get(ecore_wl2_display) );
  }
}

void DisplayConnectionEcoreWl2::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  // calculate DPI
  float xres, yres;

  // 1 inch = 25.4 millimeters
#if 0
  Ecore_Wl2_Window *ecore_wl2_window_find(unsigned int id);
  Ecore_Wl2_Window *ecore_wl2_window_surface_find(struct wl_surface *surface);

  Ecore_Wl2_Output *ecore_wl2_window_output_find(Ecore_Wl2_Window *window);
  ecore_wl2_window_output_find(wdata->win);
  xres = ecore_wl2_output_dpi_get();
  yres = ecore_wl2_output_dpi_get();
#endif
  xres = 293.0;
  yres = 293.0;
  dpiHorizontal = int(xres + 0.5f);  // rounding
  dpiVertical   = int(yres + 0.5f);
}

void DisplayConnectionEcoreWl2::GetDpi(Any nativeWindow, unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
    // calculate DPI
    float xres, yres;

    Ecore_Wl2_Output *wl2_output = ecore_wl2_window_output_find(AnyCast<Ecore_Wl2_Window*>( nativeWindow ));

    if (!wl2_output)
    {
      DALI_LOG_ERROR("Failed to get Ecore_Wl2_Output.\n");
      return;
    }
    xres = ecore_wl2_output_dpi_get(wl2_output);
    yres = ecore_wl2_output_dpi_get(wl2_output);

    dpiHorizontal = int(xres + 0.5f);  // rounding
    dpiVertical   = int(yres + 0.5f);
}

EGLNativeDisplayType DisplayConnectionEcoreWl2::GetNativeDisplay()
{
  return EGLNativeDisplayType();
}

void DisplayConnectionEcoreWl2::ReleaseNativeDisplay()
{

}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
