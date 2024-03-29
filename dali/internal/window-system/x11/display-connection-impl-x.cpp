/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include <dali/internal/window-system/x11/display-connection-impl-x.h>

// EXTERNAL_HEADERS
#include <dali/integration-api/debug.h>

// INTERNAL HEADERS
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/window-system/x11/pixmap-render-surface-x.h>
#include <dali/internal/window-system/x11/window-system-x.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
DisplayConnectionX11::DisplayConnectionX11()
: mGraphics(nullptr),
  mDisplay(nullptr)
{
}

DisplayConnectionX11::~DisplayConnectionX11()
{
}

Any DisplayConnectionX11::GetDisplay()
{
  return Any(mDisplay);
}

void DisplayConnectionX11::ConsumeEvents()
{
  // Event consumption should only be done in WindowSystemX.
}

bool DisplayConnectionX11::InitializeGraphics()
{
  auto               eglGraphics = static_cast<EglGraphics*>(mGraphics);
  EglImplementation& eglImpl     = eglGraphics->GetEglImplementation();

  if(!eglImpl.InitializeGles(reinterpret_cast<EGLNativeDisplayType>(mDisplay)))
  {
    DALI_LOG_ERROR("Failed to initialize GLES.\n");
    return false;
  }

  return true;
}

void DisplayConnectionX11::SetSurfaceType(Dali::RenderSurfaceInterface::Type type)
{
  if(type == Dali::RenderSurfaceInterface::WINDOW_RENDER_SURFACE)
  {
    mDisplay = WindowSystem::GetImplementation().GetXDisplay();
  }
}

void DisplayConnectionX11::SetGraphicsInterface(GraphicsInterface& graphics)
{
  mGraphics = &graphics;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
