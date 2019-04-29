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

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-graphics.h>

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
}

Any DisplayConnectionWin::GetDisplay()
{
  return Any(mDisplay);
}

void DisplayConnectionWin::ConsumeEvents()
{
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

bool DisplayConnectionWin::InitializeGraphics()
{
  auto eglGraphics = static_cast<EglGraphics *>( mGraphics );
  EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  if( !eglImpl.InitializeGles( reinterpret_cast<EGLNativeDisplayType>( mDisplay ) ) )
  {
    DALI_LOG_ERROR( "Failed to initialize GLES.\n" );
    return false;
  }

  return true;
}

void DisplayConnectionWin::SetSurfaceType( Dali::Integration::RenderSurface::Type type )
{
  if( type == Dali::Integration::RenderSurface::WINDOW_RENDER_SURFACE )
  {
     mDisplay = GetDC( GetForegroundWindow() );
  }
}

void DisplayConnectionWin::SetGraphicsInterface( GraphicsInterface& graphics )
{
  mGraphics = &graphics;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
