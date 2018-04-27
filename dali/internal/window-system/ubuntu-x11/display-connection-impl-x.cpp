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
#include <dali/internal/window-system/ubuntu-x11/display-connection-impl-x.h>

// EXTERNAL_HEADERS
#include <Ecore_X.h>
#include <dali/integration-api/debug.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/ubuntu-x11/pixmap-render-surface-ecore-x.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

DisplayConnection* DisplayConnectionX11::New()
{
  //DisplayConnection* pDisplayConnection(new DisplayConnection());

  //return pDisplayConnection;
  return nullptr;
}

DisplayConnectionX11::DisplayConnectionX11()
: mDisplay(NULL)
{
}

DisplayConnectionX11::~DisplayConnectionX11()
{
  if(mDisplay)
  {
    XCloseDisplay(mDisplay);
  }
}

Any DisplayConnectionX11::GetDisplay()
{
  return Any(mDisplay);
}

void DisplayConnectionX11::ConsumeEvents()
{
  // check events so that we can flush the queue and avoid any potential memory leaks in X
  // looping if events remain
  int events(0);
  do
  {
    // Check if there are any events in the queue
    events = XEventsQueued(mDisplay, QueuedAfterFlush);

    if (events > 0)
    {
      // Just flush event to prevent memory leak from event queue as the events get built up in
      // memory but are only deleted when we retrieve them
      XEvent ev;
      XNextEvent(mDisplay, &ev);
    }
  }
  while (events > 0);
}

bool DisplayConnectionX11::InitializeEgl(EglInterface& egl)
{
  EglImplementation& eglImpl = static_cast<EglImplementation&>(egl);

  if (!eglImpl.InitializeGles(reinterpret_cast<EGLNativeDisplayType>(mDisplay)))
  {
    DALI_LOG_ERROR("Failed to initialize GLES.\n");
    return false;
  }

  return true;
}

void DisplayConnectionX11::SetSurfaceType( RenderSurface::Type type )
{
  if( type == RenderSurface::WINDOW_RENDER_SURFACE )
  {
    // Because of DDK issue, we need to use separated x display instead of ecore default display
    mDisplay = XOpenDisplay(0);
  }
}

void DisplayConnectionX11::GetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical)
{
  // calculate DPI
  float xres, yres;

  // 1 inch = 25.4 millimeters
  xres = ecore_x_dpi_get();
  yres = ecore_x_dpi_get();

  dpiHorizontal = int(xres + 0.5f);  // rounding
  dpiVertical   = int(yres + 0.5f);
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
