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

#include "window-render-surface-cocoa.h"

namespace Dali::Internal::Adaptor
{
WindowRenderSurfaceCocoa::WindowRenderSurfaceCocoa(Dali::PositionSize positionSize, Any surface, bool isTransparent)
  : WindowRenderSurface(positionSize, surface, isTransparent)
  , mReady(false)
{
}

void WindowRenderSurfaceCocoa::StartRender()
{
  std::unique_lock<std::mutex> lock(mCondMutex);
  WindowRenderSurface::StartRender();
  while (!mReady)
  {
    mRenderWait.wait(lock);
  }
}

void WindowRenderSurfaceCocoa::CreateSurface()
{
  std::lock_guard<std::mutex> lock(mCondMutex);
  WindowRenderSurface::CreateSurface();
  mReady = true;
  mRenderWait.notify_all();
}
} // Dali::Internal::Adaptor
