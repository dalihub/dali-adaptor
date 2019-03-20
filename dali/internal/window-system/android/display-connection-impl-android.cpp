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
#include <dali/internal/window-system/android/display-connection-impl-android.h>
#include <dali/internal/graphics/gles/egl-graphics.h>

// EXTERNAL_HEADERS
#include <dali/integration-api/debug.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

DisplayConnection* DisplayConnectionAndroid::New()
{
  DisplayConnection* pDisplayConnection(new DisplayConnectionAndroid());

  return pDisplayConnection;
}

DisplayConnectionAndroid::DisplayConnectionAndroid()
: mDisplay( NULL ),
  mSurfaceType( Integration::RenderSurface::WINDOW_RENDER_SURFACE ),
  mGraphics( nullptr )
{
}

DisplayConnectionAndroid::~DisplayConnectionAndroid()
{
}

Any DisplayConnectionAndroid::GetDisplay()
{
  return Any( mDisplay );
}

void DisplayConnectionAndroid::ConsumeEvents()
{
}

bool DisplayConnectionAndroid::InitializeGraphics()
{
  auto eglGraphics = static_cast<EglGraphics*>( mGraphics );
  EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  if( !eglImpl.InitializeGles( mDisplay ) )
  {
    DALI_LOG_ERROR("Failed to initialize GLES.\n");
    return false;
  }

  return true;
}

void DisplayConnectionAndroid::SetSurfaceType( Integration::RenderSurface::Type type )
{
  mSurfaceType = type;
  mDisplay = EGL_DEFAULT_DISPLAY;
}

void DisplayConnectionAndroid::SetGraphicsInterface( GraphicsInterface& graphics )
{
  mGraphics = &graphics;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
