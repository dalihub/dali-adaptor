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
#include <dali/internal/window-system/x11/display-connection-impl-x.h>

// EXTERNAL_HEADERS

// INTERNAL HEADERS
#include <dali/internal/window-system/x11/window-system-x.h>

#if !defined(VULKAN_ENABLED)
#include <dali/internal/graphics/common/egl-include.h>
#endif

namespace Dali::Internal::Adaptor
{
DisplayConnectionX11::DisplayConnectionX11()
: mDisplay(nullptr)
{
}

DisplayConnectionX11::~DisplayConnectionX11() = default;

Any DisplayConnectionX11::GetDisplay()
{
  return {mDisplay};
}

Any DisplayConnectionX11::GetNativeGraphicsDisplay()
{
#if defined(VULKAN_ENABLED)
  return {nullptr};
#else
  return {static_cast<EGLNativeDisplayType>(mDisplay)};
#endif
}

void DisplayConnectionX11::ConsumeEvents()
{
  // Event consumption should only be done in WindowSystemX.
}

void DisplayConnectionX11::SetSurfaceType(Dali::Integration::RenderSurfaceInterface::Type type)
{
  if(type == Dali::Integration::RenderSurfaceInterface::WINDOW_RENDER_SURFACE)
  {
    mDisplay = WindowSystem::GetImplementation().GetXDisplay();
  }
}

} // namespace Dali::Internal::Adaptor
