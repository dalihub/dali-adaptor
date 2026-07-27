/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/common/egl-include.h>

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
: mDisplay(EGL_DEFAULT_DISPLAY)
{
}

DisplayConnectionWin::~DisplayConnectionWin() = default;

Any DisplayConnectionWin::GetDisplay()
{
  return {mDisplay};
}

Any DisplayConnectionWin::GetNativeGraphicsDisplay()
{
  return {mDisplay};
}

void DisplayConnectionWin::ConsumeEvents()
{
}

void DisplayConnectionWin::SetSurfaceType(Dali::Integration::RenderSurfaceInterface::Type type)
{
  static_cast<void>(type);
  // ANGLE owns the native display connection on Windows.  A DC obtained from
  // the foreground window is neither guaranteed to belong to DALi nor can it
  // be released safely here, so use EGL's platform default just like the
  // Android and macOS display-connection backends.
  mDisplay = EGL_DEFAULT_DISPLAY;
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
