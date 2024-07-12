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
#include <dali/internal/window-system/windows/display-connection-impl-win.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES

#if !defined(VULKAN_ENABLED)
#include <dali/internal/graphics/common/egl-include.h>
#endif

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
DisplayConnection* DisplayConnectionWin::New()
{
  auto* pDisplayConnection(new DisplayConnectionWin());

  return static_cast<DisplayConnection*>(pDisplayConnection);
}

DisplayConnectionWin::DisplayConnectionWin()
: mDisplay(nullptr),
  mGraphics(nullptr)
{
}

DisplayConnectionWin::~DisplayConnectionWin() = default;

Any DisplayConnectionWin::GetDisplay()
{
  return Any(mDisplay);
}

Any DisplayConnectionWin::GetNativeGraphicsDisplay()
{
#if defined(VULKAN_ENABLED)
  return {nullptr};
#else
  return {eglGetDisplay(mDisplay)};
#endif
}

void DisplayConnectionWin::ConsumeEvents()
{
}

void DisplayConnectionWin::SetSurfaceType(Dali::Integration::RenderSurfaceInterface::Type type)
{
  if(type == Dali::Integration::RenderSurfaceInterface::WINDOW_RENDER_SURFACE)
  {
    mDisplay = GetDC(GetForegroundWindow());
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
