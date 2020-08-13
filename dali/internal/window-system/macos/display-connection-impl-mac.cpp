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

 // CLASS HEADER
#include <EGL/egl.h>
#include <dali/internal/window-system/macos/display-connection-impl-mac.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/internal/graphics/gles/egl-graphics.h>

namespace Dali::Internal::Adaptor
{

DisplayConnection* DisplayConnectionCocoa::New()
{
  return new DisplayConnectionCocoa();
}

DisplayConnectionCocoa::DisplayConnectionCocoa()
: mGraphics(nullptr)
{
}

DisplayConnectionCocoa::~DisplayConnectionCocoa()
{
}

Any DisplayConnectionCocoa::GetDisplay()
{
  return EGL_DEFAULT_DISPLAY;
}

void DisplayConnectionCocoa::ConsumeEvents()
{
}

bool DisplayConnectionCocoa::InitializeEgl(EglInterface& egl)
{
  EglImplementation& eglImpl = static_cast<EglImplementation&>( egl );

  if(!eglImpl.InitializeGles(EGL_DEFAULT_DISPLAY))
  {
    DALI_LOG_ERROR( "Failed to initialize GLES.\n" );
    return false;
  }

  return true;
}

bool DisplayConnectionCocoa::InitializeGraphics()
{
  auto eglGraphics = static_cast<EglGraphics *>( mGraphics );
  EglImplementation& eglImpl = eglGraphics->GetEglImplementation();

  if(!eglImpl.InitializeGles(EGL_DEFAULT_DISPLAY))
  {
    DALI_LOG_ERROR( "Failed to initialize GLES.\n" );
    return false;
  }

  return true;
}

void DisplayConnectionCocoa::SetSurfaceType( Dali::RenderSurfaceInterface::Type type )
{
}

void DisplayConnectionCocoa::SetGraphicsInterface( GraphicsInterface& graphics )
{
  mGraphics = &graphics;
}

} // namespace Dali::Internal::Adaptor
