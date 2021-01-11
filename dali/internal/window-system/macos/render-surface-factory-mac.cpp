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
#include <dali/internal/window-system/macos/render-surface-factory-mac.h>

// EXTERNAL HEADERS
#include <dali/integration-api/debug.h>

// INTERNAL HEADERS
#include <dali/integration-api/adaptor-framework/native-render-surface.h>
#include <dali/internal/window-system/common/display-utils.h>
#include <dali/internal/window-system/common/pixmap-render-surface.h>
#include "window-render-surface-cocoa.h"

namespace Dali::Internal::Adaptor
{

std::unique_ptr< WindowRenderSurface >
RenderSurfaceFactoryCocoa::CreateWindowRenderSurface(
  Dali::PositionSize positionSize,
  Any surface,
  bool isTransparent
)
{
  return Utils::MakeUnique< WindowRenderSurfaceCocoa >( positionSize, surface, isTransparent );
}

std::unique_ptr< PixmapRenderSurface >
RenderSurfaceFactoryCocoa::CreatePixmapRenderSurface(
  Dali::PositionSize positionSize,
  Any surface,
  bool isTransparent
)
{
  DALI_LOG_ERROR("Pixmap isn't been supported in Windows");
  return nullptr;
}

std::unique_ptr< NativeRenderSurface >
RenderSurfaceFactoryCocoa::CreateNativeRenderSurface(
  SurfaceSize surfaceSize,
  Any surface,
  bool isTransparent
)
{
  return std::unique_ptr< NativeRenderSurface >( nullptr );
}

// this should be created from somewhere
std::unique_ptr< RenderSurfaceFactory > GetRenderSurfaceFactory()
{
  // returns Window factory
  return Utils::MakeUnique< RenderSurfaceFactoryCocoa >();
}

} // namespace Dali::Adaptor::Internal
