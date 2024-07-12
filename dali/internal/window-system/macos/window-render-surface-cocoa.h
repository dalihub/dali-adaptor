#pragma once

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

// EXTERNAL INCLUDES
#include <condition_variable>
#include <mutex>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/window-render-surface.h>

namespace Dali::Internal::Adaptor
{
/**
 * We must create the EGL Window before we enter the run loop.
 * This specialization ensures this condition is respected.
 */
class WindowRenderSurfaceCocoa : public WindowRenderSurface
{
public:
  /**
   * @copydoc Dali::WindowRenderSurface()
   */
  WindowRenderSurfaceCocoa(Dali::PositionSize positionSize, Any surface, bool isTransparent = true);

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::StartRender()
   */
  void StartRender() override;

  /**
   * @copydoc Dali::Integration::RenderSurfaceInterface::CreateSurface()
   */
  void CreateSurface() override;

private:
  std::mutex              mCondMutex;
  std::condition_variable mRenderWait;
  bool                    mReady;
};
} // namespace Dali::Internal::Adaptor
