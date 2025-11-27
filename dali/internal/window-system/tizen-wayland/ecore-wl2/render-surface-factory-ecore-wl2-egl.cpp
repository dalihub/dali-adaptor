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
#include <dali/internal/window-system/tizen-wayland/ecore-wl2/render-surface-factory-ecore-wl2.h>

// INTERNAL HEADERS
#include <dali/internal/offscreen/egl/offscreen-render-surface-egl.h>
#include <dali/internal/window-system/common/display-utils.h>
#include <dali/internal/window-system/common/pixmap-render-surface.h>
#include <dali/internal/window-system/common/window-render-surface.h>
#include <dali/internal/window-system/tizen-wayland/native-render-surface-ecore-wl-egl.h>

// EXTERNAL INCLUDES
#include <memory>

namespace Dali::Internal::Adaptor
{
std::unique_ptr<WindowRenderSurface> RenderSurfaceFactoryEcoreWl2::CreateWindowRenderSurface(Dali::PositionSize positionSize, Any surface, bool isTransparent)
{
  return Utils::MakeUnique<WindowRenderSurface>(positionSize, surface, isTransparent);
}

std::unique_ptr<PixmapRenderSurface> RenderSurfaceFactoryEcoreWl2::CreatePixmapRenderSurface(Dali::PositionSize positionSize, Any surface, bool isTransparent)
{
  return std::unique_ptr<PixmapRenderSurface>(nullptr);
}

std::unique_ptr<NativeRenderSurface> RenderSurfaceFactoryEcoreWl2::CreateNativeRenderSurface(SurfaceSize surfaceSize, Any surface, bool isTransparent)
{
  return Utils::MakeUnique<NativeRenderSurfaceEcoreWl>(surfaceSize, surface, isTransparent);
}

std::unique_ptr<OffscreenRenderSurface> RenderSurfaceFactoryEcoreWl2::CreateOffscreenRenderSurface()
{
  return Utils::MakeUnique<OffscreenRenderSurfaceEgl>();
}

// this should be created from somewhere
std::unique_ptr<RenderSurfaceFactory> GetRenderSurfaceFactory()
{
  // returns Window factory
  return Utils::MakeUnique<RenderSurfaceFactoryEcoreWl2>();
}

} // namespace Dali::Internal::Adaptor
