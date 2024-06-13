/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/internal/system/linux/dali-ecore-x.h>

// INTERNAL HEADERS

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
DisplayConnection* DisplayConnectionX11::New()
{
  return nullptr;
  DisplayConnection* pDisplayConnection(new DisplayConnectionX11());
  return pDisplayConnection;
}

DisplayConnectionX11::DisplayConnectionX11()
: mDisplay(nullptr)
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
  return {mDisplay};
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

    if(events > 0)
    {
      // Just flush event to prevent memory leak from event queue as the events get built up in
      // memory but are only deleted when we retrieve them
      XEvent ev;
      XNextEvent(mDisplay, &ev);
    }
  } while(events > 0);
}

void DisplayConnectionX11::SetSurfaceType(Dali::Integration::RenderSurfaceInterface::Type type)
{
  if(type == Dali::Integration::RenderSurfaceInterface::WINDOW_RENDER_SURFACE)
  {
    // Because of DDK issue, we need to use separated x display instead of ecore default display
    mDisplay = XOpenDisplay(0);
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
