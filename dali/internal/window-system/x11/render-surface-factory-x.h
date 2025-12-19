#ifndef DALI_INTERNAL_WINDOW_SYSTEM_X11_RENDER_SURFACE_FACTORY_X_H
#define DALI_INTERNAL_WINDOW_SYSTEM_X11_RENDER_SURFACE_FACTORY_X_H

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

#include <dali/internal/window-system/common/render-surface-factory.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{
class RenderSurfaceFactoryX : public RenderSurfaceFactory
{
public:
  std::unique_ptr<WindowRenderSurface> CreateWindowRenderSurface(Dali::PositionSize positionSize, Any surface, bool isTransparent = false) override;

  std::unique_ptr<PixmapRenderSurface> CreatePixmapRenderSurface(Dali::PositionSize positionSize, Any surface, bool isTransparent = false) override;

  std::unique_ptr<NativeRenderSurface> CreateNativeRenderSurface(SurfaceSize surfaceSize, Any surface, bool isTransparent = false) override;

  std::unique_ptr<OffscreenRenderSurface> CreateOffscreenRenderSurface() override;
};

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_WINDOW_SYSTEM_X11_RENDER_SURFACE_FACTORY_X_H
