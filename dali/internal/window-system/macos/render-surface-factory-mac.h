#pragma once

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

#include <dali/internal/window-system/common/render-surface-factory.h>

namespace Dali::Internal::Adaptor
{

class RenderSurfaceFactoryCocoa : public RenderSurfaceFactory
{
public:
  std::unique_ptr< WindowRenderSurface > CreateWindowRenderSurface(
    Dali::PositionSize positionSize,
    Any surface,
    bool isTransparent = false
  ) override;

  std::unique_ptr< PixmapRenderSurface > CreatePixmapRenderSurface(
    Dali::PositionSize positionSize,
    Any surface,
    bool isTransparent = false
  ) override;

  std::unique_ptr< NativeRenderSurface > CreateNativeRenderSurface(
    SurfaceSize surfaceSize,
    Any surface,
    bool isTransparent = false
  ) override;
};

} // namespace Dali::Internal::Adaptor
