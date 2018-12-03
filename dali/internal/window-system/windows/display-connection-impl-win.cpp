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
#include <dali/internal/window-system/windows/display-connection-impl-win.h>

// EXTERNAL_HEADERS
#include <dali/integration-api/debug.h>

// INTERNAL HEADERS
#include <dali/internal/window-system/windows/pixmap-render-surface-win.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

DisplayConnection* DisplayConnectionWin::New()
{
  DisplayConnection* pDisplayConnection(new DisplayConnectionWin());

  return pDisplayConnection;
}

DisplayConnectionWin::DisplayConnectionWin()
: mDisplay(NULL)
{
}

DisplayConnectionWin::~DisplayConnectionWin()
{
  if(mDisplay)
  {
    //XCloseDisplay(mDisplay);
  }
}

Any DisplayConnectionWin::GetDisplay()
{
  return Any(mDisplay);
}

void DisplayConnectionWin::ConsumeEvents()
{
  // check events so that we can flush the queue and avoid any potential memory leaks in X
  // looping if events remain
  int events(0);
  do
  {
    // Check if there are any events in the queue
    //events = XEventsQueued(mDisplay, QueuedAfterFlush);

    //if (events > 0)
    //{
    //  // Just flush event to prevent memory leak from event queue as the events get built up in
    //  // memory but are only deleted when we retrieve them
    //  XEvent ev;
    //  XNextEvent(mDisplay, &ev);
    //}
  }
  while (events > 0);
}

bool DisplayConnectionWin::InitializeEgl(EglInterface& egl)
{
  EglImplementation& eglImpl = static_cast<EglImplementation&>( egl );

  if( !eglImpl.InitializeGles( reinterpret_cast<EGLNativeDisplayType>( mDisplay ) ) )
  {
    DALI_LOG_ERROR( "Failed to initialize GLES.\n" );
    return false;
  }

  return true;
}

void DisplayConnectionWin::SetSurfaceType( RenderSurface::Type type )
{
  if( type == RenderSurface::WINDOW_RENDER_SURFACE )
  {
     mDisplay = GetDC( GetForegroundWindow() );
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
