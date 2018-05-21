#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_RENDER_SURFACE_FACTORY_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_RENDER_SURFACE_FACTORY_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/integration-api/render-surface.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/any.h>
#include <memory>

namespace Dali
{

class NativeRenderSurface;

namespace Internal
{
namespace Adaptor
{

class WindowRenderSurface;
class PixmapRenderSurface;

class RenderSurfaceFactory
{
public:

  RenderSurfaceFactory() = default;
  virtual ~RenderSurfaceFactory() = default;

  virtual std::unique_ptr< WindowRenderSurface > CreateWindowRenderSurface( Dali::PositionSize positionSize, Any surface, bool isTransparent = false ) = 0;

  virtual std::unique_ptr< PixmapRenderSurface > CreatePixmapRenderSurface( Dali::PositionSize positionSize, Any surface, bool isTransparent = false ) = 0;

  virtual std::unique_ptr< NativeRenderSurface > CreateNativeRenderSurface( Dali::PositionSize positionSize, bool isTransparent = false ) = 0;
};

extern std::unique_ptr< RenderSurfaceFactory > GetRenderSurfaceFactory();

} // namespace Adaptor
} // namespace Internal
} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_RENDER_SURFACE_FACTORY_H
